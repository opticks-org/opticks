/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "AppVerify.h"
#include "MessageLogResource.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "RasterDataDescriptor.h"
#include "SetThresholdOptions.h"
#include "ThresholdLayer.h"

REGISTER_PLUGIN_BASIC(OpticksWizardItems, SetThresholdOptions);

SetThresholdOptions::SetThresholdOptions() : mFirstThreshold(0.0), mSecondThreshold(0.0), mHasFirst(false),
   mHasSecond(false)
{
   setName("Set Threshold Options");
   setVersion(APP_VERSION_NUMBER);
   setCreator("Ball Aerospace & Technologies, Corp.");
   setCopyright(APP_COPYRIGHT);
   setDescription("Set threshold layer parameters.");
   setDescriptorId("{1128b639-0b83-4473-bce3-4bcb8ce30ed1}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

SetThresholdOptions::~SetThresholdOptions()
{}

bool SetThresholdOptions::setBatch()
{
   DesktopItems::setBatch();
   return true;
}

bool SetThresholdOptions::getInputSpecification(PlugInArgList*& pArgList)
{
   VERIFY(DesktopItems::getInputSpecification(pArgList) && pArgList != NULL);
   VERIFY(pArgList->addArg<ThresholdLayer>(Executable::LayerArg(), "The threshold layer for which to set its "
      "properties."));
   VERIFY(pArgList->addArg<double>("First Threshold", "The first threshold value is used as the sole threshold "
      "value for the lower and upper pass areas. It is also used as the lower threshold value for the middle and "
      "outside pass areas.  If no region units are provided, the threshold value is assumed to be a raw value."));
   VERIFY(pArgList->addArg<double>("Second Threshold", "The second threshold value is used as the upper threshold "
      "value for the middle and outside pass areas. It is ignored for the lower and upper pass areas.  If no "
      "region units are provided, the threshold value is assumed to be a raw value."));
   VERIFY(pArgList->addArg<PassArea>("Pass Area", "The region based on the threshold values in which data will be "
      "considered to pass the threshold."));
   VERIFY(pArgList->addArg<RegionUnits>("Region Units", "The units of the threshold values."));
   VERIFY(pArgList->addArg<SymbolType>("Symbol", "The symbol that is used to mark the pixels that pass the "
      "threshold value(s)."));
   VERIFY(pArgList->addArg<ColorType>("Color", "The color that is used to draw the pixel symbols."));
   unsigned int defaultBand(0);
   VERIFY(pArgList->addArg<unsigned int>("Display Band", defaultBand, "The original band number to be displayed. "
      "This is a one-based index. If no value is provided, the first active band will be displayed."));

   return true;
}

bool SetThresholdOptions::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

bool SetThresholdOptions::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Execute Wizard Item", "app", "DCBBB270-9360-4c96-8CE9-A9D414FC68EE");
   pStep->addProperty("Item", getName());
   mpStep = pStep.get();

   if (!extractInputArgs(pInArgList))
   {
      return false;
   }

   if (mpLayer == NULL)
   {
      reportError("Invalid layer specified.", "{82023d69-a504-45ed-b91b-335d7c8a634f}");
      return false;
   }
   
   // set displayed band
   const RasterDataDescriptor* pDesc =
      dynamic_cast<const RasterDataDescriptor*>(mpLayer->getDataElement()->getDataDescriptor());
   if (pDesc != NULL)
   {
      DimensionDescriptor band;
      if (mDisplayBandNumber > 0)
      {
         band = pDesc->getOriginalBand(mDisplayBandNumber - 1);
         if (band.isValid() == false)
         {
            reportError("The specified band is invalid.", "{456aae8d-46ed-4080-8771-abb28fbd2ab1}");
            return false;
         }
      }
      else
      {
         band = pDesc->getActiveBand(mDisplayBandNumber);
      }
      mpLayer->setDisplayedBand(band);
   }

   if (mHasFirst)
   {
      double threshold = mFirstThreshold;
      if (mRegionUnits.isValid())
      {
         if (mRegionUnits != RAW_VALUE)
         {
            threshold = mpLayer->convertThreshold(mRegionUnits, mFirstThreshold, RAW_VALUE);
         }
      }
      else
      {
         reportWarning("The region units were not provided, so the threshold value will be set into "
            "the layer as raw value units.", "2B4E6C81-0AA7-4036-93AF-4F8397EA8D96");
      }

      mpLayer->setFirstThreshold(threshold);
   }
   if (mHasSecond)
   {
      double threshold = mSecondThreshold;
      if (mRegionUnits.isValid())
      {
         if (mRegionUnits != RAW_VALUE)
         {
            threshold = mpLayer->convertThreshold(mRegionUnits, mSecondThreshold, RAW_VALUE);
         }
      }
      else
      {
         reportWarning("The region units were not provided, so the threshold value will be set into "
            "the layer as raw value units.", "9423280D-260E-476F-B1EB-3472377ED284");
      }

      mpLayer->setSecondThreshold(threshold);
   }
   if (mPassArea.isValid())
   {
      mpLayer->setPassArea(mPassArea);
   }
   if (mRegionUnits.isValid())
   {
      mpLayer->setRegionUnits(mRegionUnits);
   }
   if (mSymbol.isValid())
   {
      mpLayer->setSymbol(mSymbol);
   }
   if (mColor.isValid())
   {
      mpLayer->setColor(mColor);
   }

   reportComplete();
   return true;
}

bool SetThresholdOptions::extractInputArgs(PlugInArgList* pInArgList)
{
   if (!DesktopItems::extractInputArgs(pInArgList))
   {
      reportError("Unable to extract input arguments.", "3F4C3D60-C1F3-4ff2-AF9C-F67F23D25037");
      return false;
   }

   mpLayer = pInArgList->getPlugInArgValue<ThresholdLayer>(Executable::LayerArg());
   mHasFirst = pInArgList->getPlugInArgValue("First Threshold", mFirstThreshold);
   mHasSecond = pInArgList->getPlugInArgValue("Second Threshold", mSecondThreshold);
   pInArgList->getPlugInArgValue("Pass Area", mPassArea);
   pInArgList->getPlugInArgValue("Region Units", mRegionUnits);
   pInArgList->getPlugInArgValue("Symbol", mSymbol);
   pInArgList->getPlugInArgValue("Color", mColor);
   pInArgList->getPlugInArgValue("Display Band", mDisplayBandNumber);

   return true;
}
