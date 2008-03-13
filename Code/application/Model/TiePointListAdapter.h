/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef TIEPOINTLISTADAPTER_H
#define TIEPOINTLISTADAPTER_H

#include "TiePointList.h"
#include "TiePointListImp.h"

class TiePointListAdapter : public TiePointList, public TiePointListImp
{
public:
   TiePointListAdapter(const DataDescriptorImp& descriptor, const std::string& id);
   ~TiePointListAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   TIEPOINTLISTADAPTER_METHODS(TiePointListImp)
};

#endif
