/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QEvent>
#include <QtGui/QHeaderView>
#include <QtGui/QInputDialog>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>

#include "AddPlotDlg.h"
#include "AppVerify.h"
#include "Axis.h"
#include "CartesianGridlines.h"
#include "CartesianPlot.h"
#include "DesktopServices.h"
#include "DockWindow.h"
#include "DockWindowWidget.h"
#include "ElidedLabel.h"
#include "PlotManager.h"
#include "PlotPropertiesDlg.h"
#include "PlotSet.h"
#include "PlotSetGroup.h"
#include "PlotView.h"
#include "PlotWidget.h"
#include "PolarGridlines.h"
#include "PolarPlot.h"
#include "SessionManager.h"
#include "Slot.h"
#include "StringUtilities.h"
#include "TypesFile.h"
#include "WorkspaceWindow.h"

#include <string>
#include <vector>
using namespace std;

DockWindowWidget::DockWindowWidget(QWidget* pParent) :
   QWidget(pParent)
{
   // Plot windows
   QLabel* pWindowLabel = new QLabel("Plot Windows:", this);
   mpWindowList = new QListWidget(this);
   mpWindowList->setFixedWidth(150);
   mpWindowList->setSelectionMode(QAbstractItemView::SingleSelection);

   mpDeleteWindowCheck = new QCheckBox("Delete windows on close", this);

   QPushButton* pAddWindowButton = new QPushButton("Add...", this);
   QPushButton* pDeleteWindowButton = new QPushButton("Delete", this);
   QPushButton* pShowWindowButton = new QPushButton("Show", this);
   QPushButton* pHideWindowButton = new QPushButton("Hide", this);

   // Plots
   QLabel* pPlotLabel = new QLabel("Plots:", this);

   QStringList columnNames;
   columnNames.append("Name");
   columnNames.append("Type");

   mpPlotTree = new QTreeWidget(this);
   mpPlotTree->setColumnCount(columnNames.count());
   mpPlotTree->setHeaderLabels(columnNames);
   mpPlotTree->setRootIsDecorated(true);
   mpPlotTree->setSelectionMode(QAbstractItemView::SingleSelection);

   QHeaderView* pHeader = mpPlotTree->header();
   if (pHeader != NULL)
   {
      pHeader->setSortIndicatorShown(true);
      pHeader->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
      pHeader->setStretchLastSection(false);
      pHeader->resizeSection(0, 200);
   }

   QPushButton* pAddPlotButton = new QPushButton("Add...", this);
   QPushButton* pDeletePlotButton = new QPushButton("Delete", this);
   QPushButton* pActivateButton = new QPushButton("Activate", this);
   QPushButton* pRenameButton = new QPushButton("Rename...", this);
   QPushButton* pPropertiesButton = new QPushButton("Properties...", this);

   // Horizontal line
   QFrame* pHLine = new QFrame(this);
   pHLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   // Workspace window
   QLabel* pActiveWindowLabel = new QLabel("Active Workspace Window:", this);
   mpActiveWindowLabel = new ElidedLabel(this);
   mpActiveWindowLabel->setMinimumWidth(100);

   // Layout
   QVBoxLayout* pWindowLayout = new QVBoxLayout();
   pWindowLayout->setMargin(0);
   pWindowLayout->setSpacing(5);
   pWindowLayout->addWidget(pAddWindowButton);
   pWindowLayout->addWidget(pDeleteWindowButton);
   pWindowLayout->addWidget(pShowWindowButton);
   pWindowLayout->addWidget(pHideWindowButton);
   pWindowLayout->addStretch(10);

   QVBoxLayout* pPlotLayout = new QVBoxLayout();
   pPlotLayout->setMargin(0);
   pPlotLayout->setSpacing(5);
   pPlotLayout->addWidget(pAddPlotButton);
   pPlotLayout->addWidget(pDeletePlotButton);
   pPlotLayout->addWidget(pActivateButton);
   pPlotLayout->addWidget(pRenameButton);
   pPlotLayout->addStretch(10);
   pPlotLayout->addWidget(pPropertiesButton);

   QHBoxLayout* pActiveLayout = new QHBoxLayout();
   pActiveLayout->setMargin(0);
   pActiveLayout->setSpacing(5);
   pActiveLayout->addWidget(pActiveWindowLabel);
   pActiveLayout->addWidget(mpActiveWindowLabel, 10);

   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(7);
   pGrid->setSpacing(10);
   pGrid->addWidget(pWindowLabel, 0, 0);
   pGrid->addWidget(mpWindowList, 1, 0);
   pGrid->addWidget(mpDeleteWindowCheck, 2, 0);
   pGrid->addLayout(pWindowLayout, 1, 1, 2, 1);
   pGrid->addWidget(pPlotLabel, 0, 2);
   pGrid->addWidget(mpPlotTree, 1, 2, 2, 1);
   pGrid->addLayout(pPlotLayout, 1, 3, 2, 1);
   pGrid->addWidget(pHLine, 3, 0, 1, 4);
   pGrid->addLayout(pActiveLayout, 4, 0, 1, 4);
   pGrid->setRowStretch(1, 10);
   pGrid->setColumnStretch(2, 10);

   // Initialization
   WorkspaceWindow* pWindow = mpDesktop->getCurrentWorkspaceWindow();
   if (pWindow != NULL)
   {
      const string& windowName = pWindow->getDisplayName(true);
      if (windowName.empty() == false)
      {
         mpActiveWindowLabel->setText(QString::fromStdString(windowName));
      }
   }

   VERIFYNR(mpDesktop->attach(SIGNAL_NAME(DesktopServices, WindowAdded), Slot(this, &DockWindowWidget::windowAdded)));
   VERIFYNR(mpDesktop->attach(SIGNAL_NAME(DesktopServices, WindowActivated),
      Slot(this, &DockWindowWidget::windowActivated)));
   VERIFYNR(mpDesktop->attach(SIGNAL_NAME(DesktopServices, WindowRemoved),
      Slot(this, &DockWindowWidget::windowRemoved)));
   VERIFYNR(Service<SessionManager>()->attach(SIGNAL_NAME(SessionManager, SessionRestored),
      Slot(this, &DockWindowWidget::sessionRestored)));

   updatePlotWindowList();

   // Connections
   VERIFYNR(connect(mpWindowList, SIGNAL(itemSelectionChanged()), this, SLOT(updatePlotList())));
   VERIFYNR(connect(pAddWindowButton, SIGNAL(clicked()), this, SLOT(addPlotWindow())));
   VERIFYNR(connect(pDeleteWindowButton, SIGNAL(clicked()), this, SLOT(deletePlotWindow())));
   VERIFYNR(connect(pShowWindowButton, SIGNAL(clicked()), this, SLOT(showPlotWindow())));
   VERIFYNR(connect(pHideWindowButton, SIGNAL(clicked()), this, SLOT(hidePlotWindow())));
   VERIFYNR(connect(pAddPlotButton, SIGNAL(clicked()), this, SLOT(addPlot())));
   VERIFYNR(connect(pDeletePlotButton, SIGNAL(clicked()), this, SLOT(deletePlot())));
   VERIFYNR(connect(pActivateButton, SIGNAL(clicked()), this, SLOT(activatePlot())));
   VERIFYNR(connect(pRenameButton, SIGNAL(clicked()), this, SLOT(renamePlot())));
   VERIFYNR(connect(pPropertiesButton, SIGNAL(clicked()), this, SLOT(editPlotProperties())));
}

DockWindowWidget::~DockWindowWidget()
{
   deleteAllPlotWindows();

   // Detach from the dock windows that do not contain plot sets
   vector<Window*> dockWindows;
   mpDesktop->getWindows(DOCK_WINDOW, dockWindows);
   for (vector<Window*>::const_iterator iter = dockWindows.begin(); iter != dockWindows.end(); ++iter)
   {
      DockWindow* pDockWindow = dynamic_cast<DockWindow*>(*iter);
      if (pDockWindow != NULL)
      {
         pDockWindow->detach(SIGNAL_NAME(ViewWindow, WidgetSet), Slot(this, &DockWindowWidget::dockWindowWidgetSet));
      }
   }

   VERIFYNR(mpDesktop->detach(SIGNAL_NAME(DesktopServices, WindowAdded), Slot(this, &DockWindowWidget::windowAdded)));
   VERIFYNR(mpDesktop->detach(SIGNAL_NAME(DesktopServices, WindowActivated),
      Slot(this, &DockWindowWidget::windowActivated)));
   VERIFYNR(mpDesktop->detach(SIGNAL_NAME(DesktopServices, WindowRemoved),
      Slot(this, &DockWindowWidget::windowRemoved)));
   VERIFYNR(Service<SessionManager>()->detach(SIGNAL_NAME(SessionManager, SessionRestored),
      Slot(this, &DockWindowWidget::sessionRestored)));
}

void DockWindowWidget::hideEvent(QHideEvent* pEvent)
{
   if (mpDeleteWindowCheck->isChecked() == true)
   {
      deleteAllPlotWindows();
   }

   QWidget::hideEvent(pEvent);
}

void DockWindowWidget::windowAdded(Subject& subject, const string& signal, const boost::any& value)
{
   if (dynamic_cast<DesktopServices*>(&subject) == mpDesktop.get())
   {
      DockWindow* pDockWindow = dynamic_cast<DockWindow*>(boost::any_cast<Window*>(value));
      if (pDockWindow != NULL)
      {
         VERIFYNR(pDockWindow->attach(SIGNAL_NAME(ViewWindow, WidgetSet),
            Slot(this, &DockWindowWidget::dockWindowWidgetSet)));
      }
   }
}

void DockWindowWidget::windowActivated(Subject& subject, const string& signal, const boost::any& value)
{
   if (dynamic_cast<DesktopServices*>(&subject) == mpDesktop.get())
   {
      string windowName;

      WorkspaceWindow* pWindow = boost::any_cast<WorkspaceWindow*>(value);
      if (pWindow != NULL)
      {
         windowName = pWindow->getDisplayName(true);
      }

      mpActiveWindowLabel->setText(QString::fromStdString(windowName));
   }
}

void DockWindowWidget::windowRemoved(Subject& subject, const string& signal, const boost::any& value)
{
   if (dynamic_cast<DesktopServices*>(&subject) == mpDesktop.get())
   {
      DockWindow* pDockWindow = dynamic_cast<DockWindow*>(boost::any_cast<Window*>(value));
      if (pDockWindow != NULL)
      {
         // Detach from the dock window
         pDockWindow->detach(SIGNAL_NAME(ViewWindow, WidgetSet), Slot(this, &DockWindowWidget::dockWindowWidgetSet));

         // If the window contains plot sets, update the plot window list
         if (dynamic_cast<PlotSetGroup*>(pDockWindow->getWidget()) != NULL)
         {
            updatePlotWindowList();
         }
      }
   }
}

void DockWindowWidget::dockWindowWidgetSet(Subject& subject, const string& signal, const boost::any& value)
{
   QWidget* pWidget = boost::any_cast<QWidget*>(value);
   if (dynamic_cast<PlotSetGroup*>(pWidget) != NULL)
   {
      updatePlotWindowList();
   }
}

void DockWindowWidget::sessionRestored(Subject& subject, const string& signal, const boost::any& value)
{
   vector<Window*> dockWindows;
   mpDesktop->getWindows(DOCK_WINDOW, dockWindows);
   for (vector<Window*>::const_iterator iter = dockWindows.begin(); iter != dockWindows.end(); ++iter)
   {
      DockWindow* pDockWindow = dynamic_cast<DockWindow*>(*iter);
      if (pDockWindow != NULL)
      {
         VERIFYNR(pDockWindow->attach(SIGNAL_NAME(ViewWindow, WidgetSet),
            Slot(this, &DockWindowWidget::dockWindowWidgetSet)));
      }
   }

   updatePlotWindowList();
}

void DockWindowWidget::setSelectedPlotWindow(DockWindow* pWindow)
{
   mpWindowList->clearSelection();

   if ((pWindow != NULL) && (dynamic_cast<PlotSetGroup*>(pWindow->getWidget()) != NULL))
   {
      for (int i = 0; i < mpWindowList->count(); i++)
      {
         QListWidgetItem* pItem = mpWindowList->item(i);
         if (pItem != NULL)
         {
            QString strWindowName = pItem->text();
            if (strWindowName.isEmpty() == false)
            {
               DockWindow* pCurrentWindow = static_cast<DockWindow*>(mpDesktop->getWindow(strWindowName.toStdString(),
                  DOCK_WINDOW));
               if (pCurrentWindow == pWindow)
               {
                  mpWindowList->setItemSelected(pItem, true);
                  break;
               }
            }
         }
      }
   }
}

void DockWindowWidget::setSelectedPlotSet(PlotSet* pPlotSet)
{
   mpPlotTree->clearSelection();

   if (pPlotSet == NULL)
   {
      return;
   }

   string plotSetName = pPlotSet->getName();
   if (plotSetName.empty() == true)
   {
      return;
   }

   QString strPlotSet = QString::fromStdString(plotSetName);

   QTreeWidgetItemIterator iter(mpPlotTree);
   while (*iter != NULL)
   {
      QTreeWidgetItem* pItem = *iter;
      if (pItem != NULL)
      {
         if (pItem->parent() == NULL)
         {
            QString strCurrentPlotSet = pItem->text(0);
            if (strCurrentPlotSet == strPlotSet)
            {
               mpPlotTree->setItemSelected(pItem, true);
               break;
            }
         }
      }

      ++iter;
   }
}

void DockWindowWidget::setSelectedPlot(PlotWidget* pPlot)
{
   mpPlotTree->clearSelection();

   PlotSetGroup* pPlotSetGroup = NULL;

   DockWindow* pWindow = getSelectedPlotWindow();
   if (pWindow != NULL)
   {
      pPlotSetGroup = dynamic_cast<PlotSetGroup*>(pWindow->getWidget());
   }

   if (pPlotSetGroup == NULL)
   {
      return;
   }

   if (pPlot != NULL)
   {
      QTreeWidgetItemIterator iter(mpPlotTree);
      while (*iter != NULL)
      {
         QTreeWidgetItem* pItem = *iter;
         if (pItem != NULL)
         {
            QTreeWidgetItem* pPlotSetItem = pItem->parent();
            if (pPlotSetItem != NULL)
            {
               QString strPlotSet = pPlotSetItem->text(0);
               QString strPlotName = pItem->text(0);

               PlotSet* pPlotSet = pPlotSetGroup->getPlotSet(strPlotSet.toStdString());
               if ((pPlotSet != NULL) && (strPlotName.isEmpty() == false))
               {
                  string plotName = strPlotName.toStdString();

                  PlotWidget* pCurrentPlot = pPlotSet->getPlot(plotName);
                  if (pCurrentPlot == pPlot)
                  {
                     mpPlotTree->setItemSelected(pItem, true);
                     mpPlotTree->expandItem(pPlotSetItem);
                     break;
                  }
               }
            }
         }

         ++iter;
      }
   }
}

DockWindow* DockWindowWidget::getSelectedPlotWindow() const
{
   DockWindow* pWindow = NULL;

   QList<QListWidgetItem*> items = mpWindowList->selectedItems();
   if (items.empty() == false)
   {
      QListWidgetItem* pItem = items.front();
      if (pItem != NULL)
      {
         QString strWindow = pItem->text();
         if (strWindow.isEmpty() == false)
         {
            pWindow = static_cast<DockWindow*>(mpDesktop->getWindow(strWindow.toStdString(), DOCK_WINDOW));
         }
      }
   }

   return pWindow;
}

PlotWidget* DockWindowWidget::getSelectedPlot() const
{
   QString strPlotSet;
   QString strPlot;

   QTreeWidgetItem* pItem = NULL;

   QList<QTreeWidgetItem*> items = mpPlotTree->selectedItems();
   if (items.empty() == false)
   {
      pItem = items.front();
   }

   if (pItem != NULL)
   {
      QTreeWidgetItem* pParentItem = pItem->parent();
      if (pParentItem != NULL)
      {
         strPlotSet = pParentItem->text(0);
         strPlot = pItem->text(0);
      }
   }

   if (strPlot.isEmpty() == true)
   {
      return NULL;
   }

   PlotWidget* pPlot = NULL;

   DockWindow* pWindow = getSelectedPlotWindow();
   if (pWindow != NULL)
   {
      PlotSetGroup* pPlotSetGroup = dynamic_cast<PlotSetGroup*>(pWindow->getWidget());
      if (pPlotSetGroup != NULL)
      {
         PlotSet* pPlotSet = pPlotSetGroup->getPlotSet(strPlotSet.toStdString());
         if (pPlotSet != NULL)
         {
            pPlot = pPlotSet->getPlot(strPlot.toStdString());
         }
      }
   }

   return pPlot;
}

void DockWindowWidget::addPlotWindow()
{
   QString strName = QInputDialog::getText(this, "Select Plot Window Name", "Name:", QLineEdit::Normal);
   if (strName.isEmpty() == true)
   {
      return;
   }

   DockWindow* pWindow = static_cast<DockWindow*>(mpDesktop->createWindow(strName.toStdString(), DOCK_WINDOW));
   if (pWindow != NULL)
   {
      PlotSetGroup* pPlotSetGroup = mpDesktop->createPlotSetGroup();
      if (pPlotSetGroup == NULL)
      {
         mpDesktop->deleteWindow(pWindow);
         return;
      }

      pWindow->setWidget(pPlotSetGroup->getWidget());
      setSelectedPlotWindow(pWindow);
   }
}

void DockWindowWidget::deletePlotWindow()
{
   DockWindow* pWindow = getSelectedPlotWindow();
   if (pWindow == NULL)
   {
      QMessageBox::critical(this, PLOT_MANAGER_NAME, "Please select a plot window to delete!");
      return;
   }

   if (mpDesktop->deleteWindow(pWindow) == false)
   {
      QMessageBox::critical(this, PLOT_MANAGER_NAME, "Could not delete the plot window!");
   }
}

void DockWindowWidget::showPlotWindow()
{
   DockWindow* pWindow = getSelectedPlotWindow();
   if (pWindow != NULL)
   {
      pWindow->show();
   }
}

void DockWindowWidget::hidePlotWindow()
{
   DockWindow* pWindow = getSelectedPlotWindow();
   if (pWindow != NULL)
   {
      pWindow->hide();
   }
}

void DockWindowWidget::updatePlotWindowList()
{
   DockWindow* pCurrentDockWindow = getSelectedPlotWindow();
   mpWindowList->clear();

   vector<Window*> dockWindows;
   mpDesktop->getWindows(DOCK_WINDOW, dockWindows);

   for (vector<Window*>::iterator iter = dockWindows.begin(); iter != dockWindows.end(); ++iter)
   {
      DockWindow* pWindow = dynamic_cast<DockWindow*>(*iter);
      if ((pWindow != NULL) && (dynamic_cast<PlotSetGroup*>(pWindow->getWidget())))
      {
         string windowName = pWindow->getName();
         if (windowName.empty() == false)
         {
            QString strWindowName = QString::fromStdString(windowName);
            mpWindowList->addItem(strWindowName);
         }
      }
   }

   setSelectedPlotWindow(pCurrentDockWindow);
   updatePlotList();
}

void DockWindowWidget::deleteAllPlotWindows()
{
   vector<Window*> plotWindows;
   mpDesktop->getWindows(DOCK_WINDOW, plotWindows);

   for (vector<Window*>::iterator iter = plotWindows.begin(); iter != plotWindows.end(); ++iter)
   {
      DockWindow* pWindow = static_cast<DockWindow*>(*iter);
      if ((pWindow != NULL) && (dynamic_cast<PlotSetGroup*>(pWindow->getWidget()) != NULL))
      {
         mpDesktop->deleteWindow(pWindow);
      }
   }
}

void DockWindowWidget::addPlot()
{
   PlotSetGroup* pPlotSetGroup = NULL;

   DockWindow* pWindow = getSelectedPlotWindow();
   if (pWindow != NULL)
   {
      pPlotSetGroup = dynamic_cast<PlotSetGroup*>(pWindow->getWidget());
   }

   if (pPlotSetGroup == NULL)
   {
      QMessageBox::critical(this, PLOT_MANAGER_NAME, "Please select a plot window for which to add a plot!");
      return;
   }

   QTreeWidgetItem* pItem = NULL;

   QList<QTreeWidgetItem*> items = mpPlotTree->selectedItems();
   if (items.empty() == false)
   {
      pItem = items.front();
   }

   if (pItem == NULL)
   {
      QString strPlotSet = QInputDialog::getText(this, "Add Plot Set", "Name:", QLineEdit::Normal);
      if (strPlotSet.isEmpty() == false)
      {
         PlotSet* pPlotSet = pPlotSetGroup->createPlotSet(strPlotSet.toStdString());
         if (pPlotSet != NULL)
         {
            updatePlotList();
            setSelectedPlotSet(pPlotSet);
         }
      }
   }
   else
   {
      AddPlotDlg plotDlg(this);

      int iReturn = plotDlg.exec();
      if (iReturn == QDialog::Rejected)
      {
         return;
      }

      QString strPlotSet;

      QTreeWidgetItem* pPlotSetItem = pItem->parent();
      if (pPlotSetItem == NULL)
      {
         strPlotSet = pItem->text(0);
      }
      else
      {
         strPlotSet = pPlotSetItem->text(0);
      }

      PlotSet* pPlotSet = NULL;
      if (strPlotSet.isEmpty() == false)
      {
         pPlotSet = pPlotSetGroup->getPlotSet(strPlotSet.toStdString());
      }

      if (pPlotSet != NULL)
      {
         PlotType eType = plotDlg.getType();
         QString strPlotName = plotDlg.getName();

         if (strPlotName.isEmpty() == false)
         {
            string plotName = strPlotName.toStdString();

            PlotWidget* pPlot = pPlotSet->createPlot(plotName, eType);
            if (pPlot != NULL)
            {
               // Gridlines
               bool bGridlines = plotDlg.useGridlines();

               CartesianPlot* pCartesianPlot = dynamic_cast<CartesianPlot*>(pPlot->getPlot());
               if (pCartesianPlot != NULL)
               {
                  CartesianGridlines* pGridlines = pCartesianPlot->getGridlines(HORIZONTAL);
                  if (pGridlines != NULL)
                  {
                     pGridlines->setVisible(bGridlines);
                  }

                  pGridlines = pCartesianPlot->getGridlines(VERTICAL);
                  if (pGridlines != NULL)
                  {
                     pGridlines->setVisible(bGridlines);
                  }
               }

               PolarPlot* pPolarPlot = dynamic_cast<PolarPlot*>(pPlot->getPlot());
               if (pPolarPlot != NULL)
               {
                  PolarGridlines* pGridlines = pPolarPlot->getGridlines();
                  if (pGridlines != NULL)
                  {
                     pGridlines->setVisible(bGridlines);
                  }
               }

               // Legend
               bool bLegend = plotDlg.useLegend();
               pPlot->showLegend(bLegend);

               Axis* pXAxis = pPlot->getAxis(AXIS_BOTTOM);
               if (pXAxis != NULL)
               {
                  QString strXAxis = plotDlg.getAxisText(AXIS_BOTTOM);
                  if (strXAxis.isEmpty() == false)
                  {
                     string axisName = strXAxis.toStdString();
                     pXAxis->setTitle(axisName);
                  }
               }

               Axis* pYAxis = pPlot->getAxis(AXIS_LEFT);
               if (pYAxis != NULL)
               {
                  QString strYAxis = plotDlg.getAxisText(AXIS_LEFT);
                  if (strYAxis.isEmpty() == false)
                  {
                     string axisName = strYAxis.toStdString();
                     pYAxis->setTitle(axisName);
                  }
               }

               plotDlg.setPlot(pPlot);

               updatePlotList();
               setSelectedPlot(pPlot);
            }
         }
      }
   }
}

void DockWindowWidget::deletePlot()
{
   PlotSetGroup* pPlotSetGroup = NULL;

   DockWindow* pWindow = getSelectedPlotWindow();
   if (pWindow != NULL)
   {
      pPlotSetGroup = dynamic_cast<PlotSetGroup*>(pWindow->getWidget());
   }

   if (pPlotSetGroup == NULL)
   {
      QMessageBox::critical(this, PLOT_MANAGER_NAME,
         "Please select a plot window for which to delete a plot set or plot!");
      return;
   }

   QTreeWidgetItem* pItem = NULL;

   QList<QTreeWidgetItem*> items = mpPlotTree->selectedItems();
   if (items.empty() == false)
   {
      pItem = items.front();
   }

   if (pItem == NULL)
   {
      QMessageBox::critical(this, PLOT_MANAGER_NAME, "Please select a plot set or plot to delete!");
      return;
   }

   bool bSuccess = false;

   if (pItem->parent() != NULL)
   {
      PlotWidget* pPlot = getSelectedPlot();
      if (pPlot != NULL)
      {
         PlotSet* pPlotSet = pPlotSetGroup->getPlotSet(pPlot);
         if (pPlotSet != NULL)
         {
            bSuccess = pPlotSet->deletePlot(pPlot);
         }
      }
   }
   else
   {
      QString strPlotSet = pItem->text(0);
      if (strPlotSet.isEmpty() == false)
      {
         PlotSet* pPlotSet = pPlotSetGroup->getPlotSet(strPlotSet.toStdString());
         if (pPlotSet != NULL)
         {
            bSuccess = pPlotSetGroup->deletePlotSet(pPlotSet);
         }
      }
   }

   if (bSuccess == true)
   {
      updatePlotList();
      setSelectedPlot(pPlotSetGroup->getCurrentPlot());
   }
}

void DockWindowWidget::updatePlotList()
{
   mpPlotTree->clear();

   DockWindow* pWindow = getSelectedPlotWindow();
   if (pWindow == NULL)
   {
      return;
   }

   PlotSetGroup* pPlotSetGroup = dynamic_cast<PlotSetGroup*>(pWindow->getWidget());
   if (pPlotSetGroup == NULL)
   {
      return;
   }

   const vector<PlotSet*>& plotSets = pPlotSetGroup->getPlotSets();
   for (vector<PlotSet*>::const_iterator iter = plotSets.begin(); iter != plotSets.end(); ++iter)
   {
      PlotSet* pPlotSet = *iter;
      if (pPlotSet != NULL)
      {
         string plotSetName = pPlotSet->getName();

         QTreeWidgetItem* pItem = new QTreeWidgetItem(mpPlotTree);
         if (pItem != NULL)
         {
            pItem->setText(0, QString::fromStdString(plotSetName));

            vector<PlotWidget*> plots;
            pPlotSet->getPlots(plots);

            for (unsigned int i = 0; i < plots.size(); i++)
            {
               PlotWidget* pPlot = plots[i];
               if (pPlot != NULL)
               {
                  PlotView* pPlotView = pPlot->getPlot();
                  if (pPlotView != NULL)
                  {
                     const string& plotName = pPlotView->getName();
                     if (plotName.empty() == false)
                     {
                        QTreeWidgetItem* pPlotItem = new QTreeWidgetItem(pItem);
                        if (pPlotItem != NULL)
                        {
                           PlotType eType = pPlotView->getPlotType();
                           string plotType = StringUtilities::toDisplayString(eType);

                           pPlotItem->setText(0, QString::fromStdString(plotName));
                           pPlotItem->setText(1, QString::fromStdString(plotType));
                        }
                     }
                  }
               }
            }
         }
      }
   }
}

void DockWindowWidget::activatePlot()
{
   PlotSetGroup* pPlotSetGroup = NULL;

   DockWindow* pWindow = getSelectedPlotWindow();
   if (pWindow != NULL)
   {
      pPlotSetGroup = dynamic_cast<PlotSetGroup*>(pWindow->getWidget());
   }

   if (pPlotSetGroup == NULL)
   {
      QMessageBox::critical(this, PLOT_MANAGER_NAME,
         "Please select a plot window for which to activate a plot set or plot!");
      return;
   }

   QTreeWidgetItem* pItem = NULL;

   QList<QTreeWidgetItem*> items = mpPlotTree->selectedItems();
   if (items.empty() == false)
   {
      pItem = items.front();
   }

   if (pItem == NULL)
   {
      QMessageBox::critical(this, PLOT_MANAGER_NAME, "Please select a plot set or plot to activate!");
      return;
   }

   if (pItem->parent() != NULL)
   {
      PlotWidget* pPlot = getSelectedPlot();
      if (pPlot != NULL)
      {
         pPlotSetGroup->setCurrentPlot(pPlot);
      }
   }
   else
   {
      QString strPlotSet = pItem->text(0);
      if (strPlotSet.isEmpty() == false)
      {
         PlotSet* pPlotSet = pPlotSetGroup->getPlotSet(strPlotSet.toStdString());
         if (pPlotSet != NULL)
         {
            pPlotSetGroup->setCurrentPlotSet(pPlotSet);
         }
      }
   }
}

void DockWindowWidget::renamePlot()
{
   PlotSetGroup* pPlotSetGroup = NULL;

   DockWindow* pWindow = getSelectedPlotWindow();
   if (pWindow != NULL)
   {
      pPlotSetGroup = dynamic_cast<PlotSetGroup*>(pWindow->getWidget());
   }

   if (pPlotSetGroup == NULL)
   {
      QMessageBox::critical(this, PLOT_MANAGER_NAME,
         "Please select a plot window for which to rename a plot set or plot!");
      return;
   }

   QTreeWidgetItem* pItem = NULL;

   QList<QTreeWidgetItem*> items = mpPlotTree->selectedItems();
   if (items.empty() == false)
   {
      pItem = items.front();
   }

   if (pItem == NULL)
   {
      QMessageBox::critical(this, PLOT_MANAGER_NAME, "Please select a plot set or plot to rename!");
      return;
   }

   if (pItem->parent() != NULL)
   {
      PlotWidget* pPlot = getSelectedPlot();
      if (pPlot != NULL)
      {
         QString strCurrentName;

         PlotView* pPlotView = pPlot->getPlot();
         if (pPlotView != NULL)
         {
            string currentPlotName = pPlotView->getName();
            if (currentPlotName.empty() == false)
            {
               strCurrentName = QString::fromLatin1(currentPlotName.c_str());
            }
         }

         QString strPlotName = QInputDialog::getText(this, "Select Plot Name", "Name:", QLineEdit::Normal,
            strCurrentName);
         if ((strPlotName.isEmpty() == false) && (strPlotName != strCurrentName))
         {
            string newPlotName = strPlotName.toStdString();

            PlotSet* pPlotSet = pPlotSetGroup->getPlotSet(pPlot);
            if (pPlotSet != NULL)
            {
               bool bSuccess = pPlotSet->renamePlot(pPlot, newPlotName);
               if (bSuccess == false)
               {
                  QMessageBox::critical(this, PLOT_MANAGER_NAME, "Could not rename the plot because a plot with the " +
                     strPlotName + " name already exists!");
               }
               else
               {
                  pItem->setText(0, strPlotName);
               }
            }
         }
      }
   }
   else
   {
      QString strPlotSet = pItem->text(0);

      PlotSet* pPlotSet = pPlotSetGroup->getPlotSet(strPlotSet.toStdString());
      if (pPlotSet != NULL)
      {
         QString strNewPlotSet = QInputDialog::getText(this, "Select Plot Set Name", "Name:", QLineEdit::Normal,
            strPlotSet);
         if ((strNewPlotSet.isEmpty() == false) && (strNewPlotSet != strPlotSet))
         {
            bool bSuccess = pPlotSetGroup->renamePlotSet(pPlotSet, strNewPlotSet.toStdString());
            if (bSuccess == false)
            {
               QMessageBox::critical(this, PLOT_MANAGER_NAME,
                  "Could not rename the plot set because a plot set with the " + strPlotSet + " name already exists!");
            }
            else
            {
               pItem->setText(0, strNewPlotSet);
            }
         }
      }
   }
}

void DockWindowWidget::editPlotProperties()
{
   if (getSelectedPlotWindow() == NULL)
   {
      QMessageBox::critical(this, PLOT_MANAGER_NAME,
         "Please select a plot window for which to edit a plot's properties!");
      return;
   }

   PlotWidget* pPlot = getSelectedPlot();
   if (pPlot == NULL)
   {
      QMessageBox::critical(this, PLOT_MANAGER_NAME, "Please select a plot for which to edit its properties!");
      return;
   }

   try
   {
      PlotPropertiesDlg propDlg(pPlot, this);
      propDlg.exec();
   }
   catch (AssertException except)
   {
      QMessageBox::critical(this, PLOT_MANAGER_NAME, except.getText().c_str());
      return;
   }
}
