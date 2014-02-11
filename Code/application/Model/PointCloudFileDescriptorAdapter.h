/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef POINTCLOUDFILEDESCRIPTORADAPTER_H
#define POINTCLOUDFILEDESCRIPTORADAPTER_H

#include "PointCloudFileDescriptor.h"
#include "PointCloudFileDescriptorImp.h"

class PointCloudFileDescriptorAdapter : public PointCloudFileDescriptor, public PointCloudFileDescriptorImp
   POINTCLOUDFILEDESCRIPTORADAPTEREXTENSION_CLASSES
{
public:
   PointCloudFileDescriptorAdapter();
   ~PointCloudFileDescriptorAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   // FileDescriptor
   FileDescriptor* copy() const;

   POINTCLOUDFILEDESCRIPTORADAPTER_METHODS(PointCloudFileDescriptorImp)
};

#endif
