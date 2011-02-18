/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "RegionObjectAdapter.h"

using namespace std;

RegionObjectAdapter::RegionObjectAdapter(PlotViewImp* pPlot, bool bPrimary) :
   RegionObjectImp(pPlot, bPrimary)
{
}

RegionObjectAdapter::~RegionObjectAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

const string& RegionObjectAdapter::getObjectType() const
{
   static string type("RegionObjectAdapter");
   return type;
}

bool RegionObjectAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "RegionObject") || (className == "Region"))
   {
      return true;
   }

   return RegionObjectImp::isKindOf(className);
}
