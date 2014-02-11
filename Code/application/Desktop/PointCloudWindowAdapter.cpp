/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "PointCloudWindowAdapter.h"

using namespace std;

PointCloudWindowAdapter::PointCloudWindowAdapter(const string& id, const string& windowName, QWidget* parent) :
   PointCloudWindowImp(id, windowName, parent)
{
}

PointCloudWindowAdapter::~PointCloudWindowAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

const string& PointCloudWindowAdapter::getObjectType() const
{
   static string type("PointCloudWindowAdapter");
   return type;
}

bool PointCloudWindowAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "PointCloudWindow"))
   {
      return true;
   }

   return PointCloudWindowImp::isKindOf(className);
}
