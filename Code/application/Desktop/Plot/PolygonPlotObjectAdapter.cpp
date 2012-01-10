/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "PolygonPlotObjectAdapter.h"

using namespace std;

PolygonPlotObjectAdapter::PolygonPlotObjectAdapter(PlotViewImp* pPlot, bool bPrimary) :
   PolygonPlotObjectImp(pPlot, bPrimary)
{
}

PolygonPlotObjectAdapter::~PolygonPlotObjectAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

const string& PolygonPlotObjectAdapter::getObjectType() const
{
   static string type("PolygonPlotObjectAdapter");
   return type;
}

bool PolygonPlotObjectAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "PolygonPlotObject"))
   {
      return true;
   }

   return PolygonPlotObjectImp::isKindOf(className);
}
