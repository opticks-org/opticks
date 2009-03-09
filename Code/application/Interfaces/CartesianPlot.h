/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CARTESIANPLOT_H
#define CARTESIANPLOT_H

#include "PlotView.h"
#include "TypesFile.h"

class CartesianGridlines;

/**
 *  A two-dimensional Cartesian plot.
 *
 *  A Cartesian plot can display either a linear scale or a log scale on either
 *  the x-axis, the y-axis, or both.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The following methods are called: setXScaleType(), setYScaleType(),
 *    setXDataType(), setYDataType().
 *  - Everything documented in PlotView.
 *
 *  @see     PlotView
 */
class CartesianPlot : public PlotView
{
public:
   /**
    *  Emitted with any<ScaleType> when the X scale type changes.
    */
   SIGNAL_METHOD(CartesianPlot, XScaleTypeChanged);

   /**
    *  Emitted with any<ScaleType> when the Y scale type changes.
    */
   SIGNAL_METHOD(CartesianPlot, YScaleTypeChanged);

   /**
    *  Emitted with any<std::string> when the X data type changes.
    */
   SIGNAL_METHOD(CartesianPlot, XDataTypeChanged);

   /**
    *  Emitted with any<std::string> when the Y data type changes.
    */
   SIGNAL_METHOD(CartesianPlot, YDataTypeChanged);

   /**
    *  @copydoc SessionItem::getContextMenuActions()
    *
    *  @default The default implementation returns the context menu actions
    *           listed \ref cartesianplot "here".  The default actions can be
    *           removed or additional actions can be added by attaching to the
    *           signalAboutToShowContextMenu() signal.
    */
   virtual std::list<ContextMenuAction> getContextMenuActions() const = 0;

   /**
    *  Returns a pointer to the plot's horizontal or vertical gridlines.
    *
    *  @param   orientation
    *           The orientation of the gridlines to get.
    *
    *  @return  A pointer to the plot's horizontal or vertical gridlines
    *           object.
    */
   virtual CartesianGridlines* getGridlines(OrientationType orientation) = 0;

   /**
    *  Returns read-only access to the plot's horizontal or vertical gridlines.
    *
    *  @param   orientation
    *           The orientation of the gridlines to get.
    *
    *  @return  A const pointer to the plot's horizontal or vertical gridlines
    *           object.  The plot object represented by the returned pointer
    *           should not be modified.  To modify the values, call the
    *           non-const version of getGridlines(OrientationType).
    */
   virtual const CartesianGridlines* getGridlines(OrientationType orientation) const = 0;

   /**
    *  Sets the scale type of the x-axis.
    *
    *  @param   scaleType
    *           The new x-axis scale type.
    *
    *  @notify  This method notifies signalXScaleTypeChanged() if the given
    *           scale type is different than the current scale type.
    */
   virtual void setXScaleType(ScaleType scaleType) = 0;

   /**
    *  Sets the scale type of the y-axis.
    *
    *  @param   scaleType
    *           The new y-axis scale type.
    *
    *  @notify  This method notifies signalYScaleTypeChanged() if the given
    *           scale type is different than the current scale type.
    */
   virtual void setYScaleType(ScaleType scaleType) = 0;

   /**
    *  Returns the scale type of the x-axis.
    *
    *  @return  The x-axis scale type.
    */
   virtual ScaleType getXScaleType() const = 0;

   /**
    *  Returns the scale type of the y-axis.
    *
    *  @return  The y-axis scale type.
    */
   virtual ScaleType getYScaleType() const = 0;

   /**
    *  Sets the data type of the x-axis.
    *
    *  The data type is an optional string that can be set to describe the data
    *  that is plotted along the x-axis.  If the plot is contained in a
    *  PlotWidget, the data type is used as the axis text.
    *
    *  @param   dataType
    *           The x-axis data type.  An empty string indicates that no data
    *           type is associated or known.
    *
    *  @notify  This method notifies signalXDataTypeChanged() if the given data
    *           type is different than the current data type.
    */
   virtual void setXDataType(const std::string& dataType) = 0;

   /**
    *  Sets the data type of the y-axis.
    *
    *  The data type is an optional string that can be set to describe the data
    *  that is plotted along the y-axis.  If the plot is contained in a
    *  PlotWidget, the data type is used as the axis text.
    *
    *  @param   dataType
    *           The y-axis data type.  An empty string indicates that no data
    *           type is associated or known.
    *
    *  @notify  This method notifies signalYDataTypeChanged() if the given data
    *           type is different than the current data type.
    */
   virtual void setYDataType(const std::string& dataType) = 0;

   /**
    *  Returns the data type of the x-axis.
    *
    *  @return  The x-axis data type.  An empty string indicates that no data
    *           type has been set.
    *
    *  @see     setXDataType()
    */
   virtual std::string getXDataType() const = 0;

   /**
    *  Returns the data type of the y-axis.
    *
    *  @return  The y-axis data type.  An empty string indicates that no data
    *           type has been set.
    *
    *  @see     setYDataType()
    */
   virtual std::string getYDataType() const = 0;

protected:
   /**
    * This object should be destroyed by calling DesktopServices::deleteView().
    */
   virtual ~CartesianPlot() {}
};

#endif
