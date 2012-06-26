/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef ARROW_H
#define ARROW_H

#include "ColorType.h"
#include "LocationType.h"
#include "PlotObject.h"
#include "TypesFile.h"

/**
 *  A set of points connected by a line forming an arrow.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The following methods are called: setArrowStyle(), setBaseLocation(),
 *    setTipLocation(), setLocation().
 *  - Everything else documented in PlotObject.
 *
 *  @see     PlotObject
 */
class Arrow : public PlotObject
{
public:
   /**
    *  Returns the arrow style
    *  
    *  @return  Returns the arrow style as an enumerated type
    */
   virtual ArrowStyle getArrowStyle() const = 0;
   
   /**
    *  Returns the location of the beginning of the arrow
    *
    *  @return  LocationType
    */
   virtual LocationType getBaseLocation() const = 0;
   
   /**
    *  Returns the location of the end of the arrow
    *
    *  @return  LocationType
    */
   virtual LocationType getTipLocation() const = 0;
   
   /**
    *  Returns the color of the arrow
    *
    *  @return  A color object
    */
   virtual ColorType getColor() const = 0;
   
   /**
    *  Determines if this plot object resides at this point, most likely a mouse click
    *
    *  @param point  The location of the mouse click
    *  @return       True if the plot object is at this point
    */
   virtual bool hit(LocationType point) const = 0;
   
   /**
    *  Sets the objects arrow style
    *  
    *  @param eStyle  The arrow style
    *
    *  @notify  This method will notify Subject::signalModified.
    */
   virtual void setArrowStyle(const ArrowStyle& eStyle) = 0;
   
   /**
    *  Sets the location of the arrow, both beginning and ending
    *
    *  @param baseLocation  The beginning point
    *  @param tipLocation   The ending point
    *
    *  @notify  This method will notify Subject::signalModified.
    */
   virtual void setLocation(const LocationType& baseLocation, const LocationType& tipLocation) = 0;
   
   /**
    *  Sets the beginning location of the arrow
    *
    *  @param baseLocation 
    *
    *  @notify  This method will notify Subject::signalModified.
    */
   virtual void setBaseLocation(const LocationType& baseLocation) = 0;
   
   /**
    *  Sets the ending location of the arrow
    *
    *  @param tipLocation 
    *
    *  @notify  This method will notify Subject::signalModified.
    */
   virtual void setTipLocation(const LocationType& tipLocation) = 0;
   
   /**
    *  Sets the color of the arrow
    *
    *  @param newColor 
    */
   virtual void setColor(const ColorType& newColor) = 0;

protected:
   /**
    * This should be destroyed by calling PlotView::deleteObject.
    */
   virtual ~Arrow() {}
};

#endif
