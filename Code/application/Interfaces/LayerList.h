/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef LAYERLIST_H
#define LAYERLIST_H

#include "LocationType.h"
#include "Subject.h"
#include "TypesFile.h"

#include <string>
#include <vector>

class DataElement;
class Layer;
class RasterElement;

/**
 *  Manages layers associated with a raster data set.
 *
 *  The layer list manages layers associated with a data set.  The
 *  layer list is contained within a spatial data view to provide access to
 *  draw the layers.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - A Layer is added to the list.
 *  - A Layer is removed from the list.
 *  - The list changes what RasterElement it is associated with.
 *  - Everything documented in Subject.
 *
 *  @see     RasterElement, SpatialDataView
 */
class LayerList : public Subject
{
public:
   /**
    *  Emitted with boost::any<Layer*> when a layer is added.
    */
   SIGNAL_METHOD(LayerList, LayerAdded)

   /**
    *  Emitted with boost::any<Layer*> when a layer is deleted from the list.
    */
   SIGNAL_METHOD(LayerList, LayerDeleted)

   /**
    *  Returns the primary RasterElement associated with the list.
    *
    *  @return  The primary RasterElement associated with the list.  NULL is
    *           returned if there is no primary RasterElement.
    *
    *  @see     RasterElement
    */
   virtual RasterElement* getPrimaryRasterElement() const = 0;

   /**
    *  Queries whether the list contains a given layer.
    *
    *  @param    pLayer
    *            The layer to query for its existence.  Cannot be NULL.
    *
    *  @return   TRUE if the layer is contained in the list, otherwise FALSE.
    */
   virtual bool containsLayer(Layer* pLayer) const = 0;

   /**
    *  Retrieves a layer in the list.
    *
    *  @param   layerType
    *           The layer type.
    *  @param   pElement
    *           The data element used as the basis for drawing the layer.
    *
    *  @return  A pointer to the existing layer, which can be safely cast to a
    *           derived layer class according to its layer type.  NULL is
    *           returned if a layer with the given parameters does not exist in
    *           the list.
    *
    *  @see     getLayers()
    */
   virtual Layer* getLayer(const LayerType& layerType, const DataElement* pElement) const = 0;

   /**
    *  Retrieves a layer in the list.
    *
    *  @param   layerType
    *           The layer type.
    *  @param   pElement
    *           The data element used as the basis for drawing the layer.  If
    *           \c NULL is passed in, the layer with the given name and type is
    *           returned.
    *  @param   layerName
    *           The layer name.  If the name is empty, the name of the data
    *           element is used.
    *
    *  @return  A pointer to the existing layer, which can be safely cast to a
    *           derived layer class according to its layer type.  \c NULL is
    *           returned if a layer with the given parameters does not exist in
    *           the list.
    *
    *  @see     getLayers()
    */
   virtual Layer* getLayer(const LayerType& layerType, const DataElement* pElement,
      const std::string& layerName) const = 0;

   /**
    *  Retrieves all layers of a given type in the list.
    *
    *  @param   layerType
    *           The layer type.
    *  @param   layers
    *           A vector that is populated with pointers to the layers of the
    *           given type contained in the list.
    *
    *  @deprecated
    *           This method is deprecated and may be removed in a future
    *           version.\  Use getLayers(LayerType) const instead, which returns
    *           a vector by value.
    */
   virtual void getLayers(LayerType layerType, std::vector<Layer*>& layers) const = 0;

   /**
    *  Retrieves all layers in the list.
    *
    *  @param   layers
    *           A vector that is populated with pointers to all layers
    *           contained in the list.
    *
    *  @deprecated
    *           This method is deprecated and may be removed in a future
    *           version.\  Use getLayers() const instead, which returns a vector
    *           by value.
    */
   virtual void getLayers(std::vector<Layer*>& layers) const = 0;

   /**
    *  Retrieves all layers in the list.
    *
    *  @return  A vector containing pointers to all layers in the list.
    *
    *  @see     getLayers(const LocationType&) const,
    *           getLayers(LayerType) const,
    *           getLayers(LayerType, const LocationType&) const
    */
   virtual std::vector<Layer*> getLayers() const = 0;

   /**
    *  Retrieves all layers at a given pixel coordinate in the list.
    *
    *  @param   worldCoord
    *           The pixel coordinate in world coordinates.
    *
    *  @return  A vector containing pointers to the layers in the list whose
    *           extents include the given pixel coordinate.
    *
    *  @see     getLayers() const,
    *           getLayers(LayerType) const,
    *           getLayers(LayerType, const LocationType&) const,
    *           Layer::getExtents()
    */
   virtual std::vector<Layer*> getLayers(const LocationType& worldCoord) const = 0;

   /**
    *  Retrieves all layers of a given type in the list.
    *
    *  @param   layerType
    *           The layer type.
    *
    *  @return  A vector containing pointers to the layers of the given type
    *           in the list.
    *
    *  @see     getLayers()const,
    *           getLayers(const LocationType&) const,
    *           getLayers(LayerType, const LocationType&) const
    */
   virtual std::vector<Layer*> getLayers(LayerType layerType) const = 0;

   /**
    *  Retrieves all layers of a given type at a given pixel coordinate in the
    *  list.
    *
    *  @param   layerType
    *           The layer type.
    *  @param   worldCoord
    *           The pixel coordinate in world coordinates.
    *
    *  @return  A vector containing pointers to the layers of the given type
    *           in the list whose extents include the given pixel coordinate.
    *
    *  @see     getLayers() const,
    *           getLayers(LayerType) const,
    *           getLayers(const LocationType&) const,
    *           Layer::getExtents()
    */
   virtual std::vector<Layer*> getLayers(LayerType layerType, const LocationType& worldCoord) const = 0;

   /**
    *  Returns the number of layers in the list of a given type.
    *
    *  @param   layerType
    *           The layer type for which to get the number of layers contained
    *           in the list.
    *
    *  @return  The number of layers contained in the list of the given type.
    *
    *  @see     getNumLayers() const,
    *           getNumLayers(const LocationType&) const,
    *           getNumLayers(LayerType, const LocationType&) const
    */
   virtual unsigned int getNumLayers(LayerType layerType) const = 0;

   /**
    *  Returns the total number of layers in the list.
    *
    *  @return  The total number of layers contained in the list.
    *
    *  @see     getNumLayers(LayerType) const,
    *           getNumLayers(const LocationType&) const,
    *           getNumLayers(LayerType, const LocationType&) const
    */
   virtual unsigned int getNumLayers() const = 0;

   /**
    *  Returns the number of layers in the list at a given pixel coordinate.
    *
    *  @param   worldCoord
    *           The pixel coordinate in world coordinates.
    *
    *  @return  The number of layers contained in the list whose extents include
    *           the given pixel coordinate.
    *
    *  @see     getNumLayers() const,
    *           getNumLayers(LayerType) const,
    *           getNumLayers(LayerType, const LocationType&) const,
    *           Layer::getExtents()
    */
   virtual unsigned int getNumLayers(const LocationType& worldCoord) const = 0;

   /**
    *  Returns the number of layers of a given type at a given pixel coordinate
    *  in the list.
    *
    *  @param   layerType
    *           The layer type.
    *  @param   worldCoord
    *           The pixel coordinate in world coordinates.
    *
    *  @return  The number of layers contained in the list of the given type
    *           whose extents include the given pixel coordinate.
    *
    *  @see     getNumLayers() const,
    *           getNumLayers(LayerType) const,
    *           getNumLayers(const LocationType&) const,
    *           Layer::getExtents()
    */
   virtual unsigned int getNumLayers(LayerType layerType, const LocationType& worldCoord) const = 0;

   /**
    *  Renames a layer to a user-defined name.
    *
    *  This method prompts the user to select a new layer name and renames
    *  the layer.  The user selected name is guaranteed to be unique within
    *  the layer list for the layer type.
    *
    *  @param   pLayer
    *           The layer to rename.  Must be a contained within the list and
    *           cannot be NULL.
    *
    *  @return  TRUE if the layer was successfully renamed, otherwise FALSE.
    *           FALSE is returned if the user cancels the dialog to select a
    *           new name.
    */
   virtual bool renameLayer(Layer* pLayer) const = 0;

   /**
    *  Renames a layer to a given name.
    *
    *  @param   pLayer
    *           The layer to rename.  Must be a contained within the list and
    *           cannot be NULL.
    *  @param   newName
    *           The layer to rename.  Must be a contained within the list and
    *           cannot be empty.  The name must also be unique within the layer
    *           list for the layer type.
    *
    *  @return  TRUE if the layer was successfully renamed, otherwise FALSE.
    *           FALSE is returned if the another layer of the same type has the
    *           same name as the given name.
    */
   virtual bool renameLayer(Layer* pLayer, const std::string& newName) const = 0;

protected:
   /**
    * A plug-in cannot create this object, it can only retrieve an already existing
    * object from SpatialDataView::getLayerList.  The SpatialDataView will manage any instances
    * of this object.
    */
   virtual ~LayerList() {}
};

#endif
