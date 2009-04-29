/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */
#include "SessionExplorerImp.h"

#include "AnimationModel.h"
#include "ContextMenuAction.h"
#include "ContextMenuActions.h"
#include "ContextMenuImp.h"
#include "AppAssert.h"
#include "DesktopServices.h"
#include "ElementModel.h"
#include "Icons.h"
#include "PlugInModel.h"
#include "SessionItem.h"
#include "SessionItemModel.h"
#include "WindowModel.h"

#include <QtGui/QApplication>
#include <QtGui/QClipboard>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QHeaderView>
#include <QtGui/QMenu>
#include <QtGui/QMessageBox>
#include <QtGui/QSortFilterProxyModel>
#include <QtGui/QTabWidget>
#include <QtGui/QTreeView>

using namespace std;

SessionExplorerImp::SessionExplorerImp(const string& id, const string& windowName, QWidget* pParent) :
   DockWindowImp(id, windowName, pParent),
   mpWindowTree(NULL),
   mpAnimationTree(NULL),
   mpElementTree(NULL),
   mpPlugInTree(NULL),
   mpSeparatorAction(NULL),
   mpExpandAction(NULL),
   mpCollapseAction(NULL),
   mpRenameAction(NULL),
   mpCopyNameToClipboardAction(NULL)
{
   // Tab widget
   QTabWidget* pTabWidget = new QTabWidget(this);
   pTabWidget->setTabPosition(QTabWidget::South);

   // Windows tree
   mpWindowTree = new QTreeView();
   mpWindowTree->setEditTriggers(QAbstractItemView::EditKeyPressed | QAbstractItemView::SelectedClicked);
   mpWindowTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
   mpWindowTree->setRootIsDecorated(true);
   mpWindowTree->setSortingEnabled(true);
   mpWindowTree->setSelectionBehavior(QAbstractItemView::SelectItems);
   mpWindowTree->sortByColumn(0, Qt::AscendingOrder);
   mpWindowTree->setDragEnabled(true);
   mpWindowTree->setDragDropMode(QAbstractItemView::InternalMove);
   mpWindowTree->setDropIndicatorShown(true);
   mpWindowTree->viewport()->installEventFilter(this);

   QHeaderView* pWindowHeader = mpWindowTree->header();
   if (pWindowHeader != NULL)
   {
      pWindowHeader->hide();
   }

   // Animations tree
   mpAnimationTree = new QTreeView();
   mpAnimationTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
   mpAnimationTree->setRootIsDecorated(true);
   mpAnimationTree->setSortingEnabled(true);
   mpAnimationTree->setSelectionBehavior(QAbstractItemView::SelectItems);
   mpAnimationTree->sortByColumn(0, Qt::AscendingOrder);

   QHeaderView* pAnimationHeader = mpAnimationTree->header();
   if (pAnimationHeader != NULL)
   {
      pAnimationHeader->hide();
   }

   // Elements tree
   mpElementTree = new QTreeView();
   mpElementTree->setEditTriggers(QAbstractItemView::EditKeyPressed | QAbstractItemView::SelectedClicked);
   mpElementTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
   mpElementTree->setRootIsDecorated(true);
   mpElementTree->setSortingEnabled(true);
   mpElementTree->setDragEnabled(true);
   mpElementTree->setDragDropMode(QAbstractItemView::DragOnly);
   mpElementTree->setSelectionBehavior(QAbstractItemView::SelectItems);
   mpElementTree->sortByColumn(0, Qt::AscendingOrder);

   QHeaderView* pElementHeader = mpElementTree->header();
   if (pElementHeader != NULL)
   {
      pElementHeader->hide();
   }

   // Plug-ins tree
   mpPlugInTree = new QTreeView();
   mpPlugInTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
   mpPlugInTree->setRootIsDecorated(true);
   mpPlugInTree->setSortingEnabled(true);
   mpPlugInTree->setSelectionBehavior(QAbstractItemView::SelectItems);
   mpPlugInTree->sortByColumn(0, Qt::AscendingOrder);

   QHeaderView* pPlugInHeader = mpPlugInTree->header();
   if (pPlugInHeader != NULL)
   {
      pPlugInHeader->hide();
   }

   // Actions
   Service<DesktopServices> pDesktop;
   string shortcutContext = windowTitle().toStdString();

   mpSeparatorAction = new QAction(this);
   mpSeparatorAction->setSeparator(true);

   mpExpandAction = new QAction("&Expand All", this);
   mpExpandAction->setAutoRepeat(false);
   mpExpandAction->setShortcutContext(Qt::WidgetShortcut);
   mpExpandAction->setStatusTip("Expands all nodes in the tree");
   pDesktop->initializeAction(mpExpandAction, shortcutContext);
   mpWindowTree->addAction(mpExpandAction);
   mpAnimationTree->addAction(mpExpandAction);
   mpElementTree->addAction(mpExpandAction);
   mpPlugInTree->addAction(mpExpandAction);

   mpCollapseAction = new QAction("&Collapse All", this);
   mpCollapseAction->setAutoRepeat(false);
   mpCollapseAction->setShortcutContext(Qt::WidgetShortcut);
   mpCollapseAction->setStatusTip("Collapses all nodes in the tree");
   pDesktop->initializeAction(mpCollapseAction, shortcutContext);
   mpWindowTree->addAction(mpCollapseAction);
   mpAnimationTree->addAction(mpCollapseAction);
   mpElementTree->addAction(mpCollapseAction);
   mpPlugInTree->addAction(mpCollapseAction);

   mpRenameAction = new QAction("Re&name...", this);
   mpRenameAction->setAutoRepeat(false);
   mpRenameAction->setShortcutContext(Qt::WidgetShortcut);
   mpRenameAction->setStatusTip("Rename the selected item");
   pDesktop->initializeAction(mpRenameAction, shortcutContext);
   mpWindowTree->addAction(mpRenameAction);
   mpAnimationTree->addAction(mpRenameAction);
   mpElementTree->addAction(mpRenameAction);
   mpPlugInTree->addAction(mpRenameAction);

   mpCopyNameToClipboardAction = new QAction("Copy name to clipboard", this);
   mpCopyNameToClipboardAction->setAutoRepeat(false);
   mpCopyNameToClipboardAction->setShortcutContext(Qt::WidgetShortcut);
   mpCopyNameToClipboardAction->setStatusTip("Copy the full item name to the clipboard");
   pDesktop->initializeAction(mpCopyNameToClipboardAction, shortcutContext);
   mpWindowTree->addAction(mpCopyNameToClipboardAction);
   mpAnimationTree->addAction(mpCopyNameToClipboardAction);
   mpElementTree->addAction(mpCopyNameToClipboardAction);
   mpPlugInTree->addAction(mpCopyNameToClipboardAction);

   // Initialization
   pTabWidget->addTab(mpWindowTree, "Windows");
   pTabWidget->addTab(mpAnimationTree, "Animations");
   pTabWidget->addTab(mpElementTree, "Elements");
   pTabWidget->addTab(mpPlugInTree, "Plug-Ins");

   Icons* pIcons = Icons::instance();
   REQUIRE(pIcons != NULL);
   setIcon(pIcons->mSessionExplorer);

   setWidget(pTabWidget);

   // Connections
   VERIFYNR(connect(pTabWidget, SIGNAL(currentChanged(int)), this, SLOT(treeViewChanged())));
   VERIFYNR(connect(mpExpandAction, SIGNAL(triggered()), this, SLOT(expandCurrentTreeView())));
   VERIFYNR(connect(mpCollapseAction, SIGNAL(triggered()), this, SLOT(collapseCurrentTreeView())));
   VERIFYNR(connect(mpRenameAction, SIGNAL(triggered()), this, SLOT(renameItem())));
   VERIFYNR(connect(mpCopyNameToClipboardAction, SIGNAL(triggered()), this, SLOT(copyNameToClipboard())));
}

SessionExplorerImp::~SessionExplorerImp()
{
}

list<ContextMenuAction> SessionExplorerImp::getContextMenuActions() const
{
   list<ContextMenuAction> menuActions = DockWindowImp::getContextMenuActions();
   menuActions.push_back(ContextMenuAction(mpSeparatorAction, APP_SESSIONEXPLORER_SEPARATOR_ACTION));
   menuActions.push_back(ContextMenuAction(mpExpandAction, APP_SESSIONEXPLORER_EXPAND_ALL_ACTION));
   menuActions.push_back(ContextMenuAction(mpCollapseAction, APP_SESSIONEXPLORER_COLLAPSE_ALL_ACTION));

   return menuActions;
}

void SessionExplorerImp::setItemViewType(SessionExplorer::ItemViewType itemView)
{
   if (itemView == getItemViewType())
   {
      return;
   }

   QTabWidget* pTabWidget = dynamic_cast<QTabWidget*>(widget());
   if (pTabWidget != NULL)
   {
      QTreeView* pTreeView = getTreeView(itemView);
      if (pTreeView != NULL)
      {
         pTabWidget->setCurrentWidget(pTreeView);
      }
   }
}

SessionExplorer::ItemViewType SessionExplorerImp::getItemViewType() const
{
   SessionExplorer::ItemViewType itemView;

   QTreeView* pCurrentWidget = getCurrentTreeView();
   if (pCurrentWidget == mpWindowTree)
   {
      itemView = SessionExplorer::WINDOW_ITEMS;
   }
   else if (pCurrentWidget == mpAnimationTree)
   {
      itemView = SessionExplorer::ANIMATION_ITEMS;
   }
   else if (pCurrentWidget == mpElementTree)
   {
      itemView = SessionExplorer::ELEMENT_ITEMS;
   }
   else if (pCurrentWidget == mpPlugInTree)
   {
      itemView = SessionExplorer::PLUGIN_ITEMS;
   }

   return itemView;
}

void SessionExplorerImp::setSelectedSessionItems(const vector<SessionItem*>& selectedItems)
{
   SessionExplorer::ItemViewType itemView = getItemViewType();
   setSelectedSessionItems(itemView, selectedItems);
}

vector<SessionItem*> SessionExplorerImp::getSelectedSessionItems() const
{
   SessionExplorer::ItemViewType itemView = getItemViewType();
   return getSelectedSessionItems(itemView);
}

SessionItem* SessionExplorerImp::getCurrentSessionItem() const
{
   SessionExplorer::ItemViewType itemView = getItemViewType();
   return getCurrentSessionItem(itemView);
}

void SessionExplorerImp::setSelectedSessionItems(SessionExplorer::ItemViewType itemView,
                                                 const vector<SessionItem*>& selectedItems)
{
   if (selectedItems == getSelectedSessionItems(itemView))
   {
      return;
   }

   QTreeView* pTreeView = getTreeView(itemView);
   VERIFYNRV(pTreeView != NULL);

   QItemSelectionModel* pSelectionModel = pTreeView->selectionModel();
   VERIFYNRV(pSelectionModel != NULL);

   QAbstractProxyModel* pProxyModel = dynamic_cast<QAbstractProxyModel*>(pTreeView->model());
   VERIFYNRV(pProxyModel != NULL);

   SessionItemModel* pModel = dynamic_cast<SessionItemModel*>(pProxyModel->sourceModel());
   VERIFYNRV(pModel != NULL);

   // Clear the current selection
   pSelectionModel->clearSelection();

   // Select the new session items
   if (selectedItems.empty() == false)
   {
      // Get the selected items
      QItemSelection selection;

      // Get the session item indexes and add them to the selection
      for (vector<SessionItem*>::const_iterator iter = selectedItems.begin(); iter != selectedItems.end(); ++iter)
      {
         SessionItem* pItem = *iter;
         if (pItem != NULL)
         {
            QModelIndex index = pProxyModel->mapFromSource(pModel->index(pItem));
            if (index.isValid() == true)
            {
               selection.select(index, index);
            }
         }
      }

      // Select the items in the view
      pSelectionModel->select(selection, QItemSelectionModel::Select);
   }
}

vector<SessionItem*> SessionExplorerImp::getSelectedSessionItems(SessionExplorer::ItemViewType itemView) const
{
   vector<SessionItem*> selectedItems;

   QTreeView* pTreeView = getTreeView(itemView);
   if (pTreeView != NULL)
   {
      QItemSelectionModel* pSelectionModel = pTreeView->selectionModel();
      if (pSelectionModel != NULL)
      {
         QModelIndexList selectedIndexes = pSelectionModel->selectedRows();
         for (int i = 0; i < selectedIndexes.count(); ++i)
         {
            QModelIndex index = selectedIndexes[i];
            if (index.isValid() == true)
            {
               SessionItem* pItem = index.data(SessionItemModel::SessionItemRole).value<SessionItem*>();
               if (pItem != NULL)
               {
                  selectedItems.push_back(pItem);
               }
            }
         }
      }
   }

   return selectedItems;
}

SessionItem* SessionExplorerImp::getCurrentSessionItem(SessionExplorer::ItemViewType itemView) const
{
   SessionItem* pItem = NULL;

   QTreeView* pTreeView = getTreeView(itemView);
   if (pTreeView != NULL)
   {
      QModelIndex index = pTreeView->currentIndex();
      if (index.isValid() == true)
      {
         pItem = index.data(SessionItemModel::SessionItemRole).value<SessionItem*>();
      }
   }

   return pItem;
}

void SessionExplorerImp::expandSessionItem(SessionItem* pItem)
{
   QTreeView* pTreeView = getCurrentTreeView();
   VERIFYNRV(pTreeView != NULL);

   QAbstractProxyModel* pProxyModel = dynamic_cast<QAbstractProxyModel*>(pTreeView->model());
   VERIFYNRV(pProxyModel != NULL);

   SessionItemModel* pModel = dynamic_cast<SessionItemModel*>(pProxyModel->sourceModel());
   VERIFYNRV(pModel != NULL);

   QModelIndex index = pProxyModel->mapFromSource(pModel->index(pItem));
   if (index.isValid() == true)
   {
      pTreeView->expand(index);
   }
}

void SessionExplorerImp::collapseSessionItem(SessionItem* pItem)
{
   QTreeView* pTreeView = getCurrentTreeView();
   VERIFYNRV(pTreeView != NULL);

   QAbstractProxyModel* pProxyModel = dynamic_cast<QAbstractProxyModel*>(pTreeView->model());
   VERIFYNRV(pProxyModel != NULL);

   SessionItemModel* pModel = dynamic_cast<SessionItemModel*>(pProxyModel->sourceModel());
   VERIFYNRV(pModel != NULL);

   QModelIndex index = pProxyModel->mapFromSource(pModel->index(pItem));
   if (index.isValid() == true)
   {
      pTreeView->collapse(index);
   }
}

bool SessionExplorerImp::isSessionItemExpanded(SessionItem* pItem) const
{
   QTreeView* pTreeView = getCurrentTreeView();
   VERIFY(pTreeView != NULL);

   QAbstractProxyModel* pProxyModel = dynamic_cast<QAbstractProxyModel*>(pTreeView->model());
   VERIFY(pProxyModel != NULL);

   SessionItemModel* pModel = dynamic_cast<SessionItemModel*>(pProxyModel->sourceModel());
   VERIFY(pModel != NULL);

   QModelIndex index = pProxyModel->mapFromSource(pModel->index(pItem));
   if (index.isValid() == true)
   {
      return pTreeView->isExpanded(index);
   }

   return false;
}

void SessionExplorerImp::updateData(SessionItem* pItem)
{
   if (pItem == NULL)
   {
      return;
   }

   SessionItemModel* pModel = getModel(mpWindowTree);
   if (pModel != NULL)
   {
      pModel->updateData(pItem);
   }

   pModel = getModel(mpAnimationTree);
   if (pModel != NULL)
   {
      pModel->updateData(pItem);
   }

   pModel = getModel(mpElementTree);
   if (pModel != NULL)
   {
      pModel->updateData(pItem);
   }

   pModel = getModel(mpPlugInTree);
   if (pModel != NULL)
   {
      pModel->updateData(pItem);
   }
}

bool SessionExplorerImp::event(QEvent* pEvent)
{
   bool success = DockWindowImp::event(pEvent);
   if ((pEvent != NULL) && (pEvent->type() == QEvent::Polish))
   {
      // To ensure that objects add and remove actions from the correct menu (SessionExplorer or selected
      // SessionItem(s)), attach to the dock window just before the window is shown to make it the last slot connected
      attach(SIGNAL_NAME(DockWindow, AboutToShowContextMenu), Slot(this, &SessionExplorerImp::updateContextMenu));
   }

   return success;
}

bool SessionExplorerImp::eventFilter(QObject* pObject, QEvent* pEvent)
{
   if ((pObject != NULL) && (pEvent != NULL))
   {
      if (pObject == mpWindowTree->viewport())
      {
         QEvent::Type type = pEvent->type();
         if ((type == QEvent::DragEnter) || (type == QEvent::DragMove) || (type == QEvent::Drop))
         {
            QDropEvent* pDropEvent = static_cast<QDropEvent*>(pEvent);

            const QMimeData* pData = pDropEvent->mimeData();
            if (pData != NULL)
            {
               if (pData->hasFormat("application/x-sessionitem") == true)
               {
                  WindowModel* pWindowModel = dynamic_cast<WindowModel*>(mpWindowTree->model());
                  VERIFY(pWindowModel != NULL);

                  SessionItemModel* pSourceModel = dynamic_cast<SessionItemModel*>(pWindowModel->sourceModel());
                  VERIFY(pSourceModel != NULL);

                  // Get the index of the parent item containing the selected layer item(s)
                  QModelIndex index;

                  vector<SessionItem*> selectedLayers = getSelectedSessionItems();
                  if (selectedLayers.empty() == false)
                  {
                     SessionItem* pItem = selectedLayers.front();
                     if (pItem != NULL)
                     {
                        QModelIndex itemIndex = pWindowModel->mapFromSource(pSourceModel->index(pItem));
                        if (itemIndex.isValid() == true)
                        {
                           index = itemIndex.parent();
                        }
                     }
                  }

                  // Update the drop point to be between the layer items
                  QPoint dropPos = pDropEvent->pos();
                  QPoint& newPos = const_cast<QPoint&>(pDropEvent->pos());

                  QRect layersRect;
                  if (index.isValid() == true)
                  {
                     int numLayers = pWindowModel->rowCount(index);
                     for (int i = 0; i < numLayers; ++i)
                     {
                        QModelIndex layerIndex = index.child(i, 0);

                        QRect layerRect = mpWindowTree->visualRect(layerIndex);
                        if (layerRect.isValid() == true)
                        {
                           layersRect = layersRect.united(layerRect);

                           if (layerRect.contains(dropPos) == true)
                           {
                              int topDist = dropPos.y() - layerRect.top();
                              int bottomDist = layerRect.bottom() - dropPos.y();

                              if (topDist < bottomDist)
                              {
                                 newPos.setY(layerRect.top());
                              }
                              else
                              {
                                 newPos.setY(layerRect.bottom());
                              }
                           }
                        }
                     }
                  }

                  // Do not allow drops outside of the sibling layers
                  if (type != QEvent::Drop)
                  {
                     mpWindowTree->setDropIndicatorShown(layersRect.contains(dropPos));
                  }

                  // Send the event to the tree view
                  return false;
               }
            }
         }
      }
   }

   return DockWindowImp::eventFilter(pObject, pEvent);
}

void SessionExplorerImp::updateContextMenu(Subject& subject, const string& signal, const boost::any& value)
{
   QTreeView* pTreeView = getCurrentTreeView();
   if (pTreeView == NULL)
   {
      return;
   }

   ContextMenuImp* pMenu = static_cast<ContextMenuImp*>(boost::any_cast<ContextMenu*>(value));
   if (pMenu == NULL)
   {
      return;
   }

   // Check if the user clicked on a valid item
   const QPoint& mouseLocation = pMenu->getMouseLocation();
   QPoint treePos = pTreeView->mapFromGlobal(mouseLocation);

   QModelIndex selectedIndex = pTreeView->indexAt(treePos);
   if (selectedIndex.isValid() == true)
   {
      // Create a new list of actions for the selected session item(s)
      list<ContextMenuAction> defaultActions;

      // If only one session item is selected, get its context menu actions
      vector<SessionItem*> selectedItems = getSelectedSessionItems();
      if (selectedItems.size() == 1)
      {
         SessionItem* pItem = selectedItems.front();
         if (pItem != NULL)
         {
            defaultActions = pItem->getContextMenuActions();
            SessionItemImp* pItemImp = dynamic_cast<SessionItemImp*>(pItem);
            if (pItemImp != NULL && pItemImp->canRename())
            {
               defaultActions.push_back(ContextMenuAction(mpRenameAction, APP_SESSIONEXPLORER_RENAME_ACTION));
            }
            if (pItemImp != NULL)
            {
               defaultActions.push_back(ContextMenuAction(mpCopyNameToClipboardAction,
                  APP_SESSIONEXPLORER_COPY_NAME_TO_CLIPBOARD_ACTION));
            }
         }
      }

      // Create a new context menu
      QObject* pActionParent = pMenu->getActionParent();
      ContextMenuImp menu(selectedItems, mouseLocation, defaultActions, NULL, pActionParent);

      // Notify to allow additional actions to be added
      notify(SIGNAL_NAME(SessionExplorer, AboutToShowSessionItemContextMenu),
         boost::any(static_cast<ContextMenu*>(&menu)));

      // Remove the default actions from the dock window menu
      pMenu->clear();

      // Add the actions from the new menu
      QList<ContextMenuAction> menuActions = menu.getActions();
      for (int i = 0; i < menuActions.count(); ++i)
      {
         pMenu->addAction(menuActions[i]);
      }
   }
}

QTreeView* SessionExplorerImp::getCurrentTreeView() const
{
   QTabWidget* pTabWidget = dynamic_cast<QTabWidget*>(widget());
   if (pTabWidget != NULL)
   {
      return dynamic_cast<QTreeView*>(pTabWidget->currentWidget());
   }

   return NULL;
}

QTreeView* SessionExplorerImp::getTreeView(SessionExplorer::ItemViewType itemView) const
{
   QTreeView* pTreeView = NULL;
   switch (itemView)
   {
      case SessionExplorer::WINDOW_ITEMS:
         pTreeView = mpWindowTree;
         break;

      case SessionExplorer::ANIMATION_ITEMS:
         pTreeView = mpAnimationTree;
         break;

      case SessionExplorer::ELEMENT_ITEMS:
         pTreeView = mpElementTree;
         break;

      case SessionExplorer::PLUGIN_ITEMS:
         pTreeView = mpPlugInTree;
         break;

      default:
         break;
   }

   return pTreeView;
}

SessionItemModel* SessionExplorerImp::getCurrentModel() const
{
   SessionItemModel* pModel = NULL;

   QTreeView* pTreeView = getCurrentTreeView();
   if (pTreeView != NULL)
   {
      pModel = getModel(pTreeView);
   }

   return pModel;
}

SessionItemModel* SessionExplorerImp::getModel(QTreeView* pTreeView) const
{
   if (pTreeView == NULL)
   {
      return NULL;
   }

   SessionItemModel* pModel = dynamic_cast<SessionItemModel*>(pTreeView->model());
   if (pModel == NULL)
   {
      QAbstractProxyModel* pProxyModel = dynamic_cast<QAbstractProxyModel*>(pTreeView->model());
      if (pProxyModel != NULL)
      {
         pModel = dynamic_cast<SessionItemModel*>(pProxyModel->sourceModel());
      }
   }

   return pModel;
}

void SessionExplorerImp::expandCurrentTreeView()
{
   QTreeView* pTreeView = getCurrentTreeView();
   if (pTreeView != NULL)
   {
      pTreeView->expandAll();
   }
}

void SessionExplorerImp::collapseCurrentTreeView()
{
   QTreeView* pTreeView = getCurrentTreeView();
   if (pTreeView != NULL)
   {
      pTreeView->collapseAll();
   }
}

void SessionExplorerImp::treeViewChanged()
{
   SessionExplorer::ItemViewType itemView = getItemViewType();
   notify(SIGNAL_NAME(SessionExplorer, ItemViewTypeChanged), boost::any(itemView));
}

void SessionExplorerImp::renameItem()
{
   QTreeView* pView = getCurrentTreeView();
   if (pView == NULL)
   {
      return;
   }

   QModelIndex index = pView->currentIndex();
   if (index.isValid() == false)
   {
      return;
   }

   QItemSelectionModel* pSelectionModel = pView->selectionModel();
   if ((pSelectionModel != NULL) && (pSelectionModel->isSelected(index) == true))
   {
      if (index.flags() & Qt::ItemIsEditable)
      {
         pView->edit(index);
      }
      else
      {
         QMessageBox::critical(pView, windowTitle(), "The selected session item cannot be renamed.");
      }
   }
}

void SessionExplorerImp::copyNameToClipboard()
{
   QTreeView* pView = getCurrentTreeView();
   if (pView == NULL)
   {
      return;
   }

   QModelIndex index = pView->currentIndex();
   if (index.isValid())
   {
      SessionItem* pItem = index.data(SessionItemModel::SessionItemRole).value<SessionItem*>();
      if (pItem != NULL)
      {
         QApplication::clipboard()->setText(QString::fromStdString(pItem->getName()));
      }
   }
}

void SessionExplorerImp::initialize()
{
   // Window model
   if (mpWindowTree->model() == NULL)
   {
      WindowModel* pWindowModel = new WindowModel(this);
      mpWindowTree->setModel(pWindowModel);
   }

   // Animation model
   if (mpAnimationTree->model() == NULL)
   {
      AnimationModel* pAnimationModel = new AnimationModel(this);

      QSortFilterProxyModel* pAnimationProxyModel = new QSortFilterProxyModel(this);
      pAnimationProxyModel->setSourceModel(pAnimationModel);
      pAnimationProxyModel->setDynamicSortFilter(true);

      mpAnimationTree->setModel(pAnimationProxyModel);
   }

   // Element model
   if (mpElementTree->model() == NULL)
   {
      ElementModel* pElementModel = new ElementModel(this);

      QSortFilterProxyModel* pElementProxyModel = new QSortFilterProxyModel(this);
      pElementProxyModel->setSourceModel(pElementModel);
      pElementProxyModel->setDynamicSortFilter(true);

      mpElementTree->setModel(pElementProxyModel);
   }

   // Plug-in model
   if (mpPlugInTree->model() == NULL)
   {
      PlugInModel* pPlugInModel = new PlugInModel(this);

      QSortFilterProxyModel* pPlugInProxyModel = new QSortFilterProxyModel(this);
      pPlugInProxyModel->setSourceModel(pPlugInModel);
      pPlugInProxyModel->setDynamicSortFilter(true);

      mpPlugInTree->setModel(pPlugInProxyModel);
   }
}
