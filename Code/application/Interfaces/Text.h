/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef TEXT_H
#define TEXT_H

#include "ColorType.h"
#include "LocationType.h"
#include "PlotObject.h"

/**
 *  A text object that can be displayed in a plot
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The following methods are called: setLocation(), setText(), setColor().
 *  - Everything else documented in PlotObject.
 *
 *  @see     Text
 */
class Text : public PlotObject
{
public:
   /**
    *  Emitted with any<LocationType> when the text is moved.
    */
   SIGNAL_METHOD(Text, LocationChanged)
   /**
    *  Emitted with any<ColorType> when the color is changed.
    */
   SIGNAL_METHOD(Text, ColorChanged)
   /**
    *  Emitted with any<std::string> when the text is changed.
    */
   SIGNAL_METHOD(Text, TextChanged)

   /**
    *  Returns the texts x location
    *  
    *  @return  The x location
    */
   virtual double getXLocation() const = 0;
   
   /**
    *  Returns the texts y location
    *  
    *  @return  The y location
    */
   virtual double getYLocation() const = 0;
   
   /**
    *  Returns the text location
    *  
    *  @return  The text location
    *
    *  @see LocationType
    */
   virtual const LocationType& getLocation() const = 0;
   
   /**
    *  Returns the text
    *  
    *  @return  The text
    */
   virtual std::string getText() const = 0;
   
   /**
    *  Returns the text color
    *  
    *  @return  The text color
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
    *  Sets the text location
    *  
    *  @param location  The new location
    *
    *  @see LocationType
    *
    *  @notify  This method will notify signalLocationChanged with any<LocationType>.
    */
   virtual void setLocation(const LocationType& location) = 0;
   
   /**
    *  Sets the text location
    *  
    *  @param dX  The x location
    *  @param dY  The y location
    *
    *  @notify  This method will notify signalLocationChanged with any<LocationType>.
    */
   virtual void setLocation(double dX, double dY) = 0;
   
   /**
    *  Sets the text
    *  
    *  @param strText  The new text
    *
    *  @notify  This method will notify signalTextChanged with any<std::string>.
    */
   virtual void setText(const std::string& strText) = 0;
   
   /**
    *  Sets the text color
    *  
    *  @param clrText  The new color
    *
    *  @notify  This method will notify signalColorChanged with any<ColorType>.
    */
   virtual void setColor(const ColorType& clrText) = 0;

protected:
   /**
    * This should be destroyed by calling PlotView::deleteObject.
    */
   virtual ~Text() {}
};

#endif
