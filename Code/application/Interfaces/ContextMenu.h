/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CONTEXTMENU_H
#define CONTEXTMENU_H

#include "ConfigurationSettings.h"
#include "ContextMenuAction.h"
#include "ModelServices.h"

#include <string>
#include <vector>

class QAction;
class QObject;
class QPoint;
class SessionItem;

/**
 *  Provides access to customize a context menu for one or more session items.
 *
 *  A ContextMenu is created by objects that display a context menu to the user
 *  for one or more session items.  An object usually creates the menu before
 *  it emits a signal that allows attached object to add actions to the menu or
 *  to remove one or more of the default actions.
 *
 *  Actions can be added to or removed from the menu using ContextMenuAction
 *  objects, which gives all objects that need to modify the actions a chance
 *  to add or remove actions.  The actual list of actions that are displayed to
 *  the user is not created until the menu is shown.
 *
 *  @see     \ref contextmenus "Customizing context menus"
 */
class ContextMenu
{
public:
   SETTING(LogActions, ContextMenu, bool, false)

   /**
    *  Returns the session items for which the context menu will display
    *  actions.
    *
    *  @return  The session items for which the context menu has been
    *           requested.
    */
   virtual const std::vector<SessionItem*>& getSessionItems() const = 0;

   /**
    *  Returns only the session items of a specified type for which the context
    *  menu will display actions.
    *
    *  This is a convenience method that gets all session items for which the
    *  context menu has been requested filters the returned items based on the
    *  specified type.
    *
    *  @return  The session items of the specified type for which the context
    *           menu has been requested.  If the returned vector is empty, no
    *           session items of the specified type are included in those items
    *           for which the context menu is requested.  The pointers in the
    *           returned vector are guaranteed to be non-\b NULL.
    *
    *  @see     getSessionItems()
    */
   template<typename T>
   std::vector<T*> getSessionItems() const
   {
      std::vector<T*> items;

      const std::vector<SessionItem*>& allItems = getSessionItems();
      for (std::vector<SessionItem*>::const_iterator iter = allItems.begin(); iter != allItems.end(); ++iter)
      {
         T* pItem = model_cast<T*>(*iter);
         if (pItem != NULL)
         {
            items.push_back(pItem);
         }
      }

      return items;
   }

   /**
    *  Returns the context menu mouse click location.
    *
    *  @return  The mouse click location for the context menu in global
    *           coordinates.
    */
   virtual const QPoint& getMouseLocation() const = 0;

   /**
    *  Returns a pointer to an object that can be used as a parent for a menu
    *  action.
    *
    *  This method provides a QObject that can optionally be used as the parent
    *  for a menu action.  By using this object as a parent, the action will
    *  automatically be deleted when the context menu itself is deleted.
    *
    *  @return  A pointer to the object that can be used as a parent object for
    *           menu actions.
    *
    *  @warning Actions created with the returned object as a parent should not
    *           be stored, since the action will automatically be deleted when
    *           the context menu is deleted.
    */
   virtual QObject* getActionParent() const = 0;

   /**
    *  Adds an action to a context menu.
    *
    *  @param   menuAction
    *           The action to add to the context menu.
    *
    *  @return  Returns \b true if the action was successfully added to the
    *           menu.  Returns \b false if the given action has already been
    *           added to the menu.
    *
    *  @see     ContextMenuAction, addAction(QAction*, const std::string&),
    *           addActionBefore(), addActionAfter(), removeAction()
    */
   virtual bool addAction(const ContextMenuAction& menuAction) = 0;

   /**
    *  Adds an action to a context menu.
    *
    *  This is a convenience method that creates a ContextMenuAction to append
    *  the given action to the list of actions that will be displayed in the
    *  context menu and calls addAction(const ContextMenuAction&).
    *
    *  @param   pAction
    *           The action to add to the context menu.
    *  @param   id
    *           The unique identifier for the action.
    *
    *  @return  Returns \b true if the action was successfully added to the
    *           menu.  Returns \b false if the given action has already been
    *           added to the menu, if \em pAction is \b NULL, or if \em id is
    *           empty.
    *
    *  @see     addActionBefore(), addActionAfter(), removeAction()
    */
   virtual bool addAction(QAction* pAction, const std::string& id) = 0;

   /**
    *  Inserts an action into a context menu before another action.
    *
    *  This is a convenience method that creates a ContextMenuAction to add
    *  the given action to the list of actions that will be displayed in the
    *  context menu, sets the placement of the action to appear before
    *  the given action, and calls addAction(const ContextMenuAction&).
    *
    *  @param   pAction
    *           The action to add to the context menu.
    *  @param   id
    *           The unique identifier for the action to add.
    *  @param   beforeId
    *           The unique identifier for the action that should appear below
    *           the added action.  If \em beforeId is empty, this method is
    *           identical to addAction(QAction*, const std::string&).
    *
    *  @return  Returns \b true if the action was successfully added to the
    *           menu.  Returns \b false if the given action has already been
    *           added to the menu, if \em pAction is \b NULL, or if \em id is
    *           empty.
    *
    *  @see     addActionAfter(), removeAction()
    */
   virtual bool addActionBefore(QAction* pAction, const std::string& id, const std::string& beforeId) = 0;

   /**
    *  Inserts an action into a context menu after another action.
    *
    *  This is a convenience method that creates a ContextMenuAction to add
    *  the given action to the list of actions that will be displayed in the
    *  context menu, sets the placement of the action to appear after
    *  the given action, and calls addAction(const ContextMenuAction&).
    *
    *  @param   pAction
    *           The action to add to the context menu.
    *  @param   id
    *           The unique identifier for the action to add.
    *  @param   afterId
    *           The unique identifier for the action that should appear above
    *           the added action.  If \em afterId is empty, this method is
    *           identical to addAction(QAction*, const std::string&).
    *
    *  @return  Returns \b true if the action was successfully added to the
    *           menu.  Returns \b false if the given action has already been
    *           added to the menu, if \em pAction is \b NULL, or if \em id is
    *           empty.
    *
    *  @see     addActionBefore(), removeAction()
    */
   virtual bool addActionAfter(QAction* pAction, const std::string& id, const std::string& afterId) = 0;

   /**
    *  Removes an action from the context menu.
    *
    *  This method registers the given action to be removed from the context
    *  menu.  The action is not removed until all slot methods that are
    *  connected to the about-to-show context menu signal are called.  This
    *  allows for correct placement of all actions in the menu before any
    *  actions are removed.
    *
    *  @param   id
    *           The unique identifier for the action to remove from the menu.
    *
    *  @see     addAction(const ContextMenuAction&)
    */
   virtual void removeAction(const std::string& id) = 0;

protected:
   /**
    *  This object does not need to be destroyed directly.  It will be deleted
    *  by the object that creates and displays the context menu.
    */
   virtual ~ContextMenu() {}
};

#endif
