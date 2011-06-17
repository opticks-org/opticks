/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "LayerUndo.h"
#include "PseudocolorLayerAdapter.h"
#include "RasterElement.h"
#include "SpatialDataView.h"

using namespace std;

PseudocolorLayerAdapter::PseudocolorLayerAdapter(const string& id, const string& layerName,
                                                 RasterElement* pRasterElement) :
   PseudocolorLayerImp(id, layerName, pRasterElement)
{
}

PseudocolorLayerAdapter::~PseudocolorLayerAdapter()
{
   SpatialDataView* pView = dynamic_cast<SpatialDataView*>(getView());
   if (pView != NULL)
   {
      pView->addUndoAction(new DeleteLayer(pView, this));
   }

   notify(SIGNAL_NAME(Subject, Deleted));
}

const string& PseudocolorLayerAdapter::getObjectType() const
{
   static string type("PseudocolorLayerAdapter");
   return type;
}

bool PseudocolorLayerAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "PseudocolorLayer"))
   {
      return true;
   }

   return PseudocolorLayerImp::isKindOf(className);
}

Layer* PseudocolorLayerAdapter::copy(const string& layerName, bool bCopyElement, DataElement* pParent) const
{
   // Get the layer name
   string name = layerName;
   if (name.empty() == true)
   {
      name = getName();
   }

   // Get the raster element
   RasterElement* pRasterElement = NULL;

   RasterElement* pCurrentRasterElement = dynamic_cast<RasterElement*>(getDataElement());
   if (bCopyElement == true && pCurrentRasterElement != NULL)
   {
      pRasterElement = dynamic_cast<RasterElement*>(pCurrentRasterElement->copy(name, pParent));
   }
   else
   {
      pRasterElement = pCurrentRasterElement;
   }

   if (pRasterElement == NULL)
   {
      return NULL;
   }

   // Create the new layer
   PseudocolorLayerAdapter* pLayer = new PseudocolorLayerAdapter(SessionItemImp::generateUniqueId(),
      name, pRasterElement);
   if (pLayer != NULL)
   {
      *pLayer = *this;
   }

   return pLayer;
}
