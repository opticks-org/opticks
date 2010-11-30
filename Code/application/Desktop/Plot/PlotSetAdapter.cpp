/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "PlotSetAdapter.h"

using namespace std;

PlotSetAdapter::PlotSetAdapter(const string& id, const string& plotSetName, PlotWindow* pPlotWindow, QWidget* parent) :
   PlotSetImp(id, plotSetName, pPlotWindow, parent)
{
}

PlotSetAdapter::~PlotSetAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

const string& PlotSetAdapter::getObjectType() const
{
   static string type("PlotSetAdapter");
   return type;
}

bool PlotSetAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "PlotSet"))
   {
      return true;
   }

   return PlotSetImp::isKindOf(className);
}
