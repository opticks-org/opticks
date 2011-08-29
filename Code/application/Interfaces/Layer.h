/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef LAYER_H
#define LAYER_H

#include "LocationType.h"
#include "SessionItem.h"
#include "Subject.h"
#include "Serializable.h"
#include "TypesFile.h"

#include <string>
#include <vector>

class DataElement;
class View;

/**
 *  %Services to adjust characteristics of layers.
 *
 *  This abstract base class provides access to properties for all layers that can
 *  be displayed in a layer list.  This class contains accessor methods to set and
 *  get the layer name, and each subclass contains accessor methods specific to the
 *  layer type.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *     - The following methods are called: setXScaleFactor(), setYScaleFactor(),
 *       setXOffset(), and setYOffset().
 *     - The DataElement for the layer posts a Subject::signalModified notification.
 *       Layer will then itself notify Subject::signalModified.
 *     - Everything else documented in Subject.
 *
 *  @see     LayerList
 */
class Layer : public SessionItem, public Subject, public Serializable
{
public:
   /**
    *  Emitted with any<std::string> when the layer name changes.
    */
   SIGNAL_METHOD(Layer, NameChanged)

   /**
    *  Emitted when the layer extents change.
    */
   SIGNAL_METHOD(Layer, ExtentsModified)

   /**
    *  @copydoc SessionItem::getContextMenuActions()
    *
    *  @default The default implementation returns the context menu actions
    *           listed \ref layer "here".
    */
   virtual std::list<ContextMenuAction> getContextMenuActions() const = 0;

   /**
    *  Returns the layer type
    *
    *  @return  The layer type.
    */
   virtual LayerType getLayerType() const = 0;

   /**
    *  Returns the associated data element.
    *
    *  @return  The data element upon which the layer is based.
    */
   virtual DataElement* getDataElement() const = 0;

   /**
    * Determine if the DataElement within the Layer is unique to all layers.
    *
    * @return True if this is the only layer with the given DataElement,
    *         false otherwise.
    */
   virtual bool hasUniqueElement() const = 0;
   /**
    *  Returns the view in which the layer is displayed.
    *
    *  @return  A pointer to the view in which the layer is displayed.  NULL
    *           is returned if the layer is not displayed in a view.
    */
   virtual View* getView() const = 0;

   /**
    *  Links a layer with this layer.
    *
    *  This method establishes a link between this layer and another layer.  Linked layers
    *  are linked by action.  %Setting properties in a layer sets the same property values
    *  in linked layers.
    *
    *  @param   pLayer
    *           The layer to link with this layer.  Cannot be NULL and must be of the same
    *           layer type as this layer.
    *
    *  @return  <b>true</b> if the layer was successfully linked with this
    *           layer, otherwise <b>false</b>.
    */
   virtual bool linkLayer(Layer* pLayer) = 0;

   /**
    *  Retrieves layers linked with this layer.
    *
    *  @param   linkedLayers
    *           A reference to a vector that is populated with pointers to layers linked
    *           with this layer.  If no layers are linked, the vector is emptied.
    */
   virtual void getLinkedLayers(std::vector<Layer*>& linkedLayers) const = 0;

   /**
    *  Queries whether a layer is linked with this layer.
    *
    *  @param   pLayer
    *           The layer to query for linkage with this layer.
    *
    *  @return  <b>true</b> if the layer is linked with this layer; otherwise
    *           <b>false</b>.
    */
   virtual bool isLayerLinked(Layer* pLayer) const = 0;

   /**
    *  Breaks the link between this layer and a given layer.
    *
    *  @param   pLayer
    *           The layer to unlink with this layer.  Cannot be NULL.
    *
    *  @return  <b>true</b> if the layer was successfully unlinked with this
    *           layer; otherwise <b>false</b>.
    */
   virtual bool unlinkLayer(Layer* pLayer) = 0;

   /**
    *  Creates a new layer with the same settings and properties as this layer.
    *
    *  @param   layerName
    *           The name for the new layer.  If an empty string is passed in
    *           the name of this layer is used for the new layer.
    *  @param   bCopyElement
    *           Set this value to \b true to make a copy of this layer's
    *           element.  Set this value to \b false to use this layer's
    *           element in the new layer.
    *  @param   pElement
    *           If the element is also copied, this is the parent of the new
    *           element.  If the element is not being copied, this parameter
    *           is ignored.
    *
    *  @warning When copying Annotation and AOI layers, the data element is
    *           always copied so a new layer name or a parent element should be
    *           passed in.  If the layer name is empty and an Annotation or AOI
    *           element cannot be created because one already exists, the
    *           element will be created with a GUID as a name.  The layer name
    *           will be the name of this layer.
    *
    *  @return  A pointer to the new layer.  \b NULL is returned if an error
    *           occurs.
    */
   virtual Layer* copy(const std::string& layerName = std::string(), bool bCopyElement = false,
      DataElement* pElement = NULL) const = 0;

   /**
    *  Retrieves the layer extents in world coordinates.
    *
    *  %Layer extents are defined as the pixel coordinate range of all data
    *  in the data element being displayed by the layer.  The populated extent
    *  coordinates already account for layer scaling and offset values.
    *
    *  @param   minX
    *           The minimum coordinate value of the layer in the X dimension.
    *  @param   minY
    *           The minimum coordinate value of the layer in the Y dimension.
    *  @param   maxX
    *           The maximum coordinate value of the layer in the X dimension.
    *  @param   maxY
    *           The maximum coordinate value of the layer in the Y dimension.
    *
    *  @return  Returns \c true if the extents of the layer were successfully
    *           retrieved; otherwise returns \c false.
    *
    *  @see     View::getExtents()
    */
   virtual bool getExtents(double& minX, double& minY, double& maxX, double& maxY) = 0;

   /**
    *  Sets the X-dimension scale factor when drawing.
    *
    *  %Any scaling is applied before the offset is applied.
    *
    *  @param   xScaleFactor
    *           The factor by which the pixels in the X-dimension are scaled
    *           while drawing.  If this value is negative or zero, this method
    *           does nothing.
    *
    *  @notify  This method will notify signalExtentsModified() if the scale
    *           factor value changes.
    */
   virtual void setXScaleFactor(double xScaleFactor) = 0;

   /**
    *  Returns the X-dimension scale factor when drawing.
    *
    *  %Any scaling is applied before the offset is applied.
    *
    *  @return  The factor by which the pixels in the X-dimension are scaled
    *           while drawing.
    */
   virtual double getXScaleFactor() const = 0;

   /**
    *  Sets the Y-dimension scale factor when drawing.
    *
    *  %Any scaling is applied before the offset is applied.
    *
    *  @param   yScaleFactor
    *           The factor by which the pixels in the Y-dimension are scaled
    *           while drawing.  If this value is negative or zero, this method
    *           does nothing.
    *
    *  @notify  This method will notify signalExtentsModified() if the scale
    *           factor value changes.
    */
   virtual void setYScaleFactor(double yScaleFactor) = 0;

   /**
    *  Returns the Y-dimension scale factor when drawing.
    *
    *  %Any scaling is applied before the offset is applied.
    *
    *  @return  The factor by which the pixels in the Y-dimension are scaled
    *           while drawing.
    */
   virtual double getYScaleFactor() const = 0;

   /**
    * Get the X-coordinate offset from the view's coordinate system.
    *
    * Offsets are applied after scale factors.
    *
    * @return The X offset.
    */
   virtual double getXOffset() const = 0;

   /**
    * Set the X-coordinate offset from the view's coordinate system.
    *
    * Offsets are applied after scale factors.
    *
    * @param xOffset
    *        The X offset.
    *
    *  @notify This method will notify signalExtentsModified.
    */
   virtual void setXOffset(double xOffset) = 0;

   /**
    * Get the Y-coordinate offset from the view's coordinate system.
    *
    * Offsets are applied after scale factors.
    *
    * @return The Y offset.
    */
   virtual double getYOffset() const = 0;

   /**
    * Set the Y-coordinate offset from the view's coordinate system.
    *
    * Offsets are applied after scale factors.
    *
    * @param yOffset
    *        The Y offset.
    *
    *  @notify This method will notify signalExtentsModified.
    */
   virtual void setYOffset(double yOffset) = 0;

   /**
    * Translate from world coordinates to data coordinates for this layer.
    *
    * @param worldX
    *        The x-coordinate to translate from.
    * @param worldY
    *        The y-coordinate to translate from.
    * @param dataX
    *        The x-coordinate to translate to.
    * @param dataY
    *        The y-coordinate to translate to.
    */
   virtual void translateWorldToData(double worldX, double worldY, double &dataX, double &dataY) const = 0;

   /**
    * Translate from data coordinates to world coordinates for this layer.
    *
    * @param dataX
    *        The x-coordinate to translate from.
    * @param dataY
    *        The y-coordinate to translate from.
    * @param worldX
    *        The x-coordinate to translate to.
    * @param worldY
    *        The y-coordinate to translate to.
    */
   virtual void translateDataToWorld(double dataX, double dataY, double &worldX, double &worldY) const = 0;

   /**
    * Translate from screen coordinates to data coordinates for this layer.
    *
    * @param screenX
    *        The x-coordinate to translate from.
    * @param screenY
    *        The y-coordinate to translate from.
    * @param dataX
    *        The x-coordinate to translate to.
    * @param dataY
    *        The y-coordinate to translate to.
    *
    * @see View::translateScreenToWorld, translateWorldToData
    */
   virtual void translateScreenToData(double screenX, double screenY, double &dataX, double &dataY) const = 0;

   /**
    * Translate from data coordinates to screen coordinates for this layer.
    *
    * @param dataX
    *        The x-coordinate to translate from.
    * @param dataY
    *        The y-coordinate to translate from.
    * @param screenX
    *        The x-coordinate to translate to.
    * @param screenY
    *        The y-coordinate to translate to.
    *
    * @see translateDataToWorld, View::translateWorldToScreen
    */
   virtual void translateDataToScreen(double dataX, double dataY, double &screenX, double &screenY) const = 0;

   /**
    *  Queries whether the layer's data coordinate system is flipped from the
    *  screen coordinate system.
    *
    *  @param   dataLowerLeft
    *           A lower left data coordinate for which to perform the
    *           data-to-screen transformation.
    *  @param   dataUpperRight
    *           An upper right data coordinate for which to perform the
    *           data-to-screen transformation.
    *  @param   bHorizontalFlip
    *           This parameter is set to \c true if the data coordinate system
    *           is flipped about the y-axis relative to the screen coordinate
    *           system; otherwise the parameter is set to \c false.
    *  @param   bVerticalFlip
    *           This parameter is set to \c true if the data coordinate system
    *           is flipped about the x-axis relative to the screen coordinate
    *           system; otherwise the parameter is set to \c false.
    *
    *  @see     translateDataToScreen()
    */
   virtual void isFlipped(const LocationType& dataLowerLeft, const LocationType& dataUpperRight,
      bool& bHorizontalFlip, bool& bVerticalFlip) const = 0;

protected:
   /**
    * This should be destroyed by calling SpatialDataView::deleteLayer.
    */
   virtual ~Layer() {}
};

#endif
