/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "PlotGroupAdapter.h"

using namespace std;

PlotGroupAdapter::PlotGroupAdapter(PlotViewImp* pPlot, bool bPrimary) :
   PlotGroupImp(pPlot, bPrimary)
{
}

PlotGroupAdapter::~PlotGroupAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

const string& PlotGroupAdapter::getObjectType() const
{
   static string type("PlotGroupAdapter");
   return type;
}

bool PlotGroupAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "PlotGroup"))
   {
      return true;
   }

   return PlotGroupImp::isKindOf(className);
}
