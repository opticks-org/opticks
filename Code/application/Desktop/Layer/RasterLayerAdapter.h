/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef RASTERLAYERADAPTER_H
#define RASTERLAYERADAPTER_H

#include "RasterLayer.h"
#include "RasterLayerImp.h"

class RasterElement;

class RasterLayerAdapter : public RasterLayer, public RasterLayerImp RASTERLAYERADAPTEREXTENSION_CLASSES
{
public:
   RasterLayerAdapter(const std::string& id, const std::string& layerName, RasterElement* pRasterElement);
   ~RasterLayerAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   // Layer
   Layer* copy(const std::string& layerName = std::string(), bool bCopyElement = false,
      DataElement* pParent = NULL) const;

   RASTERLAYERADAPTER_METHODS(RasterLayerImp)
};

#endif
