/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <vector>
#include <string>
#include <sstream>

#include "AppVerify.h"
#include "AppVersion.h"
#include "RasterElement.h"
#include "DataVariant.h"
#include "DynamicObject.h"
#include "MessageLogResource.h"
#include "NitfConstants.h"
#include "NitfUtilities.h"
#include "NitfResource.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "RasterDataDescriptor.h"
#include "RasterFileDescriptor.h"
#include "RpcGeoreference.h"
#include "SessionItemDeserializer.h"
#include "SessionItemSerializer.h"
#include "TypeConverter.h"
#include "UtilityServices.h"

#include <ossim/base/ossimKeywordlist.h>

using namespace Nitf;
using namespace Nitf::TRE;
using namespace Nitf::TRE::RPC;
using namespace std;
XERCES_CPP_NAMESPACE_USE

Nitf::RpcGeoreference::RpcGeoreference() : mpRaster(NULL)
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
}

namespace
{
   struct FindFirstValidRpc
   {
      bool operator()(const DynamicObject& dynObj)
      {
         return (dynObj.getAttribute(getCoefficient(LINE_NUMERATOR_COEF_PREFIX, 1)).getPointerToValue<double>() != NULL);
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

   mpRaster = pInParam->getPlugInArgValue<RasterElement>(DataElementArg());
   Progress *pProgress = pInParam->getPlugInArgValue<Progress>(ProgressArg());
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

   if (pProgress)
   {
      pProgress->updateProgress(messageText, 100, NORMAL);
   }


   /********************************************************************************
   this is the RPC section of the Geocode referencing.  It will try to load the RPC parameters
   out of the metadata, if they don't exist the georeferencing will fail.
   ********************************************************************************/
   bool bInit = false;

   const DynamicObject *pMetadata = mpRaster->getMetadata();

   if (pMetadata == NULL)
   {
      messageText = "Invalid metadata in Raster Element!";
      if(pProgress)
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
      if(pProgress)
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

   const DynamicObject* pRpc = getRpcInstance(mpRaster);
   if (canHandleRasterElement(mpRaster) && pRpc != NULL)
   {
      for (int i = 0; i < NUM_RPC_COEFFICIENTS; i++)
      {
         istringstream vstrm(pRpc->getAttribute(getCoefficient(LINE_NUMERATOR_COEF_PREFIX, i+1)).toXmlString());
         vstrm >> lineRPCnumCoefficients[i];
      }
      for (int i = 0; i < NUM_RPC_COEFFICIENTS; i++)
      {
         istringstream vstrm(pRpc->getAttribute(getCoefficient(LINE_DENOMINATOR_COEF_PREFIX, i+1)).toXmlString());
         vstrm >> lineRPCdenCoefficients[i];
      }
      for (int i = 0; i < NUM_RPC_COEFFICIENTS; i++)
      {
         istringstream vstrm(pRpc->getAttribute(getCoefficient(SAMPLE_NUMERATOR_COEF_PREFIX, i+1)).toXmlString());
         vstrm >> sampRPCnumCoefficients[i];
      }
      for (int i = 0; i < NUM_RPC_COEFFICIENTS; i++)
      {
         istringstream vstrm(pRpc->getAttribute(getCoefficient(SAMPLE_DENOMINATOR_COEF_PREFIX, i+1)).toXmlString());
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
         if(pProgress)
         {
            pProgress->updateProgress(messageText, 0, ERRORS);
         }
         pStep->finalize(Message::Failure, messageText);
         return false;
      }

      RasterDataDescriptor *pDescriptor = dynamic_cast<RasterDataDescriptor*>(mpRaster->getDataDescriptor());
      if (pDescriptor == NULL)
      {
         messageText = "Could not find Raster Data Descriptor";
         if(pProgress)
         {
            pProgress->updateProgress(messageText, 0, ERRORS);
         }
         pStep->finalize(Message::Failure, messageText);
         return false;
      }

      try
      {
         mpChipConverter.reset(new Nitf::ChipConverter(*pDescriptor));
      }
      catch (const string& message)
      {
         messageText = message;
         if(pProgress)
         {
            pProgress->updateProgress(messageText, 0, ERRORS);
         }
         pStep->finalize(Message::Failure, messageText);
         return false;
      }

      vector<double> xNumCoef(NUM_RPC_COEFFICIENTS), xDenCoef(NUM_RPC_COEFFICIENTS),
         yNumCoef(NUM_RPC_COEFFICIENTS), yDenCoef(NUM_RPC_COEFFICIENTS);
      for(int i = 0; i < NUM_RPC_COEFFICIENTS; i++)
      {
         xNumCoef[i] = sampRPCnumCoefficients[i];
         xDenCoef[i] = sampRPCdenCoefficients[i];
         yNumCoef[i] = lineRPCnumCoefficients[i];
         yDenCoef[i] = lineRPCdenCoefficients[i];
      }
      mModel.setAttributes(rpcSampOffset,rpcLineOffset,rpcSampScale,rpcLineScale,
         rpcLatOffset,rpcLongOffset,rpcHeightOffset,rpcLatScale,rpcLongScale,rpcHeightScale,
         xNumCoef, xDenCoef, yNumCoef, yDenCoef, (mRpcVersion == "A") ? ossimRpcModel::A : ossimRpcModel::B);

      bInit = true;
   }
   else
   {
      messageText = "File does not have an RPC TRE.";
      if(pProgress)
      {
         pProgress->updateProgress(messageText, 0, ERRORS);
      }
      pStep->finalize(Message::Failure, messageText);
      return false;
   }

   if (!bInit)
   {
      messageText = "RPC Georeference failed to initialize state for georeferencing!";
      if(pProgress)
      {
         pProgress->updateProgress(messageText, 0, ERRORS);
      }
      pStep->finalize(Message::Failure, messageText);
      return false;
   }

   if (mpRaster != NULL)
   {
      mpRaster->setGeoreferencePlugin(this);
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
   pArgList = NULL;

   if (!isBatch())
   {
      // do not support interactive
      return true;
   }

   // Set up list
   VERIFY((pArgList = Service<PlugInManagerServices>()->getPlugInArgList()) != NULL);
   VERIFY(pArgList->addArg<Progress>(ProgressArg(), NULL));
   VERIFY(pArgList->addArg<RasterElement>(DataElementArg()));

   return true;
}

bool Nitf::RpcGeoreference::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

LocationType Nitf::RpcGeoreference::pixelToGeo(LocationType pixel) const
{
   pixel = mpChipConverter->activeToOriginal(pixel);
   ossimDpt imagePoint(pixel.mX, pixel.mY);
   ossimGpt worldPoint;
   mModel.lineSampleToWorld(imagePoint, worldPoint);
   if(worldPoint.isNan())
   {
      return LocationType();
   }
   return LocationType(worldPoint.latd(), worldPoint.lond());
}

LocationType Nitf::RpcGeoreference::geoToPixel(LocationType geo) const
{
   ossimGpt worldPoint;
   worldPoint.latd(geo.mX);
   worldPoint.lond(geo.mY);
   ossimDpt imagePoint;
   mModel.worldToLineSample(worldPoint, imagePoint);
   if(imagePoint.isNan())
   {
      return LocationType();
   }
   return mpChipConverter->originalToActive(LocationType(imagePoint.x, imagePoint.y));
}

const DynamicObject* Nitf::RpcGeoreference::getRpcInstance(RasterElement *pRaster) const
{
   int i = 0;
   bool found = false;

   if (pRaster != NULL)
   {
      DataDescriptor *pDd = pRaster->getDataDescriptor();
      if (pDd != NULL)
      {
         DynamicObject *pMetadata = pDd->getMetadata();

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
      }
   }
   return NULL;
}

bool Nitf::RpcGeoreference::canHandleRasterElement(RasterElement* pRaster) const
{
   bool ret_val = false;
   if (pRaster != NULL)
   {
      const DynamicObject* pRpc = getRpcInstance(pRaster);
      if (pRpc != NULL)
      {
         try
         {
            ret_val = dv_cast<bool>(pRpc->getAttribute("SUCCESS"));
         }
         catch (bad_cast e)
         {
            // do nothing special
         }
      }
   }
   return ret_val;
}

bool Nitf::RpcGeoreference::hasAbort()
{
   return false;
}

QWidget *Nitf::RpcGeoreference::getGui(RasterElement *pRaster)
{
   return NULL;
}

bool Nitf::RpcGeoreference::serialize(SessionItemSerializer &serializer) const
{
   if (mpRaster == NULL)
   {
      return false;
   }
   string id = mpRaster->getId();
   serializer.serialize(id.c_str(), id.size());
   serializer.endBlock();

   ossimKeywordlist kwl;
   if(!mModel.saveState(kwl))
   {
      return false;
   }
   ossimString str = kwl.toString();
   return serializer.serialize(str.chars(), str.size());
}

bool Nitf::RpcGeoreference::deserialize(SessionItemDeserializer &deserializer)
{
   vector<int64_t> sizes = deserializer.getBlockSizes();
   if(sizes.size() != 2)
   {
      return false;
   }
   string id(sizes[0], '\0');
   string state(sizes[1], '\0');
   bool success = deserializer.deserialize(const_cast<char*>(id.c_str()), id.size());
   deserializer.nextBlock();
   success = success && deserializer.deserialize(const_cast<char*>(state.c_str()), state.size());
   if(!success)
   {
      return false;
   }

   mpRaster = dynamic_cast<RasterElement*>(Service<SessionManager>()->getSessionItem(id));
   if(mpRaster == NULL)
   {
      return false;
   }
   try
   {
      RasterDataDescriptor *pDescriptor = dynamic_cast<RasterDataDescriptor*>(mpRaster->getDataDescriptor());
      if(pDescriptor == NULL)
      {
         return false;
      }
      mpChipConverter.reset(new Nitf::ChipConverter(*pDescriptor));
   }
   catch (const string&)
   {
      return false;
   }

   stringstream str(state);
   ossimKeywordlist kwl;
   if(!kwl.parseStream(str))
   {
      return false;
   }
   return mModel.loadState(kwl);
}
