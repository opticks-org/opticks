/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SPATIALDATAVIEW_H
#define SPATIALDATAVIEW_H

#include "PerspectiveView.h"
#include "ConfigurationSettings.h"
#include "DataVariantValidator.h"
#include "EnumWrapper.h"
#include "TypesFile.h"

#include <vector>

class Animation;
class DataElement;
class Layer;
class LayerList;
class QImage;
class RasterElement;

/**
* Specifies the panning range limit for the spatial data view
*/
enum PanLimitTypeEnum 
{
   NO_LIMIT=0,       /**< The panning range is not limited */
   CUBE_EXTENTS,     /**< The panning range is limited to the extents of the primary raster layer */ 
   MAX_EXTENTS       /**< The panning range is limited to the extents of all displayed layers*/
};

/**
 * @EnumWrapper ::PanLimitTypeEnum.
 */
typedef EnumWrapper<PanLimitTypeEnum> PanLimitType;

/**
 *  A view to display layers.
 *
 *  The spatial data view provides a means by which one or more layers are displayed.
 *  Layers that are available for display are contained in the view's layer list, which
 *  has an associated data set.  Layers for other data sets can be added to this view,
 *  which adds the layer to the layer list.  It is also possible to add all layers in
 *  another layer list to the view.  When displaying layers the view maintains a display
 *  index, which specifies the order in which the layers are drawn in the view.  For
 *  raster layers, a texture mode is available that specifies how the pixels are drawn
 *  at their boundaries.
 *
 *  The spatial data view defines the following mouse modes, where the name given is the
 *  name populated by MouseMode::getName():
 *  - LayerMode
 *  - MeasurementMode
 *  - PanMode
 *  - RotateMode
 *  - ZoomInMode
 *  - ZoomOutMode
 *  - ZoomBoxMode
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The following methods are called: setLayerDisplayIndex(), setTextureMode(), 
 *    showLayer(), hideLayer(), setActiveLayer(), setFrontLayer(), setBackLayer(),
 *    bringLayerForward(), sendLayerBackward().
 *  - The display index of any layer changes.
 *  - Everything else documented in PerspectiveView.
 *
 *  @see     Layer, LayerList
 */
class SpatialDataView : public PerspectiveView
{
public:
   SETTING(GeoCoordTooltip, SpatialDataView, bool, false)
   SETTING(ConfirmLayerDelete, SpatialDataView, bool, true)
   SETTING(FastPanSpeed, SpatialDataView, int, 20)
   SETTING(MaximumZoomRatio, SpatialDataView, double, 1.5)
   SETTING(MinimumZoomPixels, SpatialDataView, double, 1.0)
   SETTING(PanLimit, SpatialDataView, PanLimitType, CUBE_EXTENTS)
   SETTING(SlowPanSpeed, SpatialDataView, int, 10)
   SETTING(MousePanSensitivity, SpatialDataView, int, 100)

   /**
    *  Emitted with boost::any<TextureMode> when the texture mode is changed.
    */
   SIGNAL_METHOD(SpatialDataView, TextureModeChanged)

   /**
    *  Emitted with boost::any<Layer*> when a layer is shown.
    */
   SIGNAL_METHOD(SpatialDataView, LayerShown)

   /**
    *  Emitted with boost::any<Layer*> when a layer is hidden.
    */
   SIGNAL_METHOD(SpatialDataView, LayerHidden)

   /**
    *  Emitted when the display index of any layer changes.
    */
   SIGNAL_METHOD(SpatialDataView, LayerDisplayIndexesChanged)

   /**
    *  Emitted with boost::any<Layer*> when the layer that is edited with the
    *  LayerMode mouse mode changes.
    *
    *  This signal may be emitted if the layer is not displayed.
    *
    *  @see     setActiveLayer()
    */
   SIGNAL_METHOD(SpatialDataView, LayerActivated)

   /**
    *  Emitted with boost::any<bool> when mouse pan is enabled or
    *  disabled by the user.
    */
   SIGNAL_METHOD(SpatialDataView, MousePanEnabled)

   /**
    *  @copydoc SessionItem::getContextMenuActions()
    *
    *  @default The default implementation returns the context menu actions
    *           listed \ref spatialdataview "here".  The default actions can be
    *           removed or additional actions can be added by attaching to the
    *           signalAboutToShowContextMenu() signal.
    */
   virtual std::list<ContextMenuAction> getContextMenuActions() const = 0;

   /**
    *  Associates a RasterElement with the view.
    *
    *  This method associates a RasterElement with the view.  Once the
    *  RasterElement is associated with the view, it cannot be changed.
    *
    *  @return  TRUE if the element was successfully associated with the view.
    *           FALSE is returned if another RasterElement was previously
    *           associated with the view.
    *
    *  @see     RasterElement
    *  @see     LayerList::getPrimaryRasterElement()
    */
   virtual bool setPrimaryRasterElement(RasterElement* pRasterElement) = 0;

   /**
    *  Sets the view texture mode.
    *
    *  When zoomed in such that a scene pixel maps to more than one screen  pixel, the
    *  window can either replicate the scene pixel or interpolate  between scene pixels.
    *
    *  This method does not call refresh() so that multiple calls to modify
    *  view settings can be made without refreshing the view after each modification.
    *
    *  @param   textureMode
    *           The new texture mode.
    *
    *  @notify  This method will notify signalTextureModeChanged() with
    *           boost::any<TextureMode>.
    *
    *  @see     TextureMode
    */
   virtual void setTextureMode(const TextureMode& textureMode) = 0;

   /**
    *  Returns the view texture mode.
    *
    *  @return  The current texture mode.
    *
    *  @see     TextureMode
    */
   virtual TextureMode getTextureMode() const = 0;

   /**
    *  Returns the list of available display layers.
    *
    *  @return  The layer list used for displaying layers in the view.
    */
   virtual LayerList* getLayerList() const = 0;

   /**
    *  Adds a new layer to the view.
    *
    *  This method creates a new layer of a given type based on a given element
    *  and adds it to this view.  The layer is created with a name based on the
    *  name of the data element.  If a layer already exists with the same name
    *  as the element, then the layer is created with a unique name based on the
    *  given layer type.  In this case, the element name will not necessarily
    *  match the layer name.
    *
    *  @param   layerType
    *           The layer type.
    *  @param   pElement
    *           The data element used as the basis for drawing the layer.  If
    *           \c NULL is passed in, an element is created with a unique
    *           default name based on the element type associated with the given
    *           layer type.
    *
    *  @return  A pointer to the created layer, which can be safely cast to a
    *           derived layer class according to the layer type.  \c NULL is
    *           returned if the layer or data element could not be created.
    *
    *  @see     createLayer(const LayerType&, DataElement*, const std::string&)
    */
   virtual Layer* createLayer(const LayerType& layerType, DataElement* pElement) = 0;

   /**
    *  Adds a new layer to the view.
    *
    *  This method creates a new layer of a given type with a given name based
    *  on a given element and adds it to this view.  The layer is created with
    *  a name based on the name of the data element.  If a layer already exists
    *  with the same name as the element, then the layer is created with a
    *  unique name based on the given layer type.  In this case, the element
    *  name will not necessarily match the layer name.
    *
    *  @param   layerType
    *           The layer type.
    *  @param   pElement
    *           The data element used as the basis for drawing the layer.  If
    *           \c NULL is passed in, an element is created with a unique
    *           default name based on the element type associated with the given
    *           layer type.
    *  @param   layerName
    *           The name to assign to the created layer.  If the given name is
    *           empty, the name of the data element is used.
    *
    *  @return  A pointer to the created layer, which can be safely cast to a
    *           derived layer class according to the layer type.  \c NULL is
    *           returned if the layer or data element could not be created.
    *
    *  @see     createLayer(const LayerType&, DataElement*)
    */
   virtual Layer* createLayer(const LayerType& layerType, DataElement* pElement, const std::string& layerName) = 0;

   /**
    *  Adds an existing layer to the view.
    *
    *  @param   pLayer
    *           The layer to add to the view.  Cannot be NULL.
    *
    *  @return  TRUE if the layer was successfully added to the view.  FALSE
    *           is returned if the layer already exists in the view.
    *
    *  @see     createLayer(), addLayerList()
    */
   virtual bool addLayer(Layer* pLayer) = 0;

   /**
    *  Adds the layers from a layer list to the view.
    *
    *  The method adds the layers from a layer list into the view.  If the
    *  given layer list contains a layer that already exists in this list, the
    *  layer is not added again.
    *
    *  @param   pLayerList
    *           The layer list from which to add its layers to the view.
    *           Cannot be NULL.
    *
    *  @return  TRUE if the layers from the layer list were successfully
    *           added to the view.  This includes the cases where layers are
    *           not added to this list if they already exist in this list.
    *           FALSE is returned if an error occurs.
    *
    *  @see     addLayer()
    */
   virtual bool addLayerList(const LayerList* pLayerList) = 0;

   /**
    *  Returns layer types from which a given layer can be derived.
    *
    *  This is a convenience method that calls Layer::getLayerType() and then
    *  calls getDerivedLayerTypes(LayerType) const.
    *
    *  @param   pLayer
    *           The layer to query for layer types which can be derived.
    *
    *  @return  All layer types that can be derived from the given layer.
    *
    *  @see     getDerivedLayerTypes(LayerType) const, deriveLayer()
    */
   virtual std::vector<LayerType> getDerivedLayerTypes(const Layer* pLayer) const = 0;

   /**
    *  Returns layer types that can be derived from a given layer type.
    *
    *  @param   layerType
    *           The layer type to query for layer types which can be derived.
    *
    *  @return  All layer types that can be derived from the given layer type.
    *
    *  @see     getDerivedLayerTypes(const Layer*) const, deriveLayer()
    */
   virtual std::vector<LayerType> getDerivedLayerTypes(LayerType layerType) const = 0;

   /**
    *  Creates a new layer based on an existing layer of another type.
    *
    *  This method creates a new layer with a different type, but the same data
    *  element and name as an existing layer.  If a new annotation or AOI layer
    *  is created, the data element is copied.
    *
    *  @param   pLayer
    *           The existing layer from which to derive the new layer.  This layer
    *           can, but does not need to be contained in this view.
    *  @param   newLayerType
    *           The layer type for the new layer.  If \em newLayerType is the
    *           same as the type for \em pLayer a copy of the layer is made.
    *
    *  @return  A pointer to the created layer, which can be safely cast to a
    *           derived layer class according to the new layer type.  \b NULL
    *           is returned if the given layer does not support creating a new
    *           layer of the given type.
    *
    *  @see     getDerivedLayerTypes()
    */
   virtual Layer* deriveLayer(const Layer* pLayer, const LayerType& newLayerType) = 0;

   /**
    *  Converts an existing layer from one type to another.
    *
    *  This method is a convenience method that calls deriveLayer() followed by
    *  deleteLayer() with both calls wrapped in a single undo action group.
    *
    *  @param   pLayer
    *           The existing layer to convert to the new type.  The existing layer
    *           does not need to be contained in the view, but the new converted
    *           layer is only added to this view.
    *  @param   newLayerType
    *           The new layer type.
    *
    *  @return  A pointer to the created layer, which can be safely cast to a
    *           derived layer class according to the new layer type.  \b NULL is
    *           returned if the given layer cannot be converted to the new layer
    *           type.
    */
   virtual Layer* convertLayer(Layer* pLayer, const LayerType& newLayerType) = 0;

   /**
    *  Displays a layer in the layer list.
    *
    *  This method displays the given layer in the view.  The layer is displayed on
    *  top of the other displayed layers and has a display index of 0.
    *
    *  @param   pLayer
    *           The layer to display.  The layer must be included in the layer
    *           list.  Cannot be NULL.
    *
    *  @return  TRUE is the layer was successfully displayed.  FALSE if the layer
    *           does not exist in the layer list or if an error occurred.
    *
    *  @notify  This method will notify signalLayerShown() with
    *           boost::any<Layer*>.
    */
   virtual bool showLayer(Layer* pLayer) = 0;

   /**
    *  Hides a displayed layer in the layer list.
    *
    *  This method hides a displayed layer, but does not remove or delete the layer
    *  from the layer list.
    *
    *  @param   pLayer
    *           The layer to remove from the display.  The layer must be included
    *           in the layer list.  Cannot be NULL.
    *
    *  @return  TRUE is the layer was successfully hidden.  FALSE if the layer does
    *           not exist in the layer list or if an error occurred.
    *
    *  @notify  This method will notify signalLayerHidden() with
    *           boost::any<Layer*>.
    */
   virtual bool hideLayer(Layer* pLayer) = 0;

   /**
    *  Retrieves the displayed layers in the layer list.
    *
    *  @param   displayedLayers
    *           A reference to a vector to contain pointers for each displayed layer.
    *
    *  @see     isLayerDisplayed()
    */
   virtual void getDisplayedLayers(std::vector<Layer*>& displayedLayers) const = 0;

   /**
    *  Queries whether a layer is currently displayed.
    *
    *  @param   pLayer
    *           The layer to query for its display.  Cannot be NULL.
    *
    *  @return  TRUE if the layer is displayed.  FALSE if the layer is not displayed,
    *           or the layer does not exist.
    */
   virtual bool isLayerDisplayed(Layer* pLayer) const = 0;

   /**
    *  Returns the top-most layer.
    *
    *  This method returns the top-most visible layer in the view regardless of layer type.
    *
    *  @return  A pointer to the current layer.  \b NULL is returned if no
    *           visible layers are present in the view.
    *
    *  @see     getTopMostLayer(const LayerType&) const, getTopMostElement(),
    *           getActiveLayer()
    */
   virtual Layer* getTopMostLayer() const = 0;

   /**
    *  Returns the top-most layer of a given type.
    *
    *  This method returns the top-most visible layer of the given type.
    *
    *  @param   layerType
    *           The type of layer for which to get the top-most layer.
    *
    *  @return  A pointer to the current layer of the given type.  \b NULL is
    *           returned if no visible layers of the given type are present in the
    *           view.
    *
    *  @see     getTopMostLayer()
    */
   virtual Layer* getTopMostLayer(const LayerType& layerType) const = 0;

   /**
    *  Returns the data element of the top-most layer.
    *
    *  This method returns the data element being displayed by the top-most
    *  visible layer regardless of the layer type.
    *
    *  @return  A pointer to the data element of the current layer.  \b NULL is
    *           returned if no visible layers are present in the view.
    */
   virtual DataElement* getTopMostElement() const = 0;

   /**
    *  Returns the data element of the top-most layer of a given type.
    *
    *  This method returns the data element being displayed by the top-most
    *  visible layer of the given layer type.
    *
    *  @param   layerType
    *           The type of layer for which to get the top-most layer to get
    *           its element.
    *
    *  @return  A pointer to the data element of the current layer of the given
    *           type.  \b NULL is returned if no visible layers of the given type are
    *           present in the view.
    */
   virtual DataElement* getTopMostElement(LayerType layerType) const = 0;

   /**
    *  Returns the data element of a given type in the top-most layer of layers
    *  displaying elements of the given type.
    *
    *  This method returns the data element of the given type in the topmost
    *  layer that displays elements of the given type, regardless of whether
    *  the layer is shown or hidden.
    *
    *  @param   elementType
    *           The element type for which to get the element of the top-most
    *           layer displaying an element of that type.
    *
    *  @return  A pointer to the data element in the top-most layer that
    *           displays an element of the given type.  \b NULL is returned if
    *           no layers with an element of the given type are present in the
    *           view.
    */
   virtual DataElement* getTopMostElement(const std::string& elementType) const = 0;

   /**
    *  Enables an existing layer to be edited with the mouse.
    *
    *  This method sets the layer that will be edited with the mouse if the
    *  "LayerMode" mouse mode is active.  The active layer is maintained when
    *  the "LayerMode" mouse mode is not active.
    *
    *  @param   pLayer
    *           The layer to activate, which must be able to accept mouse
    *           events or \b NULL to set no layer as active.  If the given
    *           layer is not present in this view, this method does nothing.
    *
    *  @notify  This method will notify signalLayerActivated() if the active
    *           layer is successfully set.
    *
    *  @see     getTopMostLayer(), setMouseMode()
    */
   virtual void setActiveLayer(Layer* pLayer) = 0;

   /**
    *  Returns the layer that is edited with the mouse.
    *
    *  @return  The active layer that receives mouse events when the
    *           "LayerMode" mouse mode is active.  \b NULL is returned if an
    *           active layer has not yet been set on the view.
    *
    *  @see     setActiveLayer()
    */
   virtual Layer* getActiveLayer() const = 0;

   /**
    *  Moves an existing layer to the top position of the display order.
    *
    *  This method moves the given layer to the top of the display order.  If the
    *  given layer is already the top layer in the display, its position does
    *  not change.  Layers in the top position may obscur other layers on the display.
    *
    *  @param   pLayer
    *           The layer to set as the top layer.  The layer must be included in the
    *           layer list.  Cannot be NULL.
    *
    *  @return  TRUE is the layer was successfully repositioned to the top of the
    *           display order.  FALSE if the layer does not exist in the layer list
    *           or an error occurred.
    *
    *  @see     bringLayerForward()
    *
    *  @notify  This method will notify signalLayerDisplayIndexesChanged().
    */
   virtual bool setFrontLayer(Layer* pLayer) = 0;

   /**
    *  Moves an existing layer to the bottom position of the display order.
    *
    *  This method moves the given layer to the bottom of the display order.  If the
    *  given layer is already the bottom layer in the display, its position does
    *  not change.  Layers in the bottom position may be obscurred by other layers
    *  on the display.
    *
    *  @param   pLayer
    *           The layer to set as the bottom layer.  The layer must be included in
    *           the layer list.  Cannot be NULL.
    *
    *  @return  TRUE is the layer was successfully repositioned to the bottom of the
    *           display order.  FALSE if the layer does not exist in the layer list
    *           or an error occurred.
    *
    *  @see     sendLayerBackward()
    *
    *  @notify  This method will notify signalLayerDisplayIndexesChanged().
    */
   virtual bool setBackLayer(Layer* pLayer) = 0;

   /**
    *  Moves an existing layer up one position in the display order.
    *
    *  This method increases the display order of an existing layer by one position.
    *  If the given layer is already the top layer in the display, its position does
    *  not change.
    *
    *  @param   pLayer
    *           The layer to reposition.  The layer must be included in the layer
    *           list.  Cannot be NULL.
    *
    *  @return  TRUE is the layer was successfully repositioned in the display order.
    *           FALSE if the layer does not exist in the layer list or an error
    *           occurred.
    *
    *  @see     setFrontLayer()
    *
    *  @notify  This method will notify signalLayerDisplayIndexesChanged().
    */
   virtual bool bringLayerForward(Layer* pLayer) = 0;

   /**
    *  Moves an existing layer down one position in the display order.
    *
    *  This method decreases the display order of an existing layer by one position.
    *  If the given layer is already the bottom layer in the display, its position does
    *  not change.
    *
    *  @param   pLayer
    *           The layer to reposition.  The layer must be included in the layer
    *           list.  Cannot be NULL.
    *
    *  @return  TRUE is the layer was successfully repositioned in the display order.
    *           FALSE if the layer does not exist in the layer list or an error
    *           occurred.
    *
    *  @see     setBackLayer()
    *
    *  @notify  This method will notify signalLayerDisplayIndexesChanged().
    */
   virtual bool sendLayerBackward(Layer* pLayer) = 0;

   /**
    *  Sets the display order for the given layer.
    *
    *  This method sets the layer z-order of a currently displayed layer.  If the given
    *  layer is hidden, it is automatically shown.
    *
    *  @param   pLayer
    *           The layer for which to set the display index.  The layer must be included
    *           in the layer list.  Cannot be NULL.
    *  @param   iIndex
    *           The new zero-based display index for the layer.  A display index of 0 is
    *           the topmost layer.
    *
    *  @return  TRUE if the layer order was successfully set.  FALSE if the layer does not
    *           exist in the layer list or an error occurred.
    *
    *  @notify  This method will notify signalLayerDisplayIndexesChanged().
    */
   virtual bool setLayerDisplayIndex(Layer* pLayer, int iIndex) = 0;

   /**
    *  Returns the display index for the given layer.
    *
    *  This method returns an index value representing the layer z-order in the displayed
    *  layers.  The topmost layer has a display order of 0, and pixels drawn in this may
    *  obscure those in lower layers.
    *
    *  @param   pLayer
    *           The layer for which to get its current display index.
    *
    *  @return  The zero-based layer index representing the z-order value.  A value of -1
    *           is returned if the layer is not displayed or if the layer does not exist
    *           in the layer list.
    */
   virtual int getLayerDisplayIndex(Layer* pLayer) const = 0;

   /**
    *  Deletes a layer in the view.
    *
    *  This method removes the given layer from the view and deletes it.  It
    *  also deletes its corresponding data element only if no other layers
    *  are currently displaying the same element.
    *
    *  @param   pLayer
    *           The layer in the view to delete.  Cannot be NULL.
    *
    *  @return  TRUE is the layer was successfully removed and deleted from
    *           the view, otherwise FALSE.
    *
    *  @see     clear()
    */
   virtual bool deleteLayer(Layer* pLayer) = 0;

   /**
    *  Deletes all layers in the view.
    */
   virtual void clear() = 0;

   /**
    *  Clears all analyst markings on the view.  The resulting view is as if
    *  the RasterElement had just been loaded.  This action cannot be undone.
    */
   virtual void clearMarkings() = 0;

   /**
    *  Retrieves 2D image data for a displayed layer.
    *
    *  @param   pLayer
    *           The layer for which to get an image.
    *  @param   pImage
    *           A pointer to the layer image pixel data.
    *  @param   iWidth
    *           The number of pixel columns in the layer image.
    *  @param   iHeight
    *           The number of pixel rows in the layer image.
    *  @param   iDepth
    *           The number of bits per pixel in the layer image.
    *  @param   iNumColors
    *           The number of colors present in the layer image.  This is not the
    *           total number of available colors specified by the depth.
    *  @param   transparent
    *           A reference to a color that is populated with the color value that
    *           is used to represent transparent pixels in the image.  Transparent
    *           pixels are those pixels within the layer extents that do not contain
    *           valid layer data. If a valid color is passed in, this will be used as
    *           the transparency color.
    *  @param   bbox
    *           An integer array that is populated with the extents of the layer in
    *           screen pixels.
    *
    *  @return  TRUE if the layer image was successfully retrieved, otherwise FALSE.
    */
   virtual bool getLayerImage(Layer* pLayer, unsigned char*& pImage, int& iWidth, int& iHeight,
      int& iDepth, int& iNumColors, ColorType& transparent, int bbox[4]) = 0;

   /**
    * Retrieves 2D image data for a displayed layer.
    *
    * @param pLayer
    *        The layer for which to get an image.
    * @param image
    *        The QImage which will contain the layer image. The state of this object will be reset.
    * @param transparent
    *        A reference to a color that is populated with the color value that
    *        is used to represent transparent pixels in the image.  Transparent
    *        pixels are those pixels within the layer extents that do not contain
    *        valid layer data.  If a valid color is passed in, this will be used as
    *        the transparency color.
    * @param bbox
    *        Contains layer extents for drawing.
    *        - This parameter is used as input if the layer is not currently displayed.
    *        When used as input, the values are interpreted as follows:
    *             - bbox[0] The width of the image
    *             - bbox[1] The height of the image
    *             - bbox[2] Ignored
    *             - bbox[3] Ignored
    *        - When the method returns this parameter is populated with the bounding box of the drawn portion of the layer.
    *             - bbox[0] Minimum X
    *             - bbox[1] Minimum Y
    *             - bbox[2] Maximum X
    *             - bbox[3] Maximum Y
    *          
    * @return \c True if the layer image was successfully retrieved, otherwise \c false.
    */
   virtual bool getLayerImage(Layer* pLayer, QImage &image, ColorType& transparent, int bbox[4]) = 0;

   /**
    * Sets the type of limit on panning the view.
    *
    * This method sets how panning will be limited within the view.
    *
    *  @param  eType
    *          The type of panning limit for the view.
    *    
    *  @see  PanLimitType, getPanLimit()
    */
   virtual void setPanLimit(PanLimitType eType) = 0;

   /**
   * Returns the current type of limit on panning in the view.
   *
   * This method returns how panning is currently limited within the view.
   *
   *  @return  The type of panning limit for the view.
   *    
   *  @see  PanLimitType, setPanLimit()
   */
   virtual PanLimitType getPanLimit() const = 0;

   /**
   * Sets the limits for the minimum zoom.
   *
   * This method sets the minimum zoom which is the ratio of the
   * scene extents to the current view size, e.g., a value of 0.5 
   * would limit zooming out such that the scene would fill no less 
   * than one half of the view's shortest dimension.
   *
   *  A value of 0 will disable the minimum zoom limit.
   *
   *  @param  dMinZoom
   *          The ratio of the scene extents to the current view size.
   *    
   *  @see  getMinimumZoom()
   */
   virtual void setMinimumZoom(double dMinZoom) = 0;

   /**
   * Returns the current minimum zoom
   *
   * This method returns the current minimum zoom.
   *
   *  @return  The current minimum zoom.
   *    
   *  @see  setMinimumZoom()
   */
   virtual double getMinimumZoom() const = 0;

   /**
   * Sets the limits for the maximum zoom.
   *
   * This method sets the maximum zoom which is defined as the minimum number
   * of scene pixels that are visible in the view in the shortest view dimension,
   * e.g., a value of 1.5 would limit zooming in such that one and a half scene
   * pixels would fill the view in the shortest dimension.
   *
   *  A value of 0 will disable the maximum zoom limit.
   *
   *  @param  dMaxZoom
   *          The maximum zoom.
   *    
   *  @see  getMaximumZoom()
   */
   virtual void setMaximumZoom(double dMaxZoom) = 0;

   /**
   * Returns the current maximum zoom
   *
   * This method returns the current maximum zoom.
   *
   *  @return  The current maximum zoom.
   *    
   *  @see  setMaximumZoom()
   */
   virtual double getMaximumZoom() const = 0;

   /**
    *  Creates a new animation attached to the primary raster layer.
    *
    *  This method creates a frame-based animation in the view's animation
    *  controller that is attached to the primary raster layer so that the
    *  displayed bands are automatically updated when the animation is played.
    *
    *  The animation name is set to the primary raster layer name.  If the
    *  primary raster layer already contains an animation, a new animation is
    *  not created.
    *
    *  An animation controller is created if the view does not already have an
    *  associated animation controller.  If the view already has an animation
    *  controller that is time-based and it is empty, the controller will be
    *  deleted and a new frame-based controller will be created with the same
    *  name as the view.  If the existing time-based controller is not empty, a
    *  new animation is not created.
    *
    *  If the animation is successfully created, the animation controller is
    *  activated on the animation toolbar, and the toolbar is displayed if
    *  necessary.
    *
    *  @return  A pointer to the created Animation for the primary raster
    *           layer.  \b NULL is returned if there is no primary raster
    *           layer, or if the animation could not be created.
    *
    *  @notify  This method may notify
    *           AnimationServices::signalControllerCreated() with
    *           boost::any<AnimationController*> if the animation controller
    *           for the view needs to be created and may also notify
    *           AnimationToolBar::signalControllerChanged() with
    *           boost::any<AnimationController*> if the controller is
    *           successfully activated on the animation toolbar.
    *
    *  @see     getAnimationController(), RasterLayer::getAnimation()
    */
   virtual Animation* createDefaultAnimation() = 0;

   /**
    *  Resets the stretch type, units, values, and displayed band(s)
    *  for all channels in all visible raster layers back to those used when the layer was created.
    *
    *  @notify This method may notify the same signals as RasterLayer::resetStretch() for each visible RasterLayer.
    *
    *  @see    RasterLayer::resetStretch()
    */
   virtual void resetStretch() = 0;

protected:
   /**
    * This should be destroyed by calling DesktopServices::deleteView.
    */
   virtual ~SpatialDataView() {}
};

/**
 * \cond INTERNAL
 * This template specialization is required to allow this type to be put into a DataVariant.
 */
template <> class VariantTypeValidator<PanLimitType> {};
/// \endcond

#endif
