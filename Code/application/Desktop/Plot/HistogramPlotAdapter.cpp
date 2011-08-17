/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "HistogramPlotAdapter.h"

using namespace std;

HistogramPlotAdapter::HistogramPlotAdapter(const string& id, const string& viewName, QGLContext* pDrawContext,
                                           QWidget* pParent) :
   HistogramPlotImp(id, viewName, pDrawContext, pParent)
{
}

HistogramPlotAdapter::~HistogramPlotAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

const string& HistogramPlotAdapter::getObjectType() const
{
   static string type("HistogramPlotAdapter");
   return type;
}

bool HistogramPlotAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "HistogramPlot"))
   {
      return true;
   }

   return HistogramPlotImp::isKindOf(className);
}
