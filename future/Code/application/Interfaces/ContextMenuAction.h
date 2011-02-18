/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CONTEXTMENUACTION_H
#define CONTEXTMENUACTION_H

#include "EnumWrapper.h"

#include <string>

class QAction;

/**
 *  An action to be used on a context menu.
 *
 *  @see     \ref contextmenus "Customizing context menus"
 */
struct ContextMenuAction
{
   /**
    *  Defines how the associated buddy action is used when adding this action
    *  to the context menu.
    */
   enum BuddyTypeEnum
   {
      BEFORE,  /**< This action should appear above the buddy action. */
      AFTER,   /**< This action should appear below the buddy action. */
      REMOVE   /**< The buddy action should be removed from the context menu. */
   };

   /**
    *  @EnumWrapper ::BuddyTypeEnum.
    */
   typedef EnumWrapper<BuddyTypeEnum> BuddyType;

   /**
    *  Creates the context menu action.
    *
    *  @param   pAction
    *           The Qt action to add to the context menu.  If \b NULL is passed
    *           in, this action can only be used to remove an action from the
    *           menu.
    *  @param   id
    *           The unique identifier for this action.  If an empty string is
    *           passed in, this action can only be used to remove an action
    *           from the menu.
    */
   ContextMenuAction(QAction* pAction, const std::string& id) :
      mpAction(pAction),
      mId(id)
   {
   }

   /**
    *  Assigns the values in this action to those of another action.
    *
    *  @param   menuAction
    *           The source action from which to assign this action's values.
    *
    *  @return  A reference to *this, which has been changed to have the same
    *           values as \em menuAction.
    */
   ContextMenuAction& operator=(const ContextMenuAction& menuAction)
   {
      if (this != &menuAction)
      {
         mpAction = menuAction.mpAction;
         mId = menuAction.mId;
         mBuddyType = menuAction.mBuddyType;
         mBuddyId = menuAction.mBuddyId;
      }

      return *this;
   }

   /**
    *  Compares two actions.
    *
    *  @param   menuAction
    *           The action to compare against this action.
    *
    *  @return  Returns \b true if all values in this action are equal to those
    *           of \em menuAction.
    */
   bool operator==(const ContextMenuAction& menuAction) const
   {
      if ((menuAction.mpAction == mpAction) && (menuAction.mId == mId) &&
         (menuAction.mBuddyType == mBuddyType) && (menuAction.mBuddyId == mBuddyId))
      {
         return true;
      }

      return false;
   }

   /**
    *  The Qt action that should appear on the context menu.
    *
    *  If this value is \b NULL, this ContextMenuAction can only be used to
    *  remove an action from the menu.
    *
    *  @see     mBuddyId
    */
   QAction* mpAction;

   /**
    *  The unique identifier for the Qt action that should appear on the
    *  context menu.
    *
    *  If this string is empty, this ContextMenuAction can only be used to
    *  remove an action from the menu.
    *
    *  @see     mBuddyId
    */
   std::string mId;

   /**
    *  Defines how the buddy action is used.
    *
    *  @see     BuddyType
    */
   BuddyType mBuddyType;

   /**
    *  The unique identifier for the associated buddy action.
    *
    *  The buddy action is used as a reference when placing this action on the
    *  menu.  It can also be used as an action which should be removed from the
    *  menu.
    *
    *  @see     mBuddyType
    */
   std::string mBuddyId;
};

#endif
