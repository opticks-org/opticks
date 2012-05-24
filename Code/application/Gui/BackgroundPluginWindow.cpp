/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QCoreApplication>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QProgressBar>
#include <QtGui/QPushButton>

#include "BackgroundPluginWindow.h"
#include "Executable.h"
#include "PlugIn.h"
#include "PlugInCallback.h"
#include "PlugInManagerServicesImp.h"
#include "Progress.h"
#include "Slot.h"

using namespace std;

///////////////////////
// BackgroundPluginItem
///////////////////////

BackgroundPluginItem::BackgroundPluginItem(const QString& strPlugInName, Progress* pProgress, PlugIn* pPlugIn,
                                           QListWidget* pListWidget) :
   QListWidgetItem(pListWidget, 1111),
   mpNameLabel(NULL),
   mpProgressBar(NULL),
   mpStatusLabel(NULL),
   mpPlugIn(pPlugIn),
   mpProgress(pProgress),
   mFinished(false)
{
   // Item widget
   QWidget* pStatusWidget = new QWidget(pListWidget);
   pStatusWidget->setFixedHeight(40);

   // Plug-in label
   mpNameLabel = new QLabel(strPlugInName, pStatusWidget);

   // Progress bar
   mpProgressBar = new QProgressBar(pStatusWidget);
   mpProgressBar->setRange(0, 100);

   // Status label
   mpStatusLabel = new QLabel(pStatusWidget);
   mpStatusLabel->setFixedHeight(mpStatusLabel->sizeHint().height());

   // Layout
   QGridLayout* pGrid = new QGridLayout(pStatusWidget);
   pGrid->setMargin(5);
   pGrid->setSpacing(5);
   pGrid->addWidget(mpNameLabel, 0, 0);
   pGrid->addWidget(mpProgressBar, 0, 1);
   pGrid->addWidget(mpStatusLabel, 1, 0, 1, 2);
   pGrid->setColumnStretch(1, 10);

   // Initialization
   if (pListWidget != NULL)
   {
      pListWidget->setItemWidget(this, pStatusWidget);
   }

   update();
   setSizeHint(pStatusWidget->sizeHint());
}

BackgroundPluginItem::~BackgroundPluginItem()
{}

void BackgroundPluginItem::setData(int role, const QVariant& value)
{
   if (role == Qt::TextColorRole)
   {
      QColor textColor = qvariant_cast<QColor>(value);

      QPalette namePalette = mpNameLabel->palette();
      namePalette.setColor(QPalette::Text, textColor);
      mpNameLabel->setPalette(namePalette);

      QPalette progressPalette = mpProgressBar->palette();
      progressPalette.setColor(QPalette::Text, textColor);
      mpProgressBar->setPalette(progressPalette);

      QPalette statusPalette = mpStatusLabel->palette();
      statusPalette.setColor(QPalette::Text, textColor);
      mpStatusLabel->setPalette(statusPalette);
   }

   QListWidgetItem::setData(role, value);
}

Progress* BackgroundPluginItem::getProgress() const
{
   return mpProgress;
}

PlugIn* BackgroundPluginItem::getPlugIn() const
{
   return mpPlugIn;
}

void BackgroundPluginItem::update()
{
   string text;
   int percent;
   ReportingLevel gran;

   mpProgress->getProgress(text, percent, gran);
   mpProgressBar->setValue(percent);

   mpStatusLabel->setText(QString::fromStdString(text));
}

void BackgroundPluginItem::finish()
{
   // Abort the plug-in
   Executable* pExecutable = dynamic_cast<Executable*>(mpPlugIn);
   if (pExecutable != NULL)
   {
      if (!pExecutable->hasAbort() || pExecutable->abort())
      {
         mFinished = true;
      }
   }
}

bool BackgroundPluginItem::isFinished() const
{
   return mFinished;
}

void BackgroundPluginItem::setStatus(QString statusStr)
{
   mpStatusLabel->setText(statusStr);
}

/////////////////////////
// BackgroundPluginWindow
/////////////////////////

// Custom event types - these values must be between 1000 and 65535 as stated in the QEvent documentation
const int BackgroundPluginWindow::BPW_PMODIFIED = 60000;
const int BackgroundPluginWindow::BPW_CALLBACK = 60001;

BackgroundPluginWindow::BackgroundPluginWindow(const string& id, QWidget* parent) :
   DockWindowAdapter(id, "Background Plug-In Window", parent),
   mpStatusList(NULL),
   mpDismissButton(NULL),
   mpAbortButton(NULL),
   mSessionClosing(false)
{
   // Widget frame
   QWidget* pWindowWidget = new QWidget(this);

   // Status list widget
   mpStatusList = new QListWidget(pWindowWidget);
   mpStatusList->setSelectionMode(QAbstractItemView::SingleSelection);
   mpStatusList->setUniformItemSizes(false);

   // Dismiss button
   mpDismissButton = new QPushButton("Dismiss", pWindowWidget);
   mpDismissButton->setToolTip("Remove selected status items if the plugin has completed or has been aborted.");

   // Abort button
   mpAbortButton = new QPushButton("Abort", pWindowWidget);
   mpAbortButton->setToolTip("Abort selected plugins if they support user aborts.");

   // Layout
   QHBoxLayout* pButtonLayout = new QHBoxLayout();
   pButtonLayout->setMargin(0);
   pButtonLayout->setSpacing(5);
   pButtonLayout->addStretch(10);
   pButtonLayout->addWidget(mpDismissButton);
   pButtonLayout->addWidget(mpAbortButton);

   QVBoxLayout* pLayout = new QVBoxLayout(pWindowWidget);
   pLayout->setMargin(2);
   pLayout->setSpacing(5);
   pLayout->addWidget(mpStatusList);
   pLayout->addLayout(pButtonLayout);

   // Initialization
   setIcon(QIcon(":/icons/BackgroundTask"));

   setWidget(pWindowWidget);
   mpDismissButton->setEnabled(false);
   mpAbortButton->setEnabled(false);

   // Connections
   connect(mpStatusList, SIGNAL(itemSelectionChanged()), this, SLOT(changeSelection()));
   connect(mpDismissButton, SIGNAL(clicked()), this, SLOT(dismissClicked()));
   connect(mpAbortButton, SIGNAL(clicked()), this, SLOT(abortClicked()));
   Service<SessionManager> pSessionMgr;
   mpSessionMgrImp.reset(dynamic_cast<SessionManagerImp*>(pSessionMgr.get()));
   mpSessionMgrImp.addSignal(SIGNAL_NAME(SessionManagerImp, SessionFullyClosed),
      Slot(this, &BackgroundPluginWindow::sessionClosed));
   mpSessionMgr.reset(pSessionMgr.get());
   mpSessionMgr.addSignal(SIGNAL_NAME(SessionManager, Closed),
      Slot(this, &BackgroundPluginWindow::sessionClosing));
}

BackgroundPluginWindow::~BackgroundPluginWindow()
{
}

BackgroundPluginItem* BackgroundPluginWindow::addItem(PlugIn* pPlugIn, Progress* pProgress)
{
   QString name; // the Item's ID

   // First, query for a name
   if (pPlugIn != NULL)
   {
      name = QString::fromStdString(pPlugIn->getName());
   }

   // if the name query above did not give us an ID,
   // create something reasonable. PlugIn LOC where LOC
   // is the hexadecimal memory location of the plug-in pointer
   if (name.isEmpty() == true)
   {
      name = "PlugIn " + QString::number(reinterpret_cast<size_t>(pPlugIn), 16);
   }

   pProgress->attach(SIGNAL_NAME(Subject, Modified), Slot(this, &BackgroundPluginWindow::progressChanged));

   BackgroundPluginItem* pItem = new BackgroundPluginItem(name, pProgress, pPlugIn, mpStatusList);
   show();
   return pItem;
}

BackgroundPluginItem* BackgroundPluginWindow::getItem(Progress* pProgress)
{
   for (int i = 0; i < mpStatusList->count(); ++i)
   {
      BackgroundPluginItem* pItem = static_cast<BackgroundPluginItem*>(mpStatusList->item(i));
      if (pItem != NULL)
      {
         Progress* pCurrentProgress = pItem->getProgress();
         if (pCurrentProgress == pProgress)
         {
            return pItem;
         }
      }
   }

   return NULL;
}

void BackgroundPluginWindow::progressChanged(Subject &subject, const string &signal, const boost::any &v)
{
   Progress* pProgress = dynamic_cast<Progress*>(&subject);
   if (NN(pProgress))
   {
      QCoreApplication::postEvent(this, new ModifiedEvent(pProgress));
   }
}

bool BackgroundPluginWindow::addCallback(PlugInCallback *callback)
{
   if ((callback != NULL) && (mSessionClosing == false))
   {
      QCoreApplication::postEvent(this, new CallbackEvent(callback));
      return true;
   }

   return false;
}

void BackgroundPluginWindow::customEvent(QEvent* pEvent)
{
   int type = static_cast<int>(pEvent->type());
   switch (type)
   {
      case BPW_PMODIFIED:
      {
         Progress* pProgress = (static_cast<ModifiedEvent*>(pEvent))->getProgress();
         if (pProgress != NULL)
         {
            BackgroundPluginItem* pItem = getItem(pProgress);
            if (pItem != NULL)
            {
               pItem->update();
            }
         }

         break;
      }
      case BPW_CALLBACK:
      {
         PlugInCallback* pCallback = (static_cast<CallbackEvent*>(pEvent))->getCallback();
         if (pCallback != NULL)
         {
            if (pCallback->finish() == true)
            {
               (*pCallback)();
            }

            PlugIn* pPlugIn(NULL);
            bool bDestroy(true);

            BackgroundPluginItem* pItem(getItem(pCallback->progress()));
            if (pItem != NULL)
            {
               pPlugIn = pItem->getPlugIn();

               // finish the item and mark it as complete
               pItem->finish();
               pItem->update();

               // detach so we don't get anymore updates
               Progress* pProgress(pItem->getProgress());
               if (pProgress != NULL)
               {
                  pProgress->detach(SIGNAL_NAME(Subject, Modified),
                     Slot(this, &BackgroundPluginWindow::progressChanged));
               }

               Executable* pExecutable = dynamic_cast<Executable*>(pPlugIn);
               if (pExecutable != NULL)
               {
                  bDestroy = pExecutable->isDestroyedAfterExecute();
               }

               changeSelection(); // update button status
            }
            else
            {
               pPlugIn = pCallback->getPlugIn();
            }

            delete pCallback;

            if (pPlugIn != NULL && bDestroy)
            {
               PlugInManagerServicesImp* pManager(PlugInManagerServicesImp::instance());
               if (pManager != NULL)
               {
                  pManager->destroyPlugIn(pPlugIn);
               }
            }
         }

         break;
      }

      default:
         break;
   }
}

void BackgroundPluginWindow::changeSelection()
{
   bool bRunning = false;
   bool bFinished = false;

   const QPalette& listPalette = mpStatusList->palette();

   for (int i = 0; i < mpStatusList->count(); ++i)
   {
      BackgroundPluginItem* pItem = static_cast<BackgroundPluginItem*>(mpStatusList->item(i));
      if (pItem != NULL)
      {
         QColor clrBackground = listPalette.color(QPalette::Base);
         QColor clrText = listPalette.color(QPalette::Text);

         if (mpStatusList->isItemSelected(pItem) == true)
         {
            clrBackground = listPalette.color(QPalette::Highlight);
            clrText = listPalette.color(QPalette::HighlightedText);

            if (pItem->isFinished() == true)
            {
               bFinished = true;
            }
            else
            {
               bRunning = true;
            }
         }

         pItem->setBackgroundColor(clrBackground);
         pItem->setTextColor(clrText);
      }
   }

   mpDismissButton->setEnabled(bFinished);
   mpAbortButton->setEnabled(bRunning);
}

void BackgroundPluginWindow::dismissClicked()
{
   QList<QListWidgetItem*> selectedItems = mpStatusList->selectedItems();
   for (int i = 0; i < selectedItems.count(); ++i)
   {
      BackgroundPluginItem* pItem = static_cast<BackgroundPluginItem*>(selectedItems[i]);
      if (pItem != NULL)
      {
         if (pItem->isFinished() == true)
         {
            delete pItem;
         }
      }
   }

   changeSelection();
}

void BackgroundPluginWindow::abortClicked()
{
   QList<QListWidgetItem*> selectedItems = mpStatusList->selectedItems();
   for (int i = 0; i < selectedItems.count(); ++i)
   {
      BackgroundPluginItem* pItem = static_cast<BackgroundPluginItem*>(selectedItems[i]);
      if (pItem != NULL)
      {
         if (pItem->isFinished() == false)
         {
            pItem->finish();
            pItem->update();
         }
      }
   }

   changeSelection();
}

void BackgroundPluginWindow::sessionClosed(Subject& subject, const std::string& signal, const boost::any& value)
{
   // Enable callbacks to be registered again now that the session is closed
   mSessionClosing = false;

   // Clear the destroyed plug-in items from the list
   mpStatusList->clear();
}

void BackgroundPluginWindow::sessionClosing(Subject& subject, const std::string& signal, const boost::any& value)
{
   // abort any background plug-ins that are running
   for (int i = 0; i < mpStatusList->count(); ++i)
   {
      BackgroundPluginItem* pItem = static_cast<BackgroundPluginItem*>(mpStatusList->item(i));
      if (pItem != NULL)
      {
         if (pItem->isFinished() == false)
         {
            pItem->finish();
            pItem->update();
         }
      }
   }

   // Immediately process all registered callbacks to force all background
   // plug-ins to finish processing before they are destroyed
   QCoreApplication::sendPostedEvents(this, BPW_PMODIFIED);
   QCoreApplication::sendPostedEvents(this, BPW_CALLBACK);

   // Prevent any callbacks from being registered while the session is closing
   mSessionClosing = true;
}
