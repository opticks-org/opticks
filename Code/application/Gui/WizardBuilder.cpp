/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QAction>
#include <QtGui/QCloseEvent>
#include <QtGui/QDockWidget>
#include <QtGui/QFileDialog>
#include <QtGui/QGraphicsScene>
#include <QtGui/QMessageBox>
#include <QtGui/QMenu>
#include <QtGui/QToolBar>

#include "AppAssert.h"
#include "AppVerify.h"
#include "Icons.h"
#include "MenuBarImp.h"
#include "WizardBuilder.h"
#include "WizardClipboard.h"
#include "WizardItemImp.h"
#include "WizardItemPalette.h"
#include "WizardItemProperties.h"
#include "WizardObjectAdapter.h"
#include "WizardProperties.h"
#include "WizardView.h"

#include <limits>
#include <string>
#include <vector>
using namespace std;

WizardBuilder::WizardBuilder(QWidget* pParent) :
   QMainWindow(pParent),
   mpView(NULL),
   mpWizardProperties(NULL),
   mpItemProperties(NULL)
{
   ////////////////////
   // Graphics Scene //
   ////////////////////

   QGraphicsScene* pScene = new QGraphicsScene(this);

   mpView = new WizardView(pScene, this);
   VERIFYNR(connect(mpView, SIGNAL(filenameChanged(const QString&)), this, SLOT(updateCaption(const QString&))));
   VERIFYNR(connect(mpView, SIGNAL(selectionChanged()), this, SLOT(updateItemProperties())));

   /////////////////////////
   // Wizard item palette //
   /////////////////////////

   WizardItemPalette* pPalette = new WizardItemPalette(this);

   QDockWidget* pPaletteDock = new QDockWidget("Wizard Item Palette", this);
   pPaletteDock->setObjectName("Wizard Item Palette");
   pPaletteDock->setWidget(pPalette);
   addDockWidget(Qt::LeftDockWidgetArea, pPaletteDock);

   ///////////////////////
   // Wizard properties //
   ///////////////////////

   mpWizardProperties = new WizardProperties(this);

   QDockWidget* pWizardPropertiesDock = new QDockWidget("Wizard Properties", this);
   pWizardPropertiesDock->setObjectName("Wizard Properties");
   pWizardPropertiesDock->setWidget(mpWizardProperties);
   addDockWidget(Qt::RightDockWidgetArea, pWizardPropertiesDock);

   ////////////////////////////
   // Wizard item properties //
   ////////////////////////////

   mpItemProperties = new WizardItemProperties(this);

   QDockWidget* pItemPropertiesDock = new QDockWidget("Wizard Item Properties", this);
   pItemPropertiesDock->setObjectName("Wizard Item Properties");
   pItemPropertiesDock->setWidget(mpItemProperties);
   addDockWidget(Qt::RightDockWidgetArea, pItemPropertiesDock);

   /////////////
   // Actions //
   /////////////

   Icons* pIcons = Icons::instance();
   REQUIRE(pIcons != NULL);

   // File
   QAction* pNewAction = new QAction(pIcons->mNew, "&New", this);
   pNewAction->setAutoRepeat(false);
   pNewAction->setShortcut(QKeySequence::New);
   pNewAction->setStatusTip("Creates a new, empty wizard");
   pNewAction->setToolTip("New");
   VERIFYNR(connect(pNewAction, SIGNAL(triggered()), this, SLOT(newWizard())));

   QAction* pOpenAction = new QAction(pIcons->mOpen, "&Open...", this);
   pOpenAction->setAutoRepeat(false);
   pOpenAction->setShortcut(QKeySequence::Open);
   pOpenAction->setStatusTip("Opens an existing wizard from a file");
   pOpenAction->setToolTip("Open");
   VERIFYNR(connect(pOpenAction, SIGNAL(triggered()), this, SLOT(openWizard())));

   QAction* pSaveAction = new QAction(pIcons->mSave, "&Save", this);
   pSaveAction->setAutoRepeat(false);
   pSaveAction->setShortcut(QKeySequence::Save);
   pSaveAction->setStatusTip("Saves the current wizard to the current file");
   pSaveAction->setToolTip("Save");
   VERIFYNR(connect(pSaveAction, SIGNAL(triggered()), mpView, SLOT(save())));

   QAction* pSaveAsAction = new QAction("Save &As...", this);
   pSaveAsAction->setAutoRepeat(false);
   pSaveAsAction->setShortcut(QKeySequence::SaveAs);
   pSaveAsAction->setStatusTip("Saves the current wizard to a new file");
   pSaveAsAction->setToolTip("Save As");
   VERIFYNR(connect(pSaveAsAction, SIGNAL(triggered()), mpView, SLOT(saveAs())));

   QAction* pPrintAction = new QAction(pIcons->mPrint, "&Print", this);
   pPrintAction->setAutoRepeat(false);
   pPrintAction->setShortcut(QKeySequence::Print);
   pPrintAction->setStatusTip("Prints the current wizard");
   pPrintAction->setToolTip("Print");
   VERIFYNR(connect(pPrintAction, SIGNAL(triggered()), mpView, SLOT(print())));

   QAction* pCloseAction = new QAction("&Close", this);
   pCloseAction->setAutoRepeat(false);
   pCloseAction->setShortcut(QKeySequence::Close);
   pCloseAction->setStatusTip("Closes the wizard builder");
   pCloseAction->setToolTip("Close");
   VERIFYNR(connect(pCloseAction, SIGNAL(triggered()), this, SLOT(close())));

   // Edit
   QAction* pCutAction = new QAction(pIcons->mCut, "Cu&t", this);
   pCutAction->setAutoRepeat(false);
   pCutAction->setShortcut(QKeySequence::Cut);
   pCutAction->setStatusTip("Cuts the currently selected wizard items and moves them to the clipboard");
   pCutAction->setToolTip("Cut");
   VERIFYNR(connect(pCutAction, SIGNAL(triggered()), this, SLOT(cut())));

   QAction* pCopyAction = new QAction(pIcons->mCopy, "&Copy", this);
   pCopyAction->setAutoRepeat(false);
   pCopyAction->setShortcut(QKeySequence::Copy);
   pCopyAction->setStatusTip("Copies the currently selected wizard items to the clipboard");
   pCopyAction->setToolTip("Copy");
   VERIFYNR(connect(pCopyAction, SIGNAL(triggered()), this, SLOT(copy())));

   QAction* pPasteAction = new QAction(pIcons->mPaste, "&Paste", this);
   pPasteAction->setAutoRepeat(false);
   pPasteAction->setShortcut(QKeySequence::Paste);
   pPasteAction->setStatusTip("Adds the clipboard wizard items to the wizard");
   pPasteAction->setToolTip("Paste");
   VERIFYNR(connect(pPasteAction, SIGNAL(triggered()), this, SLOT(paste())));

   // View
   QAction* pZoomInAction = new QAction(pIcons->mZoomIn, "Zoom In", this);
   pZoomInAction->setAutoRepeat(true);
   pZoomInAction->setShortcut(QKeySequence::ZoomIn);
   pZoomInAction->setStatusTip("Increases the zoom level in the view about the window center");
   pZoomInAction->setToolTip("Zoom In");
   VERIFYNR(connect(pZoomInAction, SIGNAL(triggered()), mpView, SLOT(zoomIn())));

   QAction* pZoomOutAction = new QAction(pIcons->mZoomOut, "Zoom Out", this);
   pZoomOutAction->setAutoRepeat(true);
   pZoomOutAction->setShortcut(QKeySequence::ZoomOut);
   pZoomOutAction->setStatusTip("Decreases the zoom level in the view about the window center");
   pZoomOutAction->setToolTip("Zoom Out");
   VERIFYNR(connect(pZoomOutAction, SIGNAL(triggered()), mpView, SLOT(zoomOut())));

   QAction* pZoomToFitAction = new QAction(pIcons->mZoomToFit, "Zoom to Fit", this);
   pZoomToFitAction->setAutoRepeat(false);
   pZoomToFitAction->setShortcut(Qt::Key_E);
   pZoomToFitAction->setStatusTip("Zooms the view to the maximum extent of the wizard items");
   pZoomToFitAction->setToolTip("Zoom to Fit");
   VERIFYNR(connect(pZoomToFitAction, SIGNAL(triggered()), mpView, SLOT(zoomToFit())));

   QAction* pExecuteAction = new QAction("Execute", this);
   pExecuteAction->setAutoRepeat(false);
   pExecuteAction->setStatusTip("Runs the wizard in its current state");
   pExecuteAction->setToolTip("Execute");
   VERIFYNR(connect(pExecuteAction, SIGNAL(triggered()), mpView, SLOT(execute())));

   // Item
   QAction* pSelectAction = new QAction("&Select All", this);
   pSelectAction->setAutoRepeat(false);
   pSelectAction->setShortcut(QKeySequence::SelectAll);
   pSelectAction->setStatusTip("Selects all wizard items");
   pSelectAction->setToolTip("Select All Items");
   VERIFYNR(connect(pSelectAction, SIGNAL(triggered()), mpView, SLOT(selectAllItems())));

   QAction* pRemoveAction = new QAction(pIcons->mDelete, "&Remove", this);
   pRemoveAction->setAutoRepeat(false);
   pRemoveAction->setShortcut(QKeySequence("Ctrl+R"));
   pRemoveAction->setStatusTip("Removes the currently selected items");
   pRemoveAction->setToolTip("Remove Item");
   VERIFYNR(connect(pRemoveAction, SIGNAL(triggered()), mpView, SLOT(removeSelectedItems())));

   QAction* pEditAction = new QAction(pIcons->mValueEdit, "&Edit...", this);
   pEditAction->setAutoRepeat(false);
   pEditAction->setStatusTip("Edits the selected item");
   pEditAction->setToolTip("Edit Item");
   VERIFYNR(connect(pEditAction, SIGNAL(triggered()), this, SLOT(editItem())));

   QAction* pIncreaseAction = new QAction(pIcons->mIncrease, "&Increase Order", this);
   pIncreaseAction->setStatusTip("Increases the execution order of the currently selected item by one");
   pIncreaseAction->setToolTip("Increase Order");
   VERIFYNR(connect(pIncreaseAction, SIGNAL(triggered()), this, SLOT(increaseCurrentItemOrder())));

   QAction* pDecreaseAction = new QAction(pIcons->mDecrease, "&Decrease Order", this);
   pDecreaseAction->setStatusTip("Decreases the execution order of the currently selected item by one");
   pDecreaseAction->setToolTip("Decrease Order");
   VERIFYNR(connect(pDecreaseAction, SIGNAL(triggered()), this, SLOT(decreaseCurrentItemOrder())));

   /////////////
   // Toolbar //
   /////////////

   QToolBar* pToolbar = new QToolBar("Wizard", this);
   pToolbar->setIconSize(QSize(16, 16));
   pToolbar->addAction(pNewAction);
   pToolbar->addAction(pOpenAction);
   pToolbar->addAction(pSaveAction);
   pToolbar->addSeparator();
   pToolbar->addAction(pPrintAction);
   pToolbar->addSeparator();
   pToolbar->addAction(pCutAction);
   pToolbar->addAction(pCopyAction);
   pToolbar->addAction(pPasteAction);
   pToolbar->addSeparator();
   pToolbar->addAction(pZoomInAction);
   pToolbar->addAction(pZoomOutAction);
   pToolbar->addAction(pZoomToFitAction);
   pToolbar->addSeparator();
   pToolbar->addAction(pEditAction);
   pToolbar->addAction(pRemoveAction);
   pToolbar->addSeparator();
   pToolbar->addAction(pIncreaseAction);
   pToolbar->addAction(pDecreaseAction);

   addToolBar(pToolbar);

   //////////////
   // Menu bar //
   //////////////

   // Create the menu bar
   MenuBarImp* pMenuBar = new MenuBarImp("Menu Bar", this);

   ///////////
   // Menus //
   ///////////

   string context = "Wizard Builder";

   // File
   QMenu* pFile = pMenuBar->addMenu("&File");
   pMenuBar->insertCommand(pNewAction, pFile, context);
   pMenuBar->insertCommand(pOpenAction, pFile, context);
   pMenuBar->insertCommand(pSaveAction, pFile, context);
   pMenuBar->insertCommand(pSaveAsAction, pFile, context);
   pFile->addSeparator();
   pMenuBar->insertCommand(pPrintAction, pFile, context);
   pFile->addSeparator();
   pMenuBar->insertCommand(pCloseAction, pFile, context);

   // Edit
   QMenu* pEdit = pMenuBar->addMenu("&Edit");
   pMenuBar->insertCommand(pCutAction, pEdit, context);
   pMenuBar->insertCommand(pCopyAction, pEdit, context);
   pMenuBar->insertCommand(pPasteAction, pEdit, context);

   // View
   QMenu* pView = pMenuBar->addMenu("&View");

   QMenu* pToolbarsMenu = new QMenu("Toolbars", pView);
   pToolbarsMenu->addAction(pToolbar->toggleViewAction());

   QMenu* pToolsMenu = new QMenu("Tools", pView);
   pToolsMenu->addAction(pPaletteDock->toggleViewAction());
   pToolsMenu->addAction(pWizardPropertiesDock->toggleViewAction());
   pToolsMenu->addAction(pItemPropertiesDock->toggleViewAction());

   pView->addMenu(pToolbarsMenu);
   pView->addMenu(pToolsMenu);
   pView->addSeparator();
   pMenuBar->insertCommand(pZoomInAction, pView, context);
   pMenuBar->insertCommand(pZoomOutAction, pView, context);
   pMenuBar->insertCommand(pZoomToFitAction, pView, context);
   pView->addSeparator();
   pMenuBar->insertCommand(pExecuteAction, pView, context);

   // Item
   QMenu* pItem = pMenuBar->addMenu("&Item");
   pMenuBar->insertCommand(pSelectAction, pItem, context);
   pItem->addSeparator();
   pMenuBar->insertCommand(pEditAction, pItem, context);
   pMenuBar->insertCommand(pRemoveAction, pItem, context);
   pItem->addSeparator();
   pMenuBar->insertCommand(pIncreaseAction, pItem, context);
   pMenuBar->insertCommand(pDecreaseAction, pItem, context);

   setMenuBar(pMenuBar);

   ////////////////
   // Status bar //
   ////////////////

   statusBar();

   ////////////////////
   // Initialization //
   ////////////////////

   setAttribute(Qt::WA_DeleteOnClose);
   setCentralWidget(mpView);
   resize(1000, 700);
   newWizard();

   // Dock window and toolbar configuration
   if (WizardBuilder::hasSettingConfiguration())
   {
      string configData = WizardBuilder::getSettingConfiguration();
      if (configData.empty() == false)
      {
         QByteArray windowConfiguration(configData.c_str(), configData.size());
         restoreState(QByteArray::fromBase64(windowConfiguration));
      }
   }

   // Window geometry
   if (WizardBuilder::hasSettingGeometry())
   {
      string geometryData = WizardBuilder::getSettingGeometry();
      if (geometryData.empty() == false)
      {
         QByteArray windowGeometry(geometryData.c_str(), geometryData.size());
         restoreGeometry(QByteArray::fromBase64(windowGeometry));
      }
   }
}

WizardBuilder::~WizardBuilder()
{}

void WizardBuilder::closeEvent(QCloseEvent* pEvent)
{
   if (destroyCurrentWizard() == false)
   {
      pEvent->ignore();
      return;
   }

   // Dock window and toolbar configuration
   QByteArray windowConfiguration = saveState().toBase64();

   QString configText(windowConfiguration);
   if (configText.isEmpty() == false)
   {
      string configData = configText.toStdString();
      WizardBuilder::setSettingConfiguration(configData);
   }

   // Window geometry
   QByteArray windowGeometry = saveGeometry().toBase64();

   QString geometryText(windowGeometry);
   if (geometryText.isEmpty() == false)
   {
      string geometryData = geometryText.toStdString();
      WizardBuilder::setSettingGeometry(geometryData);
   }

   // Close the window
   QMainWindow::closeEvent(pEvent);
}

bool WizardBuilder::destroyCurrentWizard()
{
   WizardObjectImp* pWizard = dynamic_cast<WizardObjectImp*>(mpView->getWizard());
   if (pWizard == NULL)
   {
      return true;
   }

   // Prompt the user to save the wizard if necessary
   if (mpView->isModified() == true)
   {
      QString strName = "Untitled";

      const string& wizardName = pWizard->getName();
      if (wizardName.empty() == false)
      {
         strName = QString::fromStdString(wizardName);
      }

      int iReturn = QMessageBox::question(this, "Wizard Builder", "Do you want to save the changes to " +
         strName + "?", QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel);
      if (iReturn == QMessageBox::Yes)
      {
         bool bSuccess = mpView->save();
         if (bSuccess == false)
         {
            return false;
         }
      }
      else if (iReturn == QMessageBox::Cancel)
      {
         return false;
      }
   }

   // Reset the view and the properties widgets
   mpView->setWizard(NULL);
   mpWizardProperties->setWizard(NULL);

   // Delete the wizard
   delete pWizard;

   return true;
}

void WizardBuilder::newWizard()
{
   if (destroyCurrentWizard() == false)
   {
      return;
   }

   WizardObject* pWizard = mpView->getWizard();
   if (pWizard == NULL)
   {
      pWizard = new WizardObjectAdapter();
      if (pWizard != NULL)
      {
         pWizard->setName("Wizard");
      }
   }

   mpView->setWizard(pWizard);
   mpWizardProperties->setWizard(pWizard);
}

void WizardBuilder::openWizard()
{
   WizardObjectImp* pWizard = dynamic_cast<WizardObjectImp*>(mpView->getWizard());
   if (pWizard != NULL)
   {
      if (mpView->isModified() == true)
      {
         QString strName = "Untitled";

         const string& wizardName = pWizard->getName();
         if (wizardName.empty() == false)
         {
            strName = QString::fromStdString(wizardName);
         }

         int iReturn = QMessageBox::question(this, "Wizard Builder", "Do you want to save the changes to " +
            strName + "?", QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel);
         if (iReturn == QMessageBox::Yes)
         {
            bool bSuccess = mpView->save();
            if (bSuccess == false)
            {
               return;
            }
         }
         else if (iReturn == QMessageBox::Cancel)
         {
            return;
         }
      }
   }

   // Get the default directory from the options
   QString strDefaultDir = QDir::currentPath();

   const Filename* pWizardPath = ConfigurationSettings::getSettingWizardPath();
   if (pWizardPath != NULL)
   {
      strDefaultDir = QString::fromStdString(pWizardPath->getFullPathAndName());
   }

   // Get the filename from the user
   QString strOpenFilename = QFileDialog::getOpenFileName(this, QString(), strDefaultDir, "Wizard Files (*.wiz)");
   if (strOpenFilename.isEmpty() == true)
   {
      return;
   }

   // Load the wizard from the file
   if (mpView->setWizard(strOpenFilename) == true)
   {
      // Initialize the properties widgets
      mpWizardProperties->setWizard(mpView->getWizard());

      // Delete the current wizard
      delete pWizard;
   }
}

void WizardBuilder::updateCaption(const QString& filename)
{
   QString captionName = "Untitled";
   if (filename.isEmpty() == false)
   {
      QFileInfo fileInfo = QFileInfo(filename);
      captionName = fileInfo.fileName();
   }

   setWindowTitle("Wizard Builder - " + captionName);
}

void WizardBuilder::cut()
{
   copy();
   mpView->removeSelectedItems();
}

void WizardBuilder::copy()
{
   WizardObject* pWizard = mpView->getWizard();
   if (pWizard == NULL)
   {
      return;
   }

   WizardClipboard* pClipboard = WizardClipboard::instance();
   if (pClipboard == NULL)
   {
      return;
   }

   // Get the selected items from the wizard instead of the view to preserve the execution order
   vector<WizardItem*> items = pWizard->getItems();
   vector<WizardItem*> clipboardItems;

   for (unsigned int i = 0; i < items.size(); i++)
   {
      WizardItem* pItem = items[i];
      if (pItem != NULL)
      {
         bool bSelected = mpView->isItemSelected(pItem);
         if (bSelected == true)
         {
            clipboardItems.push_back(pItem);
         }
      }
   }

   // Set the items in the clipboard
   pClipboard->setItems(clipboardItems);
}

void WizardBuilder::paste()
{
   WizardObjectImp* pWizard = dynamic_cast<WizardObjectImp*>(mpView->getWizard());
   if (pWizard == NULL)
   {
      return;
   }

   WizardClipboard* pClipboard = WizardClipboard::instance();
   if (pClipboard == NULL)
   {
      return;
   }

   const vector<WizardItem*>& items = pClipboard->getItems();
   if (items.empty() == true)
   {
      return;
   }

   // Deselect all items
   mpView->deselectAllItems();

   // Copy the items from the clipboard and determine the center of the items
   QPointF lowerLeft(numeric_limits<float>::max(), numeric_limits<float>::max());
   QPointF upperRight(-numeric_limits<float>::max(), -numeric_limits<float>::max());

   vector<WizardItem*> newItems;
   for (unsigned int i = 0; i < items.size(); i++)
   {
      WizardItemImp* pClipboardItem = static_cast<WizardItemImp*>(items[i]);
      if (pClipboardItem != NULL)
      {
         WizardItemImp* pItem = new WizardItemImp(mpView->getWizard(), string(), string());
         pItem->copyItem(pClipboardItem);
         newItems.push_back(static_cast<WizardItem*>(pItem));

         lowerLeft.setX(min(lowerLeft.x(), pItem->getXPosition()));
         lowerLeft.setY(min(lowerLeft.y(), pItem->getYPosition()));
         upperRight.setX(max(upperRight.x(), pItem->getXPosition()));
         upperRight.setY(max(upperRight.y(), pItem->getYPosition()));
      }
   }

   // Add the items to the wizard and select them in the view
   for (vector<WizardItem*>::iterator iter = newItems.begin(); iter != newItems.end(); ++iter)
   {
      WizardItem* pItem = *iter;
      if (pItem != NULL)
      {
         pWizard->addItem(pItem);
         mpView->selectItem(pItem);
      }
   }

   // Set the item connections
   vector<WizardConnection> connections = WizardItemImp::getConnections(items);
   WizardItemImp::setConnections(newItems, connections);

   // Ensure the pasted items are visible in the view
   mpView->centerOn(lowerLeft.x() + upperRight.x() / 2.0, lowerLeft.y() + upperRight.y() / 2.0);
}

void WizardBuilder::editItem()
{
   vector<WizardItem*> selectedItems = mpView->getSelectedItems();

   vector<WizardItem*>::size_type numItems = selectedItems.size();
   if (numItems > 1)
   {
      QMessageBox::warning(this, "Wizard Builder", "Please select only one item.");
   }
   else if (numItems == 1)
   {
      WizardItem* pItem = selectedItems.front();
      if (pItem != NULL)
      {
         const string& itemType = pItem->getType();
         if (itemType != "Value")
         {
            QMessageBox::warning(this, "Wizard Builder", "You can only edit a Value item.");
            return;
         }

         mpView->editItem(pItem);
      }
   }
   else
   {
      QMessageBox::warning(this, "Wizard Builder", "Please select a Value item to edit.");
   }
}

void WizardBuilder::increaseCurrentItemOrder()
{
   WizardObjectImp* pWizard = dynamic_cast<WizardObjectImp*>(mpView->getWizard());
   if (pWizard == NULL)
   {
      return;
   }

   vector<WizardItem*> selectedItems = mpView->getSelectedItems();

   vector<WizardItem*>::size_type numItems = selectedItems.size();
   if (numItems > 1)
   {
      QMessageBox::warning(this, "Wizard Builder", "Please select only one item.");
      return;
   }
   else if (numItems == 1)
   {
      WizardItem* pItem = selectedItems.front();
      if (pItem != NULL)
      {
         pWizard->increaseItemOrder(pItem);
      }
   }
}

void WizardBuilder::decreaseCurrentItemOrder()
{
   WizardObjectImp* pWizard = dynamic_cast<WizardObjectImp*>(mpView->getWizard());
   if (pWizard == NULL)
   {
      return;
   }

   vector<WizardItem*> selectedItems = mpView->getSelectedItems();

   vector<WizardItem*>::size_type numItems = selectedItems.size();
   if (numItems > 1)
   {
      QMessageBox::warning(this, "Wizard Builder", "Please select only one item.");
      return;
   }
   else if (numItems == 1)
   {
      WizardItem* pItem = selectedItems.front();
      if (pItem != NULL)
      {
         pWizard->decreaseItemOrder(pItem);
      }
   }
}

void WizardBuilder::updateItemProperties()
{
   WizardItem* pItem = NULL;

   vector<WizardItem*> selectedItems = mpView->getSelectedItems();
   if (selectedItems.empty() == false)
   {
      pItem = selectedItems.front();
   }

   mpItemProperties->setWizardItem(pItem);
}
