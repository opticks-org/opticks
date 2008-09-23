/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "GraphicElement.h"
#include "GraphicLayerAdapter.h"
#include "LayerUndo.h"
#include "SpatialDataView.h"

using namespace std;

GraphicLayerAdapter::GraphicLayerAdapter(const string& id, const string& layerName, GraphicElement* pElement) :
   GraphicLayerImp(id, layerName, pElement)
{
}

GraphicLayerAdapter::~GraphicLayerAdapter()
{
   SpatialDataView* pView = dynamic_cast<SpatialDataView*>(getView());
   if (pView != NULL)
   {
      pView->addUndoAction(new DeleteLayer(pView, this));
   }

   notify(SIGNAL_NAME(Subject, Deleted));
}

const string& GraphicLayerAdapter::getObjectType() const
{
   static string type("GraphicLayerAdapter");
   return type;
}

bool GraphicLayerAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "GraphicLayer"))
   {
      return true;
   }

   return GraphicLayerImp::isKindOf(className);
}

Layer* GraphicLayerAdapter::copy(const string& layerName, bool bCopyElement, DataElement* pParent) const
{
   // Get the layer name
   string name = layerName;
   if (name.empty() == true)
   {
      name = getName();
   }

   // Get the graphic element
   GraphicElement* pElement = NULL;

   GraphicElement* pCurrentElement  = dynamic_cast<GraphicElement*>(getDataElement());
   if (bCopyElement == true && pCurrentElement != NULL)
   {
      pElement = dynamic_cast<GraphicElement*>(pCurrentElement->copy(name, pParent));
   }
   else
   {
      pElement = pCurrentElement;
   }

   if (pElement == NULL)
   {
      return NULL;
   }

   // Create the new layer
   GraphicLayerAdapter* pLayer = new GraphicLayerAdapter(SessionItemImp::generateUniqueId(), name, pElement);
   if (pLayer != NULL)
   {
      *pLayer = *this;
   }

   return pLayer;
}
