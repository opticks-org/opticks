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
#include "MessageLogResource.h"
#include "PlugInArgList.h"
#include "PlugInRegistration.h"
#include "RasterLayer.h"
#include "SetDisplayMode.h"
#include "SpatialDataView.h"

#include <string>

REGISTER_PLUGIN_BASIC(OpticksWizardItems, SetDisplayMode);

SetDisplayMode::SetDisplayMode() :
   mpRasterLayer(NULL),
   mDisplayMode(GRAYSCALE_MODE)
{
   setName("Set Display Mode");
   setVersion(APP_VERSION_NUMBER);
   setCreator("Ball Aerospace & Technologies, Corp.");
   setCopyright(APP_COPYRIGHT);
   setDescription("Sets the Display Mode for the specified Raster Layer");
   setDescriptorId("{C0CF874D-5DA8-479f-94E9-86B3B10D0559}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

SetDisplayMode::~SetDisplayMode()
{
}

bool SetDisplayMode::getInputSpecification(PlugInArgList*& pArgList)
{
   VERIFY(DesktopItems::getInputSpecification(pArgList) == true);
   VERIFY(pArgList != NULL);
   VERIFY(pArgList->addArg<RasterLayer>(Executable::LayerArg()) == true);
   VERIFY(pArgList->addArg<DisplayMode>("Display Mode") == true);
   return true;
}

bool SetDisplayMode::extractInputArgs(PlugInArgList* pInArgList)
{
   if (pInArgList == NULL || DesktopItems::extractInputArgs(pInArgList) == false)
   {
      return false;
   }

   mpRasterLayer = pInArgList->getPlugInArgValue<RasterLayer>(Executable::LayerArg());
   if (mpRasterLayer == NULL)
   {
      reportError("Invalid " + Executable::LayerArg() + " specified.", "8637A7B9-E423-4ab1-ADE3-8511F657112D");
      return false;
   }

   if (pInArgList->getPlugInArgValue("Display Mode", mDisplayMode) == false)
   {
      reportError("Invalid Display Mode specified.", "896DC4DD-4816-4be0-BC01-FCB51FCF2FA2");
      return false;
   }

   return true;
}

bool SetDisplayMode::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Execute Wizard Item", "app", "0B137CEF-B7AF-4999-95AC-2873C39279E5");
   pStep->addProperty("Item", getName());
   mpStep = pStep.get();

   if (extractInputArgs(pInArgList) == false)
   {
      return false;
   }

   mpRasterLayer->setDisplayMode(mDisplayMode);
   reportComplete();
   return true;
}
