/*
 * The information in this file is
 * Copyright(c) 2015 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "OptionsFullScreen.h"

#include <algorithm>
#include <string>
#include <vector>

#include <QtGui/QComboBox>
#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QListWidget>
#include <QtGui/QListWidgetItem>
#include <QtGui/QVBoxLayout>

#include "AppVerify.h"
#include "ConfigurationSettings.h"
#include "DesktopServices.h"
#include "DockWindow.h"
#include "LabeledSection.h"
#include "Service.h"
#include "ToolBar.h"
#include "Window.h"

OptionsFullScreen::OptionsFullScreen() :
   QWidget(NULL),
   mpDockWindows(NULL),
   mpToolBars(NULL),
   mpWindowLayout(NULL)
{
   // Add a widget and grid layout for requesting user specify which toolbars and dock windows to keep displayed.
   QWidget* pWindowsWidget = new QWidget(this);
   QGridLayout* pWindowsLayout = new QGridLayout(pWindowsWidget);
   pWindowsLayout->setMargin(0);
   pWindowsLayout->setSpacing(5);

   // Add a group box, layout and widget for the dock windows.
   QGroupBox* pDockWindowsGroupBox = new QGroupBox("Dock Windows", this);
   QVBoxLayout* pDockWindowsLayout = new QVBoxLayout(pDockWindowsGroupBox);
   mpDockWindows = new QListWidget(pDockWindowsGroupBox);
   pDockWindowsLayout->addWidget(mpDockWindows);

   // Get alphabetized dock window names.
   // Limitation: This will not get the names of tearable dock window tabs, if the tabs have not yet been torn yet.
   //    Believe this should be a training issue, and user would have to tear the tabs before displaying the Options
   //    dialog to specify that torn tab not be hidden.
   std::vector<Window*> dockWindows;
   std::vector<std::string> dockWindowNames;
   Service<DesktopServices> pDesktop;
   pDesktop->getWindows(DOCK_WINDOW, dockWindows);
   for (std::vector<Window*>::const_iterator iter = dockWindows.begin(); iter != dockWindows.end(); ++iter)
   {
      Window* pWindow = *iter;
      VERIFYNR(NULL != pWindow);
      std::string name = pWindow->getName();
      dockWindowNames.push_back(name);
   }
   std::sort(dockWindowNames.begin(), dockWindowNames.end());

   // Get previously defined list of 'stay visible dock windows'.
   const std::vector<std::string> defaults;
   const std::vector<std::string>* pStayVisibleDockWindowNames = OptionsFullScreen::getSettingDockWindowsStayVisible();
   VERIFYNR(NULL != pStayVisibleDockWindowNames);
   if (NULL == pStayVisibleDockWindowNames)
   {
      pStayVisibleDockWindowNames = &defaults;
   }

   // Add the dock windows names to dock windows list widget.
   for (std::vector<std::string>::const_iterator iter = dockWindowNames.begin(); iter != dockWindowNames.end(); ++iter)
   {
      // Add the dock window name with a checkbox.
      QString name(QString::fromStdString(*iter));
      QListWidgetItem* pItem = new QListWidgetItem(name, mpDockWindows);
      pItem->setFlags(pItem->flags() | Qt::ItemIsUserCheckable); // set checkable flag

      // Set check state to match config settings.
      Qt::CheckState checked = Qt::Unchecked;
      std::string nameStr = name.toStdString();
      if (pStayVisibleDockWindowNames->end() != std::find(pStayVisibleDockWindowNames->begin(), pStayVisibleDockWindowNames->end(), nameStr))
      {
         checked = Qt::Checked;
      }
      pItem->setCheckState(checked); // AND initialize check state

      mpDockWindows->addItem(pItem);
   }

   // Add a group box, layout and widget for the toolbars.
   QGroupBox* pToolBarsGroupBox = new QGroupBox("Tool Bars", this);
   QVBoxLayout* pToolBarsLayout = new QVBoxLayout(pToolBarsGroupBox);
   mpToolBars = new QListWidget(this);
   pToolBarsLayout->addWidget(mpToolBars);

   // Get alphabetized toolbar names.
   std::vector<Window*> toolbars;
   std::vector<std::string> toolbarNames;
   pDesktop->getWindows(TOOLBAR, toolbars);
   for (std::vector<Window*>::const_iterator iter = toolbars.begin(); iter != toolbars.end(); ++iter)
   {
      Window* pWindow = *iter;
      VERIFYNR(NULL != pWindow);
      std::string name = pWindow->getName();
      toolbarNames.push_back(name);
   }
   std::sort(toolbarNames.begin(), toolbarNames.end());

   // Get previously defined list of 'stay visible toolbars'.
   const std::vector<std::string>* pStayVisibleToolBarNames = OptionsFullScreen::getSettingToolBarsStayVisible();
   VERIFYNR(NULL != pStayVisibleToolBarNames);
   if (NULL == pStayVisibleToolBarNames)
   {
      pStayVisibleToolBarNames = &defaults;
   }

   // Add the toolbar names to toolbars list widget.
   for (std::vector<std::string>::const_iterator iter = toolbarNames.begin(); iter != toolbarNames.end(); ++iter)
   {
      // Add the toolbar name with a checkbox.
      QString name(QString::fromStdString(*iter));
      QListWidgetItem* pItem = new QListWidgetItem(name, mpToolBars);
      pItem->setFlags(pItem->flags() | Qt::ItemIsUserCheckable); // set checkable flag

      // Set check state to match config settings.
      Qt::CheckState checked = Qt::Unchecked;
      std::string nameStr = name.toStdString();
      if (pStayVisibleToolBarNames->end() != std::find(pStayVisibleToolBarNames->begin(), pStayVisibleToolBarNames->end(), nameStr))
      {
         checked = Qt::Checked;
      }
      pItem->setCheckState(checked); // AND initialize check state

      mpToolBars->addItem(pItem);
   }

   // Add the dock window and toolbar group boxes to the windows labeled section.
   pWindowsLayout->addWidget(pDockWindowsGroupBox, 0, 0, Qt::AlignLeft);
   pWindowsLayout->addWidget(pToolBarsGroupBox, 1, 0, Qt::AlignLeft);
   pWindowsLayout->setColumnStretch(1, 10);
   LabeledSection* pWindowsSection = new LabeledSection(pWindowsWidget, "Check To Remain Visible, While In Full Screen Mode", this);

   // Add labeled section handle how windows will be laid out, when toggling in/out of full screen mode.
   QWidget* pWindowLayoutWidget = new QWidget(this);
   QGridLayout* pWindowLayoutLayout = new QGridLayout(pWindowLayoutWidget);
   pWindowLayoutLayout->setMargin(0);
   pWindowLayoutLayout->setSpacing(5);
   QLabel* pWindowLayoutLabel = new QLabel("Window Tile Method:", pWindowLayoutWidget);
   mpWindowLayout = new QComboBox(pWindowLayoutWidget);
   mpWindowLayout->addItem("Tile");
   mpWindowLayout->addItem("Tile Horizontally");
   mpWindowLayout->addItem("Tile Vertically");
   mpWindowLayout->addItem("Maximize Active");
   pWindowLayoutLayout->addWidget(pWindowLayoutLabel, 0, 0, Qt::AlignLeft);
   pWindowLayoutLayout->addWidget(mpWindowLayout, 0, 1, Qt::AlignLeft);
   LabeledSection* pWindowLayoutSection = new LabeledSection(pWindowLayoutWidget, "Workspace Window Layout", this);

   // Set workspace window layout method according to config setting.
   const std::string defaultLayout = "Maximize Active";
   const std::string* pSelectedWindowLayout = &defaultLayout;
   if (hasSettingWorkspaceWindowLayout())
   {
      pSelectedWindowLayout = getSettingWorkspaceWindowLayout();
   }
   VERIFYNRV(NULL != pSelectedWindowLayout);
   QString selectedText = QString::fromStdString(*pSelectedWindowLayout);
   int selectedIndex = mpWindowLayout->findText(selectedText);
   VERIFYNRV(selectedIndex < mpWindowLayout->count() && selectedIndex >= 0);
   mpWindowLayout->setCurrentIndex(selectedIndex);

   // Dialog layout
   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(10);
   pLayout->addWidget(pWindowsSection);
   pLayout->addWidget(pWindowLayoutSection);
   pLayout->addStretch(10);
}
   
void OptionsFullScreen::applyChanges()
{
   VERIFYNRV(NULL != mpToolBars);
   VERIFYNRV(NULL != mpDockWindows);
   VERIFYNRV(NULL != mpWindowLayout);
   std::vector<std::string> checkedToolBars;
   for (int idx = 0; idx < mpToolBars->count(); ++idx)
   {
      QListWidgetItem* pItem = mpToolBars->item(idx);
      VERIFYNRV(NULL != pItem);
      if (pItem->checkState() == Qt::Checked)
      {
         std::string text = pItem->text().toStdString();
         checkedToolBars.push_back(text);
      }
   }
   OptionsFullScreen::setSettingToolBarsStayVisible(&checkedToolBars);

   std::vector<std::string> checkedDockWindows;
   for (int idx = 0; idx < mpDockWindows->count(); ++idx)
   {
      QListWidgetItem* pItem = mpDockWindows->item(idx);
      VERIFYNRV(NULL != pItem);
      if (pItem->checkState() == Qt::Checked)
      {
         std::string text = pItem->text().toStdString();
         checkedDockWindows.push_back(text);
      }
   }
   OptionsFullScreen::setSettingDockWindowsStayVisible(&checkedDockWindows);

   std::string layout = mpWindowLayout->currentText().toStdString();
   OptionsFullScreen::setSettingWorkspaceWindowLayout(&layout);
}

OptionsFullScreen::~OptionsFullScreen()
{
}
