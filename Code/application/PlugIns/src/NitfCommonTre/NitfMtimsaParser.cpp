/*
 * The information in this file is
 * Copyright(c) 2021 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"
#include "AppVersion.h"
#include "DataVariant.h"
#include "DateTime.h"
#include "Endian.h"
#include "NitfMtimsaParser.h"
#include "NitfConstants.h"
#include "NitfUtilities.h"
#include "ObjectResource.h"
#include "PlugInRegistration.h"

using namespace std;
using namespace Nitf;
using namespace Nitf::TRE;

REGISTER_PLUGIN(OpticksNitfCommonTre, MtimsaParser, Nitf::MtimsaParser());

Nitf::MtimsaParser::MtimsaParser()
{
   setName("MTIMSA");
   setDescriptorId("{A176767D-312E-42DA-BDF2-C15117A2968E}");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

bool Nitf::MtimsaParser::runAllTests(Progress* pProgress, ostream& failure)
{
   // TRE doesn't have NTB tests yet
   return true;
}

Nitf::TreState Nitf::MtimsaParser::isTreValid(const DynamicObject& tre, ostream& reporter) const
{
   TreState status(VALID);
   set<string> testSet;
   unsigned int numFields = 0;

   // TODO

   return status;
}

template<typename T>
void readDtValues(istream& input, DynamicObject& output, size_t count)
{
   std::vector<char> buf;
   std::vector<T> dts;
   Endian swapper(BIG_ENDIAN_ORDER);
   while (count-- > 0)
   {
      if (readFromStream(input, buf, sizeof(T)))
      {
         T tmp = *reinterpret_cast<T*>(buf.data());
         swapper.swapValue(tmp);
         dts.push_back(tmp);
      }
   }
   output.setAttribute(MTIMSA::DT, dts);
}

template<typename T>
void writeDtValues(const DynamicObject& input, ostream& output)
{
   Endian swapper(BIG_ENDIAN_ORDER);
   std::vector<T> dts = dv_cast<std::vector<T>>(input.getAttribute(MTIMSA::DT));
   for (auto dt = dts.begin(); dt != dts.end(); ++dt)
   {
      T tmp = *dt;
      swapper.swapValue(tmp);
      output << sizeString(reinterpret_cast<char*>(&tmp), sizeof(T));
   }
}

bool Nitf::MtimsaParser::toDynamicObject(istream& input, size_t numBytes, DynamicObject& output,
   string &errorMessage) const
{
   vector<char> buf;
   bool success(true);

   readField<unsigned int>(input, output, success, MTIMSA::IMAGE_SEG_INDEX, 3, errorMessage, buf);
   readField<unsigned int>(input, output, success, MTIMSA::GEOCOORDS_STATIC, 2, errorMessage, buf);
   readField<std::string>(input, output, success, MTIMSA::LAYER_ID, 36, errorMessage, buf);
   readField<unsigned int>(input, output, success, MTIMSA::CAMERA_SET_INDEX, 3, errorMessage, buf);
   readField<std::string>(input, output, success, MTIMSA::CAMERA_ID, 36, errorMessage, buf);
   readField<unsigned int>(input, output, success, MTIMSA::TIME_INTERVAL_INDEX, 6, errorMessage, buf);
   readField<unsigned int>(input, output, success, MTIMSA::TEMP_BLOCK_INDEX, 3, errorMessage, buf);
   readField<float>(input, output, success, MTIMSA::NOMINAL_FRAME_RATE, 13, errorMessage, buf);
   readField<unsigned int>(input, output, success, MTIMSA::REFERENCE_FRAME_NUM, 9, errorMessage, buf);
   {
      FactoryResource<DynamicObject> baseTimestamp;
      output.setAttribute(MTIMSA::BASE_TIMESTAMP, *baseTimestamp.get());
   }
   DynamicObject& baseTimestamp = dv_cast<DynamicObject>(output.getAttribute(MTIMSA::BASE_TIMESTAMP));

   std::string dtgString;
   FactoryResource<DateTime> pBaseDate;
   double baseFrac = 0.0;
   int basePrecision = 0;
   readAndConvertFromStream(input, dtgString, success, MTIMSA::BASE_TIMESTAMP, 24, errorMessage, buf, true);
   DtgParseCCYYMMDDhhmmssns(dtgString, pBaseDate.get(), baseFrac, basePrecision);
   baseTimestamp.setAttribute("TIMESTAMP", *pBaseDate.get());
   baseTimestamp.setAttribute("FRACTIONAL_SECONDS", baseFrac);
   baseTimestamp.setAttribute("PRECISION", basePrecision);

   if (readFromStream(input, buf, 8, false))
   {
      uint64_t tmp = *reinterpret_cast<uint64_t*>(buf.data());
      output.setAttribute(MTIMSA::DT_MULTIPLIER, tmp);
   }
   uint8_t dtSize = 0;
   if (readFromStream(input, buf, 1, false))
   {
      dtSize = *reinterpret_cast<uint8_t*>(buf.data());
      output.setAttribute(MTIMSA::DT_SIZE, dtSize);
   }
   if (readFromStream(input, buf, 4, false))
   {
      uint32_t tmp = *reinterpret_cast<uint32_t*>(buf.data());
      output.setAttribute(MTIMSA::NUMBER_FRAMES, tmp);
   }
   uint32_t numberDt = 0;
   if (readFromStream(input, buf, 4, false))
   {
      numberDt = *reinterpret_cast<uint32_t*>(buf.data());
      output.setAttribute(MTIMSA::NUMBER_DT, numberDt);
   }
   switch (dtSize)
   {
   case 1:
      readDtValues<uint8_t>(input, output, numberDt);
      break;
   case 2:
      readDtValues<uint16_t>(input, output, numberDt);
      break;
   case 4:
      readDtValues<uint32_t>(input, output, numberDt);
      break;
   case 8:
      readDtValues<uint64_t>(input, output, numberDt);
      break;
   default:
      ;  // do nothing
   }

   int64_t numRead = input.tellg();
   if (numRead < 0 || static_cast<uint64_t>(numRead) > std::numeric_limits<size_t>::max() ||
      numRead != static_cast<int64_t>(numBytes))
   {
      numReadErrMsg(numRead, numBytes, errorMessage);
      return false;
   }

   return success;
}

bool Nitf::MtimsaParser::fromDynamicObject(const DynamicObject& input, ostream& output, size_t& numBytesWritten,
   string &errorMessage) const
{
   if (output.tellp() < 0 || static_cast<uint64_t>(output.tellp()) > std::numeric_limits<size_t>::max())
   {
      return false;
   }
   size_t sizeIn = max<size_t>(0, static_cast<size_t>(output.tellp()));
   size_t sizeOut(sizeIn);

   try
   {
      output << toString(dv_cast<unsigned int>(input.getAttribute(MTIMSA::IMAGE_SEG_INDEX)), 3);
      output << toString(dv_cast<unsigned int>(input.getAttribute(MTIMSA::GEOCOORDS_STATIC)), 2);
      output << sizeString(dv_cast<std::string>(input.getAttribute(MTIMSA::LAYER_ID)), 36);
      output << toString(dv_cast<unsigned int>(input.getAttribute(MTIMSA::CAMERA_SET_INDEX)), 3);
      output << sizeString(dv_cast<std::string>(input.getAttribute(MTIMSA::CAMERA_ID)), 36);
      output << toString(dv_cast<unsigned int>(input.getAttribute(MTIMSA::TIME_INTERVAL_INDEX)), 6);
      output << toString(dv_cast<unsigned int>(input.getAttribute(MTIMSA::TEMP_BLOCK_INDEX)), 3);
      // TODO: NOMINAL_FRAME_RATE
      output << toString(dv_cast<unsigned int>(input.getAttribute(MTIMSA::REFERENCE_FRAME_NUM)), 9);
      const DynamicObject& baseTimestamp = dv_cast<DynamicObject>(input.getAttribute(MTIMSA::BASE_TIMESTAMP));
      const DateTime& baseDatetime = dv_cast<DateTime>(baseTimestamp.getAttribute("TIMESTAMP"));
      double baseFrac = dv_cast<double>(baseTimestamp.getAttribute("FRACTIONAL_SECONDS"));
      int basePrecision = dv_cast<int>(baseTimestamp.getAttribute("PRECISION"));
      if (baseDatetime.isValid())
      {
         output << baseDatetime.getFormattedUtc("%Y%m%d%H%M%S");
         output << "." << toString(baseFrac, basePrecision);
         if (basePrecision < 9)
         {
            output << sizeString("-", 9 - basePrecision, '-');
         }
      }
      else
      {
         output << sizeString(" ", 25);
      }

      Endian swapper(BIG_ENDIAN_ORDER);
      uint64_t dtMultiplier = dv_cast<uint64_t>(input.getAttribute(MTIMSA::DT_MULTIPLIER));
      swapper.swapValue(dtMultiplier);
      output << sizeString(reinterpret_cast<char*>(&dtMultiplier), 8);

      uint8_t dtSize = dv_cast<uint8_t>(input.getAttribute(MTIMSA::DT_SIZE));
      output << sizeString(reinterpret_cast<char*>(&dtSize), 1);

      uint32_t numberFrames = dv_cast<uint32_t>(input.getAttribute(MTIMSA::NUMBER_FRAMES));
      swapper.swapValue(numberFrames);
      output << sizeString(reinterpret_cast<char*>(&numberFrames), 4);

      uint32_t numberDt = dv_cast<uint32_t>(input.getAttribute(MTIMSA::NUMBER_DT));
      swapper.swapValue(numberDt);
      output << sizeString(reinterpret_cast<char*>(&numberDt), 4);

      switch (dtSize)
      {
      case 1:
         writeDtValues<uint8_t>(input, output);
         break;
      case 2:
         writeDtValues<uint16_t>(input, output);
         break;
      case 4:
         writeDtValues<uint32_t>(input, output);
         break;
      case 8:
         writeDtValues<uint64_t>(input, output);
         break;
      default:
         ;  // do nothing
      }
   }
   catch (const bad_cast&)
   {
      return false;
   }

   if (output.tellp() < 0 || static_cast<uint64_t>(output.tellp()) > std::numeric_limits<size_t>::max())
   {
      return false;
   }
   sizeOut = static_cast<size_t>(output.tellp());
   numBytesWritten = sizeOut - sizeIn;
   return true;
}
