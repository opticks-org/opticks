/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"
#include "AppVerify.h"
#include "DrawUtil.h"
#include "GraphicLayer.h"
#include "GraphicLayerImp.h"
#include "MoveObjectImp.h"
#include "Undo.h"
#include "View.h"
#include "ViewImp.h"

#include <list>
using namespace std;

namespace 
{
   class MoveObjects
   {
   public:
      MoveObjects(LocationType delta) : mDelta(delta) {}

      void operator()(GraphicObject *pObj) const
      {
         dynamic_cast<GraphicObjectImp*>(pObj)->move(mDelta);
      }
   private:
      LocationType mDelta;
   };

   class MoveHandles
   {
   public:
      MoveHandles(GraphicObject* pObject, LocationType coord, int handle, bool maintainAspect) :
         mHandle(handle),
         mMaintainAspect(maintainAspect)
      {
         LocationType startCoord = dynamic_cast<GraphicObjectImp*>(pObject)->getHandle(handle);
         mRelCoord = coord - startCoord;
      }

      void operator()(GraphicObject* pObj) const
      {
         GraphicObjectImp* pObjImp = dynamic_cast<GraphicObjectImp*>(pObj);
         VERIFYNRV(pObjImp != NULL);
         LocationType adjustedCoord = pObjImp->getHandle(mHandle) + mRelCoord;
         LocationType handlePos = pObjImp->getHandle(mHandle);
         LocationType oldCenter;
         double theta = pObj->getRotation();
         if (theta != 0.0)
         {
            LocationType ll = pObj->getLlCorner();
            LocationType ur = pObj->getUrCorner();
            oldCenter.mX = (ll.mX + ur.mX) / 2;
            oldCenter.mY = (ll.mY + ur.mY) / 2;
            adjustedCoord = DrawUtil::getRotatedCoordinate(
               adjustedCoord, oldCenter, -theta);
         }
         pObjImp->moveHandle(mHandle, adjustedCoord, mMaintainAspect);
         
         if (theta != 0.0)
         {
            LocationType newLlCorner = pObj->getLlCorner();
            LocationType newUrCorner = pObj->getUrCorner();
            LocationType newCenter((newUrCorner.mX + newLlCorner.mX) / 2, (newUrCorner.mY + newLlCorner.mY) / 2);

            LocationType objectRelativeLocation;
            objectRelativeLocation.mX = newCenter.mX - oldCenter.mX;
            objectRelativeLocation.mY = newCenter.mY - oldCenter.mY;

            // Fix location, since the object rotates around its center
            LocationType delta;
            double sinTheta = sin(theta*PI/180.0);
            double cosTheta = cos(theta*PI/180.0);
            delta.mX = (cosTheta - 1.0) * objectRelativeLocation.mX;
            delta.mX -= sinTheta * objectRelativeLocation.mY;
            delta.mY = sinTheta * objectRelativeLocation.mX;
            delta.mY += (cosTheta - 1.0) * objectRelativeLocation.mY;
            dynamic_cast<GraphicObjectImp*>(pObj)->move(delta);
         }
      }
   private:
      LocationType mRelCoord;
      int mHandle;
      bool mMaintainAspect;
   };

   class RotateHandles
   {
   public:
      RotateHandles(GraphicObject *pObject, LocationType coord, int handle)
         : mpObject(pObject)
      {
         GraphicObjectImp* pObjImp = dynamic_cast<GraphicObjectImp*>(mpObject);
         double startRot = pObject->getRotation();
         pObjImp->rotateHandle(handle, coord);
         double stopRot = pObject->getRotation();
         mDeltaRot = stopRot-startRot;
      }

      void operator()(GraphicObject *pObj) const
      {
         if (pObj != mpObject)
         {
            pObj->setRotation(pObj->getRotation()+mDeltaRot);
         }
      }
   private:
      GraphicObject* mpObject;
      double mDeltaRot;
   };

   class HitHandle
   {
   public:
      HitHandle(const GraphicLayer* pLayer, LocationType coord) : 
         mpLayer(dynamic_cast<const GraphicLayerImp*>(pLayer)),
         mpObject(NULL),
         mCoord(coord),
         mHandle(-1)
      {
      }

      void operator()(GraphicObject *pObj)
      {
         if (mpLayer != NULL && mpObject == NULL && pObj != NULL)
         {
            mpLayer->grabHandle(mCoord, mpObject, mHandle);
         }
      }

      operator bool() const
      {
         return (mpObject != NULL);
      }

      GraphicObject *getObject() const
      {
         return mpObject;
      }

      int getHandle() const
      {
         return mHandle;
      }

   private:
      const GraphicLayerImp* mpLayer;
      GraphicObject* mpObject;
      LocationType mCoord;
      int mHandle;
   };
}

MoveObjectImp::MoveObjectImp(const string& id, GraphicObjectType type, GraphicLayer* pLayer,
                             LocationType pixelCoord) :
   GraphicObjectImp(id, type, pLayer, pixelCoord),
   mSelectedHandle(-1),
   mpLastSelectedObject(NULL),
   mpUndoGroup(NULL),
   mLeftPressed(false)
{
}

MoveObjectImp::~MoveObjectImp()
{
   if (mpUndoGroup != NULL)
   {
      delete mpUndoGroup;
   }
}

void MoveObjectImp::draw(double zoomFactor) const
{
}

bool MoveObjectImp::hit(LocationType) const
{
   return false;
}

bool MoveObjectImp::processMousePress(LocationType sceneCoord, 
                                 Qt::MouseButton button,
                                 Qt::MouseButtons buttons,
                                 Qt::KeyboardModifiers modifiers)
{
   GraphicLayer* pLayer = getLayer();
   GraphicLayerImp* pLayerImp = dynamic_cast<GraphicLayerImp*>(pLayer);

   VERIFY(pLayerImp != NULL);

   bool used = false;
   if (button == Qt::LeftButton)
   {
      mLeftPressed = true;
      if (modifiers & Qt::ControlModifier)
      {
         pLayerImp->cloneSelection(pLayer);
      }

      bool deselected = false;
      if (modifiers & Qt::ShiftModifier)
      {
         GraphicObject* pCurrentSelectedObject = pLayerImp->hit(sceneCoord);
         if (pLayer->isObjectSelected(pCurrentSelectedObject))
         {
            pLayer->deselectObject(pCurrentSelectedObject);
            mpLastSelectedObject = NULL;
            mSelectedHandle = -1;
            deselected = true;
         }
      }

      if (! deselected)
      {
         if (pLayerImp->grabHandle(sceneCoord, mpLastSelectedObject, mSelectedHandle))
         {
            // do nothing, grabHandle already set some members for us
         }
         else
         {
            mpLastSelectedObject = pLayerImp->hit(sceneCoord);

            if (!pLayer->isObjectSelected(mpLastSelectedObject) && !(modifiers & Qt::ShiftModifier))
            {
               pLayer->deselectAllObjects();
            }

            pLayer->selectObject(mpLastSelectedObject);
         }
      }

      mLastCoord = sceneCoord;
      mPressCoord = sceneCoord;
      used = true;
   }

   return used;
}


bool MoveObjectImp::processMouseMove(LocationType sceneCoord, 
                                 Qt::MouseButton button,
                                 Qt::MouseButtons buttons,
                                 Qt::KeyboardModifiers modifiers)
{
   bool used = false;

   if (mLeftPressed) // use this rather than buttons & Qt::LeftButton because
      // we can get mousemove event with the button pressed without the user clicking within the layer
   {  
      GraphicLayerImp* pLayer = dynamic_cast<GraphicLayerImp*>(getLayer());
      VERIFY(pLayer != NULL);
      bool bLayerLocked = pLayer->getLayerLocked();

      if (mpLastSelectedObject == NULL)
      {
         ViewImp* pView = dynamic_cast<ViewImp*>(pLayer->getView());
         VERIFY(pView != NULL);

         LocationType llWorld;
         pLayer->translateDataToWorld(sceneCoord.mX, sceneCoord.mY, llWorld.mX, llWorld.mY);
         LocationType urWorld;
         pLayer->translateDataToWorld(mPressCoord.mX, mPressCoord.mY, urWorld.mX, urWorld.mY);
         pView->setSelectionBox(llWorld, urWorld);
      }
      else if (!bLayerLocked)
      {
         if (mpUndoGroup == NULL)
         {
            mpUndoGroup = new UndoGroup(pLayer->getView(), "Move Object");
         }

         if (mSelectedHandle != -1)
         {
            switch (getGraphicObjectType())
            {
            case MOVE_OBJECT:
               applyHandleChange(::MoveHandles(mpLastSelectedObject, sceneCoord, mSelectedHandle,
                  modifiers & Qt::ShiftModifier));
               break;
            case ROTATE_OBJECT:
               applyHandleChange(::RotateHandles(mpLastSelectedObject, sceneCoord, mSelectedHandle));
               break;
            default:
               break;
            };
         }
         else
         {
            list<GraphicObject*> selected;
            getLayer()->getSelectedObjects(selected);
            for_each(selected.begin(), selected.end(), 
               ::MoveObjects(sceneCoord - mLastCoord));
         }
      }
      mLastCoord = sceneCoord;
      used = true;
   }
   return used;
}

bool MoveObjectImp::processMouseRelease(LocationType sceneCoord, 
                                 Qt::MouseButton button,
                                 Qt::MouseButtons buttons,
                                 Qt::KeyboardModifiers modifiers)
{
   bool used = false;
   if (button == Qt::LeftButton)
   {
      if (mpLastSelectedObject == NULL)
      {
         GraphicLayerImp* pLayer = dynamic_cast<GraphicLayerImp*>(getLayer());
         VERIFY(pLayer != NULL);
         bool append = modifiers & Qt::ShiftModifier;
         pLayer->selectObjects(mPressCoord, sceneCoord, append);
         ViewImp* pView = dynamic_cast<ViewImp*>(pLayer->getView());
         VERIFY(pView != NULL);
         pView->setSelectionBox(QRect());
      }
      mSelectedHandle = -1;
      used = true;
      if (mpUndoGroup != NULL)
      {
         delete mpUndoGroup;
         mpUndoGroup = NULL;
      }
      mLeftPressed = false;
   }

   return used;
}

bool MoveObjectImp::processMouseDoubleClick(LocationType sceneCoord, Qt::MouseButton button,
                                            Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers)
{
   bool used = false;
   if (button == Qt::LeftButton)
   {
      GraphicObjectImp* pEditObject = dynamic_cast<GraphicObjectImp*>(mpLastSelectedObject);
      if (pEditObject != NULL)
      {
         used = pEditObject->edit();
      }

      mLeftPressed = false;
   }

   return used;
}

template <typename T>
bool MoveObjectImp::applyHandleChange(const T &functor)
{
   if (mSelectedHandle < 8)
   {
      list<GraphicObject*> selected;
      getLayer()->getSelectedObjects(selected);
      for_each(selected.begin(), selected.end(), 
         functor);
   }
   else if (mpLastSelectedObject != NULL)
   {
      functor(mpLastSelectedObject);
   }

   return true;
}

bool MoveObjectImp::isVisible() const
{
   return false;
}

bool MoveObjectImp::hasCornerHandles() const
{
   return false;
}

bool MoveObjectImp::insertionUndoable() const
{
   return false;
}

const string& MoveObjectImp::getObjectType() const
{
   static string type("MoveObjectImp");
   return type;
}

bool MoveObjectImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "MoveObject"))
   {
      return true;
   }

   return GraphicObjectImp::isKindOf(className);
}
