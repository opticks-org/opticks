/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GRIDLINESADAPTER_H
#define GRIDLINESADAPTER_H

#include "Gridlines.h"
#include "GridlinesImp.h"

class GridlinesAdapter : public Gridlines, public GridlinesImp GRIDLINESADAPTEREXTENSION_CLASSES
{
public:
   GridlinesAdapter(PlotViewImp* pPlot, bool bPrimary);
   ~GridlinesAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   GRIDLINESADAPTER_METHODS(GridlinesImp)
};

#endif
