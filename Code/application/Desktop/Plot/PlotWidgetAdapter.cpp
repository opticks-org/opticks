/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "PlotWidgetAdapter.h"

using namespace std;

PlotWidgetAdapter::PlotWidgetAdapter(const string& id, const string& plotName, PlotType plotType,
                                     PlotSet* pPlotSet, QWidget* pParent) :
   PlotWidgetImp(id, plotName, plotType, pPlotSet, pParent)
{
}

PlotWidgetAdapter::~PlotWidgetAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

const string& PlotWidgetAdapter::getObjectType() const
{
   static string type("PlotWidgetAdapter");
   return type;
}

bool PlotWidgetAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "PlotWidget"))
   {
      return true;
   }

   return PlotWidgetImp::isKindOf(className);
}
