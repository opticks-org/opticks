/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "LayerListAdapter.h"

using namespace std;

LayerListAdapter::LayerListAdapter()
{
}

LayerListAdapter::~LayerListAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

const string& LayerListAdapter::getObjectType() const
{
   static string type("LayerListAdapter");
   return type;
}

bool LayerListAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "LayerList"))
   {
      return true;
   }

   return LayerListImp::isKindOf(className);
}
