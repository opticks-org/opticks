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
#include "GeoreferenceUtilities.h"
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

#define IGM_GEO_NAME "IGM_GEO"

IgmGeoreference::IgmGeoreference() :
   mpGui(NULL),
   mpRaster(NULL),
   mpIgmRaster(NULL),
   mNumRows(0),
   mNumColumns(0),
   mpIgmDesc(NULL)
{
   setName("IGM Georeference");
   setVersion(APP_VERSION_NUMBER);
   setCreator("Ball Aerospace & Technologies, Corp.");
   setCopyright(APP_COPYRIGHT);
   setDescription("IGM Georeferencing plugin");
   setDescriptorId("{EE7B8EB9-8493-4af4-A594-A41FEC3DC4CC}");
   destroyAfterExecute(false);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);

   mpIgmRaster.addSignal(SIGNAL_NAME(Subject, Deleted), Slot(this, &IgmGeoreference::elementDeleted));
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
   VERIFY(pArgList->addArg<RasterElement>("IGM Element", NULL, "If specified, this element contains the IGM data. "
      "If not specified, the existing " IGM_GEO_NAME " child element will be used. If neither exists, the IGM data will be "
      "loaded from the file specified in the GUI. In batch mode, either this argument must be specified or an existing "
      IGM_GEO_NAME " child element must be present."));
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

   // Check the plug-in arg first, then the GUI, and finally look for an existing element
   mpIgmRaster.reset(pInArgList->getPlugInArgValue<RasterElement>("IGM Element"));
   if (mpIgmRaster.get() == NULL && mpGui != NULL && !mpGui->useExisting() && !mpGui->getFilename().isEmpty())
   {
      ImporterResource importer("Auto Importer", mpGui->getFilename().toStdString(), progress.getCurrentProgress());
      if (!importer->execute())
      {
         progress.report("Unable to load IGM data. The file type may not be recognized.", 0, ERRORS, true);
         return false;
      }
      std::vector<DataElement*> igmDataElement = importer->getImportedElements();
      if (igmDataElement.empty())
      {
         progress.report("Unable to load IGM data. The file type may not be recognized.", 0, ERRORS, true);
         return false;
      }
      mpIgmRaster.reset(dynamic_cast<RasterElement*>(igmDataElement.front()));
      if (mpIgmRaster.get() == NULL)
      {
         progress.report("Not a valid IGM file", 0, ERRORS, true);
         return false;
      }
      Service<ModelServices>()->destroyElement(
         Service<ModelServices>()->getElement(IGM_GEO_NAME, TypeConverter::toString<RasterElement>(), mpRaster));
      Service<ModelServices>()->setElementParent(mpIgmRaster.get(), mpRaster);
      Service<ModelServices>()->setElementName(mpIgmRaster.get(), IGM_GEO_NAME);
   }
   if (mpIgmRaster.get() == NULL)
   {
      mpIgmRaster.reset(static_cast<RasterElement*>(Service<ModelServices>()->getElement(
         IGM_GEO_NAME, TypeConverter::toString<RasterElement>(), mpRaster)));
   }
   if (mpIgmRaster.get() == NULL)
   {
      progress.report("No IGM specified", 0, ERRORS, true);
      return false;
   }

   RasterDataDescriptor* pMainDesc = dynamic_cast<RasterDataDescriptor*>(mpRaster->getDataDescriptor());
   VERIFY(pMainDesc != NULL);
   mNumRows = pMainDesc->getRowCount();
   mNumColumns = pMainDesc->getColumnCount();

   mpIgmDesc = static_cast<const RasterDataDescriptor*>(mpIgmRaster->getDataDescriptor());
   VERIFY(mpIgmDesc != NULL);
   if (mpIgmDesc->getRowCount() != mNumRows || mpIgmDesc->getColumnCount() != mNumColumns)
   {
      progress.report("IGM sizes do not match", 0, ERRORS, true);
      return false;
   }

   EncodingType latLonType = mpIgmDesc->getDataType();
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

   // calculate the reverse polynomial
   std::list<GcpPoint> gcpList;
   const unsigned int maxX = mpIgmDesc->getColumnCount() - 1;
   const unsigned int maxY = mpIgmDesc->getRowCount() - 1;
   unsigned int skipX = maxX / 4;   // Appropriate skip factor for a well determined 2 order system.
   unsigned int skipY = maxY / 4;   // Appropriate skip factor for a well determined 2 order system.

   std::vector<LocationType> latlonValues;
   std::vector<LocationType> pixelValues;
   for (unsigned int row = 0; row <= maxY; row += skipY)
   {
      // Read one row at a time.
      for (unsigned int col = 0; col <= maxX; col += skipX)
      {
         // Note that the location of the pixel coordinates and geocoordinates has been reversed.
         // This is to ensure that when the reverse method, IgmGeoreference::geoToPixel(),
         // is called that GcpGeoreference::pixelToGeo() returns the appropriate results.
         LocationType pt(col, row);
         latlonValues.push_back(pt);
         pixelValues.push_back(pixelToGeo(pt, NULL));
      }
   }
   unsigned int numCoeffs = COEFFS_FOR_ORDER(2);
   if (pixelValues.size() < numCoeffs)
   {
      progress.report("Insufficient number of points for geo to pixel conversion.", 0, ERRORS, true);
      return false;
   }
   mLatCoefficients.resize(numCoeffs, 0.0);
   mLonCoefficients.resize(numCoeffs, 0.0);
   // Find the maximum separation to determine if it is the antimeridian or the poles
   bool badValues = true;
   bool badPixelValues = true;
   double maxLonSeparation = 0.0;
   for (size_t i = 0; i < latlonValues.size(); ++i)
   {
      for (size_t j = i + 1; j < latlonValues.size(); ++j)
      {
         double tmpY = fabs(latlonValues[i].mY - latlonValues[j].mY);
         if (fabs(latlonValues[i].mX - latlonValues[j].mX) > 1e-20 || tmpY > 1e-20)
         {
            badValues = false;
         }
         maxLonSeparation = std::max(maxLonSeparation, tmpY);
         if (fabs(pixelValues[i].mX - pixelValues[j].mX) > 1e-20 ||
             fabs(pixelValues[i].mY - pixelValues[j].mY) > 1e-20)
         {
            badPixelValues = false;
         }
      }
   }
   if (badValues || badPixelValues)
   {
      progress.report("Invalid IGM data. Unable to calculate geo to pixel coversion.", 0, ERRORS, true);
      return false;
   }
   if (maxLonSeparation > 180.0)
   {
      for (std::vector<LocationType>::iterator val = latlonValues.begin(); val != latlonValues.end(); ++val)
      {
         if (val->mY < 0.0)
         {
            val->mY += 360.0;
         }
      }
   }

   if (!GeoreferenceUtilities::computeFit(pixelValues, latlonValues, 0, mLatCoefficients) ||
       !GeoreferenceUtilities::computeFit(pixelValues, latlonValues, 1, mLonCoefficients))
   {
      progress.report("Unable to calculate geo to pixel conversion.", 0, ERRORS, true);
      return false;
   }

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
   bool igmPresent = 
      Service<ModelServices>()->getElement("IGM_GEO", TypeConverter::toString<RasterElement>(), pRaster) != NULL;
   mpGui->hasExisting(igmPresent);
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

void IgmGeoreference::elementDeleted(Subject& subject, const std::string& signal, const boost::any& data)
{
   // Delete any lat-lon layers in the raster's view since they will no longer be valid
   Service<DesktopServices> pDesktop;
   SpatialDataWindow* pWindow = dynamic_cast<SpatialDataWindow*>(pDesktop->getCurrentWorkspaceWindow());
   SpatialDataView* pView = (pWindow == NULL) ? NULL : pWindow->getSpatialDataView();
   if (pView == NULL)
   {
      return;
   }

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
   pixel.clampMaximum(LocationType(mpIgmDesc->getColumnCount() - 1, mpIgmDesc->getRowCount() - 1));

   DimensionDescriptor column(mpIgmDesc->getActiveColumn(pixel.mX));
   DimensionDescriptor row(mpIgmDesc->getActiveRow(pixel.mY));
   // first/second is either northing/easting or longitude/latitude
   DimensionDescriptor firstBand(mpIgmDesc->getActiveBand(0));
   DimensionDescriptor secondBand(mpIgmDesc->getActiveBand(1));
   if (!column.isValid() || !row.isValid() || !firstBand.isValid() || !secondBand.isValid())
   {
      return LocationType();
   }

   if (pAccurate)
   {
      *pAccurate = true;
   }

   if (mZone == 100) // no zone...assume we are lat/lon instead of UTM
   {
      return LocationType(mpIgmRaster->getPixelValue(column, row, secondBand),
         mpIgmRaster->getPixelValue(column, row, firstBand));
   }
   char hemisphere = 'N';
   double northing = mpIgmRaster->getPixelValue(column, row, secondBand);
   if (northing < 0.0)
   {
      hemisphere = 'S';
      northing = -northing;
   }
   UtmPoint uPoint(mpIgmRaster->getPixelValue(column, row, firstBand), northing, mZone, hemisphere);
   return LocationType(uPoint.getLatLonCoordinates().getLatitude().getValue(),
      uPoint.getLatLonCoordinates().getLongitude().getValue());
}

LocationType IgmGeoreference::geoToPixel(LocationType geo, bool* pAccurate) const
{
   if (pAccurate != NULL)
   {
      bool outsideCols = geo.mX < 0.0 || geo.mX > static_cast<double>(mNumColumns);
      bool outsideRows = geo.mY < 0.0 || geo.mY > static_cast<double>(mNumRows);
      *pAccurate = !(outsideCols || outsideRows);
   }
   return GeoreferenceUtilities::evaluatePolynomial(geo, mLatCoefficients, mLonCoefficients, 2);
}
