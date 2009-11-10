/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef POLYGONPLOTOBJECT_H
#define POLYGONPLOTOBJECT_H

#include "ColorType.h"
#include "LocationType.h"
#include "PointSet.h"
#include "TypesFile.h"

/**
 *  A set of points connected by a line.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The following methods are called: setFillColor(), setFillStyle(),
 *    setHatchStyle().
 *  - Everything else documented in PointSet.
 *
 *  @see     PlotObject
 */
class PolygonPlotObject : public PointSet
{
public:   
   /**
    *  Returns the polygon fill color
    *  
    *  @return  The polygon fill color
    */
   virtual ColorType getFillColor() const = 0;
   
   /**
    *  Returns the polygon fill style
    *  
    *  @return  The polygon fill style
    *
    *  @see FillStyle
    */
   virtual FillStyle getFillStyle() const = 0;
   
   /**
    *  Returns the hatch style
    *  
    *  @return  The hatch style
    *
    *  @see SymbolType
    */
   virtual SymbolType getHatchStyle() const = 0;
   
   /**
    *  Returns the fill state of the polygon
    *  
    *  @return  True, is filled, false is not filled
    */
   virtual bool isFilled() const = 0;
   
   /**
    *  Determines if this plot object resides at this point, most likely a mouse click
    *
    *  @param point  The location of the mouse click
    *  @return       True if the plot object is at this point. This check includes the center
    *                if the object is filled, otherwise it only includes the outline.
    */
   virtual bool hit(LocationType point) const = 0;
   
   /**
    *  Sets the polygon fill color
    *  
    *  @param fillColor  The new color
    *
    *  @notify  This method will notify Subject::signalModified.
    */
   virtual void setFillColor(const ColorType& fillColor) = 0;
   
   /**
    *  Sets the polygon fill style
    *  
    *  @param fillStyle  The new fill style
    *
    *  @notify  This method will notify Subject::signalModified.
    */
   virtual void setFillStyle(const FillStyle& fillStyle) = 0;
   
   /**
    *  Sets the polygon hatch style
    *  
    *  @param hatchStyle  The new hatch sytle
    *
    *  @notify  This method will notify Subject::signalModified.
    */
   virtual void setHatchStyle(const SymbolType& hatchStyle) = 0;

protected:
   /**
    * This should be destroyed by calling PlotView::deleteObject.
    */
   virtual ~PolygonPlotObject() {}
};

#endif
