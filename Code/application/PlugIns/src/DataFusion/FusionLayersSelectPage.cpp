/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QTreeWidget>

#include "AppVerify.h"
#include "FusionLayersSelectPage.h"
#include "GraphicLayer.h"
#include "Layer.h"
#include "LayerList.h"
#include "Slot.h"
#include "SpatialDataView.h"
#include "StringUtilities.h"
#include "TypesFile.h"

#include <functional>
#include <set>
#include <string>
using namespace std;

const QString FusionLayersSelectPage::LAYER_NAME_COLUMN = "Layer Name";
const QString FusionLayersSelectPage::LAYER_TYPE_COLUMN = "Layer Type";

FusionLayersSelectPage::FusionLayersSelectPage(QWidget* pParent) :
   FusionPage(pParent)
{
   QLabel* pLayerLabel = new QLabel("Available Layers for Fusion", this);

   QStringList columnNames;
   columnNames.append(LAYER_NAME_COLUMN);
   columnNames.append(LAYER_TYPE_COLUMN);

   mpLayerView = new QTreeWidget(this);
   mpLayerView->setColumnCount(columnNames.count());
   mpLayerView->setHeaderLabels(columnNames);
   mpLayerView->setSortingEnabled(true);
   mpLayerView->setSelectionMode(QAbstractItemView::ExtendedSelection);
   mpLayerView->setRootIsDecorated(false);

   QHeaderView* pHeader = mpLayerView->header();
   if (pHeader != NULL)
   {
      pHeader->setSortIndicatorShown(true);
      pHeader->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
      pHeader->setStretchLastSection(false);
      pHeader->resizeSection(0, 250);
   }

   // Layout
   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(5);
   pLayout->addWidget(pLayerLabel, 0, Qt::AlignLeft);
   pLayout->addWidget(mpLayerView, 10);

   // Connections
   connect(mpLayerView, SIGNAL(itemSelectionChanged()), this, SIGNAL(modified()));
}

FusionLayersSelectPage::~FusionLayersSelectPage()
{
   setViews(NULL, NULL);
}

void FusionLayersSelectPage::setViews(SpatialDataView* pPrimary, SpatialDataView* pSecondary)
{
   vector<Layer*> layers;

   vector<Layer*>::iterator it;
   SpatialDataView* pOldPrimaryView = getPrimaryView();
   if (pOldPrimaryView != NULL)
   {
      LayerList* pLayerList = pOldPrimaryView->getLayerList();
      if (pLayerList != NULL)
      {
         pLayerList->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &FusionLayersSelectPage::layerListDeleted));
         pLayerList->detach(SIGNAL_NAME(Subject, Modified), Slot(this, &FusionLayersSelectPage::layerListModified));
      }
   }

   FusionPage::setViews(pPrimary, pSecondary);

   if (pPrimary != NULL)
   {
      LayerList* pLayerList = pPrimary->getLayerList();
      if (pLayerList != NULL)
      {
         pLayerList->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &FusionLayersSelectPage::layerListDeleted));
         pLayerList->attach(SIGNAL_NAME(Subject, Modified), Slot(this, &FusionLayersSelectPage::layerListModified));
      }
   }
}

struct LayerSetBuilder : unary_function<Layer*, bool>
{
   explicit LayerSetBuilder(set<Layer*>& emptySet) : mSet(emptySet)
   {
   }

   bool operator()(Layer* pLayer)
   {
      if (pLayer != NULL)
      {
         mSet.insert(pLayer);
      }
      return true;
   }

   const set<Layer*>& getSet() const
   {
      return mSet;
   }

private:
   set<Layer*>& mSet;
};

bool FusionLayersSelectPage::areAllSelectedLayersAvailable() const
{
   vector<Layer*> currentlySelectedLayers = getSelectedLayers(); // layers selected as of now
   vector<Layer*> layersSelectedByUser = mSelectedLayers; // layers selected when the page changed

   set<Layer*> allLayers;

   LayerSetBuilder functor(allLayers);
   std::for_each(currentlySelectedLayers.begin(), currentlySelectedLayers.end(), functor);
   std::for_each(layersSelectedByUser.begin(), layersSelectedByUser.end(), functor);

   return allLayers.size() == layersSelectedByUser.size() && allLayers.size() == currentlySelectedLayers.size();
}

vector<Layer*> FusionLayersSelectPage::getSelectedLayers() const
{
   vector<Layer*> layers;

   for (LayerMap::const_iterator it = mLayers.begin(); it != mLayers.end(); ++it)
   {
      if (mpLayerView->isItemSelected(it.value()) == true)
      {
         layers.push_back(it.key());
      }
   }
   return layers;
}

bool FusionLayersSelectPage::isValid() const
{
   return true;
}

void FusionLayersSelectPage::hideEvent(QHideEvent* pEvt)
{
   mSelectedLayers = getSelectedLayers();
   FusionPage::hideEvent(pEvt);
}

void FusionLayersSelectPage::addLayerToGui(Layer* pLayer)
{
   VERIFYNRV(pLayer != NULL);

   if (dynamic_cast<GraphicLayer*>(pLayer) != NULL)
   {
      pLayer->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &FusionLayersSelectPage::layerDeleted));
      pLayer->attach(SIGNAL_NAME(Subject, Modified), Slot(this, &FusionLayersSelectPage::layerModified));

      string layerName = pLayer->getName();
      LayerType eLayerType = pLayer->getLayerType();
      string layerType = StringUtilities::toDisplayString(eLayerType);
      QTreeWidgetItem* pItem = new QTreeWidgetItem(mpLayerView);
      pItem->setText(0, QString::fromStdString(layerName));
      pItem->setText(1, QString::fromStdString(layerType));
      mLayers.insert(pLayer, pItem);
   }
}

void FusionLayersSelectPage::attached(Subject& subject, const string& signal, const Slot& slot)
{
   layerListAttached(subject, signal, boost::any());
}

void FusionLayersSelectPage::detached(Subject& subject, const string& signal, const Slot& slot)
{
   if (slot == Slot(this, &FusionLayersSelectPage::layerListDeleted))
   {
      layerListDeleted(subject, signal, boost::any());
   }
   else if (slot == Slot(this, &FusionLayersSelectPage::layerDeleted))
   {
      layerDeleted(subject, signal, boost::any());
   }
}

void FusionLayersSelectPage::layerListAttached(Subject& subject, const string& signal, const boost::any& v)
{
   LayerList* pLayerList = dynamic_cast<LayerList*>(&subject);
   if (pLayerList != NULL)
   {
      vector<Layer*> layerListLayers;
      pLayerList->getLayers(layerListLayers);

      for (vector<Layer*>::iterator it = layerListLayers.begin(); it != layerListLayers.end(); ++it)
      {
         LayerMap::iterator found = mLayers.find(*it);
         if (found == mLayers.end())
         {
            addLayerToGui(*it);
         }
      }
   }
}

void FusionLayersSelectPage::layerListModified(Subject& subject, const string& signal, const boost::any& v)
{
   LayerList* pLayerList = dynamic_cast<LayerList*>(&subject);
   if (pLayerList != NULL)
   {
      vector<Layer*> layerListLayers;
      pLayerList->getLayers(layerListLayers);
      // for each layer list layer, check map. If not there, add it.
      for (vector<Layer*>::iterator it = layerListLayers.begin(); it != layerListLayers.end(); ++it)
      {
         LayerMap::iterator found = mLayers.find(*it);
         if (found == mLayers.end())
         {
            addLayerToGui(*it);
         }
      }
   }
}

void FusionLayersSelectPage::layerListDeleted(Subject& subject, const string& signal, const boost::any& v)
{
   LayerList* pLayerList = dynamic_cast<LayerList*>(&subject);
   if (pLayerList != NULL)
   {
      vector<Layer*> pLayers;
      pLayerList->getLayers(pLayers);
      for (vector<Layer*>::iterator it = pLayers.begin(); it != pLayers.end(); ++it)
      {
         Layer* pLayer = *it;
         if (pLayer != NULL)
         {
            pLayer->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &FusionLayersSelectPage::layerDeleted));
            pLayer->detach(SIGNAL_NAME(Subject, Modified), Slot(this, &FusionLayersSelectPage::layerModified));
         }
      }
   }
}

void FusionLayersSelectPage::layerModified(Subject& subject, const string& signal, const boost::any& v)
{
   Layer* pLayer = dynamic_cast<Layer*>(&subject);
   if (pLayer != NULL)
   {
      LayerMap::iterator it = mLayers.find(pLayer);
      if (it != mLayers.end())
      {
         string newName = pLayer->getName();
         it.value()->setText(0, newName.c_str());
      }
   }
}

void FusionLayersSelectPage::layerDeleted(Subject& subject, const string& signal, const boost::any& v)
{
   Layer* pLayer = dynamic_cast<Layer*>(&subject);
   if (pLayer != NULL)
   {
      LayerMap::iterator it = mLayers.find(pLayer);
      if (it != mLayers.end())
      {
         delete it.value(); //remove the QTreeWidgetItem
         mLayers.remove(pLayer);
      }
   }
}
