/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DATAELEMENTGROUPADAPTER_H
#define DATAELEMENTGROUPADAPTER_H

#include "DataElementGroup.h"
#include "DataElementGroupImp.h"

class DataElementGroupAdapter : public DataElementGroup, public DataElementGroupImp
{
public:
   DataElementGroupAdapter(const DataDescriptorImp& descriptor, const std::string& id);
   ~DataElementGroupAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   DATAELEMENTGROUPADAPTER_METHODS(DataElementGroupImp)
};

#endif
