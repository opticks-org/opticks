/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PSEUDOCOLORLAYERADAPTER_H
#define PSEUDOCOLORLAYERADAPTER_H

#include "PseudocolorLayer.h"
#include "PseudocolorLayerImp.h"

class RasterElement;

class PseudocolorLayerAdapter : public PseudocolorLayer, public PseudocolorLayerImp PSEUDOCOLORLAYERADAPTEREXTENSION_CLASSES
{
public:
   PseudocolorLayerAdapter(const std::string& id, const std::string& layerName, RasterElement* pRasterElement);
   ~PseudocolorLayerAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   // Layer
   Layer* copy(const std::string& layerName = std::string(), bool bCopyElement = false,
      DataElement* pParent = NULL) const;

   PSEUDOCOLORLAYERADAPTER_METHODS(PseudocolorLayerImp)

};

#endif
