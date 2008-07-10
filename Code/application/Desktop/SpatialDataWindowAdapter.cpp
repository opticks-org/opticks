/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "SpatialDataWindowAdapter.h"

using namespace std;

SpatialDataWindowAdapter::SpatialDataWindowAdapter(const string& id, const string& windowName, QWidget* parent) :
   SpatialDataWindowImp(id, windowName, parent)
{
}

SpatialDataWindowAdapter::~SpatialDataWindowAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

const string& SpatialDataWindowAdapter::getObjectType() const
{
   static string type("SpatialDataWindowAdapter");
   return type;
}

bool SpatialDataWindowAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "SpatialDataWindow"))
   {
      return true;
   }

   return SpatialDataWindowImp::isKindOf(className);
}
