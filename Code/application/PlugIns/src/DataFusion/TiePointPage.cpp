/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QSet>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QMouseEvent>
#include <QtGui/QPushButton>
#include <QtGui/QSpinBox>
#include <QtGui/QTreeWidget>
#include <QtGui/QTreeWidgetItem>

#include "AppAssert.h"
#include "AppVerify.h"
#include "GcpLayer.h"
#include "GcpList.h"
#include "LayerList.h"
#include "ModelServices.h"
#include "ModuleManager.h"
#include "Resource.h"
#include "RasterElement.h"
#include "Slot.h"
#include "SpatialDataView.h"
#include "TiePointList.h"
#include "TiePointPage.h"

using namespace std;

const string TiePointPage::FUSION_GCP_NAME = "Fusion Control Points";

TiePointPage::TiePointPage(QWidget* pParent) :
   FusionPage(pParent),
   TIE_POINT_NAME("Tie Points"),
   mpTiePoints(NULL),
   mpPrimaryGcpLayer(NULL), mpSecondaryGcpLayer(NULL)
{
   // Primary GCPs
   QLabel* pPrimaryGcpLabel = new QLabel("Primary Image GCPs", this);

   QStringList columnNames;
   columnNames.append("GCP Name");
   columnNames.append("Pixel X");
   columnNames.append("Pixel Y");

   mpPrimaryGcps = new QTreeWidget(this);
   mpPrimaryGcps->setColumnCount(columnNames.count());
   mpPrimaryGcps->setHeaderLabels(columnNames);
   mpPrimaryGcps->setSortingEnabled(true);
   mpPrimaryGcps->setRootIsDecorated(false);
   mpPrimaryGcps->setSelectionMode(QAbstractItemView::SingleSelection);

   QHeaderView* pHeader = mpPrimaryGcps->header();
   if (pHeader != NULL)
   {
      pHeader->setDefaultSectionSize(70);
      pHeader->setStretchLastSection(false);
      pHeader->setSortIndicatorShown(true);
      pHeader->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
      pHeader->setStretchLastSection(false);
   }

   mpPrimGcpRemoveButton = new QPushButton("Remove", this);
   mpPrimGcpRemoveButton->setEnabled(false);

   // Secondary GCPs
   QLabel* pSecondaryGcpLabel = new QLabel("Secondary Image GCPs", this);

   mpSecondaryGcps = new QTreeWidget(this);
   mpSecondaryGcps->setColumnCount(columnNames.count());
   mpSecondaryGcps->setHeaderLabels(columnNames);
   mpSecondaryGcps->setSortingEnabled(true);
   mpSecondaryGcps->setRootIsDecorated(false);
   mpSecondaryGcps->setSelectionMode(QAbstractItemView::SingleSelection);

   pHeader = mpSecondaryGcps->header();
   if (pHeader != NULL)
   {
      pHeader->setDefaultSectionSize(70);
      pHeader->setStretchLastSection(false);
      pHeader->setSortIndicatorShown(true);
      pHeader->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
      pHeader->setStretchLastSection(false);
   }

   mpSecGcpRemoveButton = new QPushButton("Remove", this);
   mpSecGcpRemoveButton->setEnabled(false);

   // Tie points
   QLabel* pTiePointPairsLabel = new QLabel("Tie Points", this);

   columnNames.clear();
   columnNames.append("Primary X");
   columnNames.append("Primary Y");
   columnNames.append("Secondary X");
   columnNames.append("Secondary Y");

   mpTiePointView = new QTreeWidget(this);
   mpTiePointView->setColumnCount(columnNames.count());
   mpTiePointView->setHeaderLabels(columnNames);
   mpTiePointView->setSortingEnabled(true);
   mpTiePointView->setRootIsDecorated(false);
   mpTiePointView->setSelectionMode(QAbstractItemView::SingleSelection);

   pHeader = mpTiePointView->header();
   if (pHeader != NULL)
   {
      pHeader->setSortIndicatorShown(true);
      pHeader->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
      pHeader->setStretchLastSection(false);
   }

   mpDeriveButton = new QPushButton("Derive", this);
   mpDeriveButton->setToolTip("Derives a tie point from two selected GCPs. GCPs are deleted when done.");
   mpDeriveButton->setEnabled(false);

   mpTiePointRemoveButton = new QPushButton("Remove", this);
   mpTiePointRemoveButton->setEnabled(false);

   QLabel* pZoomPercentageLabel = new QLabel("Zoom %", this);

   mpZoomBox = new QSpinBox(this);
   mpZoomBox->setMouseTracking(true);
   mpZoomBox->setFocusPolicy(Qt::StrongFocus);
   mpZoomBox->setButtonSymbols(QSpinBox::UpDownArrows);
   mpZoomBox->setMaximum(5000);
   mpZoomBox->setMinimum(10);
   mpZoomBox->setSingleStep(10);
   mpZoomBox->setValue(100);
   mpZoomBox->setSuffix("%");
   mpZoomBox->setEnabled(false);

   // Layout
   QHBoxLayout* pTiePointLayout = new QHBoxLayout();
   pTiePointLayout->setMargin(0);
   pTiePointLayout->setSpacing(5);
   pTiePointLayout->addWidget(mpDeriveButton);
   pTiePointLayout->addWidget(mpTiePointRemoveButton);
   pTiePointLayout->addStretch();
   pTiePointLayout->addWidget(pZoomPercentageLabel);
   pTiePointLayout->addWidget(mpZoomBox);

   QGridLayout* pLayout = new QGridLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(5);
   pLayout->addWidget(pPrimaryGcpLabel, 0, 0);
   pLayout->addWidget(mpPrimaryGcps, 1, 0);
   pLayout->addWidget(mpPrimGcpRemoveButton, 2, 0, Qt::AlignLeft);
   pLayout->addWidget(pSecondaryGcpLabel, 0, 1);
   pLayout->addWidget(mpSecondaryGcps, 1, 1);
   pLayout->addWidget(mpSecGcpRemoveButton, 2, 1, Qt::AlignLeft);
   pLayout->addWidget(pTiePointPairsLabel, 3, 0, 1, 2);
   pLayout->addWidget(mpTiePointView, 4, 0, 1, 2);
   pLayout->addLayout(pTiePointLayout, 5, 0, 1, 2);
   pLayout->setRowStretch(1, 10);
   pLayout->setRowStretch(4, 10);

   // signals and slots connections
   bool cSuccess = connect(mpDeriveButton, SIGNAL(clicked()), this, SLOT(deriveTiePoint()));
   VERIFYNR(cSuccess);

   cSuccess = connect(mpPrimGcpRemoveButton, SIGNAL(clicked()), this, SLOT(removeGcp()));
   VERIFYNR(cSuccess);

   cSuccess = connect(mpSecGcpRemoveButton, SIGNAL(clicked()), this, SLOT(removeGcp()));
   VERIFYNR(cSuccess);

   cSuccess = connect(mpTiePointRemoveButton, SIGNAL(clicked()), this, SLOT(removeTiePoint()));
   VERIFYNR(cSuccess);

   cSuccess = connect(mpPrimaryGcps, SIGNAL(itemSelectionChanged()), this, SLOT(enableGcpActions()));
   VERIFYNR(cSuccess);

   cSuccess = connect(mpSecondaryGcps, SIGNAL(itemSelectionChanged()), this, SLOT(enableGcpActions()));
   VERIFYNR(cSuccess);

   cSuccess = connect(mpTiePointView, SIGNAL(itemSelectionChanged()), this, SLOT(enableTiePointActions()));
   VERIFYNR(cSuccess);

   cSuccess = connect(mpTiePointView, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),
                      this, SLOT(verifyTiePoint(QTreeWidgetItem*, int)));
   VERIFYNR(cSuccess);
}

TiePointPage::~TiePointPage()
{
   SpatialDataView* pPrimaryView = getPrimaryView();
   if (pPrimaryView != NULL && mpPrimaryGcpLayer != NULL)
   {
      GcpLayer* pLayer = mpPrimaryGcpLayer; // save the pointer since detach() resets the member pointer
      // detach before the delete, otherwise the layer will be copied and will stay
      mpPrimaryGcpLayer->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &TiePointPage::gcpLayerDeleted));
      pPrimaryView->deleteLayer(pLayer);
   }
   SpatialDataView* pSecondaryView = getSecondaryView();
   if (pSecondaryView != NULL && mpSecondaryGcpLayer != NULL)
   {
      GcpLayer* pLayer = mpSecondaryGcpLayer; // save the pointer since detach() resets the member pointer
      // detach before the delete, otherwise the layer will be copied and will stay
      mpSecondaryGcpLayer->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &TiePointPage::gcpLayerDeleted));
      pSecondaryView->deleteLayer(pLayer);
   }
   // detach layers and set them to NULL
   setGcpLayer(mpPrimaryGcpLayer, NULL);
   setGcpLayer(mpSecondaryGcpLayer, NULL);
   if (mpTiePoints != NULL)
   {
      mpTiePoints->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &TiePointPage::tiePointListDeleted));
   }
   setViews(NULL, NULL);
}

void TiePointPage::attached(Subject &subject, const string &signal, const Slot &slot)
{
   if (dynamic_cast<GcpLayer*>(&subject) != NULL)
   {
      gcpLayerAttached(subject, signal, boost::any());
   }
   else if (dynamic_cast<GcpList*>(&subject) != NULL)
   {
      gcpListModified(subject, signal, boost::any());
   }
}

void TiePointPage::detached(Subject &subject, const string &signal, const Slot &slot)
{
   if (dynamic_cast<GcpLayer*>(&subject) != NULL)
   {
      gcpLayerDetached(subject, signal, boost::any());
   }
   else if (dynamic_cast<GcpList*>(&subject) != NULL)
   {
      gcpListDetached(subject, signal, boost::any());
   }
   else if (dynamic_cast<TiePointList*>(&subject) != NULL)
   {
      tiePointListDeleted(subject, signal, boost::any());
   }
   else if (dynamic_cast<SpatialDataView*>(&subject) != NULL)
   {
      viewDeleted(subject, signal, boost::any());
   }
}

void TiePointPage::gcpLayerAttached(Subject &subject, const string &signal, const boost::any &v)
{
   GcpLayer* pGcpLayer = dynamic_cast<GcpLayer*>(&subject);
   if (pGcpLayer != NULL)
   {
      DataElement* pElement = pGcpLayer->getDataElement();
      if (pElement != NULL)
      {
         pElement->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &TiePointPage::gcpListDeleted));
         pElement->attach(SIGNAL_NAME(Subject, Modified), Slot(this, &TiePointPage::gcpListModified));
      }
   }
}

void TiePointPage::gcpLayerDetached(Subject &subject, const string &signal, const boost::any &v)
{
   GcpLayer* pGcpLayer = dynamic_cast<GcpLayer*>(&subject);
   if (pGcpLayer != NULL)
   {
      DataElement* pElement = pGcpLayer->getDataElement();
      if (pElement != NULL)
      {
         pElement->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &TiePointPage::gcpListDeleted));
         pElement->detach(SIGNAL_NAME(Subject, Modified), Slot(this, &TiePointPage::gcpListModified));
      }
      if (pGcpLayer == mpPrimaryGcpLayer)
      {
         mpPrimaryGcpLayer = NULL;
      }
      else if (pGcpLayer == mpSecondaryGcpLayer)
      {
         mpSecondaryGcpLayer = NULL;
      }
   }
}

void TiePointPage::gcpLayerDeleted(Subject &subject, const string &signal, const boost::any &v)
{
   GcpLayer* pGcpLayer = dynamic_cast<GcpLayer*>(&subject);
   if (pGcpLayer != NULL)
   {
      if (pGcpLayer == mpPrimaryGcpLayer)
      {
         SpatialDataView* pView = getPrimaryView();
         GcpLayer* pCopy = NULL;
         if (pView != NULL)
         {
            pCopy = static_cast<GcpLayer*>(pGcpLayer->copy(false));
            if (pCopy != NULL)
            {
               pView->addLayer(pCopy);
            }
         }
         setGcpLayer(mpPrimaryGcpLayer, pCopy);
      }
      if (pGcpLayer == mpSecondaryGcpLayer)
      {
         SpatialDataView* pView = getSecondaryView();
         GcpLayer* pCopy = NULL;
         if (pView != NULL)
         {
            pCopy = static_cast<GcpLayer*>(pGcpLayer->copy(false));
            if (pCopy != NULL)
            {
               pView->addLayer(pCopy);
            }
         }
         setGcpLayer(mpSecondaryGcpLayer, pCopy);
      }
   }
}

void TiePointPage::gcpListDeleted(Subject &subject, const string &signal, const boost::any &v)
{
   GcpList* pGcpList = dynamic_cast<GcpList*>(&subject);
   if (pGcpList != NULL)
   {
      GcpList* pPrimaryList = NULL;
      if (mpPrimaryGcpLayer != NULL)
      {
         pPrimaryList = static_cast<GcpList*>(mpPrimaryGcpLayer->getDataElement());
         VERIFYNRV(pPrimaryList != NULL);
      }
      GcpList* pSecondaryList = NULL;
      if (mpSecondaryGcpLayer != NULL)
      {
         pSecondaryList = static_cast<GcpList*>(mpSecondaryGcpLayer->getDataElement());
         VERIFYNRV(pSecondaryList != NULL);
      }

      if (pGcpList == pPrimaryList)
      {
         mpPrimaryGcps->clearSelection();
         mpPrimaryGcps->clear();
      }
      else if (pGcpList == pSecondaryList)
      {
         mpSecondaryGcps->clearSelection();
         mpSecondaryGcps->clear();
      }

      Service<ModelServices> pModel;
      VERIFYNRV(pModel.get() != NULL);
      pModel->destroyElement(mpTiePoints);
      emit modified();
   }
}

void TiePointPage::gcpListDetached(Subject &subject, const string &signal, const boost::any &v)
{
   GcpList* pGcpList = dynamic_cast<GcpList*>(&subject);
   if (pGcpList != NULL)
   {
      GcpList* pPrimaryList = NULL;
      if (mpPrimaryGcpLayer != NULL)
      {
         pPrimaryList = static_cast<GcpList*>(mpPrimaryGcpLayer->getDataElement());
         VERIFYNRV(pPrimaryList != NULL);
      }
      GcpList* pSecondaryList = NULL;
      if (mpSecondaryGcpLayer != NULL)
      {
         pSecondaryList = static_cast<GcpList*>(mpSecondaryGcpLayer->getDataElement());
         VERIFYNRV(pSecondaryList != NULL);
      }

      if (pGcpList == pPrimaryList)
      {
         mpPrimaryGcps->clearSelection();
         mpPrimaryGcps->clear();
      }
      else if (pGcpList == pSecondaryList)
      {
         mpSecondaryGcps->clearSelection();
         mpSecondaryGcps->clear();
      }
   }
}

void TiePointPage::gcpListModified(Subject &subject, const string &signal, const boost::any &v)
{
   GcpList* pGcpList = dynamic_cast<GcpList*>(&subject);
   if (pGcpList != NULL)
   {
      GcpList* pPrimaryList = NULL;
      if (mpPrimaryGcpLayer != NULL)
      {
         pPrimaryList = static_cast<GcpList*>(mpPrimaryGcpLayer->getDataElement());
         VERIFYNRV(pPrimaryList != NULL);
      }
      GcpList* pSecondaryList = NULL;
      if (mpSecondaryGcpLayer != NULL)
      {
         pSecondaryList = static_cast<GcpList*>(mpSecondaryGcpLayer->getDataElement());
         VERIFYNRV(pSecondaryList != NULL);
      }

      if (pGcpList == pPrimaryList)
      {
         populateListBox(*pPrimaryList, *mpPrimaryGcps, mPrimaryMap);
      }
      else if (pGcpList == pSecondaryList)
      {
         populateListBox(*pSecondaryList, *mpSecondaryGcps, mSecondaryMap);
      }
   }
}

void TiePointPage::tiePointListDeleted(Subject &subject, const string &signal, const boost::any &v)
{
   TiePointList* pTiePointList = dynamic_cast<TiePointList*>(&subject);
   if (pTiePointList != NULL)
   {
      mpTiePointView->clear();
      mpTiePoints = NULL;
      emit modified();
   }
}

bool TiePointPage::isValid() const
{
   // this page is valid iff the Tie Point List View has tie points. Doesn't matter how they get there.
   SpatialDataView* pPrimary = getPrimaryView();
   SpatialDataView* pSecondary = getSecondaryView();
   return (pPrimary != NULL && pSecondary != NULL && pPrimary != pSecondary && mpTiePointView->topLevelItemCount() > 0);
}

void TiePointPage::setViews(SpatialDataView* pPrimary, SpatialDataView* pSecondary)
{
   if (pPrimary != getPrimaryView() || pSecondary != getSecondaryView()) // the views are different
   {
      Service<ModelServices> pModel;
      VERIFYNRV(pModel.get() != NULL);
      pModel->destroyElement(mpTiePoints);
   }
   FusionPage::setViews(pPrimary, pSecondary);
   if (pPrimary != NULL && pSecondary != NULL)
   {
      GcpLayer* pPrimaryLayer = createFusionGcpLayer(pPrimary);
      GcpLayer* pSecondaryLayer = createFusionGcpLayer(pSecondary);

      if (pPrimaryLayer != mpPrimaryGcpLayer)
      {
         setGcpLayer(mpPrimaryGcpLayer, NULL);
      }
      if (pSecondaryLayer != mpSecondaryGcpLayer)
      {
         setGcpLayer(mpSecondaryGcpLayer, NULL);
      }

      setGcpLayer(mpPrimaryGcpLayer, pPrimaryLayer);
      setGcpLayer(mpSecondaryGcpLayer, pSecondaryLayer);
   }
}

void TiePointPage::showEvent(QShowEvent* pEvt)
{
   SpatialDataView* pPrimaryView = getPrimaryView();
   SpatialDataView* pSecondaryView = getSecondaryView();

   if (pPrimaryView != NULL)
   {
      if (mpPrimaryGcpLayer != NULL)
      {
         pPrimaryView->showLayer(mpPrimaryGcpLayer);
      }
   }
   if (pSecondaryView != NULL)
   {
      if (mpSecondaryGcpLayer != NULL)
      {
         pSecondaryView->showLayer(mpSecondaryGcpLayer);
      }
   }

   FusionPage::showEvent(pEvt);
}

void TiePointPage::hideEvent(QHideEvent* pEvt)
{
   SpatialDataView* pPrimaryView = getPrimaryView();
   SpatialDataView* pSecondaryView = getSecondaryView();

   if (pPrimaryView != NULL)
   {
      if (mpPrimaryGcpLayer != NULL)
      {
         pPrimaryView->hideLayer(mpPrimaryGcpLayer);
      }
   }
   if (pSecondaryView != NULL)
   {
      if (mpSecondaryGcpLayer != NULL)
      {
         pSecondaryView->hideLayer(mpSecondaryGcpLayer);
      }
   }
   FusionPage::hideEvent(pEvt);
}

GcpLayer* TiePointPage::createFusionGcpLayer(SpatialDataView* pView)
{
   VERIFYRV(pView != NULL, NULL);

   LayerList* pLayerList = pView->getLayerList();
   VERIFYRV(pLayerList != NULL, NULL);

   vector<Layer*> vect;
   pLayerList->getLayers(GCP_LAYER, vect);

   GcpLayer* pFusionGcpLayer = NULL;
   if (vect.size() > 0)
   {
      for(vector<Layer*>::iterator it = vect.begin(); it != vect.end(); ++it)
      {
         GcpLayer* pLayer = static_cast<GcpLayer*>(*it);
         if (pLayer != NULL)
         {
            string layerName = pLayer->getName();
            if (layerName == FUSION_GCP_NAME)
            {
               pFusionGcpLayer = pLayer;
               break;
            }
         }
      }
   }
   if (pFusionGcpLayer == NULL) // not found or no layers available
   {
      RasterElement* pRaster = dynamic_cast<RasterElement*>(pLayerList->getPrimaryRasterElement());
      VERIFYRV(pRaster != NULL, NULL);
      Service<ModelServices> pModel;
      VERIFYRV(pModel.get() != NULL, NULL);

      DataDescriptor* pDescriptor = pModel->createDataDescriptor(FUSION_GCP_NAME, "GcpList", pRaster);
      if (pDescriptor != NULL)
      {
         DataElement* pGcpList = static_cast<GcpList*>(pModel->createElement(pDescriptor));
         if (pGcpList != NULL)
         {
            pFusionGcpLayer = static_cast<GcpLayer*>(pView->createLayer(GCP_LAYER, pGcpList));
         }
      }
   }
   VERIFYRV(pFusionGcpLayer != NULL, NULL); // Something went really wrong if pFusionGcpLayer is NULL
   return pFusionGcpLayer;
}


void TiePointPage::setGcpLayer(GcpLayer*& pMemberLayer, GcpLayer* pLayer)
{
   if (pMemberLayer != pLayer)
   {
      // detach the old one if it exists
      if (pMemberLayer != NULL)
      {
         pMemberLayer->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &TiePointPage::gcpLayerDeleted));
      }
      pMemberLayer = pLayer;
      if (pLayer != NULL)
      {
         pLayer->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &TiePointPage::gcpLayerDeleted));
      }
   }
}

void TiePointPage::populateListBox(GcpList& gcpList, QTreeWidget& listBox, QMap<QTreeWidgetItem*,const GcpPoint*>& map)
{
   map.clear();
   listBox.clearSelection();
   listBox.clear(); // clear the ListBox
   const list<GcpPoint>& gcps = gcpList.getSelectedPoints();
   int position = 1; // starting GCP #
   // add to the primary ListBox
   for (list<GcpPoint>::const_iterator it = gcps.begin(); it != gcps.end(); ++it)
   {
      const GcpPoint& pt = *it;
      QString gcpName = "GCP " + QString::number(position);
      QTreeWidgetItem* pItem = new QTreeWidgetItem(&listBox);
      pItem->setText(0, gcpName);
      pItem->setText(1, QString::number(pt.mPixel.mX + 1));
      pItem->setText(2, QString::number(pt.mPixel.mY + 1));

      map.insert(pItem, &pt);
      ++position;
   }

   // every time a GCP is added, fit to the GCP
   int i;
   for (i = 0; i < mpPrimaryGcps->columnCount(); ++i)
   {
      mpPrimaryGcps->resizeColumnToContents(i);
   }
   for (i = 0; i < mpSecondaryGcps->columnCount(); ++i)
   {
      mpSecondaryGcps->resizeColumnToContents(i);
   }
}

void TiePointPage::removeGcp(QTreeWidget& view, const GcpLayer& layer,
                             QMap<QTreeWidgetItem*,const GcpPoint*>& map, QPushButton& button)
{
   QTreeWidgetItem* pSelected = NULL;

   QList<QTreeWidgetItem*> selectedGcpItems = view.selectedItems();
   if (selectedGcpItems.empty() == false)
   {
      pSelected = selectedGcpItems.front();
   }

   VERIFYNRV(pSelected != NULL);

   GcpList* pList = dynamic_cast<GcpList*>(layer.getDataElement());
   VERIFYNRV(pList != NULL);

   QMap<QTreeWidgetItem*, const GcpPoint*>::iterator it = map.find(pSelected);
   if (it != map.end())
   {
      pList->removePoint(*(it.value()));
   }

   button.setEnabled(false);
}

void TiePointPage::deriveTiePoint()
{
   VERIFYNRV(mpPrimaryGcpLayer != NULL);
   VERIFYNRV(mpSecondaryGcpLayer != NULL);

   QTreeWidgetItem* pPrimary = NULL;
   QTreeWidgetItem* pSecondary = NULL;

   QList<QTreeWidgetItem*> primaryItems = mpPrimaryGcps->selectedItems();
   if (primaryItems.empty() == false)
   {
      pPrimary = primaryItems.front();
   }

   QList<QTreeWidgetItem*> secondaryItems = mpSecondaryGcps->selectedItems();
   if (secondaryItems.empty() == false)
   {
      pSecondary = secondaryItems.front();
   }

   if (pPrimary != NULL && pSecondary != NULL)
   {
      if (mpTiePoints == NULL)
      {
         GcpList* pGcpList = dynamic_cast<GcpList*>(mpPrimaryGcpLayer->getDataElement());
         GcpList* pMissionGcpList = dynamic_cast<GcpList*>(mpSecondaryGcpLayer->getDataElement());

         VERIFYNRV(pGcpList != NULL);
         VERIFYNRV(pMissionGcpList != NULL);

         string missionDatasetName = pMissionGcpList->getName();

         Service<ModelServices> pModel;

         RasterElement* pPrimaryDataset = NULL;

         SpatialDataView* pPrimaryView = getPrimaryView();
         if (pPrimaryView != NULL)
         {
            LayerList* pLayerList = pPrimaryView->getLayerList();
            if (pLayerList != NULL)
            {
               pPrimaryDataset = pLayerList->getPrimaryRasterElement();
            }
         }

         DataElement* pElement = pModel->getElement(TIE_POINT_NAME, "TiePointList", pPrimaryDataset);
         if (pElement == NULL)
         {
            DataDescriptor* pDescriptor = pModel->createDataDescriptor(TIE_POINT_NAME, "TiePointList", pPrimaryDataset);
            if (pDescriptor != NULL)
            {
               pElement = pModel->createElement(pDescriptor);
            }
         }
         VERIFYNRV(pElement != NULL);
         mpTiePoints = static_cast<TiePointList*>(pElement);
         VERIFYNRV(mpTiePoints != NULL);
       
         mpTiePoints->setMissionDatasetName(missionDatasetName);
         mpTiePoints->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &TiePointPage::tiePointListDeleted));
      }

      QString xPrimary = pPrimary->text(PIXEL_X);
      QString yPrimary = pPrimary->text(PIXEL_Y);

      QString xSecondary = pSecondary->text(PIXEL_X);
      QString ySecondary = pSecondary->text(PIXEL_Y);

      vector<TiePoint> tiePoints = mpTiePoints->getTiePoints();
      TiePoint newPt;
      // compensate for adding 1 in populateListBox()
      newPt.mReferencePoint.mX = (int) xPrimary.toDouble();
      newPt.mReferencePoint.mY = (int) yPrimary.toDouble();
      newPt.mMissionOffset.mX = xSecondary.toFloat();
      newPt.mMissionOffset.mY = ySecondary.toFloat();

      tiePoints.push_back(newPt);
      mpTiePoints->adoptTiePoints(tiePoints);

      // show the true state of the tie point object, not the GCPs
      QTreeWidgetItem* pItem = new QTreeWidgetItem(mpTiePointView);
      VERIFYNRV(pItem != NULL);

      pItem->setText(0, xPrimary);
      pItem->setText(1, yPrimary);
      pItem->setText(2, xSecondary);
      pItem->setText(3, ySecondary);

      // after adding a tie point, destroy the GCPs
      removeGcp(*mpPrimaryGcps, *mpPrimaryGcpLayer, mPrimaryMap, *mpPrimGcpRemoveButton);
      removeGcp(*mpSecondaryGcps, *mpSecondaryGcpLayer, mSecondaryMap, *mpSecGcpRemoveButton);
   }
   mpDeriveButton->setEnabled(false);
   emit modified();
}

void TiePointPage::enableGcpActions()
{
   const QTreeWidget* pView = static_cast<const QTreeWidget*>(sender());
   QTreeWidgetItem* pPrimary = NULL;
   QTreeWidgetItem* pSecondary = NULL;

   QList<QTreeWidgetItem*> primaryItems = mpPrimaryGcps->selectedItems();
   if (primaryItems.empty() == false)
   {
      pPrimary = primaryItems.front();
   }

   QList<QTreeWidgetItem*> secondaryItems = mpSecondaryGcps->selectedItems();
   if (secondaryItems.empty() == false)
   {
      pSecondary = secondaryItems.front();
   }

   if (pView == mpPrimaryGcps)
   {
      mpPrimGcpRemoveButton->setEnabled(pPrimary != NULL);
   }
   else if (pView == mpSecondaryGcps)
   {
      mpSecGcpRemoveButton->setEnabled(pSecondary != NULL);
   }
   mpDeriveButton->setEnabled(pPrimary != NULL && pSecondary != NULL);
}

void TiePointPage::enableTiePointActions()
{
   QTreeWidgetItem* pItem = NULL;

   QList<QTreeWidgetItem*> selectedTiePointItems = mpTiePointView->selectedItems();
   if (selectedTiePointItems.empty() == false)
   {
      pItem = selectedTiePointItems.front();
   }

   mpTiePointRemoveButton->setEnabled(pItem != NULL);
   mpZoomBox->setEnabled(pItem != NULL);
}

void TiePointPage::removeTiePoint()
{
   VERIFYNRV(mpTiePointView != NULL);

   QList<QTreeWidgetItem*> selectedTiePointItems = mpTiePointView->selectedItems();

   for (QList<QTreeWidgetItem*>::iterator it = selectedTiePointItems.begin(); it != selectedTiePointItems.end(); ++it)
   {
      QTreeWidgetItem* pItem = *it;

      VERIFYNRV(pItem != NULL);

      QString px = pItem->text(PRI_X), py = pItem->text(PRI_Y);
      QString sx = pItem->text(SEC_X), sy = pItem->text(SEC_Y);

      GcpList* pPrimary = dynamic_cast<GcpList*>(mpPrimaryGcpLayer->getDataElement());
      VERIFYNRV(pPrimary != NULL);
      GcpList* pSecondary = dynamic_cast<GcpList*>(mpSecondaryGcpLayer->getDataElement());
      VERIFYNRV(pSecondary != NULL);

      GcpPoint primary;
      primary.mPixel = LocationType(px.toDouble()-1, py.toDouble()-1);
      pPrimary->addPoint(primary);

      GcpPoint secondary;
      secondary.mPixel = LocationType(sx.toDouble()-1, sy.toDouble()-1);
      pSecondary->addPoint(secondary);

      delete pItem;
   }
   enableTiePointActions();   
}

void TiePointPage::removeGcp()
{
   VERIFYNRV(mpPrimGcpRemoveButton != NULL);
   VERIFYNRV(mpSecGcpRemoveButton != NULL);

   const QPushButton* pButton = static_cast<const QPushButton*>(sender());
   if (pButton == mpPrimGcpRemoveButton)
   {
      removeGcp(*mpPrimaryGcps, *mpPrimaryGcpLayer, mPrimaryMap, *mpPrimGcpRemoveButton);
   }
   else if (pButton == mpSecGcpRemoveButton)
   {
      removeGcp(*mpSecondaryGcps, *mpSecondaryGcpLayer, mSecondaryMap, *mpSecGcpRemoveButton);
   }
}

void TiePointPage::verifyTiePoint(QTreeWidgetItem* pItem, int column)
{
   SpatialDataView* pPrimaryView = getPrimaryView();
   SpatialDataView* pSecondaryView = getSecondaryView();

   if (pItem != NULL)
   {
      VERIFYNRV(pPrimaryView != NULL && pSecondaryView != NULL);

      LocationType primary(pItem->text(PRI_X).toDouble(), pItem->text(PRI_Y).toDouble());
      LocationType secondary(pItem->text(SEC_X).toDouble(), pItem->text(SEC_Y).toDouble());

      int zPct = mpZoomBox->value();
      pPrimaryView->zoomToPoint(primary, zPct);
      pSecondaryView->zoomToPoint(secondary, zPct);
      pPrimaryView->refresh();
      pSecondaryView->refresh();
   }
}
