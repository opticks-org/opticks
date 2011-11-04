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

#include <memory>

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

   for (DOMNode* pChild = pOwner->getFirstChild(); pChild != NULL; pChild = pChild->getNextSibling())
   {
      if (XMLString::equals(pChild->getNodeName(), X(pName)))
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

   for (DOMNode* pChild = pOwner->getFirstChild(); pChild != NULL; pChild = pChild->getNextSibling())
   {
      if (XMLString::equals(pChild->getNodeName(), X(pName)))
      {
         pOwner->removeChild(pChild);
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
   DOMLSSerializer* pSerializer = NULL;
   DOMLSOutput* pOutput = NULL;

   try
   {
      pSerializer = mpImpl->createLSSerializer();
      DOMConfiguration* pConfig = pSerializer->getDomConfig();

      pOutput = mpImpl->createLSOutput();
      pOutput->setEncoding(XMLUni::fgUTF8EncodingString);

      std::auto_ptr<MemBufFormatTarget> pByteStream(new MemBufFormatTarget);
      pOutput->setByteStream(pByteStream.get());

      if (pConfig->canSetParameter(XMLUni::fgDOMWRTDiscardDefaultContent, false))
      {
         pConfig->setParameter(XMLUni::fgDOMWRTDiscardDefaultContent, false);
      }

      if (pConfig->canSetParameter(XMLUni::fgDOMNamespaceDeclarations, true))
      {
         pConfig->setParameter(XMLUni::fgDOMNamespaceDeclarations, true);
      }

      if (pConfig->canSetParameter(XMLUni::fgDOMWRTFormatPrettyPrint, true))
      {
         pConfig->setParameter(XMLUni::fgDOMWRTFormatPrettyPrint, true);
      }

      if (pConfig->canSetParameter(XMLUni::fgDOMXMLDeclaration, true))
      {
         pConfig->setParameter(XMLUni::fgDOMXMLDeclaration, true);
      }

      std::string buf;
      if (pSerializer->write(mpDoc, pOutput) && pByteStream->getRawBuffer() != NULL)
      {
         buf = std::string(reinterpret_cast<const char*>(pByteStream->getRawBuffer()));
      }

      pOutput->release();
      pSerializer->release();
      return buf;
   }
   catch (const XMLException& exc)
   {
      if (pOutput != NULL)
      {
         pOutput->release();
      }

      if (pSerializer != NULL)
      {
         pSerializer->release();
      }

      logException(&exc);
      throw XmlBase::XmlException(A(exc.getMessage()));
   }
   catch (const DOMException& exc)
   {
      if (pOutput != NULL)
      {
         pOutput->release();
      }

      if (pSerializer != NULL)
      {
         pSerializer->release();
      }

      logException(&exc);
      throw XmlBase::XmlException(A(exc.msg));
   }
   catch (...)
   {
      if (pOutput != NULL)
      {
         pOutput->release();
      }

      if (pSerializer != NULL)
      {
         pSerializer->release();
      }

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
