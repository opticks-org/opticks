/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef AXIS_H
#define AXIS_H

#include "ColorType.h"
#include "TypesFile.h"

#include <string>
#include <vector>

class Font;
class QWidget;

/**
 *  Displays plot values along a scale.
 *
 *  The Axis widget calculates values according to a scale type and displays
 *  values along the scale with tick marks.  In the PlotWidget, axis widgets
 *  are positioned around a PlotView to show the currently displayed area of
 *  the plot.
 *
 *  This interface provides access to associate a title with the axis and to
 *  change its text properties and to define the scale that is used to
 *  calculate the axis values.
 *
 *  @see     PlotWidget
 */
class Axis
{
public:
   /**
    *  Returns the Qt widget that displays the axis.
    *
    *  This method returns the Qt widget that displays the axis.  This can be
    *  used to add the axis to a custom dialog or widget if it is not already
    *  contained in a PlotWidget.
    *
    *  @return  A pointer to the Qt widget that displays the axis.
    */
   virtual QWidget* getWidget() = 0;

   /**
    *  Returns the axis position.
    *
    *  The axis poisition is the position where the axis should be displayed
    *  relative to a plot view.  It defines how the tick marks are drawn and
    *  how the text is oriented.
    *
    *  @return  The axis position.
    */
   virtual AxisPosition getPosition() const = 0;

   /**
    *  Sets the title text.
    *
    *  @param   title
    *           The new title text.
    */
   virtual void setTitle(const std::string& title) = 0;

   /**
    *  Returns the title text.
    *
    *  @return  The current title.  An empty string is returned if a title has
    *           not yet been set.
    */
   virtual std::string getTitle() const = 0;

   /**
    *  Sets the font for the title text.
    *
    *  @param   font
    *           The new title font.
    */
   virtual void setTitleFont(const Font& font) = 0;

   /**
    *  Returns read-only access to the title text font.
    *
    *  @return  The current title text font.  To modify the font values, call
    *           setTitleFont() instead.
    */
   virtual const Font& getTitleFont() const = 0;

   /**
    *  Sets the title text color.
    *
    *  @param   titleColor
    *           The new title text color.
    */
   virtual void setTitleColor(const ColorType& titleColor) = 0;

   /**
    *  Returns the title color.
    *
    *  @return  The current title text color.  A valid color is returned even
    *           if the title is empty.
    */
   virtual ColorType getTitleColor() const = 0;

   /**
    *  Sets the axis scale type.
    *
    *  @param   scaleType
    *           The new scale type.
    */
   virtual void setScaleType(ScaleType scaleType) = 0;

   /**
    *  Returns the axis scale type.
    *
    *  @return  The scale type.
    */
   virtual ScaleType getScaleType() const = 0;

   /**
    *  Sets the minimum and maximum values.
    *
    *  @param   dMin
    *           The new minimum value.
    *  @param   dMax
    *           The new maximum value.
    */
   virtual void setValueRange(double dMin, double dMax) = 0;

   /**
    *  Returns the minimum axis value.
    *
    *  @return  The minimum value of the axis.
    */
   virtual double getMinimumValue() const = 0;

   /**
    *  Returns the maximum axis value.
    *
    *  @return  The maximum value of the axis.
    */
   virtual double getMaximumValue() const = 0;

   /**
    *  Returns the values of all major tick mark locations.
    *
    *  @return  The major tick mark values.  The values are guaranteed to be
    *           between the minimum and maximum value inclusive.
    *
    *  @see     getMinorTickLocations(), getMinimumValue(), getMaximumValue()
    */
   virtual std::vector<double> getMajorTickLocations() const = 0;

   /**
    *  Returns the values of all minor tick mark locations.
    *
    *  @return  The minor tick mark values.  The values are guaranteed to be
    *           between the minimum and maximum value inclusive.
    *
    *  @see     getMajorTickLocations(), getMinimumValue(), getMaximumValue()
    */
   virtual std::vector<double> getMinorTickLocations() const = 0;

   /**
    *  Sets the maximum number of major tick marks along the axis scale.
    *
    *  This method sets the maximum number of major tick marks to display along
    *  the axis scale.  Calling this method recalculates the scale based on the
    *  current minimum and maximum values.
    *
    *  @param   numTicks
    *           The new maximum number of major tick marks.
    *
    *  @see     setMaxNumMinorTicks()
    */
   virtual void setMaxNumMajorTicks(int numTicks) = 0;

   /**
    *  Sets the maximum number of minor tick marks along the axis scale.
    *
    *  This method sets the maximum number of minor tick marks to display
    *  between each major tick mark along the axis scale.  Calling this method
    *  recalculates the scale based on the current minimum and maximum values.
    *
    *  @param   numTicks
    *           The new maximum number of minor tick marks.
    *
    *  @see     setMaxNumMajorTicks()
    */
   virtual void setMaxNumMinorTicks(int numTicks) = 0;

   /**
    *  Returns the maximum number of major tick marks along the axis scale.
    *
    *  This method returns the maximum number of major tick marks that are
    *  displayed between the minimum and maximum value.  The default value is
    *  ten.
    *
    *  @return  The maximum value of major tick marks.
    */
   virtual int getMaxNumMajorTicks() const = 0;

   /**
    *  Returns the maximum number of minor tick marks along the axis scale.
    *
    *  This method returns the maximum number of minor tick marks that are
    *  displayed between each major tick mark along the axis scale.  The
    *  default value is four.
    *
    *  @return  The maximum value of minor tick marks.
    */
   virtual int getMaxNumMinorTicks() const = 0;

   /**
    *  Sets the format for the value text labels.
    *
    *  This method sets the display format for the value text labels displayed
    *  at the major tick mark locations.  This method can be called to set a
    *  desired precision or to enforce or prevent scientific notation on the
    *  value labels.
    *
    *  @param   labelFormat
    *           The new text format for the values, whose syntax is identical
    *           to the format string provided to sprintf() in the standard C++
    *           library.
    */
   virtual void setLabelFormat(const std::string& labelFormat) = 0;

   /**
    *  Returns the format for the value text labels.
    *
    *  This method returns the display format for the value text labels
    *  displayed at the major tick mark locations.
    *
    *  @return  The text format for the values, whose syntax is identical to
    *           the format string provided to sprintf() in the standard C++
    *           library.
    */
   virtual std::string getLabelFormat() const = 0;

protected:
   /**
    *  This object should be destroyed by calling DesktopServices::deleteAxis().
    */
   virtual ~Axis() {}
};

#endif
