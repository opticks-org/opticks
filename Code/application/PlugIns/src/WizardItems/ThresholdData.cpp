/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AoiElement.h"
#include "AoiLayer.h"
#include "AppVersion.h"
#include "AppVerify.h"
#include "BitMask.h"
#include "DataAccessor.h"
#include "DataAccessorImpl.h"
#include "MessageLogResource.h"
#include "ModelServices.h"
#include "ObjectResource.h"
#include "PlugInArgList.h"
#include "PlugInRegistration.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "SpatialDataView.h"
#include "Statistics.h"
#include "ThresholdData.h"

REGISTER_PLUGIN_BASIC(OpticksWizardItems, ThresholdData);

namespace
{
double convertToRawUnits(Statistics* pStatistics, const RegionUnits& units, double value)
{
   VERIFYRV(pStatistics, 0.0);
   if (units == RAW_VALUE)
   {
      return value;
   }
   double minVal = pStatistics->getMin();
   double maxVal = pStatistics->getMax();
   double average = pStatistics->getAverage();
   double stdDev = pStatistics->getStandardDeviation();

   // Convert the threshold value to a raw threshold value
   switch (units)
   {
   case PERCENTAGE:
      return (((maxVal - minVal) * value) / 100) + minVal;
   case PERCENTILE:
      {
         const double* pPercentiles = pStatistics->getPercentiles();
         VERIFYRV(pPercentiles, 0.0);
         if (value < 0.0 || value > 100.0)
         {
            return pPercentiles[0] + value * (pPercentiles[1000] - pPercentiles[0]) / 100.0;
         }
         int lower = static_cast<int>(10.0 * value);
         if (lower < 0)
         {
            return pPercentiles[0];
         }
         else if (lower > 999)
         {
            return pPercentiles[1000];
         }
         return pPercentiles[lower] + (pPercentiles[lower + 1] - pPercentiles[lower]) *
            (10.0 * value - static_cast<double>(lower));
      }
   case STD_DEV:
      return (value * stdDev) + average;
   default:
      break;
   }
   return 0.0;
}
}

ThresholdData::ThresholdData() :
      mpInputElement(NULL),
      mFirstThreshold(0.0),
      mSecondThreshold(0.0)
{
   setName("Threshold Data");
   setVersion(APP_VERSION_NUMBER);
   setCreator("Ball Aerospace & Technologies, Corp.");
   setCopyright(APP_COPYRIGHT);
   setDescription("Perform a threshold operation and generate an AOI result.\n"
                  "If a threshold layer is available, you can use \"Derive Layer\" instead.");
   setDescriptorId("{017563f2-2d43-4078-8f79-024e9f591d70}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

ThresholdData::~ThresholdData()
{
}

bool ThresholdData::setBatch()
{
   DesktopItems::setBatch();
   return true;
}

bool ThresholdData::getInputSpecification(PlugInArgList*& pArgList)
{
   VERIFY(DesktopItems::getInputSpecification(pArgList) && (pArgList != NULL));
   VERIFY(pArgList->addArg<RasterElement>(Executable::DataElementArg(), "The element which will be thresholded."));
   VERIFY(pArgList->addArg<SpatialDataView>(Executable::ViewArg(),
      "If this optional view is specified, an AOI layer will be created and added to the view."));
   VERIFY(pArgList->addArg<double>("First Threshold", "The first threshold value."));
   VERIFY(pArgList->addArg<double>("Second Threshold",
      "The second threshold value. Ignored if \"Pass Area\" is upper or lower."));
   RegionUnits defaultRegionUnits(RAW_VALUE);
   VERIFY(pArgList->addArg<RegionUnits>("Region Units", defaultRegionUnits,
      "The units for \"First Threshold\" and \"Second Threshold\"."));
   PassArea defaultPassArea(UPPER);
   VERIFY(pArgList->addArg<PassArea>("Pass Area", defaultPassArea, "The area which will be set in the output AOI."));

   return true;
}

bool ThresholdData::getOutputSpecification(PlugInArgList*& pArgList)
{
   VERIFY(pArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   VERIFY(pArgList->addArg<AoiElement>("Result", "The new AOI."));
   VERIFY(pArgList->addArg<AoiLayer>("Result Layer", "The new AOI layer."));
   return true;
}

bool ThresholdData::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   VERIFY(pInArgList != NULL);
   StepResource pStep("Execute Wizard Item", "app", "{2501975d-7cd5-49b0-a3e7-49f7106793c0}");
   pStep->addProperty("Item", getName());
   mpStep = pStep.get();

   if (!extractInputArgs(pInArgList))
   {
      return false;
   }

   DataAccessor acc = mpInputElement->getDataAccessor();
   if (!acc.isValid())
   {
      reportError("Unable to access data element.", "{b5f1b7dd-7cf7-4cd5-b5bc-7b747d3561b9}");
      return false;
   }
   const RasterDataDescriptor* pDesc = static_cast<const RasterDataDescriptor*>(mpInputElement->getDataDescriptor());
   VERIFY(pDesc);
   // If necessary, convert region units
   if (mRegionUnits != RAW_VALUE)
   {
      Statistics* pStatistics = mpInputElement->getStatistics(pDesc->getActiveBand(0));
      if (pStatistics == NULL)
      {
         reportError("Unable to calculate data statistics.", "{61a44ced-a4aa-4423-b379-5783137eb980}");
         return false;
      }
      mFirstThreshold = convertToRawUnits(pStatistics, mRegionUnits, mFirstThreshold);
      mSecondThreshold = convertToRawUnits(pStatistics, mRegionUnits, mSecondThreshold);
   }
   FactoryResource<BitMask> pBitmask;
   for (unsigned int row = 0; row < pDesc->getRowCount(); ++row)
   {
      reportProgress("Thresholding data", 100 * row / pDesc->getRowCount(),
         "{2fc3dbea-1307-471c-bba2-bf86032be518}");
      for (unsigned int col = 0; col < pDesc->getColumnCount(); ++col)
      {
         VERIFY(acc.isValid());
         double val = ModelServices::getDataValue(pDesc->getDataType(), acc->getColumn(), 0);
         switch (mPassArea)
         {
         case UPPER:
            if (val >= mFirstThreshold)
            {
               pBitmask->setPixel(col, row, true);
            }
            break;
         case LOWER:
            if (val <= mFirstThreshold)
            {
               pBitmask->setPixel(col, row, true);
            }
            break;
         case MIDDLE:
            if (val >= mFirstThreshold && val <= mSecondThreshold)
            {
               pBitmask->setPixel(col, row, true);
            }
            break;
         case OUTSIDE:
            if (val <= mFirstThreshold || val >= mSecondThreshold)
            {
               pBitmask->setPixel(col, row, true);
            }
            break;
         default:
            reportError("Unknown or invalid pass area.", "{19c92b3b-52e9-442b-a01f-b545f819f200}");
            return false;
         }
         acc->nextColumn();
      }
      acc->nextRow();
   }
   std::string aoiName = pDesc->getName() + "_aoi";
   ModelResource<AoiElement> pAoi(aoiName, mpInputElement);
   if (pAoi.get() == NULL)
   {
      reportWarning("Overwriting existing AOI.", "{d953a030-dd63-43a1-98db-b0f491dee123}");
      Service<ModelServices>()->destroyElement(
         Service<ModelServices>()->getElement(aoiName, TypeConverter::toString<AoiElement>(), mpInputElement));
      pAoi = ModelResource<AoiElement>(aoiName, mpInputElement);
   }
   if (pAoi.get() == NULL)
   {
      reportError("Unable to create output AOI.", "{f76c2f4d-9a7f-4055-9383-022116cdcadb}");
      return false;
   }
   pAoi->addPoints(pBitmask.get());
   AoiLayer* pLayer = NULL;
   if (mpView != NULL)
   {
      if ((pLayer = static_cast<AoiLayer*>(mpView->createLayer(AOI_LAYER, pAoi.get()))) == NULL)
      {
         reportWarning("Unable to create AOI layer, continuing thresholding.",
            "{5eca6ea0-33c1-4b1a-b777-c8e1b86fd2fb}");
      }
   }
   if (pOutArgList != NULL)
   {
      pOutArgList->setPlugInArgValue("Result", pAoi.get());
      if (pLayer != NULL)
      {
         pOutArgList->setPlugInArgValue("Result Layer", pLayer);
      }
   }
   pAoi.release();

   reportComplete();
   return true;
}

bool ThresholdData::extractInputArgs(PlugInArgList* pInArgList)
{
   if (!DesktopItems::extractInputArgs(pInArgList))
   {
      return false;
   }
   if ((mpInputElement = pInArgList->getPlugInArgValue<RasterElement>(Executable::DataElementArg())) == NULL)
   {
      reportError("Invalid input element.", "{075b6787-8abe-4d74-b621-8392f56d4117}");
      return false;
   }
   mpView = pInArgList->getPlugInArgValue<SpatialDataView>(Executable::ViewArg());
   if (!pInArgList->getPlugInArgValue("Pass Area", mPassArea) || !mPassArea.isValid())
   {
      reportError("Invalid pass area.", "{56519d59-5004-4afe-9875-7cd4d05b0377}");
      return false;
   }
   if (!pInArgList->getPlugInArgValue("Region Units", mRegionUnits) || !mRegionUnits.isValid())
   {
      reportError("Invalid region units.", "{23d58876-f2c3-4b7d-bc1d-63bd4cae26e9}");
      return false;
   }
   if (!pInArgList->getPlugInArgValue("First Threshold", mFirstThreshold))
   {
      reportError("Invalid first threshold.", "{5c173244-551b-42ae-a2c2-a6b4d4ec0f96}");
      return false;
   }
   if (mPassArea != UPPER && mPassArea != LOWER)
   {
      if (!pInArgList->getPlugInArgValue("Second Threshold", mSecondThreshold))
      {
         reportError("Invalid second threshold.", "{a7c293c7-b94e-4924-a531-9afab8a2d7f3}");
         return false;
      }
   }

   return true;
}
