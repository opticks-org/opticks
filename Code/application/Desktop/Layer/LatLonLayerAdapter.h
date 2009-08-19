/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef LATLONLAYERADAPTER_H
#define LATLONLAYERADAPTER_H

#include "LatLonLayer.h"
#include "LatLonLayerImp.h"

class RasterElement;

class LatLonLayerAdapter : public LatLonLayer, public LatLonLayerImp LATLONLAYERADAPTEREXTENSION_CLASSES
{
public:
   LatLonLayerAdapter(const std::string& id, const std::string& layerName, RasterElement* pRaster);
   ~LatLonLayerAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   // Layer
   Layer* copy(const std::string& layerName = std::string(), bool bCopyElement = false,
      DataElement* pParent = NULL) const;

   LATLONLAYERADAPTER_METHODS(LatLonLayerImp)
};

#endif
