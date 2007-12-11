/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef XERCES_INCLUDES_H
#define XERCES_INCLUDES_H

#include "AppConfig.h"

#ifdef APPLICATION_XERCES
//Ignore Warning Only for Xerces
#if defined(WIN_API)
#pragma warning( push, 1 )
#endif

#include <xercesc/dom/DOM.hpp>
#include <xercesc/dom/DOMNode.hpp>
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
#include <xqilla/xqilla-dom3.hpp>

#if defined(WIN_API)
#pragma warning( pop )
#endif
#else
#define XERCES_CPP_NAMESPACE_QUALIFIER xercesc_2_7::
namespace xercesc_2_7
{
   class DOMNode;
}
#endif

#endif

