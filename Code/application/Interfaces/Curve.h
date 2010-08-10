/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef CURVE_H
#define CURVE_H

#include "ColorType.h"
#include "LocationType.h"
#include "PlotObject.h"
#include "TypesFile.h"

#include <vector>

/**
 *  A set of points connected by a line.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The following methods are called: setPoints(), setColor(), setLineWidth(),
 *    setLineStyle(). 
 *  - Everything else documented in PlotObject.
 *
 *  @see     PlotObject
 */
class Curve : public PlotObject
{
public:
   /**
    *  Emitted with any<vector<LocationType> > when the points are changed.
    */
   SIGNAL_METHOD(Curve, PointsChanged)

   /**
    *  Sets the location of each point in the curve.
    *
    *  @param   points
    *           The points for the curve.  Any existing points are removed, and the
    *           connecting line connects the points in the order in which they are
    *           contained in the vector.
    *
    *  @return  TRUE if the curve points were successfully set, otherwise FALSE.
    *
    *  @notify  signalPointsChanged with any<vector<LocationType> >
    */
   virtual bool setPoints(const std::vector<LocationType>& points) = 0;

   /**
    *  Returns the curve point locations.
    *
    *  @return  The points for the curve.  The order of the points in the vector
    *           indicate how the connecting line is drawn.
    */
   virtual const std::vector<LocationType>& getPoints() const = 0;

   /**
    *  Sets the curve color.
    *
    *  @param   curveColor
    *           The new curve color.  Must be a valid color.
    *
    *  @see     ColorType::isValid()
    *
    *  @notify  Subject::signalModified
    */
   virtual void setColor(const ColorType& curveColor) = 0;

   /**
    *  Returns the curve color.
    *
    *  @return  The current curve color.
    */
   virtual ColorType getColor() const = 0;

   /**
    *  Sets the curve line width.
    *
    *  @param   iWidth
    *           The new curve line width.  Cannot be zero.
    *
    *  @notify  Subject::signalModified
    */
   virtual void setLineWidth(int iWidth) = 0;

   /**
    *  Returns the curve line width.
    *
    *  @return  The current curve line width.
    */
   virtual int getLineWidth() const = 0;

   /**
    *  Sets the curve line style.
    *
    *  @param   lineStyle
    *           The new curve line style.
    *
    *  @notify  Subject::signalModified
    *
    *  @see     LineStyle
    */
   virtual void setLineStyle(const LineStyle& lineStyle) = 0;

   /**
    *  Returns the curve line style.
    *
    *  @return  The current curve line style.
    */
   virtual LineStyle getLineStyle() const = 0;

protected:
   /**
    * This should be destroyed by calling PlotView::deleteObject.
    */
   virtual ~Curve() {}
};

#endif
