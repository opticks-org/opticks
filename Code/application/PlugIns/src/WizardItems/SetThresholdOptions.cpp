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
#include "SetThresholdOptions.h"
#include "ThresholdLayer.h"

SetThresholdOptions::SetThresholdOptions() : mFirstThreshold(0.0), mSecondThreshold(0.0), mHasFirst(false), mHasSecond(false)
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
{
}

bool SetThresholdOptions::setBatch()
{
   DesktopItems::setBatch();
   return true;
}

bool SetThresholdOptions::getInputSpecification(PlugInArgList*& pArgList)
{
   VERIFY(DesktopItems::getInputSpecification(pArgList) && pArgList != NULL);
   VERIFY(pArgList->addArg<ThresholdLayer>(Executable::LayerArg()));
   VERIFY(pArgList->addArg<double>("First Threshold"));
   VERIFY(pArgList->addArg<double>("Second Threshold"));
   VERIFY(pArgList->addArg<PassArea>("Pass Area", PassArea()));
   VERIFY(pArgList->addArg<RegionUnits>("Region Units", RegionUnits()));
   VERIFY(pArgList->addArg<SymbolType>("Symbol", SymbolType()));
   VERIFY(pArgList->addArg<ColorType>("Color", ColorType()));

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

   if (mHasFirst)
   {
      mpLayer->setFirstThreshold(mFirstThreshold);
   }
   if (mHasSecond)
   {
      mpLayer->setSecondThreshold(mSecondThreshold);
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
   if (!pInArgList->getPlugInArgValue("Pass Area", mPassArea))
   {
      reportError("Invalid pass area.", "{f2128d5e-6de5-493d-a959-adcbca640785}");
      return false;
   }
   if (!pInArgList->getPlugInArgValue("Region Units", mRegionUnits))
   {
      reportError("Invalid region units.", "{255b1442-bfcf-4eca-a778-1233077d19e6}");
      return false;
   }
   if (!pInArgList->getPlugInArgValue("Symbol", mSymbol))
   {
      reportError("Invalid symbol.", "{56f2762f-49fc-48f0-ad99-3c324e8839d6}");
      return false;
   }
   if (!pInArgList->getPlugInArgValue("Color", mColor))
   {
      reportError("Invalid color.", "{63cf1c3a-27b5-4c77-b5ed-d090a6251d55}");
      return false;
   }

   return true;
}
