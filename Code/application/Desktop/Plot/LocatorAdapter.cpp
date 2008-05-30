/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "LocatorAdapter.h"

using namespace std;

LocatorAdapter::LocatorAdapter(PlotViewImp* pPlot, bool bPrimary) :
   LocatorImp(pPlot, bPrimary)
{
}

LocatorAdapter::~LocatorAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

const string& LocatorAdapter::getObjectType() const
{
   static string type("LocatorAdapter");
   return type;
}

bool LocatorAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "Locator"))
   {
      return true;
   }

   return LocatorImp::isKindOf(className);
}
