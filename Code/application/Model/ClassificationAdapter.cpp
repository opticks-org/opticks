/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ClassificationAdapter.h"

using namespace std;

ClassificationAdapter::ClassificationAdapter()
{}

ClassificationAdapter::~ClassificationAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

const string& ClassificationAdapter::getObjectType() const
{
   static string sType("ClassificationAdapter");
   return sType;
}

bool ClassificationAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "Classification"))
   {
      return true;
   }

   return ClassificationImp::isKindOf(className);
}
