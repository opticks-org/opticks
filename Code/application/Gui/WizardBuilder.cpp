/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QFileDialog>
#include <QtGui/QHeaderView>
#include <QtGui/QInputDialog>
#include <QtGui/QKeyEvent>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QMenuBar>
#include <QtGui/QMessageBox>
#include <QtGui/QPainter>
#include <QtGui/QPrintDialog>
#include <QtGui/QMenu>
#include <QtGui/QPrinter>
#include <QtGui/QSplitter>
#include <QtGui/QToolBar>
#include <QtGui/QWidgetAction>

#include "AppVersion.h"
#include "WizardBuilder.h"
#include "BatchWizard.h"
#include "ConfigurationSettings.h"
#include "AppAssert.h"
#include "DesktopServicesImp.h"
#include "FilenameImp.h"
#include "Icons.h"
#include "MenuBarImp.h"
#include "ModelServicesImp.h"
#include "NameTypeValueDlg.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInDescriptor.h"
#include "PlugInManagerServicesImp.h"
#include "PlugInResource.h"
#include "PlugInSelectDlg.h"
#include "SystemServicesImp.h"
#include "StringUtilities.h"
#include "WizardClipboard.h"
#include "WizardItemImp.h"
#include "WizardNodeImp.h"
#include "WizardObjectAdapter.h"
#include "WizardUtilities.h"
#include "WizardView.h"

#include <vector>
#include <string>
using namespace std;

WizardBuilder::WizardBuilder(QWidget* pParent) :
   QMainWindow(pParent),
   mpWizard(NULL),
   mFilename(QString()),
   mpCanvas(NULL),
   mpView(NULL),
   mbModified(false),
   mpInputsTree(NULL),
   mpOutputsTree(NULL),
   mpWizardBatchCheck(NULL),
   mpItemBatchCheck(NULL)
{
   ////////////
   // Canvas //
   ////////////

   QSplitter* pHSplitter = new QSplitter(Qt::Horizontal, this);
   setCentralWidget(pHSplitter);

   mpCanvas = new Q3Canvas(5000, 5000);
   if (mpCanvas != NULL)
   {
      mpView = new WizardView(mpCanvas, pHSplitter);
      if (mpView != NULL)
      {
         connect(mpView, SIGNAL(selectionChanged()), this, SLOT(updateItemInfo()));
         connect(mpView, SIGNAL(itemPositionChanged(WizardItem*, double, double)), this,
            SLOT(setItemPosition(WizardItem*, double, double)));
         connect(mpView, SIGNAL(itemPressed(QMouseEvent*, WizardItem*)), this,
            SLOT(itemMousePressEvent(QMouseEvent*, WizardItem*)));
         connect(mpView, SIGNAL(itemDoubleClicked(QMouseEvent*, WizardItem*)), this,
            SLOT(itemMouseDoubleClickEvent(QMouseEvent*, WizardItem*)));
         connect(mpView, SIGNAL(itemsConnected(WizardItem*, WizardItem*)), this,
            SLOT(updateConnectedItemsOrder(WizardItem*, WizardItem*)));
         connect(mpView, SIGNAL(nodeTypeChanged(WizardNode*)), this, SLOT(updateItemInfo()));
         connect(mpView, SIGNAL(nodeTypeChanged(WizardNode*)), this, SLOT(setModified()));
      }
   }

   /////////////
   // Actions //
   /////////////

   Icons* pIcons = Icons::instance();
   REQUIRE(pIcons != NULL);

   // File
   QAction* pNewAction = new QAction(pIcons->mNew, "&New", this);
   pNewAction->setAutoRepeat(false);
   pNewAction->setShortcut(QKeySequence("Ctrl+N"));
   pNewAction->setStatusTip("Creates a new, empty wizard");
   pNewAction->setToolTip("New");
   connect(pNewAction, SIGNAL(triggered()), this, SLOT(newWizard()));

   QAction* pOpenAction = new QAction(pIcons->mOpen, "&Open...", this);
   pOpenAction->setAutoRepeat(false);
   pOpenAction->setShortcut(QKeySequence("Ctrl+O"));
   pOpenAction->setStatusTip("Opens an existing wizard from a file");
   pOpenAction->setToolTip("Open");
   connect(pOpenAction, SIGNAL(triggered()), this, SLOT(openWizard()));

   QAction* pSaveAction = new QAction(pIcons->mSave, "&Save", this);
   pSaveAction->setAutoRepeat(false);
   pSaveAction->setShortcut(QKeySequence("Ctrl+S"));
   pSaveAction->setStatusTip("Saves the current wizard to the current file");
   pSaveAction->setToolTip("Save");
   connect(pSaveAction, SIGNAL(triggered()), this, SLOT(saveWizard()));

   QAction* pSaveAsAction = new QAction("Save &As...", this);
   pSaveAsAction->setAutoRepeat(false);
   pSaveAsAction->setStatusTip("Saves the current wizard to a new file");
   pSaveAsAction->setToolTip("Save As");
   connect(pSaveAsAction, SIGNAL(triggered()), this, SLOT(saveNewWizard()));

   QAction* pPageSetupAction = new QAction(pIcons->mPageSetup, "Page Se&tup...", this);
   pPageSetupAction->setAutoRepeat(false);
   pPageSetupAction->setStatusTip("Configues the layout of the page as it would be printed");
   pPageSetupAction->setToolTip("Page Setup");
   pPageSetupAction->setEnabled(false);   // Temporary: Disabled until capability is added
   connect(pPageSetupAction, SIGNAL(triggered()), this, SLOT(wizardPageSetup()));

   QAction* pPrintAction = new QAction(pIcons->mPrint, "&Print", this);
   pPrintAction->setAutoRepeat(false);
   pPrintAction->setShortcut(QKeySequence("Ctrl+P"));
   pPrintAction->setStatusTip("Prints the current wizard");
   pPrintAction->setToolTip("Print");
   connect(pPrintAction, SIGNAL(triggered()), this, SLOT(printWizard()));

   QAction* pCloseAction = new QAction("&Close", this);
   pCloseAction->setAutoRepeat(false);
   pCloseAction->setStatusTip("Closes the wizard builder");
   pCloseAction->setToolTip("Close");
   connect(pCloseAction, SIGNAL(triggered()), this, SLOT(exitBuilder()));

   // Edit
   QAction* pCutAction = new QAction(pIcons->mCut, "Cu&t", this);
   pCutAction->setAutoRepeat(false);
   pCutAction->setShortcut(QKeySequence("Ctrl+X"));
   pCutAction->setStatusTip("Cuts the currently selected wizard items and moves them to the clipboard");
   pCutAction->setToolTip("Cut");
   connect(pCutAction, SIGNAL(triggered()), this, SLOT(cut()));

   QAction* pCopyAction = new QAction(pIcons->mCopy, "&Copy", this);
   pCopyAction->setAutoRepeat(false);
   pCopyAction->setShortcut(QKeySequence("Ctrl+C"));
   pCopyAction->setStatusTip("Copies the currently selected wizard items to the clipboard");
   pCopyAction->setToolTip("Copy");
   connect(pCopyAction, SIGNAL(triggered()), this, SLOT(copy()));

   QAction* pPasteAction = new QAction(pIcons->mPaste, "&Paste", this);
   pPasteAction->setAutoRepeat(false);
   pPasteAction->setShortcut(QKeySequence("Ctrl+V"));
   pPasteAction->setStatusTip("Adds the clipboard wizard items to the wizard");
   pPasteAction->setToolTip("Paste");
   connect(pPasteAction, SIGNAL(triggered()), this, SLOT(paste()));

   // Item
   QAction* pDesktopAction = new QAction(pIcons->mDesktop, "Add Des&ktop...", this);
   pDesktopAction->setAutoRepeat(false);
   pDesktopAction->setStatusTip("Adds a new item representing a desktop service function");
   pDesktopAction->setToolTip("Add Desktop Item");
   connect(pDesktopAction, SIGNAL(triggered()), this, SLOT(addDesktopItem()));

   QAction* pPlugInAction = new QAction(pIcons->mPlugIn, "Add Plu&g-In...", this);
   pPlugInAction->setAutoRepeat(false);
   pPlugInAction->setStatusTip("Adds a new item representing a plug-in");
   pPlugInAction->setToolTip("Add Plug-In Item");
   connect(pPlugInAction, SIGNAL(triggered()), this, SLOT(addPlugInItem()));

   QAction* pValueAction = new QAction(pIcons->mValue, "Add &Value...", this);
   pValueAction->setAutoRepeat(false);
   pValueAction->setStatusTip("Adds a new item containing a specific value");
   pValueAction->setToolTip("Add Value Item");
   connect(pValueAction, SIGNAL(triggered()), this, SLOT(addValueItem()));

   QAction* pSelectAction = new QAction("&Select All", this);
   pSelectAction->setAutoRepeat(false);
   pSelectAction->setStatusTip("Selects all wizard items");
   pSelectAction->setToolTip("Select All Items");
   connect(pSelectAction, SIGNAL(triggered()), this, SLOT(selectAllItems()));

   QAction* pRemoveAction = new QAction(pIcons->mDelete, "&Remove", this);
   pRemoveAction->setAutoRepeat(false);
   pRemoveAction->setShortcut(QKeySequence("Ctrl+R"));
   pRemoveAction->setStatusTip("Removes the currently selected items");
   pRemoveAction->setToolTip("Remove Item");
   connect(pRemoveAction, SIGNAL(triggered()), this, SLOT(removeItems()));

   QAction* pEditAction = new QAction(pIcons->mValueEdit, "&Edit...", this);
   pEditAction->setAutoRepeat(false);
   pEditAction->setStatusTip("Edits the selected item");
   pEditAction->setToolTip("Edit Item");
   connect(pEditAction, SIGNAL(triggered()), this, SLOT(editItem()));

   QAction* pIncreaseAction = new QAction(pIcons->mIncrease, "&Increase Order", this);
   pIncreaseAction->setStatusTip("Increases the execution order of the currently selected item by one");
   pIncreaseAction->setToolTip("Increase Order");
   connect(pIncreaseAction, SIGNAL(triggered()), this, SLOT(increaseCurrentItemOrder()));

   QAction* pDecreaseAction = new QAction(pIcons->mDecrease, "&Decrease Order", this);
   pDecreaseAction->setStatusTip("Decreases the execution order of the currently selected item by one");
   pDecreaseAction->setToolTip("Decrease Order");
   connect(pDecreaseAction, SIGNAL(triggered()), this, SLOT(decreaseCurrentItemOrder()));

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
   pMenuBar->insertCommand(pPageSetupAction, pFile, context);
   pMenuBar->insertCommand(pPrintAction, pFile, context);
   pFile->addSeparator();
   pMenuBar->insertCommand(pCloseAction, pFile, context);

   // Edit
   QMenu* pEdit = pMenuBar->addMenu("&Edit");
   pMenuBar->insertCommand(pCutAction, pEdit, context);
   pMenuBar->insertCommand(pCopyAction, pEdit, context);
   pMenuBar->insertCommand(pPasteAction, pEdit, context);

   // Item
   QMenu* pItem = pMenuBar->addMenu("&Item");
   pMenuBar->insertCommand(pDesktopAction, pItem, context);
   pMenuBar->insertCommand(pPlugInAction, pItem, context);
   pMenuBar->insertCommand(pValueAction, pItem, context);
   pItem->addSeparator();
   pMenuBar->insertCommand(pSelectAction, pItem, context);
   pMenuBar->insertCommand(pRemoveAction, pItem, context);
   pItem->addSeparator();
   pMenuBar->insertCommand(pEditAction, pItem, context);
   pItem->addSeparator();
   pMenuBar->insertCommand(pIncreaseAction, pItem, context);
   pMenuBar->insertCommand(pDecreaseAction, pItem, context);

   setMenuBar(pMenuBar);

   /////////////
   // Toolbar //
   /////////////

   QToolBar* pToolbar = new QToolBar("Wizard Builder", this);
   pToolbar->setIconSize(QSize(16, 16));
   pToolbar->addAction(pNewAction);
   pToolbar->addAction(pOpenAction);
   pToolbar->addAction(pSaveAction);
   pToolbar->addSeparator();
   pToolbar->addAction(pPageSetupAction);
   pToolbar->addAction(pPrintAction);
   pToolbar->addSeparator();
   pToolbar->addAction(pCutAction);
   pToolbar->addAction(pCopyAction);
   pToolbar->addAction(pPasteAction);
   pToolbar->addSeparator();
   pToolbar->addAction(pDesktopAction);
   pToolbar->addAction(pPlugInAction);
   pToolbar->addAction(pValueAction);
   pToolbar->addAction(pRemoveAction);
   pToolbar->addSeparator();
   pToolbar->addAction(pEditAction);
   pToolbar->addSeparator();
   pToolbar->addAction(pIncreaseAction);
   pToolbar->addAction(pDecreaseAction);

   addToolBar(pToolbar);

   ////////////////////
   // Wizard widgets //
   ////////////////////

   QWidget* pInfoWidget = new QWidget(pHSplitter);

   QFont ftHeading = QFont("Verdana", 16, QFont::Bold);

   // Wizard label
   QLabel* pWizardLabel = new QLabel("Wizard", pInfoWidget);
   pWizardLabel->setFont(ftHeading);

   // Name
   QLabel* pNameLabel = new QLabel("Wizard Name:", pInfoWidget);
   mpNameEdit = new QLineEdit(pInfoWidget);

   connect(mpNameEdit, SIGNAL(textChanged(const QString&)), this,
      SLOT(setWizardName(const QString&)));

   // Menu location
   QLabel* pMenuLabel = new QLabel("Menu Location:", pInfoWidget);
   mpMenuEdit = new QLineEdit(pInfoWidget);

   connect(mpMenuEdit, SIGNAL(textChanged(const QString&)), this,
      SLOT(setWizardMenuLocation(const QString&)));

   // Batch mode check box
   mpWizardBatchCheck = new QCheckBox("Run Wizard in Batch Mode", pInfoWidget);
   mpWizardBatchCheck->installEventFilter(this);
   connect(mpWizardBatchCheck, SIGNAL(toggled(bool)), this, SLOT(setWizardBatchMode(bool)));

   // Execute button
   QPushButton* pExecuteButton = new QPushButton("&Execute", pInfoWidget);
   pExecuteButton->setFixedWidth(70);
   pExecuteButton->installEventFilter(this);
   connect(pExecuteButton, SIGNAL(clicked()), this, SLOT(runWizard()));

   // Layout
   QHBoxLayout* pHWizardLayout = new QHBoxLayout();
   pHWizardLayout->setMargin(0);
   pHWizardLayout->setSpacing(10);
   pHWizardLayout->addWidget(mpWizardBatchCheck);
   pHWizardLayout->addStretch(10);
   pHWizardLayout->addWidget(pExecuteButton);

   QFrame* pHLine = new QFrame(pInfoWidget);
   pHLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   //////////////////
   // Item widgets //
   //////////////////

   QLabel* pItemLabel = new QLabel("Selected Item(s)", pInfoWidget);
   pItemLabel->setFont(ftHeading);

   QStringList columnNames;
   columnNames.append("Item");
   columnNames.append("Order");
   columnNames.append("Name");
   columnNames.append("Type");
   columnNames.append("Description");

   QLabel* pInLabel = new QLabel("Inputs:", pInfoWidget);
   mpInputsTree = new QTreeWidget(pInfoWidget);
   mpInputsTree->setColumnCount(columnNames.size());
   mpInputsTree->setHeaderLabels(columnNames);
   mpInputsTree->setSortingEnabled(false);
   mpInputsTree->setSelectionMode(QAbstractItemView::SingleSelection);
   mpInputsTree->setRootIsDecorated(false);
   mpInputsTree->installEventFilter(this);

   QHeaderView* pHeader = mpInputsTree->header();
   if (pHeader != NULL)
   {
      pHeader->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
      pHeader->setStretchLastSection(false);
      pHeader->resizeSection(1, 50);
   }

   connect(mpInputsTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(editItem(QTreeWidgetItem*)));

   QLabel* pOutLabel = new QLabel("Outputs:", pInfoWidget);
   mpOutputsTree = new QTreeWidget(pInfoWidget);
   mpOutputsTree->setColumnCount(columnNames.size());
   mpOutputsTree->setHeaderLabels(columnNames);
   mpOutputsTree->setSortingEnabled(false);
   mpOutputsTree->setSelectionMode(QAbstractItemView::SingleSelection);
   mpOutputsTree->setRootIsDecorated(false);
   mpOutputsTree->installEventFilter(this);

   pHeader = mpOutputsTree->header();
   if (pHeader != NULL)
   {
      pHeader->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
      pHeader->setStretchLastSection(false);
      pHeader->resizeSection(1, 50);
   }

   connect(mpOutputsTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(editItem(QTreeWidgetItem*)));

   mpItemBatchCheck = new QCheckBox("Run Item in Batch Mode", pInfoWidget);
   mpItemBatchCheck->setTristate(true);
   mpItemBatchCheck->setEnabled(false);
   mpItemBatchCheck->installEventFilter(this);
   connect(mpItemBatchCheck, SIGNAL(clicked()), this, SLOT(setItemBatchMode()));

   mpConnectButton = new QPushButton("&Connect", pInfoWidget);
   mpConnectButton->setFixedWidth(70);
   mpConnectButton->setEnabled(false);
   mpConnectButton->installEventFilter(this);
   connect(mpConnectButton, SIGNAL(clicked()), this, SLOT(connectItems()));

   QHBoxLayout* pHItemLayout = new QHBoxLayout();
   pHItemLayout->setMargin(0);
   pHItemLayout->setSpacing(10);
   pHItemLayout->addWidget(mpItemBatchCheck);
   pHItemLayout->addStretch(10);
   pHItemLayout->addWidget(mpConnectButton);

   ////////////////
   // Status bar //
   ////////////////

   statusBar();

   ////////////
   // Layout //
   ////////////

   QGridLayout* pGrid = new QGridLayout(pInfoWidget);
   pGrid->setMargin(5);
   pGrid->setSpacing(5);
   pGrid->addWidget(pWizardLabel, 0, 0);
   pGrid->addWidget(pNameLabel, 1, 0);
   pGrid->addWidget(mpNameEdit, 2, 0);
   pGrid->addWidget(pMenuLabel, 3, 0);
   pGrid->addWidget(mpMenuEdit, 4, 0);
   pGrid->addLayout(pHWizardLayout, 5, 0);
   pGrid->addWidget(pHLine, 6, 0);
   pGrid->addWidget(pItemLabel, 7, 0);
   pGrid->addWidget(pInLabel, 8, 0);
   pGrid->addWidget(mpInputsTree, 9, 0);
   pGrid->setRowStretch(9, 10);
   pGrid->addWidget(pOutLabel, 10, 0);
   pGrid->addWidget(mpOutputsTree, 11, 0);
   pGrid->setRowStretch(11, 10);
   pGrid->addLayout(pHItemLayout, 12, 0);

   ////////////////////
   // Initialization //
   ////////////////////

   setAttribute(Qt::WA_DeleteOnClose);
   newWizard();
}

WizardBuilder::~WizardBuilder()
{
}

bool WizardBuilder::eventFilter(QObject* o, QEvent* e)
{
   if (e != NULL)
   {
      if (e->type() == QEvent::KeyPress)
      {
         QKeyEvent* k = static_cast<QKeyEvent*>(e);
         if (k->key() == Qt::Key_Delete)
         {
            keyPressEvent(k);
            return true;   // Prevent event from being passed onto widgets
         }
      }
   }

   return QMainWindow::eventFilter(o, e);
}

void WizardBuilder::keyPressEvent(QKeyEvent* e)
{
   if (e == NULL)
   {
      return;
   }

   if (e->key() == Qt::Key_Delete)
   {
      removeItems();
   }
}

void WizardBuilder::closeEvent(QCloseEvent* e)
{
   bool bSuccess = destroyCurrentWizard();
   if (bSuccess == true)
   {
      QMainWindow::closeEvent(e);
   }
   else if (e != NULL)
   {
      e->ignore();
   }
}

void WizardBuilder::buildWizard(WizardObject* pWizardObject)
{
   WizardObjectImp* pWizard = dynamic_cast<WizardObjectImp*>(pWizardObject);
   if (pWizard == NULL)
   {
      return;
   }

   // Name
   string wizardName = "";
   if (pWizard != NULL)
   {
      wizardName = pWizard->getName();
   }

   mpNameEdit->clear();

   if (wizardName.empty() == false)
   {
      QString strName = QString::fromStdString(wizardName);
      mpNameEdit->setText(strName);
   }

   // Menu location
   string menuLocation = "";
   if (pWizard != NULL)
   {
      menuLocation = pWizard->getMenuLocation();
   }

   mpMenuEdit->clear();

   if (menuLocation.empty() == false)
   {
      QString strMenu = QString::fromStdString(menuLocation);
      mpMenuEdit->setText(strMenu);
   }

   // Wizard batch mode
   bool bWizardBatch = false;
   if (pWizard != NULL)
   {
      bWizardBatch = pWizard->isBatch();
   }

   mpWizardBatchCheck->setChecked(bWizardBatch);

   // View
   vector<WizardItem*> items;
   if (pWizard != NULL)
   {
      items = pWizard->getItems();
   }

   mpView->buildView(items, true);
}

void WizardBuilder::buildInputNodes(WizardItem* pItem, const PlugInArgList* pArgList)
{
   WizardItemImp* pItemImp = static_cast<WizardItemImp*>(pItem);
   if (pItemImp == NULL)
   {
      return;
   }

   int iCount = 0;
   if (pArgList != NULL)
   {
      iCount = pArgList->getCount();
   }

   // Clear existing nodes
   pItemImp->clearInputNodes();

   // Set the input nodes
   for (int i = 0; i < iCount; i++)
   {
      // Get the current plug-in arg
      PlugInArg* pArg = NULL;
      pArgList->getArg(i, pArg);

      string argName = "";
      string argType = "";
      string argDescription = "";
      QColor clrNode = Qt::white;

      if (pArg != NULL)
      {
         // Get the node name
         argName = pArg->getName();

         // Get the node type
         argType = pArg->getType();
         if (argType.empty() == true)
         {
            argType = "Unknown";
         }

         argDescription = pArg->getDescription();
      }

      WizardNodeImp* pNode = static_cast<WizardNodeImp*>(pItemImp->addInputNode(argName, argType, argDescription));
      if (pNode != NULL)
      {
         vector<string> validTypes;
         ModelServicesImp* pModel = ModelServicesImp::instance();
         if (pModel != NULL)
         {
            pModel->getElementTypes(argType, validTypes);

            if (validTypes.empty() == true)
            {
               pModel->getDataDescriptorTypes(argType, validTypes);
            }

            if (validTypes.empty() == true)
            {
               pModel->getFileDescriptorTypes(argType, validTypes);
            }
         }

         DesktopServicesImp* pDesktop = DesktopServicesImp::instance();
         if (validTypes.empty() && pDesktop != NULL)
         {
            pDesktop->getViewTypes(argType, validTypes);
         }
         if (validTypes.empty() && pDesktop != NULL)
         {
            pDesktop->getLayerTypes(argType, validTypes);
         }

         pNode->setValidTypes(validTypes);
         if (pArg != NULL)
         {
            pNode->setValue(pArg->getDefaultValue());
         }

         pNode->attach(SIGNAL_NAME(Subject, Modified), Slot(mpView, &WizardView::nodeModified));
         pNode->attach(SIGNAL_NAME(Subject, Deleted), Slot(mpView, &WizardView::nodeDeleted));
      }
   }
}

void WizardBuilder::buildOutputNodes(WizardItem* pItem, const PlugInArgList* pArgList)
{
   WizardItemImp* pItemImp = static_cast<WizardItemImp*>(pItem);
   if (pItemImp == NULL)
   {
      return;
   }

   int iCount = 0;
   if (pArgList != NULL)
   {
      iCount = pArgList->getCount();
   }

   // Clear existing nodes
   pItemImp->clearOutputNodes();

   // Set the output nodes
   for (int i = 0; i < iCount; i++)
   {
      // Get the current plug-in arg
      PlugInArg* pArg = NULL;
      pArgList->getArg(i, pArg);

      string argName = "";
      string argType = "";
      string argDescription = "";
      QColor clrNode = Qt::white;

      if (pArg != NULL)
      {
         // Get the node name
         argName = pArg->getName();

         // Get the node type
         argType = pArg->getType();
         if (argType.empty() == true)
         {
            argType = "Unknown";
         }

         argDescription = pArg->getDescription();
      }

      WizardNodeImp* pNode = static_cast<WizardNodeImp*>(pItemImp->addOutputNode(argName, argType, argDescription));
      if (pNode != NULL)
      {
         vector<string> validTypes;
         ModelServicesImp* pModel = ModelServicesImp::instance();
         if (pModel != NULL)
         {
            pModel->getElementTypes(argType, validTypes);

            if (validTypes.empty() == true)
            {
               pModel->getDataDescriptorTypes(argType, validTypes);
            }

            if (validTypes.empty() == true)
            {
               pModel->getFileDescriptorTypes(argType, validTypes);
            }
         }

         DesktopServicesImp* pDesktop = DesktopServicesImp::instance();
         if (validTypes.empty() && pDesktop != NULL)
         {
            pDesktop->getViewTypes(argType, validTypes);
         }
         if (validTypes.empty() && pDesktop != NULL)
         {
            pDesktop->getLayerTypes(argType, validTypes);
         }

         pNode->setValidTypes(validTypes);
         if (pArg != NULL)
         {
            pNode->setValue(pArg->getDefaultValue());
         }

         pNode->attach(SIGNAL_NAME(Subject, Modified), Slot(mpView, &WizardView::nodeModified));
         pNode->attach(SIGNAL_NAME(Subject, Deleted), Slot(mpView, &WizardView::nodeDeleted));
      }
   }
}

bool WizardBuilder::setItemBatchMode(WizardItem* pItem, bool bBatch)
{
   if (pItem == NULL)
   {
      return false;
   }

   bool bModeSupport = true;

   string itemType = pItem->getType();
   if (itemType != "Value")
   {
      // Get a pointer to the plug-in
      Service<PlugInManagerServices> pManager;

      string itemName = "";
      itemName = pItem->getName();

      PlugInDescriptor* pDescriptor = pManager->getPlugInDescriptor(itemName);
      if (pDescriptor == NULL)
      {
         return false;
      }

      // Update the input and output nodes
      const PlugInArgList* pInArgList = NULL;
      const PlugInArgList* pOutArgList = NULL;

      if (bBatch == true)
      {
         bModeSupport = pDescriptor->hasBatchSupport();
         pInArgList = pDescriptor->getBatchInputArgList();
         pOutArgList = pDescriptor->getBatchOutputArgList();
      }
      else
      {
         bModeSupport = pDescriptor->hasInteractiveSupport();
         pInArgList = pDescriptor->getInteractiveInputArgList();
         pOutArgList = pDescriptor->getInteractiveOutputArgList();
      }

      buildInputNodes(pItem, pInArgList);
      buildOutputNodes(pItem, pOutArgList);
   }

   // Set the mode and support flag in the wizard item
   WizardItemImp* pItemImp = static_cast<WizardItemImp*>(pItem);
   if (pItemImp != NULL)
   {
      pItemImp->setBatchMode(bBatch, bModeSupport);
   }

   updateItemInfo();
   return true;
}

void WizardBuilder::editItem(WizardItem* pItem)
{
   if (pItem == NULL)
   {
      return;
   }

   string itemType = pItem->getType();
   if (itemType != "Value")
   {
      QMessageBox::critical(this, "Wizard Builder", "You can only edit an item with the Value type!");
      return;
   }

   vector<WizardNode*> outputNodes;

   WizardItemImp* pItemImp = static_cast<WizardItemImp*>(pItem);
   if (pItemImp != NULL)
   {
      outputNodes = pItemImp->getOutputNodes();
   }

   if (outputNodes.empty() == true)
   {
      return;
   }

   WizardNodeImp* pNode = static_cast<WizardNodeImp*>(outputNodes.front());
   if (pNode == NULL)
   {
      return;
   }

   string nodeName = pNode->getName();
   string nodeType = pNode->getType();
   void* pValue = pNode->getValue();

   NameTypeValueDlg dlgValue(this);
   dlgValue.setWindowTitle("Wizard Value Item");
   dlgValue.setValue(QString::fromStdString(nodeName), DataVariant(nodeType, pValue));

   int iReturn = dlgValue.exec();
   if (iReturn == QDialog::Accepted)
   {
      QString strName = dlgValue.getName();
      QString strType = dlgValue.getType();
      if ((strName.isEmpty() == true) || (strType.isEmpty() == true))
      {
         return;
      }

      string newNodeName = strName.toStdString();
      string newNodeType = strType.toStdString();

      const DataVariant& var = dlgValue.getValue();

      if (newNodeName != nodeName)
      {
         pNode->setName(newNodeName);
         pItemImp->setName("Value - " + newNodeName);
      }

      if (newNodeType != nodeType)
      {
         vector<string> validTypes;
         validTypes.push_back(newNodeType);

         pNode->setOriginalType(newNodeType);
         pNode->setType(newNodeType);
         pNode->setValidTypes(validTypes);
         pNode->clearConnectedNodes();
      }

      if (var.getPointerToValueAsVoid() != pValue)
      {
         pNode->setValue(var.getPointerToValueAsVoid());
      }

      updateItemInfo();
      setModified();
   }
}

void WizardBuilder::refreshOrder()
{
   if ((mpView != NULL) && (mpWizard != NULL))
   {
      vector<WizardItem*> items = mpWizard->getItems();
      mpView->updateOrder(items);
   }
}

int WizardBuilder::getExecutionOrder(WizardItem* pItem)
{
   if ((pItem == NULL) || (mpWizard == NULL))
   {
      return -1;
   }

   vector<WizardItem*> items = mpWizard->getItems();
   for (unsigned int i = 0; i < items.size(); i++)
   {
      WizardItem* pCurrentItem = items[i];
      if (pCurrentItem == pItem)
      {
         return static_cast<int>(i + 1);
      }
   }

   return -1;
}

bool WizardBuilder::destroyCurrentWizard()
{
   if (mpWizard == NULL)
   {
      return true;
   }

   // Prompt the user to save the wizard if necessary
   if (mbModified == true)
   {
      QString strName = "Untitled";

      string wizardName = mpWizard->getName();
      if (wizardName.empty() == false)
      {
         strName = QString::fromStdString(wizardName);
      }

      int iReturn = QMessageBox::warning(this, "Wizard Builder", "Do you want to save the changes to " +
         strName + "?", QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel);
      if (iReturn == QMessageBox::Yes)
      {
         bool bSuccess = saveWizard();
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

   // Clear the view
   mpView->clear();
   mpView->ensureVisible(0, 0);

   // Reset the canvas size
   if ((mpCanvas->width() != 5000) && (mpCanvas->height() != 5000))
   {
      mpCanvas->resize(5000, 5000);
   }

   // Delete the wizard
   delete mpWizard;
   mpWizard = NULL;

   // Update the wizard filename
   mFilename.clear();

   return true;
}

void WizardBuilder::newWizard()
{
   bool bDestroy = destroyCurrentWizard();
   if (bDestroy == false)
   {
      return;
   }

   setWindowTitle("Wizard Builder - Untitled");

   if (mpWizard == NULL)
   {
      mpWizard = new WizardObjectAdapter();
      if (mpWizard != NULL)
      {
         mpWizard->setName("Wizard");
      }
   }

   buildWizard(mpWizard);
   mbModified = false;
}

void WizardBuilder::openWizard()
{
   if (mpWizard != NULL)
   {
      if (mbModified == true)
      {
         QString strName = "Untitled";

         string wizardName = mpWizard->getName();
         if (wizardName.empty() == false)
         {
            strName = QString::fromStdString(wizardName);
         }

         int iReturn = QMessageBox::warning(this, "Wizard Builder", "Do you want to save the changes to " +
            strName + "?", QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel);
         if (iReturn == QMessageBox::Yes)
         {
            bool bSuccess = saveWizard();
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
   FactoryResource<WizardObject> pWizard(WizardUtilities::readWizard(strOpenFilename.toStdString()));

   if (pWizard.get() == NULL)
   {
      QMessageBox::critical(this, "Wizard Builder", "Could not load the wizard from the file!");
      return;
   }

   // Delete the current wizard object
   delete mpWizard;
   mpWizard = dynamic_cast<WizardObjectAdapter*>(pWizard.release());

   // Update the wizard filename
   mFilename = strOpenFilename;

   // Set the caption
   if (strOpenFilename.isEmpty() == false)
   {
      QFileInfo fileInfo = QFileInfo(strOpenFilename);
      strOpenFilename = fileInfo.fileName();
   }
   else
   {
      strOpenFilename = "Untitled";
   }

   setWindowTitle("Wizard Builder - " + strOpenFilename);

   // Scroll to show the origin
   mpView->ensureVisible(0, 0);

   // Reset the canvas size
   if ((mpCanvas->width() != 5000) && (mpCanvas->height() != 5000))
   {
      mpCanvas->resize(5000, 5000);
   }

   // Construct the canvas items based on the model data
   buildWizard(mpWizard);
   mbModified = false;
}

bool WizardBuilder::saveWizard()
{
   if (mpWizard == NULL)
   {
      return false;
   }

   bool bSuccess = false;
   if (mFilename.isEmpty() == false)
   {
      FILE* pFile = fopen(mFilename.toLatin1(), "wb");
      if (pFile != NULL)
      {
         XMLWriter xml("Wizard");

         bSuccess = mpWizard->toXml(&xml);
         if (bSuccess == true)
         {
            xml.writeToFile(pFile);
         }

         fclose(pFile);

         if (bSuccess == false)
         {
            remove(mFilename.toLatin1());
         }
      }

      if (bSuccess == false)
      {
         QMessageBox::critical(this, "Wizard Builder", "The wizard file '" + mFilename +
            "' could not be saved!");
      }
      else
      {
         string batchFilename = WizardUtilities::deriveBatchWizardFilename(mFilename.toStdString());
         bool bCreatedBatchWizard = false;
         BatchWizard* pBatchWizard = WizardUtilities::createBatchWizardFromWizard(mpWizard, mFilename.toStdString());
         if (pBatchWizard != NULL)
         {
            vector<BatchWizard*> batchWizards;
            batchWizards.push_back(pBatchWizard);
            if (WizardUtilities::writeBatchWizard(batchWizards, batchFilename))
            {
               bCreatedBatchWizard = true;
            }
         }
         delete pBatchWizard;
         if (!bCreatedBatchWizard)
         {
            QMessageBox::critical(this, "Wizard Builder", "Could not save the '" +
               QString::fromStdString(batchFilename) + "' file!");
         }
      }

      mbModified = false;
   }
   else
   {
      bSuccess = saveNewWizard();
   }

   return bSuccess;
}

bool WizardBuilder::saveNewWizard()
{
   if (mpWizard == NULL)
   {
      return false;
   }

   // Get the default wizard directory
   QString strDefaultDir = QDir::currentPath();
   if (mFilename.isEmpty() == true)
   {
      const Filename* pWizardPath = ConfigurationSettings::getSettingWizardPath();
      if (pWizardPath != NULL)
      {
         strDefaultDir = QString::fromStdString(pWizardPath->getFullPathAndName());
      }
   }
   else
   {
      QFileInfo fileInfo = QFileInfo(mFilename);
      strDefaultDir = fileInfo.absolutePath();
   }

   // Create a default filename
   QString strDefaultFile;
   if (mFilename.isEmpty() == false)
   {
      QFileInfo fileInfo(mFilename);
      strDefaultFile = fileInfo.completeBaseName();
   }
   else
   {
      string wizardName = mpWizard->getName();
      if (wizardName.empty() == false)
      {
         strDefaultFile = QString::fromStdString(wizardName);
      }
   }

   if (strDefaultFile.isEmpty() == false)
   {
      strDefaultFile += ".wiz";
   }

   // Invoke a file selection dialog and get the save file name
   QFileDialog dlg(this, "Save Wizard", strDefaultDir, "Wizard Files (*.wiz);;All Files (*)");
   dlg.setAcceptMode(QFileDialog::AcceptSave);
   dlg.setFileMode(QFileDialog::AnyFile);
   dlg.setConfirmOverwrite(true);
   dlg.setDefaultSuffix("wiz");
   dlg.selectFile(strDefaultFile);

   QString strFilename;
   if (dlg.exec() == QDialog::Accepted)
   {
      strFilename = dlg.selectedFiles().front();
   }
   if (strFilename.isEmpty() == true)
   {
      return false;
   }

   // Set the member filename
   mFilename = strFilename;

   // Save the wizard
   bool bSuccess = saveWizard();
   if (bSuccess == true)
   {
      setWindowTitle("Wizard Builder - " + strFilename);
   }

   return bSuccess;
}

void WizardBuilder::wizardPageSetup()
{
}

void WizardBuilder::printWizard()
{
   if (mpView == NULL)
   {
      return;
   }

   Q3CanvasItemList lstItems = mpCanvas->allItems();
   if (lstItems.empty() == true)
   {
      QMessageBox::critical(this, "Wizard Builder", "There are no wizard items to print!");
      return;
   }

   QPrinter wizardPrinter(QPrinter::ScreenResolution);
   SystemServicesImp::instance()->WriteLogInfo(string(APP_NAME) + " is Printing a Wizard");

   QPrintDialog dlg(&wizardPrinter, this);
   if (dlg.exec() == QDialog::Accepted)
   {
      QPainter p;
      if (p.begin(&wizardPrinter) == true)
      {
         mpView->drawView(&p);
         p.end();
      }
   }
}

void WizardBuilder::exitBuilder()
{
   close();
}

void WizardBuilder::cut()
{
   copy();
   removeItems();
}

void WizardBuilder::copy()
{
   if ((mpView == NULL) || (mpWizard == NULL))
   {
      return;
   }

   WizardClipboard* pClipboard = WizardClipboard::instance();
   if (pClipboard == NULL)
   {
      return;
   }

   // Get the selected items from the wizard instead of the view to preserve the execution order
   vector<WizardItem*> items = mpWizard->getItems();
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
   if (mpWizard == NULL)
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
   mpView->deselectAllItems(false);

   // Get the items from the clipboard and add them to the wizard
   vector<WizardItem*> newItems;
   for (unsigned int i = 0; i < items.size(); i++)
   {
      WizardItemImp* pClipboardItem = static_cast<WizardItemImp*>(items[i]);
      if (pClipboardItem != NULL)
      {
         const string& itemName = pClipboardItem->getName();
         const string& itemType = pClipboardItem->getType();

         WizardItemImp* pItem = static_cast<WizardItemImp*>(mpWizard->addItem(itemName, itemType));
         if (pItem != NULL)
         {
            *pItem = *pClipboardItem;
            newItems.push_back(static_cast<WizardItem*>(pItem));
         }
      }
   }

   // Set the item connections
   vector<WizardConnection> connections = WizardItemImp::getConnections(items);
   WizardItemImp::setConnections(newItems, connections);

   // Add the items to the view
   if (mpView != NULL)
   {
      mpView->buildView(newItems, false);
   }

   // Force the items to be in batch mode if the wizard is set to batch mode
   bool bBatch = mpWizardBatchCheck->isChecked();
   if (bBatch == true)
   {
      for (unsigned int i = 0; i < newItems.size(); i++)
      {
         WizardItemImp* pItem = static_cast<WizardItemImp*>(newItems[i]);
         if (pItem != NULL)
         {
            bool bItemBatch = pItem->getBatchMode();
            if (bItemBatch == false)
            {
               setItemBatchMode(pItem, true);
            }
         }
      }
   }

   // Select the pasted items
   mpView->selectItems(newItems, true);

   // Set the modified flag
   setModified();
}

void WizardBuilder::addDesktopItem()
{
   if (mpWizard == NULL)
   {
      QMessageBox::critical(this, "Wizard Builder", "The current wizard is invalid!  A new item cannot be added.");
      return;
   }

   PlugInSelectDlg dlgPlugIns(this);
   dlgPlugIns.setWindowTitle("Select Wizard Item");
   dlgPlugIns.setDisplayedPlugInType(PlugInManagerServices::WizardType());
   dlgPlugIns.setShowPlugInsForWizardOnly(true);

   int iReturn = dlgPlugIns.exec();
   if (iReturn == QDialog::Accepted)
   {
      string itemName = dlgPlugIns.getSelectedPlugInName();

      WizardItemImp* pItem = static_cast<WizardItemImp*>(mpWizard->addItem(itemName, "Wizard"));
      if (pItem != NULL)
      {
         pItem->attach(SIGNAL_NAME(Subject, Modified), Slot(mpView, &WizardView::itemModified));
         pItem->attach(SIGNAL_NAME(Subject, Deleted), Slot(mpView, &WizardView::itemDeleted));

         // Set the batch mode
         bool bBatch = mpWizardBatchCheck->isChecked();
         setItemBatchMode(pItem, bBatch);

         // Set the item position to the viewport origin
         double dXCoord = pItem->getXPosition();
         double dYCoord = pItem->getYPosition();
         QPoint ptContents = mpView->viewportToContents(QPoint(static_cast<int>(dXCoord), static_cast<int>(dYCoord)));

         pItem->setPosition(ptContents.x(), ptContents.y());

         // Set the modified flag
         setModified();
      }
   }
}

void WizardBuilder::addPlugInItem()
{
   if (mpWizard == NULL)
   {
      QMessageBox::critical(this, "Wizard Builder", "The current wizard is invalid!  A new item cannot be added.");
      return;
   }

   PlugInSelectDlg dlgPlugIns(this);
   vector<string> excludedTypes;
   excludedTypes.push_back(PlugInManagerServices::WizardType());
   dlgPlugIns.setExcludedPlugInTypes(excludedTypes);
   dlgPlugIns.setShowPlugInsForWizardOnly(true);

   int iReturn = dlgPlugIns.exec();
   if (iReturn == QDialog::Accepted)
   {
      string itemName = dlgPlugIns.getSelectedPlugInName();
      string type = dlgPlugIns.getSelectedPlugInType();

      if ((itemName.empty() == true) || (type.empty() == true))
      {
         return;
      }

      WizardItemImp* pItem = static_cast<WizardItemImp*>(mpWizard->addItem(itemName, type));
      if (pItem != NULL)
      {
         pItem->attach(SIGNAL_NAME(Subject, Modified), Slot(mpView, &WizardView::itemModified));
         pItem->attach(SIGNAL_NAME(Subject, Deleted), Slot(mpView, &WizardView::itemDeleted));

         // Set the batch mode
         bool bBatch = mpWizardBatchCheck->isChecked();
         setItemBatchMode(pItem, bBatch);

         // Set the item position to the viewport origin
         double dXCoord = pItem->getXPosition();
         double dYCoord = pItem->getYPosition();
         QPoint ptContents = mpView->viewportToContents(QPoint(static_cast<int>(dXCoord), static_cast<int>(dYCoord)));

         pItem->setPosition(ptContents.x(), ptContents.y());

         // Set the modified flag
         setModified();
      }
   }
}

void WizardBuilder::addValueItem()
{
   if (mpWizard == NULL)
   {
      QMessageBox::critical(this, "Wizard Builder", "The current wizard is invalid!  A new item cannot be added.");
      return;
   }

   NameTypeValueDlg dlgValue(this);
   dlgValue.setWindowTitle("Wizard Value Item");

   int iReturn = dlgValue.exec();
   if (iReturn == QDialog::Accepted)
   {
      QString strName = dlgValue.getName();
      QString strType = dlgValue.getType();
      if ((strName.isEmpty() == true) || (strType.isEmpty() == true))
      {
         return;
      }

      string name = strName.toStdString();
      string type = strType.toStdString();

      WizardItemImp* pItem = static_cast<WizardItemImp*>(mpWizard->addItem("Value - " + name, "Value"));
      if (pItem != NULL)
      {
         pItem->attach(SIGNAL_NAME(Subject, Modified), Slot(mpView, &WizardView::itemModified));
         pItem->attach(SIGNAL_NAME(Subject, Deleted), Slot(mpView, &WizardView::itemDeleted));

         // Set the batch mode
         bool bBatch = mpWizardBatchCheck->isChecked();
         setItemBatchMode(pItem, bBatch);

         // Add the output node
         WizardNodeImp* pNode = static_cast<WizardNodeImp*>(pItem->addOutputNode(name, type, ""));
         if (pNode != NULL)
         {
            const DataVariant& var = dlgValue.getValue();

            pNode->setValue(var.getPointerToValueAsVoid());
            pNode->attach(SIGNAL_NAME(Subject, Modified), Slot(mpView, &WizardView::nodeModified));
            pNode->attach(SIGNAL_NAME(Subject, Deleted), Slot(mpView, &WizardView::nodeDeleted));
         }

         // Set the item position to the viewport origin
         double dXCoord = pItem->getXPosition();
         double dYCoord = pItem->getYPosition();
         QPoint ptContents = mpView->viewportToContents(QPoint(static_cast<int>(dXCoord), static_cast<int>(dYCoord)));

         pItem->setPosition(ptContents.x(), ptContents.y());

         // Set the modified flag
         setModified();
      }
   }
}

void WizardBuilder::selectAllItems()
{
   mpView->selectAllItems(true);
}

void WizardBuilder::editItem()
{
   vector<WizardItem*> selectedItems = mpView->getSelectedItems();

   unsigned int numItems = selectedItems.size();
   if (numItems > 1)
   {
      QMessageBox::warning(this, "Wizard Builder", "Please select only one item!");
   }
   else if (numItems == 1)
   {
      WizardItem* pItem = selectedItems.front();
      if (pItem != NULL)
      {
         editItem(pItem);
      }
   }
   else
   {
      QMessageBox::warning(this, "Wizard Builder", "Please select an item to edit!");
   }
}

void WizardBuilder::editItem(QTreeWidgetItem* pTreeItem)
{
   if (pTreeItem == NULL)
   {
      return;
   }

   // Get the item based on its execution order
   WizardItem* pItem = NULL;
   if (mpWizard != NULL)
   {
      QString strOrder = pTreeItem->text(1);
      int iOrder = strOrder.toInt();

      vector<WizardItem*> items = mpWizard->getItems();
      pItem = items.at(iOrder - 1);
   }

   // Edit the item
   if (pItem != NULL)
   {
      editItem(pItem);
   }
}

void WizardBuilder::removeItems()
{
   if (mpWizard == NULL)
   {
      return;
   }

   // Connections
   vector<WizardConnection> selectedConnections = mpView->getSelectedConnections();

   unsigned int connectionCount = selectedConnections.size();
   for (unsigned int i = 0; i < connectionCount; i++)
   {
      WizardConnection currentConnection = selectedConnections[i];

      if ((currentConnection.mpOutputNode != NULL) && (currentConnection.mpInputNode != NULL))
      {
         static_cast<WizardNodeImp*>(currentConnection.mpOutputNode)->removeConnectedNode(
            currentConnection.mpInputNode);
         static_cast<WizardNodeImp*>(currentConnection.mpInputNode)->removeConnectedNode(
            currentConnection.mpOutputNode);
      }
   }

   // Items
   vector<WizardItem*> selectedItems = mpView->getSelectedItems();

   unsigned int itemCount = selectedItems.size();
   for (unsigned int i = 0; i < itemCount; i++)
   {
      WizardItem* pItem = selectedItems[i];
      if (pItem != NULL)
      {
         mpWizard->removeItem(pItem);
      }
   }

   if (itemCount > 0)
   {
      refreshOrder();
   }

   if ((connectionCount > 0) || (itemCount > 0))
   {
      setModified();
   }
}

void WizardBuilder::itemMousePressEvent(QMouseEvent* e, WizardItem* pItem)
{
   if ((e == NULL) || (pItem == NULL))
   {
      return;
   }

   if (e->button() == Qt::RightButton)
   {
      if ((mpWizard == NULL) || (mpView == NULL))
      {
         return;
      }

      // Create the menu
      QMenu* pMenu = new QMenu(this);
      if (pMenu == NULL)
      {
         return;
      }

      // Create the item name label
      string itemName = pItem->getName();
      if (itemName.empty() == false)
      {
         QLabel* pNameLabel = new QLabel(QString::fromStdString(itemName), pMenu);
         pNameLabel->setAlignment(Qt::AlignCenter);

         QFont ftName = QApplication::font();
         ftName.setBold(true);
         pNameLabel->setFont(ftName);

         QWidgetAction* pLabelAction = new QWidgetAction(pMenu);
         pLabelAction->setDefaultWidget(pNameLabel);
         pMenu->addAction(pLabelAction);
         pMenu->addSeparator();
      }

      // Get the selection state
      QString strCommand = "&Select";

      bool bSelected = false;
      bSelected = mpView->isItemSelected(pItem);
      if (bSelected == true)
      {
         strCommand = "&Deselect";
      }

      // Set the menu commands
      Icons* pIcons = Icons::instance();
      REQUIRE(pIcons != NULL);

      QAction* pSelectionAction = pMenu->addAction(strCommand);
      QAction* pSelectAllAction = pMenu->addAction("Select &All");
      QAction* pDeselectAllAction = pMenu->addAction("Deselect A&ll");
      pMenu->addSeparator();
      QAction* pBatchAction = pMenu->addAction("&Batch Mode");
      QAction* pInteractiveAction = pMenu->addAction("&Interactive Mode");
      pMenu->addSeparator();
      QAction* pEditAction = pMenu->addAction(pIcons->mValueEdit, "&Edit...");
      pMenu->addSeparator();
      QAction* pRemoveConnectionsAction = pMenu->addAction("Remove Co&nnections");
      QAction* pRemoveItemAction = pMenu->addAction(pIcons->mDelete, "&Remove Item");

      // Update the current batch mode state
      bool bBatch = pItem->getBatchMode();
      pBatchAction->setChecked(bBatch);
      pInteractiveAction->setChecked(!bBatch);

      bool bWizardBatch = mpWizard->isBatch();
      if (bWizardBatch == true)
      {
         pBatchAction->setEnabled(false);
         pInteractiveAction->setEnabled(false);
      }

      // Get the menu location
      QPoint ptClick = e->globalPos();

      // Invoke the menu
      QAction* pInvokedAction = pMenu->exec(ptClick);
      if (pInvokedAction == pSelectionAction)
      {
         if (bSelected == false)
         {
            mpView->selectItem(pItem, true);
         }
         else
         {
            mpView->deselectItem(pItem, true);
         }
      }
      else if (pInvokedAction == pSelectAllAction)
      {
         selectAllItems();
      }
      else if (pInvokedAction == pDeselectAllAction)
      {
         mpView->deselectAllItems(true);
      }
      else if (pInvokedAction == pBatchAction)
      {
         if (bBatch == false)
         {
            setItemBatchMode(pItem, true);
         }
      }
      else if (pInvokedAction == pInteractiveAction)
      {
         if (bBatch == true)
         {
            setItemBatchMode(pItem, false);
         }
      }
      else if (pInvokedAction == pEditAction)
      {
         editItem(pItem);
      }
      else if (pInvokedAction == pRemoveConnectionsAction)
      {
         // Input nodes
         vector<WizardNode*> inputNodes = pItem->getInputNodes();
         for (unsigned int i = 0; i < inputNodes.size(); i++)
         {
            WizardNodeImp* pNode = static_cast<WizardNodeImp*>(inputNodes[i]);
            if (pNode != NULL)
            {
               pNode->clearConnectedNodes();
            }
         }

         // Output nodes
         vector<WizardNode*> outputNodes = pItem->getOutputNodes();
         for (unsigned int i = 0; i < outputNodes.size(); i++)
         {
            WizardNodeImp* pNode = static_cast<WizardNodeImp*>(outputNodes[i]);
            if (pNode != NULL)
            {
               pNode->clearConnectedNodes();
            }
         }

         if ((inputNodes.empty() == false) || (outputNodes.empty() == false))
         {
            setModified();
         }
      }
      else if (pInvokedAction == pRemoveItemAction)
      {
         mpWizard->removeItem(pItem);
         refreshOrder();
         setModified();
      }
   }
}

void WizardBuilder::itemMouseDoubleClickEvent(QMouseEvent* e, WizardItem* pItem)
{
   if ((e == NULL) || (pItem == NULL))
   {
      return;
   }

   if (e->button() != Qt::LeftButton)
   {
      return;
   }

   bool bSelected = false;
   if (mpView != NULL)
   {
      bSelected = mpView->isItemSelected(pItem);
   }

   if (bSelected == true)
   {
      editItem(pItem);
   }
}

void WizardBuilder::setWizardName(const QString& strName)
{
   if (mpWizard == NULL)
   {
      return;
   }

   string wizardName = "";
   if (strName.isEmpty() == false)
   {
      wizardName = strName.toStdString();
   }

   string currentName = mpWizard->getName();
   if (wizardName != currentName)
   {
      mpWizard->setName(wizardName);
      setModified();
   }
}

void WizardBuilder::setWizardMenuLocation(const QString& strMenu)
{
   if (mpWizard == NULL)
   {
      return;
   }

   string menuLocation = "";
   if (strMenu.isEmpty() == false)
   {
      menuLocation = strMenu.toStdString();
   }

   string currentLocation = mpWizard->getMenuLocation();
   if (menuLocation != currentLocation)
   {
      mpWizard->setMenuLocation(menuLocation);
      setModified();
   }
}

void WizardBuilder::setWizardBatchMode(bool bBatch)
{
   if (mpWizard == NULL)
   {
      return;
   }

   bool bWizardBatch = mpWizard->isBatch();
   if (bWizardBatch == bBatch)
   {
      return;
   }

   mpWizard->setBatch(bBatch);
   setModified();

   if (bBatch == true)
   {
      vector<WizardItem*> items = mpWizard->getItems();
      for (unsigned int i = 0; i < items.size(); i++)
      {
         WizardItemImp* pItem = static_cast<WizardItemImp*>(items[i]);
         if (pItem != NULL)
         {
            bool bItemBatch = pItem->getBatchMode();
            if (bBatch != bItemBatch)
            {
               setItemBatchMode(pItem, bBatch);
            }
         }
      }

      // Gray out the item batch check
      mpItemBatchCheck->setEnabled(false);
   }
   else
   {
      // Update the item batch check
      updateItemInfo();
   }
}

void WizardBuilder::setItemPosition(WizardItem* pItem, double dX, double dY)
{
   WizardItemImp* pItemImp = static_cast<WizardItemImp*>(pItem);
   if (pItemImp != NULL)
   {
      pItemImp->setPosition(dX, dY);
      setModified();
   }
}

void WizardBuilder::setItemBatchMode()
{
   bool bBatch = true;
   if (mpItemBatchCheck->checkState() == Qt::Unchecked)
   {
      bBatch = false;
   }

   if (mpItemBatchCheck->checkState() == Qt::PartiallyChecked)
   {
      mpItemBatchCheck->setCheckState(Qt::Checked);
   }

   vector<WizardItem*> selectedItems = mpView->getSelectedItems();
   int iModifiedCount = 0;

   int iCount = selectedItems.size();
   for (int i = 0; i < iCount; i++)
   {
      WizardItemImp* pItem = static_cast<WizardItemImp*>(selectedItems.at(i));
      if (pItem != NULL)
      {
         bool bItemBatch = pItem->getBatchMode();
         if (bBatch != bItemBatch)
         {
            setItemBatchMode(pItem, bBatch);
            iModifiedCount++;
         }
      }
   }

   if (iModifiedCount > 0)
   {
      setModified();
   }
}

void WizardBuilder::connectItems()
{
   // Get the list view item pointers
   QTreeWidgetItem* pInputTreeItem = mpInputsTree->currentItem();
   if (pInputTreeItem == NULL)
   {
      QMessageBox::warning(this, "Wizard Builder", "Please select an input to connect!");
      return;
   }

   QTreeWidgetItem* pOutputTreeItem = mpOutputsTree->currentItem();
   if (pOutputTreeItem == NULL)
   {
      QMessageBox::warning(this, "Wizard Builder", "Please select an output to connect!");
      return;
   }

   // Get the item name and node name and type
   QString strInputItemName = pInputTreeItem->text(0);
   QString strInputItemOrder = pInputTreeItem->text(1);
   QString strInputNodeName = pInputTreeItem->text(2);
   QString strInputNodeType = pInputTreeItem->text(3);
   QString strOutputItemName = pOutputTreeItem->text(0);
   QString strOutputItemOrder = pOutputTreeItem->text(1);
   QString strOutputNodeName = pOutputTreeItem->text(2);
   QString strOutputNodeType = pOutputTreeItem->text(3);

   string inputItemName = strInputItemName.toStdString();
   int iInputItemOrder = strInputItemOrder.toInt();
   string inputNodeName = strInputNodeName.toStdString();
   string inputNodeType = strInputNodeType.toStdString();
   string outputItemName = strOutputItemName.toStdString();
   int iOutputItemOrder = strOutputItemOrder.toInt();
   string outputNodeName = strOutputNodeName.toStdString();
   string outputNodeType = strOutputNodeType.toStdString();

   // Check for mismatched node types
   if (strInputNodeType != strOutputNodeType)
   {
      QMessageBox::warning(this, "Wizard Builder", "The items cannot be connected "
         "-- the types do not match!");
      return;
   }

   // Get the WizardItem pointers
   WizardItem* pInputItem = NULL;
   WizardItem* pOutputItem = NULL;

   vector<WizardItem*> selectedItems = mpView->getSelectedItems();

   int iCount = 0;
   iCount = selectedItems.size();
   for (int i = 0; i < iCount; i++)
   {
      WizardItem* pItem = NULL;
      pItem = selectedItems.at(i);
      if (pItem != NULL)
      {
         string itemName = "";
         itemName = pItem->getName();

         int iItemOrder = 0;
         iItemOrder = getExecutionOrder(pItem);

         if (itemName.empty() == false)
         {
            if ((itemName == outputItemName) && (iItemOrder == iOutputItemOrder))
            {
               pOutputItem = pItem;
            }

            if ((itemName == inputItemName) && (iItemOrder == iInputItemOrder))
            {
               pInputItem = pItem;
            }
         }
      }
   }

   if ((pInputItem == NULL) || (pOutputItem == NULL))
   {
      return;
   }

   // Get the nodes
   WizardNodeImp* pOutputNode = static_cast<WizardNodeImp*>(pOutputItem->getOutputNode(outputNodeName, outputNodeType));
   if (pOutputNode == NULL)
   {
      return;
   }

   WizardNodeImp* pInputNode = static_cast<WizardNodeImp*>(pInputItem->getInputNode(inputNodeName, inputNodeType));
   if (pInputNode == NULL)
   {
      return;
   }

   // Check for circular connections between the two items
   bool bConnected = static_cast<WizardItemImp*>(pOutputItem)->isItemConnected(pInputItem, true);
   if (bConnected == true)
   {
      QMessageBox::critical(this, "Wizard Builder", "These nodes cannot be connected "
         "since a circular connection between the two items would be created!");
      return;
   }

   // Check if the nodes are already connected
   bConnected = pInputNode->isNodeConnected(pOutputNode);
   if (bConnected == true)
   {
      QMessageBox::warning(this, "Wizard Builder", "These nodes are already connected!");
      return;
   }

   // Check for multiple output nodes connected to the same input node
   if (pInputNode->getNumConnectedNodes() > 0)
   {
      QMessageBox::warning(this, "Wizard Builder", "The input node already has a valid input.  "
         "The connection will not be made.");
      return;
   }

   // Connect the nodes
   pOutputNode->addConnectedNode(pInputNode);
   pInputNode->addConnectedNode(pOutputNode);
}

void WizardBuilder::updateConnectedItemsOrder(WizardItem* pOutputItem, WizardItem* pInputItem)
{
   if ((mpWizard == NULL) || (pOutputItem == NULL) || (pInputItem == NULL))
   {
      return;
   }

   // Update the execution order if necessary
   int iOutputOrder = getExecutionOrder(pOutputItem);
   int iInputOrder = getExecutionOrder(pInputItem);

   if (iOutputOrder < iInputOrder)
   {
      return;
   }

   while (iOutputOrder > iInputOrder)
   {
      bool bSuccess = mpWizard->decreaseItemOrder(pOutputItem);
      if (bSuccess == false)
      {
         break;
      }

      iOutputOrder = getExecutionOrder(pOutputItem);
      iInputOrder = getExecutionOrder(pInputItem);
   }

   refreshOrder();
   updateItemInfo();
   setModified();
}

void WizardBuilder::increaseCurrentItemOrder()
{
   if (mpWizard == NULL)
   {
      return;
   }

   vector<WizardItem*> selectedItems = mpView->getSelectedItems();

   int iCount = selectedItems.size();
   if (iCount > 1)
   {
      QMessageBox::warning(this, "Wizard Builder", "Please select only one item!");
      return;
   }
   else if (iCount == 1)
   {
      WizardItem* pItem = selectedItems.front();
      if (pItem != NULL)
      {
         bool bSuccess = mpWizard->increaseItemOrder(pItem);
         if (bSuccess == true)
         {
            refreshOrder();
            setModified();
         }
      }
   }
}

void WizardBuilder::decreaseCurrentItemOrder()
{
   if (mpWizard == NULL)
   {
      return;
   }

   vector<WizardItem*> selectedItems = mpView->getSelectedItems();

   int iCount = selectedItems.size();
   if (iCount > 1)
   {
      QMessageBox::warning(this, "Wizard Builder", "Please select only one item!");
      return;
   }
   else if (iCount == 1)
   {
      WizardItem* pItem = selectedItems.front();
      if (pItem != NULL)
      {
         bool bSuccess = mpWizard->decreaseItemOrder(pItem);
         if (bSuccess == true)
         {
            refreshOrder();
            setModified();
         }
      }
   }
}

void WizardBuilder::updateItemInfo()
{
   mpInputsTree->clear();
   mpOutputsTree->clear();

   vector<WizardItem*> selectedItems = mpView->getSelectedItems();

   int iNumSelected = 0;
   int iNumBatch = 0;

   int iCount = selectedItems.size();
   for (int i = 0; i < iCount; i++)
   {
      WizardItem* pItem = selectedItems.at(i);
      if (pItem == NULL)
      {
         continue;
      }

      QString strItemName;
      QString strOrder;

      // Name
      string itemName = pItem->getName();
      if (itemName.empty() == false)
      {
         strItemName = QString::fromStdString(itemName);
      }

      // Execution order
      int iOrder = getExecutionOrder(pItem);
      strOrder = QString::number(iOrder);

      // Input nodes
      vector<WizardNode*> inputNodes = pItem->getInputNodes();

      unsigned int j = 0;
      unsigned int uiNodes = inputNodes.size();
      for (j = 0; j < uiNodes; j++)
      {
         WizardNode* pNode = inputNodes.at(j);
         if (pNode != NULL)
         {
            QString strNodeName;
            QString strNodeType;
            QString strNodeDescription;

            string nodeName = pNode->getName();
            if (nodeName.empty() == false)
            {
               strNodeName = QString::fromStdString(nodeName);
            }

            string nodeType = pNode->getType();
            if (nodeType.empty() == false)
            {
               strNodeType = QString::fromStdString(nodeType);
            }

            string nodeDescripton = pNode->getDescription();
            if (nodeDescripton.empty() == false)
            {
               strNodeDescription = QString::fromStdString(nodeDescripton);
            }

            QTreeWidgetItem* pTreeItem = new QTreeWidgetItem(mpInputsTree);
            if (pTreeItem != NULL)
            {
               pTreeItem->setText(0, strItemName);
               pTreeItem->setText(1, strOrder);
               pTreeItem->setText(2, strNodeName);
               pTreeItem->setText(3, strNodeType);
               pTreeItem->setText(4, strNodeDescription);
            }
         }
      }

      // Output nodes
      vector<WizardNode*> outputNodes = pItem->getOutputNodes();

      uiNodes = outputNodes.size();
      for (j = 0; j < uiNodes; j++)
      {
         WizardNode* pNode = outputNodes.at(j);
         if (pNode != NULL)
         {
            QString strNodeName;
            QString strNodeType;
            QString strNodeDescription;

            string nodeName = pNode->getName();
            if (nodeName.empty() == false)
            {
               strNodeName = QString::fromStdString(nodeName);
            }

            string nodeType = pNode->getType();
            if (nodeType.empty() == false)
            {
               strNodeType = QString::fromStdString(nodeType);
            }

            string nodeDescription = pNode->getDescription();
            if (nodeDescription.empty() == false)
            {
               strNodeDescription = QString::fromStdString(nodeDescription);
            }
            

            QTreeWidgetItem* pTreeItem = new QTreeWidgetItem(mpOutputsTree);
            if (pTreeItem != NULL)
            {
               pTreeItem->setText(0, strItemName);
               pTreeItem->setText(1, strOrder);
               pTreeItem->setText(2, strNodeName);
               pTreeItem->setText(3, strNodeType);
               pTreeItem->setText(4, strNodeDescription);
            }
         }
      }

      iNumSelected++;

      if (pItem->getBatchMode() == true)
      {
         iNumBatch++;
      }
   }

   // Update the batch check
   if (iNumBatch == 0)
   {
      mpItemBatchCheck->setCheckState(Qt::Unchecked);
   }
   else if (iNumBatch == iNumSelected)
   {
      mpItemBatchCheck->setCheckState(Qt::Checked);
   }
   else
   {
      mpItemBatchCheck->setCheckState(Qt::PartiallyChecked);
   }

   if (mpWizardBatchCheck->isChecked() == false)
   {
      // Gray out the batch check if no items are selected
      if (iNumSelected == 0)
      {
         mpItemBatchCheck->setEnabled(false);
      }
      else
      {
         mpItemBatchCheck->setEnabled(true);
      }
   }

   // Gray out the connect button if no items are selected
   if (iNumSelected == 0)
   {
      mpConnectButton->setEnabled(false);
   }
   else
   {
      mpConnectButton->setEnabled(true);
   }
}

void WizardBuilder::setModified()
{
   mbModified = true;
}

void WizardBuilder::runWizard()
{
   vector<WizardItem*> items;
   if (mpWizard != NULL)
   {
      items = mpWizard->getItems();
   }

   if (items.size() == 0)
   {
      QMessageBox::critical(this, "Wizard Builder", "There are no items in the wizard to execute!");
      return;
   }

   WizardUtilities::runWizard(mpWizard);
}
