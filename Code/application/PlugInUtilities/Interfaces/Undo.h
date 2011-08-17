/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef UNDO_H
#define UNDO_H

#include <string>

class View;

/**
 *  Starts and ends an undo group on a view.
 *
 *  This class provides a means to group multiple undo actions into a single
 *  undoable operation.  When the UndoGroup is created, undo actions are queued
 *  and are added to the view as a single action when the group is destroyed.
 *  Reference counting is performed so that nested groups can be created that
 *  will not add the actions to the view until the last group is destroyed.
 *
 *  @see        View::inUndoGroup()
 */
class UndoGroup
{
public:
   /**
    *  Starts the undo group.
    *
    *  The constructor creates the UndoGroup object and queues all undo actions
    *  to be added when the group is destroyed.
    *
    *  @param   pView
    *           The view in which to create the undo group.
    *  @param   text
    *           The name for the undo action group that will appear to the
    *           user.  This name will not appear to the user if this group is
    *           nested inside of another group.
    */
   UndoGroup(View* pView, const std::string& text);

   /**
    *  Ends the undo group.
    *
    *  The destructor destroys the UndoGroup object and adds the queued undo
    *  actions to the view as a single action.  The queued undo actions are
    *  not added to the view if another undo group already existed on the view
    *  when this group was created.
    *
    *  @see     View::inUndoGroup()
    */
   ~UndoGroup();

private:
   View* mpView;
   bool mEndGroupOnDestroy;
};

/**
 *  Blocks and unblocks undo actions from being added to a view.
 *
 *  This class provides a means to temporarily block undo actions from being
 *  added to a view.  Actions are blocked on the given view when creating the
 *  UndoLock object and unblocked when the lock is destroyed.  Reference
 *  counting is performed such that destroying the lock will not allow actions
 *  to be added to the view again if actions were blocked when the lock was
 *  created.
 *
 *  @see        View::isUndoBlocked()
 */
class UndoLock
{
public:
   /**
    *  Prevents undo actions from being added to a view.
    *
    *  The constructor creates the UndoLock object and blocks undo actions from
    *  being added to the given view.
    *
    *  @param   pView
    *           The view in which undo actions should be blocked.
    */
   UndoLock(View* pView);

   /**
    *  Allows undo actions to be added to a view.
    *
    *  The destructor destroys the UndoLock object and allow undo actions to be
    *  added to the view only if undo actions were not blocked when this object
    *  was created.
    *
    *  @see     View::isUndoBlocked()
    */
   ~UndoLock();

private:
   View* mpView;
   bool mUnlockOnDestroy;
};

#endif
