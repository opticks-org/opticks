/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "CartesianGridlinesAdapter.h"

using namespace std;

CartesianGridlinesAdapter::CartesianGridlinesAdapter(OrientationType orientation, PlotViewImp* pPlot, bool bPrimary) :
   CartesianGridlinesImp(orientation, pPlot, bPrimary)
{
}

CartesianGridlinesAdapter::~CartesianGridlinesAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

const string& CartesianGridlinesAdapter::getObjectType() const
{
   static string type("CartesianGridlinesAdapter");
   return type;
}

bool CartesianGridlinesAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "CartesianGridlines"))
   {
      return true;
   }

   return CartesianGridlinesImp::isKindOf(className);
}
