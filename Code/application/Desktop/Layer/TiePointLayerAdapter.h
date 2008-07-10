/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef TIE_POINT_LAYER_ADAPTER_H
#define TIE_POINT_LAYER_ADAPTER_H

#include "TiePointLayer.h"
#include "TiePointLayerImp.h"

class TiePointLayerAdapter : public TiePointLayer, public TiePointLayerImp TIEPOINTLAYERADAPTEREXTENSION_CLASSES
{
public:
   TiePointLayerAdapter(const std::string& id, const std::string& layerName, TiePointList* pElement);
   ~TiePointLayerAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   // Layer
   Layer* copy(const std::string& layerName = std::string(), bool bCopyElement = false,
      DataElement* pParent = NULL) const;

   TIEPOINTLAYERADAPTER_METHODS(TiePointLayerImp)
};

#endif
