/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "ColorType.h"
#include "IceThresholdLayerExporter.h"
#include "IceUtilities.h"
#include "IceWriter.h"
#include "PlugInArgList.h"
#include "PlugInRegistration.h"
#include "RasterElement.h"
#include "RasterDataDescriptor.h"
#include "RasterFileDescriptor.h"
#include "RasterUtilities.h"
#include "Statistics.h"
#include "StringUtilities.h"
#include "ThresholdLayer.h"

using namespace std;

REGISTER_PLUGIN_BASIC(OpticksIce, IceThresholdLayerExporter);

IceThresholdLayerExporter::IceThresholdLayerExporter() :
   IceExporterShell(IceUtilities::THRESHOLD_LAYER),
   mpLayer(NULL),
   mpRaster(NULL),
   mFirstThreshold(0.0),
   mSecondThreshold(0.0),
   mRegionUnits(RAW_VALUE),
   mPassArea(UPPER),
   mpOutputDescriptor(NULL)
{
   setName("Ice Threshold Layer Exporter");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setShortDescription("Exports Ice Threshold Layer Files");
   setDescriptorId("{24CF80AC-7A37-4B67-ACA8-54AE4999C47A}");
   setExtensions("Ice Threshold Layers Files (*.thl.ice.h5)");
   setSubtype(TypeConverter::toString<ThresholdLayer>());
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
   setAbortSupported(true);
}

IceThresholdLayerExporter::~IceThresholdLayerExporter()
{}

bool IceThresholdLayerExporter::getInputSpecification(PlugInArgList*& pArgList)
{
   DO_IF(IceExporterShell::getInputSpecification(pArgList) == false, return false);

   if (isBatch() == false)
   {
      VERIFY(pArgList->addArg<ThresholdLayer>(Exporter::ExportItemArg(), "Threshold layer to be exported."));
   }
   else
   {
      VERIFY(pArgList->addArg<RasterElement>(Executable::DataElementArg(), "The element from which a threshold layer "
         "will be exported."));
      VERIFY(pArgList->addArg<double>("First Threshold", "The first threshold value.  The default value is " +
         StringUtilities::toDisplayString(mFirstThreshold) + "."));
      VERIFY(pArgList->addArg<double>("Second Threshold", "The second threshold value. Ignored if \"Pass Area\" is "
         "upper or lower.  The default value is " + StringUtilities::toDisplayString(mSecondThreshold) + "."));
      VERIFY(pArgList->addArg<RegionUnits>("Region Units", mRegionUnits, "The units for \"First Threshold\" and "
         "\"Second Threshold\".  The default value is " + StringUtilities::toDisplayString(mRegionUnits) + "."));
      VERIFY(pArgList->addArg<PassArea>("Pass Area", mPassArea, "The area which will be flagged by default in the "
         "exported data set.  The default value is " + StringUtilities::toDisplayString(mPassArea) + "."));
   }

   VERIFY(pArgList->addArg<FileDescriptor>(Exporter::ExportDescriptorArg(),
      "File descriptor for the exported raster element."));
   return true;
}

void IceThresholdLayerExporter::parseInputArgs(PlugInArgList* pInArgList)
{
   IceExporterShell::parseInputArgs(pInArgList);

   if (isBatch() == false)
   {
      mpLayer = pInArgList->getPlugInArgValue<ThresholdLayer>(Exporter::ExportItemArg());
      ICEVERIFY_MSG(mpLayer != NULL, "No threshold layer to export.");
   }
   else
   {
      mpRaster = pInArgList->getPlugInArgValue<RasterElement>(Executable::DataElementArg());
      ICEVERIFY_MSG(mpRaster != NULL, "No raster element to export.");

      pInArgList->getPlugInArgValue<double>("First Threshold", mFirstThreshold);
      pInArgList->getPlugInArgValue<double>("Second Threshold", mSecondThreshold);
      pInArgList->getPlugInArgValue<RegionUnits>("Region Units", mRegionUnits);
      pInArgList->getPlugInArgValue<PassArea>("Pass Area", mPassArea);
   }

   mpOutputDescriptor = pInArgList->getPlugInArgValue<FileDescriptor>(Exporter::ExportDescriptorArg());
   ICEVERIFY_MSG(mpOutputDescriptor != NULL, "No output file descriptor provided.");
}

void IceThresholdLayerExporter::getOutputCubeAndFileDescriptor(RasterElement*& pOutputCube,
                                                               RasterFileDescriptor*& pOutputFileDescriptor)
{
   if (mpLayer != NULL)
   {
      pOutputCube = dynamic_cast<RasterElement*>(mpLayer->getDataElement());
   }
   else
   {
      pOutputCube = mpRaster;
   }

   ICEVERIFY_MSG(pOutputCube != NULL, "Could not obtain a raster element to export.");

   pOutputFileDescriptor = dynamic_cast<RasterFileDescriptor*>(
      RasterUtilities::generateFileDescriptorForExport(pOutputCube->getDataDescriptor(),
      mpOutputDescriptor->getFilename().getFullPathAndName()));
   ICEVERIFY_MSG(pOutputFileDescriptor != NULL, "Unable to create an output file descriptor.");
}

void IceThresholdLayerExporter::finishWriting(IceWriter& writer)
{
   if (mpLayer != NULL)
   {
      writer.writeLayer("/Layers/ThresholdLayer1", outputCubePath(), mpLayer, mpProgress);
   }
   else if (mpRaster != NULL)
   {
      // layer is NULL so no displayed band specified - just use default of first active band
      unsigned int bandNumber(0);

      if (mRegionUnits != RAW_VALUE)
      {
         const RasterDataDescriptor* pDescriptor =
            dynamic_cast<const RasterDataDescriptor*>(mpRaster->getDataDescriptor());
         ICEVERIFY_MSG(pDescriptor != NULL, "Unable to get the raster data descriptor.");

         Statistics* pStatistics = mpRaster->getStatistics(pDescriptor->getActiveBand(bandNumber));
         ICEVERIFY_MSG(pStatistics != NULL, "Unable to get the raster element statistics.");

         mFirstThreshold = convertThreshold(pStatistics, mRegionUnits, mFirstThreshold);
         mSecondThreshold = convertThreshold(pStatistics, mRegionUnits, mSecondThreshold);
      }

      writer.writeThresholdLayer("/Layers/ThresholdLayer1", outputCubePath(), mpRaster->getName(), 1.0, 1.0, 0.0, 0.0,
         ThresholdLayer::getSettingMarkerSymbol(), ThresholdLayer::getSettingMarkerColor(), mFirstThreshold,
         mSecondThreshold, mRegionUnits, mPassArea, mpProgress, bandNumber);
   }
}

double IceThresholdLayerExporter::convertThreshold(Statistics* pStatistics, RegionUnits regionUnits,
                                                   double value) const
{
   if ((pStatistics == NULL) || (regionUnits == RAW_VALUE))
   {
      return value;
   }

   double minVal = pStatistics->getMin();
   double maxVal = pStatistics->getMax();
   double average = pStatistics->getAverage();
   double stdDev = pStatistics->getStandardDeviation();

   // Convert the threshold value to a raw threshold value
   switch (regionUnits)
   {
   case PERCENTAGE:
      return (((maxVal - minVal) * value) / 100) + minVal;

   case PERCENTILE:
      {
         const double* pPercentiles = pStatistics->getPercentiles();
         if (pPercentiles == NULL)
         {
            return value;
         }

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

   return value;
}
