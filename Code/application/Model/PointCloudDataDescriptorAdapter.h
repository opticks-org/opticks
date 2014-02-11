/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef POINTCLOUDDATADESCRIPTORADAPTER_H
#define POINTCLOUDDATADESCRIPTORADAPTER_H

#include "PointCloudDataDescriptor.h"
#include "PointCloudDataDescriptorImp.h"

class PointCloudDataDescriptorAdapter : public PointCloudDataDescriptor, public PointCloudDataDescriptorImp
   POINTCLOUDDATADESCRIPTORADAPTEREXTENSION_CLASSES
{
public:
   PointCloudDataDescriptorAdapter(const std::string& name, const std::string& type, DataElement* pParent);
   PointCloudDataDescriptorAdapter(const std::string& name, const std::string& type,
      const std::vector<std::string>& parent);
   ~PointCloudDataDescriptorAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   POINTCLOUDDATADESCRIPTORADAPTER_METHODS(PointCloudDataDescriptorImp)
};

#endif
