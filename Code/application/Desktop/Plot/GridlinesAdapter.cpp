/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "GridlinesAdapter.h"

using namespace std;

GridlinesAdapter::GridlinesAdapter(PlotViewImp* pPlot, bool bPrimary) :
   GridlinesImp(pPlot, bPrimary)
{
}

GridlinesAdapter::~GridlinesAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

const string& GridlinesAdapter::getObjectType() const
{
   static string type("GridlinesAdapter");
   return type;
}

bool GridlinesAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "Gridlines"))
   {
      return true;
   }

   return GridlinesImp::isKindOf(className);
}
