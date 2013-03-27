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
#include "Executable.h"
#include "Georeference.h"
#include "GeoreferenceDescriptor.h"
#include "GeoreferenceDlg.h"
#include "GeoreferencePlugIn.h"
#include "LatLonLayer.h"
#include "LayerList.h"
#include "MessageLogResource.h"
#include "PlugIn.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "PlugInResource.h"
#include "Progress.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterLayer.h"
#include "SpatialDataView.h"
#include "StringUtilities.h"
#include "Undo.h"

REGISTER_PLUGIN_BASIC(OpticksGeoreference, GeoreferencePlugIn);

GeoreferencePlugIn::GeoreferencePlugIn() :
   mpProgress(NULL),
   mpRaster(NULL),
   mpView(NULL),
   mCreateLayer(GeoreferenceDescriptor::getSettingCreateLayer()),
   mLayerName("GEO_RESULTS"),
   mDisplayLayer(GeoreferenceDescriptor::getSettingDisplayLayer()),
   mGeocoordType(GeoreferenceDescriptor::getSettingGeocoordType()),
   mLatLonFormat(GeoreferenceDescriptor::getSettingLatLonFormat())
{
   setName("Georeference");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setMenuLocation("[Geo]\\Georeference");
   setDescriptorId("{EB9F4BE5-4E9E-4f54-BC16-A2B33B535CEA}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

GeoreferencePlugIn::~GeoreferencePlugIn()
{}

bool GeoreferencePlugIn::execute(PlugInArgList* pInParam, PlugInArgList* pOutParam)
{
   StepResource pStep("Run GeoReference", "app", "DE61E049-5B44-41A8-BE10-6DA2FFE23E0C");
   pStep->addProperty("name", getName());

   mpProgress = pInParam->getPlugInArgValue<Progress>(Executable::ProgressArg());
   mpRaster = pInParam->getPlugInArgValue<RasterElement>(Executable::DataElementArg());
   mpView = pInParam->getPlugInArgValue<SpatialDataView>(Executable::ViewArg());

   if (mpRaster == NULL)
   {
      std::string message = "No data cube passed to the plug-in.";
      if (mpProgress)
      {
         mpProgress->updateProgress(message, 0, ERRORS);
      }
      pStep->finalize(Message::Failure, message);
      return false;
   }

   std::string sourceDatasetName = mpRaster->getFilename();
   pStep->addProperty("sourceDataset", sourceDatasetName);

   PlugIn* pGeoPlugin = NULL;
   if (!isBatch())
   {
      FactoryResource<Filename> fn;
      if (fn.get() == NULL)
      {
         // the dialog could not be properly created
         std::string message = "Unable to allocate memory for a Filename object!";
         if (mpProgress)
         {
            mpProgress->updateProgress(message, 0, ERRORS);
         }
         pStep->finalize(Message::Failure, message);
         return false;
      }
      fn->setFullPathAndName(sourceDatasetName);
      QString geoDlgTitle = QString("Georeference: ") + QString::fromStdString(fn->getFileName());

      RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(mpRaster->getDataDescriptor());
      if (pDescriptor == NULL)
      {
         std::string message = "Could not get the raster data information from the data set.";
         if (mpProgress)
         {
            mpProgress->updateProgress(message, 0, ERRORS);
         }
         pStep->finalize(Message::Failure, message);
         return false;
      }

      // Create a copy of the raster data descriptor for the dialog to modify
      DataDescriptorResource<RasterDataDescriptor> pEditDescriptor(dynamic_cast<RasterDataDescriptor*>(
         pDescriptor->copy()));
      VERIFY(pEditDescriptor.get() != NULL);

      // Reset the georeference parameters to the default values
      FactoryResource<GeoreferenceDescriptor> pDefaultGeorefDescriptor;
      pEditDescriptor->setGeoreferenceDescriptor(pDefaultGeorefDescriptor.get());
      pEditDescriptor->setDefaultGeoreferencePlugIn();

      // Display the dialog, where the user edits the georeference parameters in the copied data descriptor
      GeoreferenceDlg dlgGeo(Service<DesktopServices>()->getMainWidget());
      dlgGeo.setWindowTitle(geoDlgTitle);
      dlgGeo.setDataDescriptor(pEditDescriptor.get());
      if (dlgGeo.exec() == QDialog::Rejected)
      {
         pStep->finalize(Message::Failure, "Georeferencing canceled");
         return false;
      }

      // Copy the edited georeference parameters to the raster data descriptor
      GeoreferenceDescriptor* pGeorefDescriptor = pDescriptor->getGeoreferenceDescriptor();
      VERIFY(pGeorefDescriptor != NULL);

      const GeoreferenceDescriptor* pEditGeorefDescriptor = pEditDescriptor->getGeoreferenceDescriptor();
      VERIFY(pEditGeorefDescriptor != NULL);

      if (pGeorefDescriptor->clone(pEditGeorefDescriptor) == false)
      {
         std::string message = "Could not set the georeference parameters into the data set.";
         if (mpProgress)
         {
            mpProgress->updateProgress(message, 0, ERRORS);
         }
         pStep->finalize(Message::Failure, message);
         return false;
      }

      // Read the values from the descriptor
      std::string plugInName = pGeorefDescriptor->getGeoreferencePlugInName();
      mLayerName = pGeorefDescriptor->getLayerName();
      mCreateLayer = pGeorefDescriptor->getCreateLayer();
      mDisplayLayer = pGeorefDescriptor->getDisplayLayer();
      mGeocoordType = pGeorefDescriptor->getGeocoordType();
      mLatLonFormat = pGeorefDescriptor->getLatLonFormat();

      // Execute the selected plug-in
      ExecutableResource pGeoExecutable(plugInName);
      if (pGeoExecutable.get() != NULL)
      {
         PlugInArgList& argList = pGeoExecutable->getInArgList();
         argList.setPlugInArgValue(Executable::DataElementArg(), mpRaster);
         argList.setPlugInArgValue(Executable::ProgressArg(), mpProgress);
         argList.setPlugInArgValueLoose(Executable::ViewArg(), mpView);

         if (pGeoExecutable->execute() == false)
         {
            std::string message = "The georeference plug-in did not complete successfully.";
            if (mpProgress)
            {
               mpProgress->updateProgress(message, 0, ERRORS);
            }
            pStep->finalize(Message::Failure, message);
            return false;
         }

         pGeoExecutable.release();
      }
      else
      {
         std::string message = "The georeference plug-in is invalid.";
         if (mpProgress)
         {
            mpProgress->updateProgress(message, 0, ERRORS);
         }

         pStep->finalize(Message::Failure, message);
         return false;
      }
   }
   else
   {
      pInParam->getPlugInArgValue("Results Name", mLayerName);
      pInParam->getPlugInArgValue("Create Layer", mCreateLayer);
      pInParam->getPlugInArgValue("Display Layer", mDisplayLayer);
      pInParam->getPlugInArgValue("Coordinate Type", mGeocoordType);
      pInParam->getPlugInArgValue("Latitude/Longitude Format", mLatLonFormat);
   }

   if (mpView != NULL)
   {
      // Delete existing lat/long layers displaying the raster element
      LayerList* pLayerList = mpView->getLayerList();
      if (pLayerList != NULL)
      {
         std::vector<Layer*> layers = pLayerList->getLayers(LAT_LONG);
         for (std::vector<Layer*>::iterator iter = layers.begin(); iter != layers.end(); ++iter)
         {
            LatLonLayer* pLayer = dynamic_cast<LatLonLayer*>(*iter);
            if (pLayer != NULL)
            {
               if (dynamic_cast<RasterElement*>(pLayer->getDataElement()) == mpRaster)
               {
                  mpView->deleteLayer(pLayer);
               }
            }
         }
      }

      // Create the new lat/long layer if necessary
      if (mCreateLayer == true)
      {
         UndoGroup undoGroup(mpView, "Add Latitude/Longitude Layer");

         LatLonLayer* pLatLonLayer = dynamic_cast<LatLonLayer*>(mpView->createLayer(LAT_LONG, mpRaster, mLayerName));
         if (pLatLonLayer == NULL)
         {
            std::string warningTxt = "Could not create the latitude/longitude layer.";
            if (mLayerName.empty())
            {
               warningTxt += " The input layer name was blank.";
            }
            if (mpProgress != NULL)
            {
               mpProgress->updateProgress(warningTxt, 99, WARNING);
            }
            pStep->addMessage(warningTxt, "app", "84B6CB10-9902-4ECA-BD9B-0BF6DEDF288D");
         }
         else
         {
            // Set the layer offset and scale factor to that of the corresponding raster layer
            LayerList* pLayerList = mpView->getLayerList();
            if (pLayerList != NULL)
            {
               RasterLayer* pRasterLayer = dynamic_cast<RasterLayer*>(pLayerList->getLayer(RASTER, mpRaster));
               if (pRasterLayer != NULL)
               {
                  pLatLonLayer->setXOffset(pRasterLayer->getXOffset());
                  pLatLonLayer->setYOffset(pRasterLayer->getYOffset());
                  pLatLonLayer->setXScaleFactor(pRasterLayer->getXScaleFactor());
                  pLatLonLayer->setYScaleFactor(pRasterLayer->getYScaleFactor());
               }
            }

            pStep->addProperty("resultsName", mLayerName);
            pStep->addProperty("geocoordType", mGeocoordType);
            pStep->addProperty("latLonFormat", mLatLonFormat);
            pLatLonLayer->setGeocoordType(mGeocoordType);
            pLatLonLayer->setLatLonFormat(mLatLonFormat);

            if (pOutParam != NULL)
            {
               pOutParam->setPlugInArgValue<LatLonLayer>("Latitude/Longitude Layer", pLatLonLayer);
            }

            if (mDisplayLayer)
            {
               mpView->showLayer(pLatLonLayer);
            }
            else
            {
               mpView->hideLayer(pLatLonLayer);
            }
         }
      }
   }

   // Update the georeference descriptor with the current georeference parameters
   RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(mpRaster->getDataDescriptor());
   if (pDescriptor != NULL)
   {
      GeoreferenceDescriptor* pGeorefDescriptor = pDescriptor->getGeoreferenceDescriptor();
      if (pGeorefDescriptor != NULL)
      {
         pGeorefDescriptor->setLayerName(mLayerName);
         pGeorefDescriptor->setCreateLayer(mCreateLayer);
         pGeorefDescriptor->setDisplayLayer(mDisplayLayer);
         pGeorefDescriptor->setGeocoordType(mGeocoordType);
         pGeorefDescriptor->setLatLonFormat(mLatLonFormat);
      }
   }

   std::string message = "Georeference completed.";
   if (mpProgress)
   {
      mpProgress->updateProgress(message, 100, NORMAL);
   }
   pStep->finalize(Message::Success);

   return true;
}

bool GeoreferencePlugIn::getInputSpecification(PlugInArgList*& pArgList)
{
   pArgList = Service<PlugInManagerServices>()->getPlugInArgList();
   VERIFY(pArgList != NULL);

   VERIFY(pArgList->addArg<Progress>(Executable::ProgressArg(), NULL, Executable::ProgressArgDescription()));
   VERIFY(pArgList->addArg<RasterElement>(Executable::DataElementArg(), NULL, "Data element to be georeferenced."));
   VERIFY(pArgList->addArg<SpatialDataView>(Executable::ViewArg(), NULL, "View in which the georeferencing results "
      "are displayed."));

   if (isBatch())
   {
      // Do not set a default value in the input args so that the georeference
      // descriptor values will be used if the arg value is not set
      VERIFY(pArgList->addArg<std::string>("Results Name", "Name for the layer created as a result of the "
         "georeferencing.  If not specified, the layer name contained in the " + Executable::DataElementArg() +
         " input will be used."));
      VERIFY(pArgList->addArg<bool>("Create Layer", "Whether or not to create a results layer.  If not "
         "specified, the value contained in the " + Executable::DataElementArg() + " input will be used."));
      VERIFY(pArgList->addArg<bool>("Display Layer", "If a results layer is created, whether or not to display "
         "the new layer.  If not specified, the value contained in the " + Executable::DataElementArg() +
         " input will be used.  This value is ignored if the results layer is not created."));
      VERIFY(pArgList->addArg<GeocoordType>("Coordinate Type", "The geographic coordinate type that should "
         "be displayed in the results layer.  If not specified, the coordinate type contained in the " +
         Executable::DataElementArg() + " input will be used."));
      VERIFY(pArgList->addArg<DmsFormatType>("Latitude/Longitude Format", &mLatLonFormat, "If a results layer "
         "with the " + StringUtilities::toDisplayString(GEOCOORD_LATLON) + " coordinate type is created, the "
         "format in which the values should be displayed.  If not specified, the latitude/longitude format "
         "contained in the " + Executable::DataElementArg() + " input will be used.  This value is ignored if "
         "the coordinate type is not " + StringUtilities::toDisplayString(GEOCOORD_LATLON) + "."));
   }

   return true;
}

bool GeoreferencePlugIn::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = Service<PlugInManagerServices>()->getPlugInArgList();
   VERIFY(pArgList != NULL);

   VERIFY(pArgList->addArg<LatLonLayer>("Latitude/Longitude Layer", NULL, "The layer created to display the "
      "georeferencing results."));
   return true;
}
