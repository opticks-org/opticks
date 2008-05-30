/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef RASTERELEMENTADAPTER_H
#define RASTERELEMENTADAPTER_H

#include "RasterElement.h"
#include "RasterElementImp.h"

class RasterElementAdapter : public RasterElement, public RasterElementImp RASTERELEMENTADAPTEREXTENSION_CLASSES
{
public:
   RasterElementAdapter(const DataDescriptorImp& descriptor, const std::string& id);
   ~RasterElementAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   RASTERELEMENTADAPTER_METHODS(RasterElementImp)
};

#endif
