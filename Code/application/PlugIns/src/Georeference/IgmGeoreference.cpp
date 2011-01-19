/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "AppVerify.h"
#include "DesktopServices.h"
#include "DynamicObject.h"
#include "IgmGeoreference.h"
#include "IgmGui.h"
#include "Importer.h"
#include "GcpList.h"
#include "GeoPoint.h"
#include "Layer.h"
#include "LayerList.h"
#include "MatrixFunctions.h"
#include "MessageLogResource.h"
#include "ModelServices.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInRegistration.h"
#include "ProgressTracker.h"
#include "PlugInResource.h"
#include "RasterElement.h"
#include "RasterDataDescriptor.h"
#include "RasterUtilities.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "SessionItemDeserializer.h"
#include "SessionItemSerializer.h"
#include "Statistics.h"
#include "XercesIncludes.h"
#include "xmlreader.h"
#include "xmlwriter.h"

#include <list>
#include <sstream>
#include <boost/tuple/tuple.hpp>

XERCES_CPP_NAMESPACE_USE

REGISTER_PLUGIN_BASIC(OpticksGeoreference, IgmGeoreference);

IgmGeoreference::IgmGeoreference() :
   mpGui(NULL),
   mpRaster(NULL),
   mpGcpPlugIn(NULL),
   mpIgmRaster(NULL)
{
   setName("IGM Georeference");
   setVersion(APP_VERSION_NUMBER);
   setCreator("Ball Aerospace & Technologies, Corp.");
   setCopyright(APP_COPYRIGHT);
   setDescription("IGM Georeferencing plugin");
   setDescriptorId("{EE7B8EB9-8493-4af4-A594-A41FEC3DC4CC}");
   destroyAfterExecute(false);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

IgmGeoreference::~IgmGeoreference()
{
}

bool IgmGeoreference::setInteractive()
{
   ExecutableShell::setInteractive();
   return false;
}

bool IgmGeoreference::getInputSpecification(PlugInArgList*& pArgList)
{
   bool success = GeoreferenceShell::getInputSpecification(pArgList);
   return success;
}

bool IgmGeoreference::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   VERIFY(pInArgList != NULL);

   ProgressTracker progress(pInArgList->getPlugInArgValue<Progress>(Executable::ProgressArg()),
      "Georeferencing IGM...", "app", "{53992A0E-6D3A-40cc-9F59-058F21A497DB}");

   // Get RasterElement input.
   mpRaster = pInArgList->getPlugInArgValue<RasterElement>(Executable::DataElementArg());
   if (mpRaster == NULL)
   {
      progress.report("No raster element specified", 0, ERRORS, true);
      return false;
   }

   std::string geoName = "IGM_GEO";

   if (mpGui != NULL && mpGui->getFilename().isEmpty() == false || mpRaster->isGeoreferenced())
   {
      ImporterResource importer("Auto Importer", mpGui->getFilename().toStdString());
      if (importer->execute() == false)
      {
         progress.report("File type not recognized as IGM", 0, ERRORS, true);
         return false;
      }
      std::vector<DataElement*> igmDataElement = importer->getImportedElements();
      if (igmDataElement.empty() == true)
      {
         progress.report("No data was imported, file type may be invalid", 0, ERRORS, true);
         return false;
      }

      mpIgmGeo.reset(dynamic_cast<RasterElement*>(igmDataElement.front()));
      if (mpIgmGeo.get() == NULL)
      {
         progress.report("Not a valid IGM file", 0, ERRORS, true);
         return false;
      }
      Service<ModelServices>()->destroyElement(Service<ModelServices>()->getElement(geoName, "RasterElement", mpRaster));
      Service<ModelServices>()->setElementName(mpIgmGeo.get(), geoName);
      Service<ModelServices>()->setElementParent(mpIgmGeo.get(), mpRaster);
   }

   RasterDataDescriptor* pMainDesc = dynamic_cast<RasterDataDescriptor*>(mpRaster->getDataDescriptor());
   VERIFY(pMainDesc != NULL);
   mLoadedRows = pMainDesc->getRows();
   mLoadedColumns = pMainDesc->getColumns();
   mLoadedBands = pMainDesc->getBands();

   mpIgmRaster.reset(dynamic_cast<RasterElement*>(Service<ModelServices>()->getElement(
      geoName, TypeConverter::toString<RasterElement>(), mpRaster)));
   mpIgmRaster.addSignal(SIGNAL_NAME(Subject, Deleted), Slot(this, &IgmGeoreference::elementDeleted));
   if (mpIgmRaster.get() == NULL)
   {
      progress.report("No IGM specified", 0, ERRORS, true);
      return false;
   }

   RasterDataDescriptor* pLatLonDesc = dynamic_cast<RasterDataDescriptor*>(mpIgmRaster->getDataDescriptor());
   VERIFY(pLatLonDesc != NULL);
   if (pLatLonDesc->getRowCount() != pMainDesc->getRowCount() ||
      pLatLonDesc->getColumnCount() != pMainDesc->getColumnCount())
   {
      progress.report("IGM sizes do not match", 0, ERRORS, true);
      return false;
   }

   EncodingType latLonType = pLatLonDesc->getDataType();
   if (latLonType != FLT4BYTES && latLonType != FLT8BYTES)
   {
      progress.report("IGM types must be floating point", 0, ERRORS, true);
      return false;
   }

   const DynamicObject* pMetadata = mpIgmRaster->getMetadata();
   mZone = 100; // Sentinel value for not valid
   if (pMetadata != NULL)
   {
      QString sZone;
      std::string stdZone;
      DataVariant range = pMetadata->getAttribute("zone");
      bool success = range.getValue(stdZone);
      if (success)
      {
         mZone = sZone.fromStdString(stdZone).toInt();
      }
   }
   std::string gcpListName = "Corner Coordinates";
   ModelResource<GcpList> pGcpList(dynamic_cast<GcpList*>(Service<ModelServices>()->createElement(gcpListName,
      TypeConverter::toString<GcpList>(), mpIgmRaster.get())));

   if (pGcpList.get() == NULL)
   {
      progress.report("Unable to create GCP List", 0, ERRORS, true);
      return false;
   }
   std::list<GcpPoint> gcpList;
   const unsigned int maxX = mLoadedColumns.size() - 1;
   const unsigned int maxY = mLoadedRows.size() - 1;
   unsigned int skipX = maxX / 4;   // Appropriate skip factor for a well determined 2 order system.
   unsigned int skipY = maxY / 4;   // Appropriate skip factor for a well determined 2 order system.
   for (unsigned int row = 0; row <= maxY; row += skipY)
   {
      // Read one row at a time.
      for (unsigned int col = 0; col <= maxX; col += skipX)
      {
         // Note that the location of the pixel coordinates and geocoordinates has been reversed.
         // This is to ensure that when the reverse method, IgmGeoreference::geoToPixel(),
         // is called that GcpGeoreference::pixelToGeo() returns the appropriate results.
         GcpPoint pt;
         bool geoOk = false;

         pt.mCoordinate = LocationType(col, row);
         pt.mPixel  = pixelToGeo(pt.mCoordinate, &geoOk);
         gcpList.push_back(pt);
      }
   }

   pGcpList->addPoints(gcpList);

   // Reset the GCP Georeference plug-in, and execute GCP Georeference on mpIgmRaster.
   // If the operation succeeds, keep a pointer to the GCP Georeference plug-in for use later.
   ExecutableResource pGeoreference = ExecutableResource("GCP Georeference", "", progress.getCurrentProgress());
   if (pGeoreference.get() == NULL)
   {
      progress.report("Unable to create GCP Georeference plug-in", 0, ERRORS, true);
      return false;
   }

   int order = 2;
   mpGcpPlugIn = NULL;
   PlugInArgList& gcpArgs = pGeoreference->getInArgList();
   bool bConditionsForExecution =
      (gcpArgs.setPlugInArgValue<RasterElement>(Executable::DataElementArg(), mpIgmRaster.get()) == false ||
      gcpArgs.setPlugInArgValue<GcpList>("GCP List", pGcpList.get()) == false ||  gcpArgs.setPlugInArgValue<int>("Order", &order) == false);

   if (bConditionsForExecution || pGeoreference->execute() == false)
   {
      progress.report("Unable to execute GCP Georeference plug-in", 0, ERRORS, true);
      return false;
   }

   mpGcpPlugIn = dynamic_cast<Georeference*>(pGeoreference->getPlugIn());
   mpRaster->setGeoreferencePlugin(this);

   progress.report("Georeference finished", 100, NORMAL);
   progress.upALevel();
   return true;
}

bool IgmGeoreference::canHandleRasterElement(RasterElement* pRaster) const
{
   return true;
}

QWidget* IgmGeoreference::getGui(RasterElement* pRaster)
{
   if (pRaster == NULL)
   {
      delete mpGui;
      return NULL;
   }

   mpGui = new IgmGui(pRaster);
   return mpGui;
}

bool IgmGeoreference::validateGuiInput() const
{
   if (mpGui != NULL)
   {
      return mpGui->validateInput();
   }
   return false;
}

bool IgmGeoreference::serialize(SessionItemSerializer &serializer) const
{
   return false;
}

bool IgmGeoreference::deserialize(SessionItemDeserializer &deserializer)
{
   return false;
}

LocationType IgmGeoreference::pixelToGeoQuick(LocationType pixel, bool* pAccurate) const
{
   LocationType geo = pixelToGeo(pixel, pAccurate);
   return geo;
}

void IgmGeoreference::elementDeleted(Subject& subject, const std::string& signal, const boost::any& data)
{
   Service<DesktopServices> pDesktop;
   SpatialDataWindow* pWindow = static_cast<SpatialDataWindow*>(pDesktop->getCurrentWorkspaceWindow());
   SpatialDataView* pView = (pWindow == NULL) ? NULL : pWindow->getSpatialDataView();

   std::vector<Layer*> latLonLayers;
   pView->getLayerList()->getLayers(LAT_LONG, latLonLayers);
   for (std::vector<Layer*>::const_iterator layer = latLonLayers.begin(); layer != latLonLayers.end(); ++layer)
   {
      if ((*layer)->getLayerType() == LAT_LONG && (*layer)->getDataElement()->getParent() == mpRaster->getParent())
      {
         pView->deleteLayer((*layer));
      }
   }
}

LocationType IgmGeoreference::pixelToGeo(LocationType pixel, bool* pAccurate) const
{
   if (pAccurate)
   {
      *pAccurate = false;
   }

   if (mpIgmRaster.get() == NULL)
   {
      return LocationType();
   }

   // Input pixel is in Active Numbers: enforce input to be within bounds.
   pixel.clampMinimum(LocationType(0, 0));
   pixel.clampMaximum(LocationType(mLoadedColumns.size() - 1, mLoadedRows.size() - 1));

   const DimensionDescriptor& column = mLoadedColumns[static_cast<unsigned int>(pixel.mX)];
   const DimensionDescriptor& row = mLoadedRows[static_cast<unsigned int>(pixel.mY)];
   const DimensionDescriptor& northingBand = mLoadedBands[static_cast<unsigned int>(0)];
   const DimensionDescriptor& eastingBand = mLoadedBands[static_cast<unsigned int>(1)];
   if (column.isValid() == false || row.isValid() == false || northingBand.isValid() == false || eastingBand.isValid() == false)
   {
      return LocationType();
   }

   if (pAccurate)
   {
      *pAccurate = true;
   }

   UtmPoint uPoint(mpIgmRaster->getPixelValue(column, row, northingBand),
      mpIgmRaster->getPixelValue(column, row, eastingBand), mZone, 'N');

   return LocationType(uPoint.getLatLonCoordinates().getLatitude().getValue(),
      uPoint.getLatLonCoordinates().getLongitude().getValue());
}

LocationType IgmGeoreference::geoToPixel(LocationType geo, bool* pAccurate) const
{
   // Since GCP Georeference values are stored in reverse order, perform the reverse operation here.
   if (mpGcpPlugIn == NULL || mpIgmGeo.get() == NULL)
   {
      return LocationType();
   }
   else
   {
      return mpGcpPlugIn->pixelToGeo(geo, pAccurate);
   }
}
