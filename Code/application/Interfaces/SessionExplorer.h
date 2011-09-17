/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SESSIONEXPLORER_H
#define SESSIONEXPLORER_H

#include "DockWindow.h"
#include "ModelServices.h"
#include "Service.h"
#include "EnumWrapper.h"

#include <vector>

class SessionItem;

/**
 *  \ingroup ServiceModule
 *  A window that displays all items in the current session.
 *
 *  The session explorer window contains a tab widget that contains tree views
 *  displaying all session items in the current session.  The available views
 *  in the window are defined by the SessionExplorer::ItemViewType enum.
 *
 *  The tree view contained on each tab shows the relationship of session items
 *  to other session items.  The contents of each tree view are updated
 *  automatically.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The user changes the active tab in the window, or the setItemViewType()
 *    method is called.
 *  - The context menu for the selected session item(s) is about to be
 *    displayed.
 *  - Everything else documented in DockWindow.
 *
 *  @see     DockWindow, SessionItem
 */
class SessionExplorer : public DockWindow
{
public:
   /**
    *  Defines the various tree views displaying different types of session
    *  items in the session explorer.
    *
    *  The session explorer window contains tabs that each display a tree view
    *  of session items.  This enum defines the available views and which
    *  session items are displayed.  To change the active view, call
    *  setItemViewType().  The user changes the active view by clicking on a
    *  new tab in the session explorer window.
    */
   enum ItemViewTypeEnum
   {
      WINDOW_ITEMS,     /**< Displays toolbars, dock windows, and workspace
                             windows and their contents. */
      ANIMATION_ITEMS,  /**< Displays all animation controllers and their
                             animations. */
      ELEMENT_ITEMS,    /**< Displays data elements and their children. */
      PLUGIN_ITEMS      /**< Displays all modules, plug-ins, and currently
                             running plug-in instances. */
   };

   /**
    * @EnumWrapper SessionExplorer::ItemViewTypeEnum.
    */
   typedef EnumWrapper<ItemViewTypeEnum> ItemViewType;

   /**
    *  Emitted with boost::any<SessionExplorer::ItemViewType> when the current
    *  tree view tab changes in the session explorer window.
    *
    *  @see     setItemViewType()
    */
   SIGNAL_METHOD(SessionExplorer, ItemViewTypeChanged)

   /**
    *  Emitted with boost::any<ContextMenu*> when the user right-clicks on a
    *  session item in the window to invoke a context menu.
    *
    *  This signal provides a means by which an object can be notified when a
    *  context menu is invoked by the user clicking on an item in the session
    *  explorer.  To receive notification for when a context menu is invoked
    *  when the user clicks on any session item, attach to the
    *  DesktopServices::signalAboutToShowContextMenu() signal instead.
    *
    *  This signal is emitted after the
    *  DesktopServices::signalAboutToShowContextMenu() signal is emitted and
    *  after the signalAboutToShowContextMenu() signal is emitted, but before
    *  the context menu is shown to give attached objects a chance to add or
    *  modify the context menu of the selected session item(s) that will be
    *  displayed to the user.
    *
    *  Objects that attach to the
    *  DesktopServices::signalAboutToShowContextMenu() signal may also need to
    *  attach to this signal if they are interested in the selected item(s) in
    *  the session explorer.
    *
    *  The ContextMenu pointer value is guaranteed to be non-\c NULL.
    *
    *  @see     \ref callingsequence "Context menu calling sequence"
    */
   SIGNAL_METHOD(SessionExplorer, AboutToShowSessionItemContextMenu)

   /**
    *  Sets the current tree view tab in the session explorer window.
    *
    *  @param   itemView
    *           The tree view type to display.
    *
    *  @notify  This method will notify signalItemViewTypeChanged() with
    *           boost::any<SessionExplorer::ItemViewType> if the current tab
    *           changes.
    */
   virtual void setItemViewType(ItemViewType itemView) = 0;

   /**
    *  Returns the current tree view tab in the session explorer window.
    *
    *  @return  The tree view type on the currently displayed tab.
    */
   virtual ItemViewType getItemViewType() const = 0;

   /**
    *  Sets the selected session items on the current view tab.
    *
    *  @param   selectedItems
    *           The session items to select.  If the vector is empty, all
    *           session items in the current view are unselected.
    */
   virtual void setSelectedSessionItems(const std::vector<SessionItem*>& selectedItems) = 0;

   /**
    *  Returns the selected session items on the current view tab.
    *
    *  This is a convenience method that calls getItemViewType() and then
    *  getSelectedSessionItems(ItemViewType).
    *
    *  @return  The selected session items on the current view.  If the
    *           returned vector is empty, no session items are selected.
    *
    *  @see     getCurrentSessionItem()
    */
   virtual std::vector<SessionItem*> getSelectedSessionItems() const = 0;

   /**
    *  Returns the selected session items of a specified type on the current
    *  view tab.
    *
    *  This is a convenience method that gets all selected session items on the
    *  current view and filters the returned items based on the specified type.
    *
    *  @return  The selected session items of the specified type on the current
    *           view.  If the returned vector is empty, no session items of the
    *           specified type are selected.  The pointers in the returned
    *           vector are guaranteed to be non-\b NULL.
    *
    *  @see     getCurrentSessionItem()
    */
   template<typename T>
   std::vector<T*> getSelectedSessionItems() const
   {
      std::vector<T*> selectedItems;

      std::vector<SessionItem*> items = getSelectedSessionItems();
      for (std::vector<SessionItem*>::iterator iter = items.begin(); iter != items.end(); ++iter)
      {
         T* pItem = model_cast<T*>(*iter);
         if (pItem != NULL)
         {
            selectedItems.push_back(pItem);
         }
      }

      return selectedItems;
   }

   /**
    *  Returns the current session item on the current view tab.
    *
    *  This is a convenience method that calls getItemViewType() and then
    *  getCurrentSessionItem(ItemViewType).
    *
    *  @return  The current session item on the current view.  \b NULL is
    *           returned if no session item is current in the tree view.
    *
    *  @see     getSelectedSessionItems()
    */
   virtual SessionItem* getCurrentSessionItem() const = 0;

   /**
    *  Sets the selected session items on a given view tab.
    *
    *  @param   itemView
    *           The view in which to select the session items.
    *  @param   selectedItems
    *           The session items to select.  If the vector is empty, all
    *           session items in the given view are unselected.
    */
   virtual void setSelectedSessionItems(ItemViewType itemView, const std::vector<SessionItem*>& selectedItems) = 0;

   /**
    *  Returns the selected session items on a given view tab.
    *
    *  This method returns the currently selected session items on the given
    *  view tab, which may be different than the current session item.
    *
    *  @param   itemView
    *           The view in which to get the selected session items.
    *
    *  @return  The selected session items.  If the returned vector is empty,
    *           no session items are selected on the given view tab.
    *
    *  @see     getCurrentSessionItem(ItemViewType)
    */
   virtual std::vector<SessionItem*> getSelectedSessionItems(ItemViewType itemView) const = 0;

   /**
    *  Returns the current session item on a given view tab.
    *
    *  This method returns the current session item on the given view tab,
    *  which may be different than the selected session items.  The current
    *  item is identified in the tree view by a focus rectangle.
    *
    *  @param   itemView
    *           The view in which to get the current session item.
    *
    *  @return  The current session item on the given view.  \b NULL is
    *           returned if no session item is current in the tree view.
    *
    *  @see     getSelectedSessionItems(ItemViewType)
    */
   virtual SessionItem* getCurrentSessionItem(ItemViewType itemView) const = 0;

   /**
    *  Expands a given session item on the current view tab.
    *
    *  This method expands the node in the tree view for the given session item
    *  to display its children.  This method does nothing if the session item
    *  is not displayed in the current tree view or if the session item has no
    *  child items.
    *
    *  @param   pItem
    *           The session item to expand in the view.
    *
    *  @see     collapseSessionItem()
    */
   virtual void expandSessionItem(SessionItem* pItem) = 0;

   /**
    *  Collapes a given session item on the current view tab.
    *
    *  This method collapses the node in the tree view for the given session
    *  item to hide its children.  This method does nothing if the session item
    *  is not displayed in the current tree view or if the session item has no
    *  child items.
    *
    *  @param   pItem
    *           The session item to collapse in the view.
    *
    *  @see     expandSessionItem()
    */
   virtual void collapseSessionItem(SessionItem* pItem) = 0;

   /**
    *  Queries whether a given session item is expanded on the current view
    *  tab.
    *
    *  This method collapses the node in the tree view for the given session
    *  item to hide its children.  This method does nothing if the session item
    *  is not displayed in the current tree view or if the session item has no
    *  child items.
    *
    *  @param   pItem
    *           The session item in the view to query for its expanded state.
    *
    *  @return  Returns \b true if the given session item is expanded in the
    *           current view and its children are displayed; otherwise returns
    *           \b false.
    *
    *  @see     expandSessionItem(), collapseSessionItem()
    */
   virtual bool isSessionItemExpanded(SessionItem* pItem) const = 0;

   /**
    *  @copydoc SessionItem::getContextMenuActions()
    *
    *  @default The default implementation returns the context menu actions
    *           listed \ref sessionexplorer "here".  If the user has clicked on
    *           a session item in the current tree view, the actions returned
    *           by this method are not displayed, but rather the actions for
    *           the selected session item(s) are displayed instead.  These
    *           default actions can be removed or additional actions can be
    *           added by attaching to the signalAboutToShowContextMenu()
    *           signal.  To override the default actions of the selected
    *           session item(s) attach to the
    *           signalAboutToShowSessionItemContextMenu() signal instead.
    */
   virtual std::list<ContextMenuAction> getContextMenuActions() const = 0;

protected:
   /**
    *  The session explorer cannot be destroyed directly.  It is destroyed
    *  automatically on application shutdown.
    */
   virtual ~SessionExplorer() {}
};

#endif
