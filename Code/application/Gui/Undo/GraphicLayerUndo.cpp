/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AnnotationLayer.h"
#include "AppVerify.h"
#include "ClassificationLayer.h"
#include "GraphicElement.h"
#include "GraphicGroup.h"
#include "GraphicGroupImp.h"
#include "GraphicLayer.h"
#include "GraphicLayerImp.h"
#include "GraphicLayerUndo.h"
#include "GraphicObjectFactory.h"
#include "GraphicObjectImp.h"
#include "MultipointObjectImp.h"
#include "ProductView.h"
#include "SessionManager.h"
#include "Undo.h"

#include <algorithm>
#include <string>
using namespace std;

/////////////////////////
// GraphicLayerMemento //
/////////////////////////

GraphicLayerMemento::GraphicLayerMemento(GraphicLayer* pLayer)
{
}

void GraphicLayerMemento::toLayer(Layer* pLayer) const
{
   GraphicLayer* pGraphicLayer = dynamic_cast<GraphicLayer*>(pLayer);
   if (pGraphicLayer != NULL)
   {
      GraphicElement* pElement = dynamic_cast<GraphicElement*>(pGraphicLayer->getDataElement());
      if (pElement != NULL)
      {
         GraphicGroupImp* pGroup = dynamic_cast<GraphicGroupImp*>(pElement->getGroup());
         if (pGroup != NULL)
         {
            pGroup->setLayer(pGraphicLayer);
         }
      }
   }
}

////////////////////////////
// AnnotationLayerMemento //
////////////////////////////

AnnotationLayerMemento::AnnotationLayerMemento(AnnotationLayer* pLayer) :
   GraphicLayerMemento(pLayer)
{
}

////////////////////////////////
// CreateDestroyGraphicObject //
////////////////////////////////

CreateDestroyGraphicObject::CreateDestroyGraphicObject(GraphicGroup* pGroup, GraphicObject* pObject) :
   UndoAction(pGroup),
   mLayerGroup(false),
   mpClone(NULL),
   mIndex(-1)
{
   // Get the layer ID
   GraphicLayerImp* pLayer = NULL;

   GraphicGroupImp* pGroupImp = dynamic_cast<GraphicGroupImp*>(pGroup);
   if (pGroupImp != NULL)
   {
      pLayer = dynamic_cast<GraphicLayerImp*>(pGroupImp->getLayer());
      if (pLayer != NULL)
      {
         View* pView = pLayer->getView();
         if (pView != NULL)
         {
            mViewId = pView->getId();
         }

         mLayerId = pLayer->getId();

         if (pLayer->getGroup() == pGroup)
         {
            mLayerGroup = true;
         }
      }
   }

   if (pObject != NULL)
   {
      // Get the object ID
      mObjectId = pObject->getId();

      // Copy the object
      mpClone = dynamic_cast<GraphicObjectImp*>
         (GraphicObjectFactory::createObject(pObject->getGraphicObjectType(), NULL));
      if (mpClone != NULL)
      {
         mpClone->replicateObject(pObject);
      }

      // Get the stacking index
      if (pLayer != NULL)
      {
         mIndex = pLayer->getObjectStackingIndex(pObject);
      }
   }
}

CreateDestroyGraphicObject::~CreateDestroyGraphicObject()
{
   if (mpClone != NULL)
   {
      delete mpClone;
   }
}

SessionItem* CreateDestroyGraphicObject::getSessionItem() const
{
   if (mLayerId.empty() == false)
   {
      const string& itemId = getSessionItemId();
      if (itemId.empty() == false)
      {
         return GraphicUndoUtilities::getObject(mViewId, mLayerId, itemId);
      }
   }

   return NULL;
}

void CreateDestroyGraphicObject::updateSessionItem(const string& oldId, const string& newId)
{
   if (oldId.empty() == false)
   {
      if (oldId == mViewId)
      {
         mViewId = newId;
      }

      if (oldId == mLayerId)
      {
         mLayerId = newId;

         // If the member object is the layer's group, update the group
         if (mLayerGroup == true)
         {
            GraphicGroup* pGroup = NULL;
            if (mLayerId.empty() == false)
            {
               Service<SessionManager> pManager;

               GraphicLayerImp* pLayer = dynamic_cast<GraphicLayerImp*>(pManager->getSessionItem(mLayerId));
               if (pLayer != NULL)
               {
                  pGroup = pLayer->getGroup();
               }
            }

            setSessionItem(pGroup);
            return;
         }
      }

      if (oldId == mObjectId)
      {
         mObjectId = newId;
      }
   }

   UndoAction::updateSessionItem(oldId, newId);
}

void CreateDestroyGraphicObject::createObject()
{
   if (mpClone == NULL)
   {
      return;
   }

   GraphicGroup* pGroup = dynamic_cast<GraphicGroup*>(getSessionItem());
   if (pGroup != NULL)
   {
      string oldObjectId = mObjectId;
      mObjectId.clear();

      GraphicObject* pObject = pGroup->addObject(mpClone->getGraphicObjectType());
      if (pObject != NULL)
      {
         mObjectId = pObject->getId();

         GraphicObjectImp* pObjectImp = dynamic_cast<GraphicObjectImp*>(pObject);
         if (pObjectImp != NULL)
         {
            pObjectImp->replicateObject(dynamic_cast<GraphicObject*>(mpClone));
         }

         if (mIndex != -1)
         {
            GraphicGroupImp* pGroupImp = dynamic_cast<GraphicGroupImp*>(pGroup);
            if (pGroupImp != NULL)
            {
               GraphicLayerImp* pLayer = dynamic_cast<GraphicLayerImp*>(pGroupImp->getLayer());
               if (pLayer != NULL)
               {
                  pLayer->setObjectStackingIndex(pObject, mIndex);
               }
            }
         }
      }

      emit sessionItemChanged(oldObjectId, mObjectId);
   }
}

void CreateDestroyGraphicObject::destroyObject()
{
   GraphicGroup* pGroup = static_cast<GraphicGroup*>(getSessionItem());
   if (pGroup != NULL)
   {
      GraphicObject* pObject = GraphicUndoUtilities::getObject(mViewId, mLayerId, mObjectId);
      if (pObject != NULL)
      {
         pGroup->removeObject(pObject, true);
      }
   }
}

//////////////////////
// AddGraphicObject //
//////////////////////

AddGraphicObject::AddGraphicObject(GraphicGroup* pGroup, GraphicObject* pObject) :
   CreateDestroyGraphicObject(pGroup, pObject)
{
   setText("Add Graphic Object");
}

void AddGraphicObject::executeUndo()
{
   destroyObject();
}

void AddGraphicObject::executeRedo()
{
   createObject();
}

/////////////////////////
// RemoveGraphicObject //
/////////////////////////

RemoveGraphicObject::RemoveGraphicObject(GraphicGroup* pGroup, GraphicObject* pObject) :
   CreateDestroyGraphicObject(pGroup, pObject)
{
   setText("Delete Graphic Object");
}

void RemoveGraphicObject::executeUndo()
{
   createObject();
}

void RemoveGraphicObject::executeRedo()
{
   destroyObject();
}

//////////////////////////////
// SetGraphicObjectProperty //
//////////////////////////////

SetGraphicObjectProperty::SetGraphicObjectProperty(GraphicObject* pObject, const GraphicProperty* pOldProperty,
                                                   const GraphicProperty* pNewProperty) :
   UndoAction(pObject),
   mpOldProperty(NULL),
   mpNewProperty(NULL)
{
   GraphicObjectImp* pObjectImp = dynamic_cast<GraphicObjectImp*>(pObject);
   if (pObjectImp != NULL)
   {
      GraphicLayer* pLayer = pObjectImp->getLayer();
      if (pLayer != NULL)
      {
         View* pView = pLayer->getView();
         if (pView != NULL)
         {
            mViewId = pView->getId();
         }

         mLayerId = pLayer->getId();
      }
   }

   QString actionText = "Set Object Property";
   if ((pOldProperty != NULL) && (pNewProperty != NULL))
   {
      string oldName = pOldProperty->getName();
      string newName = pNewProperty->getName();
      if (oldName == newName)
      {
         actionText = "Set Object " + QString::fromStdString(oldName);
         mpOldProperty = pOldProperty->copy();
         mpNewProperty = pNewProperty->copy();
      }
   }

   setText(actionText);
}

SetGraphicObjectProperty::~SetGraphicObjectProperty()
{
   if (mpOldProperty != NULL)
   {
      delete mpOldProperty;
   }

   if (mpNewProperty != NULL)
   {
      delete mpNewProperty;
   }
}

SessionItem* SetGraphicObjectProperty::getSessionItem() const
{
   if (mLayerId.empty() == false)
   {
      const string& objectId = getSessionItemId();
      if (objectId.empty() == false)
      {
         return GraphicUndoUtilities::getObject(mViewId, mLayerId, objectId);
      }
   }

   return NULL;
}

void SetGraphicObjectProperty::updateSessionItem(const string& oldId, const string& newId)
{
   if (oldId.empty() == false)
   {
      if (oldId == mViewId)
      {
         mViewId = newId;
      }

      if (oldId == mLayerId)
      {
         mLayerId = newId;
      }
   }

   UndoAction::updateSessionItem(oldId, newId);
}

void SetGraphicObjectProperty::executeUndo()
{
   GraphicObjectImp* pObject = dynamic_cast<GraphicObjectImp*>(getSessionItem());
   if ((pObject != NULL) && (mpOldProperty != NULL))
   {
      pObject->setProperty(mpOldProperty);
   }
}

void SetGraphicObjectProperty::executeRedo()
{
   GraphicObjectImp* pObject = dynamic_cast<GraphicObjectImp*>(getSessionItem());
   if ((pObject != NULL) && (mpNewProperty != NULL))
   {
      pObject->setProperty(mpNewProperty);
   }
}

/////////////////////////////
// SetGraphicStackingOrder //
/////////////////////////////

SetGraphicStackingOrder::SetGraphicStackingOrder(GraphicLayer* pLayer, GraphicObject* pObject, int oldIndex,
                                                 int newIndex) :
   UndoAction(pLayer),
   mOldIndex(oldIndex),
   mNewIndex(newIndex)
{
   if (pObject != NULL)
   {
      mObjectId = pObject->getId();
   }

   setText("Set Object Stacking Order");
}

void SetGraphicStackingOrder::updateSessionItem(const string& oldId, const string& newId)
{
   if ((oldId.empty() == false) && (oldId == mObjectId))
   {
      mObjectId = newId;
   }

   UndoAction::updateSessionItem(oldId, newId);
}

void SetGraphicStackingOrder::executeUndo()
{
   GraphicLayerImp* pLayer = dynamic_cast<GraphicLayerImp*>(getSessionItem());
   if (pLayer != NULL)
   {
      string viewId;

      View* pView = pLayer->getView();
      if (pView != NULL)
      {
         viewId = pView->getId();
      }

      GraphicObject* pObject = GraphicUndoUtilities::getObject(viewId, pLayer->getId(), mObjectId);
      if (pObject != NULL)
      {
         pLayer->setObjectStackingIndex(pObject, mOldIndex);
      }
   }
}

void SetGraphicStackingOrder::executeRedo()
{
   GraphicLayerImp* pLayer = dynamic_cast<GraphicLayerImp*>(getSessionItem());
   if (pLayer != NULL)
   {
      string viewId;

      View* pView = pLayer->getView();
      if (pView != NULL)
      {
         viewId = pView->getId();
      }

      GraphicObject* pObject = GraphicUndoUtilities::getObject(viewId, pLayer->getId(), mObjectId);
      if (pObject != NULL)
      {
         pLayer->setObjectStackingIndex(pObject, mNewIndex);
      }
   }
}

////////////////////////////////
// GroupUngroupGraphicObjects //
////////////////////////////////

GroupUngroupGraphicObjects::GroupUngroupGraphicObjects(GraphicLayer* pLayer, GraphicGroup* pGroup) :
   UndoAction(pLayer)
{
   if (pGroup != NULL)
   {
      const list<GraphicObject*>& objects = pGroup->getObjects();
      for (list<GraphicObject*>::const_iterator iter = objects.begin(); iter != objects.end(); ++iter)
      {
         GraphicObject* pObject = *iter;
         if (pObject != NULL)
         {
            string objectId = pObject->getId();
            if (objectId.empty() == false)
            {
               mObjectIds.push_back(objectId);
            }
         }
      }

      mGroupId = pGroup->getId();
   }
}

void GroupUngroupGraphicObjects::updateSessionItem(const string& oldId, const string& newId)
{
   if (oldId.empty() == false)
   {
      for (list<string>::iterator iter = mObjectIds.begin(); iter != mObjectIds.end(); ++iter)
      {
         if (oldId == *iter)
         {
            *iter = newId;
            break;
         }
      }

      if (oldId == mGroupId)
      {
         mGroupId = newId;
      }
   }

   UndoAction::updateSessionItem(oldId, newId);
}

void GroupUngroupGraphicObjects::group()
{
   GraphicLayer* pLayer = dynamic_cast<GraphicLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      list<GraphicObject*> selectedObjects;
      pLayer->getSelectedObjects(selectedObjects);
      pLayer->deselectAllObjects();

      for (list<string>::const_iterator iter = mObjectIds.begin(); iter != mObjectIds.end(); ++iter)
      {
         string objectId = *iter;
         if (objectId.empty() == false)
         {
            string viewId;

            View* pView = pLayer->getView();
            if (pView != NULL)
            {
               viewId = pView->getId();
            }

            GraphicObject* pObject = GraphicUndoUtilities::getObject(viewId, pLayer->getId(), objectId);
            if (pObject != NULL)
            {
               pLayer->selectObject(pObject);
            }
         }
      }

      pLayer->groupSelection();

      list<GraphicObject*> newSelectedObjects;
      pLayer->getSelectedObjects(newSelectedObjects);
      VERIFYNRV(newSelectedObjects.size() == 1);

      GraphicGroup* pGroup = dynamic_cast<GraphicGroup*>(newSelectedObjects.front());
      VERIFYNRV(pGroup != NULL);

      for (list<GraphicObject*>::iterator iter = selectedObjects.begin(); iter != selectedObjects.end(); ++iter)
      {
         GraphicObject* pObject = *iter;
         if ((pObject != NULL) && (pGroup->hasObject(pObject) == false))
         {
            pLayer->selectObject(pObject);
         }
      }

      string oldGroupId = mGroupId;
      mGroupId = pGroup->getId();
      emit sessionItemChanged(oldGroupId, mGroupId);
   }
}

void GroupUngroupGraphicObjects::ungroup()
{
   GraphicLayer* pLayer = dynamic_cast<GraphicLayer*>(getSessionItem());
   if ((pLayer != NULL) && (mGroupId.empty() == false))
   {
      string viewId;

      View* pView = pLayer->getView();
      if (pView != NULL)
      {
         viewId = pView->getId();
      }

      GraphicGroup* pGroup = dynamic_cast<GraphicGroup*>(GraphicUndoUtilities::getObject(viewId,
         pLayer->getId(), mGroupId));
      if (pGroup != NULL)
      {
         list<GraphicObject*> selectedObjects;
         pLayer->getSelectedObjects(selectedObjects);

         pLayer->deselectAllObjects();
         pLayer->selectObject(pGroup);
         pLayer->ungroupSelection();

         list<GraphicObject*> objects;
         pLayer->getSelectedObjects(objects);

         mObjectIds.clear();
         for (list<GraphicObject*>::iterator iter = objects.begin(); iter != objects.end(); ++iter)
         {
            GraphicObject* pObject = *iter;
            if (pObject != NULL)
            {
               string objectId = pObject->getId();
               if (objectId.empty() == false)
               {
                  mObjectIds.push_back(objectId);
               }
            }
         }

         for (list<GraphicObject*>::iterator iter = selectedObjects.begin(); iter != selectedObjects.end(); ++iter)
         {
            GraphicObject* pObject = *iter;
            if ((pObject != NULL) && (pObject != pGroup))
            {
               pLayer->selectObject(pObject);
            }
         }
      }
   }
}

/////////////////////////
// GroupGraphicObjects //
/////////////////////////

GroupGraphicObjects::GroupGraphicObjects(GraphicLayer* pLayer, GraphicGroup* pGroup) :
   GroupUngroupGraphicObjects(pLayer, pGroup)
{
   setText("Group Objects");
}

void GroupGraphicObjects::executeUndo()
{
   ungroup();
}

void GroupGraphicObjects::executeRedo()
{
   group();
}

///////////////////////////
// UngroupGraphicObjects //
///////////////////////////

UngroupGraphicObjects::UngroupGraphicObjects(GraphicLayer* pLayer, GraphicGroup* pGroup) :
   GroupUngroupGraphicObjects(pLayer, pGroup)
{
   setText("Ungroup Objects");
}

void UngroupGraphicObjects::executeUndo()
{
   group();
}

void UngroupGraphicObjects::executeRedo()
{
   ungroup();
}

/////////////////
// AddVertices //
/////////////////

AddVertices::AddVertices(MultipointObjectImp* pObject, const vector<LocationType>& oldVertices,
                         const vector<LocationType>& oldGeoVertices, const vector<LocationType>& newVertices,
                         const vector<LocationType>& newGeoVertices) :
   UndoAction(dynamic_cast<SessionItem*>(pObject)),
   mOldVertices(oldVertices),
   mOldGeoVertices(oldGeoVertices),
   mNewVertices(newVertices),
   mNewGeoVertices(newGeoVertices)
{
   if (pObject != NULL)
   {
      GraphicLayer* pLayer = pObject->getLayer();
      if (pLayer != NULL)
      {
         View* pView = pLayer->getView();
         if (pView != NULL)
         {
            mViewId = pView->getId();
         }

         mLayerId = pLayer->getId();
      }
   }

   setText("Add Vertices");
}

SessionItem* AddVertices::getSessionItem() const
{
   if (mLayerId.empty() == false)
   {
      const string& objectId = getSessionItemId();
      if (objectId.empty() == false)
      {
         return GraphicUndoUtilities::getObject(mViewId, mLayerId, objectId);
      }
   }

   return NULL;
}

void AddVertices::updateSessionItem(const string& oldId, const string& newId)
{
   if (oldId.empty() == false)
   {
      if (oldId == mViewId)
      {
         mViewId = newId;
      }

      if (oldId == mLayerId)
      {
         mLayerId = newId;
      }
   }

   UndoAction::updateSessionItem(oldId, newId);
}

void AddVertices::executeUndo()
{
   MultipointObjectImp* pObject = dynamic_cast<MultipointObjectImp*>(getSessionItem());
   if (pObject != NULL)
   {
      pObject->clearVertices();
      pObject->addVertices(mOldVertices, mOldGeoVertices);
   }
}

void AddVertices::executeRedo()
{
   MultipointObjectImp* pObject = dynamic_cast<MultipointObjectImp*>(getSessionItem());
   if (pObject != NULL)
   {
      pObject->clearVertices();
      pObject->addVertices(mNewVertices, mNewGeoVertices);
   }
}

//////////////////////////
// GraphicUndoUtilities //
//////////////////////////

GraphicObject* GraphicUndoUtilities::getObject(const string& viewId, const string& layerId, const string& objectId)
{
   if ((layerId.empty() == true) || (objectId.empty() == true))
   {
      return NULL;
   }

   // Get the layer from the session manager
   Service<SessionManager> pManager;

   GraphicLayerImp* pLayer = dynamic_cast<GraphicLayerImp*>(pManager->getSessionItem(layerId));
   if (pLayer == NULL)
   {
      // The layer could be a layer in a product
      ProductView* pView = dynamic_cast<ProductView*>(pManager->getSessionItem(viewId));
      if (pView != NULL)
      {
         GraphicLayerImp* pLayoutLayer = dynamic_cast<GraphicLayerImp*>(pView->getLayoutLayer());
         if (pLayoutLayer != NULL)
         {
            if (pLayoutLayer->getId() == layerId)
            {
               pLayer = pLayoutLayer;
            }
         }

         if (pLayer == NULL)
         {
            ClassificationLayer* pClassificationLayer =
               dynamic_cast<ClassificationLayer*>(pView->getClassificationLayer());
            if (pClassificationLayer != NULL)
            {
               if (pClassificationLayer->getId() == layerId)
               {
                  pLayer = dynamic_cast<GraphicLayerImp*>(pClassificationLayer);
               }
            }
         }
      }
   }

   if (pLayer != NULL)
   {
      GraphicGroup* pGroup = pLayer->getGroup();
      if (pGroup != NULL)
      {
         if (pGroup->getId() == objectId)
         {
            return pGroup;
         }

         return getObject(pGroup, objectId);
      }
   }

   return NULL;
}

GraphicObject* GraphicUndoUtilities::getObject(const GraphicGroup* pGroup, const string& objectId)
{
   if ((pGroup == NULL) || (objectId.empty() == true))
   {
      return NULL;
   }

   const list<GraphicObject*>& objects = pGroup->getObjects();
   for (list<GraphicObject*>::const_iterator iter = objects.begin(); iter != objects.end(); ++iter)
   {
      GraphicObject* pObject = *iter;
      if (pObject != NULL)
      {
         if (pObject->getId() == objectId)
         {
            return pObject;
         }
      }

      GraphicGroup* pChildGroup = dynamic_cast<GraphicGroup*>(*iter);
      if (pChildGroup != NULL)
      {
         GraphicObject* pGroupObject = getObject(pChildGroup, objectId);
         if (pGroupObject != NULL)
         {
            return pGroupObject;
         }
      }
   }

   return NULL;
}

//////////////////////////
// SetGraphicObjectName //
//////////////////////////

SetGraphicObjectName::SetGraphicObjectName(GraphicObject *pObject, const string &oldName, const string &newName) :
   UndoAction(pObject),
   mOldName(oldName),
   mNewName(newName)
{
   GraphicObjectImp* pObjectImp = dynamic_cast<GraphicObjectImp*>(pObject);
   if (pObjectImp != NULL)
   {
      GraphicLayer* pLayer = pObjectImp->getLayer();
      if (pLayer != NULL)
      {
         View* pView = pLayer->getView();
         if (pView != NULL)
         {
            mViewId = pView->getId();
         }

         mLayerId = pLayer->getId();
      }
   }
   setText("Rename Graphic Object");
}

void SetGraphicObjectName::executeUndo()
{
   renameGraphicObject(mOldName);
}

void SetGraphicObjectName::executeRedo()
{
   renameGraphicObject(mNewName);
}

SessionItem* SetGraphicObjectName::getSessionItem() const
{
   if (mLayerId.empty() == false)
   {
      const string& objectId = getSessionItemId();
      if (objectId.empty() == false)
      {
         return GraphicUndoUtilities::getObject(mViewId, mLayerId, objectId);
      }
   }

   return NULL;
}

void SetGraphicObjectName::updateSessionItem(const string& oldId, const string& newId)
{
   if (oldId.empty() == false)
   {
      if (oldId == mViewId)
      {
         mViewId = newId;
      }

      if (oldId == mLayerId)
      {
         mLayerId = newId;
      }
   }

   UndoAction::updateSessionItem(oldId, newId);
}

void SetGraphicObjectName::renameGraphicObject(const string& name)
{
  
   GraphicObjectImp* pObjectImp = dynamic_cast<GraphicObjectImp*>(getSessionItem());
   if (pObjectImp == NULL)
   {
      return;
   }
   pObjectImp->setName(name);

}