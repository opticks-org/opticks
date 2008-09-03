/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef RASTERLAYER_H
#define RASTERLAYER_H

#include "Layer.h"
#include "ColorType.h"
#include "ComplexData.h"
#include "ConfigurationSettings.h"
#include "DimensionDescriptor.h"
#include "TypesFile.h"

#include <string>
#include <vector>

class Animation;
class ImageFilterDescriptor;
class RasterElement;
class Statistics;

/**
 *  Adjusts the properties of a raster layer.
 *
 *  A raster layer displays all pixel values in shades of a certain color.  There are
 *  two display modes, grayscale and RGB.  For grayscale mode, values are displayed in
 *  shades of gray.  In RGB mode, values can be displayed in either the red, green, or
 *  blue display colors.  To enhance the layer display and contrast, values can be
 *  stretched for each display color to fit the display range.  Values between an upper
 *  and lower threshold value are stretch according to a stretch type and stretch units.
 *  This class provides the means to set all of these properties for the current raster
 *  layer and also as the default properties for new raster layers.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The following methods are called: setColorMap(), setComplexComponent(),
 *    setDisplayedBand(), setStretchType(), setStretchUnits(), setStretchValues(),
 *    setAlpha(), setAnimation(), enableFilter(), enableFilters(), disableFilter().
 *  - Everything else documented in Layer.
 *
 *  @see     Layer
 */
class RasterLayer : public Layer
{
public:
   SETTING(BackgroundTileGeneration, RasterLayer, bool, false)
   SETTING(ComplexComponent, RasterLayer, ComplexComponent, COMPLEX_MAGNITUDE)
   SETTING(GrayscaleStretchUnits, RasterLayer, RegionUnits, PERCENTILE)
   SETTING(GrayscaleStretchType, RasterLayer, StretchType, EXPONENTIAL)
   SETTING(BlueLowerStretchValue, RasterLayer, double, 0.0)
   SETTING(BlueUpperStretchValue, RasterLayer, double, 0.0)
   SETTING(GpuImage, RasterLayer, bool, false)
   SETTING(GrayLowerStretchValue, RasterLayer, double, 0.0)
   SETTING(GrayUpperStretchValue, RasterLayer, double, 0.0)
   SETTING(GreenLowerStretchValue, RasterLayer, double, 0.0)
   SETTING(GreenUpperStretchValue, RasterLayer, double, 0.0)
   SETTING(RedLowerStretchValue, RasterLayer, double, 0.0)
   SETTING(RedUpperStretchValue, RasterLayer, double, 0.0)
   SETTING(RedStretchUnits, RasterLayer, RegionUnits, PERCENTILE)
   SETTING(GreenStretchUnits, RasterLayer, RegionUnits, PERCENTILE)
   SETTING(BlueStretchUnits, RasterLayer, RegionUnits, PERCENTILE)
   SETTING(RgbStretchType, RasterLayer, StretchType, EXPONENTIAL)
   SETTING(FastContrastStretch, RasterLayer, bool, false)

   /**
    *  Emitted with boost::any<ComplexComponent> when the component is changed.
    */
   SIGNAL_METHOD(RasterLayer, ComplexComponentChanged)

   /**
    *  Emitted with boost::any<std::pair<RasterChannelType,DimensionDescriptor> > 
    *  when the displayed bands are changed.
    */
   SIGNAL_METHOD(RasterLayer, DisplayedBandChanged)

   /**
    *  Emitted with boost::any<std::vector<ImageFilterDescriptor*> > when the
    *  filters are changed.  It contains the enabled filters at the time it is
    *  emitted.
    */
   SIGNAL_METHOD(RasterLayer, FiltersChanged)

   /**
    *  Emitted with boost::any<bool> when the GPU is enabled or disabled.
    */
   SIGNAL_METHOD(RasterLayer, GpuImageEnabled)

   /**
    *  Emitted with boost::any<std::pair<DisplayMode,StretchType> > when the stretch type is changed.
    */
   SIGNAL_METHOD(RasterLayer, StretchTypeChanged)

   /**
    *  Emitted with boost::any<std::pair<RasterChannelType,RegionUnits> > when the stretch units are changed.
    */
   SIGNAL_METHOD(RasterLayer, StretchUnitsChanged)

   /**
    *  Emitted with boost::any<boost::tuple<RasterChannelType,double,double> > when the stretch values are changed.
    */
   SIGNAL_METHOD(RasterLayer, StretchValuesChanged)

   /**
    *  Emitted with boost::any<unsigned int> when the opacity is changed.
    */
   SIGNAL_METHOD(RasterLayer, AlphaChanged)

   /**
    *  Emitted with boost::any<DisplayMode> when the display mode is changed.
    */
   SIGNAL_METHOD(RasterLayer, DisplayModeChanged)

   /**
    *  Emitted when the color map changes with boost::any<std::string>
    *  containing the name of the new color map.
    */
   SIGNAL_METHOD(RasterLayer, ColorMapChanged)

   /**
    *  Emitted when the Animation changes with boost::any<Animation*>
    *  containing a pointer to the new Animation.
    */
   SIGNAL_METHOD(RasterLayer, AnimationChanged)

   /**
    *  @copydoc SessionItem::getContextMenuActions()
    *
    *  @default The default implementation returns the context menu actions
    *           listed \ref rasterlayer "here".
    */
   virtual std::list<ContextMenuAction> getContextMenuActions() const = 0;

   /**
    *  Queries whether the system hardware supports GPU images for this layer.
    *
    *  GPU images can be used to increase image display performance.  For these images, the
    *  contrast stretch is performed on the GPU instead of the CPU.  For GPU images to be
    *  supported, the system hardware must have the appropriate graphics capabilities and
    *  drivers.
    *
    *  When GPU images are displayed, there may occasionally be a loss of color resolution
    *  based on the data type and the range of data values being displayed.  If this occurs,
    *  The GPU image can be toggled off by calling enableGpuImage().
    *
    *  Currently, data sets containing complex data cannot be displayed with GPU images.
    *
    *  @return  True if the system hardware supports GPU images and the data set displayed
    *           by this layer supports GPU images.  Data sets containing complex data cannot
    *           be displayed with GPU images.
    *
    *  @see     enableGpuImage()
    */
   virtual bool isGpuImageSupported() const = 0;

   /**
    *  Toggles the GPU image display of the layer.
    *
    *  This method enabled or disables the use of the GPU image when displaying the layer.
    *  If GPU images are not supported, this method does nothing.
    *  View::refresh() should be called to force a redraw as this method does not automatically redraw the layer.
    *
    *  @param   bEnable
    *           Set this value to true to enable the GPU image or to false to disable the
    *           GPU image.
    *
    *  @notify  This method will notify signalGpuImageEnabled() with boost::any<bool>.
    *
    *  @see     isGpuImageSupported(), isGpuImageEnabled()
    */
   virtual void enableGpuImage(bool bEnable) = 0;

   /**
    *  Queries whether the GPU image is enabled.
    *
    *  @return  True if the GPU image is currently enabled, otherwise false.  False is
    *           also returned if GPU images are not supported.
    *
    *  @see     isGpuImageSupported(), enableGpuImage()
    */
   virtual bool isGpuImageEnabled() const = 0;

   /**
    *  Sets the display mode for the current raster layer.
    *
    *  @param   eMode
    *           The new display mode.
    *
    *  @see     getDisplayMode(), toggleDisplayMode()
    */
   virtual void setDisplayMode(const DisplayMode& eMode) = 0;

   /**
    *  Returns the display mode of the current raster layer.
    *
    *  @return  The current display mode.
    *
    *  @see     setDisplayMode()
    */
   virtual DisplayMode getDisplayMode() const = 0;

   /**
    *  Toggles the display mode of the current raster layer.
    *
    *  This toggles the current display mode between grayscale and RGB.
    *
    *  @see     setDisplayMode()
    */
   virtual void toggleDisplayMode() = 0;

   /**
    *  Sets a single band to display in the cube layer.
    *
    *  This method replaces the existing list of bands to display with the single input
    *  band.  The stretch for the color stays the same if the stretch units is not
    *  RegionUnits::RAW_VALUE.
    *
    *  @param   eColor
    *           The color for the new displayed bands.
    *  @param   band
    *           The band to display.
    *  @param   pRasterElement
    *           The raster element on which the band is located.  Defaults to the loaded cube.
    *
    *  @notify  signalDisplayedBandChanged() with boost::any<std::pair<RasterChannelType,const DimensionDescriptor*> >
    *
    *  @see     getDisplayedBand()
    */
   virtual void setDisplayedBand(RasterChannelType eColor, DimensionDescriptor band,
      RasterElement* pRasterElement = NULL) = 0;

   /**
    *  Returns the displayed band for the given color.
    *
    *  @param   eColor
    *           The color for the displayed bands.
    *
    *  @return  The displayed band for the given color.
    *
    *  @see     isBandDisplayed(), getDisplayedRasterElement()
    */
   virtual DimensionDescriptor getDisplayedBand(RasterChannelType eColor) const = 0;

   /**
    *  Returns the displayed raster element object for a given color.
    *
    *  @param   eColor
    *           The color for the displayed raster element.
    *
    *  @return  The displayed raster element of the given color.
    *
    *  @see     getDisplayedBand()
    */
   virtual RasterElement* getDisplayedRasterElement(RasterChannelType eColor) const = 0;

   /**
    *  Queries if a band is currently displayed.
    *
    *  @param   eColor
    *           The color of the display list to check.
    *  @param   band
    *           The band to query for its display.
    *  @param   pRasterElement
    *           The RasterElement on which the band is located.  Defaults to the loaded cube.
    *
    *  @return  TRUE is the band is displayed.  FALSE if the band is not
    *           displayed or the band number is out of range of the cube.
    *
    *  @see     getDisplayedBand(), getDisplayedRasterElement()
    */
   virtual bool isBandDisplayed(RasterChannelType eColor, DimensionDescriptor band,
      const RasterElement* pRasterElement = NULL) const = 0;

   /**
    * Get the statistics for the displayed channel.
    *
    * @param eChannel
    *        The channel where to get statistics from.
    *
    * @return Statistics for the given channel.
    */
   virtual Statistics *getStatistics(RasterChannelType eChannel) const = 0;

   /**
    *  Sets the current colormap that will be used for the GRAY RasterChannelType.
    *
    *  @param   name
    *           The name of the color map.
    *  @param   colorMap
    *           A vector containing 256 colors.
    *
    *  @notify  This method will notify Subject::signalModified.
    */
   virtual void setColorMap(const std::string& name, const std::vector<ColorType>& colorMap) = 0;

   /**
    *  Retrieves the current color table.
    *
    *  @return  The current color table.
    */
   virtual const std::vector<ColorType>& getColorMap() const = 0;

   /**
    *  Retrieves the name of the current color map.
    *
    *  @return  The current color map name.
    */
   virtual const std::string& getColorMapName() const = 0;

   /**
    *  Sets the display component for complex raster data.
    *
    *  This method sets the display component for complex data.  If the data
    *  is not complex, calling this method has no effect.
    *
    *  @param   eComponent
    *           The new complex display component.
    *
    *  @notify  This method will notify signalComplexComponentChanged() with boost::any<ComplexComponent>.
    *
    *  @see     ComplexComponent, getComplexComponent()
    */
   virtual void setComplexComponent(const ComplexComponent& eComponent) = 0;

   /**
    *  Returns the currently displayed component of complex raster data.
    *
    *  @return  The current complex display component.
    *
    *  @see     setComplexComponent()
    */
   virtual ComplexComponent getComplexComponent() const = 0;

   /**
    *  Sets the stretch type for the given display mode.
    *
    *  @param   eMode
    *           The display mode.
    *  @param   eType
    *           The new stretch type.
    *
    *  @notify  This method will notify signalStretchTypeChanged() with boost::any<std::pair<DisplayMode,StretchType> >.
    *
    *  @see     getStretchType()
    */
   virtual void setStretchType(const DisplayMode& eMode, const StretchType& eType) = 0;

   /**
    *  Returns the stretch type for the given display mode.
    *
    *  @param   eMode
    *           The display mode.
    *
    *  @return  The current stretch type.
    *
    *  @see     setStretchType()
    */
   virtual StretchType getStretchType(const DisplayMode& eMode) const = 0;

   /**
    *  Sets the stretch units for the given display mode.
    *
    *  @param   eMode
    *           The display mode.  RGB_MODE sets red, green, and blue.
    *  @param   eUnits
    *           The new stretch units.
    *
    *  @notify  This method will notify signalStretchUnitsChanged() with 
    *           boost::any<std::pair<RasterChannelType,RegionUnits> > for each RasterChannelType
    *           in the specified DisplayMode.
    *
    *  @see     getStretchUnits()
    */
   virtual void setStretchUnits(const DisplayMode& eMode, const RegionUnits& eUnits) = 0;

   /**
    *  Sets the stretch units for the given color.
    *
    *  @param   eColor
    *           The color to set.
    *  @param   eUnits
    *           The new stretch units.
    *
    *  @notify  This method will notify signalStretchUnitsChanged() with 
    *           boost::any<std::pair<RasterChannelType,RegionUnits> >.
    *
    *  @see     getStretchUnits()
    */
   virtual void setStretchUnits(const RasterChannelType& eColor, const RegionUnits& eUnits) = 0;

   /**
    *  Returns the stretch units for the given color.
    *
    *  @param   eColor
    *           The color.
    *
    *  @return  The current stretch units.
    *
    *  @see     setStretchUnits()
    */
   virtual RegionUnits getStretchUnits(const RasterChannelType& eColor) const = 0;

   /**
    *  Sets the stretch values for the given display color.
    *
    *  @param   eColor
    *           The display color.
    *  @param   dLower
    *           The lower stretch value.  Should be less than the upper value.
    *  @param   dUpper
    *           The upper stretch value.  Should be greater than the lower value.
    *
    *  @notify  This method will notify signalStretchValuesChanged() with 
    *           boost::any<boost::tuple<RasterChannelType,double,double> >.
    *
    *  @see     getStretchValues()
    */
   virtual void setStretchValues(const RasterChannelType& eColor, double dLower, double dUpper) = 0;

   /**
    *  Retreives the stretch values for the given display color.
    *
    *  @param   eColor
    *           The display color.
    *  @param   dLower
    *           A double value to contain the lower stretch value.
    *  @param   dUpper
    *           A double value to contain the upper stretch value.
    *
    *  @see     getStretchValues()
    */
   virtual void getStretchValues(const RasterChannelType& eColor, double& dLower, double& dUpper) const = 0;

   /**
    *  Converts a stretch value from one stretch units to another.
    *
    *  @param   eColor
    *           The display color.  This value is needed to get the data value
    *           statistics.
    *  @param   eUnits
    *           The initial stretch units.
    *  @param   dStretchValue
    *           The stretch value to convert.
    *  @param   eNewUnits
    *           The stretch units for the converted stretch value.
    *
    *  @return  The converted stretch value.  If an error occurred, 0.0 is returned.
    *
    *  @see     convertStretchValue()
    */
   virtual double convertStretchValue(const RasterChannelType& eColor, const RegionUnits& eUnits,
      double dStretchValue, const RegionUnits& eNewUnits) const = 0;

   /**
    *  Converts a stretch value from the current stretch units to another.
    *
    *  @param   eColor
    *           The display color.  This value is needed to get the data value
    *           statistics.
    *  @param   dStretchValue
    *           The stretch value to convert.
    *  @param   eNewUnits
    *           The stretch units for the converted stretch value.
    *
    *  @return  The converted stretch value.  If an error occurred, 0.0 is returned.
    *
    *  @see     convertStretchValue()
    */
   virtual double convertStretchValue(const RasterChannelType& eColor, double dStretchValue,
      const RegionUnits& eNewUnits) const = 0;

   /**
    *  Sets the opacity for the raster layer.
    *
    *  @param   alpha
    *           The opacity for the layer (0=transparent, 255=opaque).
    *
    *  @notify  This method will notify signalAlphaChanged() with boost::any<unsigned int>.
    *
    *  @see     getAlpha()
    */
   virtual void setAlpha(unsigned int alpha) = 0;

   /**
    *  Returns the opacity for the raster layer.
    *
    *  @return  The current opacity (0=transparent, 255=opaque).
    *
    *  @see     setAlpha()
    */
   virtual unsigned int getAlpha() const = 0;

   /**
    *  Determines if the layer can currently support fast changing of
    *  contrast stretch parameters (i.e. without regenerating the
    *  textures). This is a function of the underlying computer
    *  platform and the current contrast and alpha blending settings.
    *
    *  @return  true if fast contrast stretch changes are currently
    *     supported, false otherwise.
    *
    *  @see     setAlpha()
    */
   virtual bool canApplyFastContrastStretch() const = 0;

   /**
    *  This method is OBSOLETE and should not be used.
    *  It is maintained for binary compatibility.
    */
   virtual bool enableFastContrastStretch(bool enable) = 0;

   /**
    *  Queries whether an image filter is supported by the layer.
    *
    *  Not all layers support each filter type.  This method queries the layer
    *  to see if it supports the given type.
    *
    *  @param   filterName
    *           The name of the filter to query whether it is supported.
    *
    *  @return  Returns \c true if the filter is supported by the layer,
    *           otherwise returns \c false.  Since image filtering is only
    *           available for GPU images, \c false is also returned if GPU
    *           images are not supported.
    *
    *  @see     getSupportedFilters()
    */
   virtual bool isFilterSupported(const std::string& filterName) const = 0;

   /**
    *  Returns the names of all image filters supported by the layer.
    *
    *  @return  A vector of supported image filter names.
    *
    *  @see     isFilterSupported()
    */
   virtual std::vector<std::string> getSupportedFilters() const = 0;

   /**
    *  Enables an image filter.
    *
    *  This method enables the use of the given filter.  Each of the enabled
    *  filters are applied to the image before it is drawn to the screen.
    *  View::refresh() should be called to force a redraw as this method does
    *  not automatically redraw the layer.
    *
    *  Filtering is only available for GPU images.
    *
    *  @param   filterName
    *           The name of the filter to enable.  If the string is empty,
    *           already enabled, or not supported, this method does nothing.
    *
    *  @notify  This method will notify signalFiltersChanged() with
    *           boost::any<std::vector<ImageFilterDescriptor*> > containing the
    *           enabled filters at the end of the call.
    *
    *  @see     getEnabledFilter(), enableFilters(), disableFilter(),
    *           isGpuImageEnabled()
    */
   virtual void enableFilter(const std::string& filterName) = 0;

   /**
    *  Enables multiple image filters.
    *
    *  This method enables the use of the given filters.  Each of the enabled
    *  filters are applied to the image before it is drawn to the screen.  This
    *  method is similar to enableFilter(), but any currently enabled filters
    *  that are not in the given vector are disabled.  View::refresh() should
    *  be called to force a redraw as this method does not automatically redraw
    *  the layer.
    *
    *  Filtering is only available for GPU images.
    *
    *  @param   filterNames
    *           The names of the filters to enable.
    *
    *  @notify  This method will notify signalFiltersChanged() with
    *           boost::any<std::vector<ImageFilterDescriptor*> > containing the
    *           enabled filters at the end of the call.
    *
    *  @see     getEnabledFilters(), enableFilter(), disableFilter(),
    *           isGpuImageSupported()
    */
   virtual void enableFilters(const std::vector<std::string>& filterNames) = 0;

   /**
    *  Disables an image filter.
    *
    *  This method disables an image filter, so that the filter is not applied
    *  to an image before it is drawn.  View::refresh() should be called to
    *  force a redraw as this method does not automatically redraw the layer.
    *
    *  @param   filterName
    *           The name of the filter to disable.  If the string is empty, or
    *           if it is not already enabled, this method does nothing.
    *
    *  @notify  This method will notify signalFiltersChanged() with
    *           boost::any<std::vector<ImageFilterDescriptor*> > containing the
    *           enabled filters at the end of the call.
    *
    *  @see     enableFilter()
    */
   virtual void disableFilter(const std::string& filterName) = 0;

   /**
    *  Queries whether an image filter is enabled.
    *
    *  @param   filterName
    *           The name of the filter to query whether it is enabled.
    *
    *  @return  Returns \c true if the filter is currently enabled, otherwise
    *           returns \c false.
    *
    *  @see     enableFilter(), disableFilter()
    */
   virtual bool isFilterEnabled(const std::string& filterName) const = 0;

   /**
    *  Returns the enabled image filter with the given name.
    *
    *  @param   filterName
    *           Name of the enabled image filter descriptor requested.
    *
    *  @return  The enabled image filter descriptor pointer.  \c NULL is
    *           returned if the filter with the given name is not currently
    *           enabled.
    */
   virtual ImageFilterDescriptor* getEnabledFilter(const std::string& filterName) const = 0;

   /**
    *  Returns all currently enabled image filters.
    *
    *  @return  A reference to a vector of enabled filters.  The vector should
    *           not be modified.
    *
    *  @see     getEnabledFilterNames(), enableFilter(), disableFilter()
    */
   virtual const std::vector<ImageFilterDescriptor*>& getEnabledFilters() const = 0;

   /**
    *  Returns the names of all currently enabled image filters.
    *
    *  @return  The names of the currently enabled filters.
    *
    *  @see     getEnabledFilters()
    */
   virtual std::vector<std::string> getEnabledFilterNames() const = 0;

   /**
    *  Resets the image filter.
    *
    *  This method resets the image filter.  If the image filter has a
    *  ImageFilterDescriptor::FEEDBACK_FILTER type, the method clears the
    *  filter's estimate image buffer.  View::refresh() should be called to
    *  force a redraw as this method does not automatically redraw the layer.
    *
    *  Filtering is only available for GPU images.
    *
    *  @param   filterName
    *           The name of the filter to reset.
    *
    *  @see     freezeFilter()
    */
   virtual void resetFilter(const std::string& filterName) = 0;

   /**
    *  Freezes the image filter with its current values.
    *
    *  This method freezes the image filter.  If the image filter has a
    *  ImageFilterDescriptor::FEEDBACK_FILTER type, the method freezes the
    *  filter's estimate image buffer.  The estimate buffer's values do not
    *  update anymore.
    *
    *  Filtering is only available for GPU images.
    *
    *  @param   filterName
    *           The name of the filter to freeze.
    *
    *  @param   toggle
    *           Set this value to \c true to freeze the filter or to
    *           \c false to unfreeze the filter.
    *
    *  @see     resetFilter()
    */
   virtual void freezeFilter(const std::string& filterName, bool toggle = true) = 0;

   /**
    *  Returns the number of elements read in from the filtered results image
    *  buffer.
    *
    *  @param   xCoord
    *           The "x" coordinate used to read from the filtered results image
    *           buffer.
    *  @param   yCoord
    *           The "y" coordinate used to read from the filtered results image
    *           buffer.
    *  @param   width
    *           The width of the box of elements to read out of the filtered
    *           results image buffer.
    *  @param   height
    *           The height of the box of elements to read out of the filtered
    *           results image buffer.
    *  @param   values
    *           A vector populated with the filtered results.
    *
    *  @return  Number of elements read.  Zero is returned if
    *           isGpuImageEnabled() returns \c false or if getEnabledFilters()
    *           returns an empty vector.
    */
   virtual unsigned int readFilterBuffer(double xCoord, double yCoord, int width, int height,
      std::vector<float>& values) = 0;

   /**
    *  Associates an animation object with the layer.
    *
    *  This method associates an animation with the layer to automatically update the
    *  displayed band as the animation object is updated.  When the animation object is
    *  updated, the display mode is changed to grayscale mode before changing the
    *  displayed band.
    *
    *  @param   pAnimation
    *           The animation object to associate.  The layer does not take ownership
    *           of the animation, so a previously associated animation is not deleted.
    *           A value of NULL can be used to disable the automatic displayed
    *           band updates when the animation object is updated.
    *
    *  @notify  This method will notify signalAnimationChanged() with boost::any<Animation*>.
    *
    *  @see     getAnimation(), Animation
    */
   virtual void setAnimation(Animation* pAnimation) = 0;

   /**
    *  Returns the animation object associated with the layer.
    *
    *  @return  The associated animation object.  NULL is returned if no animation is
    *           currently associated with the layer (the default state).
    *
    *  @see     setAnimation(), Animation
    */
   virtual Animation* getAnimation() const = 0;

protected:
   /**
    * This should be destroyed by calling SpatialDataView::deleteLayer.
    */
   virtual ~RasterLayer() {}
};

#endif
