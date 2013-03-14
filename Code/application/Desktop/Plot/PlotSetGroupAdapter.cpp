/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "PlotSetGroupAdapter.h"

PlotSetGroupAdapter::PlotSetGroupAdapter(QWidget* pParent) :
   PlotSetGroupImp(pParent)
{}

PlotSetGroupAdapter::~PlotSetGroupAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

const std::string& PlotSetGroupAdapter::getObjectType() const
{
   static std::string sType("PlotSetGroupAdapter");
   return sType;
}

bool PlotSetGroupAdapter::isKindOf(const std::string& className) const
{
   if ((className == getObjectType()) || (className == "PlotSetGroup"))
   {
      return true;
   }

   return PlotSetGroupImp::isKindOf(className);
}
