/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "PlotViewAdapter.h"

using namespace std;

PlotViewAdapter::PlotViewAdapter(const string& id, const string& viewName, QGLContext* drawContext,
                                 QWidget* parent) :
   PlotViewImp(id, viewName, drawContext, parent)
{
}

PlotViewAdapter::~PlotViewAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

const string& PlotViewAdapter::getObjectType() const
{
   static string type("PlotViewAdapter");
   return type;
}

bool PlotViewAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "PlotView"))
   {
      return true;
   }

   return PlotViewImp::isKindOf(className);
}
