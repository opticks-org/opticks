/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"
#include "AppVersion.h"
#include "Blob.h"
#include "DataVariant.h"
#include "NitfConstants.h"
#include "NitfEngrdaParser.h"
#include "NitfUtilities.h"
#include "ObjectResource.h"
#include "PlugInRegistration.h"

#include <QtCore/QString>
#include <sstream>
#include <string>

using namespace Nitf::TRE;

REGISTER_PLUGIN(OpticksNitfCommonTre, EngrdaParser, Nitf::EngrdaParser());

Nitf::EngrdaParser::EngrdaParser()
{
   setName("ENGRDA");
   setDescriptorId("{0e462a75-6a28-4a08-9460-3c7c1e6a66ff}");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

bool Nitf::EngrdaParser::runAllTests(Progress* pProgress, std::ostream& failure)
{
   return true;
}

Nitf::TreState Nitf::EngrdaParser::isTreValid(const DynamicObject& tre, std::ostream& reporter) const
{
   TreState status(VALID);
   return status;
}

bool Nitf::EngrdaParser::toDynamicObject(std::istream& input,
                                         size_t numBytes,
                                         DynamicObject& output,
                                         std::string &errorMessage) const
{
   std::vector<char> buf;
   bool success(true);

   Nitf::readField<std::string>(input, output, success, ENGRDA::RESRC, 20, errorMessage, buf);
   Nitf::readField<unsigned int>(input, output, success, ENGRDA::RECNT, 3, errorMessage, buf);
   unsigned int recnt = QString(&buf.front()).toUInt();
   for (unsigned int rec = 0; rec < recnt; ++rec)
   {
      std::string recId = QString::number(rec).toStdString();
      {
         FactoryResource<DynamicObject> record;
         output.setAttribute(recId, *record.get());
      }
      DynamicObject& record = dv_cast<DynamicObject>(output.getAttribute(recId));
      unsigned int lbln(0);
      Nitf::readAndConvertFromStream(input, lbln, success, ENGRDA::ENGLN, 2, errorMessage, buf);
      Nitf::readField<std::string>(input, record, success, ENGRDA::ENGLBL, lbln, errorMessage, buf);
      Nitf::readField<unsigned int>(input, record, success, ENGRDA::ENGMTXC, 4, errorMessage, buf);
      Nitf::readField<unsigned int>(input, record, success, ENGRDA::ENGMTXR, 4, errorMessage, buf);
      Nitf::readField<std::string>(input, record, success, ENGRDA::ENGTYP, 1, errorMessage, buf);
      char typ = buf.front();
      Nitf::readField<unsigned int>(input, record, success, ENGRDA::ENGDTS, 1, errorMessage, buf);
      int dts = QString(&buf.front()).toInt(); // bytes per element
      Nitf::readField<std::string>(input, record, success, ENGRDA::ENGDATU, 2, errorMessage, buf);
      unsigned int datc(0); // data count
      Nitf::readAndConvertFromStream(input, datc, success, ENGRDA::ENGDATC, 8, errorMessage, buf);

      // read the data
      Nitf::readFromStream(input, buf, datc * dts, false);
      record.setAttribute(ENGRDA::ENGDATA, Blob(&buf.front(), buf.size()));
   }

   size_t numRead = input.tellg();
   if (numRead != numBytes)
   {
      numReadErrMsg(numRead, numBytes, errorMessage);
      return false;
   }

   return success;
}

bool Nitf::EngrdaParser::fromDynamicObject(const DynamicObject& input,
                                           std::ostream& output,
                                           size_t& numBytesWritten,
                                           std::string &errorMessage) const
{
   size_t sizeIn = std::max(static_cast<std::ostream::pos_type>(0), output.tellp());
   size_t sizeOut(sizeIn);

   try
   {
      output << sizeString(dv_cast<std::string>(input.getAttribute(ENGRDA::RESRC)), 20);
      unsigned int cnt = dv_cast<unsigned int>(input.getAttribute(ENGRDA::RECNT));
      output << toString(cnt, 3);
      for (size_t recNum = 0; recNum < cnt; ++recNum)
      {
         const DynamicObject& rec = dv_cast<const DynamicObject>(
            input.getAttribute(StringUtilities::toXmlString(recNum)));
         std::string lbl = dv_cast<std::string>(rec.getAttribute(ENGRDA::ENGLBL));
         output << toString(lbl.size(), 2);
         output << sizeString(lbl, lbl.size());
         output << toString(dv_cast<unsigned int>(rec.getAttribute(ENGRDA::ENGMTXC)), 4);
         output << toString(dv_cast<unsigned int>(rec.getAttribute(ENGRDA::ENGMTXR)), 4);
         output << sizeString(dv_cast<std::string>(rec.getAttribute(ENGRDA::ENGTYP)), 1);
         unsigned int dts = dv_cast<unsigned int>(rec.getAttribute(ENGRDA::ENGDTS));
         output << toString(dts, 1);
         output << sizeString(dv_cast<std::string>(rec.getAttribute(ENGRDA::ENGDATU)), 2);
         const Blob* data = dv_cast<Blob>(&rec.getAttribute(ENGRDA::ENGDATA));
         const std::vector<unsigned char>& rawData = data->get();
         unsigned int datc = rawData.size() / dts;
         output << toString(datc, 8);
         output.write(reinterpret_cast<const char*>(&(rawData.front())), rawData.size());
      }
   }
   catch (const std::bad_cast&)
   {
      return false;
   }

   sizeOut = output.tellp();
   numBytesWritten = sizeOut - sizeIn;
   return true;
}
