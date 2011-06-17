/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef PLOTGROUP_H
#define PLOTGROUP_H

#include "LocationType.h"
#include "PlotObject.h"

#include <vector>

/**
 *  A set of plotobjects.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The following methods are called: insertObject(), removeObject(),
 *    addObject(), insertObjects(). 
 *  - Everything else documented in PlotObject.
 *
 *  @see     PlotObject
 */
class PlotGroup : public PlotObject
{
public:
   /**
    *  Emitted with any<PlotObject*> when an object is added to the group.
    */
   SIGNAL_METHOD(PlotGroup, ObjectAdded)

   /**
    *  Creates a new plot object of type PlotObjectType and adds it 
    *  to the PlotGroup
    *
    *  @param eType  The PlotObjectType
    *  @return  A pointer to the plot object
    *
    *  @notify  This method will notify signalObjectAdded with any(PlotObject*>.
    */
   virtual PlotObject* addObject(const PlotObjectType& eType) = 0;
   
   /**
    *  Inserts a vector of plot objects into the plot group
    *  
    *  @param objects  The vector of plot objects
    *
    *  @notify  This method will notify signalObjectAdded with any(PlotObject*>.
    */
   virtual void insertObjects(const std::vector<PlotObject*>& objects) = 0;
   
   /**
    *  Checks to see if the provide plot object is in this plot group
    *  
    *  @param pObject  The plot object to look for
    *
    *  @return  True if the plot object is in this plot group, false otherwise.
    */
   virtual bool hasObject(PlotObject* pObject) const = 0;
   
   /**
    *  Retrieves a vector of plot objects from the plot group
    *  
    *  @return  A vector of plot objects
    */
   virtual const std::vector<PlotObject*>& getObjects() const = 0;
   
   /**
    *  Returns the number of plot objects in the plot group
    *  
    *  @return  The number of plot objects
    */
   virtual unsigned int getNumObjects() const = 0;
   
   /**
    *  Determines if a plot object resides at this point, most likely a mouse click
    *
    *  @param point  The location of the mouse click
    *
    *  @return       The object that was hit, NULL otherwise.
    */
   virtual PlotObject* hitObject(LocationType point) const = 0;
   
   /**
    *  Determines if a plot object within this plot group resides at this point, most likely a mouse click
    *
    *  @param point  The location of the mouse click
    *
    *  @return       True if a plot object is at this point
    */
   virtual bool hit(LocationType point) = 0;
   
   /**
    *  Sets the plot groups visibility
    *  
    *  @param bVisible  True - visible, False - not visible
    */
   virtual void setVisible(bool bVisible) = 0;
   
   /**
    *  Sets the plot groups selected property
    *  
    *  @param bSelect  True - selected, False - not selected
    */
   virtual void setSelected(bool bSelect) = 0;
   
   /**
    *  Inserts an existing plot object into the plot group
    *  
    *  @param pObject  Pointer to a plot object
    *
    *  @return  True if successfully inserted, false otherwise
    *
    *  @notify  This method will notify signalObjectAdded with any(PlotObject*>.
    */
   virtual bool insertObject(PlotObject* pObject) = 0;
   
   /**
    *  Removes a plot object from the plot group
    *  
    *  @param pObject  The plot object to remove
    *  @param bDelete  True - deletes the object upon removal, False - just removes the plot object
    *  @return  True if the removal was successfule, false otherwise.
    *
    *  @notify  This method will notify Subject::signalModified.
    */
   virtual bool removeObject(PlotObject* pObject, bool bDelete = false) = 0;
   
   /**
    *  Clears the plot groups plot objects
    *  
    *  @param bDelete  True - deletes all the plot objects, false - Just removes the plot objects
    */
   virtual void clear(bool bDelete = false) = 0;

protected:
   /**
    * This should be destroyed by calling PlotView::deleteObject.
    */
   virtual ~PlotGroup() {}
};

#endif
