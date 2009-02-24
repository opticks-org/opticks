/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QEvent>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QListWidget>
#include <QtGui/QGroupBox>
#include <QtGui/QPushButton>

#include "ApplicationServices.h"
#include "ConfigurationSettings.h"
#include "AppVerify.h"
#include "DataFusion.h"
#include "DatasetPage.h"
#include "Slot.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "TypesFile.h"

#include <vector>
#include <string>
using namespace std;

const std::string DatasetPage::BYPASS_TIE_POINT_STATE = "Bypass Tie Point Page";

DatasetPage::DatasetPage(QWidget* pParent) :
   FusionPage(pParent)
{
   // Primary data set
   QLabel* pPrimaryDatasetLabel = new QLabel("Primary Data Set", this);
   mpPrimaryList = new QListWidget(this);

   // Secondary data set
   QLabel* pSecondaryDatasetLabel = new QLabel("Secondary Data Set", this);
   mpSecondaryList = new QListWidget(this);

   // Browse button
   QPushButton* pBrowser = new QPushButton("Browse for Data", this);

   // Algorithm group
   mpGroupBox = new QGroupBox("Algorithm Options", this);

   mpBypassCheckBox = new QCheckBox("Bypass Tie Point Step", mpGroupBox);
   mpSaveCheckBox = new QCheckBox("Save Settings", mpGroupBox);

   QVBoxLayout* pAgreementLayout = new QVBoxLayout(mpGroupBox);
   pAgreementLayout->setMargin(10);
   pAgreementLayout->setSpacing(5);
   pAgreementLayout->addWidget(mpBypassCheckBox);
   pAgreementLayout->addWidget(mpSaveCheckBox);

   QString tip = "Only check this box if you think that the georeferencing provided\n"
      "by both datasets is sufficiently accurate to bypass the Tie Point Step.";
   mpGroupBox->setToolTip(tip);

   // Layout
   QHBoxLayout* pSecondaryLayout = new QHBoxLayout();
   pSecondaryLayout->setMargin(0);
   pSecondaryLayout->setSpacing(5);
   pSecondaryLayout->addWidget(pSecondaryDatasetLabel, Qt::AlignBottom);
   pSecondaryLayout->addStretch();
   pSecondaryLayout->addWidget(pBrowser);

   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(5);
   pLayout->addWidget(pPrimaryDatasetLabel);
   pLayout->addWidget(mpPrimaryList, 10);
   pLayout->addLayout(pSecondaryLayout);
   pLayout->addWidget(mpSecondaryList, 10);
   pLayout->addWidget(mpGroupBox);

   // connections
   connect(pBrowser, SIGNAL(clicked()), this, SLOT(importData()));
   connect(mpPrimaryList, SIGNAL(itemSelectionChanged()), this, SIGNAL(modified()));
   connect(mpSecondaryList, SIGNAL(itemSelectionChanged()), this, SIGNAL(modified()));
   connect(mpBypassCheckBox, SIGNAL(toggled(bool)), this, SIGNAL(modified()));

   // fetch settings
   bool bChecked = false;
   if (DatasetPage::hasSettingBypassTiePointStep())
   {
      bChecked = DatasetPage::getSettingBypassTiePointStep();
   }
   mpBypassCheckBox->setChecked(bChecked);

   vector<Window*> windows;
   mpDesktop->getWindows(SPATIAL_DATA_WINDOW, windows);
   for (vector<Window*>::iterator it = windows.begin(); it != windows.end(); ++it)
   {
      Window* pWindow = *it;
      if (pWindow != NULL)
      {
         pWindow->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &DatasetPage::windowDeleted));
         pWindow->attach(SIGNAL_NAME(Subject, Modified), Slot(this, &DatasetPage::windowModified));
      }
   }

   mpDesktop->attach(SIGNAL_NAME(DesktopServices, WindowAdded), Slot(this, &DatasetPage::windowAdded));
}

DatasetPage::~DatasetPage()
{
   vector<Window*> windows;
   mpDesktop->getWindows(SPATIAL_DATA_WINDOW, windows);
   for (vector<Window*>::iterator it = windows.begin(); it != windows.end(); ++it)
   {
      Window* pWindow = *it;
      if (pWindow != NULL)
      {
         pWindow->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &DatasetPage::windowDeleted));
         pWindow->detach(SIGNAL_NAME(Subject, Modified), Slot(this, &DatasetPage::windowModified));
      }
   }

   mpDesktop->detach(SIGNAL_NAME(DesktopServices, WindowAdded), Slot(this, &DatasetPage::windowAdded));
   setViews(NULL, NULL);
}

bool DatasetPage::canBypassTiePointStep() const
{
   VERIFY(mpBypassCheckBox != NULL);
   // only allow the bypass if the widget is enabled (ie. RPCs are present) and the box is checked
   return mpGroupBox->isEnabled() && mpBypassCheckBox->isChecked();
}

void DatasetPage::setViews(SpatialDataView* pPrimary, SpatialDataView* pSecondary)
{
   FusionPage::setViews(pPrimary, pSecondary);
   setView(pPrimary, *mpPrimaryList);
   setView(pSecondary, *mpSecondaryList);
}

SpatialDataView* DatasetPage::getPrimaryView() const
{
   return getView(mpPrimaryList);
}

SpatialDataView* DatasetPage::getSecondaryView() const
{
   return getView(mpSecondaryList);
}

void DatasetPage::hideEvent(QHideEvent* pEvt)
{
   bool bSave = mpSaveCheckBox->isEnabled() && mpSaveCheckBox->isChecked();
   if (bSave)
   {
      bool bChecked = mpBypassCheckBox->isChecked();
      DatasetPage::setSettingBypassTiePointStep(bChecked);
   }

   FusionPage::hideEvent(pEvt);
}

void DatasetPage::importData()
{
   VERIFYNRV(mpDesktop.get() != NULL);
   mpDesktop->importFile("Raster Element");
}

bool DatasetPage::isValid() const
{
   VERIFY(mpPrimaryList != NULL);
   VERIFY(mpSecondaryList != NULL);

   SpatialDataView* pPrimary = getPrimaryView();
   SpatialDataView* pSecondary = getSecondaryView();

   return pPrimary != NULL && pSecondary != NULL && pPrimary != pSecondary;
}

void DatasetPage::attached(Subject& subject, const string& signal, const Slot& slot)
{
   windowAttached(subject, signal, boost::any());
}

void DatasetPage::detached(Subject& subject, const string& signal, const Slot& slot)
{
   windowDeleted(subject, signal, boost::any());
}

void DatasetPage::windowAdded(Subject& subject, const string& signal, const boost::any& value)
{
   if (dynamic_cast<DesktopServices*>(&subject) == mpDesktop.get())
   {
      Window* pWindow = boost::any_cast<Window*>(value);
      if (dynamic_cast<SpatialDataWindow*>(pWindow) != NULL)
      {
         pWindow->attach(SIGNAL_NAME(Subject, Modified), Slot(this, &DatasetPage::windowModified));
         pWindow->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &DatasetPage::windowDeleted));
      }
   }
}

void DatasetPage::windowAttached(Subject& subject, const string& signal, const boost::any& v)
{
   SpatialDataWindow* pWindow = dynamic_cast<SpatialDataWindow*>(&subject);
   if (pWindow != NULL)
   {
      string windowName = pWindow->getName();
      VERIFYNRV(!windowName.empty());

      QList<QListWidgetItem*> primaryItems = mpPrimaryList->findItems(QString::fromStdString(windowName),
         Qt::MatchExactly);
      QList<QListWidgetItem*> secondaryItems = mpSecondaryList->findItems(QString::fromStdString(windowName),
         Qt::MatchExactly);

      QListWidgetItem* pPrimaryItem = NULL;
      if (primaryItems.empty() == false)
      {
         pPrimaryItem = primaryItems.front();
      }

      QListWidgetItem* pSecondaryItem = NULL;
      if (secondaryItems.empty() == false)
      {
         pSecondaryItem = secondaryItems.front();
      }

      if (pPrimaryItem == NULL) // not found
      {
         mpPrimaryList->addItem(windowName.c_str()); // insert at end
         QListWidgetItem* pItem = mpPrimaryList->item(mpPrimaryList->count()-1); // get last item
         if (pItem != NULL)
         {
            mPrimaryMap.insert(pWindow, pItem);
         }
      }
      if (pSecondaryItem == NULL)
      {
         mpSecondaryList->addItem(windowName.c_str()); // insert at end
         QListWidgetItem* pItem = mpSecondaryList->item(mpPrimaryList->count()-1); // get last item
         if (pItem != NULL)
         {
            mSecondaryMap.insert(pWindow, pItem);
         }
      }
   }
}

void DatasetPage::windowDeleted(Subject& subject, const string& signal, const boost::any& v)
{
   SpatialDataWindow* pWindow = dynamic_cast<SpatialDataWindow*>(&subject);
   if (pWindow != NULL)
   {
      string windowName = pWindow->getName();
      VERIFYNRV(!windowName.empty());

      QList<QListWidgetItem*> primaryItems = mpPrimaryList->findItems(QString::fromStdString(windowName),
         Qt::MatchExactly);
      QList<QListWidgetItem*> secondaryItems = mpSecondaryList->findItems(QString::fromStdString(windowName),
         Qt::MatchExactly);

      QListWidgetItem* pPrimaryItem = NULL;
      if (primaryItems.empty() == false)
      {
         pPrimaryItem = primaryItems.front();
      }

      QListWidgetItem* pSecondaryItem = NULL;
      if (secondaryItems.empty() == false)
      {
         pSecondaryItem = secondaryItems.front();
      }

      if (pPrimaryItem != NULL) // was found, so remove it
      {
         delete (mpPrimaryList->takeItem(mpPrimaryList->row(mPrimaryMap[pWindow])));
         mPrimaryMap.remove(pWindow);
      }
      if (pSecondaryItem != NULL) // was found, so remove it
      {
         delete (mpSecondaryList->takeItem(mpSecondaryList->row(mSecondaryMap[pWindow])));
         mSecondaryMap.remove(pWindow);
      }
      emit modified();
   }
}

void DatasetPage::windowModified(Subject& subject, const string& signal, const boost::any& v)
{
   SpatialDataWindow* pWindow = dynamic_cast<SpatialDataWindow*>(&subject);
   if (pWindow != NULL)
   {
      string windowName = pWindow->getName();
      VERIFYNRV(!windowName.empty());

      QList<QListWidgetItem*> primaryItems = mpPrimaryList->findItems(QString::fromStdString(windowName),
         Qt::MatchExactly);
      QList<QListWidgetItem*> secondaryItems = mpSecondaryList->findItems(QString::fromStdString(windowName),
         Qt::MatchExactly);

      QListWidgetItem* pPrimaryItem = NULL;
      if (primaryItems.empty() == false)
      {
         pPrimaryItem = primaryItems.front();
      }

      QListWidgetItem* pSecondaryItem = NULL;
      if (secondaryItems.empty() == false)
      {
         pSecondaryItem = secondaryItems.front();
      }

      if (pPrimaryItem == NULL) // not found
      {
         mpPrimaryList->addItem(windowName.c_str()); // insert at end
         QListWidgetItem* pItem = mpPrimaryList->item(mpPrimaryList->count()-1); // get last item
         if (pItem != NULL)
         {
            mPrimaryMap.insert(pWindow, pItem);
         }
      }
      if (pSecondaryItem == NULL)
      {
         mpSecondaryList->addItem(windowName.c_str()); // insert at end
         QListWidgetItem* pItem = mpSecondaryList->item(mpPrimaryList->count()-1); // get last item
         if (pItem != NULL)
         {
            mSecondaryMap.insert(pWindow, pItem);
         }
      }
   }
}

void DatasetPage::setView(SpatialDataView* pView, QListWidget& box)
{
   string viewName;
   if (pView != NULL)
   {
      viewName = pView->getName();
   }

   QList<QListWidgetItem*> items = box.findItems(viewName.c_str(), Qt::MatchExactly);
   if (items.empty() == false)
   {
      box.clearSelection();
      box.setItemSelected(items.front(), true);
   }
}

SpatialDataView* DatasetPage::getView(QListWidget* pBox) const
{
   if (pBox == NULL)
   {
      return NULL;
   }

   SpatialDataView* pView = NULL;

   QList<QListWidgetItem*> selectedItems = pBox->selectedItems();
   if (selectedItems.empty() == false)
   {
      QListWidgetItem* pItem = selectedItems.front();
      if (pItem != NULL)
      {
         QString windowName = pItem->text();
         if (!windowName.isEmpty())
         {
            SpatialDataWindow* pWindow =
               dynamic_cast<SpatialDataWindow*>(mpDesktop->getWindow(windowName.toStdString(), SPATIAL_DATA_WINDOW));
            if (pWindow != NULL)
            {
               pView = pWindow->getSpatialDataView();
            }
         }
      }
   }

   return pView;
}
