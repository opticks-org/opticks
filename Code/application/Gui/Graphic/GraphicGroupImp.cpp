/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <math.h>

#include <QtGui/QMessageBox>

#include "AppAssert.h"
#include "AppConfig.h"
#include "AppVerify.h"
#include "AppVersion.h"
#include "CgmObject.h"
#include "CgmObjectImp.h"
#include "DrawUtil.h"
#include "glCommon.h"
#include "GraphicElement.h"
#include "GraphicGroupImp.h"
#include "GraphicLayer.h"
#include "GraphicLayerImp.h"
#include "GraphicLayerUndo.h"
#include "GraphicObjectFactory.h"
#include "StringUtilities.h"
#include "TextObject.h"
#include "TextObjectImp.h"
#include "Undo.h"
#include "ViewImp.h"
#include "XercesIncludes.h"
#include "xmlreader.h"

#include <algorithm>
using namespace std;

XERCES_CPP_NAMESPACE_USE

GraphicGroupImp::GraphicGroupImp(const string& id, GraphicObjectType type, GraphicLayer* pLayer,
                                 LocationType pixelCoord) :
   GraphicObjectImp(id, type, pLayer, pixelCoord),
   mbNeedsLayout(true)
{
}

GraphicGroupImp::~GraphicGroupImp()
{
   setLayer(NULL);
   removeAllObjects(true);
}

GraphicGroupImp& GraphicGroupImp::operator= (const GraphicGroupImp& graphicGroup)
{
   if (this != &graphicGroup)
   {
      bool interactive = true;
      GraphicElement* pElement = getElement();
      if (pElement != NULL)
      {
         interactive = pElement->getInteractive();
         pElement->setInteractive(false);
      }
      removeAllObjects(true);

      list<GraphicObject*>::const_iterator iter = graphicGroup.mObjects.begin();
      while (iter != graphicGroup.mObjects.end())
      {
         GraphicObject* pObject = *iter;
         if (pObject != NULL)
         {
            GraphicObjectType eType = pObject->getGraphicObjectType();
            
            bool bSelected = false;

            GraphicLayer* pOrigLayer = NULL;
            pOrigLayer = graphicGroup.getLayer();
            if (pOrigLayer != NULL)
            {
               bSelected = pOrigLayer->isObjectSelected(pObject);
            }

            GraphicObject* pCloneObject = addObject(eType);
            GraphicObjectImp* pCloneObjectImp = dynamic_cast<GraphicObjectImp*>(pCloneObject);
            if (pCloneObject != NULL && pCloneObject != NULL)
            {
               bool bSuccess = pCloneObjectImp->replicateObject(pObject);
               if ((bSuccess == true) && (bSelected == true))
               {
                  GraphicLayer* pLayer = NULL;
                  pLayer = getLayer();
                  if (pLayer != NULL)
                  {
                     pLayer->selectObject(pCloneObject);
                  }
               }
            }
         }

         ++iter;
      }
      if (pElement != NULL)
      {
         pElement->setInteractive(interactive);
      }
   }

   return *this;
}

void GraphicGroupImp::draw(double zoomFactor) const
{
   if (mbNeedsLayout == true)
   {
      const_cast<GraphicGroupImp*>(this)->updateLayout();
   }

   ViewImp* pParentWidget = NULL;

   GraphicLayer* pLayer = getLayer();
   if (pLayer != NULL)
   {
      pParentWidget = dynamic_cast<ViewImp*>(pLayer->getView());
   }

   int iBadObjects = 0;

   list<GraphicObject*>::const_iterator iter;
   iter = mObjects.begin();
   while (iter != mObjects.end())
   {
      GraphicObject* pObject = *iter;
      ++iter;
      GraphicObjectImp* pObjectImp = dynamic_cast<GraphicObjectImp*>(pObject);
      if (pObject != NULL && pObjectImp != NULL)
      {
         pObjectImp->rotateViewMatrix();

         try
         {
            pObjectImp->draw(zoomFactor);
            pObjectImp->drawLabel();
         }
         // only thrown on debug
         catch (AssertException except)
         {
            emit const_cast<GraphicGroupImp*>(this)->abortedAdd(pObject);
            pLayer->removeObject(pObject, true);
            pObject = NULL;

            QMessageBox::warning(pParentWidget, APP_NAME, QString::fromStdString(except.getText()) +
               "\nThe object has been deleted.");
         }
         catch (...)
         {
            emit const_cast<GraphicGroupImp*>(this)->abortedAdd(pObject);
            iBadObjects++;
            const_cast<GraphicGroupImp*>(this)->removeObject(pObject, true);
         }

         glMatrixMode(GL_MODELVIEW);
         glPopMatrix();
      }
   }

   if (iBadObjects > 0)
   {
      QMessageBox::warning(pParentWidget, APP_NAME, "One or more graphic objects exist "
         "that cannot be drawn properly.  The objects have been deleted.");
   }
}

bool GraphicGroupImp::setProperty(const GraphicProperty* pProperty)
{
   if (pProperty == NULL)
   {
      return false;
   }

   string propertyName = "";
   propertyName = pProperty->getName();

   bool bProperty = false;
   bProperty = hasProperty(propertyName);
   if (bProperty == true)
   {
      bool bSuccess = false;
      bSuccess = GraphicObjectImp::setProperty(pProperty);
      if ((bSuccess == true) && (propertyName == "BoundingBox"))
      {
         mbNeedsLayout = true;
      }

      return bSuccess;
   }

   for (list<GraphicObject*>::iterator it = mObjects.begin(); it != mObjects.end(); ++it)
   {
      GraphicObjectImp* pObject = dynamic_cast<GraphicObjectImp*>(*it);
      if (pObject != NULL)
      {
         if (pObject->hasProperty(propertyName) == true)
         {
            pObject->setProperty(pProperty);
         }
      }
   }

   return true;
}

void GraphicGroupImp::updateBoundingBox()
{
   double dMaxX = -1e30;
   double dMinX = 1e30;
   double dMaxY = -1e30;
   double dMinY = 1e30;

   for (list<GraphicObject*>::iterator iter = mObjects.begin(); iter != mObjects.end(); ++iter)
   {
      GraphicObject* pObject = *iter;
      if (pObject == NULL)
      {
         continue;
      }

      LocationType llCorner = pObject->getLlCorner();
      LocationType urCorner = pObject->getUrCorner();
      LocationType center((llCorner.mX + urCorner.mX) / 2.0, (llCorner.mY + urCorner.mY) / 2.0);

      double angle = pObject->getRotation();
      double cosTheta = cos(PI / 180.0 * angle);
      double sinTheta = sin(PI / 180.0 * angle);

      vector<LocationType> corners;
      corners.push_back(llCorner);
      corners.push_back(LocationType(llCorner.mX, urCorner.mY));
      corners.push_back(urCorner);
      corners.push_back(LocationType(urCorner.mX, llCorner.mY));

      vector<LocationType>::const_iterator pit;
      for (pit = corners.begin(); pit != corners.end(); ++pit)
      {
         LocationType realPoint
         (
            center.mX + (pit->mX - center.mX) * cosTheta - (pit->mY - center.mY) * sinTheta,
            center.mY + (pit->mX - center.mX) * sinTheta + (pit->mY - center.mY) * cosTheta
         );

         if (realPoint.mX > dMaxX)
         {
            dMaxX = realPoint.mX;
         }

         if (realPoint.mX < dMinX)
         {
            dMinX = realPoint.mX;
         }

         if (realPoint.mY > dMaxY)
         {
            dMaxY = realPoint.mY;
         }

         if (realPoint.mY < dMinY)
         {
            dMinY = realPoint.mY;
         }
      }
   }

   LocationType groupLlCorner;
   LocationType groupUrCorner;
   if (!mObjects.empty())
   {
      groupLlCorner.mX = dMinX;
      groupLlCorner.mY = dMinY;
      groupUrCorner.mX = dMaxX;
      groupUrCorner.mY = dMaxY;
   }

   setBoundingBox(groupLlCorner, groupUrCorner);
   updateHandles();

   mbNeedsLayout = false;
   mLlCorner = groupLlCorner;
   mUrCorner = groupUrCorner;
}

class ConnectObject
{
public:
   ConnectObject(GraphicGroupImp* pObj) :
      mpObj(pObj) {}

   void operator()(GraphicObject* pObj)
   {
      VERIFYNR(QObject::connect(dynamic_cast<GraphicObjectImp*>(pObj), SIGNAL(propertyModified(GraphicProperty*)),
         mpObj, SLOT(updateFromProperty(GraphicProperty*))));
      VERIFYNR(QObject::connect(dynamic_cast<GraphicObjectImp*>(pObj), SIGNAL(modified()),
         mpObj, SIGNAL(modified())));
   }

private:
   GraphicGroupImp* mpObj;
};

class DisconnectObject
{
public:
   DisconnectObject(GraphicGroupImp* pObj) :
      mpObj(pObj) {}

   void operator()(GraphicObject* pObj)
   {
      QObject::disconnect(dynamic_cast<GraphicObjectImp*>(pObj), SIGNAL(propertyModified(GraphicProperty*)),
         mpObj, SLOT(updateFromProperty(GraphicProperty*)));
      QObject::disconnect(dynamic_cast<GraphicObjectImp*>(pObj), SIGNAL(modified()),
         mpObj, SIGNAL(modified()));
   }

private:
   GraphicGroupImp* mpObj;
};

void GraphicGroupImp::updateLayout()
{
   if (mbNeedsLayout == false)
   {
      return;
   }

   LocationType llCorner = getLlCorner();
   LocationType urCorner = getUrCorner();

   if (llCorner == LocationType(0, 0) && urCorner == LocationType(0, 0))
   {
      updateBoundingBox();
      return;
   }

   for_each(mObjects.begin(), mObjects.end(), DisconnectObject(this));

   for (list<GraphicObject*>::iterator iter = mObjects.begin(); iter != mObjects.end(); ++iter)
   {
      GraphicObjectImp* pObject = dynamic_cast<GraphicObjectImp*>(*iter);
      if (pObject != NULL)
      {
         double dWidth = mUrCorner.mX - mLlCorner.mX;
         double dHeight = mUrCorner.mY - mLlCorner.mY;

         // Calculate the object's new corner locations
         LocationType oldObjectLlCorner = pObject->getLlCorner();
         LocationType oldObjectUrCorner = pObject->getUrCorner();

         LocationType llPercentage(0.0, 0.0);
         LocationType urPercentage(1.0, 1.0);
         if (dWidth != 0.0)
         {
            llPercentage.mX = (oldObjectLlCorner.mX - mLlCorner.mX) / dWidth;
            urPercentage.mX = (oldObjectUrCorner.mX - mLlCorner.mX) / dWidth;
         }

         if (dHeight != 0.0)
         {
            llPercentage.mY = (oldObjectLlCorner.mY - mLlCorner.mY) / dHeight;
            urPercentage.mY = (oldObjectUrCorner.mY - mLlCorner.mY) / dHeight;
         }

         LocationType newObjectLlCorner;
         newObjectLlCorner.mX = llCorner.mX + (urCorner.mX - llCorner.mX) * llPercentage.mX;
         newObjectLlCorner.mY = llCorner.mY + (urCorner.mY - llCorner.mY) * llPercentage.mY;

         LocationType newObjectUrCorner;
         newObjectUrCorner.mX = llCorner.mX + (urCorner.mX - llCorner.mX) * urPercentage.mX;
         newObjectUrCorner.mY = llCorner.mY + (urCorner.mY - llCorner.mY) * urPercentage.mY;

         // Set the object's bounding box
         pObject->setBoundingBox(newObjectLlCorner, newObjectUrCorner);
         pObject->updateHandles();
      }
   }

   for_each(mObjects.begin(), mObjects.end(), ConnectObject(this));

   mbNeedsLayout = false;
   mLlCorner = llCorner;
   mUrCorner = urCorner;
}

void GraphicGroupImp::updateHandles()
{
   for (list<GraphicObject*>::iterator iter = mObjects.begin(); iter != mObjects.end(); ++iter)
   {
      GraphicObjectImp* pObject = dynamic_cast<GraphicObjectImp*>(*iter);
      if (pObject != NULL)
      {
         pObject->updateHandles();
      }
   }

   GraphicObjectImp::updateHandles();
}

bool GraphicGroupImp::hit(LocationType pixelCoord) const
{
   GraphicObject* pObject = hitObject(pixelCoord);
   if (pObject != NULL)
   {
      return true;
   }

   return false;
}

GraphicObject* GraphicGroupImp::hitObject(const LocationType& pixelCoord) const
{
   list<GraphicObject*>::const_reverse_iterator iter;
   for (iter = mObjects.rbegin(); iter != mObjects.rend(); ++iter)
   {
      GraphicObject* pObject = *iter;
      GraphicObjectImp* pObjectImp = dynamic_cast<GraphicObjectImp*>(pObject);
      VERIFY(pObjectImp != NULL);
      if (pObject != NULL)
      {
         bool bHit = false;

         double dRotation = pObject->getRotation();
         if (dRotation != 0.0)
         {
            LocationType llCorner = pObject->getLlCorner();
            LocationType urCorner = pObject->getUrCorner();

            LocationType center;
            center.mX = (llCorner.mX + urCorner.mX) / 2.0;
            center.mY = (llCorner.mY + urCorner.mY) / 2.0;

            LocationType adjustedCoord = DrawUtil::getRotatedCoordinate(pixelCoord, center, -dRotation);
            bHit = pObjectImp->hit(adjustedCoord);
         }
         else
         {
            bHit = pObjectImp->hit(pixelCoord);
         }

         if (bHit == true)
         {
            return pObject;
         }
      }
   }

   return NULL;
}

void GraphicGroupImp::updateGeo()
{
   for (list<GraphicObject*>::iterator iter = mObjects.begin(); iter != mObjects.end(); ++iter)
   {
      GraphicObjectImp* pObject = dynamic_cast<GraphicObjectImp*>(*iter);
      LOG_IF(pObject == NULL, continue);
      pObject->updateGeo();
   }
}

void GraphicGroupImp::enableGeo()
{
   for (list<GraphicObject*>::iterator iter = mObjects.begin(); iter != mObjects.end(); ++iter)
   {
      GraphicObjectImp* pObject = dynamic_cast<GraphicObjectImp*>(*iter);
      LOG_IF(pObject == NULL, continue);
      pObject->enableGeo();
   }
}

GraphicObject* GraphicGroupImp::createObject(GraphicObjectType eType, LocationType pixelCoord)
{
   GraphicLayer* pLayer = getLayer();
   GraphicObject* pObject = GraphicObjectFactory::createObject(eType, pLayer, pixelCoord);

   return pObject;
}

GraphicObject* GraphicGroupImp::addObject(GraphicObjectType eType, LocationType point)
{
   View* pView = NULL;

   GraphicLayer* pLayer = getLayer();
   if (pLayer != NULL)
   {
      pView = pLayer->getView();
   }

   GraphicObject* pObject = NULL;
   {
      UndoLock lock(pView);
      pObject = createObject(eType, point);
   }

   if (pObject != NULL)
   {
      if (pView != NULL)
      {
         pView->addUndoAction(new AddGraphicObject(dynamic_cast<GraphicGroup*>(this), pObject));
      }

      insertObject(pObject);
   }

   return pObject;
}

void GraphicGroupImp::insertObject(GraphicObject* pObject)
{
   if (pObject != NULL)
   {
      if (pObject->isVisible())
      {
         mObjects.push_back(pObject);
      }
      ConnectObject(this)(pObject);
      notify(SIGNAL_NAME(GraphicGroup, ObjectAdded), boost::any(pObject));

      updateBoundingBox();
      emit modified();
   }
}

void GraphicGroupImp::insertObjects(list<GraphicObject*>& objects)
{
   for_each(objects.begin(), objects.end(), ConnectObject(this));
   mObjects.splice(mObjects.end(), objects);

   for (list<GraphicObject*>::iterator iter = objects.begin(); iter != objects.end(); ++iter)
   {
      notify(SIGNAL_NAME(GraphicGroup, ObjectAdded), boost::any(*iter));
   }

   updateBoundingBox();
   emit modified();
}

void GraphicGroupImp::insertObjects(const list<GraphicObject*>& objects)
{
   list<GraphicObject*> localCopy = objects;
   for_each(localCopy.begin(), localCopy.end(), ConnectObject(this));
   mObjects.splice(mObjects.end(), localCopy);

   for (list<GraphicObject*>::const_iterator iter = objects.begin(); iter != objects.end(); ++iter)
   {
      notify(SIGNAL_NAME(GraphicGroup, ObjectAdded), boost::any(*iter));
   }

   updateBoundingBox();
   emit modified();
}

bool GraphicGroupImp::hasObject(GraphicObject* pObject) const
{
   if (pObject == NULL)
   {
      return false;
   }

   list<GraphicObject*>::const_iterator iter = mObjects.begin();
   while (iter != mObjects.end())
   {
      GraphicObject* pCurrentObject = *iter;
      if (pCurrentObject == pObject)
      {
         return true;
      }

      ++iter;
   }

   return false;
}

const list<GraphicObject*>& GraphicGroupImp::getObjects() const
{
   return mObjects;
}

bool GraphicGroupImp::moveObjectToBack(GraphicObject* pObject)
{
   if (pObject == NULL)
   {
      return false;
   }

   list<GraphicObject*>::iterator iter = find(mObjects.begin(), mObjects.end(), pObject);
   if (iter != mObjects.end())
   {
      mObjects.erase(iter);
      mObjects.push_front(pObject);
      return true;
   }

   return false;
}

bool GraphicGroupImp::moveObjectToFront(GraphicObject* pObject)
{
   if (pObject == NULL)
   {
      return false;
   }

   list<GraphicObject*>::iterator iter = find(mObjects.begin(), mObjects.end(), pObject);
   if (iter != mObjects.end())
   {
      mObjects.erase(iter);
      mObjects.push_back(pObject);
      return true;
   }

   return false;
}

int GraphicGroupImp::getObjectStackingIndex(GraphicObject* pObject) const
{
   list<GraphicObject*>::const_iterator iter;
   int index;
   for (index = 0, iter = mObjects.begin(); iter != mObjects.end() && *iter != pObject; ++iter)
   {
      index++;
   }
   if (index == mObjects.size())
   {
      index = -1;
   }
   return index;
}

void GraphicGroupImp::setObjectStackingIndex(GraphicObject* pObject, int index)
{
   list<GraphicObject*>::iterator iter = find(mObjects.begin(), mObjects.end(), pObject);
   if (iter != mObjects.end())
   {
      mObjects.erase(iter);

      for (iter = mObjects.begin(); iter != mObjects.end() && index > 0; ++iter)
      {
         index--;
      }
      mObjects.insert(iter, pObject);
   }
}

bool GraphicGroupImp::removeObject(GraphicObject* pObject, bool bDelete)
{
   if (pObject == NULL)
   {
      return false;
   }

   GraphicLayerImp* pLayer = dynamic_cast<GraphicLayerImp*>(getLayer());
   if (pLayer != NULL)
   {
      pLayer->completeInsertion();
      pLayer->deselectObject(pObject);
   }

   list<GraphicObject*>::iterator it = find(mObjects.begin(), mObjects.end(), pObject);
   if (it != mObjects.end())
   {
      mObjects.erase(it);
      DisconnectObject(this)(pObject);
      notify(SIGNAL_NAME(GraphicGroup, ObjectRemoved), boost::any(pObject));

      if (bDelete == true)
      {
         if (pLayer != NULL)
         {
            View* pView = pLayer->getView();
            if (pView != NULL)
            {
               pView->addUndoAction(new RemoveGraphicObject(dynamic_cast<GraphicGroup*>(this), pObject));
            }
         }

         GraphicObjectImp* pObjectImp = dynamic_cast<GraphicObjectImp*> (pObject);
         delete pObjectImp;
      }

      View* pView = NULL;
      if (pLayer != NULL)
      {
         pView = pLayer->getView();
      }

      UndoLock undoLock(pView);
      updateBoundingBox();
      emit modified();
      return true;
   }

   return false;
}

void GraphicGroupImp::removeAllObjects(bool bDelete)
{
   View* pView = NULL;
   UndoGroup* pUndoGroup = NULL;
   GraphicLayerImp* pLayer = dynamic_cast<GraphicLayerImp*>(getLayer());
   if (pLayer != NULL)
   {
      pLayer->completeInsertion();
      pLayer->deselectAllObjects();

      // Use an undo group for improved performance on large graphic groups.
      if (bDelete == true)
      {
         pView = pLayer->getView();
         if (pView != NULL)
         {
            pUndoGroup = new UndoGroup(pView, "Remove All Objects");
         }
      }
   }

   for_each(mObjects.begin(), mObjects.end(), DisconnectObject(this));

   // Remove each object while iterating the loop to avoid stale pointers within mObjects.
   // The stale pointers can cause crashes if code attached to GraphicGroup, ObjectRemoved calls methods on this class.
   while (mObjects.empty() == false)
   {
      GraphicObject* pObject = mObjects.front();
      mObjects.erase(mObjects.begin());
      notify(SIGNAL_NAME(GraphicGroup, ObjectRemoved), boost::any(pObject));
      if (bDelete == true)
      {
         if (pView != NULL)
         {
            pView->addUndoAction(new RemoveGraphicObject(dynamic_cast<GraphicGroup*>(this), pObject));
         }

         // Need to cast to a GraphicObjectImp* to perform the deletion.
         delete dynamic_cast<GraphicObjectImp*>(pObject);
      }
   }

   delete pUndoGroup;

   updateBoundingBox();
   emit modified();
}

bool GraphicGroupImp::replicateObject(const GraphicObject* pObject)
{
   if (pObject == NULL)
   {
      return false;
   }

   GraphicObjectType eCurrentType = getGraphicObjectType();
   GraphicObjectType eNewType = pObject->getGraphicObjectType();

   if (eCurrentType != eNewType)
   {
      return false;
   }

   const GraphicGroupImp* pGroup = dynamic_cast<const GraphicGroupImp*>(pObject);
   if (this != pGroup)
   {
      bool bSuccess = false;
      bSuccess = GraphicObjectImp::replicateObject(pObject);
      if (bSuccess == false)
      {
         return false;
      }

      removeAllObjects(true);

      list<GraphicObject*> objects = pGroup->getObjects();

      list<GraphicObject*> newObjects;
      list<GraphicObject*>::iterator iter;
      for (iter = objects.begin(); iter != objects.end(); ++iter)
      {
         GraphicObject* pExistingObject = *iter;
         if (pExistingObject != NULL)
         {
            GraphicObjectType eObjectType = pExistingObject->getGraphicObjectType();

            GraphicObject* pNewObject = NULL;
            pNewObject = createObject(eObjectType);
            if (pNewObject != NULL)
            {
               bSuccess = dynamic_cast<GraphicObjectImp*>(pNewObject)->replicateObject(pExistingObject);
               if (bSuccess == false)
               {
                  // delete all newObjects
                  list<GraphicObject*>::iterator ppObject;
                  for (ppObject = newObjects.begin(); ppObject != newObjects.end(); ++pObject)
                  {
                     delete dynamic_cast<GraphicObjectImp*>(*ppObject);
                  }
                  return false;
               }
               newObjects.push_back(pNewObject);
            }
         }
      }

      insertObjects(newObjects);
   }

   return true;
}

CgmObject* GraphicGroupImp::convertToCgm()
{
   GraphicLayer* pLayer = getLayer();

   GraphicResource<CgmObjectImp> pCgm(CGM_OBJECT, pLayer);
   if (pCgm.get() == NULL)
   {
      return NULL;
   }

   double dAngle = getRotation();

   for (list<GraphicObject*>::iterator iter = mObjects.begin(); iter != mObjects.end(); ++iter)
   {
      GraphicObject* pObject = *iter;
      if (pObject != NULL)
      {
         GraphicObjectType eType = pObject->getGraphicObjectType();

         GraphicObject* pNewObject = pCgm->addObject(eType);
         if (pNewObject != NULL)
         {
            bool bSuccess = false;
            bSuccess = dynamic_cast<GraphicObjectImp*>(pNewObject)->replicateObject(pObject);
            if (bSuccess == false)
            {
               pCgm->removeObject(pNewObject, true);
               return NULL;
            }
         }
      }
   }

   pCgm->setRotation(dAngle);
   pCgm->updateBoundingBox();

   return dynamic_cast<CgmObject*>(pCgm.release());
}

bool GraphicGroupImp::setFillState(bool bFill)
{
   return propagateProperty(&GraphicObjectImp::setFillState, bFill);
}

bool GraphicGroupImp::setFont(const QFont& textFont)
{
   return propagateProperty(&GraphicObjectImp::setFont, textFont);
}

bool GraphicGroupImp::toXml(XMLWriter* pXml) const
{
   if (pXml == NULL)
   {
      return false;
   }

   // Serialize the group properties
   if (GraphicObjectImp::toXml(pXml) == false)
   {
      return false;
   }

   // Check if there are any objects to save
   if (mObjects.size() > 0)
   {
      // Add the objects tag
      pXml->pushAddPoint(pXml->addElement("objects"));

      // Serialize the group objects
      bool bSilentError = false;

      list<GraphicObject*>::const_iterator iter = mObjects.begin();
      while (iter != mObjects.end())
      {
         GraphicObjectImp* pObject = NULL;
         pObject = dynamic_cast<GraphicObjectImp*> (*iter);
         if (pObject != NULL)
         {
            // Add the graphic tag
            pXml->pushAddPoint(pXml->addElement("Graphic"));

            // Serialize the object
            bool bSuccess = pObject->toXml(pXml);
            DOMNode* pNode = pXml->popAddPoint();

            if (bSuccess == false)
            {
               if (pNode != NULL)
               {
                  pXml->removeChild(pNode);
               }

               if (bSilentError == false)
               {
                  ViewImp* pParentWidget = NULL;

                  GraphicLayer* pLayer = getLayer();
                  if (pLayer != NULL)
                  {
                     pParentWidget = dynamic_cast<ViewImp*>(pLayer->getView());
                  }

                  int iReturn = QMessageBox::question(pParentWidget, APP_NAME, "An graphic object cannot be "
                     "saved.  Do you want to continue saving?", QMessageBox::Yes, QMessageBox::YesAll,
                     QMessageBox::No);
                  if (iReturn == QMessageBox::No)
                  {
                     return false;
                  }
                  else if (iReturn == QMessageBox::YesAll)
                  {
                     bSilentError = true;
                  }
               }
            }
         }

         ++iter;
      }

      pXml->popAddPoint();
   }

   return true;
}

bool GraphicGroupImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL || version < 3)
   {
      return false;
   }

   // Deserialize the group properties
   if (GraphicObjectImp::fromXml(pDocument, version) == false)
   {
      return false;
   }

   // Deserialize the group objects
   bool bSilentError = false;

   XmlReader::DomParseException* pExc(NULL);
   for (DOMNode* pChld = pDocument->getFirstChild(); pChld != NULL; pChld = pChld->getNextSibling())
   {
      if (XMLString::equals(pChld->getNodeName(), X("objects")))
      {
         DOMNode* pObjectNode = NULL;
         for (pObjectNode = pChld->getFirstChild(); pObjectNode != NULL; pObjectNode = pObjectNode->getNextSibling())
         {
            if (XMLString::equals(pObjectNode->getNodeName(), X("Graphic")))
            {
               DOMElement* elmnt(static_cast<DOMElement*>(pObjectNode));
               string type(A(elmnt->getAttribute(X("type"))));

               View* pView = NULL;

               GraphicLayer* pLayer = getLayer();
               if (pLayer != NULL)
               {
                  pView = pLayer->getView();
               }

               GraphicObject* pObject = NULL;
               if (type.empty() == false)
               {
                  UndoLock lock(pView);

                  GraphicObjectType eObject = StringUtilities::fromXmlString<GraphicObjectType>(type);
                  pObject = addObject(eObject);
               }

               GraphicObjectImp* pObjectImp = dynamic_cast<GraphicObjectImp*>(pObject);
               if (pObject != NULL && pObjectImp != NULL)
               {
                  try
                  {
                     if (pObjectImp->fromXml(elmnt, version) == false)
                     {
                        removeObject(pObject, true);
                        pObject = NULL;
                     }
                     else if (pView != NULL)
                     {
                        pView->addUndoAction(new AddGraphicObject(dynamic_cast<GraphicGroup*>(this), pObject));
                     }
                  }
                  catch (XmlReader::DomParseException& exc)
                  {
                     pObject = NULL;
                     pExc = &exc;
                  }
                  catch (...)
                  {
                     pObject = NULL;
                  }
               }

               if ((pObject == NULL) && (bSilentError == false))
               {
                  QString strType;
                  if (type.empty() == false)
                  {
                     strType = "(type = " + QString::fromStdString(type) + ") ";
                  }

                  QString strError = "A graphic object " + strType + "cannot be loaded.";
                  if (pExc != NULL)
                  {
                     string exceptionText = pExc->str();
                     if (exceptionText.empty() == false)
                     {
                        strError.truncate(strError.length() - 1);
                        strError += ": " + QString::fromStdString(exceptionText);
                     }
                  }

                  strError += "\nDo you want to continue loading?";

                  ViewImp* pParentWidget = NULL;
                  if (pLayer != NULL)
                  {
                     pParentWidget = dynamic_cast<ViewImp*>(pLayer->getView());
                  }

                  int iReturn = QMessageBox::question(pParentWidget, "Graphic", strError,
                     QMessageBox::Yes, QMessageBox::YesAll, QMessageBox::No);
                  if (iReturn == QMessageBox::No)
                  {
                     removeAllObjects(true);
                     return false;
                  }
                  else if (iReturn == QMessageBox::YesAll)
                  {
                     bSilentError = true;
                  }
               }
            }
         }
      }
   }

   return true;
}

const string& GraphicGroupImp::getObjectType() const
{
   static string type("GraphicGroupImp");
   return type;
}

bool GraphicGroupImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "GraphicGroup"))
   {
      return true;
   }

   return GraphicObjectImp::isKindOf(className);
}

template<typename T, typename U>
bool GraphicGroupImp::propagateProperty(T method, U value)
{
   bool bSuccess = false;
   list<GraphicObject*>::const_iterator iter = mObjects.begin();
   while (iter != mObjects.end())
   {
      GraphicObjectImp* pCurrentObject = NULL;
      pCurrentObject = dynamic_cast<GraphicObjectImp*>(*iter);
      if (pCurrentObject != NULL)
      {
         bSuccess |= (pCurrentObject->*method)(value);
         if (pCurrentObject->getGraphicObjectType() == TEXT_OBJECT)
         {
            static_cast<TextObjectImp*>(pCurrentObject)->updateTexture();
         }
      }

      ++iter;
   }
   if (bSuccess)
   {
      updateBoundingBox();
   }
   return bSuccess;
}

void GraphicGroupImp::setLayer(GraphicLayer* pLayer)
{
   GraphicObjectImp::setLayer(pLayer);
   for (list<GraphicObject*>::iterator iter = mObjects.begin(); iter != mObjects.end(); ++iter)
   {
      GraphicObjectImp* pImp = dynamic_cast<GraphicObjectImp*>(*iter);
      if (pImp != NULL)
      {
         pImp->setLayer(pLayer);
      }
   }
}

void GraphicGroupImp::updateFromProperty(GraphicProperty* pProperty)
{
   if (dynamic_cast<BoundingBoxProperty*>(pProperty) != NULL)
   {
      bool update = true;
      GraphicElement* pElement = getElement();
      if (pElement != NULL)
      {
         update = pElement->getInteractive();
      }

      if (update)
      {
         updateBoundingBox();
      }
   }
}
