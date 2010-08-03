/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "TextAdapter.h"

using namespace std;

TextAdapter::TextAdapter(PlotViewImp* pPlot, bool bPrimary) :
   TextImp(pPlot, bPrimary)
{
}

TextAdapter::TextAdapter(PlotViewImp* pPlot, bool bPrimary, const LocationType& point) :
   TextImp(pPlot, bPrimary, point)
{
}

TextAdapter::TextAdapter(PlotViewImp* pPlot, bool bPrimary, double dX, double dY) :
   TextImp(pPlot, bPrimary, dX, dY)
{
}

TextAdapter::~TextAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

const string& TextAdapter::getObjectType() const
{
   static string type("TextAdapter");
   return type;
}

bool TextAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "Text"))
   {
      return true;
   }

   return TextImp::isKindOf(className);
}
