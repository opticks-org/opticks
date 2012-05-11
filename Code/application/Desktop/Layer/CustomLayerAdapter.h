/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CUSTOMLAYERADAPTER_H
#define CUSTOMLAYERADAPTER_H

#include "CustomLayer.h"
#include "CustomLayerImp.h"

#include <string>

class Any;
class DataElement;

class CustomLayerAdapter : public CustomLayer, public CustomLayerImp CUSTOMLAYERADAPTEREXTENSION_CLASSES
{
public:
   CustomLayerAdapter(const std::string& id, const std::string& layerName, Any* pElement);
   virtual ~CustomLayerAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   // Layer
   Layer* copy(const std::string& layerName = std::string(), bool bCopyElement = false,
      DataElement* pParent = NULL) const;

   CUSTOMLAYERADAPTER_METHODS(CustomLayerImp)

private:
   CustomLayerAdapter(const CustomLayerAdapter& rhs);
};

#endif
