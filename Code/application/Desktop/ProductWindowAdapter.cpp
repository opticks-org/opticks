/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ProductWindowAdapter.h"

using namespace std;

ProductWindowAdapter::ProductWindowAdapter(const string& id, const string& windowName, QWidget* parent) :
   ProductWindowImp(id, windowName, parent)
{
}

ProductWindowAdapter::~ProductWindowAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

const string& ProductWindowAdapter::getObjectType() const
{
   static string type("ProductWindowAdapter");
   return type;
}

bool ProductWindowAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "ProductWindow"))
   {
      return true;
   }

   return ProductWindowImp::isKindOf(className);
}
