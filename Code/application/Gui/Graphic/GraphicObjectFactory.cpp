/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QMessageBox>

#include "AppAssert.h"
#include "AppVersion.h"
#include "ArcObjectAdapter.h"
#include "ArrowObjectAdapter.h"
#include "BitMaskObjectAdapter.h"
#include "CgmObjectAdapter.h"
#include "DimensionObjectAdapter.h"
#include "EastArrowObjectAdapter.h"
#include "EllipseObjectAdapter.h"
#include "FileImageObjectAdapter.h"
#include "FrameLabelObjectAdapter.h"
#include "GraphicGroupAdapter.h"
#include "GraphicLayer.h"
#include "GraphicLayerImp.h"
#include "GraphicObject.h"
#include "GraphicObjectFactory.h"
#include "GraphicObjectImp.h"
#include "LatLonInsertObjectAdapter.h"
#include "LineObjectAdapter.h"
#include "MeasurementObjectAdapter.h"
#include "MoveObjectImp.h"
#include "MultipointObjectAdapter.h"
#include "NorthArrowObjectAdapter.h"
#include "PolygonObjectAdapter.h"
#include "PolylineObjectAdapter.h"
#include "Progress.h"
#include "RawImageObjectAdapter.h"
#include "RectangleObjectAdapter.h"
#include "RoundedRectangleObjectAdapter.h"
#include "ScaleBarObjectAdapter.h"
#include "StringUtilities.h"
#include "TextObjectAdapter.h"
#include "TrailObjectAdapter.h"
#include "TriangleObjectAdapter.h"
#include "ViewObjectAdapter.h"
#include "WidgetImageObjectAdapter.h"

#include <string>

GraphicObject* GraphicObjectFactory::createObject(GraphicObjectType objectType, GraphicLayer* pLayer,
                                                  LocationType pixelCoord)
{
   std::list<GraphicObject*> objects = createObjects(1, objectType, pLayer, pixelCoord);
   if (objects.empty() == false)
   {
      VERIFYRV(objects.size() == 1, NULL);
      return objects.front();
   }

   return NULL;
}

std::list<GraphicObject*> GraphicObjectFactory::createObjects(unsigned int numObjects, GraphicObjectType objectType,
                                                              GraphicLayer* pLayer, LocationType pixelCoord,
                                                              Progress* pProgress)
{
   if ((numObjects == 0) || (objectType.isValid() == false))
   {
      return std::list<GraphicObject*>();
   }

   // Check if the layer supports the given object type
   GraphicLayerImp* pLayerImp = dynamic_cast<GraphicLayerImp*>(pLayer);
   if ((pLayerImp != NULL) && (pLayerImp->canContainGraphicObjectType(objectType) == false))
   {
      if (pProgress != NULL)
      {
         pProgress->updateProgress("The layer does not support the " + StringUtilities::toDisplayString(objectType) +
            " object type.", 0, ERRORS);
      }

      return std::list<GraphicObject*>();
   }

   // Get the number of existing objects of the given type in the layer
   unsigned int numExistingObjects = 0;
   if (pLayer != NULL)
   {
      numExistingObjects = pLayer->getNumObjects(objectType);
   }

   // Get a base name for the objects
   std::string objectTypeName = StringUtilities::toDisplayString(objectType);

   // Create the objects
   std::list<GraphicObject*> objects;
   for (unsigned int i = 0; i < numObjects; ++i)
   {
      if (pProgress != NULL)
      {
         pProgress->updateProgress("Creating objects...", static_cast<int>(i * 100 / numObjects), NORMAL);
      }

      GraphicObject* pObject = NULL;
      try
      {
         switch (objectType)
         {
         case ARC_OBJECT:
            pObject = new ArcObjectAdapter(SessionItemImp::generateUniqueId(), objectType, pLayer, pixelCoord);
            break;

         case ARROW_OBJECT:
            pObject = new ArrowObjectAdapter(SessionItemImp::generateUniqueId(), objectType, pLayer, pixelCoord);
            break;

         case CGM_OBJECT:
            pObject = new CgmObjectAdapter(SessionItemImp::generateUniqueId(), objectType, pLayer, pixelCoord);
            break;

         case EASTARROW_OBJECT:
            pObject = new EastArrowObjectAdapter(SessionItemImp::generateUniqueId(), objectType, pLayer, pixelCoord);
            break;

         case ELLIPSE_OBJECT:
            pObject = new EllipseObjectAdapter(SessionItemImp::generateUniqueId(), objectType, pLayer, pixelCoord);
            break;

         case GROUP_OBJECT:
            pObject = new GraphicGroupAdapter(SessionItemImp::generateUniqueId(), objectType, pLayer, pixelCoord);
            break;

         case FILE_IMAGE_OBJECT:
            pObject = new FileImageObjectAdapter(SessionItemImp::generateUniqueId(), objectType, pLayer, pixelCoord);
            break;

         case FRAME_LABEL_OBJECT:
            pObject = new FrameLabelObjectAdapter(SessionItemImp::generateUniqueId(), objectType, pLayer, pixelCoord);
            break;

         case LATLONINSERT_OBJECT:
            pObject = new LatLonInsertObjectAdapter(SessionItemImp::generateUniqueId(), objectType, pLayer, pixelCoord);
            break;

         case VLINE_OBJECT: // fall through
         case HLINE_OBJECT: // fall through
         case LINE_OBJECT:
            pObject = new LineObjectAdapter(SessionItemImp::generateUniqueId(), objectType, pLayer, pixelCoord);
            break;

         case MEASUREMENT_OBJECT:
            pObject = new MeasurementObjectAdapter(SessionItemImp::generateUniqueId(), objectType, pLayer, pixelCoord);
            break;

         case ROTATE_OBJECT: // fall through
         case MOVE_OBJECT:
            pObject = new MoveObjectAdapter(SessionItemImp::generateUniqueId(), objectType, pLayer, pixelCoord);
            break;

         case MULTIPOINT_OBJECT:
            pObject = new MultipointObjectAdapter(SessionItemImp::generateUniqueId(), objectType, pLayer, pixelCoord);
            break;

         case NORTHARROW_OBJECT:
            pObject = new NorthArrowObjectAdapter(SessionItemImp::generateUniqueId(), objectType, pLayer, pixelCoord);
            break;

         case POLYGON_OBJECT:
            pObject = new PolygonObjectAdapter(SessionItemImp::generateUniqueId(), objectType, pLayer, pixelCoord);
            break;

         case POLYLINE_OBJECT:
            pObject = new PolylineObjectAdapter(SessionItemImp::generateUniqueId(), objectType, pLayer, pixelCoord);
            break;

         case RAW_IMAGE_OBJECT:
            pObject = new RawImageObjectAdapter(SessionItemImp::generateUniqueId(), objectType, pLayer, pixelCoord);
            break;

         case RECTANGLE_OBJECT:
            pObject = new RectangleObjectAdapter(SessionItemImp::generateUniqueId(), objectType, pLayer, pixelCoord);
            break;

         case ROUNDEDRECTANGLE_OBJECT:
            pObject = new RoundedRectangleObjectAdapter(SessionItemImp::generateUniqueId(), objectType, pLayer,
               pixelCoord);
            break;

         case ROW_OBJECT: // fall through
         case COLUMN_OBJECT:
            pObject = new DimensionObjectAdapter(SessionItemImp::generateUniqueId(), objectType, pLayer, pixelCoord);
            break;

         case SCALEBAR_OBJECT:
            pObject = new ScaleBarObjectAdapter(SessionItemImp::generateUniqueId(), objectType, pLayer, pixelCoord);
            break;

         case TEXT_OBJECT:
            pObject = new TextObjectAdapter(SessionItemImp::generateUniqueId(), objectType, pLayer, pixelCoord);
            break;

         case TRAIL_OBJECT:
            pObject = new TrailObjectAdapter(SessionItemImp::generateUniqueId(), objectType, pLayer, pixelCoord);
            break;

         case TRIANGLE_OBJECT:
            pObject = new TriangleObjectAdapter(SessionItemImp::generateUniqueId(), objectType, pLayer, pixelCoord);
            break;

         case VIEW_OBJECT:
            pObject = new ViewObjectAdapter(SessionItemImp::generateUniqueId(), objectType, pLayer, pixelCoord);
            break;

         case WIDGET_IMAGE_OBJECT:
            pObject = new WidgetImageObjectAdapter(SessionItemImp::generateUniqueId(), objectType, pLayer, pixelCoord);
            break;

         case BITMASK_OBJECT:
            pObject = new BitMaskObjectAdapter(SessionItemImp::generateUniqueId(), objectType, pLayer, pixelCoord);
            break;

         default:
            break;
         }
      }
      catch (AssertException exception)
      {
         QMessageBox::critical(NULL, QString("%1 Graphic Factory").arg(APP_NAME),
            QString("An error occurred while creating the graphic object!\nCause: ") +
            QString::fromStdString(exception.getText()));
         pObject = NULL;
      }
      catch (...)
      {
         QMessageBox::critical(NULL, QString("%1 Graphic Factory").arg(APP_NAME),
            "An error occurred while creating the graphic object!");
         pObject = NULL;
      }

      if (pObject != NULL)
      {
         // Set a default object name
         std::string objectName = objectTypeName + " " + StringUtilities::toDisplayString(++numExistingObjects);
         pObject->setName(objectName);

         // Add the object to the list
         objects.push_back(pObject);
      }
   }

   return objects;
}
