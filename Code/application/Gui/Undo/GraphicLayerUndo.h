/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GRAPHICLAYERUNDO_H
#define GRAPHICLAYERUNDO_H

#include "LayerUndo.h"
#include "LocationType.h"
#include "UndoAction.h"

#include <list>
#include <string>
#include <vector>

class AnnotationLayer;
class GraphicGroup;
class GraphicLayer;
class GraphicObject;
class GraphicObjectImp;
class GraphicProperty;
class Layer;
class MultipointObjectImp;
class PolylineObjectImp;

class GraphicLayerMemento : public LayerMemento
{
protected:
   GraphicLayerMemento(GraphicLayer* pLayer);
   void toLayer(Layer* pLayer) const;

private:
   GraphicLayerMemento(const GraphicLayerMemento& rhs);
   GraphicLayerMemento& operator=(const GraphicLayerMemento& rhs);
};


class AnnotationLayerMemento : public GraphicLayerMemento
{
public:
   AnnotationLayerMemento(AnnotationLayer* pLayer);

private:
   AnnotationLayerMemento(const AnnotationLayerMemento& rhs);
   AnnotationLayerMemento& operator=(const AnnotationLayerMemento& rhs);
};


class CreateDestroyGraphicObject : public UndoAction
{
public:
   CreateDestroyGraphicObject(GraphicGroup* pGroup, GraphicObject* pObject);
   ~CreateDestroyGraphicObject();

   SessionItem* getSessionItem() const;
   void updateSessionItem(const std::string& oldId, const std::string& newId);

protected:
   void createObject();
   void destroyObject();

private:
   CreateDestroyGraphicObject(const CreateDestroyGraphicObject& rhs);
   CreateDestroyGraphicObject& operator=(const CreateDestroyGraphicObject& rhs);
   std::string mViewId;
   std::string mLayerId;
   std::string mObjectId;
   bool mLayerGroup;
   GraphicObjectImp* mpClone;
   int mIndex;
};


class AddGraphicObject : public CreateDestroyGraphicObject
{
public:
   AddGraphicObject(GraphicGroup* pGroup, GraphicObject* pObject);

   void executeUndo();
   void executeRedo();

private:
   AddGraphicObject(const AddGraphicObject& rhs);
   AddGraphicObject& operator=(const AddGraphicObject& rhs);
};


class RemoveGraphicObject : public CreateDestroyGraphicObject
{
public:
   RemoveGraphicObject(GraphicGroup* pGroup, GraphicObject* pObject);

   void executeUndo();
   void executeRedo();

private:
   RemoveGraphicObject(const RemoveGraphicObject& rhs);
   RemoveGraphicObject& operator=(const RemoveGraphicObject& rhs);
};


class SetGraphicObjectProperty : public UndoAction
{
public:
   SetGraphicObjectProperty(GraphicObject* pObject, const GraphicProperty* pOldProperty,
      const GraphicProperty* pNewProperty);
   ~SetGraphicObjectProperty();

   SessionItem* getSessionItem() const;
   void updateSessionItem(const std::string& oldId, const std::string& newId);

   void executeUndo();
   void executeRedo();

private:
   SetGraphicObjectProperty(const SetGraphicObjectProperty& rhs);
   SetGraphicObjectProperty& operator=(const SetGraphicObjectProperty& rhs);

   std::string mViewId;
   std::string mLayerId;
   GraphicProperty* mpOldProperty;
   GraphicProperty* mpNewProperty;
};

class SetGraphicObjectName : public UndoAction
{
public:
   SetGraphicObjectName(GraphicObject *pObject, const std::string& oldName, const std::string& newName);

   SessionItem* getSessionItem() const;
   void updateSessionItem(const std::string& oldId, const std::string& newId);

   void executeUndo();
   void executeRedo();

protected:
   void renameGraphicObject(const std::string& name);

private:
   SetGraphicObjectName(const SetGraphicObjectName& rhs);
   SetGraphicObjectName& operator=(const SetGraphicObjectName& rhs);
   std::string mViewId;
   std::string mLayerId;
   std::string mOldName;
   std::string mNewName;
};

class SetGraphicStackingOrder : public UndoAction
{
public:
   SetGraphicStackingOrder(GraphicLayer* pLayer, GraphicObject* pObject, int oldIndex, int newIndex);

   void updateSessionItem(const std::string& oldId, const std::string& newId);

   void executeUndo();
   void executeRedo();

private:
   SetGraphicStackingOrder(const SetGraphicStackingOrder& rhs);
   SetGraphicStackingOrder& operator=(const SetGraphicStackingOrder& rhs);
   std::string mObjectId;
   int mOldIndex;
   int mNewIndex;
};


class GroupUngroupGraphicObjects : public UndoAction
{
public:
   GroupUngroupGraphicObjects(GraphicLayer* pLayer, GraphicGroup* pGroup);

   void updateSessionItem(const std::string& oldId, const std::string& newId);

protected:
   void group();
   void ungroup();

private:
   GroupUngroupGraphicObjects(const GroupUngroupGraphicObjects& rhs);
   GroupUngroupGraphicObjects& operator=(const GroupUngroupGraphicObjects& rhs);
   std::list<std::string> mObjectIds;
   std::string mGroupId;
};


class GroupGraphicObjects : public GroupUngroupGraphicObjects
{
public:
   GroupGraphicObjects(GraphicLayer* pLayer, GraphicGroup* pGroup);

   void executeUndo();
   void executeRedo();

private:
   GroupGraphicObjects(const GroupGraphicObjects& rhs);
   GroupGraphicObjects& operator=(const GroupGraphicObjects& rhs);
};


class UngroupGraphicObjects : public GroupUngroupGraphicObjects
{
public:
   UngroupGraphicObjects(GraphicLayer* pLayer, GraphicGroup* pGroup);

   void executeUndo();
   void executeRedo();

private:
   UngroupGraphicObjects(const UngroupGraphicObjects& rhs);
   UngroupGraphicObjects& operator=(const UngroupGraphicObjects& rhs);
};

class AddVertices : public UndoAction
{
public:
   AddVertices(MultipointObjectImp* pObject, const std::vector<LocationType>& oldVertices,
      const std::vector<LocationType>& oldGeoVertices, const std::vector<LocationType>& newVertices,
      const std::vector<LocationType>& newGeoVertices);

   SessionItem* getSessionItem() const;
   void updateSessionItem(const std::string& oldId, const std::string& newId);

   void executeUndo();
   void executeRedo();

private:
   AddVertices(const AddVertices& rhs);
   AddVertices& operator=(const AddVertices& rhs);

   std::string mViewId;
   std::string mLayerId;
   std::vector<LocationType> mOldVertices;
   std::vector<LocationType> mOldGeoVertices;
   std::vector<LocationType> mNewVertices;
   std::vector<LocationType> mNewGeoVertices;
};

class NewPath : public UndoAction
{
public:
   NewPath(PolylineObjectImp* pObject, unsigned int path);

   SessionItem* getSessionItem() const;
   void updateSessionItem(const std::string& oldId, const std::string& newId);

   void executeUndo();
   void executeRedo();

private:
   NewPath(const NewPath& rhs);
   NewPath& operator=(const NewPath& rhs);

   std::string mViewId;
   std::string mLayerId;
   unsigned int mPath;
};

namespace GraphicUndoUtilities
{
   GraphicObject* getObject(const std::string& viewId, const std::string& layerId, const std::string& objectId);
   GraphicObject* getObject(const GraphicGroup* pGroup, const std::string& objectId);
}

#endif
