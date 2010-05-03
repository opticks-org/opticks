/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef RASTERDATADESCRIPTORADAPTER_H
#define RASTERDATADESCRIPTORADAPTER_H

#include "RasterDataDescriptor.h"
#include "RasterDataDescriptorImp.h"

class RasterDataDescriptorAdapter : public RasterDataDescriptor, public RasterDataDescriptorImp
   RASTERDATADESCRIPTORADAPTEREXTENSION_CLASSES
{
public:
   RasterDataDescriptorAdapter(const std::string& name, const std::string& type, DataElement* pParent);
   //RasterDataDescriptorAdapter(const std::string& name, const std::string& type,
   //   const std::vector<std::string>& parent);
   ~RasterDataDescriptorAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   RASTERDATADESCRIPTORADAPTER_METHODS(RasterDataDescriptorImp)
};

#endif
