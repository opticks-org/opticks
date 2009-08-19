/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "Font.h"
#include "MessageLogResource.h"
#include "StringUtilities.h"
#include "UtilityServices.h"
#include "xmlreader.h"

#include <sstream>
#include <stdio.h>

using namespace std;
XERCES_CPP_NAMESPACE_USE

class XmlReaderErrorHandler : public DOMErrorHandler
{
public:
   XmlReaderErrorHandler(XmlBase* pLogger) :
      mpLogger(pLogger) {}

   bool handleError(const DOMError& domError)
   {
      if (mpLogger != NULL)
      {
         mpLogger->logError(domError);
      }
      if (domError.getSeverity() == DOMError::DOM_SEVERITY_FATAL_ERROR)
      {
         return false;
      }
      return true;
   }

private:
   XmlBase* mpLogger;
};

XmlReader::XmlReader(MessageLog* pLog, bool bValidate) :
   XmlBase(pLog),
   mpDoc(NULL),
   mpResult(NULL)
{
   mpImpl = DOMImplementationRegistry::getDOMImplementation(XStr("XPath2 3.0 LS").unicodeForm());
   ENSURE(mpImpl != NULL);

   mpParser = mpImpl->createDOMBuilder(DOMImplementationLS::MODE_SYNCHRONOUS, 0);
   mpParser->setFeature(X("namespaces"), true);
   mpParser->setFeature(X("validate-if-schema"), true);
   mpParser->setFeature(XMLUni::fgXercesUserAdoptsDOMDocument, true);

   if (bValidate == true)
   {
      // Get the schema location
      const Filename* pSupportFilesPath = ConfigurationSettings::getSettingSupportFilesPath();
      if (pSupportFilesPath != NULL)
      {
         mXmlSchemaLocation = pSupportFilesPath->getFullPathAndName() + SLASH + "Xml" + SLASH;
      }
   }
}

XmlReader::~XmlReader()
{   
   if (mpDoc != NULL)
   {
      mpDoc->release();
   }
   if (mpParser != NULL)
   {
      mpParser->release();
   } 
}

XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* XmlReader::parse(const Filename* pFn, string endTag)
{
   if (pFn == NULL)
   {
      return NULL;
   }
   return parse(pFn->getFullPathAndName(), endTag);
}

XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* XmlReader::parse(string fn, string endTag)
{
   if (mpDoc != NULL)
   {
      mpDoc->release();
      mpDoc = NULL;
   }
   DOMErrorHandler* pOldError = mpParser->getErrorHandler();
   try
   {
      string uri = XmlBase::PathToURL(fn);

      if (mXmlSchemaLocation.empty())
      {
         mpParser->setFeature(X("http://apache.org/xml/features/validation/schema"), false);
         mpParser->setFeature(X("validation"), false);
      }
      else
      {
         string esl("https://comet.balldayton.com/standards/namespaces/2005/v1/comet.xsd ");
         for (unsigned int version = XmlBase::VERSION; version > 0; version--)
         {
            stringstream fname;
            fname << mXmlSchemaLocation << "opticks-" << version << ".xsd";
            FILE* pTmp = fopen(fname.str().c_str(), "r");
            if (pTmp != NULL)
            {
               esl += XmlBase::PathToURL(fname.str()) + " ";
               fclose(pTmp);
               break;
            }
         }
         mpParser->setProperty(XMLUni::fgXercesSchemaExternalSchemaLocation, const_cast<XMLCh*>(X(esl.c_str())));
         mpParser->setFeature(X("http://apache.org/xml/features/validation/schema"), true);
         mpParser->setFeature(X("validation"), true);
         mpParser->setFeature(X("http://apache.org/xml/features/validation-error-as-fatal"), true);
      }

      if (!endTag.empty())
      {
         logSimpleMessage("Partial parse is not available...performing complete parse.");
      }
      XmlReaderErrorHandler errors(this);
      mpParser->setErrorHandler(&errors);
      mpDoc = mpParser->parseURI(uri.c_str());
   }
   catch (const XMLException& exc)
   {
      logException(&exc);
   }
   catch (const DOMException& exc)
   {
      logException(dynamic_cast<const XMLException*>(&exc));
   }
   catch (const XmlBase::XmlException& exc)
   {
      logSimpleMessage(exc.str());
   }
   catch (...)
   {
      logSimpleMessage("XmlReader unexpected parse error");
   }

   mpParser->setErrorHandler(pOldError);
   return mpDoc;
}

XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* XmlReader::parseString(const string& str)
{
   if (mpDoc != NULL)
   {
      mpDoc->release();
      mpDoc = NULL;
   }
   try
   {
      if (!mXmlSchemaLocation.empty())
      {
         logSimpleMessage("String parsing does not support schema validation.");
         return NULL;
      }
      mpParser->setFeature(X("http://apache.org/xml/features/validation/schema"), false);
      mpParser->setFeature(X("validation"), false);
      MemBufInputSource isrc(reinterpret_cast<const XMLByte*>(str.c_str()), str.size(), X(sNamespaceId));
      Wrapper4InputSource wrapper(&isrc, false);
      mpDoc = mpParser->parse(wrapper);
   }
   catch (const XMLException& exc)
   {
      logException(&exc);
   }
   catch (const DOMException& exc)
   {
      logException(dynamic_cast<const XMLException*>(&exc));
   }
   catch (const XmlBase::XmlException& exc)
   {
      logSimpleMessage(exc.str());
   }
   catch (...)
   {
      logSimpleMessage("XmlReader unexpected parse error");
   }

   return mpDoc;
}

XPath2Result* XmlReader::query(const string& expression, unsigned short type, bool reuse)
{
   if (mpDoc == NULL)
   {
      return NULL;
   }
   DOMXPathNSResolver* pResolver =
      const_cast<DOMXPathNSResolver*>(mpDoc->createNSResolver(mpDoc->getDocumentElement()));
   static_cast<XQillaNSResolver*>(pResolver)->addNamespaceBinding(X("opticks"), X(sNamespaceId));
   static_cast<XQillaNSResolver*>(pResolver)->addNamespaceBinding(X("xml"), X("http://www.w3.org/XML/1998/namespace"));
   static_cast<XQillaNSResolver*>(pResolver)->addNamespaceBinding(X("xs"), X("http://www.w3.org/2001/XMLSchema"));
   static_cast<XQillaNSResolver*>(pResolver)->addNamespaceBinding(X("xsi"),
      X("http://www.w3.org/2001/XMLSchema-instance"));
   static_cast<XQillaNSResolver*>(pResolver)->addNamespaceBinding(X("fn"), X("http://www.w3.org/2005/xpath-functions"));
   static_cast<XQillaNSResolver*>(pResolver)->addNamespaceBinding(X("err"), X("http://www.w3.org/2005/xqt-errors"));
   static_cast<XQillaNSResolver*>(pResolver)->addNamespaceBinding(X("local"),
      X("http://www.w3.org/2005/xquery-local-functions"));
   const DOMXPathExpression* pExpr = mpDoc->createExpression(X(expression.c_str()), pResolver);
   if (pExpr == NULL)
   {
      return NULL;
   }
   XPath2Result* pResult = reinterpret_cast<XPath2Result*>(pExpr->evaluate(mpDoc, type, reuse ? mpResult : NULL));
   if (reuse)
   {
      mpResult = pResult;
   }
   return pResult;
}

XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* findChildNode(XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* pParent,
                                                      const char* pName)
{
   if (pParent == NULL || pName == NULL)
   {
      return NULL;
   }

   XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* pChild = NULL;
   string name(pName);
   vector<string> components = StringUtilities::split(name, '/');
   for (vector<string>::iterator pComponent = components.begin(); pComponent != components.end(); ++pComponent)
   {
      for (pChild = pParent->getFirstChild(); pChild != NULL; pChild = pChild->getNextSibling())
      {
         string nodeName = A(pChild->getNodeName());
         if (XERCES_CPP_NAMESPACE_QUALIFIER XMLString::equals(pChild->getNodeName(), X(pComponent->c_str())))
         {
            break;
         }
      }
      if (pChild == NULL)
      {
         return NULL; // component not found
      }
      pParent = pChild;
   }

   return pChild;
}

XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* readFontElement(const char* pName,
                                                           XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* pParent,
                                                           Font& font)
{
   DOMElement* pElement = dynamic_cast<DOMElement*>(findChildNode(pParent, pName));
   if (pElement)
   {
      font.setFace(A(pElement->getAttribute(X("face"))));
      font.setPointSize(StringUtilities::fromXmlString<int>(A(pElement->getAttribute(X("point_size")))));
      font.setBold(StringUtilities::fromXmlString<bool>(A(pElement->getAttribute(X("bold")))));
      font.setItalic(StringUtilities::fromXmlString<bool>(A(pElement->getAttribute(X("italic")))));
      font.setUnderline(StringUtilities::fromXmlString<bool>(A(pElement->getAttribute(X("underline")))));
   }
   return pElement;
}

string findAttribute(XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* pParent, const char* pName)
{
   DOMElement* pElement = dynamic_cast<XERCES_CPP_NAMESPACE_QUALIFIER DOMElement*>(pParent);
   if (pElement != NULL)
   {
      string name(pName);
      string::size_type pos = name.find_last_of("/");
      if (pos != string::npos)
      {
         pElement = dynamic_cast<DOMElement*>(findChildNode(pParent, name.substr(0, pos).c_str()));
         if (pElement != NULL)
         {
            return A(pElement->getAttribute(X(name.substr(pos + 1, name.length() - pos).c_str())));
         }
      }
   }
   return string();
}
