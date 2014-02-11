/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "PointCloudViewAdapter.h"

using namespace std;

PointCloudViewAdapter::PointCloudViewAdapter(const std::string& id, const std::string& viewName, QGLContext* drawContext,
      QWidget* parent) : PointCloudViewImp(id, viewName, drawContext, parent)
{
}

PointCloudViewAdapter::~PointCloudViewAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

const string& PointCloudViewAdapter::getObjectType() const
{
   static string type("PointCloudViewAdapter");
   return type;
}

bool PointCloudViewAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "PointCloudView"))
   {
      return true;
   }

   return PointCloudViewImp::isKindOf(className);
}
