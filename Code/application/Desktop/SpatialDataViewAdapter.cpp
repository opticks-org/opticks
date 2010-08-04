/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "SpatialDataViewAdapter.h"

using namespace std;

SpatialDataViewAdapter::SpatialDataViewAdapter(const string& id, const string& viewName,
                                               QGLContext* drawContext, QWidget* parent) :
   SpatialDataViewImp(id, viewName, drawContext, parent)
{
}

SpatialDataViewAdapter::~SpatialDataViewAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

const string& SpatialDataViewAdapter::getObjectType() const
{
   static string type("SpatialDataViewAdapter");
   return type;
}

bool SpatialDataViewAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "SpatialDataView"))
   {
      return true;
   }

   return SpatialDataViewImp::isKindOf(className);
}
