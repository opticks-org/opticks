/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef POINTSET_H
#define POINTSET_H

#include "ColorType.h"
#include "LocationType.h"
#include "PlotObject.h"
#include "TypesFile.h"

class Point;

#include <vector>

/**
 *  A set of points.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The following methods are called: insertPoint(), setPoints(),
 *    removePoint(), setLineColor(), setLineWidth(), setLineStyle().
 *  - Everything else documented in PlotObject.
 *
 *  @see     PlotObject
 */
class PointSet : public PlotObject
{
public:
   /**
    *  Emitted with any<Point*> when a point is added to the %PointSet.
    */
   SIGNAL_METHOD(PointSet, PointAdded)
   /**
    *  Emitted with any<vector<Point*> > when the PointSet's points are replaced.
    */
   SIGNAL_METHOD(PointSet, PointsSet)

   /**
    *  Creates an empty Point plot object and adds it to the pointset
    *  
    *  @return  The newly created Point
    *
    *  @notify  This method will notify signalPointAdded with any<Point*>.
    */
   virtual Point* addPoint() = 0;
   
   /**
    *  Creates a Point plot object and adds it to the pointset
    *  
    *  @param dX  The points x value
    *  @param dY  The points y value
    *  @return  The newly created point
    *
    *  @notify  This method will notify signalPointAdded with any<Point*>.
    */
   virtual Point* addPoint(double dX, double dY) = 0;
   
   /**
    *  Inserts an existing Point plot object into the pointset
    *  
    *  @param pPoint  The point to insert
    *  @return  True if successfully inserted, false otherwise
    *
    *  @notify  This method will notify signalPointAdded with any<Point*>.
    */
   virtual bool insertPoint(Point* pPoint) = 0;
   
   /**
    *  Inserts a vector of existing Point plot objects into the pointset, replacing
    *  any existing points.  The existing points are removed but not destroyed.
    *  
    *  @param points  The new list of points
    *
    *  @notify  This method will notify signalPointsSet with any<vector<Point*> >.
    */
   virtual void setPoints(const std::vector<Point*>& points) = 0;
   
   /**
    *  Returns the list of points in the pointset
    *  
    *  @return  The list of points
    */
   virtual std::vector<Point*> getPoints() const = 0;
   
   /**
    *  Returns the number of points in the pointset
    *  
    *  @return  The number of points in the pointset
    */
   virtual unsigned int getNumPoints() const = 0;
   
   /**
    *  Checks the pointsets list of points for the point object.
    *  
    *  @param pPoint  The point to look for
    *  @return  True if the pointset contains this point, false otherwise.
    */
   virtual bool hasPoint(Point* pPoint) const = 0;
   
   /**
    *  Removes the point from the pointset
    *  
    *  @param pPoint  The point to remove
    *  @param bDelete  True - deletes the point upon removal, False - just removes the point
    *  @return  True if the point was removed, false otherwise
    *
    *  @notify  This method will notify Subject::signalModified.
    */
   virtual bool removePoint(Point* pPoint, bool bDelete) = 0;
   
   /**
    *  Clears the list of points from the pointset
    *  
    *  @param bDelete  True - deletes the points upon removal, False - just removes the points
    *
    *  @notify  This method will notify signalPointsSet with any<vector<Point*> > with the vector empty.
    */
   virtual void clear(bool bDelete) = 0;

   // Symbols

   /**
    *  Checks to see if the symbols are displayed
    *  
    *  @return  True if the symbols are displayed, false otherwise
    */
   virtual bool areSymbolsDisplayed() const = 0;

   /**
    *  Sets the symbol display attribute
    *  
    *  @param bDisplay  True displays the symbols, false turns the display off
    */
   virtual void displaySymbols(bool bDisplay) = 0;

   // Line

   /**
    *  Returns the state of the line display property
    *  
    *  @return  True if lines are displayed, false if they are not
    */
   virtual bool isLineDisplayed() const = 0;

   /**
    *  Returns the line color
    *  
    *  @return  The line color
    */
   virtual ColorType getLineColor() const = 0;

   /**
    *  Returns the line width
    *  
    *  @return  The line width
    */
   virtual int getLineWidth() const = 0;

   /**
    *  Returns the line style
    *  
    *  @return  The line style
    *
    *  @see LineStyle
    */
   virtual LineStyle getLineStyle() const = 0;

   /**
    *  Sets the line display property
    *  
    *  @param bDisplay  True - display lines, False - do not display lines
    */
   virtual void displayLine(bool bDisplay) = 0;

   /**
    *  Determines if this plot object resides at this point, most likely a mouse click
    *
    *  @param point  The location of the mouse click
    *  @return       The Point object if the plot object is at this point
    */
   virtual Point* hitPoint(LocationType point) const = 0;

   /**
    *  Determines if this plot object resides at this point, most likely a mouse click
    *
    *  @param point  The location of the mouse click
    *  @return       True if the plot object is at this point
    */
   virtual bool hit(LocationType point) const = 0;

   /**
    *  Returns the extents of the pointset.  (Its bounds)
    *  
    *  @param dMinX  The minimum x value
    *  @param dMinY  The minimum y value
    *  @param dMaxX  The maximum x value
    *  @param dMaxY  The maximum y value
    *  @return 
    */
   virtual bool getExtents(double& dMinX, double& dMinY, double& dMaxX, double& dMaxY) = 0;

   /**
    *  Sets the line color
    *  
    *  @param clrLine  The new line color
    *
    *  @notify  This method will notify Subject::signalModified.
    */
   virtual void setLineColor(const ColorType& clrLine) = 0;

   /**
    *  Sets the line width
    *  
    *  @param iWidth  The new line width
    *
    *  @notify  This method will notify Subject::signalModified.
    */
   virtual void setLineWidth(int iWidth) = 0;

   /**
    *  Sets the line style
    *  
    *  @param eStyle  The new line style
    *
    *  @see LineStyle
    *
    *  @notify  This method will notify Subject::signalModified.
    */
   virtual void setLineStyle(const LineStyle& eStyle) = 0;

   /**
    * Set the current interactivity state.
    *
    * Setting this to false will cause the PointSet to delay
    * processing until set to interactive again.  This is useful
    * when making a large number of changes to the PointSet.
    *
    * @param interactive
    *        The new interactive state.
    *
    * @see setInteractive
    */
   virtual void setInteractive(bool interactive) = 0;

   /**
    * Get the current interactivity state.
    *
    * @return The current interactivity state.
    *
    * @see getInteractive
    */
   virtual bool getInteractive() = 0;

protected:
   /**
    * This should be destroyed by calling PlotView::deleteObject.
    */
   virtual ~PointSet() {}
};

#endif
