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
#include "NitfMtimfaParser.h"
#include "NitfConstants.h"
#include "NitfUtilities.h"
#include "ObjectResource.h"
#include "PlugInRegistration.h"

using namespace std;
using namespace Nitf;
using namespace Nitf::TRE;

REGISTER_PLUGIN(OpticksNitfCommonTre, MtimfaParser, Nitf::MtimfaParser());

Nitf::MtimfaParser::MtimfaParser()
{
   setName("MTIMFA");
   setDescriptorId("{1DE29023-899B-44A8-A420-2022243139FC}");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

bool Nitf::MtimfaParser::runAllTests(Progress* pProgress, ostream& failure)
{
   // TRE doesn't have NTB tests yet
   return true;
}

Nitf::TreState Nitf::MtimfaParser::isTreValid(const DynamicObject& tre, ostream& reporter) const
{
   TreState status(VALID);
   set<string> testSet;
   unsigned int numFields = 0;

   // TODO

   return status;
}


bool Nitf::MtimfaParser::toDynamicObject(istream& input, size_t numBytes, DynamicObject& output,
   string &errorMessage) const
{
   vector<char> buf;
   bool success(true);

   readField<string>(input, output, success, MTIMFA::LAYER_ID, 36, errorMessage, buf);
   readField<unsigned int>(input, output, success, MTIMFA::CAMERA_SET_INDEX, 3, errorMessage, buf);
   readField<unsigned int>(input, output, success, MTIMFA::TIME_INTERVAL_INDEX, 6, errorMessage, buf);
   int numCameras = 0;
   readAndConvertFromStream(input, numCameras, success, MTIMFA::NUM_CAMERAS_DEFINED, 3, errorMessage, buf);
   {
      FactoryResource<DynamicObject> camerasDefined;
      output.setAttribute(MTIMFA::CAMERAS_DEFINED, *camerasDefined.get());
   }
   DynamicObject& camerasDefined = dv_cast<DynamicObject>(output.getAttribute(MTIMFA::CAMERAS_DEFINED));
   for (auto cameraIdx = 0; cameraIdx < numCameras; ++cameraIdx)
   {
      {
         FactoryResource<DynamicObject> cameraData;
         camerasDefined.setAttribute(StringUtilities::toDisplayString(cameraIdx), *cameraData.get());
      }
      DynamicObject& cameraData = dv_cast<DynamicObject>(camerasDefined.getAttribute(StringUtilities::toDisplayString(cameraIdx)));

      readField<std::string>(input, cameraData, success, MTIMFA::CAMERA_ID, 36, errorMessage, buf);
      int numTempBlocks = 0;
      readAndConvertFromStream(input, numTempBlocks, success, MTIMFA::NUM_TEMP_BLOCKS, 3, errorMessage, buf);
      {
         FactoryResource<DynamicObject> tempBlocks;
         cameraData.setAttribute(MTIMFA::TEMP_BLOCKS, *tempBlocks.get());
      }
      DynamicObject& tempBlocks = dv_cast<DynamicObject>(cameraData.getAttribute(MTIMFA::TEMP_BLOCKS));
      for (auto tempBlockIdx = 0; tempBlockIdx < numTempBlocks; ++tempBlockIdx)
      {
         {
            FactoryResource<DynamicObject> tempBlockData;
            tempBlocks.setAttribute(StringUtilities::toDisplayString(tempBlockIdx), *tempBlockData.get());
         }
         DynamicObject& tempBlockData = dv_cast<DynamicObject>(tempBlocks.getAttribute(StringUtilities::toDisplayString(tempBlockIdx)));
         
         {
            FactoryResource<DynamicObject> startDateData;
            tempBlockData.setAttribute(MTIMFA::START_TIMESTAMP, *startDateData.get());
            FactoryResource<DynamicObject> endDateData;
            tempBlockData.setAttribute(MTIMFA::END_TIMESTAMP, *endDateData.get());
         }
         DynamicObject& startDateData = dv_cast<DynamicObject>(tempBlockData.getAttribute(MTIMFA::START_TIMESTAMP));
         DynamicObject& endDateData = dv_cast<DynamicObject>(tempBlockData.getAttribute(MTIMFA::END_TIMESTAMP));

         std::string dtgString;
         FactoryResource<DateTime> pStartDate;
         FactoryResource<DateTime> pEndDate;
         double startFrac = 0.0;
         double endFrac = 0.0;
         int startPrecision = 0;
         int endPrecision = 0;
         readAndConvertFromStream(input, dtgString, success, MTIMFA::START_TIMESTAMP, 24, errorMessage, buf, true);
         DtgParseCCYYMMDDhhmmssns(dtgString, pStartDate.get(), startFrac, startPrecision);
         startDateData.setAttribute("TIMESTAMP", *pStartDate.get());
         startDateData.setAttribute("FRACTIONAL_SECONDS", startFrac);
         startDateData.setAttribute("PRECISION", startPrecision);

         readAndConvertFromStream(input, dtgString, success, MTIMFA::END_TIMESTAMP, 24, errorMessage, buf, true);
         DtgParseCCYYMMDDhhmmssns(dtgString, pEndDate.get(), endFrac, endPrecision);
         endDateData.setAttribute("TIMESTAMP", *pEndDate.get());
         endDateData.setAttribute("FRACTIONAL_SECONDS", endFrac);
         endDateData.setAttribute("PRECISION", endPrecision);

         readField<unsigned int>(input, tempBlockData, success, MTIMFA::IMAGE_SEG_INDEX, 3, errorMessage, buf, true);
      }
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

bool Nitf::MtimfaParser::fromDynamicObject(const DynamicObject& input, ostream& output, size_t& numBytesWritten,
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
      output << sizeString(dv_cast<std::string>(input.getAttribute(MTIMFA::LAYER_ID)), 36);
      output << toString(dv_cast<unsigned int>(input.getAttribute(MTIMFA::CAMERA_SET_INDEX)), 3);
      output << toString(dv_cast<unsigned int>(input.getAttribute(MTIMFA::TIME_INTERVAL_INDEX)), 6);
      output << toString(dv_cast<unsigned int>(input.getAttribute(MTIMFA::NUM_CAMERAS_DEFINED)), 4);
      const DynamicObject& camerasDefined = dv_cast<DynamicObject>(input.getAttribute(MTIMFA::CAMERAS_DEFINED));
      std::vector<std::string> camerasDefinedRecs;
      camerasDefined.getAttributeNames(camerasDefinedRecs);
      for (auto camerasDefinedName = camerasDefinedRecs.begin(); camerasDefinedName != camerasDefinedRecs.end(); ++camerasDefinedName)
      {
         const DynamicObject& camera = dv_cast<DynamicObject>(camerasDefined.getAttribute(*camerasDefinedName));
         output << sizeString(dv_cast<std::string>(camera.getAttribute(MTIMFA::CAMERA_ID)), 36);
         output << toString(dv_cast<unsigned int>(camera.getAttribute(MTIMFA::NUM_TEMP_BLOCKS)), 3);
         const DynamicObject& tempBlocks = dv_cast<DynamicObject>(camera.getAttribute(MTIMFA::TEMP_BLOCKS));
         std::vector<std::string> tempBlocksRecs;
         tempBlocks.getAttributeNames(tempBlocksRecs);
         for (auto tempBlocksName = tempBlocksRecs.begin(); tempBlocksName != tempBlocksRecs.end(); ++tempBlocksName)
         {
            const DynamicObject& tempBlock = dv_cast<DynamicObject>(tempBlocks.getAttribute(*tempBlocksName));
            const DynamicObject& startTimestamp = dv_cast<DynamicObject>(tempBlock.getAttribute(MTIMFA::START_TIMESTAMP));
            const DateTime& startDatetime = dv_cast<DateTime>(startTimestamp.getAttribute("TIMESTAMP"));
            double startFrac = dv_cast<double>(startTimestamp.getAttribute("FRACTIONAL_SECONDS"));
            int startPrecision = dv_cast<int>(startTimestamp.getAttribute("PRECISION"));
            if (startDatetime.isValid())
            {
               output << startDatetime.getFormattedUtc("%Y%m%d%H%M%S");
               output << "." << toString(startFrac, startPrecision);
               if (startPrecision < 9)
               {
                  output << sizeString("-", 9 - startPrecision, '-');
               }
            }
            else
            {
               output << sizeString(" ", 25);
            }
            const DynamicObject& endTimestamp = dv_cast<DynamicObject>(tempBlock.getAttribute(MTIMFA::END_TIMESTAMP));
            const DateTime& endDatetime = dv_cast<DateTime>(endTimestamp.getAttribute("TIMESTAMP"));
            double endFrac = dv_cast<double>(endTimestamp.getAttribute("FRACTIONAL_SECONDS"));
            int endPrecision = dv_cast<int>(endTimestamp.getAttribute("PRECISION"));
            if (endDatetime.isValid())
            {
               output << endDatetime.getFormattedUtc("%Y%m%d%H%M%S");
               output << "." << toString(endFrac, endPrecision);
               if (endPrecision < 9)
               {
                  output << sizeString("-", 9 - endPrecision, '-');
               }
            }
            else
            {
               output << sizeString(" ", 25);
            }
            output << toString(dv_cast<unsigned int>(tempBlock.getAttribute(MTIMFA::IMAGE_SEG_INDEX)), 3);
         }
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
