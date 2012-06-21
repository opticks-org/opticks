/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "WizardNodeImp.h"
#include "DateTimeImp.h"
#include "DynamicObjectAdapter.h"
#include "FilenameImp.h"
#include "StringUtilities.h"
#include "WizardItemImp.h"
#include "xmlreader.h"
#include "xmlwriter.h"

#include <sstream>
using namespace std;
XERCES_CPP_NAMESPACE_USE

WizardNodeImp::WizardNodeImp(WizardItem* pItem, string name, string type, string description) : 
   mpItem(pItem),
   mName(name),
   mType(type),
   mDescription(description),
   mOriginalType(type),
   mpShallowCopyValue(NULL)
{
   if (type.empty() == false)
   {
      mValidTypes.push_back(type);
   }
}

WizardNodeImp::~WizardNodeImp()
{
   notify(SIGNAL_NAME(Subject, Deleted));    // Call before clearing the connected nodes so that
                                             // connected objects can have access to them

   clearConnectedNodes();
   deleteValue();
}

bool WizardNodeImp::copyNode(const WizardNodeImp* pNode)
{
   if ((pNode == NULL) || (pNode == this))
   {
      return false;
   }

   mName = pNode->mName;
   mType = pNode->mType;
   mDescription = pNode->mDescription;
   mOriginalType = pNode->mOriginalType;
   setValidTypes(pNode->mValidTypes);
   setValue(pNode->mpShallowCopyValue);

   return true;
}

WizardItem* WizardNodeImp::getItem() const
{
   return mpItem;
}

void WizardNodeImp::setName(const string& nodeName)
{
   if (nodeName != mName)
   {
      mName = nodeName;
      notify(SIGNAL_NAME(WizardNodeImp, Renamed), boost::any(mName));
   }
}

const string& WizardNodeImp::getName() const
{
   return mName;
}

void WizardNodeImp::setType(const string& nodeType)
{
   if (nodeType != mType)
   {
      if (mpShallowCopyValue != NULL)
      {
         deleteValue();
      }

      mType = nodeType;
      notify(SIGNAL_NAME(WizardNodeImp, TypeChanged), boost::any(mType));
   }
}

const string& WizardNodeImp::getType() const
{
   return mType;
}

void WizardNodeImp::setDescription(const string& nodeDescription)
{
   if (nodeDescription != mDescription)
   {
      mDescription = nodeDescription;
      notify(SIGNAL_NAME(WizardNodeImp, DescriptionChanged), boost::any(mDescription));
   }
}

const string& WizardNodeImp::getDescription() const
{
   return mDescription;
}

void WizardNodeImp::setOriginalType(const string& originalType)
{
   mOriginalType = originalType;
}

const string& WizardNodeImp::getOriginalType() const
{
   return mOriginalType;
}

void WizardNodeImp::setValidTypes(const vector<string>& validTypes)
{
   mValidTypes.clear();

   for (vector<string>::const_iterator iter = validTypes.begin(); iter != validTypes.end(); ++iter)
   {
      string type = (*iter).c_str();
      mValidTypes.push_back(type);
   }

   if (mValidTypes.empty() == true)
   {
      mValidTypes.push_back(mOriginalType);
   }
}

const vector<string>& WizardNodeImp::getValidTypes() const
{
   return mValidTypes;
}

void WizardNodeImp::setValue(void* pValue)
{
   if (pValue != mpShallowCopyValue)
   {
      if (mpShallowCopyValue != NULL)
      {
         deleteValue();
      }

      if (pValue != NULL)
      {
         mDeepCopyValue = DataVariant(mType, pValue, false);
         if (mDeepCopyValue.isValid())
         {
            mpShallowCopyValue = mDeepCopyValue.getPointerToValueAsVoid();
         }
         else
         {
            mpShallowCopyValue = pValue;
         }
      }

      notify(SIGNAL_NAME(WizardNodeImp, ValueChanged), boost::any(mpShallowCopyValue));
   }
}

void* WizardNodeImp::getValue() const
{
   return mpShallowCopyValue;
}

void WizardNodeImp::deleteValue()
{
   if (mDeepCopyValue.isValid())
   {
      mDeepCopyValue = DataVariant();
   }
   mpShallowCopyValue = NULL;
}

bool WizardNodeImp::addConnectedNode(WizardNode* pNode)
{
   if (pNode == NULL)
   {
      return false;
   }

   if (isNodeConnected(pNode) == true)
   {
      return false;
   }

   string type = pNode->getType();
   if (type != mType)
   {
      return false;
   }

   mConnectedNodes.push_back(pNode);
   notify(SIGNAL_NAME(WizardNodeImp, NodeConnected), boost::any(pNode));

   WizardNodeImp* pNodeImp = static_cast<WizardNodeImp*>(pNode);
   if (pNodeImp != NULL)
   {
      pNodeImp->addConnectedNode(this);
   }

   return true;
}

int WizardNodeImp::getNumConnectedNodes() const
{
   int iCount = static_cast<int>(mConnectedNodes.size());
   return iCount;
}

const vector<WizardNode*>& WizardNodeImp::getConnectedNodes() const
{
   return mConnectedNodes;
}

bool WizardNodeImp::removeConnectedNode(WizardNode* pNode)
{
   if (pNode == NULL)
   {
      return false;
   }

   vector<WizardNode*>::iterator iter = mConnectedNodes.begin();
   while (iter != mConnectedNodes.end())
   {
      WizardNode* pCurrentNode = *iter;
      if (pCurrentNode == pNode)
      {
         mConnectedNodes.erase(iter);
         notify(SIGNAL_NAME(WizardNodeImp, NodeDisconnected), boost::any(pNode));

         WizardNodeImp* pNodeImp = static_cast<WizardNodeImp*>(pNode);
         if (pNodeImp != NULL)
         {
            pNodeImp->removeConnectedNode(this);
         }

         return true;
      }

      iter++;
   }

   return false;
}

void WizardNodeImp::clearConnectedNodes()
{
   while (mConnectedNodes.empty() == false)
   {
      WizardNode* pNode = mConnectedNodes.front();
      if (pNode != NULL)
      {
         bool bSuccess = removeConnectedNode(pNode);
         if (bSuccess == false)
         {
            break;
         }
      }
   }
}

bool WizardNodeImp::isNodeConnected(WizardNode* pNode) const
{
   if (pNode == NULL)
   {
      return false;
   }

   vector<WizardNode*>::const_iterator iter = mConnectedNodes.begin();
   while (iter != mConnectedNodes.end())
   {
      WizardNode* pCurrentNode = *iter;
      if (pCurrentNode == pNode)
      {
         return true;
      }

      iter++;
   }

   return false;
}

bool WizardNodeImp::toXml(XMLWriter* pXml) const
{
   // This will get all of the infomation for the wizard node
   // except the description.
   // version will always be the latest since conversion should happen at deserialization time
   pXml->addAttr("version", "4");
   pXml->addAttr("name", mName);
   pXml->addAttr("originalType", mOriginalType);
   pXml->addAttr("type", mType);

   if (mDescription.empty() == false)
   {
      pXml->addText(mDescription, pXml->addElement("description"));
   }

   for (vector<string>::const_iterator iter = mValidTypes.begin(); iter != mValidTypes.end(); ++iter)
   {
      pXml->addText(*iter, pXml->addElement("validType"));
   }

   if (mpShallowCopyValue != NULL)
   {
      DataVariant var(mType, mpShallowCopyValue, false);
      if (!var.isValid())
      {
         return false;
      }
      DataVariant::Status status;
      string value = var.toXmlString(&status);
      if (status == DataVariant::FAILURE)
      {
         return false;
      }
      pXml->addText(value, pXml->addElement("value"));
   }

   return true;
}

bool WizardNodeImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   DOMElement* elmnt(static_cast<DOMElement*>(pDocument));

   string itemVersion = A(elmnt->getAttribute(X("version")));
   if (itemVersion == "4") // the current version
   {
      // do nothing special
   }
   else if (itemVersion == "Wizard Node Version 3.0" || true)
   {
//#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : The || true preserves a loophole in previous " \
//"parsing code where anything could be put in this field by another app generating a wizard file. We want to " \
//"preserve this behavior in 4.3.0 but in 4.4.0 add an else {return false;} and remove the || true (tclarke)")
      // set compatible behavior for older wizard files
      WizardItemImp* pItemImp = dynamic_cast<WizardItemImp*>(mpItem);
      VERIFY(pItemImp);
      if (pItemImp->getType() == "Value")
      {
         pItemImp->setBatchMode(true);
      }
   }
   mName = A(elmnt->getAttribute(X("name")));
   mOriginalType = A(elmnt->getAttribute(X("originalType")));
   mType = A(elmnt->getAttribute(X("type")));
   mDescription.clear();
   deleteValue();
   mValidTypes.clear();

   for (DOMNode* pChld = pDocument->getFirstChild(); pChld != NULL; pChld = pChld->getNextSibling())
   {
      if (XMLString::equals(pChld->getNodeName(), X("description")))
      {
         mDescription = A(pChld->getTextContent());
      }
      else if (XMLString::equals(pChld->getNodeName(), X("validType")))
      {
         string vt(A(pChld->getTextContent()));
         mValidTypes.push_back(vt);
      }
      else if (XMLString::equals(pChld->getNodeName(), X("value")))
      {
         string value(A(pChld->getTextContent()));

         DataVariant parsedValue(mType, NULL, false);
         if (!parsedValue.isValid())
         {
            return false;
         }
         DataVariant::Status status = parsedValue.fromXmlString(mType, value);
         if (status != DataVariant::SUCCESS)
         {
            return false;
         }
         mDeepCopyValue = parsedValue;
         mpShallowCopyValue = mDeepCopyValue.getPointerToValueAsVoid();
      }
   }

   notify(SIGNAL_NAME(Subject, Modified));
   return true;
}

const string& WizardNodeImp::getObjectType() const
{
   static string sType("WizardNodeImp");
   return sType;
}

bool WizardNodeImp::isKindOf(const string& className) const
{
   if ((className == getType()) || (className == "WizardNode"))
   {
      return true;
   }

   return SubjectImp::isKindOf(className);
}
