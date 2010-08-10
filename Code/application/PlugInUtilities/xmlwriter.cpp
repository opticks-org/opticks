/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "Font.h"
#include "XercesIncludes.h"
#include "xmlwriter.h"

XERCES_CPP_NAMESPACE_USE

XMLWriter::XMLWriter(const char* pRootElementName, MessageLog* pLog, bool useNamespace) :
   XmlBase(pLog),
   mSingleChildInstance(false)
{
   if (useNamespace)
   {
      mpWithNamespace = XMLString::transcode(sNamespaceId);
   }
   else
   {
      mpWithNamespace = NULL;
   }
   mpAddPoint.push(NULL);
   try
   {
      mpImpl = DOMImplementationRegistry::getDOMImplementation(X("LS"));
      mpDoc = mpImpl->createDocument(mpWithNamespace, X(pRootElementName), NULL);
   }
   catch (const XMLException& exc)
   {
      logException(&exc);
      throw XmlBase::XmlException(A(exc.getMessage()));
   }
   catch (const DOMException& exc)
   {
      logException(&exc);
      throw XmlBase::XmlException(A(exc.msg));
   }
   catch (...)
   {
      logSimpleMessage(std::string("XMLWriter unexpected exception"));
      throw XmlBase::XmlException("XMLWriter unexpected exception");
   }
}

XMLWriter::XMLWriter(const std::string& rootElementName, const std::string& xmlNamespace, MessageLog* pLog,
                     bool useNamespace) :
   XmlBase(pLog),
   mSingleChildInstance(false)
{
   if (useNamespace)
   {
      mpWithNamespace = XMLString::transcode(xmlNamespace.c_str());
   }
   else
   {
      mpWithNamespace = NULL;
   }
   mpAddPoint.push(NULL);
   try
   {
      mpImpl = DOMImplementationRegistry::getDOMImplementation(X("LS"));
      mpDoc = mpImpl->createDocument(mpWithNamespace, X(rootElementName.c_str()), NULL);
   }
   catch (const XMLException& exc)
   {
      logException(&exc);
      throw XmlBase::XmlException(A(exc.getMessage()));
   }
   catch (const DOMException& exc)
   {
      logException(&exc);
      throw XmlBase::XmlException(A(exc.msg));
   }
   catch (...)
   {
      logSimpleMessage(std::string("XMLWriter unexpected exception"));
      throw XmlBase::XmlException("XMLWriter unexpected exception");
   }
}

XMLWriter::~XMLWriter()
{
   mpDoc->release();
   if (mpWithNamespace != NULL)
   {
      XMLString::release(&mpWithNamespace);
   }
}

bool XMLWriter::addAttr(const char* pName, const char* pValue, DOMElement* pOwner)
{
   try
   {
      if (pOwner == NULL && (mpAddPoint.top() == NULL || mpAddPoint.top()->getNodeType() != DOMNode::ELEMENT_NODE))
      {
         pOwner = mpDoc->getDocumentElement();
      }
      else if (pOwner == NULL)
      {
         pOwner = static_cast<DOMElement*>(mpAddPoint.top());
      }

      pOwner->setAttributeNS(mpWithNamespace, X(pName), X(pValue));
      return true;
   }
   catch (const XMLException& exc)
   {
      logException(&exc);
      throw XmlBase::XmlException(A(exc.getMessage()));
   }
   catch (const DOMException& exc)
   {
      logException(&exc);
      throw XmlBase::XmlException(A(exc.msg));
   }
   catch (...)
   {
      logSimpleMessage(std::string("XMLWriter unexpected exception"));
      throw XmlBase::XmlException("XMLWriter unexpected exception");
   }

   return false;
}

DOMElement* XMLWriter::addElement(const char* pName, DOMNode* pOwner)
{
   try
   {
      if (mSingleChildInstance)
      {
         removeElement(pName);
      }

      DOMElement* pElmnt(NULL);
      if (mpWithNamespace != NULL)
      {
         pElmnt = mpDoc->createElementNS(mpWithNamespace, X(pName));
      }
      else
      {
         pElmnt = mpDoc->createElement(X(pName));
      }

      if (pOwner == NULL && (mpAddPoint.top() == NULL || mpAddPoint.top()->getNodeType() != DOMNode::ELEMENT_NODE))
      {
         pOwner = mpDoc->getDocumentElement();
      }
      else if (pOwner == NULL)
      {
         pOwner = static_cast<DOMElement*>(mpAddPoint.top());
      }

      pOwner->appendChild(pElmnt);
      return pElmnt;
   }
   catch (const XMLException& exc)
   {
      logException(&exc);
      throw XmlBase::XmlException(A(exc.getMessage()));
   }
   catch (const DOMException& exc)
   {
      logException(&exc);
      throw XmlBase::XmlException(A(exc.msg));
   }
   catch (...)
   {
      logSimpleMessage(std::string("XMLWriter unexpected exception"));
      throw XmlBase::XmlException("XMLWriter unexpected exception");
   }

   return NULL;
}

XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* XMLWriter::addFontElement(const char* pName, const Font& font,
                              XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* pOwner)
{
   DOMElement* pElement = addElement(pName, pOwner);
   if (pElement != NULL)
   {
      pushAddPoint(pElement);
      addAttr("face", font.getFace());
      addAttr("point_size", font.getPointSize());
      addAttr("bold", font.getBold());
      addAttr("italic", font.getItalic());
      addAttr("underline", font.getUnderline());
      popAddPoint();
   }
   return pElement;
}

DOMText* XMLWriter::addText(const char* pValue, DOMNode* pOwner)
{
   try
   {
      DOMText* t(mpDoc->createTextNode(X(pValue)));
      if (pOwner == NULL && (mpAddPoint.top() == NULL || mpAddPoint.top()->getNodeType() != DOMNode::ELEMENT_NODE))
      {
         pOwner = mpDoc->getDocumentElement();
      }
      else if (pOwner == NULL)
      {
         pOwner = static_cast<DOMElement*>(mpAddPoint.top());
      }

      pOwner->appendChild(t);
      return t;
   }
   catch (const XMLException& exc)
   {
      logException(&exc);
      throw XmlBase::XmlException(A(exc.getMessage()));
   }
   catch (const DOMException& exc)
   {
      logException(&exc);
      throw XmlBase::XmlException(A(exc.msg));
   }
   catch (...)
   {
      logSimpleMessage(std::string("XMLWriter unexpected exception"));
      throw XmlBase::XmlException("XMLWriter unexpected exception");
   }

   return NULL;
}

bool XMLWriter::elementExists(const char* pName, DOMNode* pOwner)
{
   if (pOwner == NULL && (mpAddPoint.top() == NULL || mpAddPoint.top()->getNodeType() != DOMNode::ELEMENT_NODE))
   {
      pOwner = mpDoc->getDocumentElement();
   }
   else if (pOwner == NULL)
   {
      pOwner = static_cast<DOMElement*>(mpAddPoint.top());
   }

   for (DOMNode* chld = pOwner->getFirstChild(); chld != NULL; chld = chld->getNextSibling())
   {
      if (XMLString::equals(chld->getNodeName(), X(pName)))
      {
         return true;
      }
   }

   return false;
}

void XMLWriter::removeElement(const char* pName, DOMNode* pOwner)
{
   if (pOwner == NULL && (mpAddPoint.top() == NULL || mpAddPoint.top()->getNodeType() != DOMNode::ELEMENT_NODE))
   {
      pOwner = mpDoc->getDocumentElement();
   }
   else if (pOwner == NULL)
   {
      pOwner = static_cast<DOMElement*>(mpAddPoint.top());
   }

   for (DOMNode* chld = pOwner->getFirstChild(); chld != NULL; chld = chld->getNextSibling())
   {
      if (XMLString::equals(chld->getNodeName(), X(pName)))
      {
         pOwner->removeChild(chld);
      }
   }
}

void XMLWriter::removeChild(DOMNode* pChild, DOMNode* pOwner)
{
   if (pOwner == NULL && (mpAddPoint.top() == NULL || mpAddPoint.top()->getNodeType() != DOMNode::ELEMENT_NODE))
   {
      pOwner = mpDoc->getDocumentElement();
   }
   else if (pOwner == NULL)
   {
      pOwner = static_cast<DOMElement*>(mpAddPoint.top());
   }

   pOwner->removeChild(pChild);
}

void XMLWriter::writeToFile(FILE* pFp)
{
   if (pFp != NULL)
   {
      std::string buf(writeToString());
      fwrite(buf.c_str(), sizeof(char), buf.length(), pFp);
   }
}

std::string XMLWriter::writeToString()
{
   try
   {
      DOMWriter* serializer(mpImpl->createDOMWriter());
      if (serializer->canSetFeature(XMLUni::fgDOMWRTDiscardDefaultContent, false))
      {
         serializer->setFeature(XMLUni::fgDOMWRTDiscardDefaultContent, false);
      }

      if (serializer->canSetFeature(XMLUni::fgDOMNamespaceDeclarations, true))
      {
         serializer->setFeature(XMLUni::fgDOMNamespaceDeclarations, true);
      }

      if (serializer->canSetFeature(XMLUni::fgDOMWRTFormatPrettyPrint, true))
      {
         serializer->setFeature(XMLUni::fgDOMWRTFormatPrettyPrint, true);
      }

      if (serializer->canSetFeature(XMLUni::fgDOMXMLDeclaration, true))
      {
         serializer->setFeature(XMLUni::fgDOMXMLDeclaration, true);
      }

      MemBufFormatTarget target;
      serializer->writeNode(&target, *mpDoc);

      if (sizeof(char) != sizeof(XMLByte))
      {
         throw XmlBase::XmlException("XMLWriter: XMLByte is not a char");
      }

      std::string buf(reinterpret_cast<const char*>(target.getRawBuffer()), static_cast<int>(target.getLen()));
      serializer->release();

      return buf;
   }
   catch (const XMLException& exc)
   {
      logException(&exc);
      throw XmlBase::XmlException(A(exc.getMessage()));
   }
   catch (const DOMException& exc)
   {
      logException(&exc);
      throw XmlBase::XmlException(A(exc.msg));
   }
   catch (...)
   {
      logSimpleMessage(std::string("XMLWriter unexpected exception"));
      throw XmlBase::XmlException("XMLWriter unexpected exception");
   }
}

void XMLWriter::pushAddPoint(DOMNode* pNode)
{
   if (pNode == NULL)
   {
      mpAddPoint.push(mpDoc->getDocumentElement());
   }
   else
   {
      mpAddPoint.push(pNode);
   }
}

DOMNode* XMLWriter::popAddPoint()
{
   if (mpAddPoint.size() == 1)
   {
      return NULL;
   }

   DOMNode* top(mpAddPoint.top());
   mpAddPoint.pop();
   return top;
}

DOMNode* XMLWriter::peekAddPoint()
{
   return mpAddPoint.top();
}

template<> bool XMLWriter::addAttr(const char* pName, std::string value)
{
   return addAttr(pName, value, NULL);
}

template<> bool XMLWriter::addAttr(const char* pName, char* pValue)
{
   return addAttr(pName, pValue, NULL);
}

template<> bool XMLWriter::addAttr(const char* pName, const char* pValue)
{
   return addAttr(pName, pValue, NULL);
}
