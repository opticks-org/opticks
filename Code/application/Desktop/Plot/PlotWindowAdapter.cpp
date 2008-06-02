/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "PlotWindowAdapter.h"

using namespace std;

PlotWindowAdapter::PlotWindowAdapter(const string& id, const string& windowName, QWidget* parent) :
   PlotWindowImp(id, windowName, parent)
{
}

PlotWindowAdapter::~PlotWindowAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

const string& PlotWindowAdapter::getObjectType() const
{
   static string type("PlotWindowAdapter");
   return type;
}

bool PlotWindowAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "PlotWindow"))
   {
      return true;
   }

   return PlotWindowImp::isKindOf(className);
}
