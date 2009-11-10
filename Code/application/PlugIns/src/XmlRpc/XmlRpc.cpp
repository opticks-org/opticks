/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppAssert.h"
#include "AppVerify.h"
#include "XmlRpc.h"
#include "XmlRpcArrayParam.h"
#include "XmlRpcParam.h"
#include "XmlRpcStructParam.h"
#include "xmlwriter.h"
#include <QtCore/QDateTime>

using namespace std;
XERCES_CPP_NAMESPACE_USE

namespace
{
   XmlRpcParam *parseValueNode(DOMNode* pValueTypeNode)
   {
      if (pValueTypeNode->getNodeType() == DOMNode::TEXT_NODE)
      {
         const XMLCh* pTextData = pValueTypeNode->getNodeValue();
         if ((pTextData != NULL) && (XMLString::stringLen(pTextData) > 0))
         {
            QString buf = A(pTextData);
            return new XmlRpcParam("string", buf);
         }
      }
      string nodeName = A(pValueTypeNode->getNodeName());
      if (nodeName == "i4" || nodeName == "int")
      {
         QString buf = A(pValueTypeNode->getFirstChild()->getNodeValue());
         return new XmlRpcParam(INT_PARAM, buf.toInt());
      }
      else if (nodeName == "boolean")
      {
         QString buf = A(pValueTypeNode->getFirstChild()->getNodeValue());
         return new XmlRpcParam(BOOLEAN_PARAM, QVariant(buf.toInt() != 0));
      }
      else if (nodeName == "string")
      {
         QString buf = A(pValueTypeNode->getFirstChild()->getNodeValue());
         return new XmlRpcParam(STRING_PARAM, buf);
      }
      else if (nodeName == "double")
      {
         QString buf = A(pValueTypeNode->getFirstChild()->getNodeValue());
         return new XmlRpcParam(DOUBLE_PARAM, buf.toDouble());
      }
      else if (nodeName == "dateTime.iso8601")
      {
         QString buf = A(pValueTypeNode->getFirstChild()->getNodeValue());
         return new XmlRpcParam(DATE_PARAM, QDate::fromString(buf));
      }
      else if (nodeName == "base64")
      {
         QString buf = A(pValueTypeNode->getFirstChild()->getNodeValue());
         return new XmlRpcParam(BASE64_PARAM, buf);
      }
      else if (nodeName == "array")
      {
         XmlRpcArrayParam* pArray = new XmlRpcArrayParam;
         DOMElement* pDataElement = static_cast<DOMElement*>(
            static_cast<DOMElement*>(pValueTypeNode)->getElementsByTagName(X("data"))->item(0));
         for (DOMNode *pArrayMember = pDataElement->getFirstChild();
            pArrayMember != NULL;
            pArrayMember = pArrayMember->getNextSibling())
         {
            if (XMLString::equals(pArrayMember->getNodeName(), X("value")))
            {
               XmlRpcParam* pValue = NULL;
               for (DOMNode *pValueNode = pArrayMember->getFirstChild();
                  pValue == NULL && pValueNode != NULL;
                  pValueNode = pValueNode->getNextSibling())
               {
                  pValue = parseValueNode(pValueNode);
               }
               if (pValue != NULL)
               {
                  *pArray << pValue;
               }
            }
         }
         return pArray;
      }
      else if (nodeName == "struct")
      {
         XmlRpcStructParam* pStruct = new XmlRpcStructParam;
         for (DOMNode *pStructMember = pValueTypeNode->getFirstChild();
            pStructMember != NULL;
            pStructMember = pStructMember->getNextSibling())
         {
            if (XMLString::equals(pStructMember->getNodeName(), X("member")))
            {
               QString name;
               XmlRpcParam* pValue = NULL;
               for (DOMNode *pStructSub = pStructMember->getFirstChild();
                  pStructSub != NULL;
                  pStructSub = pStructSub->getNextSibling())
               {
                  if (XMLString::equals(pStructSub->getNodeName(), X("name")))
                  {
                     name = A(pStructSub->getTextContent());
                  }
                  else if (XMLString::equals(pStructSub->getNodeName(), X("value")))
                  {
                     for (DOMNode *pValueNode = pStructSub->getFirstChild();
                                  pValue == NULL && pValueNode != NULL;
                                  pValueNode = pValueNode->getNextSibling())
                     {
                        pValue = parseValueNode(pValueNode);
                     }
                  }
               }
               if (!name.isEmpty() && pValue != NULL)
               {
                  pStruct->insert(name, pValue);
               }
            }
         }
         return pStruct;
      }
      return NULL;
   }
};

XmlRpcMethodCall::XmlRpcMethodCall(DOMElement *pRoot)
{
   ENSURE(pRoot != NULL);
   ENSURE(XMLString::equals(pRoot->getNodeName(), X("methodCall")));
   for (DOMNode *pNode = pRoot->getFirstChild();
                pNode != NULL;
                pNode = pNode->getNextSibling())
   {
      if (XMLString::equals(pNode->getNodeName(), X("methodName")))
      {
         mMethodName = A(pNode->getFirstChild()->getNodeValue());
      }
      else if (XMLString::equals(pNode->getNodeName(), X("params")))
      {
         char id = 0;
         DOMNodeList* pParams = static_cast<DOMElement*>(pNode)->getElementsByTagName(X("param"));
         for (XMLSize_t i = 0; i < pParams->getLength(); i++)
         {
            DOMNode* pValue = static_cast<DOMElement*>(pParams->item(i))->getElementsByTagName(X("value"))->item(0);
            for (DOMNode *pValueTypeNode = pValue->getFirstChild();
                         pValueTypeNode != NULL;
                         pValueTypeNode = pValueTypeNode->getNextSibling())
            {
               XmlRpcParam* pParam = parseValueNode(pValueTypeNode);
               if (pParam != NULL)
               {
                  mParams.push_back(pParam);
               }
            }
         }
      }
   }
}

QString XmlRpcMethodCall::toXml() const
{
   XMLWriter xml("methodCall", NULL, false);
   xml.addText(mMethodName.toStdString(), xml.addElement("methodName"));
   xml.pushAddPoint(xml.addElement("params"));
   for (XmlRpcParams::const_iterator it = mParams.begin(); it != mParams.end(); ++it)
   {
      if (*it != NULL)
      {
         xml.pushAddPoint(xml.addElement("value", xml.addElement("param")));
         (*it)->toXml(xml);
         xml.popAddPoint();
      }
   }
   xml.popAddPoint();
   return QString(xml.writeToString().c_str());
}

XmlRpcMethodResponse::XmlRpcMethodResponse(DOMElement* pRoot)
{
   ENSURE(pRoot != NULL);
   ENSURE(XMLString::equals(pRoot->getNodeName(), X("methodResponse")));

   DOMNodeList* pParamsNodes = pRoot->getElementsByTagName(X("params"));
   ENSURE(pParamsNodes != NULL && pParamsNodes->getLength() == 1);
   DOMNodeList* pParamNodes = static_cast<DOMElement*>(pParamsNodes->item(0))->getElementsByTagName(X("param"));
   ENSURE(pParamNodes != NULL && pParamNodes->getLength() == 1);
   DOMNodeList* pValueNodes = static_cast<DOMElement*>(pParamNodes->item(0))->getElementsByTagName(X("value"));
   ENSURE(pValueNodes != NULL && pValueNodes->getLength() == 1);
   DOMNodeList* pTypeNodes = pValueNodes->item(0)->getChildNodes();
   ENSURE(pTypeNodes != NULL && pTypeNodes->getLength() > 0);
   mpReturnValue = parseValueNode(pTypeNodes->item(0));
}

XmlRpcMethodResponse::XmlRpcMethodResponse(const XmlRpcParam* pReturnValue) :
   mpReturnValue(pReturnValue)
{
}

XmlRpcMethodResponse::~XmlRpcMethodResponse()
{
   delete mpReturnValue;
}

QString XmlRpcMethodResponse::toXml() const
{
   XMLWriter xml("methodResponse", NULL, false);
   if ((mpReturnValue != NULL) && mpReturnValue->isValid())
   {
      xml.pushAddPoint(xml.addElement("params"));
      xml.pushAddPoint(xml.addElement("param"));
      xml.pushAddPoint(xml.addElement("value"));
      if (!mpReturnValue->toXml(xml))
      {
         return "";
      }
      xml.popAddPoint();
      xml.popAddPoint();
      xml.popAddPoint();
   }
   return QString(xml.writeToString().c_str());
}

const XmlRpcParam* XmlRpcMethodResponse::returnValue()
{
   return mpReturnValue;
}

QMap<unsigned int, QString> XmlRpcMethodFault::sFaults;

void XmlRpcMethodFault::populateFaults()
{
   /**
    *  The following are available error ranges.
    *
    *  0-99    : Reserved
    *  100-199 : Authentication and session errors
    *  200-299 : RPC method call related errors
    *  300-399 : Opticks core method call errors
    *  400-999 : Available for general use
    */
   if (sFaults.isEmpty())
   {
      sFaults[100] = QString("Unknown method");

      sFaults[200] = QString("Invalid parameters");
      sFaults[201] = QString("Bad Argument Value");
      sFaults[202] = QString("Unsupported MIME Type");

      sFaults[300] = QString("View not available");
      sFaults[301] = QString("Unable to Get Image");
      sFaults[302] = QString("Unable to import data set");
      sFaults[303] = QString("Object Not Available");
      sFaults[304] = QString("Object Already Exists");
      sFaults[305] = QString("Layer Not Available");
      sFaults[306] = QString("Layer Already Exists");
      sFaults[307] = QString("Unable to Create Requested Object");
      sFaults[308] = QString("Unable to Delete Layer");
      sFaults[309] = QString("Unable to Create View");
      sFaults[310] = QString("Unable to Link Views");
      sFaults[311] = QString("Unable to Unlink Views");
      sFaults[312] = QString("Unable to Delete Object");
      sFaults[313] = QString("Unable to Export Element");
   }
}

XmlRpcMethodFault::XmlRpcMethodFault(unsigned int faultCode, const QString &otherInformation) :
            mFaultCode(faultCode), mOtherInformation(otherInformation)
{
   populateFaults();
}

QString XmlRpcMethodFault::toXml() const
{
   XMLWriter xml("methodResponse", NULL, false);
   xml.pushAddPoint(xml.addElement("fault"));
   xml.pushAddPoint(xml.addElement("value"));
   xml.pushAddPoint(xml.addElement("struct"));
   xml.pushAddPoint(xml.addElement("member"));
   xml.addText("faultCode", xml.addElement("name"));
   xml.pushAddPoint(xml.addElement("value"));
   xml.addText(QString::number(mFaultCode).toStdString(), xml.addElement("int"));
   xml.popAddPoint(); // value
   xml.popAddPoint(); // member
   
   xml.pushAddPoint(xml.addElement("member"));
   xml.addText("faultString", xml.addElement("name"));
   xml.pushAddPoint(xml.addElement("value"));
   QMap<unsigned int, QString>::const_iterator fsit = sFaults.find(mFaultCode);
   QString faultString = (fsit == sFaults.end()) ? "Unknown Error" : fsit.value();
   if (!mOtherInformation.isEmpty())
   {
      faultString += ": " + mOtherInformation;
   }
   xml.addText(faultString.toStdString(), xml.addElement("string"));
   xml.popAddPoint(); // value
   xml.popAddPoint(); // member
   
   xml.popAddPoint(); // struct
   xml.popAddPoint(); // value
   xml.popAddPoint(); // fault
   return QString(xml.writeToString().c_str());
}
