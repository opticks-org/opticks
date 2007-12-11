/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DATAELEMENTADAPTER_H
#define DATAELEMENTADAPTER_H

#include "DataElement.h"
#include "DataElementImp.h"

class DataElementAdapter : public DataElement, public DataElementImp
{
public:
   DataElementAdapter(const DataDescriptorImp& descriptor, const std::string& id);
   ~DataElementAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   DATAELEMENTADAPTER_METHODS(DataElementImp)
};

#endif
