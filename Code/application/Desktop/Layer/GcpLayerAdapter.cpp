/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "GcpLayerAdapter.h"
#include "GcpList.h"
#include "LayerUndo.h"
#include "SpatialDataView.h"

using namespace std;

GcpLayerAdapter::GcpLayerAdapter(const string& id, const string& layerName, GcpList* pGcpList) :
   GcpLayerImp(id, layerName, pGcpList)
{
}

GcpLayerAdapter::~GcpLayerAdapter()
{
   SpatialDataView* pView = dynamic_cast<SpatialDataView*>(getView());
   if (pView != NULL)
   {
      pView->addUndoAction(new DeleteLayer(pView, this));
   }

   notify(SIGNAL_NAME(Subject, Deleted));
}

// TypeAwareObject
const string& GcpLayerAdapter::getObjectType() const
{
   static string type("GcpLayerAdapter");
   return type;
}

bool GcpLayerAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "GcpLayer"))
   {
      return true;
   }

   return GcpLayerImp::isKindOf(className);
}

// Layer
Layer* GcpLayerAdapter::copy(const string& layerName, bool bCopyElement, DataElement* pParent) const
{
   // Get the layer name
   string name = layerName;
   if (name.empty() == true)
   {
      name = getName();
   }

   // Get the GCP list
   GcpList* pGcpList = NULL;

   GcpList* pCurrentGcpList = dynamic_cast<GcpList*>(getDataElement());
   if (bCopyElement == true && pCurrentGcpList != NULL)
   {
      pGcpList = dynamic_cast<GcpList*>(pCurrentGcpList->copy(name, pParent));
   }
   else
   {
      pGcpList = pCurrentGcpList;
   }

   if (pGcpList == NULL)
   {
      return NULL;
   }

   // Create the new layer
   GcpLayerAdapter* pLayer = new GcpLayerAdapter(SessionItemImp::generateUniqueId(), name, pGcpList);
   if (pLayer != NULL)
   {
      *pLayer = *this;
   }

   return pLayer;
}
