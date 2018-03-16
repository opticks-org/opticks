/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef XERCESINCLUDES_H
#define XERCESINCLUDES_H

#include "AppConfig.h"

#ifdef APPLICATION_XERCES

#include <xercesc/dom/DOM.hpp>
#include <xercesc/framework/MemBufFormatTarget.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/framework/Wrapper4InputSource.hpp>
#include <xercesc/framework/XMLPScanToken.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/sax/ErrorHandler.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/util/Base64.hpp>
#include <xercesc/util/BinInputStream.hpp>
#include <xercesc/util/XMLStringTokenizer.hpp>
#include <xercesc/sax/InputSource.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLURL.hpp>
#include <xercesc/util/PlatformUtils.hpp>

#else
/**
 *  \cond INTERNAL
 */
#define XERCES_CPP_NAMESPACE_QUALIFIER xercesc_${XERCES_NAMESPACE_VERSION}::
namespace xercesc_${XERCES_NAMESPACE_VERSION}
{
   class BinInputStream;
   class DOMLSParser;
   class DOMDocument;
   class DOMElement;
   class DOMError;
   class DOMImplementationLS;
   class DOMNode;
   class DOMText;
   class DOMXPathResult;
   class InputSource;
   class MemoryManager;
   class SAXParseException;
   class XMLException;
   class XMLPlatformUtils;
   class XMLString;
   class XMLStringTokenizer;
}
/// \endcond
#endif

#endif


