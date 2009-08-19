/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AoiElement.h"
#include "AoiLayerAdapter.h"
#include "LayerUndo.h"
#include "SpatialDataView.h"

using namespace std;

AoiLayerAdapter::AoiLayerAdapter(const string& id, const string& layerName, AoiElement* pAoi) :
   AoiLayerImp(id, layerName, pAoi)
{
}

AoiLayerAdapter::~AoiLayerAdapter()
{
   SpatialDataView* pView = dynamic_cast<SpatialDataView*>(getView());
   if (pView != NULL)
   {
      pView->addUndoAction(new DeleteLayer(pView, this));
   }

   notify(SIGNAL_NAME(Subject, Deleted));
}

const string& AoiLayerAdapter::getObjectType() const
{
   static string type("AoiLayerAdapter");
   return type;
}

bool AoiLayerAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "AoiLayer"))
   {
      return true;
   }

   return AoiLayerImp::isKindOf(className);
}

Layer* AoiLayerAdapter::copy(const string& layerName, bool bCopyElement, DataElement* pParent) const
{
   // Get the layer name
   string name = layerName;
   if (name.empty() == true)
   {
      name = getName();
   }

   // Get the AOI
   AoiElement* pAoi = NULL;

   AoiElement* pCurrentAoi = dynamic_cast<AoiElement*>(getDataElement());
   if (pCurrentAoi != NULL)
   {
      string elementName = name;
      if ((layerName.empty() == true) && (bCopyElement == false) && (pParent == pCurrentAoi->getParent()))
      {
         elementName = SessionItemImp::generateUniqueId();
      }

      // Always create a copy of the element since the graphic group has a pointer to the layer
      pAoi = dynamic_cast<AoiElement*>(pCurrentAoi->copy(elementName, pParent));
   }

   if (pAoi == NULL)
   {
      return NULL;
   }

   // Create the new layer
   AoiLayerAdapter* pLayer = new AoiLayerAdapter(SessionItemImp::generateUniqueId(), name, pAoi);
   if (pLayer != NULL)
   {
      *pLayer = *this;
   }

   return pLayer;
}
