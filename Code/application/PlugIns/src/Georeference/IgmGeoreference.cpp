/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AnnotationLayer.h"
#include "AppVersion.h"
#include "AppVerify.h"
#include "DesktopServices.h"
#include "DynamicObject.h"
#include "IgmGeoreference.h"
#include "IgmGui.h"
#include "ImportDescriptor.h"
#include "Importer.h"
#include "GcpList.h"
#include "GeoPoint.h"
#include "GeoreferenceDescriptor.h"
#include "GeoreferenceUtilities.h"
#include "GraphicObject.h"
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
#include "ProductView.h"
#include "RasterElement.h"
#include "RasterDataDescriptor.h"
#include "RasterUtilities.h"
#include "SpatialDataView.h"
#include "SessionItemDeserializer.h"
#include "SessionItemSerializer.h"
#include "Statistics.h"
#include "TypeConverter.h"
#include "WorkspaceWindow.h"
#include "XercesIncludes.h"
#include "xmlreader.h"
#include "xmlwriter.h"

#include <QtCore/QFile>

#include <algorithm>
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
   delete mpGui;
}

bool IgmGeoreference::getInputSpecification(PlugInArgList*& pArgList)
{
   if (GeoreferenceShell::getInputSpecification(pArgList) == false)
   {
      return false;
   }

   VERIFY(pArgList != NULL);
   VERIFY(pArgList->addArg<RasterElement>("IGM Element", NULL, "If specified, this element contains the "
      "IGM data.  If not specified, the IGM data will be loaded from the IGM File input.  If neither input "
      "is specified, either the existing " IGM_GEO_NAME " child element or IGM filename contained in the " +
      Executable::DataElementArg() + " input will be used."));
   VERIFY(pArgList->addArg<Filename>("IGM File", NULL, "The name of the file containing the IGM data.  If "
      "not specified, the IGM filename contained in the " + Executable::DataElementArg() + " input will be "
      "used.  This value is ignored if the IGM Element input is specified."));

   return true;
}

bool IgmGeoreference::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   VERIFY(pInArgList != NULL);

   mProgress.initialize(pInArgList->getPlugInArgValue<Progress>(Executable::ProgressArg()),
      "Georeferencing IGM...", "app", "{53992A0E-6D3A-40cc-9F59-058F21A497DB}");

   // Get RasterElement input.
   mpRaster = pInArgList->getPlugInArgValue<RasterElement>(Executable::DataElementArg());
   if (mpRaster == NULL)
   {
      mProgress.report("No raster element specified", 0, ERRORS, true);
      return false;
   }

   // Check the plug-in args for the IGM data
   mpIgmRaster.reset(pInArgList->getPlugInArgValue<RasterElement>("IGM Element"));
   bool existingElementArgSet = (mpIgmRaster.get() != NULL);
   std::string argFilename;
   bool argFilenameSet = false;

   if (mpIgmRaster.get() == NULL)
   {
      Filename* pFilename = pInArgList->getPlugInArgValue<Filename>("IGM File");
      if (pFilename != NULL)
      {
         argFilename = pFilename->getFullPathAndName();
         if (argFilename.empty() == false)
         {
            if (loadIgmFile(argFilename) == false)
            {
               // Since a valid filename is specified, return false if an error occurs
               return false;
            }
         }

         argFilenameSet = true;
      }
   }

   // If the IGM data are not available from the plug-in args, check the georeference descriptor
   if (mpIgmRaster.get() == NULL)
   {
      const RasterDataDescriptor* pRasterDescriptor =
         dynamic_cast<const RasterDataDescriptor*>(mpRaster->getDataDescriptor());
      if (pRasterDescriptor != NULL)
      {
         const GeoreferenceDescriptor* pGeorefDescriptor = pRasterDescriptor->getGeoreferenceDescriptor();
         if (pGeorefDescriptor != NULL)
         {
            bool useExistingElement =
               dv_cast<bool>(pGeorefDescriptor->getAttributeByPath(USE_EXISTING_ELEMENT), false);
            if (useExistingElement == false)
            {
               std::string filename = dv_cast<std::string>(pGeorefDescriptor->getAttributeByPath(IGM_FILENAME),
                  std::string());
               if (filename.empty() == false)
               {
                  if (loadIgmFile(filename) == false)
                  {
                     // Since a valid filename is specified, return false if an error occurs
                     return false;
                  }
               }
            }
         }
      }
   }

   // If the IGM data are not available from the georeference descriptor, check for an existing child raster element
   if (mpIgmRaster.get() == NULL)
   {
      mpIgmRaster.reset(static_cast<RasterElement*>(Service<ModelServices>()->getElement(IGM_GEO_NAME,
         TypeConverter::toString<RasterElement>(), mpRaster)));
   }

   if (mpIgmRaster.get() == NULL)
   {
      mProgress.report("No IGM specified", 0, ERRORS, true);
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
      mProgress.report("IGM sizes do not match", 0, ERRORS, true);
      return false;
   }

   EncodingType latLonType = mpIgmDesc->getDataType();
   if (latLonType != FLT4BYTES && latLonType != FLT8BYTES)
   {
      mProgress.report("IGM types must be floating point", 0, ERRORS, true);
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
   unsigned int skipX = std::max<unsigned int>(1, maxX / 4);   // Appropriate skip factor for a well determined 2 order system.
   unsigned int skipY = std::max<unsigned int>(1, maxY / 4);   // Appropriate skip factor for a well determined 2 order system.

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
      mProgress.report("Insufficient number of points for geo to pixel conversion.", 0, ERRORS, true);
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
      mProgress.report("Invalid IGM data. Unable to calculate geo to pixel conversion.", 0, ERRORS, true);
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
      mProgress.report("Unable to calculate geo to pixel conversion.", 0, ERRORS, true);
      return false;
   }

   mpRaster->setGeoreferencePlugin(this);

   // Update the georeference descriptor with the current georeference parameters if necessary
   GeoreferenceDescriptor* pGeorefDescriptor = pMainDesc->getGeoreferenceDescriptor();
   if (pGeorefDescriptor != NULL)
   {
      // Georeference plug-in
      const std::string& plugInName = getName();
      pGeorefDescriptor->setGeoreferencePlugInName(plugInName);

      // Use existing element and IGM filename
      if ((existingElementArgSet == true) || (argFilenameSet == true))
      {
         pGeorefDescriptor->setAttributeByPath(USE_EXISTING_ELEMENT, existingElementArgSet);
         pGeorefDescriptor->setAttributeByPath(IGM_FILENAME, argFilename);
      }
   }

   mProgress.report("Georeference finished", 100, NORMAL);
   mProgress.upALevel();
   return true;
}

unsigned char IgmGeoreference::getGeoreferenceAffinity(const RasterDataDescriptor* pDescriptor) const
{
   if (pDescriptor == NULL)
   {
      return Georeference::CAN_NOT_GEOREFERENCE;
   }

   const GeoreferenceDescriptor* pGeorefDescriptor = pDescriptor->getGeoreferenceDescriptor();
   VERIFY(pGeorefDescriptor != NULL);

   // Define a return value that favors IGM Georeference over both RPC Georeference and GCP Georeference
   const unsigned char CAN_GEOREFERENCE_WITH_IGM = Georeference::CAN_GEOREFERENCE + 30;

   // Check whether a RasterElement exists, which indicates manual georeference after import
   Service<ModelServices> pModel;

   RasterElement* pRaster = dynamic_cast<RasterElement*>(pModel->getElement(pDescriptor));
   if (pRaster != NULL)
   {
      // Check if the IGM element exists
      if (pModel->getElement(IGM_GEO_NAME, TypeConverter::toString<RasterElement>(), pRaster) != NULL)
      {
         return CAN_GEOREFERENCE_WITH_IGM;
      }

      // Check if an IGM file exists
      std::string filename = dv_cast<std::string>(pGeorefDescriptor->getAttributeByPath(IGM_FILENAME), std::string());
      if ((filename.empty() == false) && (QFile::exists(QString::fromStdString(filename)) == true))
      {
         return CAN_GEOREFERENCE_WITH_IGM;
      }
   }
   else if (dv_cast<bool>(pGeorefDescriptor->getAttributeByPath(USE_EXISTING_ELEMENT), false) == true)
   {
      // The georeference will be performed by an importer with a child raster
      // element being imported at the same time, so allow georeferencing
      return CAN_GEOREFERENCE_WITH_IGM;
   }

   // Allow georeferencing with user input so that a valid IGM file can be specified
   return Georeference::CAN_GEOREFERENCE_WITH_USER_INPUT;
}

QWidget* IgmGeoreference::getWidget(RasterDataDescriptor* pDescriptor)
{
   if (pDescriptor == NULL)
   {
      return NULL;
   }

   // Create the widget
   if (mpGui == NULL)
   {
      mpGui = new IgmGui();
   }

   // Set the georeference data into the widget for the given raster data
   GeoreferenceDescriptor* pGeorefDescriptor = pDescriptor->getGeoreferenceDescriptor();
   if (pGeorefDescriptor != NULL)
   {
      Service<ModelServices> pModel;
      RasterElement* pChild = NULL;

      RasterElement* pRaster = dynamic_cast<RasterElement*>(pModel->getElement(pDescriptor));
      if (pRaster != NULL)
      {
         pChild = dynamic_cast<RasterElement*>(pModel->getElement(IGM_GEO_NAME,
            TypeConverter::toString<RasterElement>(), pRaster));
      }

      bool enableExistingElement = false;
      if ((pRaster == NULL) || (pChild != NULL))   // IGM Georeference supports using an existing element if the parent
                                                   // element is NULL, which indicates that georeferencing is being
                                                   // performed on import and the child may be imported at the same
                                                   // time as the parent, or if the child element exists after import
      {
         enableExistingElement = true;
      }

      mpGui->setGeoreferenceData(pGeorefDescriptor, enableExistingElement);
   }

   return mpGui;
}

bool IgmGeoreference::validate(const RasterDataDescriptor* pDescriptor, std::string& errorMessage) const
{
   if (GeoreferenceShell::validate(pDescriptor, errorMessage) == false)
   {
      return false;
   }

   VERIFY(pDescriptor != NULL);

   const GeoreferenceDescriptor* pGeorefDescriptor = pDescriptor->getGeoreferenceDescriptor();
   VERIFY(pGeorefDescriptor != NULL);

   // Check whether an existing IGM element should be used
   if (dv_cast<bool>(pGeorefDescriptor->getAttributeByPath(USE_EXISTING_ELEMENT), false) == true)
   {
      // Using an existing IGM element, so check that it exists in the data model
      Service<ModelServices> pModel;

      RasterElement* pRaster = dynamic_cast<RasterElement*>(pModel->getElement(pDescriptor));
      if (pRaster == NULL)
      {
         // Performing georeference on import, so just report a warning
         errorMessage = "The IGM data element is not already loaded.  Georeferencing will fail if it is not loaded "
            "at the same time as this raster data set.";
      }
      else if (pModel->getElement(IGM_GEO_NAME, TypeConverter::toString<RasterElement>(), pRaster) == NULL)
      {
         errorMessage = "The IGM data element does not exist.";
         return false;
      }
   }
   else
   {
      // Using an IGM file, so check that the file exists
      std::string filename = dv_cast<std::string>(pGeorefDescriptor->getAttributeByPath(IGM_FILENAME), std::string());
      if (filename.empty() == true)
      {
         errorMessage = "The IGM filename is invalid.";
         return false;
      }

      if (QFile::exists(QString::fromStdString(filename)) == false)
      {
         errorMessage = "The IGM file does not exist.";
         return false;
      }
   }

   return true;
}

bool IgmGeoreference::serialize(SessionItemSerializer &serializer) const
{
   if (mpRaster == NULL)
   {
      // execute() has not yet been called so there is no need to serialize any parameters
      return true;
   }

   return false;
}

bool IgmGeoreference::deserialize(SessionItemDeserializer &deserializer)
{
   if (deserializer.getBlockSizes().empty() == true)
   {
      // The plug-in was serialized before execute() was called
      return true;
   }

   return false;
}

void IgmGeoreference::elementDeleted(Subject& subject, const std::string& signal, const boost::any& data)
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
               for (std::vector<Layer*>::const_iterator layer = layers.begin(); layer != layers.end(); ++layer)
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
               for (std::list<GraphicObject*>::const_iterator object = objects.begin();
                  object != objects.end();
                  ++object)
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
                           for (std::vector<Layer*>::const_iterator layer = layers.begin();
                              layer != layers.end();
                              ++layer)
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
   LocationType pixel = GeoreferenceUtilities::evaluatePolynomial(geo, mLatCoefficients, mLonCoefficients, 2);
   if (pAccurate != NULL)
   {
      bool outsideCols = pixel.mX < 0.0 || pixel.mX > static_cast<double>(mNumColumns);
      bool outsideRows = pixel.mY < 0.0 || pixel.mY > static_cast<double>(mNumRows);
      *pAccurate = !(outsideCols || outsideRows);
   }

   return pixel;
}

bool IgmGeoreference::loadIgmFile(const std::string& igmFilename)
{
   if (igmFilename.empty() == true)
   {
      mProgress.report("The IGM filename is invalid.", 0, ERRORS, true);
      return false;
   }

   ImporterResource importer("Auto Importer", igmFilename, mProgress.getCurrentProgress());

   // Disable auto-georeference for the IGM data
   std::vector<ImportDescriptor*> importDescriptors = importer->getImportDescriptors(igmFilename);
   for (std::vector<ImportDescriptor*>::iterator iter = importDescriptors.begin();
      iter != importDescriptors.end();
      ++iter)
   {
      ImportDescriptor* pImportDescriptor = *iter;
      if (pImportDescriptor != NULL)
      {
         RasterDataDescriptor* pDataDescriptor =
            dynamic_cast<RasterDataDescriptor*>(pImportDescriptor->getDataDescriptor());
         if (pDataDescriptor != NULL)
         {
            GeoreferenceDescriptor* pGeorefDescriptor = pDataDescriptor->getGeoreferenceDescriptor();
            if (pGeorefDescriptor != NULL)
            {
               pGeorefDescriptor->setGeoreferenceOnImport(false);
            }
         }
      }
   }

   // Import the IGM data
   if (!importer->execute())
   {
      mProgress.report("Unable to load IGM data. The file type may not be recognized.", 0, ERRORS, true);
      return false;
   }

   // Set the member variable
   std::vector<DataElement*> igmDataElement = importer->getImportedElements();
   if (igmDataElement.empty())
   {
      mProgress.report("Unable to load IGM data. The file type may not be recognized.", 0, ERRORS, true);
      return false;
   }

   mpIgmRaster.reset(dynamic_cast<RasterElement*>(igmDataElement.front()));
   if (mpIgmRaster.get() == NULL)
   {
      mProgress.report("Not a valid IGM file", 0, ERRORS, true);
      return false;
   }

   // Destroy the previous IGM raster element if the loaded raster element did not replace it
   Service<ModelServices> pModel;

   DataElement* pPreviousIgm = pModel->getElement(IGM_GEO_NAME, TypeConverter::toString<RasterElement>(), mpRaster);
   if (pPreviousIgm != mpIgmRaster.get())
   {
      pModel->destroyElement(pPreviousIgm);
   }

   // Rename and reparent the element to be a child of the raster element being georeferenced
   pModel->setElementParent(mpIgmRaster.get(), mpRaster);
   pModel->setElementName(mpIgmRaster.get(), IGM_GEO_NAME);

   return true;
}
