/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "CurveAdapter.h"

using namespace std;

CurveAdapter::CurveAdapter(PlotViewImp* pPlot, bool bPrimary) :
   CurveImp(pPlot, bPrimary)
{
}

CurveAdapter::~CurveAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

const string& CurveAdapter::getObjectType() const
{
   static string type("CurveAdapter");
   return type;
}

bool CurveAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "Curve"))
   {
      return true;
   }

   return CurveImp::isKindOf(className);
}
