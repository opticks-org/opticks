/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef THRESHOLDLAYERADAPTER_H
#define THRESHOLDLAYERADAPTER_H

#include "ThresholdLayer.h"
#include "ThresholdLayerImp.h"

class RasterElement;

class ThresholdLayerAdapter : public ThresholdLayer, public ThresholdLayerImp THRESHOLDLAYERADAPTEREXTENSION_CLASSES
{
public:
   ThresholdLayerAdapter(const std::string& id, const std::string& layerName, RasterElement* pRasterElement);
   ~ThresholdLayerAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   // Layer
   Layer* copy(const std::string& layerName = std::string(), bool bCopyElement = false,
      DataElement* pParent = NULL) const;

   THRESHOLDLAYERADAPTER_METHODS(ThresholdLayerImp)
};

#endif
