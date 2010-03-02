/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AnnotationLayer.h"
#include "AoiLayer.h"
#include "AoiLayerUndo.h"
#include "DataDescriptor.h"
#include "DataDescriptorImp.h"
#include "GcpLayer.h"
#include "GcpLayerUndo.h"
#include "GcpList.h"
#include "GcpListUndo.h"
#include "GraphicElement.h"
#include "GraphicLayerUndo.h"
#include "LatLonLayer.h"
#include "LatLonLayerUndo.h"
#include "Layer.h"
#include "LayerImp.h"
#include "LayerList.h"
#include "LayerUndo.h"
#include "ModelServices.h"
#include "PseudocolorLayer.h"
#include "PseudocolorLayerUndo.h"
#include "RasterLayer.h"
#include "RasterLayerUndo.h"
#include "SessionManager.h"
#include "SpatialDataView.h"
#include "SpatialDataViewImp.h"
#include "ThresholdLayer.h"
#include "ThresholdLayerUndo.h"
#include "TiePointLayer.h"
#include "TiePointLayerUndo.h"
#include "TiePointList.h"
#include "TiePointListUndo.h"

#include <string>
using namespace std;

///////////////
// ShowLayer //
///////////////

ShowLayer::ShowLayer(Layer* pLayer) :
   UndoAction(pLayer)
{
   setText("Show Layer");
}

void ShowLayer::executeUndo()
{
   Layer* pLayer = dynamic_cast<Layer*>(getSessionItem());
   if (pLayer != NULL)
   {
      SpatialDataView* pView = dynamic_cast<SpatialDataView*>(pLayer->getView());
      if (pView != NULL)
      {
         pView->hideLayer(pLayer);
      }
   }
}

void ShowLayer::executeRedo()
{
   Layer* pLayer = dynamic_cast<Layer*>(getSessionItem());
   if (pLayer != NULL)
   {
      SpatialDataView* pView = dynamic_cast<SpatialDataView*>(pLayer->getView());
      if (pView != NULL)
      {
         pView->showLayer(pLayer);
      }
   }
}

///////////////
// HideLayer //
///////////////

HideLayer::HideLayer(Layer* pLayer) :
   UndoAction(pLayer)
{
   setText("Hide Layer");
}

void HideLayer::executeUndo()
{
   Layer* pLayer = dynamic_cast<Layer*>(getSessionItem());
   if (pLayer != NULL)
   {
      SpatialDataView* pView = dynamic_cast<SpatialDataView*>(pLayer->getView());
      if (pView != NULL)
      {
         pView->showLayer(pLayer);
      }
   }
}

void HideLayer::executeRedo()
{
   Layer* pLayer = dynamic_cast<Layer*>(getSessionItem());
   if (pLayer != NULL)
   {
      SpatialDataView* pView = dynamic_cast<SpatialDataView*>(pLayer->getView());
      if (pView != NULL)
      {
         pView->hideLayer(pLayer);
      }
   }
}

//////////////////
// SetLayerName //
//////////////////

SetLayerName::SetLayerName(Layer* pLayer, const string& oldName, const string& newName) :
   UndoAction(pLayer),
   mOldName(oldName),
   mNewName(newName)
{
   setText("Rename Layer");
}

void SetLayerName::executeUndo()
{
   renameLayer(mOldName);
}

void SetLayerName::executeRedo()
{
   renameLayer(mNewName);
}

void SetLayerName::renameLayer(const string& name)
{
   Layer* pLayer = dynamic_cast<Layer*>(getSessionItem());
   if (pLayer == NULL)
   {
      return;
   }

   bool bSuccess = false;

   SpatialDataView* pView = dynamic_cast<SpatialDataView*>(pLayer->getView());
   if (pView != NULL)
   {
      LayerList* pLayerList = pView->getLayerList();
      if (pLayerList != NULL)
      {
         bSuccess = pLayerList->renameLayer(pLayer, name);
      }
   }

   if (bSuccess == false)
   {
      LayerImp* pLayerImp = dynamic_cast<LayerImp*>(pLayer);
      if (pLayerImp != NULL)
      {
         pLayerImp->setName(name);
      }
   }
}

//////////////////////////
// SetLayerDisplayIndex //
//////////////////////////

SetLayerDisplayIndex::SetLayerDisplayIndex(Layer* pLayer, int oldIndex, int newIndex) :
   UndoAction(pLayer),
   mOldIndex(oldIndex),
   mNewIndex(newIndex)
{
   setText("Set Layer Display Index");
}

void SetLayerDisplayIndex::executeUndo()
{
   setLayerDisplayIndex(mOldIndex);
}

void SetLayerDisplayIndex::executeRedo()
{
   setLayerDisplayIndex(mNewIndex);
}

void SetLayerDisplayIndex::setLayerDisplayIndex(int index)
{
   Layer* pLayer = dynamic_cast<Layer*>(getSessionItem());
   if (pLayer != NULL)
   {
      SpatialDataView* pView = dynamic_cast<SpatialDataView*>(pLayer->getView());
      if (pView != NULL)
      {
         pView->setLayerDisplayIndex(pLayer, index);
      }
   }
}

////////////////////////
// CreateDestroyLayer //
////////////////////////

CreateDestroyLayer::CreateDestroyLayer(SpatialDataView* pView, Layer* pLayer) :
   UndoAction(pView),
   mpLayerMemento(NULL),
   mpElementDescriptor(NULL)
{
   if (pLayer != NULL)
   {
      mLayerId = pLayer->getId();

      AnnotationLayer* pAnnotationLayer = dynamic_cast<AnnotationLayer*>(pLayer);
      if (pAnnotationLayer != NULL)
      {
         mpLayerMemento = new AnnotationLayerMemento(pAnnotationLayer);
      }

      AoiLayer* pAoiLayer = dynamic_cast<AoiLayer*>(pLayer);
      if (pAoiLayer != NULL)
      {
         mpLayerMemento = new AoiLayerMemento(pAoiLayer);
      }

      GcpLayer* pGcpLayer = dynamic_cast<GcpLayer*>(pLayer);
      if (pGcpLayer != NULL)
      {
         mpLayerMemento = new GcpLayerMemento(pGcpLayer);
      }

      LatLonLayer* pLatLonLayer = dynamic_cast<LatLonLayer*>(pLayer);
      if (pLatLonLayer != NULL)
      {
         mpLayerMemento = new LatLonLayerMemento(pLatLonLayer);
      }

      PseudocolorLayer* pPseudocolorLayer = dynamic_cast<PseudocolorLayer*>(pLayer);
      if (pPseudocolorLayer != NULL)
      {
         mpLayerMemento = new PseudocolorLayerMemento(pPseudocolorLayer);
      }

      RasterLayer* pRasterLayer = dynamic_cast<RasterLayer*>(pLayer);
      if (pRasterLayer != NULL)
      {
         mpLayerMemento = new RasterLayerMemento(pRasterLayer);
      }

      ThresholdLayer* pThresholdLayer = dynamic_cast<ThresholdLayer*>(pLayer);
      if (pThresholdLayer != NULL)
      {
         mpLayerMemento = new ThresholdLayerMemento(pThresholdLayer);
      }

      TiePointLayer* pTiePointLayer = dynamic_cast<TiePointLayer*>(pLayer);
      if (pTiePointLayer != NULL)
      {
         mpLayerMemento = new TiePointLayerMemento(pTiePointLayer);
      }

      mLayerName = pLayer->getName();
      mLayerType = pLayer->getLayerType();

      DataElement* pElement = pLayer->getDataElement();
      if (pElement != NULL)
      {
         mElementId = pElement->getId();

         const DataDescriptorImp* pDescriptor = dynamic_cast<const DataDescriptorImp*>(pElement->getDataDescriptor());
         if (pDescriptor != NULL)
         {
            mpElementDescriptor = pDescriptor->copy();

            DataElement* pParent = pDescriptor->getParent();
            if (pParent != NULL)
            {
               mParentId = pParent->getId();
            }
         }
      }
   }
}

CreateDestroyLayer::~CreateDestroyLayer()
{
   if (mpLayerMemento != NULL)
   {
      delete mpLayerMemento;
   }

   DataDescriptorImp* pDescriptorImp = dynamic_cast<DataDescriptorImp*>(mpElementDescriptor);
   if (pDescriptorImp != NULL)
   {
      delete pDescriptorImp;
   }
}

void CreateDestroyLayer::updateSessionItem(const string& oldId, const string& newId)
{
   if (oldId.empty() == false)
   {
      if (oldId == mLayerId)
      {
         mLayerId = newId;
      }

      if (oldId == mElementId)
      {
         mElementId = newId;
      }

      if (oldId == mParentId)
      {
         DataDescriptorImp* pDescriptorImp = dynamic_cast<DataDescriptorImp*>(mpElementDescriptor);
         if (pDescriptorImp != NULL)
         {
            Service<SessionManager> pManager;

            DataElement* pParent = dynamic_cast<DataElement*>(pManager->getSessionItem(newId));
            pDescriptorImp->setParent(pParent);
         }
      }
   }

   UndoAction::updateSessionItem(oldId, newId);
}

void CreateDestroyLayer::createLayer()
{
   SpatialDataView* pView = dynamic_cast<SpatialDataView*>(getSessionItem());
   if (pView == NULL)
   {
      return;
   }

   if (mpElementDescriptor == NULL)
   {
      return;
   }

   const string& elementName = mpElementDescriptor->getName();
   const string& elementType = mpElementDescriptor->getType();
   DataElement* pParent = mpElementDescriptor->getParent();

   Service<ModelServices> pModel;
   DataElement* pElement = pModel->getElement(elementName, elementType, pParent);
   if (pElement == NULL)
   {
      string oldElementId = mElementId;
      mElementId.clear();

      pElement = pModel->createElement(mpElementDescriptor);
      if (pElement != NULL)
      {
         mElementId = pElement->getId();
      }

      emit sessionItemChanged(oldElementId, mElementId);
   }

   if (pElement == NULL)
   {
      return;
   }

   string oldLayerId = mLayerId;
   mLayerId.clear();

   Layer* pLayer = pView->createLayer(mLayerType, pElement, mLayerName);
   if (pLayer != NULL)
   {
      mLayerId = pLayer->getId();

      if (mpLayerMemento != NULL)
      {
         mpLayerMemento->toLayer(pLayer);
      }
   }

   emit sessionItemChanged(oldLayerId, mLayerId);
}

void CreateDestroyLayer::destroyLayer()
{
   SpatialDataViewImp* pView = dynamic_cast<SpatialDataViewImp*>(getSessionItem());
   if (pView != NULL)
   {
      Service<SessionManager> pManager;

      Layer* pLayer = dynamic_cast<Layer*>(pManager->getSessionItem(mLayerId));
      if (pLayer != NULL)
      {
         pView->deleteLayer(pLayer, false);
      }
   }
}

//////////////
// AddLayer //
//////////////

AddLayer::AddLayer(SpatialDataView* pView, Layer* pLayer) :
   CreateDestroyLayer(pView, pLayer)
{
   setText("Add Layer");
}

void AddLayer::executeUndo()
{
   destroyLayer();
}

void AddLayer::executeRedo()
{
   createLayer();
}

/////////////////
// DeleteLayer //
/////////////////

DeleteLayer::DeleteLayer(SpatialDataView* pView, Layer* pLayer) :
   CreateDestroyLayer(pView, pLayer)
{
   setText("Delete Layer");
}

void DeleteLayer::executeUndo()
{
   createLayer();
}

void DeleteLayer::executeRedo()
{
   destroyLayer();
}
