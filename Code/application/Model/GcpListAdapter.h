/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

/**
  * The documentation of this class is in GcpList.h
  */
#ifndef GCPLISTADAPTER_H
#define GCPLISTADAPTER_H

#include "GcpList.h"
#include "GcpListImp.h"

class GcpListAdapter : public GcpList, public GcpListImp
{
public:
   GcpListAdapter(const DataDescriptorImp& descriptor, const std::string& id);
   ~GcpListAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   GCPLISTADAPTER_METHODS(GcpListImp)
};

#endif
