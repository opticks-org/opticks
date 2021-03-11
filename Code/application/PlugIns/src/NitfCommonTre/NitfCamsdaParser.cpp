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
#include "Location.h"
#include "NitfCamsdaParser.h"
#include "NitfConstants.h"
#include "NitfUtilities.h"
#include "ObjectResource.h"
#include "PlugInRegistration.h"

using namespace std;
using namespace Nitf;
using namespace Nitf::TRE;

REGISTER_PLUGIN(OpticksNitfCommonTre, CamsdaParser, Nitf::CamsdaParser());

namespace
{
   inline bool readIlocField(std::istream& input, DynamicObject& output, bool& success,
      const std::string& name, int len, std::string& msg, std::vector<char>& buf)
   {
      std::string tmp;
      if (!readAndConvertFromStream<std::string>(input, tmp, success, name, len, msg, buf, false, false))
      {
         return false;
      }
      bool err = false;
      int rrrrr = StringUtilities::fromDisplayString<int>(tmp.substr(0, 5), &err);
      if (err)
      {
         return false;
      }
      int ccccc = StringUtilities::fromDisplayString<int>(tmp.substr(5, 5), &err);
      if (err)
      {
         return false;
      }
      Opticks::PixelOffset iloc(ccccc, rrrrr);
      output.setAttribute(name, iloc);
      return true;
   }

   std::string ilocToString(const Opticks::PixelOffset& iloc)
   {
      return toString(iloc.mY, 5) + toString(iloc.mX, 5);
   }
}

Nitf::CamsdaParser::CamsdaParser()
{
   setName("CAMSDA");
   setDescriptorId("{709EE494-AAED-4F7A-BB7C-3987DF918EB5}");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

bool Nitf::CamsdaParser::runAllTests(Progress* pProgress, ostream& failure)
{
   // TRE doesn't have NTB tests yet
   return true;
}

Nitf::TreState Nitf::CamsdaParser::isTreValid(const DynamicObject& tre, ostream& reporter) const
{
   TreState status(VALID);
   set<string> testSet;
   unsigned int numFields = 0;

   // TODO

   return status;
}


bool Nitf::CamsdaParser::toDynamicObject(istream& input, size_t numBytes, DynamicObject& output,
   string &errorMessage) const
{
   vector<char> buf;
   bool success(true);

   readField<unsigned int>(input, output, success, CAMSDA::NUM_CAMERA_SETS, 3, errorMessage, buf);
   unsigned int numCamSetsInTre = 0;
   readAndConvertFromStream(input, numCamSetsInTre, success, CAMSDA::NUM_CAMERA_SETS_IN_TRE, 3, errorMessage, buf);
   output.setAttribute(CAMSDA::NUM_CAMERA_SETS_IN_TRE, numCamSetsInTre);
   readField<unsigned int>(input, output, success, CAMSDA::FIRST_CAMERA_SET_IN_TRE, 3, errorMessage, buf);

   {
      FactoryResource<DynamicObject> camSets;
      output.setAttribute(CAMSDA::CAMERA_SETS, *camSets.get());
   }
   DynamicObject& camSets = dv_cast<DynamicObject>(output.getAttribute(CAMSDA::CAMERA_SETS));
   for (unsigned int camSet = 0; camSet < numCamSetsInTre; ++camSet)
   {
      {
         FactoryResource<DynamicObject> camSetData;
         camSets.setAttribute(StringUtilities::toDisplayString(camSet), *camSetData.get());
      }
      DynamicObject& camSetData = dv_cast<DynamicObject>(camSets.getAttribute(StringUtilities::toDisplayString(camSet)));
      int numCameras = 0;
      readAndConvertFromStream(input, numCameras, success, CAMSDA::NUM_CAMERAS_IN_SET, 3, errorMessage, buf);
      {
         FactoryResource<DynamicObject> cameras;
         camSetData.setAttribute(CAMSDA::CAMERAS, *cameras.get());
      }
      DynamicObject& cameras = dv_cast<DynamicObject>(camSetData.getAttribute(CAMSDA::CAMERAS));
      for (auto camera = 0; camera < numCameras; ++camera)
      {
         {
            FactoryResource<DynamicObject> camData;
            cameras.setAttribute(StringUtilities::toDisplayString(camera), *camData.get());
         }
         DynamicObject& camData = dv_cast<DynamicObject>(cameras.getAttribute(StringUtilities::toDisplayString(camera)));

         readField<std::string>(input, camData, success, CAMSDA::CAMERA_ID, 36, errorMessage, buf);
         readField<std::string>(input, camData, success, CAMSDA::CAMERA_DESC, 80, errorMessage, buf);
         readField<std::string>(input, camData, success, CAMSDA::LAYER_ID, 36, errorMessage, buf);
         readField<unsigned int>(input, camData, success, CAMSDA::IDLVL, 3, errorMessage, buf);
         readField<unsigned int>(input, camData, success, CAMSDA::IALVL, 3, errorMessage, buf);
         readIlocField(input, camData, success, CAMSDA::ILOC, 10, errorMessage, buf);
         readField<unsigned int>(input, camData, success, CAMSDA::NROWS, 8, errorMessage, buf);
         readField<unsigned int>(input, camData, success, CAMSDA::NCOLS, 8, errorMessage, buf);
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

bool Nitf::CamsdaParser::fromDynamicObject(const DynamicObject& input, ostream& output, size_t& numBytesWritten,
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
      output << toString(dv_cast<unsigned int>(input.getAttribute(CAMSDA::NUM_CAMERA_SETS)), 3);
      const DynamicObject& cameraSets = dv_cast<DynamicObject>(input.getAttribute(CAMSDA::CAMERA_SETS));
      output << toString(cameraSets.getNumAttributes(), 3);
      output << toString(dv_cast<unsigned int>(input.getAttribute(CAMSDA::FIRST_CAMERA_SET_IN_TRE)), 3);

      for (auto cameraSetIt = cameraSets.begin(); cameraSetIt != cameraSets.end(); ++cameraSetIt)
      {
         const DynamicObject& cameraSet = dv_cast<DynamicObject>(
                     dv_cast<DynamicObject>(cameraSetIt->second)
                     .getAttribute(CAMSDA::CAMERAS));
         output << toString(dv_cast<unsigned int>(cameraSet.getAttribute(CAMSDA::NUM_CAMERA_SETS_IN_TRE)), 3);
         for (auto cameraSetIt = cameraSet.begin(); cameraSetIt != cameraSet.end(); ++cameraSetIt)
         {
            const DynamicObject& camera = dv_cast<DynamicObject>(cameraSetIt->second);
            output << sizeString(dv_cast<std::string>(camera.getAttribute(CAMSDA::CAMERA_ID)), 36);
            output << sizeString(dv_cast<std::string>(camera.getAttribute(CAMSDA::CAMERA_DESC)), 80);
            output << sizeString(dv_cast<std::string>(camera.getAttribute(CAMSDA::LAYER_ID)), 36);
            output << toString(dv_cast<unsigned int>(camera.getAttribute(CAMSDA::IDLVL)), 3);
            output << toString(dv_cast<unsigned int>(camera.getAttribute(CAMSDA::IALVL)), 3);
            output << ilocToString(dv_cast<const Opticks::PixelOffset>(camera.getAttribute(CAMSDA::ILOC)));
            output << toString(dv_cast<unsigned int>(camera.getAttribute(CAMSDA::NROWS)), 8);
            output << toString(dv_cast<unsigned int>(camera.getAttribute(CAMSDA::NCOLS)), 8);
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
