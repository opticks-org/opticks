/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "DynamicObject.h"
#include "MessageLogMgr.h"
#include "MessageLogResource.h"
#include "ObjectResource.h"
#include "QuickbirdIsd.h"
#include "xmlreader.h"

#include <QtCore/QString>
#include <vector>

#define SET_FROM_XQUERY(path__, query__, accessor__, type__) \
   current = query__; \
   pResult = xml.query(query__, XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE); \
   VERIFY(pResult != NULL); \
   pMetadata->setAttributeByPath(path__, static_cast<type__>(pResult->accessor__()));

QuickbirdIsd::QuickbirdIsd(const std::string &filename) : mFilename(filename)
{
}

QuickbirdIsd::~QuickbirdIsd()
{
}

bool QuickbirdIsd::copyToMetadata(DynamicObject *pMetadata) const
{
   XmlReader xml(Service<MessageLogMgr>()->getLog(), false);
   if (mFilename.empty() || xml.parse(mFilename) == NULL)
   {
      return false;
   }
   std::string current;
   try
   {
      XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult* pResult =
         xml.query("//RPB/SPECID/text()='RPC00B'", XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
      if (pResult == NULL || !pResult->getBooleanValue())
      {
         return false;
      }
      SET_FROM_XQUERY("NITF/TRE/RPC00B/0/ERR_BIAS", "xs:float(//RPB/IMAGE/ERRBIAS/text())",
         getNumberValue, double);
      SET_FROM_XQUERY("NITF/TRE/RPC00B/0/ERR_RAND", "xs:float(//RPB/IMAGE/ERRRAND/text())",
         getNumberValue, double);
      SET_FROM_XQUERY("NITF/TRE/RPC00B/0/LINE_OFF", "xs:integer(//RPB/IMAGE/LINEOFFSET/text())",
         getIntegerValue, unsigned int);
      SET_FROM_XQUERY("NITF/TRE/RPC00B/0/SAMP_OFF", "xs:integer(//RPB/IMAGE/SAMPOFFSET/text())",
         getIntegerValue, unsigned int);
      SET_FROM_XQUERY("NITF/TRE/RPC00B/0/LAT_OFF", "xs:float(//RPB/IMAGE/LATOFFSET/text())",
         getNumberValue, double);
      SET_FROM_XQUERY("NITF/TRE/RPC00B/0/LONG_OFF", "xs:float(//RPB/IMAGE/LONGOFFSET/text())",
         getNumberValue, double);
      SET_FROM_XQUERY("NITF/TRE/RPC00B/0/HEIGHT_OFF", "xs:integer(//RPB/IMAGE/HEIGHTOFFSET/text())",
         getIntegerValue, int);
      SET_FROM_XQUERY("NITF/TRE/RPC00B/0/LINE_SCALE", "xs:integer(//RPB/IMAGE/LINESCALE/text())",
         getIntegerValue, unsigned int);
      SET_FROM_XQUERY("NITF/TRE/RPC00B/0/SAMP_SCALE", "xs:integer(//RPB/IMAGE/SAMPSCALE/text())",
         getIntegerValue, unsigned int);
      SET_FROM_XQUERY("NITF/TRE/RPC00B/0/LAT_SCALE", "xs:float(//RPB/IMAGE/LATSCALE/text())",
         getNumberValue, double);
      SET_FROM_XQUERY("NITF/TRE/RPC00B/0/LONG_SCALE", "xs:float(//RPB/IMAGE/LONGSCALE/text())",
         getNumberValue, double);
      SET_FROM_XQUERY("NITF/TRE/RPC00B/0/HEIGHT_SCALE", "xs:integer(//RPB/IMAGE/HEIGHTSCALE/text())",
         getIntegerValue, int);
      current = "fn:tokenize(//RPB/IMAGE/LINENUMCOEFList/LINENUMCOEF/text(), '\\s+')";
      pResult = xml.query(current, XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::ITERATOR_RESULT_TYPE);
      VERIFY(pResult != NULL);
      for (int num = 1; pResult->iterateNext(); num++)
      {
         pMetadata->setAttributeByPath(QString("NITF/TRE/RPC00B/0/LNNUMCOEF%1")
            .arg(num, 2, 10, QChar('0')).toStdString(), pResult->getNumberValue());
      }
      current = "fn:tokenize(//RPB/IMAGE/LINEDENCOEFList/LINEDENCOEF/text(), '\\s+')";
      pResult = xml.query(current, XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::ITERATOR_RESULT_TYPE);
      VERIFY(pResult != NULL);
      for (int num = 1; pResult->iterateNext(); num++)
      {
         pMetadata->setAttributeByPath(QString("NITF/TRE/RPC00B/0/LNDENCOEF%1")
            .arg(num, 2, 10, QChar('0')).toStdString(), pResult->getNumberValue());
      }
      current = "fn:tokenize(//RPB/IMAGE/SAMPNUMCOEFList/SAMPNUMCOEF/text(), '\\s+')";
      pResult = xml.query(current, XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::ITERATOR_RESULT_TYPE);
      VERIFY(pResult != NULL);
      for (int num = 1; pResult->iterateNext(); num++)
      {
         pMetadata->setAttributeByPath(QString("NITF/TRE/RPC00B/0/SMPNUMCOEF%1")
            .arg(num, 2, 10, QChar('0')).toStdString(), pResult->getNumberValue());
      }
      current = "fn:tokenize(//RPB/IMAGE/SAMPDENCOEFList/SAMPDENCOEF/text(), '\\s+')";
      pResult = xml.query(current, XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::ITERATOR_RESULT_TYPE);
      VERIFY(pResult != NULL);
      for (int num = 1; pResult->iterateNext(); num++)
      {
         pMetadata->setAttributeByPath(QString("NITF/TRE/RPC00B/0/SMPDENCOEF%1")
            .arg(num, 2, 10, QChar('0')).toStdString(), pResult->getNumberValue());
      }
      pMetadata->setAttributeByPath("NITF/TRE/RPC00B/0/SUCCESS", true);
      FactoryResource<DynamicObject> image;
      pMetadata->setAttributeByPath("NITF/Image Subheader", *image);
   }
   catch (const XERCES_CPP_NAMESPACE_QUALIFIER DOMException &exc)
   {
      MessageResource m(A(exc.msg), "app", "0D4EC7B3-1CA6-4039-B8D6-5808933E13C5");
      if (!current.empty())
      {
         m->addProperty("query", current);
      }
      return false;
   }

   return true;
}
