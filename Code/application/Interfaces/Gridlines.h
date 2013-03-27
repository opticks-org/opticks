/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GRIDLINES_H
#define GRIDLINES_H

#include "PlotObject.h"
#include "ColorType.h"
#include "TypesFile.h"

/**
 *  Provides a grid from which all other plot objects can be referenced.
 *
 *  The gridlines object is provided automatically by a plot view and is
 *  created as a secondary plot object.  As a secondary object, the gridlines
 *  object does not have extents, so the getExtents() method will return
 *  \b false.
 *
 *  A gridlines object consists of major and minor gridlines.  The major
 *  gridlines are always displayed, and the display of the minor gridlines can
 *  be toggled on and off with enableMinorGridlines().
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The following methods are called: enableMinorGridlines(), setColor(),
 *    setLineWidth(), setLineStyle(), setMaxNumMajorLines(), and
 *    setMaxNumMinorLines().
 *  - Everything else documented in PlotObject.
 *
 *  @see        PlotObject
 */
class Gridlines : public PlotObject
{
public:
   /**
    *  Toggles the display of the minor gridlines.
    *
    *  @param   bEnable
    *           Set this value to \b true to display the minor gridlines, or to
    *           \b false to hide the minor gridlines.
    *
    *  @notify  This method notifies Subject::signalModified() since the number
    *           of gridlines displayed changes.
    */
   virtual void enableMinorGridlines(bool bEnable) = 0;

   /**
    *  Queries whether minor gridlines are displayed.
    *
    *  @return  Returns \b true if the minor gridlines are displayed, or
    *           \b false if the minor gridlines are hidden.
    */
   virtual bool areMinorGridlinesEnabled() const = 0;

   /**
    *  Sets the color of the gridlines.
    *
    *  @param   lineColor
    *           The gridline color.
    *
    *  @notify  This method notifies Subject::signalModified() if the given
    *           color is different than the current color, and the given color
    *           is valid.
    *
    *  @see     ColorType::isValid()
    */
   virtual void setColor(const ColorType& lineColor) = 0;

   /**
    *  Returns the gridline color.
    *
    *  @return  The gridline color.
    */
   virtual ColorType getColor() const = 0;

   /**
    *  Sets the line width of the gridlines.
    *
    *  @param   lineWidth
    *           The gridline width.
    *
    *  @notify  This method notifies Subject::signalModified() if the given
    *           line width is different than the current line width, and the
    *           given line width is greater than zero.
    */
   virtual void setLineWidth(int lineWidth) = 0;

   /**
    *  Returns the gridline width.
    *
    *  @return  The gridline width.
    */
   virtual int getLineWidth() const = 0;

   /**
    *  Sets the line style of the gridlines.
    *
    *  @param   lineStyle
    *           The gridline style.
    *
    *  @notify  This method notifies Subject::signalModified() if the given
    *           line style is different than the current line style.
    */
   virtual void setLineStyle(LineStyle lineStyle) = 0;

   /**
    *  Returns the gridline style.
    *
    *  @return  The gridline style.
    */
   virtual LineStyle getLineStyle() const = 0;

   /**
    *  Sets the maximum number of major gridlines.
    *
    *  This method sets the maximum number of major gridlines to display in the
    *  plot view.  Calling this method recalculates the overall locations of
    *  the gridlines.
    *
    *  @param   numLines
    *           The new maximum number of major gridlines.
    *
    *  @notify  This method notifies Subject::signalModified() if the given
    *           maximum number of major gridlines is different than the current
    *           maximum number.
    *
    *  @see     setMaxNumMinorLines()
    */
   virtual void setMaxNumMajorLines(int numLines) = 0;

   /**
    *  Sets the maximum number of minor gridlines.
    *
    *  This method sets the maximum number of minor gridlines to display
    *  between each major gridline in the plot view.  Calling this method
    *  recalculates the overall locations of the gridlines.
    *
    *  @param   numLines
    *           The new maximum number of minor gridlines.
    *
    *  @notify  This method notifies Subject::signalModified() if the given
    *           maximum number of minor gridlines is different than the current
    *           maximum number.
    *
    *  @see     setMaxNumMajorLines()
    */
   virtual void setMaxNumMinorLines(int numLines) = 0;

   /**
    *  Returns the maximum number of major gridlines.
    *
    *  This method returns the maximum number of major gridlines that are
    *  displayed in the plot view.  The default value is ten.
    *
    *  @return  The maximum value of major gridlines.
    */
   virtual int getMaxNumMajorLines() const = 0;

   /**
    *  Returns the maximum number of minor gridlines.
    *
    *  This method returns the maximum number of minor gridlines that are
    *  displayed between each major gridline in the plot view.  The default
    *  value is four.
    *
    *  @return  The maximum value of minor gridlines.
    */
   virtual int getMaxNumMinorLines() const = 0;

protected:
   /**
    *  A plug-in cannot create this object, it can only retrieve an already
    *  existing object from a plot view.  The plot view manages any instances
    *  of this object.
    */
   virtual ~Gridlines() {}
};

#endif
