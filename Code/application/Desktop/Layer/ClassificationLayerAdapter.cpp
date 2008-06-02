/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ClassificationLayerAdapter.h"
#include "AnnotationElement.h"

using namespace std;

ClassificationLayerAdapter::ClassificationLayerAdapter(const string& id, const string& layerName,
                                                       AnnotationElement* pElement) :
   ClassificationLayerImp(id, layerName, pElement)
{
}

ClassificationLayerAdapter::~ClassificationLayerAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

const string& ClassificationLayerAdapter::getObjectType() const
{
   static string type("ClassificationLayerAdapter");
   return type;
}

bool ClassificationLayerAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "ClassificationLayer"))
   {
      return true;
   }

   return ClassificationLayerImp::isKindOf(className);
}

Layer* ClassificationLayerAdapter::copy(const string& layerName, bool bCopyElement, DataElement* pParent) const
{
   // Get the layer name
   string name = layerName;
   if (name.empty() == true)
   {
      name = getName();
   }

   // Get the annotation
   AnnotationElement* pAnnotation = NULL;

   AnnotationElement* pCurrentAnnotation  = dynamic_cast<AnnotationElement*>(getDataElement());
   if (bCopyElement == true && pCurrentAnnotation != NULL)
   {
      pAnnotation = dynamic_cast<AnnotationElement*>(pCurrentAnnotation->copy(name, pParent));
   }
   else
   {
      pAnnotation = pCurrentAnnotation;
   }

   if (pAnnotation == NULL)
   {
      return NULL;
   }

   // Create the new layer
   ClassificationLayerAdapter* pLayer = new ClassificationLayerAdapter(SessionItemImp::generateUniqueId(),
      name, pAnnotation);
   if (pLayer != NULL)
   {
      *pLayer = *this;
   }

   return pLayer;
}
