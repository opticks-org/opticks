/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "CartesianPlotAdapter.h"

using namespace std;

CartesianPlotAdapter::CartesianPlotAdapter(const string& id, const string& viewName, QGLContext* pDrawContext,
                                           QWidget* pParent) :
   CartesianPlotImp(id, viewName, pDrawContext, pParent)
{
}

CartesianPlotAdapter::~CartesianPlotAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

const string& CartesianPlotAdapter::getObjectType() const
{
   static string type("CartesianPlotAdapter");
   return type;
}

bool CartesianPlotAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "CartesianPlot"))
   {
      return true;
   }

   return CartesianPlotImp::isKindOf(className);
}
