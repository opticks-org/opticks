/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "AppVersion.h"
#include "DataVariant.h"
#include "DynamicObject.h"
#include "GeoreferenceDescriptor.h"
#include "MessageLogResource.h"
#include "NitfConstants.h"
#include "NitfResource.h"
#include "NitfUtilities.h"
#include "PlugInArgList.h"
#include "PlugInRegistration.h"
#include "Progress.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RpcGeoreference.h"
#include "RpcGui.h"
#include "SessionItemDeserializer.h"
#include "SessionItemSerializer.h"

#include <ossim/base/ossimKeywordlist.h>

#include <sstream>
#include <string>
#include <vector>

using namespace Nitf;
using namespace Nitf::TRE;
using namespace Nitf::TRE::RPC;
using namespace std;
XERCES_CPP_NAMESPACE_USE

REGISTER_PLUGIN(OpticksNitf, RpcGeoreference, Nitf::RpcGeoreference);

Nitf::RpcGeoreference::RpcGeoreference() :
   mpRaster(NULL),
   mHeight(0.0),
   mpGui(NULL)
{
   setName("RPC Georeference");
   setVersion(APP_VERSION_NUMBER);
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setDescription("RPC Georeferencing plugin");
   setDescriptorId("{466A8B43-186F-4fe1-8D5A-164B762E195B}");
   allowMultipleInstances(true);
   executeOnStartup(false);
   destroyAfterExecute(false);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

Nitf::RpcGeoreference::~RpcGeoreference()
{
   delete mpGui;
}

namespace
{
   struct FindFirstValidRpc
   {
      bool operator()(const DynamicObject& dynObj)
      {
         return dynObj.getAttribute(getRpcCoefficient(LINE_NUMERATOR_COEF_PREFIX, 1)).getPointerToValue<double>() != NULL;
      }
   };
}

bool Nitf::RpcGeoreference::execute(PlugInArgList* pInParam, PlugInArgList* pOutParam)
{
   StepResource pStep("Run Georeference", "app", "365B658B-83D1-4709-9A9A-F4FC064258FD");
   pStep->addProperty("name", getName());

   if (!isBatch())
   {
      pStep->finalize(Message::Failure, "Interactive mode is not supported for this plug-in.");
      return false;
   }

   if (pInParam == NULL)
   {
      pStep->finalize(Message::Failure, "Input argument list is NULL!");
      return false;
   }

   // Get the values from the input arg list
   mpRaster = pInParam->getPlugInArgValue<RasterElement>(Executable::DataElementArg());
   Progress* pProgress = pInParam->getPlugInArgValue<Progress>(Executable::ProgressArg());

   string messageText;
   if (mpRaster == NULL)
   {
      messageText = "No data cube passed to the plug-in.";
      if (pProgress)
      {
         pProgress->updateProgress(messageText, 0, ERRORS);
      }
      pStep->finalize(Message::Failure, messageText);
      return false;
   }

   bool heightArgSet = pInParam->getPlugInArgValue<double>("Height", mHeight);

   // If the height value was not contained in the arg list, get the value from the georeference descriptor
   RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(mpRaster->getDataDescriptor());
   if (pDescriptor == NULL)
   {
      messageText = "Could not find the raster data descriptor.";
      if (pProgress)
      {
         pProgress->updateProgress(messageText, 0, ERRORS);
      }
      pStep->finalize(Message::Failure, messageText);
      return false;
   }

   GeoreferenceDescriptor* pGeorefDescriptor = pDescriptor->getGeoreferenceDescriptor();
   if (pGeorefDescriptor != NULL)
   {
      if (heightArgSet == false)
      {
         mHeight = dv_cast<double>(pGeorefDescriptor->getAttributeByPath("RPC Georeference/Height"), mHeight);
      }
   }

   if (pProgress)
   {
      pProgress->updateProgress(messageText, 100, NORMAL);
   }

   /********************************************************************************
   this is the RPC section of the Geocode referencing.  It will try to load the RPC parameters
   out of the metadata, if they don't exist the georeferencing will fail.
   ********************************************************************************/
   bool bInit = false;

   const DynamicObject* pMetadata = mpRaster->getMetadata();
   if (pMetadata == NULL)
   {
      messageText = "Invalid metadata in Raster Element!";
      if (pProgress)
      {
         pProgress->updateProgress(messageText, 0, ERRORS);
      }
      pStep->finalize(Message::Failure, messageText);
      return false;
   }

   const DynamicObject* pNitfMetadata = pMetadata->getAttribute("NITF").getPointerToValue<DynamicObject>();
   if (pNitfMetadata == NULL)
   {
      messageText = "Could not find NITF metadata!";
      if (pProgress)
      {
         pProgress->updateProgress(messageText, 0, ERRORS);
      }
      pStep->finalize(Message::Failure, messageText);
      return false;
   }

   //now we look up all of the parameters we need and put them in the 
   // geocoord matrix.
   double lineRPCnumCoefficients[NUM_RPC_COEFFICIENTS];
   double lineRPCdenCoefficients[NUM_RPC_COEFFICIENTS];
   double sampRPCnumCoefficients[NUM_RPC_COEFFICIENTS];
   double sampRPCdenCoefficients[NUM_RPC_COEFFICIENTS];

   const DynamicObject* pRpc = getRpcInstance(pDescriptor);
   if (pRpc != NULL)
   {
      for (int i = 0; i < NUM_RPC_COEFFICIENTS; i++)
      {
         istringstream vstrm(pRpc->getAttribute(getRpcCoefficient(LINE_NUMERATOR_COEF_PREFIX, i+1)).toXmlString());
         vstrm >> lineRPCnumCoefficients[i];
      }
      for (int i = 0; i < NUM_RPC_COEFFICIENTS; i++)
      {
         istringstream vstrm(pRpc->getAttribute(getRpcCoefficient(LINE_DENOMINATOR_COEF_PREFIX, i+1)).toXmlString());
         vstrm >> lineRPCdenCoefficients[i];
      }
      for (int i = 0; i < NUM_RPC_COEFFICIENTS; i++)
      {
         istringstream vstrm(pRpc->getAttribute(getRpcCoefficient(SAMPLE_NUMERATOR_COEF_PREFIX, i+1)).toXmlString());
         vstrm >> sampRPCnumCoefficients[i];
      }
      for (int i = 0; i < NUM_RPC_COEFFICIENTS; i++)
      {
         istringstream vstrm(pRpc->getAttribute(getRpcCoefficient(SAMPLE_DENOMINATOR_COEF_PREFIX, i+1)).toXmlString());
         vstrm >> sampRPCdenCoefficients[i];
      }

      double rpcLineScale = 0.0;
      double rpcSampScale = 0.0;
      double rpcLatScale = 0.0;
      double rpcLongScale = 0.0;
      double rpcHeightScale = 0.0;
      double rpcLineOffset = 0.0;
      double rpcSampOffset = 0.0;
      double rpcLatOffset = 0.0;
      double rpcLongOffset = 0.0;
      double rpcHeightOffset = 0.0;
      try
      {
         rpcLineScale = dv_cast<unsigned int>(pRpc->getAttribute(LINE_SCALE));
         rpcSampScale = dv_cast<unsigned int>(pRpc->getAttribute(SAMP_SCALE));
         rpcLatScale = dv_cast<double>(pRpc->getAttribute(LAT_SCALE));
         rpcLongScale = dv_cast<double>(pRpc->getAttribute(LONG_SCALE));
         rpcHeightScale = dv_cast<int>(pRpc->getAttribute(HEIGHT_SCALE));
         rpcLineOffset = dv_cast<unsigned int>(pRpc->getAttribute(LINE_OFFSET));
         rpcSampOffset = dv_cast<unsigned int>(pRpc->getAttribute(SAMP_OFFSET));
         rpcLatOffset = dv_cast<double>(pRpc->getAttribute(LAT_OFFSET));
         rpcLongOffset = dv_cast<double>(pRpc->getAttribute(LONG_OFFSET));
         rpcHeightOffset = dv_cast<int>(pRpc->getAttribute(HEIGHT_OFFSET));
      }
      catch (const bad_cast&)
      {
         messageText = "RPC Georeference failed to extract all RPC fields from the RPC DynamicObject.";
         if (pProgress)
         {
            pProgress->updateProgress(messageText, 0, ERRORS);
         }
         pStep->finalize(Message::Failure, messageText);
         return false;
      }

      try
      {
         Nitf::ChipConverter* pConverter = new Nitf::ChipConverter(*pDescriptor);
         mpChipConverter.reset(pConverter);
      }
      catch (const string& message)
      {
         messageText = message;
         if (pProgress)
         {
            pProgress->updateProgress(messageText, 0, ERRORS);
         }
         pStep->finalize(Message::Failure, messageText);
         return false;
      }

      vector<double> xNumCoef(NUM_RPC_COEFFICIENTS);
      vector<double> xDenCoef(NUM_RPC_COEFFICIENTS);
      vector<double> yNumCoef(NUM_RPC_COEFFICIENTS);
      vector<double> yDenCoef(NUM_RPC_COEFFICIENTS);

      for (int i = 0; i < NUM_RPC_COEFFICIENTS; i++)
      {
         xNumCoef[i] = sampRPCnumCoefficients[i];
         xDenCoef[i] = sampRPCdenCoefficients[i];
         yNumCoef[i] = lineRPCnumCoefficients[i];
         yDenCoef[i] = lineRPCdenCoefficients[i];
      }
      mModel.setAttributes(rpcSampOffset, rpcLineOffset, rpcSampScale, rpcLineScale,
         rpcLatOffset, rpcLongOffset, rpcHeightOffset, rpcLatScale, rpcLongScale, rpcHeightScale,
         xNumCoef, xDenCoef, yNumCoef, yDenCoef, (mRpcVersion == "A") ? ossimRpcModel::A : ossimRpcModel::B);

      bInit = true;
   }
   else
   {
      messageText = "File does not have an RPC TRE.";
      if (pProgress)
      {
         pProgress->updateProgress(messageText, 0, ERRORS);
      }
      pStep->finalize(Message::Failure, messageText);
      return false;
   }

   if (!bInit)
   {
      messageText = "RPC Georeference failed to initialize state for georeferencing!";
      if (pProgress)
      {
         pProgress->updateProgress(messageText, 0, ERRORS);
      }
      pStep->finalize(Message::Failure, messageText);
      return false;
   }

   mpRaster->setGeoreferencePlugin(this);

   // Update the georeference descriptor with the current georeference parameters if necessary
   if (pGeorefDescriptor != NULL)
   {
      // Georeference plug-in
      const string& plugInName = getName();
      pGeorefDescriptor->setGeoreferencePlugInName(plugInName);

      // Height
      if (heightArgSet == true)
      {
         pGeorefDescriptor->setAttributeByPath("RPC Georeference/Height", mHeight);
      }
   }

   messageText = "RpcGeoreference execute completed.";
   if (pProgress)
   {
      pProgress->updateProgress(messageText, 100, NORMAL);
   }
   pStep->finalize(Message::Success);

   return true;
}

bool Nitf::RpcGeoreference::getInputSpecification(PlugInArgList*& pArgList)
{
   if (GeoreferenceShell::getInputSpecification(pArgList) == false)
   {
      return false;
   }

   // Do not set a default value in the input args so that the georeference
   // descriptor values will be used if the arg value is not set
   VERIFY(pArgList != NULL);
   VERIFY(pArgList->addArg<double>("Height", "Height value for the georeferencing calculations in meters.  If not "
      "specified, the height value contained in the " + Executable::DataElementArg() + " input will be used.") == true);

   return true;
}

LocationType Nitf::RpcGeoreference::pixelToGeo(LocationType pixel, bool* pAccurate) const
{
   pixel = mpChipConverter->activeToOriginal(pixel);
   ossimDpt imagePoint(pixel.mX, pixel.mY);
   ossimGpt worldPoint;
   mModel.lineSampleHeightToWorld(imagePoint, mHeight, worldPoint);
   if (worldPoint.isNan())
   {
      if (pAccurate != NULL)
      {
         *pAccurate = false;
      }
      return LocationType();
   }

   if (pAccurate != NULL)
   {
      *pAccurate = true;
   }
   return LocationType(worldPoint.latd(), worldPoint.lond());
}

LocationType Nitf::RpcGeoreference::geoToPixel(LocationType geo, bool* pAccurate) const
{
   ossimGpt worldPoint;
   worldPoint.latd(geo.mX);
   worldPoint.lond(geo.mY);
   worldPoint.height(mHeight);
   ossimDpt imagePoint;
   mModel.worldToLineSample(worldPoint, imagePoint);
   if (imagePoint.isNan())
   {
      if (pAccurate != NULL)
      {
         *pAccurate = false;
      }
      return LocationType();
   }

   if (pAccurate != NULL)
   {
      *pAccurate = true;
   }
   return mpChipConverter->originalToActive(LocationType(imagePoint.x, imagePoint.y));
}

const DynamicObject* Nitf::RpcGeoreference::getRpcInstance(const RasterDataDescriptor* pDescriptor) const
{
   if (pDescriptor == NULL)
   {
      return NULL;
   }

   const DynamicObject* pMetadata = pDescriptor->getMetadata();
   if (pMetadata != NULL)
   {
      const DynamicObject* pNitfMetadata = pMetadata->getAttribute("NITF").getPointerToValue<DynamicObject>();
      if (pNitfMetadata != NULL)
      {
         const DynamicObject* pTreData = pNitfMetadata->getAttribute("TRE").getPointerToValue<DynamicObject>();
         if (pTreData != NULL)
         {
            vector<string> keys;
            pTreData->getAttributeNames(keys);

            // look for something that matches RPC00?
            for (vector<string>::iterator it = keys.begin(); it != keys.end(); ++it)
            {
               if (it->substr(0, 5) == "RPC00")
               {
                  const DynamicObject* pRpc = getTagHandle<FindFirstValidRpc>(*pNitfMetadata, *it);
                  if (pRpc != NULL)
                  {
                     mRpcVersion = *it;
                     return pRpc;
                  }
               }
            }
         }
      }
   }

   return NULL;
}

unsigned char Nitf::RpcGeoreference::getGeoreferenceAffinity(const RasterDataDescriptor* pDescriptor) const
{
   const DynamicObject* pRpc = getRpcInstance(pDescriptor);
   if (pRpc != NULL)
   {
      if (dv_cast<bool>(pRpc->getAttribute("SUCCESS"), false) == true)
      {
         return Georeference::CAN_GEOREFERENCE + 10;   // Add additional value to favor RPC Georeference
                                                       // over GCP Georeference
      }
   }

   return Georeference::CAN_NOT_GEOREFERENCE;
}

QWidget* Nitf::RpcGeoreference::getWidget(RasterDataDescriptor* pDescriptor)
{
   if (pDescriptor == NULL)
   {
      return NULL;
   }

   // Create the widget
   if (mpGui == NULL)
   {
      mpGui = new RpcGui();
   }

   // Set the georeference data into the widget for the given raster data
   GeoreferenceDescriptor* pGeorefDescriptor = pDescriptor->getGeoreferenceDescriptor();
   if (pGeorefDescriptor != NULL)
   {
      mpGui->setGeoreferenceDescriptor(pGeorefDescriptor);
   }

   return mpGui;
}

bool Nitf::RpcGeoreference::validate(const RasterDataDescriptor* pDescriptor, std::string& errorMessage) const
{
   if (GeoreferenceShell::validate(pDescriptor, errorMessage) == false)
   {
      return false;
   }

   VERIFY(pDescriptor != NULL);

   // Check for valid RPC data
   const DynamicObject* pRpc = getRpcInstance(pDescriptor);
   if (pRpc == NULL)
   {
      errorMessage = "The raster data set does not contain RPC data.";
      return false;
   }

   if (dv_cast<bool>(pRpc->getAttribute("SUCCESS"), false) == false)
   {
      errorMessage = "The RPC data contained in the raster data set is invalid.";
      return false;
   }

   return true;
}

bool Nitf::RpcGeoreference::serialize(SessionItemSerializer& serializer) const
{
   if (mpRaster == NULL)
   {
      // execute() has not yet been called so there is no need to serialize any parameters
      return true;
   }

   string id = mpRaster->getId();
   serializer.serialize(id.c_str(), id.size());
   serializer.endBlock();

   ossimKeywordlist kwl;
   if (!mModel.saveState(kwl))
   {
      return false;
   }
   ossimString str = kwl.toString();
   return serializer.serialize(str.chars(), str.size());
}

bool Nitf::RpcGeoreference::deserialize(SessionItemDeserializer& deserializer)
{
   vector<int64_t> sizes = deserializer.getBlockSizes();
   if (sizes.empty() == true)
   {
      // The plug-in was serialized before execute() was called
      return true;
   }

   if (sizes.size() != 2)
   {
      return false;
   }
   string id(sizes[0], '\0');
   string state(sizes[1], '\0');
   bool success = deserializer.deserialize(&id[0], id.size());
   deserializer.nextBlock();
   success = success && deserializer.deserialize(&state[0], state.size());
   if (!success)
   {
      return false;
   }

   mpRaster = dynamic_cast<RasterElement*>(Service<SessionManager>()->getSessionItem(id));
   if (mpRaster == NULL)
   {
      return false;
   }
   try
   {
      RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(mpRaster->getDataDescriptor());
      if (pDescriptor == NULL)
      {
         return false;
      }

      Nitf::ChipConverter* pConverter = new Nitf::ChipConverter(*pDescriptor);
      mpChipConverter.reset(pConverter);
   }
   catch (const string&)
   {
      return false;
   }

   stringstream str(state);
   ossimKeywordlist kwl;
   if (!kwl.parseStream(str))
   {
      return false;
   }
   return mModel.loadState(kwl);
}
