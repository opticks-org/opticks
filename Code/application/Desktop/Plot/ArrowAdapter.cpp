/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ArrowAdapter.h"

using namespace std;

ArrowAdapter::ArrowAdapter(PlotViewImp* pPlot, bool bPrimary) :
   ArrowImp(pPlot, bPrimary)
{
}

ArrowAdapter::ArrowAdapter(ArrowStyle arrowStyle, PlotViewImp* pPlot, bool bPrimary) :
   ArrowImp(arrowStyle, pPlot, bPrimary)
{
}

ArrowAdapter::~ArrowAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

const string& ArrowAdapter::getObjectType() const
{
   static string type("ArrowAdapter");
   return type;
}

bool ArrowAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "Arrow"))
   {
      return true;
   }

   return ArrowImp::isKindOf(className);
}
