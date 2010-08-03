/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef AOIELEMENT_H
#define AOIELEMENT_H

#include "GraphicElement.h"
#include "LocationType.h"

#include <vector>

class BitMask;
class GraphicObject;

/**
 * Storage for a collection of pixel locations.
 *
 * An area of interest (AOI) is a concept used to select individual 
 * pixels within a two-dimensional raster area to limit or control
 * processing.  The AOI class contains a number of vector objects
 * which are added together to identify whether each pixel in a scene
 * is included in the collection.
 *
 * This subclass of Subject will notify upon the following conditions:
 *  * The following methods are called: addPoint(), addPoints(), removePoint(),
 *      removePoints(), togglePoint(), togglePoints(), toggleAllPoints(), clearPoints().
 *  * Everything else documented in GraphicElement.
 */
class AoiElement : public GraphicElement
{
public:
   /**
    * Clear all points from the AOI.
    *
    * Calling this method will delete all objects
    * within the AOI.
    *
    *  @notify  This method will notify Subject::signalModified.
    */
   virtual void clearPoints() = 0;

   /**
    * Get the count of pixels selected by the AOI.
    *
    * @return The number of points which are selected.
    */
   virtual size_t getPixelCount() const = 0;

   /**
    * Get all points selected by the AOI.
    *
    * @return A BitMask indicating which points are selected.
    */
   virtual const BitMask *getSelectedPoints() const = 0;

   /**
    * Toggle all points.  This causes an inversion of the pixels selected.
    *
    *  @notify  This method will notify Subject::signalModified.
    */
   virtual void toggleAllPoints() = 0;

   /**
    * Determine whether all points are toggled.
    *
    * @return True if all points have been toggled, false otherwise.
    */
   virtual bool getAllPointsToggled() const = 0;

   /**
    *  Adds a list of points to the collection of those currently marked
    *  as selected.
    *
    *  @param   points
    *           A vector of points to be marked as selected in the AOI.
    *
    *  @notify  This method will notify Subject::signalModified.
    */
   virtual GraphicObject *addPoints(const std::vector<LocationType>& points) = 0;

   /**
    *  Updates the AOI to include any additional points marked as selected
    *  from the incoming BitMask argument.
    *
    *  @param   pPoints
    *           A BitMask containing additional points to mark as selected.
    *           The AoiElement does not take ownership over the mask.
    *
    *  @notify  This method will notify Subject::signalModified.
    */
   virtual GraphicObject *addPoints(const BitMask* pPoints) = 0;

   /**
    *  Adds a point to the collection of those currently marked
    *  as selected.
    *
    *  @param   point
    *           A point to mark as selected in the AOI.
    *
    *  @notify  This method will notify Subject::signalModified.
    */
   virtual GraphicObject *addPoint(LocationType point) = 0;

   /**
    *  Removes a list of points from the collection of those currently marked
    *  as selected.
    *
    *  @param   points
    *           A vector of points to be marked as unselected in the AOI.
    *
    *  @notify  This method will notify Subject::signalModified.
    */
   virtual GraphicObject *removePoints(const std::vector<LocationType>& points) = 0;

   /**
    *  Updates the AOI to remove any additional points marked as selected
    *  from the incoming BitMask argument.
    *
    *  @param   pPoints
    *           A BitMask containing additional points to mark as unselected.
    *           The AoiElement does not take ownership over the mask.
    *
    *  @notify  This method will notify Subject::signalModified.
    */
   virtual GraphicObject *removePoints(const BitMask* pPoints) = 0;

   /**
    *  Removes a point from the collection of those currently marked
    *  as selected.
    *
    *  @param   point
    *           A point to mark as unselected in the AOI.
    *
    *  @notify  This method will notify Subject::signalModified.
    */
   virtual GraphicObject *removePoint(LocationType point) = 0;

   /**
    *  Toggles a set of points in the collection.  %Any point currently selected is
    *  changed to unselected. If it is currently unselected, then it changes
    *  to selected.
    *
    *  @param   points
    *           A vector of points to toggle the selected status in the AOI.
    *
    *  @notify  This method will notify Subject::signalModified.
    */
   virtual GraphicObject *togglePoints(const std::vector<LocationType>& points) = 0;

   /**
    *  Toggles a set of points in the collection.  %Any point currently selected is
    *  changed to unselected. If it is currently unselected, then it changes
    *  to selected.
    *
    *  @param   pPoints
    *           A BitMask containing selected pixels indicating those that should
    *           be toggled.  The AoiElement does not take ownership over the mask.
    *
    *  @notify  This method will notify Subject::signalModified.
    */
   virtual GraphicObject *togglePoints(const BitMask* pPoints) = 0;

   /**
    *  Toggles a point from in the collection.  If the point is currently selected,
    *  change it to unselected. If it is currently unselected, then change it
    *  to selected.
    *
    *  @param   point
    *           A point to toggle the selected status in the AOI.
    *
    *  @notify  This method will notify Subject::signalModified.
    */
   virtual GraphicObject *togglePoint(LocationType point) = 0;


protected:
   /**
    * This should be destroyed by calling ModelServices::destroyElement.
    */
   virtual ~AoiElement() {}
};

#endif
