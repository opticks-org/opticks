/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "Endian.h"
#include "XercesIncludes.h"
#include "xmlbase.h"

#include <cctype>
#include <algorithm>

XERCES_CPP_NAMESPACE_USE

const char* XmlBase::sNamespaceId = "https://comet.balldayton.com/standards/namespaces/2005/v1/comet.xsd";

const unsigned int XmlBase::VERSION = 3;

struct TransMapMember
{
   const char* mpC;
   const char* mpH;
};

static TransMapMember sTransMap[] =
{
   {"%", "%25"},
   {" ", "%20"},
   {"\"", "%22"},
   {"<", "%3c"},
   {">", "%3e"},
   {"#", "%23"},
   {"{", "%7b"},
   {"}", "%7d"},
   {"|", "%7c"},
   {"\\", "%5c"},
   {"^", "%5e"},
   {"~", "%7e"},
   {"[", "%5b"},
   {"]", "%5d"},
   {"`", "%60"},
   {"(", "%28"},
   {")", "%29"},
   {NULL, NULL}
};

std::string XmlBase::URLtoPath(const XMLCh* pUrl)
{
   try
   {
      XMLURL urlRepr(pUrl);
      std::string path(A(urlRepr.getPath()));

      // normalize
      if (path.size() >= 3 && path[0] == '/' && path[2] == ':')
      {
         path = path.substr(1);
      }
      else if (path.size() >= 3 && path.substr(0, 3) == "/./")
      {
         path = path.substr(3);
      }

      int mapPos(0);
      while (sTransMap[mapPos].mpC != NULL)
      {
         std::string::size_type pos(0);
         while ((pos = path.find(sTransMap[mapPos].mpH, pos)) != -1)
         {
            // all %xx reprs are 3 characters so we
            // replace them with the single character
            path.replace(pos, 3, sTransMap[mapPos].mpC);
            pos++;
         }
         mapPos++;
      }
      return path;
   }
   catch (...)
   {
      // catch any exceptions that xerces might throw
   }
   // if there's an exception give something back
   return std::string("");
}

std::string XmlBase::PathToURL(std::string path)
{
   if (path.empty())
   {
      return "file:///";
   }
   try
   {
      // check for absolute vs. relative url
      bool isPathAbsolute = false;
      if (path[0] == '/')
      {
         isPathAbsolute = true;
      }
      else if ((path.size() > 1) && (path[1] == ':') && (std::isalpha(path[0])))
      {
         isPathAbsolute = true;
         path.insert(0, "/");
         std::replace(path.begin(), path.end(), '\\', '/');
      }
      std::string pth(path);
      int mapPos(0);
      while (sTransMap[mapPos].mpC != NULL)
      {
         std::string::size_type pos(0);
         while ((pos = pth.find(sTransMap[mapPos].mpC, pos)) != -1)
         {
            // all replacements are 1 character so we
            // replace them with the 3 chracter %xx repr
            pth.replace(pos, 1, sTransMap[mapPos].mpH);
            pos++;
         }
         mapPos++;
      }
      // check for absolute vs. relative url
      if (isPathAbsolute)
      {
         pth.insert(0, "file://");
      }
      else
      {
         pth.insert(0, "file:///./");
      }
      return pth;
   }
   catch (...)
   {
      // catch any exceptions that xerces might throw
   }
   // if there's an exception give something back
   return std::string("");
}

XmlBase::XmlBase(MessageLog* pLog) :
   mpLog(pLog)
{}

XmlBase::~XmlBase()
{}

void XmlBase::logException(const XMLException* pExc)
{
   if (mpLog != NULL)
   {
      Message* msg(mpLog->createMessage("XmlReader DOMBuilder error", "app", "F4E4B705-352C-48bc-96AA-2DEB56966C45"));
      switch (pExc->getErrorType())
      {
         case XMLErrorReporter::ErrType_Warning:
            msg->addProperty("severity", std::string("Warning"));
            break;
         case XMLErrorReporter::ErrType_Error:
            msg->addProperty("severity", std::string("Error"));
            break;
         case XMLErrorReporter::ErrType_Fatal:
            msg->addProperty("severity", std::string("Fatal"));
            break;
         default:
            break;
      }
      msg->addProperty("line", pExc->getSrcLine());
      msg->addProperty("type", std::string(A(pExc->getType())));
      msg->addProperty("message", std::string(A(pExc->getMessage())));
      if (pExc->getSrcFile() != NULL)
      {
         msg->addProperty("source file", pExc->getSrcFile());
      }
      msg->finalize();
   }
}

void XmlBase::logException(const SAXParseException* pExc, std::string severity)
{
   if (mpLog != NULL)
   {
      Message* msg(mpLog->createMessage("XmlReader SAXParserException", "app", "EC355E3E-03CA-4081-9006-5F45D6A488B3"));
      msg->addProperty("severity", severity);
      msg->addProperty("line", pExc->getLineNumber());
      msg->addProperty("column", pExc->getColumnNumber());
      msg->addProperty("message", std::string(A(pExc->getMessage())));
      if (pExc->getPublicId() != NULL)
      {
         msg->addProperty("publicID", std::string(A(pExc->getPublicId())));
      }
      if (pExc->getSystemId() != NULL)
      {
         msg->addProperty("systemID", std::string(A(pExc->getSystemId())));
      }
      msg->finalize();
   }
}

void XmlBase::logException(const DOMException* pExc)
{
   if (mpLog != NULL)
   {
      Message* msg(mpLog->createMessage("XmlReader DOMException", "app", "3620CAD7-3535-4716-9686-E024E201481F"));
      msg->addProperty("message", std::string(A(pExc->msg)));
      msg->finalize();
   }
}

void XmlBase::logError(const XERCES_CPP_NAMESPACE_QUALIFIER DOMError& exc)
{
   if (mpLog != NULL)
   {
      Message* pMsg(mpLog->createMessage("DOMError", "app", "FE8191EE-15C6-4C33-A5F2-7BC24BD21E96"));
      pMsg->addProperty("message", A(exc.getMessage()));

      DOMLocator* pLoc = exc.getLocation();
      if (pLoc != NULL)
      {
         pMsg->addProperty("file", A(pLoc->getURI()));
         pMsg->addProperty("line", pLoc->getLineNumber());
         pMsg->addProperty("column", pLoc->getColumnNumber());
         pMsg->addProperty("offset", pLoc->getOffset());
      }
      switch (exc.getSeverity())
      {
      case DOMError::DOM_SEVERITY_WARNING:
         pMsg->addProperty("severity", "warning");
         break;
      case DOMError::DOM_SEVERITY_ERROR:
         pMsg->addProperty("severity", "error");
         break;
      case DOMError::DOM_SEVERITY_FATAL_ERROR:
         pMsg->addProperty("severity", "fatal");
         break;
      default:
         break;
      }
   }
}

void XmlBase::logSimpleMessage(std::string msg)
{
   if (mpLog != NULL)
   {
      Message* pMessage(mpLog->createMessage(msg, "app", "3620CAD7-3535-4716-9686-E024E201481F"));
      pMessage->finalize();
   }
}

XMLByte* XmlBase::encodeBase64(unsigned int* pData, unsigned int size, std::string encoding, unsigned int* pOutLen)
{
   int numBytes(size * sizeof(unsigned int));

   XMLByte* bytes(new (std::nothrow) XMLByte[numBytes]);
   if (bytes == NULL)
   {
      return NULL;
   }

   int bytesIndex(0);
   unsigned int tmp;

   // first, convert to bytes, leaving gaps as necessary
   for (unsigned int i = 0; i < size; i++)
   {
      tmp = pData[i];
      for (int j = 0; j < sizeof(tmp); j++)
      {
         bytes[bytesIndex++] = (tmp >> (8 * j)) & 0xff;
      }
   }

   unsigned int outlen;
   XMLByte* b64repr(Base64::encode(bytes, numBytes, &outlen));
   if (pOutLen != NULL)
   {
      *pOutLen = outlen;
   }

   delete [] bytes;
   return b64repr;
}

unsigned int* XmlBase::decodeBase64(const XMLByte* pData, unsigned int size, std::string encoding)
{
   unsigned int dlen;
   XMLByte* bytes(Base64::decode(pData, &dlen));
   if (dlen < (size * 4))
   {
      return NULL;
   }

   unsigned int* decoded(new (std::nothrow) unsigned int[dlen]);
   if (decoded == NULL)
   {
      return NULL;
   }

   int bytesIndex(0);
   unsigned int tmp;
   for (unsigned int i = 0; i < dlen; i++)
   {
      tmp = 0;
      for (int j = 0; j < sizeof(tmp); j++)
      {
         tmp |= bytes[bytesIndex++] << (8 * j);
      }

      decoded[i] = tmp;
   }

   XMLString::release(&bytes);
   return decoded;
}
