/*
 * The information in this file is
 * Copyright(c) 2012 Ball Aerospace & Technologies Corporation
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
#include "GeoreferenceWidget.h"
#include "LayerList.h"
#include "PlugInArgList.h"
#include "PlugInResource.h"
#include "ProductView.h"
#include "ProductWindow.h"
#include "Progress.h"
#include "PropertiesGeoreference.h"
#include "RasterElement.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "TypesFile.h"

#include <QtGui/QMessageBox>

#include <string>
#include <vector>

PropertiesGeoreference::PropertiesGeoreference() :
   mpDescriptor(NULL)
{
   setName("Georeference Properties");
   setPropertiesName("Georeference");
   setDescription("Georeference parameters for a raster element");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT_MSG);
   setVersion(APP_VERSION_NUMBER);
   setDescriptorId("{FFB9B39B-C2FA-4EB6-A1F8-F0B23F3D3061}");
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

PropertiesGeoreference::~PropertiesGeoreference()
{}

bool PropertiesGeoreference::initialize(SessionItem* pSessionItem)
{
   GeoreferenceWidget* pGeoreferencePage = dynamic_cast<GeoreferenceWidget*>(getWidget());
   VERIFY(pGeoreferencePage != NULL);

   RasterElement* pElement = dynamic_cast<RasterElement*>(pSessionItem);
   if (pElement != NULL)
   {
      const RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(pElement->getDataDescriptor());
      if (pDescriptor != NULL)
      {
         mpDescriptor =
            DataDescriptorResource<RasterDataDescriptor>(dynamic_cast<RasterDataDescriptor*>(pDescriptor->copy()));
         if (mpDescriptor.get() != NULL)
         {
            pGeoreferencePage->setDataDescriptor(mpDescriptor.get());
            return PropertiesShell::initialize(pSessionItem);
         }
      }
   }

   return false;
}

bool PropertiesGeoreference::applyChanges()
{
   if (mpDescriptor.get() == NULL)
   {
      return false;
   }

   // Validate the current georeference parameters before applying the changes
   GeoreferenceWidget* pGeoreferencePage = dynamic_cast<GeoreferenceWidget*>(getWidget());
   VERIFY(pGeoreferencePage != NULL);

   Georeference* pGeoreference = pGeoreferencePage->getSelectedPlugIn();
   if (pGeoreference != NULL)
   {
      std::string errorMessage;
      if (pGeoreference->validate(mpDescriptor.get(), errorMessage) == false)
      {
         QMessageBox::critical(pGeoreferencePage, "Georeference", QString::fromStdString(errorMessage));
         return false;
      }
      else if (errorMessage.empty() == false)
      {
         QString message = "The georeference plug-in returned the following warning.  Do you want to continue "
            "with the georeference?\n\n" + QString::fromStdString(errorMessage);
         if (QMessageBox::warning(pGeoreferencePage, "Georeference", message,
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
         {
            return false;
         }
      }
   }

   // Apply the georeference changes to the raster element and execute the georeference plug-in
   RasterElement* pElement = dynamic_cast<RasterElement*>(getSessionItem());
   if (pElement != NULL)
   {
      RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(pElement->getDataDescriptor());
      if (pDescriptor != NULL)
      {
         GeoreferenceDescriptor* pGeorefDescriptor = pDescriptor->getGeoreferenceDescriptor();
         if (pGeorefDescriptor != NULL)
         {
            // Copy the temporary georeference parameters owned by the properties plug-in
            // into the georeference descriptor contained in the raster data descriptor
            if (pGeorefDescriptor->clone(mpDescriptor->getGeoreferenceDescriptor()) == false)
            {
               QMessageBox::critical(pGeoreferencePage, "Georeference", "Could not apply the georeference changes "
                  "to the raster element.");
               return false;
            }

            // Perform the georeference
            const std::string& plugInName = pGeorefDescriptor->getGeoreferencePlugInName();
            if (plugInName.empty() == true)
            {
               pElement->setGeoreferencePlugin(NULL);
               return true;   // Return true since the georeference parameters were already successfully
                              // applied to the raster element
            }

            // Must create a new Georeference plug-in instance instead of executing the Georeference plug-in
            // contained in the GeoreferenceWidget because the widget always destroys its plug-in instances
            // when it is destroyed
            ExecutableResource geoPlugIn(plugInName, std::string(), NULL, true);
            geoPlugIn->getInArgList().setPlugInArgValue(Executable::DataElementArg(), pElement);
            if (geoPlugIn->execute() == false)
            {
               QString errorMessage = "The georeference parameters could not be applied to the raster element.";

               Progress* pProgress = geoPlugIn->getProgress();
               if (pProgress != NULL)
               {
                  std::string messageText;
                  int percent = 0;
                  ReportingLevel level;

                  pProgress->getProgress(messageText, percent, level);
                  if (messageText.empty() == false)
                  {
                     errorMessage += "\n" + QString::fromStdString(messageText);
                  }
               }

               QMessageBox::warning(pGeoreferencePage, "Georeference", errorMessage);
               pElement->setGeoreferencePlugin(NULL);
               return true;   // Return true since the georeference parameters were already successfully
                              // applied to the raster element
            }

            geoPlugIn.release();

            // Create/update the results layer
            SpatialDataView* pView = NULL;

            // Search all available spatial data windows for the view displaying the raster element
            std::vector<Window*> windows;
            Service<DesktopServices>()->getWindows(SPATIAL_DATA_WINDOW, windows);
            for (std::vector<Window*>::const_iterator iter = windows.begin(); iter != windows.end(); ++iter)
            {
               SpatialDataWindow* pWindow = dynamic_cast<SpatialDataWindow*>(*iter);
               if (pWindow != NULL)
               {
                  SpatialDataView* pCurrentView = pWindow->getSpatialDataView();
                  if (pCurrentView != NULL)
                  {
                     LayerList* pLayerList = pCurrentView->getLayerList();
                     if (pLayerList != NULL)
                     {
                        if (pLayerList->getLayer(RASTER, pElement) != NULL)
                        {
                           pView = pCurrentView;
                           break;
                        }
                     }
                  }
               }
            }

            if (pView == NULL)
            {
               // Could not find the spatial data view displaying the raster element,
               // so try searching active edit views in product windows
               Service<DesktopServices>()->getWindows(PRODUCT_WINDOW, windows);
               for (std::vector<Window*>::const_iterator iter = windows.begin(); iter != windows.end(); ++iter)
               {
                  ProductWindow* pWindow = dynamic_cast<ProductWindow*>(*iter);
                  if (pWindow != NULL)
                  {
                     ProductView* pProductView = pWindow->getProductView();
                     if (pProductView != NULL)
                     {
                        SpatialDataView* pCurrentView =
                           dynamic_cast<SpatialDataView*>(pProductView->getActiveEditView());
                        if (pCurrentView != NULL)
                        {
                           LayerList* pLayerList = pCurrentView->getLayerList();
                           if (pLayerList != NULL)
                           {
                              if (pLayerList->getLayer(RASTER, pElement) != NULL)
                              {
                                 pView = pCurrentView;
                                 break;
                              }
                           }
                        }
                     }
                  }
               }
            }

            bool createLayer = pGeorefDescriptor->getCreateLayer();
            std::string layerName = pGeorefDescriptor->getLayerName();
            bool displayLayer = pGeorefDescriptor->getDisplayLayer();
            GeocoordType geocoordType = pGeorefDescriptor->getGeocoordType();
            DmsFormatType latLonFormat = pGeorefDescriptor->getLatLonFormat();

            ExecutableResource geoDisplayPlugIn("Georeference", std::string(), NULL, true);
            PlugInArgList& inArgList = geoDisplayPlugIn->getInArgList();
            VERIFY(inArgList.setPlugInArgValue(Executable::DataElementArg(), pElement));
            VERIFY(inArgList.setPlugInArgValue(Executable::ViewArg(), pView));
            VERIFY(inArgList.setPlugInArgValue("Create Layer", &createLayer));
            VERIFY(inArgList.setPlugInArgValue("Results Name", &layerName));
            VERIFY(inArgList.setPlugInArgValue("Display Layer", &displayLayer));
            VERIFY(inArgList.setPlugInArgValue("Coordinate Type", &geocoordType));
            VERIFY(inArgList.setPlugInArgValue("Latitude/Longitude Format", &latLonFormat));

            if (geoDisplayPlugIn->execute() == false)
            {
               QMessageBox::warning(pGeoreferencePage, "Georeference", "Could not create/update the georeference "
                  "results layer.");
            }

            return true;
         }
      }
   }

   return false;
}

QWidget* PropertiesGeoreference::createWidget()
{
   QWidget* pWidget = new GeoreferenceWidget();
   return pWidget;
}
