/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef HISTOGRAMWINDOW_H
#define HISTOGRAMWINDOW_H

#include "PlotWindow.h"
#include "ConfigurationSettings.h"
#include "TypesFile.h"

class Layer;
class PlotWidget;

/**
 *  A specialized plot window to display histogram plots.
 *
 *  The histogram window manages multiple histogram plots on potentially
 *  multple plot sets.  A plot set is added for each spatial data view that
 *  contains layers.  %Histogram plot widgets are automatically added to the
 *  plot set when the layer is created.  For automatically created plots, the
 *  name of the plot set is the same name as the view, and the name of the plot
 *  is the same name as the layer.  Additional plot widgets can be added to any
 *  plot set in the window, using the basic plot window functionality as
 *  desired.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - Everything documented in PlotWindow.
 *
 *  @see        HistogramPlot, PlotWindow, PlotWidget, PlotSet, Layer
 */
class HistogramWindow : public PlotWindow
{
public:
   SETTING(LayerActivation, HistogramWindow, bool, false)

   /**
    *  Creates a histogram plot widget for a given layer.
    *
    *  This method creates a new histogram plot widget to display the data in
    *  the given layer and adds it to a plot set with the same name as the view
    *  in which the layer is displayed.
    *
    *  Only layers that support histogram data can be used. Current valid layer
    *  types are as follows:
    *  - RasterLayer
    *  - ThresholdLayer
    *
    *  If the layer is a raster layer or cube layer, multiple plot widgets are
    *  added, one for each of the gray, red, green, and blue color components.
    *
    *  @param   pLayer
    *           The layer for which to create a new histogram plot widget.
    *           Only layers that support histogram data can be used.
    *  @param   pPlotSet
    *           The plot set on which to create the new histogram plot widget.
    *           Passing in \b NULL will add the plot to the plot set
    *           representing the view in which the layer is displayed.
    *
    *  @return  The new plot widget for the given layer.  If the given layer
    *           is a raster layer or a cube layer, the grayscale plot widget
    *           is returned.  \b NULL is returned if a plot for the given layer
    *           already exists, or if the given layer does not support
    *           histogram data.
    *
    *  @see     getPlot(Layer*) const
    */
   virtual PlotWidget* createPlot(Layer* pLayer, PlotSet* pPlotSet = NULL) = 0;

   /**
    *  Returns the histogram plot widget for a given layer.
    *
    *  This method returns the histogram plot widget for a given layer that is
    *  contained in the window, regardless of the active plot set.
    *
    *  If the layer is a raster layer or cube layer, which has multiple plot
    *  widgets for the gray, red, green, and blue components, the grayscale
    *  plot is returned if the layer is currently in grayscale display mode,
    *  and the red plot is returned if the layer is in RGB display mode.  To
    *  get the plot of a specific color component, call the overloaded
    *  getPlot(Layer*, const RasterElement&) const function instead.
    *
    *  @param   pLayer
    *           The layer for which to get its histogram plot widget.
    *
    *  @return  The histogram plot widget for the given layer.  \b NULL is
    *           returned if the plot does not exist in any of the window's
    *           plot sets.
    *
    *  @see     RasterLayer, DisplayMode
    */
   virtual PlotWidget* getPlot(Layer* pLayer) const = 0;

   /**
    *  Returns the histogram plot widget for a given layer.
    *
    *  This method returns the histogram plot widget for a given layer with a
    *  given color component, regardless of the active plot set.  This is
    *  primarily useful to get the plot of a specific color component for a
    *  raster layer.
    *
    *  @param   pLayer
    *           The layer for which to get its histogram plot widget.
    *  @param   eColor
    *           The color component to use to get the plot.  If the given layer
    *           is not a raster layer, this value is ignored.
    *
    *  @return  The histogram plot widget for the given layer.  \b NULL is
    *           returned if the plot does not exist in any of the window's plot
    *           sets.
    *
    *  @see     RasterLayer
    */
   virtual PlotWidget* getPlot(Layer* pLayer, const RasterChannelType& eColor) const = 0;

   /**
    *  Sets the active histogram plot widget in the window.
    *
    *  This method set the active histogram plot widget in the window based on
    *  a given layer.  If the plot is contained on a plot set that is not
    *  active, that plot set is also activated.
    *
    *  If the layer is a raster layer, which has multiple plots
    *  for the gray, red, green, and blue components, the grayscale plot is
    *  activated if the layer is currently in grayscale display mode or the red
    *  plot is activated if the layer is in RGB display mode.  To activate the
    *  specific gray, red, green, or blue component plots, call the overloaded
    *  setCurrentPlot(Layer*, const RasterChannelType&) function.
    *
    *  @param   pLayer
    *           The layer for which to activate its plot widget in the window.
    *           If a plot displaying the given layer does not exist in the
    *           window, this method does nothing.
    *
    *  @see     RasterLayer, DisplayMode
    */
   virtual void setCurrentPlot(Layer* pLayer) = 0;

   /**
    *  Sets the active histogram plot widget in the window.
    *
    *  This method set the active histogram plot widget in the window based on
    *  a given layer and color component.  This is primarily useful to activate
    *  one of the multiple plots for a raster layer.    If the
    *  plot is contained on a plot set that is not active, that plot set is
    *  also activated.
    *
    *  @param   pLayer
    *           The layer for which to activate its plot widget in the window.
    *           If a plot displaying the given layer does not exist in the
    *           window, this method does nothing.
    *  @param   eColor
    *           The color component to use to activate the plot widget.  If the
    *           given layer is not a raster layer or cube layer, this value is
    *           ignored.
    *
    *  @return  The method returns \b true if the histogram plot was
    *           successfully activated; otherwise returns \b false.
    *
    *  @see     RasterLayer
    */
   virtual bool setCurrentPlot(Layer* pLayer, const RasterChannelType& eColor) = 0;

   /**
    *  Deletes the histogram plot widget of a given layer.
    *
    *  This method destroys the histogram plot widget of the given layer,
    *  regardless of the active plot set.
    *
    *  @param   pLayer
    *           The layer for which to delete its histogram plot widget.
    */
   virtual void deletePlot(Layer* pLayer) = 0;

protected:
   /**
    * This object should be destroyed by calling DesktopServices::deleteWindow().
    */
   virtual ~HistogramWindow() {}
};

#endif
