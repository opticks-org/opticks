/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "AppVerify.h"
#include "GeoreferenceDescriptor.h"
#include "LatLonLayer.h"
#include "LayerList.h"
#include "MessageLogResource.h"
#include "PlugInArgList.h"
#include "PlugInRegistration.h"
#include "RasterElement.h"
#include "SetViewDisplayArea.h"
#include "SpatialDataView.h"
#include "TypesFile.h"
#include "ZoomAndPanToPoint.h"

REGISTER_PLUGIN_BASIC(OpticksWizardItems, SetViewDisplayArea);

SetViewDisplayArea::SetViewDisplayArea() :
   mpView(NULL),
   mpRaster(NULL),
   mZoom(0.0)
{
   setName("Set View Display Area");
   setVersion(APP_VERSION_NUMBER);
   setCreator("Ball Aerospace & Technologies, Corp.");
   setCopyright(APP_COPYRIGHT);
   setDescription("Sets the center location and zoom level for a view.");
   setDescriptorId("{575036C9-685B-43FB-96A2-3AD8BA40DF2D}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

SetViewDisplayArea::~SetViewDisplayArea()
{}

bool SetViewDisplayArea::setBatch()
{
   DesktopItems::setBatch();
   return true;
}

bool SetViewDisplayArea::getInputSpecification(PlugInArgList*& pArgList)
{
   if (DesktopItems::getInputSpecification(pArgList) == false)
   {
      return false;
   }

   VERIFY(pArgList != NULL);
   VERIFY(pArgList->addArg<PerspectiveView>(Executable::ViewArg(), "The view for which to set its displayed area."));

   if (isBatch() == true)
   {
      VERIFY(pArgList->addArg<double>("Pixel X", "An optional argument specifying the one-based pixel x-coordinate "
         "for the center of the view.  If both latitude and longitude arguments are valid, this value is ignored."));
      VERIFY(pArgList->addArg<double>("Pixel Y", "An optional argument specifying the one-based pixel y-coordinate "
         "for the center of the view.  If both latitude and longitude arguments are valid, this value is ignored."));
      VERIFY(pArgList->addArg<double>("Latitude", "An optional argument specifying the latitude for the center "
         "of the view.  If both latitude and longitude are valid, the pixel coordinate arguments are ignored."));
      VERIFY(pArgList->addArg<double>("Longitude", "An optional argument specifying the longitude for the center "
         "of the view.  If both latitude and longitude are valid, the pixel coordinate arguments are ignored."));
      VERIFY(pArgList->addArg<double>("Zoom Percentage", "An optional argument specifying the zoom percentage "
         "for the view."));
   }

   return true;
}

bool SetViewDisplayArea::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

bool SetViewDisplayArea::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Execute Wizard Item", "app", "A104C310-CF8D-48AB-A2BF-A34C47E3FB59");
   pStep->addProperty("Item", getName());
   mpStep = pStep.get();

   if (extractInputArgs(pInArgList) == false)
   {
      return false;
   }

   VERIFY(mpView != NULL);

   if (isBatch() == false)
   {
      GeocoordType geocoordType;

      SpatialDataView* pSpatialDataView = dynamic_cast<SpatialDataView*>(mpView);
      if (pSpatialDataView != NULL)
      {
         // Get the default geocoordinate type from top lat/long layer
         LatLonLayer* pLatLonLayer = dynamic_cast<LatLonLayer*>(pSpatialDataView->getTopMostLayer(LAT_LONG));
         if (pLatLonLayer != NULL)
         {
            geocoordType = pLatLonLayer->getGeocoordType();
         }
         else if (mpRaster != NULL && mpRaster->isGeoreferenced() == true)
         {
            geocoordType = GeoreferenceDescriptor::getSettingGeocoordType();
         }
      }

      ZoomAndPanToPointDlg dialog(mpRaster, geocoordType, mpDesktop->getMainWidget());
      dialog.setZoomPct(mpView->getZoomPercentage());

      if (dialog.exec() == QDialog::Rejected)
      {
         return false;
      }

      mPixelCenter = dialog.getCenter();
      mZoom = dialog.getZoomPct();
   }

   mpView->zoomToPoint(mPixelCenter, mZoom);
   mpView->refresh();

   reportComplete();
   return true;
}

bool SetViewDisplayArea::extractInputArgs(PlugInArgList* pInArgList)
{
   if (DesktopItems::extractInputArgs(pInArgList) == false)
   {
      reportError("Unable to extract the input arguments.", "AD026324-EDB0-41FB-9C30-CF1147032707");
      return false;
   }

   // View
   mpView = pInArgList->getPlugInArgValue<PerspectiveView>(Executable::ViewArg());
   if (mpView == NULL)
   {
      reportError("The view input argument is invalid.", "FAD2656B-29C6-4933-8E47-63F7FB628108");
      return false;
   }

   // Get the raster element used for coordinate conversion from the view
   SpatialDataView* pSpatialDataView = dynamic_cast<SpatialDataView*>(mpView);
   if (pSpatialDataView != NULL)
   {
      LayerList* pLayerList = pSpatialDataView->getLayerList();
      VERIFY(pLayerList != NULL);

      mpRaster = pLayerList->getPrimaryRasterElement();
   }

   if (isBatch() == true)
   {
      bool coordinateError = false;
      mPixelCenter = mpView->getVisibleCenter();

      // Pixel coordinate
      double xCoord = 0.0;
      double yCoord = 0.0;
      bool hasPixelX = pInArgList->getPlugInArgValue<double>("Pixel X", xCoord);
      bool hasPixelY = pInArgList->getPlugInArgValue<double>("Pixel Y", yCoord);

      if ((hasPixelX == true) && (hasPixelY == true))
      {
         mPixelCenter.mX = xCoord - 1.0;
         mPixelCenter.mY = yCoord - 1.0;
      }
      else if ((hasPixelX == true) || (hasPixelY == true))
      {
         coordinateError = true;
      }

      // Latitude/longitude coordinate
      double latitude = 0.0;
      double longitude = 0.0;
      bool hasLatitude = pInArgList->getPlugInArgValue<double>("Latitude", latitude);
      bool hasLongitude = pInArgList->getPlugInArgValue<double>("Longitude", longitude);

      if ((hasLatitude == true) && (hasLongitude == true))
      {
         if (mpRaster != NULL && mpRaster->isGeoreferenced() == true)
         {
            mPixelCenter = mpRaster->convertGeocoordToPixel(LocationType(latitude, longitude));
         }
         else
         {
            reportError("Could not convert the latitude/longitude coordinate to a pixel coordinate.",
               "8A767A00-BFCE-425D-9953-8363555E0A80");
            return false;
         }

         coordinateError = false;
      }
      else if (((hasLatitude == true) || (hasLongitude == true)) && ((hasPixelX == false) || (hasPixelY == false)))
      {
         coordinateError = true;
      }

      if (coordinateError == true)
      {
         reportWarning("One or more of the center coordinate input arguments was not provided.  "
            "The center location will not be changed.", "C73929BD-4EB8-43E4-8F5E-9B0D4A6059EE");
      }

      // Zoom percentage
      if (pInArgList->getPlugInArgValue("Zoom Percentage", mZoom) == false)
      {
         mZoom = mpView->getZoomPercentage();
      }
   }

   return true;
}
