/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AnnotationElementImp.h"

using namespace std;

AnnotationElementImp::AnnotationElementImp(const DataDescriptorImp& descriptor, const string& id) :
   GraphicElementImp(descriptor, id)
{
}

AnnotationElementImp::~AnnotationElementImp()
{
}

const string& AnnotationElementImp::getObjectType() const
{
   static string sType("AnnotationElementImp");
   return sType;
}

bool AnnotationElementImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "AnnotationElement"))
   {
      return true;
   }

   return GraphicElementImp::isKindOf(className);
}

void AnnotationElementImp::getElementTypes(vector<string>& classList)
{
   classList.push_back("AnnotationElement");
   GraphicElementImp::getElementTypes(classList);
}

bool AnnotationElementImp::isKindOfElement(const string& className)
{
   if ((className == "AnnotationElementImp") || (className == "AnnotationElement"))
   {
      return true;
   }

   return GraphicElementImp::isKindOfElement(className);
}
