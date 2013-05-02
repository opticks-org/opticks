/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AnnotationLayer.h"
#include "AppVerify.h"
#include "AppVersion.h"
#include "BadValues.h"
#include "DesktopServices.h"
#include "GeoreferenceUtilities.h"
#include "GraphicObject.h"
#include "LayerList.h"
#include "MessageLogResource.h"
#include "ModisGeoreference.h"
#include "ModisUtilities.h"
#include "PlugInArgList.h"
#include "PlugInRegistration.h"
#include "ProductView.h"
#include "Progress.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "SessionItemDeserializer.h"
#include "SessionItemSerializer.h"
#include "SpatialDataView.h"
#include "TypeConverter.h"
#include "WorkspaceWindow.h"
#include "xmlreader.h"
#include "xmlwriter.h"

#include <algorithm>
#include <list>
XERCES_CPP_NAMESPACE_USE

#if defined(WIN_API)
#define isnan _isnan
#endif

#define MODIS_POLYNOMIAL_ORDER 4

REGISTER_PLUGIN_BASIC(OpticksModis, ModisGeoreference);

ModisGeoreference::ModisGeoreference() :
   mpRaster(NULL)
{
   setName("MODIS Georeference");
   setCreator("Ball Aerospace & Technologies Corp.");
   setDescriptorId("{A2C07899-8104-43b6-B2B5-949483C6223F}");
   setVersion(APP_VERSION_NUMBER);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
   setCopyright(APP_COPYRIGHT);
   setDescription("Georeferencing plug-in for MODIS data");
   allowMultipleInstances(true);
   executeOnStartup(false);
   destroyAfterExecute(false);
   setAbortSupported(true);

   mpLatitude.addSignal(SIGNAL_NAME(Subject, Deleted), Slot(this, &ModisGeoreference::elementDeleted));
   mpLongitude.addSignal(SIGNAL_NAME(Subject, Deleted), Slot(this, &ModisGeoreference::elementDeleted));
}

ModisGeoreference::~ModisGeoreference()
{}

unsigned char ModisGeoreference::getGeoreferenceAffinity(const RasterDataDescriptor* pDescriptor) const
{
   if (pDescriptor == NULL)
   {
      return Georeference::CAN_NOT_GEOREFERENCE;
   }

   // Check if the data is valid MODIS data
   const DynamicObject* pMetadata = pDescriptor->getMetadata();

   std::string modisName = ModisUtilities::getModisProductName(pMetadata);
   if ((modisName != "MOD02QKM") &&    // MODIS/Terra 250m resolution
      (modisName != "MOD02HKM") &&     // MODIS/Terra 500m resolution
      (modisName != "MOD021KM") &&     // MODIS/Terra 1km resolution
      (modisName != "MYD02QKM") &&     // MODIS/Aqua 250m resolution
      (modisName != "MYD02HKM") &&     // MODIS/Aqua 500m resolution
      (modisName != "MYD021KM"))       // MODIS/Aqua 1km resolution
   {
      return Georeference::CAN_NOT_GEOREFERENCE;
   }

   // Define a return value that allows custom georeference plug-ins to automatically
   // georeference with a lower affinity than MODIS Georeference
   const unsigned char CAN_GEOREFERENCE_WITH_MODIS = Georeference::CAN_GEOREFERENCE + 10;

   // Check whether a RasterElement exists, which indicates manual georeference after import
   Service<ModelServices> pModel;

   RasterElement* pRaster = dynamic_cast<RasterElement*>(pModel->getElement(pDescriptor));
   if (pRaster != NULL)
   {
      // Check if the 'Latitude' and 'Longitude' child elements exist
      RasterElement* pLatitude = dynamic_cast<RasterElement*>(pModel->getElement("Latitude",
         TypeConverter::toString<RasterElement>(), pRaster));
      RasterElement* pLongitude = dynamic_cast<RasterElement*>(pModel->getElement("Longitude",
         TypeConverter::toString<RasterElement>(), pRaster));

      if ((pLatitude != NULL) && (pLongitude != NULL))
      {
         // Additional checks on the 'Latitude' and 'Longitude' data sets will be performed in validate()
         // so that an appropriate error message can be provided to the user if necessary.  This just
         // allows the MODIS Georeference plug-in to be available when georeferencing.
         return CAN_GEOREFERENCE_WITH_MODIS;
      }
   }
   else
   {
      // Check if the raster element to import is the top-level raster element and not one of the child elements
      if ((pDescriptor->getParent() == NULL) && (pDescriptor->getParentDesignator().empty() == true))
      {
         // The georeference will be performed by the importer with the latitude and longitude
         // child raster elements being imported at the same time, so allow georeferencing
         return CAN_GEOREFERENCE_WITH_MODIS;
      }
   }

   return Georeference::CAN_NOT_GEOREFERENCE;
}

bool ModisGeoreference::validate(const RasterDataDescriptor* pDescriptor, std::string& errorMessage) const
{
   if (GeoreferenceShell::validate(pDescriptor, errorMessage) == false)
   {
      return false;
   }

   VERIFY(pDescriptor != NULL);

   // Check if the raster element to georeference is already imported and exists in the data model
   Service<ModelServices> pModel;

   RasterElement* pRaster = dynamic_cast<RasterElement*>(pModel->getElement(pDescriptor));
   if (pRaster != NULL)
   {
      // Check if the 'Latitude' and 'Longitude' child elements do not exist
      RasterElement* pLatitude = dynamic_cast<RasterElement*>(pModel->getElement("Latitude",
         TypeConverter::toString<RasterElement>(), pRaster));
      if (pLatitude == NULL)
      {
         errorMessage = "The 'Latitude' data element is not loaded.";
         return false;
      }

      RasterElement* pLongitude = dynamic_cast<RasterElement*>(pModel->getElement("Longitude",
         TypeConverter::toString<RasterElement>(), pRaster));
      if (pLongitude == NULL)
      {
         errorMessage = "The 'Longitude' data element is not loaded.";
         return false;
      }

      // Check if the 'Latitude' and 'Longitude' element rows and columns do not match
      const RasterDataDescriptor* pLatitudeDescriptor =
         dynamic_cast<const RasterDataDescriptor*>(pLatitude->getDataDescriptor());
      VERIFY(pLatitudeDescriptor != NULL);

      const RasterDataDescriptor* pLongitudeDescriptor =
         dynamic_cast<const RasterDataDescriptor*>(pLongitude->getDataDescriptor());
      VERIFY(pLongitudeDescriptor != NULL);

      const std::vector<DimensionDescriptor>& latitudeRows = pLatitudeDescriptor->getRows();
      const std::vector<DimensionDescriptor>& latitudeColumns = pLatitudeDescriptor->getColumns();
      const std::vector<DimensionDescriptor>& longitudeRows = pLongitudeDescriptor->getRows();
      const std::vector<DimensionDescriptor>& longitudeColumns = pLongitudeDescriptor->getColumns();

      if ((latitudeRows != longitudeRows) || (latitudeColumns != longitudeColumns))
      {
         errorMessage = "The 'Latitude' and 'Longitude' data elements do not have matching rows and columns.";
         return false;
      }
   }

   return true;
}

bool ModisGeoreference::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   if (pInArgList == NULL)
   {
      return false;
   }

   StepResource pStep("Georeferencing MODIS...", "app", "{53992A0E-6D3A-40cc-9F59-058F21A497DB}");

   // Extract the input args
   Progress* pProgress = pInArgList->getPlugInArgValue<Progress>(Executable::ProgressArg());

   mpRaster = pInArgList->getPlugInArgValue<RasterElement>(Executable::DataElementArg());
   if (mpRaster == NULL)
   {
      std::string message = "The raster element is invalid.";
      if (pProgress != NULL)
      {
         pProgress->updateProgress(message, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, message);
      return false;
   }

   RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(mpRaster->getDataDescriptor());
   VERIFY(pDescriptor != NULL);

   // Validate the Latitude and Longitude child elements
   Service<ModelServices> pModel;

   mpLatitude.reset(dynamic_cast<RasterElement*>(pModel->getElement("Latitude",
      TypeConverter::toString<RasterElement>(), mpRaster)));
   mpLongitude.reset(dynamic_cast<RasterElement*>(pModel->getElement("Longitude",
      TypeConverter::toString<RasterElement>(), mpRaster)));
   if ((mpLatitude.get() == NULL) || (mpLongitude.get() == NULL))
   {
      std::string message = "The latitude/longitude data elements are not loaded.";
      if (pProgress != NULL)
      {
         pProgress->updateProgress(message, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, message);
      return false;
   }

   RasterDataDescriptor* pLatitudeDescriptor = dynamic_cast<RasterDataDescriptor*>(mpLatitude->getDataDescriptor());
   VERIFY(pLatitudeDescriptor != NULL);

   RasterDataDescriptor* pLongitudeDescriptor = dynamic_cast<RasterDataDescriptor*>(mpLongitude->getDataDescriptor());
   VERIFY(pLongitudeDescriptor != NULL);

   const std::vector<DimensionDescriptor>& latitudeRows = pLatitudeDescriptor->getRows();
   const std::vector<DimensionDescriptor>& latitudeColumns = pLatitudeDescriptor->getColumns();
   const std::vector<DimensionDescriptor>& longitudeRows = pLongitudeDescriptor->getRows();
   const std::vector<DimensionDescriptor>& longitudeColumns = pLongitudeDescriptor->getColumns();

   if ((latitudeRows != longitudeRows) || (latitudeColumns != longitudeColumns))
   {
      std::string message = "The 'Latitude' and 'Longitude' elements do not have matching rows and columns.";
      if (pProgress != NULL)
      {
         pProgress->updateProgress(message, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, message);
      return false;
   }

   // Calculate the reverse polynomial
   if (pProgress != NULL)
   {
      pProgress->updateProgress("Georeferencing MODIS...", 0, NORMAL);
   }

   const unsigned int numRows = latitudeRows.size();
   const unsigned int numColumns = latitudeColumns.size();
   unsigned int skipX = std::max(numColumns / 32, 1u);   // Define an appropriate skip factor for the polynomial order
   unsigned int skipY = std::max(numRows / 32, 1u);

   std::vector<LocationType> latLonValues;
   std::vector<LocationType> pixelValues;
   for (unsigned int row = 0; row < numRows; row += skipY)
   {
      if (isAborted() == true)
      {
         std::string message = "MODIS Georeference aborted.";
         if (pProgress != NULL)
         {
            pProgress->updateProgress(message, 0, ABORT);
         }

         pStep->finalize(Message::Abort, message);
         return false;
      }

      DimensionDescriptor geoRow = latitudeRows[row];
      VERIFY(geoRow.isOriginalNumberValid() == true);

      for (unsigned int column = 0; column < numColumns; column += skipX)
      {
         DimensionDescriptor geoColumn = latitudeColumns[column];
         VERIFY(geoColumn.isOriginalNumberValid() == true);

         DimensionDescriptor rasterRow = pDescriptor->getOriginalRow(geoRow.getOriginalNumber());
         DimensionDescriptor rasterColumn = pDescriptor->getOriginalColumn(geoColumn.getOriginalNumber());

         if ((rasterRow.isActiveNumberValid() == true) && (rasterColumn.isActiveNumberValid() == true))
         {
            double latitude = mpLatitude->getPixelValue(geoColumn, geoRow);
            double longitude = mpLongitude->getPixelValue(geoColumn, geoRow);
            latLonValues.push_back(LocationType(latitude, longitude));

            LocationType pixel(rasterColumn.getActiveNumber() + 0.5, rasterRow.getActiveNumber() + 0.5);
            pixelValues.push_back(pixel);
         }
      }

      if (pProgress != NULL)
      {
         pProgress->updateProgress("Georeferencing MODIS...", row * 100 / numRows, NORMAL);
      }
   }

   unsigned int numCoeffs = COEFFS_FOR_ORDER(MODIS_POLYNOMIAL_ORDER);
   if (pixelValues.size() < numCoeffs)
   {
      std::string message = "There is an insufficient number of matching raster pixels and latitude/longitude "
         "pixels to perform the georeferencing coordinate transformations.";
      if (pProgress != NULL)
      {
         pProgress->updateProgress(message, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, message);
      return false;
   }

   mXCoefficients.resize(numCoeffs, 0.0);
   mYCoefficients.resize(numCoeffs, 0.0);

   // Find the maximum separation to determine if it is the antimeridian or the poles
   bool badValues = true;
   bool badPixelValues = true;
   double maxLonSeparation = 0.0;
   for (size_t i = 0; i < latLonValues.size(); ++i)
   {
      for (size_t j = i + 1; j < latLonValues.size(); ++j)
      {
         double tmpY = fabs(latLonValues[i].mY - latLonValues[j].mY);
         if ((fabs(latLonValues[i].mX - latLonValues[j].mX) > 1e-20) || (tmpY > 1e-20))
         {
            badValues = false;
         }

         maxLonSeparation = std::max(maxLonSeparation, tmpY);
         if ((fabs(pixelValues[i].mX - pixelValues[j].mX) > 1e-20) ||
             (fabs(pixelValues[i].mY - pixelValues[j].mY) > 1e-20))
         {
            badPixelValues = false;
         }
      }
   }

   if ((badValues == true) || (badPixelValues == true))
   {
      std::string message = "Invalid latitude/longitude data.  Unable to calculate geo to pixel conversion.";
      if (pProgress != NULL)
      {
         pProgress->updateProgress(message, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, message);
      return false;
   }

   if (maxLonSeparation > 180.0)
   {
      // The data spans the antimeridian, so normalize the longitude coordinate space to 0-360 degrees
      // for the polynomial fit calculations to work properly
      for (std::vector<LocationType>::iterator iter = latLonValues.begin(); iter != latLonValues.end(); ++iter)
      {
         if (iter->mY < 0.0)
         {
            iter->mY += 360.0;
         }
      }
   }

   if ((GeoreferenceUtilities::computeFit(latLonValues, pixelValues, 0, mXCoefficients) == false) ||
      (GeoreferenceUtilities::computeFit(latLonValues, pixelValues, 1, mYCoefficients) == false))
   {
      std::string message = "Unable to calculate geo to pixel conversion.";
      if (pProgress != NULL)
      {
         pProgress->updateProgress(message, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, message);
      return false;
   }

   // Set the parent raster element to use this plug-in as its georeference plug-in
   mpRaster->setGeoreferencePlugin(this);

   // Update the progress
   if (pProgress != NULL)
   {
      pProgress->updateProgress("MODIS georeference complete.", 100, NORMAL);
   }

   pStep->finalize(Message::Success);
   return true;
}

LocationType ModisGeoreference::geoToPixel(LocationType geo, bool* pAccurate) const
{
   LocationType pixel = GeoreferenceUtilities::evaluatePolynomial(geo, mXCoefficients, mYCoefficients,
      MODIS_POLYNOMIAL_ORDER);
   if (pAccurate != NULL)
   {
      VERIFYRV(mpRaster != NULL, LocationType());

      RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(mpRaster->getDataDescriptor());
      VERIFYRV(pDescriptor != NULL, LocationType());

      unsigned int numRows = pDescriptor->getRowCount();
      unsigned int numColumns = pDescriptor->getColumnCount();

      bool outsideCols = pixel.mX < 0.0 || pixel.mX > static_cast<double>(numColumns);
      bool outsideRows = pixel.mY < 0.0 || pixel.mY > static_cast<double>(numRows);
      *pAccurate = !(outsideCols || outsideRows);
   }

   return pixel;
}

LocationType ModisGeoreference::pixelToGeo(LocationType pixel, bool* pAccurate) const
{
   if (pAccurate != NULL)
   {
      *pAccurate = false;
   }

   if ((mpRaster == NULL) || (mpLatitude.get() == NULL) || (mpLongitude.get() == NULL))
   {
      return LocationType();
   }

   // Get the row and column objects for the pixel locations
   const RasterDataDescriptor* pRasterDescriptor =
      dynamic_cast<const RasterDataDescriptor*>(mpRaster->getDataDescriptor());
   VERIFYRV(pRasterDescriptor != NULL, LocationType());

   if ((pixel.mX < 0.0) || (pixel.mY < 0.0) || (pixel.mX > pRasterDescriptor->getColumnCount()) ||
      (pixel.mY > pRasterDescriptor->getRowCount()))
   {
      return LocationType();
   }

   // Add 0.5 to the pixel location since the lat/long values are located at the center of the pixel
   DimensionDescriptor rasterRow = pRasterDescriptor->getActiveRow(static_cast<unsigned int>(pixel.mY + 0.5));
   DimensionDescriptor rasterColumn = pRasterDescriptor->getActiveColumn(static_cast<unsigned int>(pixel.mX + 0.5));
   if ((rasterRow.isOriginalNumberValid() == false) || (rasterColumn.isOriginalNumberValid() == false))
   {
      return LocationType();
   }

   unsigned int rasterOriginalRowNumber = rasterRow.getOriginalNumber();
   unsigned int rasterOriginalColumnNumber = rasterColumn.getOriginalNumber();

   // Get the four latitude values closest to the pixel location
   const RasterDataDescriptor* pLatitudeDescriptor =
      dynamic_cast<const RasterDataDescriptor*>(mpLatitude->getDataDescriptor());
   VERIFYRV(pLatitudeDescriptor != NULL, LocationType());

   const std::vector<DimensionDescriptor>& latitudeRows = pLatitudeDescriptor->getRows();
   const std::vector<DimensionDescriptor>& latitudeColumns = pLatitudeDescriptor->getColumns();

   DimensionDescriptor latitudeLeftColumn = getClosestGeocoordinate(latitudeColumns, rasterOriginalColumnNumber, true);
   DimensionDescriptor latitudeRightColumn = getClosestGeocoordinate(latitudeColumns,
      rasterOriginalColumnNumber, false);
   DimensionDescriptor latitudeTopRow = getClosestGeocoordinate(latitudeRows, rasterOriginalRowNumber, false);
   DimensionDescriptor latitudeBottomRow = getClosestGeocoordinate(latitudeRows, rasterOriginalRowNumber, true);

   if ((latitudeLeftColumn.isValid() == false) || (latitudeRightColumn.isValid() == false) ||
      (latitudeTopRow.isValid() == false) || (latitudeBottomRow.isValid() == false))
   {
      return LocationType();
   }

   double latitudeLowerLeftGeo = mpLatitude->getPixelValue(latitudeLeftColumn, latitudeBottomRow);
   double latitudeUpperLeftGeo = mpLatitude->getPixelValue(latitudeLeftColumn, latitudeTopRow);
   double latitudeUpperRightGeo = mpLatitude->getPixelValue(latitudeRightColumn, latitudeTopRow);
   double latitudeLowerRightGeo = mpLatitude->getPixelValue(latitudeRightColumn, latitudeBottomRow);

   const BadValues* pLatitudeBadValues = pLatitudeDescriptor->getBadValues();
   VERIFYRV(pLatitudeBadValues != NULL, LocationType());

   if ((pLatitudeBadValues->isBadValue(latitudeLowerLeftGeo) == true) ||
      (pLatitudeBadValues->isBadValue(latitudeUpperLeftGeo) == true) ||
      (pLatitudeBadValues->isBadValue(latitudeUpperRightGeo) == true) ||
      (pLatitudeBadValues->isBadValue(latitudeLowerRightGeo) == true))
   {
      return LocationType();
   }

   // Get the four longitude values closest to the pixel location
   const RasterDataDescriptor* pLongitudeDescriptor =
      dynamic_cast<const RasterDataDescriptor*>(mpLongitude->getDataDescriptor());
   VERIFYRV(pLongitudeDescriptor != NULL, LocationType());

   const std::vector<DimensionDescriptor>& longitudeRows = pLongitudeDescriptor->getRows();
   const std::vector<DimensionDescriptor>& longitudeColumns = pLongitudeDescriptor->getColumns();

   DimensionDescriptor longitudeLeftColumn = getClosestGeocoordinate(longitudeColumns,
      rasterOriginalColumnNumber, true);
   DimensionDescriptor longitudeRightColumn = getClosestGeocoordinate(longitudeColumns,
      rasterOriginalColumnNumber, false);
   DimensionDescriptor longitudeTopRow = getClosestGeocoordinate(longitudeRows, rasterOriginalRowNumber, false);
   DimensionDescriptor longitudeBottomRow = getClosestGeocoordinate(longitudeRows, rasterOriginalRowNumber, true);

   if ((longitudeLeftColumn.isValid() == false) || (longitudeRightColumn.isValid() == false) ||
      (longitudeTopRow.isValid() == false) || (longitudeBottomRow.isValid() == false))
   {
      return LocationType();
   }

   double longitudeLowerLeftGeo = mpLongitude->getPixelValue(longitudeLeftColumn, longitudeBottomRow);
   double longitudeUpperLeftGeo = mpLongitude->getPixelValue(longitudeLeftColumn, longitudeTopRow);
   double longitudeUpperRightGeo = mpLongitude->getPixelValue(longitudeRightColumn, longitudeTopRow);
   double longitudeLowerRightGeo = mpLongitude->getPixelValue(longitudeRightColumn, longitudeBottomRow);

   const BadValues* pLongitudeBadValues = pLongitudeDescriptor->getBadValues();
   VERIFYRV(pLongitudeBadValues != NULL, LocationType());

   if ((pLongitudeBadValues->isBadValue(longitudeLowerLeftGeo) == true) ||
      (pLongitudeBadValues->isBadValue(longitudeUpperLeftGeo) == true) ||
      (pLongitudeBadValues->isBadValue(longitudeUpperRightGeo) == true) ||
      (pLongitudeBadValues->isBadValue(longitudeLowerRightGeo) == true))
   {
      return LocationType();
   }

   // Get the raster pixel values for the closest latitude and longitude values
   LocationType lowerLeftGeo(latitudeLowerLeftGeo, longitudeLowerLeftGeo);
   LocationType upperLeftGeo(latitudeUpperLeftGeo, longitudeUpperLeftGeo);
   LocationType upperRightGeo(latitudeUpperRightGeo, longitudeUpperRightGeo);
   LocationType lowerRightGeo(latitudeLowerRightGeo, longitudeLowerRightGeo);

   DimensionDescriptor rasterLeftColumn = pRasterDescriptor->getOriginalColumn(longitudeLeftColumn.getOriginalNumber());
   DimensionDescriptor rasterRightColumn =
      pRasterDescriptor->getOriginalColumn(longitudeRightColumn.getOriginalNumber());
   DimensionDescriptor rasterTopRow = pRasterDescriptor->getOriginalRow(latitudeTopRow.getOriginalNumber());
   DimensionDescriptor rasterBottomRow = pRasterDescriptor->getOriginalRow(latitudeBottomRow.getOriginalNumber());

   if ((rasterLeftColumn.isActiveNumberValid() == false) || (rasterRightColumn.isActiveNumberValid() == false) ||
      (rasterTopRow.isActiveNumberValid() == false) || (rasterBottomRow.isActiveNumberValid() == false))
   {
      return LocationType();
   }

   LocationType lowerLeftPixel(rasterLeftColumn.getActiveNumber() + 0.5, rasterBottomRow.getActiveNumber() + 0.5);
   LocationType upperLeftPixel(rasterLeftColumn.getActiveNumber() + 0.5, rasterTopRow.getActiveNumber() + 0.5);
   LocationType upperRightPixel(rasterRightColumn.getActiveNumber() + 0.5, rasterTopRow.getActiveNumber() + 0.5);
   LocationType lowerRightPixel(rasterRightColumn.getActiveNumber() + 0.5, rasterBottomRow.getActiveNumber() + 0.5);

   // Calculate the geo value using bilinear interpolation
   double latitudeX = ((lowerRightPixel.mX - pixel.mX) / (lowerRightPixel.mX - lowerLeftPixel.mX) * lowerLeftGeo.mX) +
      ((pixel.mX - lowerLeftPixel.mX) / (lowerRightPixel.mX - lowerLeftPixel.mX) * lowerRightGeo.mX);
   double latitudeY = ((lowerRightPixel.mX - pixel.mX) / (lowerRightPixel.mX - lowerLeftPixel.mX) * upperLeftGeo.mX) +
      ((pixel.mX - lowerLeftPixel.mX) / (lowerRightPixel.mX - lowerLeftPixel.mX) * upperRightGeo.mX);
   double latitude = ((upperLeftPixel.mY - pixel.mY) / (upperLeftPixel.mY - lowerLeftPixel.mY) * latitudeX) +
      ((pixel.mY - lowerLeftPixel.mY) / (upperLeftPixel.mY - lowerLeftPixel.mY) * latitudeY);

   double longitudeX = ((lowerRightPixel.mX - pixel.mX) / (lowerRightPixel.mX - lowerLeftPixel.mX) * lowerLeftGeo.mY) +
      ((pixel.mX - lowerLeftPixel.mX) / (lowerRightPixel.mX - lowerLeftPixel.mX) * lowerRightGeo.mY);
   double longitudeY = ((lowerRightPixel.mX - pixel.mX) / (lowerRightPixel.mX - lowerLeftPixel.mX) * upperLeftGeo.mY) +
      ((pixel.mX - lowerLeftPixel.mX) / (lowerRightPixel.mX - lowerLeftPixel.mX) * upperRightGeo.mY);
   double longitude = ((upperLeftPixel.mY - pixel.mY) / (upperLeftPixel.mY - lowerLeftPixel.mY) * longitudeX) +
      ((pixel.mY - lowerLeftPixel.mY) / (upperLeftPixel.mY - lowerLeftPixel.mY) * longitudeY);

   if ((isnan(latitude) != 0) || (isnan(longitude) != 0))
   {
      return LocationType();
   }

   if (pAccurate)
   {
      *pAccurate = true;
   }

   return LocationType(latitude, longitude);
}

bool ModisGeoreference::serialize(SessionItemSerializer& serializer) const
{
   if (mpRaster == NULL)
   {
      // execute() has not yet been called so there is no need to serialize any parameters
      return true;
   }

   XMLWriter writer("ModisGeoreference");
   writer.addAttr("rasterId", mpRaster->getId());
   writer.addAttr("xCoefficients", mXCoefficients);
   writer.addAttr("yCoefficients", mYCoefficients);

   if (mpLatitude.get() != NULL)
   {
      writer.addAttr("latitudeId", mpLatitude->getId());
   }

   if (mpLongitude.get() != NULL)
   {
      writer.addAttr("longitudeId", mpLongitude->getId());
   }

   return serializer.serialize(writer);
}

bool ModisGeoreference::deserialize(SessionItemDeserializer& deserializer)
{
   if (deserializer.getBlockSizes().empty() == true)
   {
      // The plug-in was serialized before execute() was called
      return true;
   }

   XmlReader reader(NULL, false);

   DOMElement* pRootElement = deserializer.deserialize(reader, "ModisGeoreference");
   if (pRootElement == NULL)
   {
      return false;
   }

   std::string rasterId = A(pRootElement->getAttribute(X("rasterId")));

   mpRaster = dynamic_cast<RasterElement*>(Service<SessionManager>()->getSessionItem(rasterId));
   if (mpRaster == NULL)
   {
      return false;
   }

   std::string latitudeId = A(pRootElement->getAttribute(X("latitudeId")));
   mpLatitude.reset(dynamic_cast<RasterElement*>(Service<SessionManager>()->getSessionItem(latitudeId)));

   std::string longitudeId = A(pRootElement->getAttribute(X("longitudeId")));
   mpLongitude.reset(dynamic_cast<RasterElement*>(Service<SessionManager>()->getSessionItem(longitudeId)));

   std::vector<double>* pXCoefficients = reinterpret_cast<std::vector<double>*>(XmlReader::StrToVector<double,
      XmlReader::StringStreamAssigner<double> >(pRootElement->getAttribute(X("xCoefficients"))));
   if (pXCoefficients != NULL)
   {
      mXCoefficients = *pXCoefficients;
      delete pXCoefficients;
   }
   else
   {
      return false;
   }

   std::vector<double>* pYCoefficients = reinterpret_cast<std::vector<double>*>(XmlReader::StrToVector<double,
      XmlReader::StringStreamAssigner<double> >(pRootElement->getAttribute(X("yCoefficients"))));
   if (pYCoefficients != NULL)
   {
      mYCoefficients = *pYCoefficients;
      delete pYCoefficients;
   }
   else
   {
      return false;
   }

   return true;
}

void ModisGeoreference::elementDeleted(Subject& subject, const std::string& signal, const boost::any& data)
{
   // Delete any lat/lon layers displaying the parent raster element since the georeferencing will no longer be valid
   std::vector<Window*> windows;
   Service<DesktopServices>()->getWindows(TypeConverter::toString<WorkspaceWindow>(), windows);
   for (std::vector<Window*>::iterator iter = windows.begin(); iter != windows.end(); ++iter)
   {
      WorkspaceWindow* pWindow = dynamic_cast<WorkspaceWindow*>(*iter);
      if (pWindow != NULL)
      {
         SpatialDataView* pSpatialDataView = dynamic_cast<SpatialDataView*>(pWindow->getView());
         if (pSpatialDataView != NULL)
         {
            LayerList* pLayerList = pSpatialDataView->getLayerList();
            if (pLayerList != NULL)
            {
               std::vector<Layer*> layers;
               pLayerList->getLayers(LAT_LONG, layers);
               for (std::vector<Layer*>::iterator layer = layers.begin(); layer != layers.end(); ++layer)
               {
                  Layer* pLayer = *layer;
                  if ((pLayer != NULL) && (pLayer->getDataElement() == mpRaster))
                  {
                     pSpatialDataView->deleteLayer(pLayer);
                  }
               }
            }
         }

         ProductView* pProductView = dynamic_cast<ProductView*>(pWindow->getView());
         if (pProductView != NULL)
         {
            AnnotationLayer* pLayoutLayer = pProductView->getLayoutLayer();
            if (pLayoutLayer != NULL)
            {
               std::list<GraphicObject*> objects;
               pLayoutLayer->getObjects(VIEW_OBJECT, objects);
               for (std::list<GraphicObject*>::iterator object = objects.begin(); object != objects.end(); ++object)
               {
                  GraphicObject* pObject = *object;
                  if (pObject != NULL)
                  {
                     SpatialDataView* pObjectView = dynamic_cast<SpatialDataView*>(pObject->getObjectView());
                     if (pObjectView != NULL)
                     {
                        LayerList* pLayerList = pObjectView->getLayerList();
                        if (pLayerList != NULL)
                        {
                           std::vector<Layer*> layers;
                           pLayerList->getLayers(LAT_LONG, layers);
                           for (std::vector<Layer*>::iterator layer = layers.begin(); layer != layers.end(); ++layer)
                           {
                              Layer* pLayer = *layer;
                              if ((pLayer != NULL) && (pLayer->getDataElement() == mpRaster))
                              {
                                 pSpatialDataView->deleteLayer(pLayer);
                              }
                           }
                        }
                     }
                  }
               }
            }
         }
      }
   }
}

DimensionDescriptor ModisGeoreference::getClosestGeocoordinate(const std::vector<DimensionDescriptor>& dims,
                                                               unsigned int originalNumber, bool lessThan) const
{
   DimensionDescriptor geocoord;
   for (std::vector<DimensionDescriptor>::const_iterator iter = dims.begin(); iter != dims.end(); ++iter)
   {
      DimensionDescriptor descriptor = *iter;
      if (descriptor.isOriginalNumberValid() == true)
      {
         if (descriptor.getOriginalNumber() >= originalNumber)
         {
            if (lessThan == true)
            {
               return geocoord;
            }

            return descriptor;
         }

         geocoord = descriptor;
      }
   }

   return DimensionDescriptor();
}
