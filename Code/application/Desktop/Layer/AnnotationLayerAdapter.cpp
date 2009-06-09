/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AnnotationElement.h"
#include "AnnotationLayerAdapter.h"
#include "LayerUndo.h"
#include "SpatialDataView.h"

using namespace std;

AnnotationLayerAdapter::AnnotationLayerAdapter(const string& id, const string& layerName, AnnotationElement* pAnno) :
   AnnotationLayerImp(id, layerName, pAnno)
{
}

AnnotationLayerAdapter::~AnnotationLayerAdapter()
{
   SpatialDataView* pView = dynamic_cast<SpatialDataView*>(getView());
   if (pView != NULL)
   {
      pView->addUndoAction(new DeleteLayer(pView, this));
   }

   notify(SIGNAL_NAME(Subject, Deleted));
}

const string& AnnotationLayerAdapter::getObjectType() const
{
   static string type("AnnotationLayerAdapter");
   return type;
}

bool AnnotationLayerAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "AnnotationLayer"))
   {
      return true;
   }

   return AnnotationLayerImp::isKindOf(className);
}

Layer* AnnotationLayerAdapter::copy(const string& layerName, bool bCopyElement, DataElement* pParent) const
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
   AnnotationLayerAdapter* pLayer = new AnnotationLayerAdapter(SessionItemImp::generateUniqueId(),
      name, pAnnotation);
   if (pLayer != NULL)
   {
      *pLayer = *this;
   }

   return pLayer;
}
