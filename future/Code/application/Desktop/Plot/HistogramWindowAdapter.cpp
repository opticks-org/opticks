/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "HistogramWindowAdapter.h"

using namespace std;

HistogramWindowAdapter::HistogramWindowAdapter(const string& id, QWidget* parent) :
   HistogramWindowImp(id, "Histogram Window", parent)
{
}

HistogramWindowAdapter::~HistogramWindowAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

// TypeAwareObject
const string& HistogramWindowAdapter::getObjectType() const
{
   static string type("HistogramWindowAdapter");
   return type;
}

bool HistogramWindowAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "HistogramWindow"))
   {
      return true;
   }

   return HistogramWindowImp::isKindOf(className);
}
