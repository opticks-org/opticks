/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PLOTVIEW_H
#define PLOTVIEW_H

#include "OrthographicView.h"
#include "TypesFile.h"

#include <list>

class AnnotationLayer;
class Gridlines;
class Locator;
class PlotObject;

/**
 *  Displays plot objects.
 *
 *  The plot view provides the means to display one or more plot objects.  Plot
 *  objects can be added and removed from the view and can also be selected and
 *  deselected.
 *
 *  The plot view defines the following mouse modes, where the name given is the name
 *  populated by MouseMode::getName():
 *  - AnnotationMode
 *  - LocatorMode
 *  - PanMode
 *  - SelectionMode
 *  - ZoomBoxMode
 *
 *  The annotation mouse mode allows users to add annotation objects to the
 *  plot.  Objects can be added programatically via an annotation layer that is
 *  accessible by calling the getAnnotationLayer() method.
 *
 *  For the locator mouse mode, the view contains a default Locator object to display
 *  on the plot.  When this mouse mode is active, the default behavior is to display
 *  the locator when the left mouse button is pressed and to hide the locator when the
 *  left mouse button is released.  Access to the locator to change its properties is
 *  provided with the getMouseLocator() method.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The following methods are called: addObject(), deleteObject(), selectObjects(),
 *    selectObject(), deleteSelectedObjects().
 *  - Everything else documented in OrthographicView.
 *
 *  @see     PlotViewExt1, PlotObject
 */
class PlotView : public OrthographicView
{
public:
   /**
    *  Emitted with any<PlotObject*> when an object is added to the plot.
    */
   SIGNAL_METHOD(PlotView, ObjectAdded);
   /**
    *  Emitted with any<PlotObject*> when an object is deleted from the plot.
    */
   SIGNAL_METHOD(PlotView, ObjectDeleted);
   /**
    *  Emitted with any<pair<PlotObject*,bool> > when an object is selected or deselected in the plot.
    */
   SIGNAL_METHOD(PlotView, ObjectSelected);

   /**
    *  Returns the plot type.
    *
    *  @return  The plot type.
    *
    *  @see     PlotType
    */
   virtual PlotType getPlotType() const = 0;

   /**
    *  Creates a new object but doesn't add it to the plot.
    *
    *  This method creates a new object of the given type.
    *  The object can be created as a primary object or secondary
    *  object.  A primary plot object is one that appears in the foreground of
    *  the plot and has an entry in the legend.  Most objects added to a plot
    *  will be primary objects, but sometimes objects such as gridlines can be
    *  created as secondary objects so that they will not be included with the
    *  main data objects in the plot.
    *
    *  @param   objectType
    *           The type of plot object to add.
    *  @param   bPrimary
    *           Set this value to TRUE to create a primary object or FALSE to
    *           create a secondary object.
    *
    *  @return  A pointer to the new plot object.  NULL is returned if an error
    *           occurred and the object could not be added.
    */
   virtual PlotObject* createObject(const PlotObjectType& objectType, bool bPrimary) = 0;

   /**
    *  Adds a new object to the plot.
    *
    *  This method adds a new object of the given type to the plot with the
    *  given type.  The object can be created as a primary object or secondary
    *  object.  A primary plot object is one that appears in the foreground of
    *  the plot and has an entry in the legend.  Most objects added to a plot
    *  will be primary objects, but sometimes objects such as gridlines can be
    *  created as secondary objects so that they will not be included with the
    *  main data objects in the plot.
    *
    *  @param   objectType
    *           The type of plot object to add.
    *  @param   bPrimary
    *           Set this value to TRUE to create a primary object or FALSE to
    *           create a secondary object.
    *
    *  @return  A pointer to the new plot object.  NULL is returned if an error
    *           occurred and the object could not be added.
    *
    *  @notify  This method will notify signalObjectAdded() with
    *           any<PlotObject*>.
    */
   virtual PlotObject* addObject(const PlotObjectType& objectType, bool bPrimary) = 0;

   /**
    *  Retrieves all objects in the plot.
    *
    *  @param   objects
    *           A list that is populated with pointers to all objects in the
    *           plot.
    */
   virtual void getObjects(std::list<PlotObject*>& objects) const = 0;

   /**
    *  Retrieves objects in the plot of a given type.
    *
    *  @param   objectType
    *           The plot object type for which to retrieve the current objects.
    *  @param   objects
    *           A list that is populated with pointers to all objects in the
    *           plot of the given type.
    */
   virtual void getObjects(const PlotObjectType& objectType, std::list<PlotObject*>& objects) const = 0;

   /**
    *  Queries whether an object exists in this plot.
    *
    *  @param   pObject
    *           The object for which to query the plot.
    *
    *  @return  TRUE if this plot contains the object, otherwise FALSE.
    */
   virtual bool containsObject(PlotObject* pObject) const = 0;

   /**
    *  Returns the number of objects in the plot.
    *
    *  @return  The total number of objects in the plot.
    */
   virtual unsigned int getNumObjects() const = 0;

   /**
    *  Removes an object from the plot and deletes it.
    *
    *  @param   pObject
    *           The annotation object to delete.
    *
    *  @return  TRUE if the object was successfully removed and deleted,
    *           otherwise FALSE.
    *
    *  @see     clear()
    *
    *  @notify  This method will notify signalObjectDeleted() with
    *           any<PlotObject*>.
    */
   virtual bool deleteObject(PlotObject* pObject) = 0;

   /**
    *  Removes all objects from the plot.
    */
   virtual void clear() = 0;

   /**
    *  Moves a given object to the front of the plot.
    *
    *  This method repositions the given object to be the topmost object in the
    *  plot.
    *
    *  @param   pObject
    *           The object to move to the front of the plot.
    *
    *  @see     moveObjectToBack()
    */
   virtual bool moveObjectToFront(PlotObject* pObject) = 0;

   /**
    *  Moves a given object to the back of the plot.
    *
    *  This method repositions the given object to be the bottommost object in the
    *  plot.  If the object is a primary object, it will still be on top of any
    *  secondary objects.
    *
    *  @param   pObject
    *           The object to move to the back of the plot.
    *
    *  @see     moveObjectToFront()
    */
   virtual bool moveObjectToBack(PlotObject* pObject) = 0;

   /**
    *  Selects or deselects an object in the plot.
    *
    *  This method sets a given plot object as selected and may alter how the object
    *  is drawn in the plot.  This is done mainly for user feedback that the object
    *  is selected.  For some objects, like the histogram, selecting the object does
    *  nothing.
    *
    *  @param   pObject
    *           The plot object to select.  Cannot be NULL.
    *  @param   bSelect
    *           Set this value to TRUE to select the object or FALSE to deselect the
    *           object.
    *
    *  @return  TRUE if the object was successfully selected or deselected, otherwise
    *           FALSE.
    *
    *  @see     selectObjects()
    *
    *  @notify  This method will notify signalObjectSelected() with
    *           any<pair<PlotObject*,bool> >.
    */
   virtual bool selectObject(PlotObject* pObject, bool bSelect) = 0;

   /**
    *  Selects or deselects multiple objects in the plot.
    *
    *  @param   objects
    *           The plot objects to select.
    *  @param   bSelect
    *           Set this value to TRUE to select the objects or FALSE to deselect the
    *           objects.
    *
    *  @see     selectObject()
    *
    *  @notify  This method will notify signalObjectSelected() with
    *           any<pair<PlotObject*,bool> > for each object selected or
    *           deselected.
    */
   virtual void selectObjects(const std::list<PlotObject*>& objects, bool bSelect) = 0;

   /**
    *  Selects or deselects all objects in the plot.
    *
    *  @param   bSelect
    *           Set this value to TRUE to select the objects or FALSE to deselect the
    *           objects.
    *
    *  @see     selectObject()
    *
    *  @notify  This method will notify signalObjectSelected() with
    *           any<pair<PlotObject*,bool> > for each object selected or
    *           deselected.
   */
   virtual void selectObjects(bool bSelect) = 0;

   /**
    *  Retrieves the currently selected objects on the plot.
    *
    *  @param   selectedObjects
    *           A list that is populated with pointers to selected plot objects.
    *  @param   filterVisible
    *           True if selectedObjects should contain only visible objects.
    *
    *  @see     getNumSelectedObjects()
    */
   virtual void getSelectedObjects(std::list<PlotObject*>& selectedObjects, bool filterVisible) const = 0;

   /**
    *  Returns the number of selected objects in the plot.
    *
    *  @param filterVisible
    *         True if the count should only include visible objects
    *
    *  @return  The number of selected objects in the plot.
    */
   virtual unsigned int getNumSelectedObjects(bool filterVisible) const = 0;

   /**
    *  Removes all selected objects from the plot and deletes them.
    *
    *  @param   filterVisible
    *           True if it should delete only visible selected objects.
    *
    *  @see     deleteObject()
    *
    *  @notify  This method will notify signalObjectDeleted() with
    *           any<PlotObject*> for each object deleted.
    */
   virtual void deleteSelectedObjects(bool filterVisible) = 0;

   /**
    * Set the selection mode for this plot.
    *
    * @param mode
    *        The new selection mode.
    */
   virtual void setSelectionMode(PlotSelectionModeType mode) = 0;

   /**
    * Get the selection mode for this plot.
    *
    * @return The current selection mode.
    */
   virtual PlotSelectionModeType getSelectionMode() const = 0;

   /**
    * Get the selection display mode for this plot.
    *
    * @return The current selection mode.
    */
   virtual PointSelectionDisplayType getSelectionDisplayMode() const = 0;

   /**
    * Set the selection display mode for this plot.
    *
    * @param mode
    *        The new selection mode.
    */
   virtual void setSelectionDisplayMode(PointSelectionDisplayType mode) = 0;

   /**
    * Returns the annotation layer for the plot.
    *
    *  The annotation layer is drawn on top of the data objects and the
    *  gridlines.  The layer can be modified, but ownership remains with the
    *  plot view.
    *
    *  @return  A pointer to the annotation layer.
    */
   virtual AnnotationLayer* getAnnotationLayer() const = 0;

   /**
    *  Returns the mouse locator object used in the "LocatorMode" mouse mode.
    *
    *  @return  A pointer to the locator object.
    */
   virtual Locator* getMouseLocator() = 0;

   /**
    *  Returns read-only access to the mouse locator object used in the
    *  "LocatorMode" mouse mode.
    *
    *  @return  A const pointer to the locator object.  The locator object
    *           represented by the returned pointer should not be modified.  To
    *           modify the values, call the non-const version of
    *           getMouseLocator().
    */
   virtual const Locator* getMouseLocator() const = 0;

   /**
    *  Translate from world coordinates to data coordinates for this plot.
    *
    *  @param   worldX
    *           World x-coordinate
    *  @param   worldY
    *           World y-coordinate
    *  @param   dataX
    *           X-coordinate of the plot
    *  @param   dataY
    *           Y-coordinate of the plot
    */
   virtual void translateWorldToData(double worldX,double worldY, double& dataX, double& dataY) const = 0;

   /**
    *  Translate from data coordinates to world coordinates for this plot.
    *
    *  @param   dataX
    *           X-coordinate of the plot
    *  @param   dataY
    *           Y-coordinate of the plot
    *  @param   worldX
    *           World x-coordinate
    *  @param   worldY
    *           World y-coordinate
    */
   virtual void translateDataToWorld(double dataX, double dataY, double& worldX, double& worldY) const = 0;

   /**
    *  Translate from screen coordinates to data coordinates for this plot.
    *
    *  @param   screenX
    *           Screen x-coordinate
    *  @param   screenY
    *           Screen y-coordinate
    *  @param   dataX
    *           X-coordinate of the plot
    *  @param   dataY
    *           Y-coordinate of the plot
    */
   virtual void translateScreenToData(double screenX, double screenY, double& dataX, double& dataY) const = 0;

   /**
    *  Translate from data coordinates to screen coordinates for this plot.
    *
    *  @param   dataX
    *           X-coordinate of the plot
    *  @param   dataY
    *           Y-coordinate of the plot
    *  @param   screenX
    *           Screen x-coordinate
    *  @param   screenY
    *           Screen y-coordinate
    */
   virtual void translateDataToScreen(double dataX, double dataY, double& screenX, double& screenY) const = 0;

   /**
    *  Enables or disables shading when displaying pointset lines.
    *
    *  @param   shading
    *           Set to true to enable GL_SMOOTH shading when drawing pointset lines.  The default
    *           is to disable shading (GL_FLAT).
    */
   virtual void setEnableShading( bool shading ) = 0;
   
   /**
    *  Returns true if shading is enabled, otherwise false.
    *
    *  @return  True if shading is enabled, otherwise false.
    */
   virtual bool isShadingEnabled() const = 0;

protected:
   /**
    * This object should be destroyed by calling DesktopServices::deleteView().
    */
   virtual ~PlotView() {}
};

/**
 *  Extends capability of the PlotView interface.
 *
 *  This class provides additional capability for the PlotView interface class.
 *  A pointer to this class can be obtained by performing a dynamic cast on a
 *  pointer to PlotView or any of its subclasses.
 *
 *  @warning A pointer to this class can only be used to call methods contained
 *           in this extension class and cannot be used to call any methods in
 *           PlotView or its subclasses.
 */
class PlotViewExt1
{
public:
   /**
    *  Sets a margin around the overall plot extents.
    *
    *  The plot extents as returned by PlotView::getExtents() is calculated
    *  based on the extents of all primary plot objects it contains.  An
    *  additional amount can be added to the plot extents by specifying a
    *  margin factor that is multiplied to the data range of the object
    *  extents.  For example a margin factor of 0.01 subtracts one percent of
    *  the data range from the minimum value and adds one percent of the data
    *  range to the maximum value.
    *
    *  Calling this method sets the margin for both X and Y dimensions.
    *
    *  The default margin factor is 0.0.
    *
    *  @param   marginFactor
    *           The factor that should be multiplied by the data range to use
    *           as additional margin around the plot objects.  If the given
    *           value is less than zero, the margin factor is set to 0.0.
    */
   virtual void setExtentsMargin(double marginFactor) = 0;

   /**
    *  Returns the margin around the overall plot extents.
    *
    *  @return  The factor that is multiplied by the data range to use
    *           as additional margin around the plot objects.
    *
    *  @see     setExtentsMargin()
    */
   virtual double getExtentsMargin() const = 0;

protected:
   /**
    *  This object should be destroyed by calling DesktopServices::deleteView().
    */
   virtual ~PlotViewExt1() {}
};

#endif
