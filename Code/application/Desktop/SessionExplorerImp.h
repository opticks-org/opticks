/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SESSIONEXPLORERIMP_H
#define SESSIONEXPLORERIMP_H

#include "DockWindowImp.h"
#include "SessionExplorer.h"

class QAction;
class QTreeView;
class SessionItemModel;

class SessionExplorerImp : public DockWindowImp
{
   Q_OBJECT

public:
   SessionExplorerImp(const std::string& id, const std::string& windowName, QWidget* pParent = 0);
   ~SessionExplorerImp();

   std::list<ContextMenuAction> getContextMenuActions() const;

   void setItemViewType(SessionExplorer::ItemViewType itemView);
   SessionExplorer::ItemViewType getItemViewType() const;

   void setSelectedSessionItems(const std::vector<SessionItem*>& selectedItems);
   std::vector<SessionItem*> getSelectedSessionItems() const;
   SessionItem* getCurrentSessionItem() const;

   void setSelectedSessionItems(SessionExplorer::ItemViewType itemView, const std::vector<SessionItem*>& selectedItems);
   std::vector<SessionItem*> getSelectedSessionItems(SessionExplorer::ItemViewType itemView) const;
   SessionItem* getCurrentSessionItem(SessionExplorer::ItemViewType itemView) const;

   void expandSessionItem(SessionItem* pItem);
   void collapseSessionItem(SessionItem* pItem);
   bool isSessionItemExpanded(SessionItem* pItem) const;

   void updateData(SessionItem* pItem);

protected:
   bool event(QEvent* pEvent);
   bool eventFilter(QObject* pObject, QEvent* pEvent);

   void updateContextMenu(Subject& subject, const std::string& signal, const boost::any& value);
   QTreeView* getCurrentTreeView() const;
   QTreeView* getTreeView(SessionExplorer::ItemViewType itemView) const;
   SessionItemModel* getCurrentModel() const;
   SessionItemModel* getModel(QTreeView* pTreeView) const;

protected slots:
   void expandCurrentTreeView();
   void collapseCurrentTreeView();
   void treeViewChanged();
   void renameItem();

private:
   QTreeView* mpWindowTree;
   QTreeView* mpAnimationTree;
   QTreeView* mpElementTree;
   QTreeView* mpPlugInTree;

   QAction* mpSeparatorAction;
   QAction* mpExpandAction;
   QAction* mpCollapseAction;
   QAction* mpRenameAction;
};

#define SESSIONEXPLORERADAPTER_METHODS(impClass) \
   DOCKWINDOWADAPTER_METHODS(impClass) \
   void setItemViewType(ItemViewType itemView) \
   { \
      impClass::setItemViewType(itemView); \
   } \
   ItemViewType getItemViewType() const \
   { \
      return impClass::getItemViewType(); \
   } \
   void setSelectedSessionItems(const std::vector<SessionItem*>& selectedItems) \
   { \
      impClass::setSelectedSessionItems(selectedItems); \
   } \
   std::vector<SessionItem*> getSelectedSessionItems() const \
   { \
      return impClass::getSelectedSessionItems(); \
   } \
   SessionItem* getCurrentSessionItem() const \
   { \
      return impClass::getCurrentSessionItem(); \
   } \
   void setSelectedSessionItems(ItemViewType itemView, const std::vector<SessionItem*>& selectedItems) \
   { \
      impClass::setSelectedSessionItems(itemView, selectedItems); \
   } \
   std::vector<SessionItem*> getSelectedSessionItems(ItemViewType itemView) const \
   { \
      return impClass::getSelectedSessionItems(itemView); \
   } \
   SessionItem* getCurrentSessionItem(ItemViewType itemView) const \
   { \
      return impClass::getCurrentSessionItem(itemView); \
   } \
   void expandSessionItem(SessionItem* pItem) \
   { \
      impClass::expandSessionItem(pItem); \
   } \
   void collapseSessionItem(SessionItem* pItem) \
   { \
      impClass::collapseSessionItem(pItem); \
   } \
   bool isSessionItemExpanded(SessionItem* pItem) const \
   { \
      return impClass::isSessionItemExpanded(pItem); \
   }

#endif
