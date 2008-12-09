/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef HISTOGRAMPLOT_H
#define HISTOGRAMPLOT_H

#include "CartesianPlot.h"
#include "TypesFile.h"

class Layer;

/**
 *  Sets the data and adjusts the properties of a histogram plot.
 *
 *  The histogram data to display in the plot can be derived from a layer by
 *  calling setHistogram(Layer*) or can be set by passing in raw histogram
 *  values into
 *  setHistogram(unsigned int,const double*,const double*,const double*,bool).
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The histogram data changes.
 *  - The layer associated with the histogram changes.
 *  - Everything documented in CartesianPlot.
 *
 *  @see     CartesianPlot
 */
class HistogramPlot : public CartesianPlot
{
public:
   /**
    *  @copydoc SessionItem::getContextMenuActions()
    *
    *  @default The default implementation returns the context menu actions
    *           listed \ref histogramplot "here".  The default actions can be
    *           removed or additional actions can be added by attaching to the
    *           signalAboutToShowContextMenu() signal.
    */
   virtual std::list<ContextMenuAction> getContextMenuActions() const = 0;

   /**
    *  Sets the histogram data values to those of the data element for a given
    *  layer.
    *
    *  @param   pLayer
    *           The layer for which to display the histogram of its data.  If
    *           \em pLayer is \b NULL or not a RasterLayer or ThresholdLayer,
    *           the existing histogram is cleared.  If \em pLayer is a
    *           RasterLayer, the histogram of the data currently displayed in
    *           grayscale mode is set.  To set the histogram for a different
    *           display color, call setHistogram(Layer*, RasterChannelType)
    *           instead.
    *
    *  @notify  This method will notify Subject::signalModified().
    *
    *  @return  Returns \b true if the histogram for the layer was set
    *           successfully, otherwise returns \b false.
    */
   virtual bool setHistogram(Layer* pLayer) = 0;

   /**
    *  Sets the histogram data values to those of the data element for a given
    *  layer with a given display color.
    *
    *  @param   pLayer
    *           The layer for which to display the histogram of its data.  See
    *           setHistogram(Layer*) for restrictions on the layer type.
    *  @param   color
    *           The display color for which to get the histogram to set.  If
    *           \em pLayer is not a RasterLayer, this parameter is ignored.
    *
    *  @notify  This method will notify Subject::signalModified().
    *
    *  @return  Returns \b true if the histogram for the layer and display
    *           color was set successfully, otherwise returns \b false.
    */
   virtual bool setHistogram(Layer* pLayer, RasterChannelType color) = 0;

   /**
    *  Sets the histogram data values.
    *
    *  The histogram plot contains a default properties page widget that allows
    *  users to set the histogram color.  The page only appears in the properties
    *  dialog when the histogram data contains at least one bin.  Calling this
    *  method creates the default properties page widget if a properties widget
    *  has not yet been set and \e binCount is greater than zero.  If the
    *  number of bins is zero and the properties page widget is the default widget,
    *  the widget is deleted.
    *
    *  @param   binCount
    *           The number of data values in the histogram.
    *  @param   pBinCenters
    *           The histogram bin center values.
    *  @param   pValues
    *           The count values for each histogram bin.
    *  @param   pBinWidths
    *           The width of each histogram bin.  If this value is \b NULL, a
    *           default bin width is calculated.
    *  @param   bAbove
    *           An optional flag to display the count values as either positive
    *           or negative.  Set this parameter to \b true to count the values
    *           as a positive number.
    *
    *  @notify  This method will notify Subject::signalModified().
    *
    *  @return  Returns \b true if the histogram data values were set successfully;
    *           otherwise returns \b false.
    *
    *  @see     getPropertiesWidget()
    */
   virtual bool setHistogram(unsigned int binCount, const double* pBinCenters, const double* pValues,
      const double* pBinWidths = NULL, bool bAbove = true) = 0;

   /**
    *  Sets the histogram color.
    *
    *  @param   histogramColor
    *           The new color for the histogram.  If \em histogramColor is
    *           invalid, the method does nothing.
    *
    *  @see     ColorType::isValid()
    */
   virtual void setHistogramColor(const ColorType& histogramColor) = 0;

   /**
    *  Returns the histogram color.
    *
    *  @return  The current histogram color.
    */
   virtual ColorType getHistogramColor() const = 0;

protected:
   /**
    * This object should be destroyed by calling DesktopServices::deleteView().
    */
   virtual ~HistogramPlot() {}
};

/**
 *  Extends capability of the HistogramPlot interface.
 *
 *  This class provides additional capability for the HistogramPlot interface
 *  class.  A pointer to this class can be obtained by performing a dynamic cast
 *  on a HistogramPlot pointer.
 *
 *  @warning A pointer to this class can only be used to call methods contained
 *           in this extension class and cannot be used to call any methods in
 *           HistogramPlot.
 */
class HistogramPlotExt1
{
public:
   /**
    *  Toggles the capability of the plot to update its zoom when the histogram
    *  data changes.
    *
    *  This method toggles the capability to automatically zoom RasterLayer
    *  plots to the extents of the histogram data when the displayed band
    *  changes.  This capability is only available for plots displaying
    *  RasterLayer histograms.
    *
    *  @param   enable
    *           Set this value to \c true to automatically zoom a RasterLayer
    *           plot when the displayed band changes.  Set this value to
    *           \c false to not modify the zoom level when the displayed band
    *           changes.
    */
   virtual void enableAutoZoom(bool enable) = 0;

   /**
    *  Queries whether the plot updates its zoom when the histogram data
    *  changes.
    *
    *  @return  Returns \c true if a RasterLayer plot automatically zooms to
    *           the extents of the histogram data when the displayed band
    *           Returns \c false if the zoom level is not modified when the
    *           displayed band changes.
    */
   virtual bool isAutoZoomEnabled() const = 0;

protected:
   /**
    *  This object should be destroyed by calling DesktopServices::deleteView().
    */
   virtual ~HistogramPlotExt1() {}
};

#endif
