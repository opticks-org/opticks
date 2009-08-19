/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "HistogramAdapter.h"

using namespace std;

HistogramAdapter::HistogramAdapter(PlotViewImp* pPlot, bool bPrimary) :
   HistogramImp(pPlot, bPrimary)
{
}

HistogramAdapter::~HistogramAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

const string& HistogramAdapter::getObjectType() const
{
   static string type("HistogramAdapter");
   return type;
}

bool HistogramAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "Histogram"))
   {
      return true;
   }

   return HistogramImp::isKindOf(className);
}
