/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef PLOTOBJECT_H
#define PLOTOBJECT_H

#include "Subject.h"
#include "TypesFile.h"

#include <string>

/**
 *  The base class for all objects displayed on a plot.
 *
 *  This class contains properties for plot objects that are common to all objects.
 *  Each object has a name, type, display state, primary or secondary state, selection
 *  state, and coordinate extents.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The following methods are called: setObjectName(), setVisible(), 
 *    setSelected().
 *  - Everything else documented in Subject.
 *
 *  @see     PlotView
 */
class PlotObject : public Subject
{
public:
   /**
    *  Emitted with any<std::string> when the object is renamed.
    */
   SIGNAL_METHOD(PlotObject, Renamed)
   /**
    *  Emitted with any<bool> when the object is shown or hidden.
    */
   SIGNAL_METHOD(PlotObject, VisibilityChanged)
   /**
    *  Emitted with any<bool> when the object is selected or deselected.
    */
   SIGNAL_METHOD(PlotObject, Selected)

   /**
    *  Sets the object name.
    *
    *  @param   objectName
    *           The new object name.  Cannot be empty.
    *
    *  @notify  This method will notify signalRenamed with any<std::string>.
    */
   virtual void setObjectName(const std::string& objectName) = 0;

   /**
    *  Retrieves the object name.
    *
    *  @param   objectName
    *           A string that is populated with the object name.
    */
   virtual void getObjectName(std::string& objectName) const = 0;

   /**
    *  Returns the specific object type.
    *
    *  @return  The object type.
    *
    *  @see     PlotObjectType
    */
   virtual PlotObjectType getType() const = 0;

   /**
    *  Toggles the display of the object on the plot.
    *
    *  @param   bVisible
    *           Set this value to TRUE to show the object or FALSE to hide the object.
    *
    *  @notify  This method will notify signalVisibilityChanged with any<bool>.
    */
   virtual void setVisible(bool bVisible) = 0;

   /**
    *  Queries whether the object is currently displayed on the plot.
    *
    *  @return  TRUE if the object is displayed or FALSE if the object is hidden.
    */
   virtual bool isVisible() const = 0;

   /**
    *  Queries whether the object is a primary object on the plot.
    *
    *  A primary plot object is one that appears in the foreground of the plot and
    *  has an entry in the legend.  Most objects added to a plot will be primary objects,
    *  but objects such as gridlines can be created as secondary objects since they are
    *  not the main focus of the plot data.
    *
    *  @return  TRUE if the object is a primary object or FALSE if the object is a
    *           secondary object.
    *
    *  @see     PlotView::addObject()
    */
   virtual bool isPrimary() const = 0;

   /**
    *  Sets the selection state of the object.
    *
    *  This method sets the plot object as selected and may alter how the object is drawn
    *  in the plot.  This is done mainly for user feedback that the object is selected.
    *  For some objects, like the histogram, selecting the object does nothing.
    *
    *  @param   bSelect
    *           Set this value to TRUE to select the object or FALSE to deselect the
    *           object.
    *
    *  @notify  This method will notify signalSelected with any<bool>.
    */
   virtual void setSelected(bool bSelect) = 0;

   /**
    *  Queries whether the object is currently selected.
    *
    *  @return  TRUE if the object is selected or FALSE if the object is not selected.
    *
    *  @see     setSelected()
    */
   virtual bool isSelected() const = 0;

   /**
    *  Retrieves the bounding box of the object.
    *
    *  This method retreives the minimum and maximum coordinates of the object at its
    *  current location in the plot.  These are provided as world coordinates.
    *
    *  @param   dMinX
    *           Populated with the minimum x-coordinate of the object.
    *  @param   dMinY
    *           Populated with the minimum y-coordinate of the object.
    *  @param   dMaxX
    *           Populated with the maximum x-coordinate of the object.
    *  @param   dMaxY
    *           Populated with the maximum y-coordinate of the object.
    *
    *  @return  TRUE if the coordinate extents were successfully populated, otherwise
    *           FALSE.
    */
   virtual bool getExtents(double& dMinX, double& dMinY, double& dMaxX, double& dMaxY) = 0;

protected:
   /**
    * This should be destroyed by calling PlotView::deleteObject.
    */
   virtual ~PlotObject() {}
};

#endif
