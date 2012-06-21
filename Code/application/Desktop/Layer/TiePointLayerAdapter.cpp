/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "LayerUndo.h"
#include "SpatialDataView.h"
#include "TiePointLayerAdapter.h"
#include "TiePointList.h"

#include <string>
using namespace std;

TiePointLayerAdapter::TiePointLayerAdapter(const string& id, const string& layerName, TiePointList* pElement) :
   TiePointLayerImp(id, layerName, pElement)
{
}

TiePointLayerAdapter::~TiePointLayerAdapter()
{
   SpatialDataView* pView = dynamic_cast<SpatialDataView*>(getView());
   if (pView != NULL)
   {
      pView->addUndoAction(new DeleteLayer(pView, this));
   }

   notify(SIGNAL_NAME(Subject, Deleted));
}

// TypeAwareObject
const string& TiePointLayerAdapter::getObjectType() const
{
   static string type("TiePointLayerAdapter");
   return type;
}

bool TiePointLayerAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "TiePointLayer"))
   {
      return true;
   }

   return TiePointLayerImp::isKindOf(className);
}

// Layer
Layer* TiePointLayerAdapter::copy(const string& layerName, bool bCopyElement, DataElement* pParent) const
{
   // Get the layer name
   string name = layerName;
   if (name.empty() == true)
   {
      name = getName();
   }

   // Get the GCP list
   TiePointList* pList = NULL;

   TiePointList* pCurrentList = dynamic_cast<TiePointList*>(getDataElement());
   if (bCopyElement == true && pCurrentList != NULL)
   {
      pList = dynamic_cast<TiePointList*>(pCurrentList->copy(name, pParent));
   }
   else
   {
      pList = pCurrentList;
   }

   if (pList == NULL)
   {
      return NULL;
   }

   // Create the new layer
   TiePointLayerAdapter* pLayer = new TiePointLayerAdapter(SessionItemImp::generateUniqueId(), name, pList);
   if (pLayer != NULL)
   {
      *pLayer = *this;
   }

   return pLayer;
}
