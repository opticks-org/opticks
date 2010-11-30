/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GRAPHICLAYER_H
#define GRAPHICLAYER_H

#include "Layer.h"
#include "LocationType.h"
#include "ColorType.h"
#include "ConfigurationSettings.h"
#include "TypesFile.h"

#include <list>
#include <string>

class GraphicObject;

/**
 *  Adjusts the properties of a Graphic layer.
 *
 *  A graphic layer consists of a list of graphic objects.
 *  Each graphic object is individually selectable and can have
 *  its properties set.  %Setting a given property sets that property
 *  on each currently selected object and sets the value that all
 *  future objects will use as a default for that property.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The following methods are called: addObject(), removeObject(),
 *    selectObject(), deselectObject(), selectAllObjects(),
 *    deselectAllObjects(), groupSelection(), ungroupSelection(), pushBack(),
 *    popFront(), clearSelection().
 *  - Objects are added through the GUI.
 *  - Objects are deleted through the GUI.
 *  - Objects are manipulated through the GUI (rotation, moved handles).
 *  - Everything else documented in Layer.
 *
 *  @see     Layer
 */
class GraphicLayer : public Layer
{
public:
   SETTING(Alpha, GraphicLayer, double, 1.0);
   SETTING(Apex, GraphicLayer, double, 0.5);
   SETTING(ArcRegion, GraphicLayer, ArcRegion, ARC_CENTER)
   SETTING(Border, GraphicLayer, bool, false)
   SETTING(Fill, GraphicLayer, bool, false)
   SETTING(FillColor, GraphicLayer, ColorType, ColorType())
   SETTING(FillStyle, GraphicLayer, FillStyle, SOLID_FILL)
   SETTING(LineColor, GraphicLayer, ColorType, ColorType())
   SETTING(LineScaled, GraphicLayer, bool, false)
   SETTING(LineStyle, GraphicLayer, LineStyle, SOLID_LINE)
   SETTING(LineWidth, GraphicLayer, unsigned int, 0)
   SETTING(HatchStyle, GraphicLayer, SymbolType, SOLID)
   SETTING(Rotation, GraphicLayer, double, 0.0)
   SETTING(StartAngle, GraphicLayer, double, 0.0)
   SETTING(StopAngle, GraphicLayer, double, 0.0)
   SETTING(SymbolName, GraphicLayer, std::string, "")
   SETTING(SymbolSize, GraphicLayer, unsigned int, 1)
   SETTING(TextColor, GraphicLayer, ColorType, ColorType())
   SETTING(TextFont, GraphicLayer, std::string, "")
   SETTING(TextFontSize, GraphicLayer, unsigned int, 12)
   SETTING(TextBold, GraphicLayer, bool, false)
   SETTING(TextItalics, GraphicLayer, bool, false)
   SETTING(TextUnderline, GraphicLayer, bool, false)
   SETTING(UnitSystem, GraphicLayer, UnitSystem, UNIT_KM)

   /**
    *  Emitted with any<list<GraphicObject*> > when the list of selected objects changes.
    */
   SIGNAL_METHOD(GraphicLayer, ObjectsSelected)

   /**
    *  Emitted with boost::any<GraphicObject*> when an object completes insertion 
    *  through GUI manipulation. The GraphicObject will be the object which
    *  was inserted, but may be NULL if the object is invalid or otherwise
    *  not included in the layer.
    */
   SIGNAL_METHOD(GraphicLayer, ObjectInsertionCompleted);

   /**
    *  Adds a new graphic object to the layer.
    *
    *  This method adds a new graphic object to the layer with the given type.  The
    *  object is selected, displaying selection nodes representing the object's
    *  bounding rectangle.
    *
    *  NOTE: A TRAIL_OBJECT should not be added by plug-in code. It is for internal Core
    *  use only by the OverviewWindow.
    *
    *  @param   objectType
    *           The type of graphic object to add.
    *
    *  @return  A pointer to the new graphic object.  NULL is returned if an error
    *           occurred.
    *
    *  @notify  This method will notify Subject::signalModified.
    *
    *  @see     removeObject()
    */
   virtual GraphicObject* addObject(const GraphicObjectType& objectType) = 0;

   /**
    *  Removes an graphic object from the layer.
    *
    *  @param   pObject
    *           The graphic object to remove.
    *  @param   bDelete
    *           This flag should be set to TRUE to delete the object.  If it is
    *           FALSE, the object is not deleted.
    *
    *  @return  TRUE if the object was successfully removed, otherwise FALSE.
    *
    *  @notify  This method will notify Subject::signalModified.
    *
    *  @see     addObject()
    */
   virtual bool removeObject(GraphicObject* pObject, bool bDelete) = 0;

   /**
    *  Retrieves all graphic objects in the layer.
    *
    *  @param   objects
    *           A reference to a list that is filled with pointers to all
    *           graphic objects in the layer.  %Any pointers previously
    *           contained in the list are removed.
    *
    *  @see     getSelectedObjects()
    */
   virtual void getObjects(std::list<GraphicObject*>& objects) const = 0;

   /**
    *  Retrieves graphic objects of a given type in the layer.
    *
    *  @param   objectType
    *           The type of graphic object to get.
    *  @param   objects
    *           A reference to a list that is filled with pointers to all
    *           graphic objects of the given type in the layer.  %Any
    *           pointers previously contained in the list are removed.
    *
    *  @see     getSelectedObjects()
    */
   virtual void getObjects(const GraphicObjectType& objectType, 
      std::list<GraphicObject*>& objects) const = 0;

   /**
    *  Returns the number of graphic objects in the layer.
    *
    *  @return  The total number of graphic objects in the layer.
    */
   virtual unsigned int getNumObjects() const = 0;

   /**
    *  Returns the number of graphic objects of a given type in the layer.
    *
    *  @param   objectType
    *           The type of object to query for the number of objects contained in the layer.
    *
    *  @return  The number of annotation objects of the given type in the layer.
    */
   virtual unsigned int getNumObjects(const GraphicObjectType& objectType) const = 0;

   /**
    *  Returns a graphic object with the given name.
    *
    *  If more than one graphic object has the given name,
    *  an arbitrary object will be returned.
    *
    *  @param   name
    *           The name of the graphic object to retrieve.
    *
    *  @return  A graphic object with the specified name
    *           or NULL if no such object exists.
    */
   virtual GraphicObject* getObjectByName(const std::string &name) const = 0;

   /**
    *  Selects a graphic object and displays its selection nodes.
    *
    *  This method sets the annotation object as selected and creates selection nodes based
    *  on the object's bounding rectangle.  Some objects have white square nodes.  These 
    *  are the bounding box handles to resize the object.  Some object have yellow diamond nodes
    *  which can be used to move object points within the current bounding box.
    *
    *  @param   pObject
    *           The graphic object to select.  The pointer cannot be NULL.
    *
    *  @return  TRUE if the object was successfully selected, otherwise FALSE.
    *
    *  @notify  This method will notify signalObjectsSelected.
    *
    *  @see     selectAllObjects(), deselectObject()
    */
   virtual bool selectObject(GraphicObject* pObject) = 0;

   /**
    *  Selects all graphic objects on the layer.
    *
    *  @notify  This method will notify signalObjectsSelected.
    *
    *  @see     selectObject(), deselectAllObjects()
    */
   virtual void selectAllObjects() = 0;

   /**
    *  Queries whether an graphic object is selected.
    *
    *  @param   pObject
    *           The graphic object to query for its selection state.
    *
    *  @return  TRUE if the given object is selected, otherwise FALSE.
    *
    *  @see     selectObject(), deselectObject()
    */
   virtual bool isObjectSelected(GraphicObject* pObject) const = 0;

   /**
    *  Fills a list with the selected graphic objects.
    *
    *  @param   selectedObjects
    *           A reference to a list that is filled with pointers to all
    *           selected graphic objects.  %Any pointers previously
    *           contained in the list are removed.
    *
    *  @see     getNumSelectedObjects(), getObjects()
    */
   virtual void getSelectedObjects(std::list<GraphicObject*>& selectedObjects) const = 0;

   /**
    *  Fills a list with the selected graphic objects.
    *
    *  @param   objectType
    *           The type of object for which to retrieve selected objects
    *           contained in the layer.
    *  @param   selectedObjects
    *           A reference to a list that is filled with pointers to all
    *           selected graphic objects.  %Any pointers previously
    *           contained in the list are removed.
    *
    *  @see     getNumSelectedObjects(), getObjects()
    */
   virtual void getSelectedObjects(const GraphicObjectType& objectType,
      std::list<GraphicObject*>& selectedObjects) const = 0;

   /**
    *  Returns the number of currently selected graphic objects.
    *
    *  This method is equivalent to calling size() on the list filled by
    *  getSelectedObjects().
    *
    *  @return  The number of currently selected graphic objects.
    *
    *  @see     getSelectedObjects()
    */
   virtual unsigned int getNumSelectedObjects() const = 0;

   /**
    *  Returns the number of currently selected graphic objects of a given type.
    *
    *  @param   objectType
    *           The type of object to query for the number of selected objects contained
    *           in the layer.
    *
    *  @return  The number of currently selected annotation objects of the given type.
    */
   virtual unsigned int getNumSelectedObjects(const GraphicObjectType& objectType) const = 0;

   /**
    *  Deselects a graphic object.
    *
    *  This method deselects a graphic object by effectively hiding its selection
    *  nodes.
    *
    *  @param   pObject
    *           The graphic object to deselect.  The pointer cannot be NULL.
    *
    *  @return  TRUE if the item was successfully deselected, otherwise FALSE.
    *
    *  @notify  This method will notify signalObjectsSelected
    *
    *  @see     deselectAllObjects(), selectObject()
    */
   virtual bool deselectObject(GraphicObject* pObject) = 0;

   /**
    *  Delete selected objects from the layer.
    *
    *  @see  selectAllObjects(), selectObject()
    */
   virtual void deleteSelectedObjects() = 0;

   /**
    *  Clear the layer by deleting all objects in the layer.
    */
   virtual void clear() = 0;

   /**
    *  Deselects all graphic objects on the layer.
    *
    *  @notify  This method will notify signalObjectsSelected.
    *
    *  @see     selectAllObjects(), selectObject()
    */
   virtual void deselectAllObjects() = 0;

   /**
    *  Groups the objects currently selected into a single group.
    *
    *  @notify  This method will notify Subject::signalModified.
    *
    *  @see     ungroupSelection()
    */
   virtual void groupSelection() = 0;

   /**
    *  Ungroups the objects in selected groups into individual objects.
    *  The objects will all be selected after the ungroup.
    *
    *  @notify  This method will notify Subject::signalModified.
    *
    *  @see     groupSelection()
    */
   virtual void ungroupSelection() = 0;

   /**
    *  Unselects all currently selected objects.
    *
    *  @notify  This method will notify signalObjectsSelected.
    *
    *  @see     deselectAllObjects()
    */
   virtual void clearSelection() = 0;

   /**
    *  Brings the currently selected objects to the front of the layer.
    *  It leaves the order of the selected objects unchanged relative to
    *  each other.
    *
    *  @notify  This method will notify Subject::signalModified.
    *
    *  @see     pushBack()
    */
   virtual void popFront() = 0;

   /**
    *  Sends the currently selected objects to the back of the layer.
    *  It leaves the order of the selected objects unchanged relative to
    *  each other.
    *
    *  @notify  This method will notify Subject::signalModified.
    *
    *  @see     popFront()
    */
   virtual void pushBack() = 0;

   /**
    *  Queries whether graphic object name labels are currently drawn.
    *
    *  @return  True if labels are currently drawn, otherwise false.
    *
    *  @see     setShowLabels()
    */
   virtual bool getShowLabels() const = 0;

   /**
    *  Sets whether to draw name labels for each graphic object.
    *
    *  This method specifies whether to draw a label for the graphic object
    *  name that is set with the GraphicObject::setName() method.  This
    *  setting is not serialized with the layer.
    *
    *  @param   bShowLabels
    *           Determines whether or not to show the labels.
    *
    *  @see     getShowLabels()
    */
   virtual void setShowLabels(bool bShowLabels) = 0;

   /**
    * Correct the coordinate for whatever snapping may be required.
    *
    * GraphicLayer does not perform any correction.
    *
    * @param coord
    *        Coordinate to correct.
    *
    * @return The corrected coordinate.
    */
   virtual LocationType correctCoordinate(const LocationType &coord) const = 0;

   /**
   *  Queries whether the layer is locked or graphic objects can be 
   *  rotated or moved.
   *
   *  @return  True if the layer is locked, otherwise false.
   *
   *  @see     setLayerLocked()
   */
   virtual bool getLayerLocked() const = 0;

   /**
   *  Sets whether to allow rotation or movement for graphic objects in the layer.
   *
   *  This method specifies whether to allow the annotation objects in the layer to
   *  be rotated or moved.  This setting is not serialized with the layer.
   *
   *  @param   bLocked
   *           Determines whether or not to lock the layer.
   *
   *  @notify  This method will notify signalModified().
   *
   *  @see     getLayerLocked()
   */
   virtual void setLayerLocked(bool bLocked) = 0;

   /**
    *  Sets the graphic object that will be created when the user adds a new
    *  object with the mouse.
    *
    *  This method provides a means for objects to set the created graphic
    *  object for only this layer.  This value is reset when the user selects a
    *  new graphic object on the toolbar.
    *
    *  @param   type
    *           The graphic object to create.
    *
    *  @see     DesktopServices::setAoiSelectionTool(),
    *           DesktopServices::setAnnotationObject()
    */
   virtual void setCurrentGraphicObjectType(GraphicObjectType type) = 0;

   /**
    *  Returns the graphic object that will be created when the user adds a
    *  new object with the mouse.
    *
    *  @return  The graphic object that will be created when the user adds a
    *           new object.  This value may be different than the return value
    *           of DesktopServices::getAoiSelectionTool() or
    *           DesktopServices::getAnnotationObject() if the
    *           setCurrentGraphicObjectType() method was called and the user
    *           has not selected a new object type on the toolbar.
    */
   virtual GraphicObjectType getCurrentGraphicObjectType() const = 0;

   /**
    *  Tests an input scene coordinate location to see if a graphic object
    *  exists at that location.
    *
    *  This method first searches selected objects and then searches all
    *  graphic objects in the layer.
    *
    *  @param   sceneCoord
    *           The scene coordinate to test for the existence of a graphic
    *           object.
    *
    *  @return  A pointer to the first (topmost) graphic object found at the
    *           input scene coordinate location.  \c NULL is returned if no
    *           graphic objects exist at the given location.
    */
   virtual GraphicObject* hit(LocationType sceneCoord) const = 0;

protected:
   /**
    * This should be destroyed by calling SpatialDataView::deleteLayer.
    */
   virtual ~GraphicLayer() {}
};

#endif
