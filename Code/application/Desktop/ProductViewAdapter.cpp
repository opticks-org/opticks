/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AnnotationLayer.h"
#include "ProductViewAdapter.h"

using namespace std;

ProductViewAdapter::ProductViewAdapter(const string& id, const string& viewName, QGLContext* drawContext,
                                       QWidget* parent) :
   ProductViewImp(id, viewName, drawContext, parent)
{
}

ProductViewAdapter::~ProductViewAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

const string& ProductViewAdapter::getObjectType() const
{
   static string type("ProductViewAdapter");
   return type;
}

bool ProductViewAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "ProductView"))
   {
      return true;
   }

   return ProductViewImp::isKindOf(className);
}
