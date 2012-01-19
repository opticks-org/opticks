/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "PlotSetAdapter.h"

PlotSetAdapter::PlotSetAdapter(const std::string& id, const std::string& plotSetName, QWidget* pParent) :
   PlotSetImp(id, plotSetName, pParent)
{}

PlotSetAdapter::~PlotSetAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

const std::string& PlotSetAdapter::getObjectType() const
{
   static std::string sType("PlotSetAdapter");
   return sType;
}

bool PlotSetAdapter::isKindOf(const std::string& className) const
{
   if ((className == getObjectType()) || (className == "PlotSet"))
   {
      return true;
   }

   return PlotSetImp::isKindOf(className);
}
