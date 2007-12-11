/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef REGIONOBJECTADAPTER_H
#define REGIONOBJECTADAPTER_H

#include "RegionObject.h"
#include "RegionObjectImp.h"

class RegionObjectAdapter : public RegionObject, public RegionObjectImp
{
public:
   RegionObjectAdapter(PlotViewImp* pPlot, bool bPrimary);
   ~RegionObjectAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   REGIONOBJECTADAPTER_METHODS(RegionObjectImp)
};

#endif
