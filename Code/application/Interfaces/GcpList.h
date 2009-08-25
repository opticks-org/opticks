/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GCPLIST_H
#define GCPLIST_H

#include "DataElement.h"
#include "LocationType.h"

#include <list>
#include <string>

/**
 *  A Ground Control Point (GCP)
 *
 *  The GcpPoint struct packages two LocationType objects (the reference and
 *  destination coordinates) with a name and root mean square (RMS) error.
 */
class GcpPoint
{
public:
   LocationType mPixel;       /**< The reference coordinate of the GCP. */
   LocationType mCoordinate;  /**< The destination coordinate of the GCP. */
   LocationType mRmsError;    /**< The error computed during georeferencing. */
};

/**
 *  A list containing a set of GCPs.
 *
 *  The GcpList class stores a list of GcpPoint structs and allows for
 *  editing of the list and saving to disk. It is an abstract class
 *  providing the interface for this capability. The GcpList is used primarily
 *  for georeferencing.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The following methods are called: addPoints(), addPoint(),
 *    removePoints(), removePoint(), clearPoints().
 *  - Everything else documented in DataElement.
 */
class GcpList : public DataElement
{
public:
   /**
    *  Emitted with any<GcpPoint> when a GCP is added to the list.
    */
   SIGNAL_METHOD(GcpList, PointAdded)
   /**
    *  Emitted with any<list<GcpPoint> > when GCPs are added to the list.
    */
   SIGNAL_METHOD(GcpList, PointsAdded)
   /**
    *  Emitted with any<GcpPoint> when a GCP is removed from the list.
    */
   SIGNAL_METHOD(GcpList, PointRemoved)
   /**
    *  Emitted with any<list<GcpPoint> > when GCPs are removed from the list.
    */
   SIGNAL_METHOD(GcpList, PointsRemoved)
   /**
    *  Emitted when the list is cleared.
    */
   SIGNAL_METHOD(GcpList, Cleared)

   /**
    *  Gets the list of GcpPoints the comprise the GcpList.
    *
    *  @return  Returns a reference to the internal list of GcpPoint
    *           structs.
    */
   virtual const std::list<GcpPoint>& getSelectedPoints() const = 0;

   /**
    *  Merges GcpPoints into the GcpList. The added points will be
    *  placed at the end of the list.
    *
    *  @param   points
    *           A list of GcpPoints to add to the GcpList.
    *
    *  @notify  This method will notify signalGcpsAdded with any<list<GcpPoint> >
    */
   virtual void addPoints(const std::list<GcpPoint>& points) = 0;

   /**
    *  Merges a GcpPoint into the GcpList. The added point will be
    *  placed at the end of the list.
    *
    *  @param   point
    *           A GcpPoint to add to the GcpList.
    *
    *  @notify  This method will notify signalGcpAdded with any<GcpPoint>
    */
   virtual void addPoint(const GcpPoint& point) = 0;

   /**
    *  Returns the number of GCPs in the list.
    *
    *  This method is equivalent to getSelectedPoints().size().
    *
    *  @return  The number of GCPs in the list.
    */
   virtual int getCount() const = 0;

   /**
    *  Removes GcpPoints from the GcpList.
    *
    *  %Any GcpPoints in the GcpList that are also in the list provided as 
    *  parameter will be removed from the GcpList.
    *
    *  @param   points
    *           A list of GcpPoints to remove from the GcpList
    *
    *  @notify  This method will notify signalGcpsRemoved with any<list<GcpPoint> >
    */
   virtual void removePoints(const std::list<GcpPoint>& points) = 0;

   /**
    *  Removes a GcpPoint from the GcpList. If the GcpPoint matches a point in
    *  the GcpList, it will be removed from the GcpList.
    *
    *  @param   point
    *           The GcpPoint to remove from the GcpList
    *
    *  @notify  This method will notify signalGcpRemoved with GcpPoint
    */
   virtual void removePoint(const GcpPoint& point) = 0;

   /**
    *  Removes all GcpPoints from the GcpList.
    *
    *  @notify  This method will notify signalCleared
    */
   virtual void clearPoints() = 0;

protected:
   /**
    * This should be destroyed by calling ModelServices::destroyElement.
    */
   virtual ~GcpList() {}
};

#endif
