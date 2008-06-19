/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "PolarPlotAdapter.h"

using namespace std;

PolarPlotAdapter::PolarPlotAdapter(const string& id, const string& viewName, QGLContext* pDrawContext,
                                   QWidget* pParent) :
   PolarPlotImp(id, viewName, pDrawContext, pParent)
{
}

PolarPlotAdapter::~PolarPlotAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

const string& PolarPlotAdapter::getObjectType() const
{
   static string type("PolarPlotAdapter");
   return type;
}

bool PolarPlotAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "PolarPlot"))
   {
      return true;
   }

   return PolarPlotImp::isKindOf(className);
}
