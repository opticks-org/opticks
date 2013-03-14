/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "TiePointListAdapter.h"

using namespace std;

TiePointListAdapter::TiePointListAdapter(const DataDescriptorImp& descriptor, const string& id) :
   TiePointListImp(descriptor, id)
{
}

TiePointListAdapter::~TiePointListAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

// TypeAwareObject
const string& TiePointListAdapter::getObjectType() const
{
   static string sType("TiePointListAdapter");
   return sType;
}

bool TiePointListAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "TiePointList"))
   {
      return true;
   }

   return TiePointListImp::isKindOf(className);
}
