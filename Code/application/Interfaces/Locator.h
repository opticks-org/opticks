/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef LOCATOR_H
#define LOCATOR_H

#include "ColorType.h"
#include "LocationType.h"
#include "EnumWrapper.h"
#include "PlotObject.h"
#include "TypesFile.h"

#include <string>

/**
 *  Marks a specific location on a plot.
 *
 *  The locator object can be used to pinpoint a particular location on a plot.  A combination
 *  of a horizontal line and vertical line are used to mark the location.  A single horizontal
 *  line or a single vertical line can also be drawn to mark a particular value along a single
 *  plot axis.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The following methods are called: setLocation(), setStyle(), setColor(),
 *    setLineWidth(), setLineStyle().
 *  - Everything else documented in PlotObject.
 *
 *  @see     PlotObject, Locator::LocatorStyle
 */
class Locator : public PlotObject
{
public:
   /**
    *  Specifies how the locator is drawn.
    */
   enum LocatorStyleEnum
   {
      HORIZONTAL_LOCATOR = 0x0001,                                /**< A single horizontal line is drawn along
                                                                       the x-axis */
      VERTICAL_LOCATOR = 0x0002,                                  /**< A single vertical line is drawn along the
                                                                       y-axis */
      CROSSHAIR_LOCATOR = HORIZONTAL_LOCATOR | VERTICAL_LOCATOR   /**< A horizonal line and vertical line are
                                                                       drawn, intersecting at the locator location*/
   };

   /**
    * @EnumWrapper Locator::LocatorStyleEnum.
    */
   typedef EnumWrapper<LocatorStyleEnum> LocatorStyle;

   /**
    *  Emitted with any<LocationType> when the locator position changes.
    */
   SIGNAL_METHOD(Locator, LocationChanged)
   /**
    *  Emitted with any<LocatorStyle> when the locator style changes.
    */
   SIGNAL_METHOD(Locator, StyleChanged)
   /**
    *  Emitted with any<std::pair<std::string,std::string> > when the locator 
    *  text changes, containing the new xText and yText as first and second
    *  respectively.
    */
   SIGNAL_METHOD(Locator, TextChanged)

   /**
    *  Sets the locator location.
    *
    *  @param   location
    *           The new locator location.
    *
    *  @param   updateText
    *           If true, the locator will update its text strings to reflect 
    *           the position of the locator. If false, it will leave them
    *           unchanged.
    *
    *  @notify  This method will notify signalLocationChanged() with
    *           any<LocationType>.
    */
   virtual void setLocation(const LocationType& location, bool updateText = true) = 0;

   /**
    *  Returns the locator location.
    *
    *  @return  The current locator location.
    */
   virtual LocationType getLocation() const = 0;

   /**
    *  Sets the locator text.
    *
    *  @param   xText
    *           The new x text string.
    *
    *  @param   yText
    *           The new y text string.
    *
    *  @notify  This method will notify signalTextChanged() with
    *           any<std::pair<std::string,std::string> > containing the new
    *           xText and yText as first and second respectively.
    */
   virtual void setText(const std::string& xText, const std::string& yText) = 0;

   /**
    *  Retrieves the locator text strings.
    *
    *  @param   xText
    *           The x text string.
    *
    *  @param   yText
    *           The y text string.
    */
   virtual void getText(std::string& xText, std::string& yText) const = 0;

   /**
    *  Sets the locator style.
    *
    *  @param   style
    *           The new locator style.
    *
    *  @see     Locator::LocatorStyle
    *
    *  @notify  This method will notify signalStyleChanged() with
    *           any<LocatorStyle>.
    */
   virtual void setStyle(LocatorStyle style) = 0;

   /**
    *  Returns the locator style.
    *
    *  @return  The current locator style.
    */
   virtual LocatorStyle getStyle() const = 0;

   /**
    *  Sets the locator color.
    *
    *  @param   locatorColor
    *           The new locator color.  Must be a valid color.
    *
    *  @see     ColorType::isValid()
    *
    *  @notify  This method will notify with Subject::signalModified().
    */
   virtual void setColor(const ColorType& locatorColor) = 0;

   /**
    *  Returns the locator color.
    *
    *  @return  The current locator color.
    */
   virtual ColorType getColor() const = 0;

   /**
    *  Sets the locator line width.
    *
    *  @param   iWidth
    *           The new locator line width.  Cannot be zero.
    *
    *  @notify  This method will notify with Subject::signalModified().
    */
   virtual void setLineWidth(int iWidth) = 0;

   /**
    *  Returns the locator line width.
    *
    *  @return  The current locator line width.
    */
   virtual int getLineWidth() const = 0;

   /**
    *  Sets the locator line style.
    *
    *  @param   lineStyle
    *           The new locator line style.
    *
    *  @see     LineStyle
    *
    *  @notify  This method will notify with Subject::signalModified().
    */
   virtual void setLineStyle(LineStyle lineStyle) = 0;

   /**
    *  Returns the locator line style.
    *
    *  @return  The current locator line style.
    */
   virtual LineStyle getLineStyle() const = 0;

protected:
   /**
    * This should be destroyed by calling PlotView::deleteObject.
    */
   virtual ~Locator() {}
};

#endif
