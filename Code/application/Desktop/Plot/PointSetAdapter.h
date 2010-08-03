/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef POINTSETADAPTER_H
#define POINTSETADAPTER_H

#include "PointSet.h"
#include "PointSetImp.h"

class PointSetAdapter : public PointSet, public PointSetImp POINTSETADAPTEREXTENSION_CLASSES
{
public:
   PointSetAdapter(PlotViewImp* pPlot, bool bPrimary);
   ~PointSetAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   POINTSETADAPTER_METHODS(PointSetImp)
};

#endif
