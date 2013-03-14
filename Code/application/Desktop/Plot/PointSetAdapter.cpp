/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "PointSetAdapter.h"

using namespace std;

PointSetAdapter::PointSetAdapter(PlotViewImp* pPlot, bool bPrimary) :
   PointSetImp(pPlot, bPrimary)
{
}

PointSetAdapter::~PointSetAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

const string& PointSetAdapter::getObjectType() const
{
   static string type("PointSetAdapter");
   return type;
}

bool PointSetAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "PointSet"))
   {
      return true;
   }

   return PointSetImp::isKindOf(className);
}
