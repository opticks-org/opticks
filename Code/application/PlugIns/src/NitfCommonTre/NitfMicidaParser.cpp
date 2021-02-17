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
#include "NitfMicidaParser.h"
#include "NitfConstants.h"
#include "NitfUtilities.h"
#include "PlugInRegistration.h"

using namespace std;
using namespace Nitf;
using namespace Nitf::TRE;

REGISTER_PLUGIN(OpticksNitfCommonTre, MicidaParser, Nitf::MicidaParser());

Nitf::MicidaParser::MicidaParser()
{
   setName("MICIDA");
   setDescriptorId("{4962B9F9-FE2C-4417-AFC1-5EDEB69D04D7}");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

bool Nitf::MicidaParser::runAllTests(Progress* pProgress, ostream& failure)
{
   // TRE doesn't have NTB tests yet
   return true;
}

Nitf::TreState Nitf::MicidaParser::isTreValid(const DynamicObject& tre, ostream& reporter) const
{
   TreState status(VALID);
   set<string> testSet;
   unsigned int numFields = 0;

   // TODO

   return status;
}


bool Nitf::MicidaParser::toDynamicObject(istream& input, size_t numBytes, DynamicObject& output,
   string &errorMessage) const
{
   vector<char> buf;
   bool success(true);

   readField<unsigned int>(input, output, success, MICIDA::MIIS_CORE_ID_VERSION, 2, errorMessage, buf);
   int numCameraIds = 0;
   readAndConvertFromStream(input, numCameraIds, success, MICIDA::NUM_CAMERA_IDS_IN_TRE, 3, errorMessage, buf);
   {
      FactoryResource<DynamicObject> cameraIds;
      output.setAttribute(MICIDA::CAMERA_IDS, *cameraIds.get());
   }
   DynamicObject& cameraIds = dv_cast<DynamicObject>(output.getAttribute(MICIDA::CAMERA_IDS));
   for (auto cameraIdIdx = 0; cameraIdIdx < numCameraIds; ++cameraIdIdx)
   {
      {
         FactoryResource<DynamicObject> cameraIdData;
         cameraIds.setAttribute(StringUtilities::toDisplayString(cameraIdIdx), *cameraIdData.get());
      }
      DynamicObject& cameraIdData = dv_cast<DynamicObject>(cameraIds.getAttribute(StringUtilities::toDisplayString(cameraIdIdx)));

      readField<std::string>(input, cameraIdData, success, MICIDA::CAMERA_ID, 36, errorMessage, buf);
      unsigned int coreIdLength = 0;
      readAndConvertFromStream<unsigned int>(input, coreIdLength, success, MICIDA::CORE_ID_LENGTH, 3, errorMessage, buf);
      cameraIdData.setAttribute(MICIDA::CORE_ID_LENGTH, coreIdLength);
      readField<std::string>(input, cameraIdData, success, MICIDA::CAMERA_CORE_ID, coreIdLength, errorMessage, buf);
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

bool Nitf::MicidaParser::fromDynamicObject(const DynamicObject& input, ostream& output, size_t& numBytesWritten,
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
      output << toString(dv_cast<unsigned int>(input.getAttribute(MICIDA::MIIS_CORE_ID_VERSION)), 2);
      const DynamicObject& cameraIds = dv_cast<DynamicObject>(input.getAttribute(MICIDA::CAMERA_IDS));
      std::vector<std::string> cameraIdsRecs;
      cameraIds.getAttributeNames(cameraIdsRecs);
      output << toString<unsigned int>(cameraIdsRecs.size(), 3);
      for (auto cameraIdName = cameraIdsRecs.begin(); cameraIdName != cameraIdsRecs.end(); ++cameraIdName)
      {
         const DynamicObject& cameraId = dv_cast<DynamicObject>(cameraIds.getAttribute(*cameraIdName));
         output << sizeString(dv_cast<std::string>(cameraId.getAttribute(MICIDA::CAMERA_ID)), 36);
         auto coreIdLength = dv_cast<unsigned int>(cameraId.getAttribute(MICIDA::CORE_ID_LENGTH));
         output << toString(coreIdLength, 3);
         output << sizeString(dv_cast<std::string>(cameraId.getAttribute(MICIDA::CAMERA_CORE_ID)), coreIdLength);
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
