/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "PolarGridlinesAdapter.h"

using namespace std;

PolarGridlinesAdapter::PolarGridlinesAdapter(PlotViewImp* pPlot, bool bPrimary) :
   PolarGridlinesImp(pPlot, bPrimary)
{
}

PolarGridlinesAdapter::~PolarGridlinesAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

const string& PolarGridlinesAdapter::getObjectType() const
{
   static string type("PolarGridlinesAdapter");
   return type;
}

bool PolarGridlinesAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "PolarGridlines"))
   {
      return true;
   }

   return PolarGridlinesImp::isKindOf(className);
}
