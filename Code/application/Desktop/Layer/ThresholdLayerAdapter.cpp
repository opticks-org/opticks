/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "LayerUndo.h"
#include "RasterElement.h"
#include "SpatialDataView.h"
#include "ThresholdLayerAdapter.h"

using namespace std;

ThresholdLayerAdapter::ThresholdLayerAdapter(const string& id, const string& layerName,
                                             RasterElement* pRasterElement) :
   ThresholdLayerImp(id, layerName, pRasterElement)
{
}

ThresholdLayerAdapter::~ThresholdLayerAdapter()
{
   SpatialDataView* pView = dynamic_cast<SpatialDataView*>(getView());
   if (pView != NULL)
   {
      pView->addUndoAction(new DeleteLayer(pView, this));
   }

   notify(SIGNAL_NAME(Subject, Deleted));
}

const string& ThresholdLayerAdapter::getObjectType() const
{
   static string type("ThresholdLayerAdapter");
   return type;
}

bool ThresholdLayerAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "ThresholdLayer"))
   {
      return true;
   }

   return ThresholdLayerImp::isKindOf(className);
}

Layer* ThresholdLayerAdapter::copy(const string& layerName, bool bCopyElement, DataElement* pParent) const
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
   ThresholdLayerAdapter* pLayer = new ThresholdLayerAdapter(SessionItemImp::generateUniqueId(), name, pRasterElement);
   if (pLayer != NULL)
   {
      *pLayer = *this;
   }

   return pLayer;
}
