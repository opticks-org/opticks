/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GRAPHICLAYERIMP_H
#define GRAPHICLAYERIMP_H

#include "AttachmentPtr.h"
#include "LayerImp.h"
#include "LocationType.h"
#include "SessionExplorer.h"
#include "TypesFile.h"

#include <list>
#include <set>
#include <memory>

#include <QtCore/QString>
#include <QtCore/QPoint>
#include <QtGui/QColor>
#include <QtGui/QFont>

class DataElement;
class GraphicGroup;
class GraphicLayer;
class GraphicObject;
class GraphicObjectImp;
class GraphicProperty;
class UndoLock;

/**
  * This class displays an graphic layer. It contains a list of graphic
  * objects and manages all drawing, inserting, deleting and manipulation of
  * objects in that list.
  */
class GraphicLayerImp : public LayerImp
{
   Q_OBJECT

public:
   static bool isKindOfLayer(const std::string& className);
   static void getLayerTypes(std::vector<std::string>& classList);

   GraphicLayerImp(const std::string& id, const std::string& layerName, DataElement* pElement);
   ~GraphicLayerImp();

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   GraphicLayerImp& operator= (const GraphicLayerImp& graphicLayer);

   virtual bool getExtents(double& x1, double& y1, double& x4, double& y4);
   virtual LayerType getLayerType() const;
   using LayerImp::setName;

   void setPaperSize(LocationType size);
   LocationType getPaperSize() const;

   virtual bool load(const QString& strFilename);

   virtual void groupSelection();
   virtual void ungroupSelection();
   virtual void popSelectedObjectToFront();
   virtual void pushSelectedObjectToBack();
   int getObjectStackingIndex(GraphicObject* pObject) const;
   void setObjectStackingIndex(GraphicObject* pObject, int index);

   bool acceptsMouseEvents() const;

   bool processMousePress(const QPoint& screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
      Qt::KeyboardModifiers modifiers);
   bool processMouseMove(const QPoint& screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
      Qt::KeyboardModifiers modifiers);
   bool processMouseRelease(const QPoint& screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
      Qt::KeyboardModifiers modifiers);
   bool processMouseDoubleClick(const QPoint& screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
      Qt::KeyboardModifiers modifiers);

   std::vector<ColorType> getColors() const;

   void draw();
   
   bool insertingObjectNull() const;

   virtual GraphicObject* addObject(const GraphicObjectType& objectType, LocationType point);
   virtual bool removeObject(GraphicObject* pObject, bool bDelete);
   virtual bool hasObject(GraphicObject* pObject) const;
   virtual std::list<GraphicObject*> getObjects() const;
   virtual std::list<GraphicObject*> getObjects(const GraphicObjectType& objectType) const;
   virtual unsigned int getNumObjects() const;
   virtual unsigned int getNumObjects(const GraphicObjectType& objectType) const;
   virtual GraphicObject* getObjectByName(const std::string &name) const;

   virtual bool selectObject(GraphicObject* pObject);
   virtual int selectObjects(const LocationType& corner1, const LocationType& corner2);
   virtual void selectAllObjects();
   virtual bool isObjectSelected(GraphicObject* pObject) const;
   virtual void getSelectedObjects(std::list<GraphicObject*>& selectedObjects) const;
   void getSelectedObjectsImpl(std::list<GraphicObject*>& selectedObjects) const;
   virtual void getSelectedObjects(const GraphicObjectType& objectType,
      std::list<GraphicObject*>& selectedObjects) const;
   void getSelectedObjectsImpl(const GraphicObjectType& objectType,
      std::list<GraphicObject*>& selectedObjects) const;
   virtual unsigned int getNumSelectedObjects() const;
   unsigned int getNumSelectedObjectsImpl() const;
   virtual unsigned int getNumSelectedObjects(const GraphicObjectType& objectType) const;
   unsigned int getNumSelectedObjectsImpl(const GraphicObjectType& objectType) const;
   virtual bool deselectObject(GraphicObject* pObject);
   virtual void deselectAllObjects();
   virtual void deleteSelectedObjects();
   virtual void clear();
   void replicateObject(GraphicObject *pDest, GraphicObject *pSource);

   void nudgeSelectedObjects(int x, int y);
   void moveSelectedObjects(LocationType delta);

   void alignSelectedObjects(GraphicAlignment alignment);
   void distributeSelectedObjects(GraphicDistribution distribution);

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

   void cloneSelection(GraphicLayer *pDest);
   GraphicObject *hit(LocationType sceneCoord) const;
   GraphicObject *hit(const QPoint &screenCoord) const;

   void setHideSelectionBox(bool hide);
   bool getShowLabels() const;
   void setShowLabels(bool bShowLabels);
   bool getLayerLocked() const;
   void setLayerLocked(bool bLocked);

   /**
    * Determine if the type is one which this layer can accept.
    */
   bool canContainGraphicObjectType(GraphicObjectType type);

   virtual GraphicGroup *getGroup() const;

   GraphicObjectType getCurrentGraphicObjectType() const;

   /**
    *  Attempts to select a selection handle of a currently selected object.
    *
    *  @param   screenCoord
    *           The scene coord location of the mouse click
    *  @param   pObject
    *           Returns the object that owns the handle, or NULL
    *  @param   handle
    *           returns the index of the selected handle, or -1
    *
    *  @return  Returns whether or not a selection handle was selected
    */
   bool grabHandle(const QPoint &screenCoord, GraphicObject *&pObject, int &handle) const;
   bool grabHandle(LocationType sceneCoord, GraphicObject *&pObject, int &handle) const;

   /**
    * Correct the coordinate for whatever snapping is required.
    *
    * The implementation in GraphicLayerImp is a passthrough.
    *
    * @param coord
    *        Coordinate to correct.
    *
    * @return The corrected coordinate.
    */
   virtual LocationType correctCoordinate(const LocationType &coord) const;

   /**
    * Call this when your object is done being inserted.
    *
    * @param bValidObject
    *        If this value is false, the object is removed from the group and
    *        deleted.
    */
   virtual void completeInsertion(bool bValidObject = true);

   virtual QColor getLabelColor(const GraphicObjectImp *pObj);

   /** 
    * Determine if the drawing may be pixel based.
    *
    * @return True if drawing will ever be pixel based, false otherwise.
    *
    * @see willDrawAsPixels
    */
   virtual bool mayDrawAsPixels() const;

   /**
    * Determine if the drawing is currently pixel based.
    *
    * This method may give a different response than mayDrawObjectsAsPixels()
    * because if the layer is zoomed out enough, it should draw as vector.
    *
    * @return True if current drawing should be pixel based, false otherwise.
    *
    * @see mayDrawAsPixels
    */
   virtual bool willDrawAsPixels() const;

   void drawSymbols(const std::string &symbolName, const std::vector<LocationType> &points, 
                   double screenSize, double objectRotation);

   virtual void temporaryGlContextChange();

public slots:
   void setCurrentGraphicObjectType(GraphicObjectType type);
   void cleanUpBadObject(GraphicObject *pObj);
   void reset();
   void clearInsertingObject();

protected:
   /**
    * Add to the list of graphic objects which this layer can accept.
    */
   void addAcceptableGraphicType(GraphicObjectType type);

   void removeAcceptableGraphicType(GraphicObjectType type);
   void clearAcceptableGraphicTypes();

   virtual void drawGroup();

   void onElementModified();

   void updateContextMenu(Subject& subject, const std::string& signal, const boost::any& value);
   void layerActivated(bool activated);

protected slots:
   void deleteObject();
   void updateHandles(GraphicProperty* pProperty);

signals:
   void objectAdded(GraphicObject* pObject);
   void objectRemoved(GraphicObject* pObject);
   void selectionChanged();
   void objectsSelected(std::list<GraphicObject*>& selectedObjects);
   void currentTypeChanged(GraphicObjectType newType);

private:
   AttachmentPtr<SessionExplorer> mpExplorer;

   bool mbHideSelectionBox;
   bool mShowLabels;
   bool mLayerLocked;
   bool mProcessingMouseEvent;
   std::set<GraphicObjectType> mAcceptableTypes;
   mutable bool mGroupHasLayerSet;

   GraphicObjectImp* mpInsertingObject;

   GraphicObjectType mCurrentType;

   UndoLock* mpUndoLock;

   // a list of the currently selected objects
   std::list<GraphicObject*> mSelectedObjects;
   std::vector<LocationType> mObjectAnchors;
   LocationType mStartAnchor;

   // currently 3
   double mHandleSize;

   // draws a single selection handle
   void drawHandle(LocationType point, bool bSelectionHandle);

   void drawSelectionRectangle(LocationType ll, LocationType ur) const;

   /**
    *  Determines if the object type is a physical object that is seen on the layer
    */
   bool isVisibleObjectType(GraphicObjectType eType) const;

   void initializeFromGroup();
};

#define GRAPHICLAYERADAPTEREXTENSION_CLASSES \
   LAYERADAPTEREXTENSION_CLASSES

#define GRAPHICLAYERADAPTER_METHODS(impClass) \
   LAYERADAPTER_METHODS(impClass) \
   GraphicObject* addObject(const GraphicObjectType& objectType) \
   { \
      return impClass::addObject(objectType, LocationType()); \
   } \
   bool removeObject(GraphicObject* pObject, bool bDelete) \
   { \
      return impClass::removeObject(pObject, bDelete); \
   } \
   void getObjects(std::list<GraphicObject*>& objects) const \
   { \
      objects = impClass::getObjects(); \
   } \
   void getObjects(const GraphicObjectType& objectType, std::list<GraphicObject*>& objects) const \
   { \
      objects = impClass::getObjects(objectType); \
   } \
   unsigned int getNumObjects() const \
   { \
      return impClass::getNumObjects(); \
   } \
   unsigned int getNumObjects(const GraphicObjectType& objectType) const \
   { \
      return impClass::getNumObjects(objectType); \
   } \
   GraphicObject* getObjectByName(const std::string &name) const \
   { \
      return impClass::getObjectByName(name); \
   } \
   bool selectObject(GraphicObject* pObject) \
   { \
      return impClass::selectObject(pObject); \
   } \
   void selectAllObjects() \
   { \
      return impClass::selectAllObjects(); \
   } \
   bool isObjectSelected(GraphicObject* pObject) const \
   { \
      return impClass::isObjectSelected(pObject); \
   } \
   void getSelectedObjects(std::list<GraphicObject*>& selectedObjects) const \
   { \
      return impClass::getSelectedObjects(selectedObjects); \
   } \
   void getSelectedObjects(const GraphicObjectType& objectType, \
      std::list<GraphicObject*>& selectedObjects) const \
   { \
      return impClass::getSelectedObjects(objectType, selectedObjects); \
   } \
   unsigned int getNumSelectedObjects() const \
   { \
      return impClass::getNumSelectedObjects(); \
   } \
   unsigned int getNumSelectedObjects(const GraphicObjectType& objectType) const \
   { \
      return impClass::getNumSelectedObjects(objectType); \
   } \
   bool deselectObject(GraphicObject* pObject) \
   { \
      return impClass::deselectObject(pObject); \
   } \
   void deselectAllObjects() \
   { \
      return impClass::deselectAllObjects(); \
   } \
   void deleteSelectedObjects() \
   { \
      return impClass::deleteSelectedObjects(); \
   } \
   void clear() \
   { \
      return impClass::clear(); \
   } \
   void groupSelection() \
   { \
      return impClass::groupSelection(); \
   } \
   void ungroupSelection() \
   { \
      return impClass::ungroupSelection(); \
   } \
   bool getShowLabels() const \
   { \
      return impClass::getShowLabels(); \
   } \
   void setShowLabels(bool bShowLabels) \
   { \
      return impClass::setShowLabels(bShowLabels); \
   } \
   LocationType correctCoordinate(const LocationType &coord) const\
   { \
      return impClass::correctCoordinate(coord); \
   } \
   void clearSelection() \
   { \
      return impClass::deselectAllObjects(); \
   } \
   void popFront() \
   { \
      return impClass::popSelectedObjectToFront(); \
   } \
   void pushBack() \
   { \
      return impClass::pushSelectedObjectToBack(); \
   } \
   bool getLayerLocked() const \
   { \
      return impClass::getLayerLocked(); \
   } \
   void setLayerLocked(bool bLocked) \
   { \
      return impClass::setLayerLocked(bLocked); \
   } \
   void setCurrentGraphicObjectType(GraphicObjectType type) \
   { \
      impClass::setCurrentGraphicObjectType(type); \
   } \
   GraphicObjectType getCurrentGraphicObjectType() const \
   { \
      return impClass::getCurrentGraphicObjectType(); \
   } \
   GraphicObject* hit(LocationType sceneCoord) const \
   { \
      return impClass::hit(sceneCoord); \
   }

#endif
