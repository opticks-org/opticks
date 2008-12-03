/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "GraphicElementAdapter.h"

using namespace std;

GraphicElementAdapter::GraphicElementAdapter(const DataDescriptorImp& descriptor, const string& id) :
   GraphicElementImp(descriptor, id)
{
}

GraphicElementAdapter::~GraphicElementAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

const string& GraphicElementAdapter::getObjectType() const
{
   static string sType = "GraphicElementAdapter";
   return sType;
}

bool GraphicElementAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "GraphicElement"))
   {
      return true;
   }

   return GraphicElementImp::isKindOf(className);
}
