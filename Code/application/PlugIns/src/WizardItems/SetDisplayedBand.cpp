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
#include "DesktopServices.h"
#include "DimensionDescriptor.h"
#include "MessageLogResource.h"
#include "PlugInArgList.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterLayer.h"
#include "SetDisplayedBand.h"
#include "SpatialDataView.h"

#include <string>

SetDisplayedBand::SetDisplayedBand() :
   mpRasterLayer(NULL),
   mRasterChannelType(GRAY),
   mOriginalNumber(0),
   mpRasterElement(NULL)
{
   setName("Set Displayed Band");
   setVersion(APP_VERSION_NUMBER);
   setCreator("Ball Aerospace & Technologies, Corp.");
   setCopyright(APP_COPYRIGHT);
   setDescription("Sets the Displayed Band for the specified Raster Layer");
   setDescriptorId("{A02B14CE-C594-48ad-ADBF-B5F0FA40C0A0}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

SetDisplayedBand::~SetDisplayedBand()
{
}

bool SetDisplayedBand::getInputSpecification(PlugInArgList*& pArgList)
{
   VERIFY(DesktopItems::getInputSpecification(pArgList) == true);
   VERIFY(pArgList != NULL);
   VERIFY(pArgList->addArg<RasterLayer>(Executable::LayerArg()) == true);
   VERIFY(pArgList->addArg<RasterChannelType>("Channel") == true);
   VERIFY(pArgList->addArg<unsigned int>("Original Band Number") == true);
   VERIFY(pArgList->addArg<RasterElement>(Executable::DataElementArg()) == true);
   return true;
}

bool SetDisplayedBand::extractInputArgs(PlugInArgList* pInArgList)
{
   if (pInArgList == NULL || DesktopItems::extractInputArgs(pInArgList) == false)
   {
      return false;
   }

   mpRasterLayer = pInArgList->getPlugInArgValue<RasterLayer>(Executable::LayerArg());
   if (mpRasterLayer == NULL)
   {
      reportError("Invalid " + Executable::LayerArg() + " specified.", "A91F331A-CAE6-4ef6-AD66-4AD8CB6A61C5");
      return false;
   }

   if (pInArgList->getPlugInArgValue("Channel", mRasterChannelType) == false)
   {
      reportError("Invalid Channel specified.", "720914CD-A86B-437e-A408-60E7D2824C97");
      return false;
   }

   if (pInArgList->getPlugInArgValue("Original Band Number", mOriginalNumber) == false || mOriginalNumber == 0)
   {
      reportError("Invalid Original Band Number specified.", "3239B7B4-4EED-429e-975B-0410BCE7F669");
      return false;
   }

   mpRasterElement = pInArgList->getPlugInArgValue<RasterElement>(Executable::DataElementArg());
   if (mpRasterElement == NULL)
   {
      mpRasterElement = dynamic_cast<RasterElement*>(mpRasterLayer->getDataElement());
      if (mpRasterElement == NULL)
      {
         reportError("Invalid " + Executable::DataElementArg() + " specified.", "2F4B694A-3B79-4375-95CA-60A3EA7E65E2");
         return false;
      }
   }

   return true;
}

bool SetDisplayedBand::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Execute Wizard Item", "app", "460AE5E1-4C7B-43f7-8929-4F0179383F50");
   pStep->addProperty("Item", getName());
   mpStep = pStep.get();

   if (extractInputArgs(pInArgList) == false)
   {
      return false;
   }

   RasterDataDescriptor* pDescriptor =
      dynamic_cast<RasterDataDescriptor*>(mpRasterElement->getDataDescriptor());
   VERIFY(pDescriptor != NULL);

   DimensionDescriptor band = pDescriptor->getOriginalBand(mOriginalNumber - 1);
   if (band.isValid() == false)
   {
      reportError("The specified band is invalid.", "37E09A70-8B68-453d-AE6F-590D95CC4E96");
      return false;
   }

   mpRasterLayer->setDisplayedBand(mRasterChannelType, band, mpRasterElement);
   reportComplete();
   return true;
}
