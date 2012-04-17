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
#include "FileDescriptor.h"
#include "Kml.h"
#include "KmlExporter.h"
#include "Layer.h"
#include "LayerList.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "ProgressTracker.h"
#include "RasterElement.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include <QtCore/QDateTime>

REGISTER_PLUGIN_BASIC(OpticksKml, KmlExporter);

KmlExporter::KmlExporter()
{
   setName("KML Exporter");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setExtensions("KML Files (*.kmz *.kml)");
   setSubtype(TypeConverter::toString<SpatialDataView>());
   setDescriptorId("{AF865E04-6931-4BF3-9E16-966FE3DB953B}");
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

KmlExporter::~KmlExporter()
{
}

bool KmlExporter::getInputSpecification(PlugInArgList*& pArgList)
{
   VERIFY((pArgList = Service<PlugInManagerServices>()->getPlugInArgList()) != NULL);
   VERIFY(pArgList->addArg<Progress>(Executable::ProgressArg(), Executable::ProgressArgDescription()));
   VERIFY(pArgList->addArg<SpatialDataView>(Exporter::ExportItemArg(), NULL, "View to be exported."));
   VERIFY(pArgList->addArg<FileDescriptor>(Exporter::ExportDescriptorArg(), "File descriptor for the output file."));
   return true;
}

bool KmlExporter::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

bool KmlExporter::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   ProgressTracker progress(pInArgList->getPlugInArgValue<Progress>(Executable::ProgressArg()), "Export KML", "app",
      "447D4133-FD36-4D00-8078-D104F2D8929D");

   FileDescriptor* pDescriptor = pInArgList->getPlugInArgValue<FileDescriptor>(Exporter::ExportDescriptorArg());
   if (pDescriptor == NULL || pDescriptor->getFilename().getFullPathAndName().empty())
   {
      progress.report("No output file specified.", 0, ERRORS, true);
      return false;
   }
   // Try and get a view and a layer...the basic exporter works on views but subclasses may work on layers.
   SpatialDataView* pView = pInArgList->getPlugInArgValue<SpatialDataView>(Exporter::ExportItemArg());
   Layer* pLayer = pInArgList->getPlugInArgValue<Layer>(Exporter::ExportItemArg());
   bool isKmz(pDescriptor->getFilename().getExtension() == "kmz");
   Kml kml(isKmz, progress.getCurrentProgress());
   bool success = false;
   if (pView == NULL && pLayer == NULL)
   {
      progress.report("Exporting Session", 10, NORMAL);
      success = kml.addSession();
   }
   else if (pView != NULL)
   {
      progress.report("Exporting View", 10, NORMAL);
      success = kml.addView(pView);
   }
   else if (pLayer != NULL)
   {
      progress.report("Exporting Layer", 10, NORMAL);
      pView = dynamic_cast<SpatialDataView*>(pLayer->getView());
      if (pView == NULL)
      {
         progress.report("Layer is not displayed.", 0, ERRORS, true);
         return false;
      }
      RasterElement* pElement = pView->getLayerList()->getPrimaryRasterElement();
      if (pElement == NULL || !pElement->isGeoreferenced())
      {
         progress.report("Primary raster layer is not geo-referenced.", 0, ERRORS, true);
         return false;
      }
      Layer* pPrimaryLayer = pView->getLayerList()->getLayer(RASTER, pElement);
      success = kml.addLayer(pLayer, pPrimaryLayer, pView, pView->getLayerList()->getNumLayers());
   }
   if (!success)
   {
      progress.report("Failed to export.", 0, ERRORS, true);
      return false;
   }
   if (!kml.toFile(pDescriptor->getFilename().getFullPathAndName()))
   {
      progress.report("KML Export Failed", 0, ERRORS, true);
      progress.upALevel();
      return false;
   }
   progress.report("KML Export Complete", 100, NORMAL);
   progress.upALevel();
   return true;
}

REGISTER_PLUGIN_BASIC(OpticksKml, KmlLayerExporter);

KmlLayerExporter::KmlLayerExporter()
{
   setName("KML Layer Exporter");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setExtensions("KML Files (*.kmz *.kml)");
   setSubtype(TypeConverter::toString<Layer>());
   setDescriptorId("{A5E6EDDE-18FB-42C2-9455-3481B5F23CF6}");
}

KmlLayerExporter::~KmlLayerExporter()
{
}

bool KmlLayerExporter::getInputSpecification(PlugInArgList*& pArgList)
{
   VERIFY((pArgList = Service<PlugInManagerServices>()->getPlugInArgList()) != NULL);
   VERIFY(pArgList->addArg<Progress>(Executable::ProgressArg(), Executable::ProgressArgDescription()));
   VERIFY(pArgList->addArg<Layer>(Exporter::ExportItemArg(), NULL, "Layer to be exported."));
   VERIFY(pArgList->addArg<FileDescriptor>(Exporter::ExportDescriptorArg(), "File descriptor for the output file."));
   return true;
}
