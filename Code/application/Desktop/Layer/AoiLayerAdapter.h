/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef AOILAYERADAPTER_H
#define AOILAYERADAPTER_H

#include "AoiLayer.h"
#include "AoiLayerImp.h"

class AoiElement;

class AoiLayerAdapter : public AoiLayer, public AoiLayerImp AOILAYERADAPTEREXTENSION_CLASSES
{
public:
   AoiLayerAdapter(const std::string& id, const std::string& layerName, AoiElement* pAoi);
   ~AoiLayerAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   // Layer
   Layer* copy(const std::string& layerName = std::string(), bool bCopyElement = false,
      DataElement* pParent = NULL) const;

   AOILAYERADAPTER_METHODS(AoiLayerImp)
};

#endif
