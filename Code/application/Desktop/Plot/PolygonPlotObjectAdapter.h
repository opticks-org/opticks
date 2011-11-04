/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef POLYGONPLOTOBJECTADAPTER_H
#define POLYGONPLOTOBJECTADAPTER_H

#include "PolygonPlotObject.h"
#include "PolygonPlotObjectImp.h"

class PolygonPlotObjectAdapter : public PolygonPlotObject, public PolygonPlotObjectImp POLYGONPLOTOBJECTADAPTEREXTENSION_CLASSES
{
public:
   PolygonPlotObjectAdapter(PlotViewImp* pPlot, bool bPrimary);
   PolygonPlotObjectAdapter(PlotViewImp* pPlot, bool bPrimary, LocationType point);
   PolygonPlotObjectAdapter(PlotViewImp* pPlot, bool bPrimary, double dX, double dY);
   ~PolygonPlotObjectAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   POLYGONPLOTOBJECTADAPTER_METHODS(PolygonPlotObjectImp)

private:
   PolygonPlotObjectAdapter(const PolygonPlotObjectAdapter& rhs);
};

#endif
