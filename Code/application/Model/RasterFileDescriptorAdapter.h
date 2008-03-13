/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef RASTERFILEDESCRIPTORADAPTER_H
#define RASTERFILEDESCRIPTORADAPTER_H

#include "RasterFileDescriptor.h"
#include "RasterFileDescriptorImp.h"

class RasterFileDescriptorAdapter : public RasterFileDescriptor, public RasterFileDescriptorImp
{
public:
   RasterFileDescriptorAdapter();
   ~RasterFileDescriptorAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   // FileDescriptor
   FileDescriptor* copy() const;

   RASTERFILEDESCRIPTORADAPTER_METHODS(RasterFileDescriptorImp)
};

#endif
