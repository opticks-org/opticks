/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AnnotationElementAdapter.h"

using namespace std;

AnnotationElementAdapter::AnnotationElementAdapter(const DataDescriptorImp& descriptor, const string& id) :
   AnnotationElementImp(descriptor, id)
{
}

AnnotationElementAdapter::~AnnotationElementAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

const string& AnnotationElementAdapter::getObjectType() const
{
   static string type("AnnotationElementAdapter");
   return type;
}

bool AnnotationElementAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "AnnotationElement"))
   {
      return true;
   }

   return AnnotationElementImp::isKindOf(className);
}
