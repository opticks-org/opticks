/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "DesktopServices.h"
#include "Layer.h"
#include "LayerImp.h"
#include "LayerList.h"
#include "LinkOptionsPage.h"
#include "SpatialDataView.h"
#include "View.h"
#include "WorkspaceWindow.h"

#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QRadioButton>
#include <QtGui/QStackedWidget>
#include <QtGui/QTreeWidget>
#include <QtGui/QTreeWidgetItem>
#include <QtGui/QVBoxLayout>

using namespace std;

LinkOptionsPage::LinkOptionsPage(QWidget* pParent) :
   QWidget(pParent)
{
   // Links
   QLabel* pLinksLabel = new QLabel("Available Links:", this);

   mpLinksTree = new QTreeWidget(this);
   mpLinksTree->setColumnCount(1);
   mpLinksTree->setRootIsDecorated(true);
   mpLinksTree->setSelectionMode(QAbstractItemView::SingleSelection);
   mpLinksTree->setSortingEnabled(false);
   mpLinksTree->setHeaderHidden(true);

   // Options
   QLabel* pOptionsLabel = new QLabel("Options:", this);
   mpStack = new QStackedWidget(this);

   // Views
   mpViewWidget = new QWidget(mpStack);
   QGroupBox* pLinkTypeBox = new QGroupBox("Link type:", mpViewWidget);
   mpAutolinkRadio = new QRadioButton("Automatic", pLinkTypeBox);
   mpMirrorRadio = new QRadioButton("Mirror", pLinkTypeBox);
   mpGeoRadio = new QRadioButton("Latitude/Longitude", pLinkTypeBox);
   mpUnlinkRadio = new QRadioButton("Unlink", pLinkTypeBox);

   QButtonGroup* pLinkTypeGroup = new QButtonGroup(pLinkTypeBox);
   pLinkTypeGroup->setExclusive(true);
   pLinkTypeGroup->addButton(mpAutolinkRadio);
   pLinkTypeGroup->addButton(mpMirrorRadio);
   pLinkTypeGroup->addButton(mpGeoRadio);
   pLinkTypeGroup->addButton(mpUnlinkRadio);

   QVBoxLayout* pLinkTypeLayout = new QVBoxLayout(pLinkTypeBox);
   pLinkTypeLayout->addWidget(mpAutolinkRadio);
   pLinkTypeLayout->addWidget(mpMirrorRadio);
   pLinkTypeLayout->addWidget(mpGeoRadio);
   pLinkTypeLayout->addWidget(mpUnlinkRadio);

   mpTwoWayCheck = new QCheckBox("Two-Way", mpViewWidget);

   mpStack->addWidget(mpViewWidget);

   // Layers
   mpLayerWidget = new QWidget(mpStack);
   mpDuplicateCheck = new QCheckBox("Duplicate Only", mpLayerWidget);

   mpStack->addWidget(mpLayerWidget);

   // Layout
   QVBoxLayout* pViewLayout = new QVBoxLayout(mpViewWidget);
   pViewLayout->setMargin(0);
   pViewLayout->setSpacing(10);
   pViewLayout->addWidget(pLinkTypeBox);
   pViewLayout->addWidget(mpTwoWayCheck, 0, Qt::AlignLeft);
   pViewLayout->addStretch(10);

   QVBoxLayout* pLayerLayout = new QVBoxLayout(mpLayerWidget);
   pLayerLayout->setMargin(0);
   pLayerLayout->setSpacing(5);
   pLayerLayout->addWidget(mpDuplicateCheck);
   pLayerLayout->addStretch(10);

   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(0);
   pGrid->setSpacing(10);
   pGrid->setRowMinimumHeight(0, 5);
   pGrid->addWidget(pLinksLabel, 1, 0);
   pGrid->addWidget(mpLinksTree, 2, 0);
   pGrid->addWidget(pOptionsLabel, 1, 1);
   pGrid->addWidget(mpStack, 2, 1);
   pGrid->setRowStretch(2, 10);
   pGrid->setColumnStretch(0, 10);
   pGrid->setColumnStretch(1, 10);

   // Initialization
   LinkType linkType = View::getSettingLinkType();
   switch (linkType)
   {
   case AUTOMATIC_LINK:
   default:
      mpAutolinkRadio->setChecked(true);
      break;

   case MIRRORED_LINK:
      mpMirrorRadio->setChecked(true);
      break;

   case GEOCOORD_LINK:
      mpGeoRadio->setChecked(true);
      break;
   }

   mpDuplicateCheck->setChecked(true);       // Temporary: Initially checked until more options exist

   // Connections
   connect(mpLinksTree, SIGNAL(itemSelectionChanged()), this, SLOT(activateOptions()));
}

LinkOptionsPage::~LinkOptionsPage()
{}

vector<View*> LinkOptionsPage::getViewLinks() const
{
   vector<View*> viewLinks;

   QMap<QTreeWidgetItem*, View*>::const_iterator iter = mViews.begin();
   while (iter != mViews.end())
   {
      Qt::CheckState checkState = Qt::Unchecked;

      QTreeWidgetItem* pItem = iter.key();
      if (pItem != NULL)
      {
         checkState = pItem->checkState(0);
      }

      if (checkState == Qt::Checked)
      {
         View* pView = iter.value();
         if (pView != NULL)
         {
            viewLinks.push_back(pView);
         }
      }

      ++iter;
   }

   return viewLinks;
}

bool LinkOptionsPage::isTwoWayLink() const
{
   return mpTwoWayCheck->isChecked();
}

vector<Layer*> LinkOptionsPage::getLayerLinks() const
{
   vector<Layer*> layerLinks;

   QMap<QTreeWidgetItem*, Layer*>::const_iterator iter = mLayers.begin();
   while (iter != mLayers.end())
   {
      Qt::CheckState checkState = Qt::Unchecked;

      QTreeWidgetItem* pItem = iter.key();
      if (pItem != NULL)
      {
         checkState = pItem->checkState(0);
      }

      if (checkState == Qt::Checked)
      {
         Layer* pLayer = iter.value();
         if (pLayer != NULL)
         {
            layerLinks.push_back(pLayer);
         }
      }

      ++iter;
   }

   return layerLinks;
}

bool LinkOptionsPage::areLayersDuplicated() const
{
   bool bDuplicate = mpDuplicateCheck->isChecked();
   return bDuplicate;
}

void LinkOptionsPage::setLinkObjects(const QString& strDataset)
{
   if (strDataset == mstrDataset)
   {
      return;
   }

   mpLinksTree->clear();
   mstrDataset = strDataset;

   if (strDataset.isEmpty() == true)
   {
      return;
   }

   SpatialDataView* pSpatialDataView = NULL;
   vector<Window*> windows;

   Service<DesktopServices> pDesktop;
   pDesktop->getWindows(windows);

   vector<Window*>::iterator iter;
   for (iter = windows.begin(); iter != windows.end(); ++iter)
   {
      WorkspaceWindow* pWindow = dynamic_cast<WorkspaceWindow*>(*iter);
      if (pWindow != NULL)
      {
         SpatialDataView* pView = dynamic_cast<SpatialDataView*>(pWindow->getActiveView());
         if (pView != NULL)
         {
            QString strViewName = QString::fromStdString(pView->getName());
            if (strViewName == strDataset)
            {
               pSpatialDataView = pView;
               break;
            }
         }
      }
   }

   if (pSpatialDataView == NULL)
   {
      return;
   }

   // Layers
   vector<Layer*> layers;

   LayerList* pLayerList = NULL;
   pLayerList = pSpatialDataView->getLayerList();
   if (pLayerList != NULL)
   {
      pLayerList->getLayers(layers);
   }

   if (layers.empty() == false)
   {
      QTreeWidgetItem* pLayerItem = new QTreeWidgetItem(mpLinksTree);

      for (unsigned int i = 0; i < layers.size(); i++)
      {
         Layer* pLayer = layers[i];
         if (pLayer != NULL)
         {
            QString strName = QString::fromStdString(pLayer->getName());

            QTreeWidgetItem* pItem = new QTreeWidgetItem(pLayerItem);
            if (pItem != NULL)
            {
               pItem->setText(0, strName);
               pItem->setCheckState(0, Qt::Unchecked);
               mLayers.insert(pItem, pLayer);
            }
         }
      }

      if (pLayerItem != NULL)
      {
         mpLinksTree->setCurrentItem(pLayerItem);
         pLayerItem->setText(0, "Layers");
         pLayerItem->setFlags(Qt::ItemIsEnabled);

         int iItemCount = pLayerItem->childCount();
         if (iItemCount == 0)
         {
            pLayerItem->setFlags(0);
         }
         else
         {
            mpLinksTree->expandItem(pLayerItem);
         }
      }
   }

   // Views
   QString strViewName = QString::fromStdString(pSpatialDataView->getName());
   if (strViewName.isEmpty() == false)
   {
      QTreeWidgetItem* pViewItem = new QTreeWidgetItem(mpLinksTree);

      QTreeWidgetItem* pItem = new QTreeWidgetItem(pViewItem);
      if (pItem != NULL)
      {
         pItem->setText(0, strViewName);
         pItem->setCheckState(0, Qt::Unchecked);
         mViews.insert(pItem, pSpatialDataView);
      }

      if (pViewItem != NULL)
      {
         pViewItem->setText(0, "Views");
         pViewItem->setFlags(Qt::ItemIsEnabled);

         int iItemCount = pViewItem->childCount();
         if (iItemCount == 0)
         {
            pViewItem->setFlags(0);
         }
         else
         {
            mpLinksTree->expandItem(pViewItem);
         }
      }
   }

   mpStack->setCurrentWidget(mpLayerWidget);
}

void LinkOptionsPage::activateOptions()
{
   QList<QTreeWidgetItem*> selectedItems = mpLinksTree->selectedItems();
   if (selectedItems.empty() == true)
   {
      return;
   }

   QTreeWidgetItem* pItem = selectedItems.front();
   if (pItem == NULL)
   {
      return;
   }

   if (mViews.contains(pItem) == true)
   {
      mpStack->setCurrentWidget(mpViewWidget);
   }
   else if (mLayers.contains(pItem) == true)
   {
      mpStack->setCurrentWidget(mpLayerWidget);
   }
   else
   {
      QString strText = pItem->text(0);
      if (strText == "Windows")
      {
         mpStack->setCurrentWidget(mpViewWidget);
      }
      else if (strText == "Layers")
      {
         mpStack->setCurrentWidget(mpLayerWidget);
      }
   }
}

LinkType LinkOptionsPage::getLinkType() const
{
   if (mpAutolinkRadio->isChecked())
   {
      return AUTOMATIC_LINK;
   }
   else if (mpMirrorRadio->isChecked())
   {
      return MIRRORED_LINK;
   }
   else if (mpGeoRadio->isChecked())
   {
      return GEOCOORD_LINK;
   }
   
   return NO_LINK;
}
