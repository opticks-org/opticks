/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "CurveCollectionAdapter.h"

using namespace std;

CurveCollectionAdapter::CurveCollectionAdapter(PlotViewImp* pPlot, bool bPrimary) :
   CurveCollectionImp(pPlot, bPrimary)
{
}

CurveCollectionAdapter::~CurveCollectionAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

const string& CurveCollectionAdapter::getObjectType() const
{
   static string type("CurveCollectionAdapter");
   return type;
}

bool CurveCollectionAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "CurveCollection"))
   {
      return true;
   }

   return CurveCollectionImp::isKindOf(className);
}
