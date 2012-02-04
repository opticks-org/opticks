/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AnnotationLayer.h"
#include "AppConfig.h"
#include "AppAssert.h"
#include "GeoAlgorithms.h"
#include "GraphicLayer.h"
#include "GraphicObject.h"
#include "LayerList.h"
#include "NorthArrowObjectImp.h"
#include "ProductView.h"
#include "RasterElement.h"
#include "SpatialDataView.h"

#include <string>
using namespace std;

NorthArrowObjectImp::NorthArrowObjectImp(const string& id, GraphicObjectType type, GraphicLayer* pLayer,
                                         LocationType pixelCoord) :
   DirectionalArrowObjectImp(id, type, pLayer, pixelCoord)
{
   // Load the arrow from the file
   string filename = "";
   const Filename* pSupportFilesPath = ConfigurationSettings::getSettingSupportFilesPath();
   if (pSupportFilesPath != NULL)
   {
      filename = pSupportFilesPath->getFullPathAndName() + SLASH + "Annotation" + SLASH + "North.ano";
   }

   if (!import(filename))
   {
      throw AssertException(filename + " is not a valid north arrow object!");
   }

   // Update the bounding box to be centered around the given point
   LocationType llCorner = getLlCorner();
   LocationType urCorner = getUrCorner();

   LocationType pixelSize = getPixelSize();
   double dWidth = (urCorner.mX - llCorner.mX) / pixelSize.mX;
   double dHeight = (urCorner.mY - llCorner.mY) / pixelSize.mY;

   llCorner.mX = pixelCoord.mX - dWidth / 2;
   llCorner.mY = pixelCoord.mY - dHeight / 2;
   urCorner.mX = pixelCoord.mX + dWidth / 2;
   urCorner.mY = pixelCoord.mY + dHeight / 2;

   setBoundingBox(llCorner, urCorner);

   // Update the selection handles
   updateHandles();

   // Orient the arrow
   orient();
}

const string& NorthArrowObjectImp::getObjectType() const
{
   static string type("NorthArrowObjectImp");
   return type;
}

bool NorthArrowObjectImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "NorthArrowObject"))
   {
      return true;
   }

   return DirectionalArrowObjectImp::isKindOf(className);
}

string NorthArrowObjectImp::getFileIndicator() const
{
   return string("northarrow");
}

void NorthArrowObjectImp::orient()
{
   if (isOriented() == true)
   {
      return;
   }

   GraphicLayer* pLayer = NULL;
   pLayer = getLayer();
   if (pLayer == NULL)
   {
      return;
   }

   View* pView = NULL;
   pView = pLayer->getView();
   if (pView == NULL)
   {
      return;
   }

   SpatialDataView* pSpatialDataView = NULL;
   if (pView->isKindOf("SpatialDataView") == true)
   {
      pSpatialDataView = static_cast<SpatialDataView*> (pView);
   }
   else if (pView->isKindOf("ProductView") == true)
   {
      ProductView* pProductView = static_cast<ProductView*> (pView);

      AnnotationLayer* pLayoutLayer = pProductView->getLayoutLayer();
      if (pLayoutLayer == pLayer)
      {
         list<GraphicObject*> viewObjects;
         pLayoutLayer->getObjects(VIEW_OBJECT, viewObjects);

         list<GraphicObject*>::iterator iter = viewObjects.begin();
         while (iter != viewObjects.end())
         {
            GraphicObject* pObject = NULL;
            pObject = *iter;
            if (pObject != NULL)
            {
               View* pObjectView = NULL;
               pObjectView = pObject->getObjectView();
               if (pObjectView != NULL)
               {
                  if (pObjectView->isKindOf("SpatialDataView") == true)
                  {
                     pSpatialDataView = static_cast<SpatialDataView*> (pObjectView);
                  }
               }
            }

            ++iter;
         }
      }
   }

   if (pSpatialDataView == NULL)
   {
      return;
   }

   LayerList* pLayerList = pSpatialDataView->getLayerList();
   VERIFYNRV(pLayerList != NULL);
   RasterElement* pRaster = pLayerList->getPrimaryRasterElement();
   VERIFYNRV(pLayerList != NULL);
   if (!pRaster->isGeoreferenced())
   {
      return;
   }

   // Calculate the angle of the object relative to the pixel coordinates
   updateHandles();

   // Create pixelStart and set it to the center of the bounding box
   LocationType pixelStart;
   pixelStart.mX = mHandles[1].mX;
   pixelStart.mY = mHandles[3].mY;

   ProductView* pProductView = dynamic_cast<ProductView*>(pView);
   if (pProductView != NULL)
   {
      // Convert to the screen coordinate system
      double dScreenX = 0;
      double dScreenY = 0;
      pLayer->translateDataToWorld(pixelStart.mX, pixelStart.mY, pixelStart.mX, pixelStart.mY);
      pProductView->translateWorldToScreen(pixelStart.mX, pixelStart.mY, dScreenX, dScreenY);

      // Convert to the spatial data view coordinate system
      pSpatialDataView->translateScreenToWorld(dScreenX,
         dScreenY, pixelStart.mX, pixelStart.mY);
      pLayer->translateWorldToData(pixelStart.mX, pixelStart.mY, pixelStart.mX, pixelStart.mY);
   }

   double dAngle;
   if (!GeoAlgorithms::getAngleToNorth(pRaster, dAngle, pixelStart))
   {
      return;
   }

   // Update the angle if the object is in the layout layer
   if (pProductView != NULL)
   {
      // Rotation
      dAngle -= pSpatialDataView->getRotation();

      // Pitch
      double dPitch = pSpatialDataView->getPitch();
      if (dPitch > 0.0)
      {
         dAngle *= -1.0;
      }
   }

   // Subtract 180 degrees since north is up in the object
   dAngle -= 180.0;

   // Rotate the object
   setRotation(dAngle);

   // Update the orientation flag
   DirectionalArrowObjectImp::orient();
}
