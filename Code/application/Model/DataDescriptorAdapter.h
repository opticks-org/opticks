/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DATADESCRIPTORADAPTER_H
#define DATADESCRIPTORADAPTER_H

#include "DataDescriptor.h"
#include "DataDescriptorImp.h"

class DataDescriptorAdapter : public DataDescriptor, public DataDescriptorImp
{
public:
   DataDescriptorAdapter(const std::string& name, const std::string& type, DataElement* pParent);
   DataDescriptorAdapter(const std::string& name, const std::string& type, const std::vector<std::string>& parent);
   ~DataDescriptorAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   DATADESCRIPTORADAPTER_METHODS(DataDescriptorImp)
};

#endif
