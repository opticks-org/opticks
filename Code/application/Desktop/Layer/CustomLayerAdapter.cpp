/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "Any.h"
#include "CustomLayerAdapter.h"
#include "LayerUndo.h"
#include "SpatialDataView.h"

CustomLayerAdapter::CustomLayerAdapter(const std::string& id, const std::string& layerName, Any* pElement) :
   CustomLayerImp(id, layerName, pElement)
{}

CustomLayerAdapter::~CustomLayerAdapter()
{
   SpatialDataView* pView = dynamic_cast<SpatialDataView*>(getView());
   if (pView != NULL)
   {
      pView->addUndoAction(new DeleteLayer(pView, this));
   }

   notify(SIGNAL_NAME(Subject, Deleted));
}

// TypeAwareObject
const std::string& CustomLayerAdapter::getObjectType() const
{
   static std::string type("CustomLayerAdapter");
   return type;
}

bool CustomLayerAdapter::isKindOf(const std::string& className) const
{
   if ((className == getObjectType()) || (className == "CustomLayer"))
   {
      return true;
   }

   return CustomLayerImp::isKindOf(className);
}

// Layer
Layer* CustomLayerAdapter::copy(const std::string& layerName, bool bCopyElement, DataElement* pParent) const
{
   // Get the layer name
   std::string name = layerName;
   if (name.empty() == true)
   {
      name = getName();
   }

   // Get the Any element
   Any* pAny = NULL;

   Any* pCurrentElement = dynamic_cast<Any*>(getDataElement());
   if (bCopyElement == true && pCurrentElement != NULL)
   {
      pAny = dynamic_cast<Any*>(pCurrentElement->copy(name, pParent));
   }
   else
   {
      pAny = pCurrentElement;
   }

   if (pAny == NULL)
   {
      return NULL;
   }

   // Create the new layer
   CustomLayerAdapter* pLayer = new CustomLayerAdapter(SessionItemImp::generateUniqueId(), name, pAny);
   if (pLayer != NULL)
   {
      *pLayer = *this;
   }

   return pLayer;
}
