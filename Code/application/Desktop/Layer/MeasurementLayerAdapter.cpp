/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AnnotationElement.h"
#include "MeasurementLayerAdapter.h"

using namespace std;

MeasurementLayerAdapter::MeasurementLayerAdapter(const string& id, const string& layerName,
                                                 AnnotationElement* pElement) :
   MeasurementLayerImp(id, layerName, pElement)
{
}

MeasurementLayerAdapter::~MeasurementLayerAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

const string& MeasurementLayerAdapter::getObjectType() const
{
   static string sType("MeasurementLayerAdapter");
   return sType;
}

bool MeasurementLayerAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "MeasurementLayer"))
   {
      return true;
   }

   return MeasurementLayerImp::isKindOf(className);
}

Layer* MeasurementLayerAdapter::copy(const string& layerName, bool bCopyElement, DataElement* pParent) const
{
   // Get the layer name
   string name = layerName;
   if (name.empty() == true)
   {
      name = getName();
   }

   // Get the annotation
   AnnotationElement* pAnnotation = NULL;

   AnnotationElement* pCurrentAnnotation = dynamic_cast<AnnotationElement*>(getDataElement());
   if (pCurrentAnnotation != NULL)
   {
      string elementName = name;
      if ((layerName.empty() == true) && (bCopyElement == false) && (pParent == pCurrentAnnotation->getParent()))
      {
         elementName = SessionItemImp::generateUniqueId();
      }

      // Always create a copy of the element since the graphic group has a pointer to the layer
      pAnnotation = dynamic_cast<AnnotationElement*>(pCurrentAnnotation->copy(elementName, pParent));
   }

   if (pAnnotation == NULL)
   {
      return NULL;
   }

   // Create the new layer
   MeasurementLayerAdapter* pLayer = new MeasurementLayerAdapter(SessionItemImp::generateUniqueId(),
      name, pAnnotation);
   if (pLayer != NULL)
   {
      *pLayer = *this;
   }

   return pLayer;
}
