/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SIMPLEVIEWS_H__
#define SIMPLEVIEWS_H__

#include "AppConfig.h"

class Layer;
class View;

#ifdef __cplusplus
extern "C"
{
#endif
   /** \addtogroup simple_api */
   /*@{*/

   /**
    * @file SimpleViews.h
    * This file contains API utilities for accessing views and layers.
    */

   /// Layers
   ///////////////////

   /**
    * Get a Layer given an identifying name.
    *
    * @param pName
    *        The name of a view and optional layer. First, the name will be checked as a session id.
    *        Next, the view name is checked against all views.
    *        If a | is present, the name after is used
    *        to obtain a Layer with the given name. If no view name is specified, the active view is
    *        used. If no layer name is specified, the behavior depends on the value of pType. If the type is a
    *        RasterLayer, the primary RasterLayer will be the default. If another type is specified, the active layer
    *        is the default if it is the correct type, otherwise the top-most layer of the proper type is specified.
    * @param pType
    *        If not \c NULL or empty, a Layer of this type will be returned. See TypeConverter for valid type names.
    * @return A pointer to the requested Layer or \c NULL if the layer is not found.
    *         If the layer is not found, getLastError() may be queried for information on the error.
    */
   EXPORT_SYMBOL Layer* getLayer(const char* pName, const char* pType);

   /**
    * Permanently destroy a Layer.
    *
    * Suitable for use as a cleanup callback.
    *
    * @param pLayer
    *        The Layer to destroy.
    *        No error checking is performed on this value and a \c NULL will cause a NOOP.
    */
   EXPORT_SYMBOL void destroyLayer(Layer* pLayer);

   /**
    * Get the name of the given Layer.
    *
    * @param pLayer
    *        The Layer to query.
    * @param pName
    *        Buffer to store the name. This will be \c NULL terminated.
    * @param nameSize
    *        The size of the buffer. If the name is too long, an error will be set.
    *        If this is 0, the minimum size of the buffer including the trailing \c NULL will be returned.
    * @return the actual length of the name.
    */
   EXPORT_SYMBOL uint32_t getLayerName(Layer* pLayer, char* pName, uint32_t nameSize);

   /**
    * Get the type name of the given Layer.
    *
    * This uses the XML encoding of LayerType.
    *
    * @param pLayer
    *        The Layer to query.
    * @param pType
    *        Buffer to store the type. This will be \c NULL terminated.
    * @param typeSize
    *        The size of the buffer. If the type is too long, an error will be set.
    *        If this is 0, the minimum size of the buffer including the trailing \c NULL will be returned.
    * @return the actual length of the type.
    */
   EXPORT_SYMBOL uint32_t getLayerType(Layer* pLayer, char* pType, uint32_t typeSize);

   /**
    * Get the DataElement displayed by a Layer.
    *
    * @param pLayer
    *        The Layer to query.
    * @return The DataElement displayed by the Layer or \c NULL on error.
    */
   EXPORT_SYMBOL DataElement* getLayerElement(Layer* pLayer);

   /**
    * Get the View which contains a Layer.
    *
    * @param pLayer
    *        The Layer to query.
    * @return The View which contains the Layer or \c NULL on error.
    */
   EXPORT_SYMBOL View* getLayerView(Layer* pLayer);

   /**
    * Get the scale and offset factors for a Layer.
    *
    * @param pLayer
    *        The Layer to query.
    * @param pScaleX
    *        Output parameter which will contain the scale factor in the X direction.
    *        May be \c NULL if this value is not needed.
    * @param pScaleY
    *        Output parameter which will contain the scale factor in the Y direction.
    *        May be \c NULL if this value is not needed.
    * @param pOffsetX
    *        Output parameter which will contain the offset in the X direction.
    *        May be \c NULL if this value is not needed.
    * @param pOffsetY
    *        Output parameter which will contain the offset in the Y direction.
    *        May be \c NULL if this value is not needed.
    * @return Zero on success, non-zero on error.
    */
   EXPORT_SYMBOL int getLayerScaleOffset(Layer* pLayer, double* pScaleX, double* pScaleY,
                                                        double* pOffsetX, double* pOffsetY);

   /**
    * Set the scale and offset factors for a Layer.
    *
    * @param pLayer
    *        The Layer to mutate.
    * @param scaleX
    *        The new scale factor in the X direction.
    *        If this is NaN, the value will not be changed.
    * @param scaleY
    *        The new scale factor in the Y direction.
    *        If this is NaN, the value will not be changed.
    * @param offsetX
    *        The new offset in the X direction.
    *        If this is NaN, the value will not be changed.
    * @param offsetY
    *        The new offset in the Y direction.
    *        If this is NaN, the value will not be changed.
    * @return Zero on success, non-zero on error.
    */
   EXPORT_SYMBOL int setLayerScaleOffset(Layer* pLayer, double scaleX, double scaleY, double offsetX, double offsetY);

   /**
    * Is the layer displayed?
    *
    * @param pLayer
    *        The Layer to access. Must be contained in a SpatialDataView.
    * @return Zero if the layer is hidden, non-zero if the layer is shown. getLastError()
    *         must be called for error status.
    */
   EXPORT_SYMBOL int isLayerDisplayed(Layer* pLayer);

   /**
    * Shown/hide a Layer.
    *
    * This will do nothing and return with no error if the requested state
    * equals the current state.
    *
    * @param pLayer
    *        The Layer to mutate. Must be contained in a SpatialDataView.
    * @param displayed
    *        If non-zero, the Layer will be shown, if zero it will be hidden.
    * @return Zero on success, non-zero on error.
    */
   EXPORT_SYMBOL int setLayerDisplayed(Layer* pLayer, int displayed);

   /**
    * Derive a new Layer and add it to the Layer's View.
    *
    * If the new Layer type is the same as the current type, a copy is made.
    *
    * @param pLayer
    *        The Layer to derive. Must be contained in a SpatialDataView.
    * @param pName
    *        The name for the new Layer. If this is \c NULL, the new Layer will have the same name.
    * @param pType
    *        The new \c NULL terminated Layer type name. This uses the XML encoding for LayerType.
    * @return \c NULL on error or the new Layer handle.
    */
   EXPORT_SYMBOL Layer* deriveLayer(Layer* pLayer, const char* pName, const char* pType);

   /**
    * Convert a Layer to a different Layer type.
    *
    * This actually creates a new Layer of the given type with the
    * same DataElement.
    *
    * @param pLayer
    *        The Layer to convert. Must be contained in a SpatialDataView.
    *        This handle will be invalid after a successful return from convertLayer()
    * @param pType
    *        The new \c NULL terminated Layer type name. This uses the XML encoding for LayerType.
    * @return \c NULL on error or the new Layer handle.
    */
   EXPORT_SYMBOL Layer* convertLayer(Layer* pLayer, const char* pType);

   /**
    * Is the Layer active?
    *
    * @param pLayer
    *        The Layer to query. Must be contained in a SpatialDataView.
    * @return Non-zero if active, zero otherwise.
    */
   EXPORT_SYMBOL int isLayerActive(Layer* pLayer);

   /**
    * Make the specified Layer active.
    *
    * @param pLayer
    *        The Layer to activate. Must be contained in a SpatialDataView.
    * @return Zero on success, non-zero on error.
    */
   EXPORT_SYMBOL int activateLayer(Layer* pLayer);

   /**
    * Get the display order of a Layer.
    *
    * @param pLayer
    *        The Layer to access. Must be in a SpatialDataView.
    * @return the display index of the Layer. 0 may indicate an error,
    *         getLastError() should be checked 
    */
   EXPORT_SYMBOL uint32_t getLayerDisplayIndex(Layer* pLayer);

   /**
    * Set the display order of a Layer.
    *
    * @param pLayer
    *        The Layer to mutate. Must be in a SpatialDataView.
    * @param newIndex
    *        The new zero based index of the Layer.
    * @return non-zero on error or zero on success.
    */
   EXPORT_SYMBOL int setLayerDisplayIndex(Layer* pLayer, uint32_t newIndex);

   /// Threshold Layers
   //////////////////////

   /**
    * Descriptor for threshold layer parameters.
    */
   struct ThresholdLayerInfo
   {
      double firstThreshold;     /**< The first threshold value in the specified regionUnits. */
      double secondThreshold;    /**< The second threshold value in the specified regionUnits.
                                      Certain pass area values ignore the second threshold */
      uint32_t passArea;         /**< The pass area. 0 -> Below 1st threshold value.
                                                     1 -> Above 1st threshold value.
                                                     2 -> Between 1st and 2nd threshold values.
                                                     3 -> Everywhere except between 1st and 2nd threshold values. */
      uint32_t regionUnits;      /**< Define the units for the threshold values.
                                      0 -> Raw values, 1 -> Percentage, 2 -> Percentile, 3 -> Std dev. */
   };

   /**
    * Get the threshold properties
    *
    * @param pLayer
    *        A threshold Layer to query.
    * @param pInfo
    *        Structure containing the threshold parameters. Must not be \c NULL.
    * @return non-zero on error or zero on success.
    */
   EXPORT_SYMBOL int getThresholdLayerInfo(Layer* pLayer, struct ThresholdLayerInfo* pInfo);

   /**
    * Set the threshold properties
    *
    * @param pLayer
    *        A threshold Layer to mutate.
    * @param pInfo
    *        Structure containing the threshold parameters. Must not be \c NULL.
    * @return non-zero on error or zero on success.
    */
   EXPORT_SYMBOL int setThresholdLayerInfo(Layer* pLayer, struct ThresholdLayerInfo* pInfo);

   /// Pseudocolor Layers
   //////////////////////

   /**
    * Get the number of pseudocolor classes attached to a PseudocolorLayer.
    *
    * @param pLayer
    *        A PseudocolorLayer to access.
    * @return the number of classes. A zero may indicate an error, getLastError() should be queried.
    */
   EXPORT_SYMBOL uint32_t getPseudocolorClassCount(Layer* pLayer);

   /**
    * Get the pseudocolor class id for a given index.
    *
    * @param pLayer
    *        A PseudocolorLayer to access.
    * @param index
    *        The zero based index to query.
    * @return The pseudocolor class ID or -1 on error.
    */
   EXPORT_SYMBOL int32_t getPseudocolorClassId(Layer* pLayer, uint32_t index);

   /**
    * Get the name of the given pseudocolor class.
    *
    * @param pLayer
    *        The PseudocolorLayer to query.
    * @param id
    *        The pseudocolor class id to query.
    * @param pName
    *        Buffer to store the name. This will be \c NULL terminated.
    * @param nameSize
    *        The size of the buffer. If the name is too long, an error will be set.
    *        If this is 0, the minimum size of the buffer including the trailing \c NULL will be returned.
    * @return the actual length of the name.
    */
   EXPORT_SYMBOL uint32_t getPseudocolorClassName(Layer* pLayer, int32_t id, char* pName, uint32_t nameSize);

   /**
    * Get the value of the given pseudocolor class.
    *
    * @param pLayer
    *        The PseudocolorLayer to query.
    * @param id
    *        The pseudocolor class id to query.
    * @return the value of the pseudocolor class. A zero may indicate an error, getLastError() should be queried.
    */
   EXPORT_SYMBOL int32_t getPseudocolorClassValue(Layer* pLayer, int32_t id);

   /**
    * Get the color of a given pseudocolor class.
    *
    * @param pLayer
    *        The PseudocolorLayer to query.
    * @param id
    *        The pseudocolor class id to query.
    * @return the color of the pseudocolor class. This is a 4 byte value in native endian.
    *         Red Green Blue Alpha ordering. A zero may indicate an error, getLastError() should be queried.
    */
   EXPORT_SYMBOL uint32_t getPseudocolorClassColor(Layer* pLayer, int32_t id);

   /**
    * Is the pseudocolor class displayed?
    *
    * @param pLayer
    *        The PseudocolorLayer to query.
    * @param id
    *        The pseudocolor class id to query.
    * @return Non-zero if the class is displayed, zero if it is not displayed
    *         or on error, getLastError() should be queried.
    */
   EXPORT_SYMBOL int isPseudocolorClassDisplayed(Layer* pLayer, int32_t id);

   /**
    * Add a new pseudocolor class.
    *
    * If the new class should be uninitialized, all parameters except pLayer must be \c NULL.
    * The new class should be initialized, all parameters must be non-\c NULL.
    *
    * @param pLayer
    *        The PseudocolorLayer to mutate.
    * @param pName
    *        If not \c NULL, the \c NULL terminated name for the new class.
    * @param pValue
    *        If not \c NULL, the value for the new class.
    * @param pColor
    *        If not \c NULL, the color for the new class. See setPseudocolorClassColor() for this value's encoding.
    * @param pDisplayed
    *        If not \c NULL, set the display state of the new class. Zero will be hidden, non-zero will be displayed.
    * @return The pseudocolor class id for the new class or -1 on error.
    */
   EXPORT_SYMBOL int32_t addPseudocolorClass(Layer* pLayer, const char* pName,
      int32_t* pValue, uint32_t* pColor, int* pDisplayed);

   /**
    * Set the name of the given pseudocolor class.
    *
    * @param pLayer
    *        The PseudocolorLayer to mutate.
    * @param id
    *        The pseudocolor class id to mutate.
    * @param pName
    *        The \c NULL terminated name for the pseudocolor class
    * @return Zero on success or non-zero on error.
    */
   EXPORT_SYMBOL int setPseudocolorClassName(Layer* pLayer, int32_t id, const char* pName);

   /**
    * Set the value of the given pseudocolor class.
    *
    * @param pLayer
    *        The PseudocolorLayer to mutate.
    * @param id
    *        The pseudocolor class id to mutate.
    * @param value
    *        The value for the pseudocolor class.
    * @return Zero on success or non-zero on error.
    */
   EXPORT_SYMBOL int setPseudocolorClassValue(Layer* pLayer, int32_t id, int32_t value);

   /**
    * Set the color of a given pseudocolor class.
    *
    * @param pLayer
    *        The PseudocolorLayer to mutate.
    * @param id
    *        The pseudocolor class id to mutate.
    * @param color
    8         The color for the pseudocolor class. This is a 4 byte value in native endian.
    *         Red Green Blue Alpha ordering.
    * @return Zero on success or non-zero on error.
    */
   EXPORT_SYMBOL int setPseudocolorClassColor(Layer* pLayer, int32_t id, uint32_t color);

   /**
    * Show or hide the pseudocolor class.
    *
    * @param pLayer
    *        The PseudocolorLayer to mutate.
    * @param id
    *        The pseudocolor class id to mutate.
    * @param display
    *        Non-zero to display the class, zero to hide the class.
    * @return Zero on success or non-zero on error.
    */
   EXPORT_SYMBOL int setPseudocolorClassDisplayed(Layer* pLayer, int32_t id, int display);

   /// Raster Layers
   ///////////////////

   /**
    * Descriptor for raster layer stretch parameters.
    */
   struct RasterLayerStretchInfo
   {
      double lowerStretch;       /**< The first stretch value */
      double upperStretch;       /**< The second stretch value. */
      uint32_t stretchType;      /**< Stretch type enum. 0 -> linear, 1 -> logarithmic,
                                                         2 -> exponential, 3 -> histogram equalization */
      uint32_t stretchUnits;     /**< Define the units for the stretch values.
                                      Identical to threshold layer RegionUnits */
   };

   /**
    * Get the raster stretch properties
    *
    * @param pLayer
    *        A RasterLayer to query.
    * @param channel
    *        The raster channel to query.
    *        0 -> gray, 1 -> red, 2 -> green, 3 -> blue
    * @param pInfo
    *        Structure containing the raster stretch parameters. Must not be \c NULL.
    * @return non-zero on error or zero on success.
    */
   EXPORT_SYMBOL int getRasterLayerStretchInfo(Layer* pLayer, uint32_t channel, struct RasterLayerStretchInfo* pInfo);

   /**
    * Set the raster stretch properties
    *
    * @param pLayer
    *        A RasterLayer to mutate.
    * @param channel
    *        The raster channel to mutate.
    *        0 -> gray, 1 -> red, 2 -> green, 3 -> blue
    * @param pInfo
    *        Structure containing the raster stretch parameters. Must not be \c NULL.
    * @return non-zero on error or zero on success.
    */
   EXPORT_SYMBOL int setRasterLayerStretchInfo(Layer* pLayer, uint32_t channel, struct RasterLayerStretchInfo* pInfo);

   /**
    * Get the complex component displayed in a RasterLayer.
    *
    * @param pLayer
    *        A RasterLayer to query.
    * @return the complex component. 0 -> magnitude, 1 -> phase, 2 -> in-phase, 3 -> quadrature.
    *         Any other value indicates an error.
    */
   EXPORT_SYMBOL uint32_t getRasterLayerComplexComponent(Layer* pLayer);

   /**
    * Set the complex component displayed in a RasterLayer.
    *
    * @param pLayer
    *        A RasterLayer to mutate.
    * @param component
    *         The complex component. 0 -> magnitude, 1 -> phase, 2 -> in-phase, 3 -> quadrature.
    * @return Non-zero on error, zero on success.
    */
   EXPORT_SYMBOL int setRasterLayerComplexComponent(Layer* pLayer, uint32_t component);

   /**
    * Get the name of the displayed colormap.
    *
    * A colormap is used when the RasterLayer is in grayscale/colormap/indexed mode.
    * The name may be the name of an internal colormap or a full path to a colormap file.
    *
    * @param pLayer
    *        The RasterLayer to query.
    * @param pName
    *        Buffer to store the name. This will be \c NULL terminated.
    * @param nameSize
    *        The size of the buffer. If the name is too long, an error will be set.
    *        If this is 0, the minimum size of the buffer including the trailing \c NULL will be returned.
    * @return the actual length of the name.
    */
   EXPORT_SYMBOL uint32_t getRasterLayerColormapName(Layer* pLayer, char* pName, uint32_t nameSize);

   /**
    * Set the displayed colormap.
    *
    * A colormap is used when the RasterLayer is in grayscale/colormap/indexed mode.
    * The name may be the name of an internal colormap or a full path to a colormap file.
    *
    * @param pLayer
    *        The RasterLayer to mutate.
    * @param pName
    *        \c NULL terminate name or full pathname of the colormap to display.
    * @return Zero on success or non-zero on error.
    */
   EXPORT_SYMBOL int setRasterLayerColormapName(Layer* pLayer, const char* pName);

   /**
    * Get the displayed colormap values.
    *
    * A colormap is used when the RasterLayer is in grayscale/colormap/indexed mode.
    *
    * @param pLayer
    *        The RasterLayer to query.
    * @param pColormap
    *        An array of 256 x 32-bit unsigned integers. Each integer represents an RGBA
    *        color quad where red is the high order byte and alpha is the low order byte.
    *        Encoding is in native endian. The must not be \c NULL.
    * @return Zero on success or non-zero on error.
    */
   EXPORT_SYMBOL int getRasterLayerColormapValues(Layer* pLayer, uint32_t* pColormap);

   /**
    * Set the displayed colormap values.
    *
    * A colormap is used when the RasterLayer is in grayscale/colormap/indexed mode.
    *
    * @param pLayer
    *        The RasterLayer to mutate.
    * @param pName
    *        The \c NULL terminate name for the new colormap. If this is \c NULL, no name will be
    *        associated with the new colormap.
    * @param pColormap
    *        An array of 256 x 32-bit unsigned integers. Each integer represents an RGBA
    *        color quad where red is the high order byte and alpha is the low order byte.
    *        Encoding is in native endian. The must not be \c NULL.
    * @return Zero on success or non-zero on error.
    */
   EXPORT_SYMBOL int setRasterLayerColormapValues(Layer* pLayer, const char* pName, uint32_t* pColormap);

   /**
    * Get the display mode of a RasterLayer.
    *
    * @param pLayer
    *        A RasterLayer to query.
    * @return A zero if the display is grayscale/colormap/indexed or a non-zero if the display is RGB.
    */
   EXPORT_SYMBOL int isRasterLayerRgbDisplayed(Layer* pLayer);

   /**
    * Set the display mode of a RasterLayer.
    *
    * @param pLayer
    *        A RasterLayer to mutate.
    * @param rgb
    *        A zero to set the display to grayscale/colormap/indexed or a non-zero to set the display to RGB.
    * @return Zero on success or non-zero on error.
    */
   EXPORT_SYMBOL int setRasterLayerRgbDisplayed(Layer* pLayer, int rgb);

   /**
    * Get the displayed elements and bands of a RasterLayer.
    *
    * @param pLayer
    *        A RasterLayer to query.
    * @param channel
    *        The color channel. 0 -> gray/index, 1 -> red, 2 -> green, 3 -> blue
    * @param pElement
    *        Output argument containing the RasterElement displayed in the channel. Ignore if \c NULL.
    * @return The active band number displayed in the channel.
    *         Zero may indicate an error, getLastError() should be checked.
    */
   EXPORT_SYMBOL uint32_t getRasterLayerDisplayedBand(Layer* pLayer, uint32_t channel, DataElement** pElement);

   /**
    * Set the displayed elements and bands of a RasterLayer.
    *
    * @param pLayer
    *        A RasterLayer to mutate.
    * @param channel
    *        The color channel. 0 -> gray/index, 1 -> red, 2 -> green, 3 -> blue
    * @param band
    *        The active band number to display.
    * @param pElement
    *        The RasterElement to display in the channel. If \c NULL, the currently displayed
    *        element will be maintained.
    * @return Zero on success or non-zero on error.
    */
   EXPORT_SYMBOL int setRasterLayerDisplayedBand(Layer* pLayer, uint32_t channel, uint32_t band, DataElement* pElement);

   /**
    * Raster statistics structure.
    */
   struct RasterStatistics
   {
      double min;                   /**< minimum value in the layer */
      double max;                   /**< maximum value in the layer */
      double mean;                  /**< mean value in the layer */
      double stddev;                /**< standard deviation of values in the layer */
      double* pHistogramCenters;    /**< array of 256 histogram center values. This is a borrowed reference. */
      unsigned int* pHistogramCounts; /**< array of 256 histogram counts. This is a borrowed reference.
                                           Note that this is not a sized data type so the current sizeof(unsigned int)
                                           must be known to use this. In most cases, this should be known. */
      double* pPercentiles;         /**< array of 1001 percentile boundaries. This is a borrowed reference. */
      uint32_t resolution;          /**< the calculation resolution. Represents the increment of the row and column
                                         counters used to calculate the statistics */
   };

   /**
    * The raster statistics for a RasterLayer.
    *
    * @param pLayer
    *        A RasterLayer to query.
    * @param channel
    *        The color channel. 0 -> gray/index, 1 -> red, 2 -> green, 3 -> blue
    * @param component
    *        The complex component to query. This should be 0 for non-complex data types.
    *        0 -> magnitude, 1 -> phase, 2 -> in-phase, 3 -> quadrature.
    * @param pStatistics
    *        Output parameter for the statistics. Arrays in this structure are borrowed
    *        references and may change when statistics are recalculated. A copy should be
    *        made if these will be used long term.
    * @return Zero if successful or non-zero on error.
    */
   EXPORT_SYMBOL int getRasterLayerStatistics(Layer* pLayer, uint32_t channel,
      uint32_t component, struct RasterStatistics* pStatistics);

   /**
    * Is the RasterLayer rendered on the GPU?
    *
    * @param pLayer
    *        A RasterLayer to query.
    * @return Zero if CPU rendering is active, non-zero if GPU rendering is active.
    *         Zero may be returned on error, getLastError() should be checked.
    */
   EXPORT_SYMBOL int getRasterLayerGpuEnabled(Layer* pLayer);

   /**
    * Set the RasterLayer render target.
    *
    * @param pLayer
    *        A RasterLayer to query.
    * @param gpu
    *        Zero to enable CPU rendering, non-zero to enable GPU rendering.
    * @return Zero on success or non-zero on error. SIMPLE_OTHER_FAILURE indicates GPU rendering is not supported.
    */
   EXPORT_SYMBOL int setRasterLayerGpuEnabled(Layer* pLayer, int gpu);

   /**
    * Query the number of filters supported by a RasterLayer.
    *
    * @param pLayer
    *        A RasterLayer to query.
    * @param enabled
    *        If zero, query all supported filters. If non-zero, query enabled filters.
    * @return The number of filters. A zero may indicate an error so getLastError() should be checked.
    */
   EXPORT_SYMBOL uint32_t getRasterLayerFilterCount(Layer* pLayer, int enabled);

   /**
    * Query the name of the filter with a given index.
    *
    * @param pLayer
    *        A RasterLayer to query.
    * @param index
    *        The filter index to query.
    * @param enabled
    *        If zero, query all supported filters. If non-zero, query enabled filters.
    * @param pName
    *        Buffer to store the name. This will be \c NULL terminated.
    * @param nameSize
    *        The size of the buffer. If the name is too long, an error will be set.
    *        If this is 0, the minimum size of the buffer including the trailing \c NULL will be returned.
    * @return the actual length of the name.
    */
   EXPORT_SYMBOL uint32_t getRasterLayerFilterName(Layer* pLayer, uint32_t index,
      int enabled, char* pName, uint32_t nameSize);

   /**
    * Enable the specified filters.
    *
    * @param pLayer
    *        A RasterLayer to mutate.
    * @param count
    *        The number of filters in the array.
    * @param pFilters
    *        Array of \c NULL terminated filter names of length \c count. This may be \c NULL iff \c count is 0.
    * @return Zero if successful or non-zero on error.
    */
   EXPORT_SYMBOL int setRasterLayerFilters(Layer* pLayer, uint32_t count, const char** pFilters);

   /**
    * Reset a feedback filter.
    *
    * @param pLayer
    *        A RasterLayer to mutate.
    * @param pFilterName
    *        The \c NULL terminated name of the filter to mutate. If this filter
    *        is not a feedback filter, nothing occurs and no error is generated.
    * @return Zero if successful or non-zero on error.
    */
   EXPORT_SYMBOL int resetRasterLayerFilter(Layer* pLayer, const char* pFilterName);

   /**
    * Freeze or unfreeze a feedback filter.
    *
    * @param pLayer
    *        A RasterLayer to mutate.
    * @param pFilterName
    *        The \c NULL terminated name of the filter to mutate. If this filter
    *        is not a feedback filter, nothing occurs and no error is generated.
    * @param freeze
    *        If zero unfreeze the filter or if non-zero freeze the filter.
    * @return Zero if successful or non-zero on error.
    */
   EXPORT_SYMBOL int setRasterLayerFilterFrozen(Layer* pLayer, const char* pFilterName, int freeze);

   /// Views
   ///////////////////

   /**
    * Get a View given an identifying name.
    *
    * @param pName
    *        The name of the view. First the name will be checked as a session id.
    *        Next the view name is checked against all views. If a | delimited is present, the first
    *        item is used as the name. This way, a Layer name specifier may be passed to obtain the
    *        View that will contain that layer.
    * @param pType
    *        If not \c NULL or empty, a View of this type will be returned. See TypeConverter for valid type names.
    * @return A pointer to the requested View or \c NULL if the view is not found.
    *         If the view is not found, getLastError() may be queried for information on the error.
    * @see getLayer()
    */
   EXPORT_SYMBOL View* getView(const char* pName, const char* pType);

   /**
    * Create a new View.
    *
    * @param pName
    *        \c NULL terminated name for the View.
    * @param pType
    *        \c NULL terminated type for the View.
    *        This uses the XML encoding of ViewType.
    * @param pElement
    *        If not \c NULL, this DataElement will be used as the
    *        primary DataElement for the new View. If the View type
    *        does not need a primary DataElement, this will be ignored.
    *        Currently, only SpatialDataView requires a primary DataElement
    *        which must be a RasterElement.
    * @return The new View handle or \c NULL if an error occurred.
    */
   EXPORT_SYMBOL View* createView(const char* pName, const char* pType, DataElement* pElement);

   /**
    * Permanently destroy a View.
    *
    * Suitable for use as a cleanup callback.
    *
    * @param pView
    *        The View to destroy.
    *        No error checking is performed on this value and a \c NULL will cause a NOOP.
    */
   EXPORT_SYMBOL void destroyView(View* pView);

   /**
    * Get the name of the given View.
    *
    * @param pView
    *        The View to query.
    * @param pName
    *        Buffer to store the name. This will be \c NULL terminated.
    * @param nameSize
    *        The size of the buffer. If the name is too long, an error will be set.
    *        If this is 0, the minimum size of the buffer including the trailing \c NULL will be returned.
    * @return the actual length of the name.
    */
   EXPORT_SYMBOL uint32_t getViewName(View* pView, char* pName, uint32_t nameSize);

   /**
    * Set the name of the given View.
    *
    * @param pView
    *        The View to mutate.
    * @param pName
    *        The new \c NULL terminated name
    * @return Zero if successful, non-zero on error.
    */
   EXPORT_SYMBOL int setViewName(View* pView, const char* pName);

   /**
    * Get the type name of the given View.
    *
    * This uses the XML encoding of ViewType.
    *
    * @param pView
    *        The View to query.
    * @param pType
    *        Buffer to store the type. This will be \c NULL terminated.
    * @param typeSize
    *        The size of the buffer. If the type is too long, an error will be set.
    *        If this is 0, the minimum size of the buffer including the trailing \c NULL will be returned.
    * @return the actual length of the type.
    */
   EXPORT_SYMBOL uint32_t getViewType(View* pView, char* pType, uint32_t typeSize);

   /**
    * Access the primary RasterElement.
    *
    * @param pView
    *        The View to query. This must be a SpatialDataView.
    * @return the primary RasterElement or \c NULL if an error occurred.
    */
   EXPORT_SYMBOL DataElement* getViewPrimaryRasterElement(View* pView);

   /**
    * Create a new Layer in a View.
    *
    * @param pView
    *        The View which will contain the new Layer. This must be a SpatialDataView.
    * @param pElement
    *        The DataElement which will be displayed by the Layer.
    * @param pType
    *        The \c NULL terminated type for the new Layer. Must not be \c NULL.
    * @param pName
    *        The \c NULL terminated name for the new Layer. If this is \c NULL, the name
    *        of pElement will be used.
    * @return the new Layer or \c NULL if an error occurred.
    */
   EXPORT_SYMBOL Layer* createLayer(View* pView, DataElement* pElement, const char* pType, const char* pName);

   /**
    * Get the number of layers in a View.
    *
    * @param pView
    *        The View to query. This must be a SpatialDataView.
    * @return The number of Layers in the View. A zero may represent an error. getLastError() should be checked.
    */
   EXPORT_SYMBOL uint32_t getViewLayerCount(View* pView);

   /**
    * Get a Layer by index from a View.
    *
    * @param pView
    *        The View to query. This must be a SpatialDataView.
    * @param index
    *        The zero based index of the Layer to query.
    * @return the Layer at index or \c NULL on error.
    */
   EXPORT_SYMBOL Layer* getViewLayer(View* pView, uint32_t index);

   /*@}*/
#ifdef __cplusplus
}
#endif

#endif
