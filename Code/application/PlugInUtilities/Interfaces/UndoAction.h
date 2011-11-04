/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef UNDOACTION_H
#define UNDOACTION_H

#include <QtCore/QObject>
#include <QtGui/QUndoCommand>

class SessionItem;

/**
 *  Base class for all undo actions added to the undo stack in a view.
 *
 *  This class can be subclassed to create custom undo actions that will be
 *  executed when the user clicks the Undo or Redo toolbar buttons.  Subclasses
 *  must override the executeUndo() and executeRedo() methods to perform the
 *  undo or redo.  The inherited QUndoCommand::undo() and QUndoCommand::redo()
 *  method obligations are implemented in this class to emit signals before and
 *  after the undo or redo is performed and to change the mouse cursor to a
 *  wait cursor while the undo or redo is performed.
 *
 *  Since undo actions typically perform operations on a SessionItem, an
 *  UndoAction optionally stores a session item by its ID and provides the
 *  getSessionItem() accessor method as a convenience to retrieve the
 *  SessionItem pointer when performing the undo or redo.  Since the stored
 *  session item may be deleted when performing the undo or redo, the
 *  sessionItemChanged() signal is provided to notify the application that the
 *  session item ID of other actions may need to be updated.
 */
class UndoAction : public QObject, public QUndoCommand
{
   Q_OBJECT

public:
   /**
    *  Creates the undo action.
    *
    *  @param   bRedoOnAdd
    *           By default the redo() method is called when the action is added
    *           to an undo stack in a view.  If this value is \c false the
    *           executeRedo() method is not called when the action is added to
    *           the undo stack.
    */
   UndoAction(bool bRedoOnAdd = false);

   /**
    *  Creates the undo action.
    *
    *  @param   pItem
    *           The session item to store by its ID.
    *  @param   bRedoOnAdd
    *           By default the redo() method is called when the action is added
    *           to an undo stack in a view.  If this value is \c false the
    *           executeRedo() method is not called when the action is added to
    *           the undo stack.
    *
    *  @see     setSessionItem()
    */
   UndoAction(SessionItem* pItem, bool bRedoOnAdd = false);

   /**
    *  Destroys the undo action.
    */
   virtual ~UndoAction();

   /**
    *  Stores the ID for a given session item.
    *
    *  @param   pItem
    *           The session item for which to store its ID.
    *
    *  @see     SessionItem::getId()
    */
   void setSessionItem(SessionItem* pItem);

   /**
    *  Returns a SessionItem pointer for the stored ID.
    *
    *  This method retrieves the session item for the stored ID value by
    *  calling SessionManager::getSessionItem().
    *
    *  @warning The session manager does not currently store graphic objects or
    *           the graphic layers contained in a product view.  Subclasses
    *           that need to access these objects should additionally store the
    *           ID for the parent graphic layer in a spatial data view or the
    *           ID for the product view and the graphic layer contained in the
    *           product view.  This method should be overridden to obtain the
    *           parent session item from the session manager and used to access
    *           the desired session item not contained in the session manager.
    *
    *  @return  The SessionItem pointer for the session item with the stored
    *           ID.  Returns \c NULL if no session item can be found in the
    *           session manager with the stored ID.
    *
    *  @see     getSessionItemId()
    */
   virtual SessionItem* getSessionItem() const;

   /**
    *  Returns a stored session item ID.
    *
    *  @return  The ID for the session item that was passed into the
    *           constructor or in setSessionItem().
    *
    *  @see     getSessionItem()
    */
   const std::string& getSessionItemId() const;

   /**
    *  Updates the stored session item ID.
    *
    *  This method is called by the application when the sessionItemChanged()
    *  signal is emitted by another undo action.  This is typically emitted
    *  when another action recreates a session item during undo or redo
    *  execution.
    *
    *  This implementation compares the old ID with the stored member and
    *  updates the member to the new ID if the old ID matches the stored
    *  member.  This method should be overridden in a subclass if the subclass
    *  needs to store an additional session item ID.
    *
    *  @param   oldId
    *           The previous ID for the session item that was deleted.
    *  @param   newId
    *           The ID for the recreated session item.
    */
   virtual void updateSessionItem(const std::string& oldId, const std::string& newId);

   /**
    *  Performs the undo to restore the application to its previous state.
    *
    *  This method is called by the application when the user clicks the Undo
    *  button on the toolbar.  It provides a default implementation of the
    *  virtual QUndoCommand::undo() method.  It emits the aboutToUndo() signal,
    *  changes the mouse cursor to a wait cursor, calls executeUndo(), restores
    *  the mouse cursor, and emits the undoComplete() signal.
    */
   void undo();

   /**
    *  Performs the redo to return the application to its modified state.
    *
    *  This method is called by the application when the user clicks the Redo
    *  button on the toolbar.  It provides a default implementation of the
    *  virtual QUndoCommand::redo() method.  It emits the aboutToRedo() signal,
    *  changes the mouse cursor to a wait cursor, calls executeRedo(), restores
    *  the mouse cursor, and emits the redoComplete() signal.
    *
    *  If this method is called in response to the undo action being added to
    *  the undo stack and the \em bRedoOnAdd parameter passed into the
    *  constructor is \c false, this method does nothing.
    */
   void redo();

signals:
   /**
    *  Emitted just before the undo is performed.
    *
    *  This signal is emitted in the default implementation of undo() before
    *  the executeUndo() method is called.
    *
    *  @see     undoComplete()
    */
   void aboutToUndo();

   /**
    *  Emitted after the undo is performed.
    *
    *  This signal is emitted in the default implementation of undo() after
    *  the executeUndo() method is called.
    *
    *  @see     aboutToUndo()
    */
   void undoComplete();

   /**
    *  Emitted just before the redo is performed.
    *
    *  This signal is emitted in the default implementation of redo() before
    *  the executeRedo() method is called.
    *
    *  @see     redoComplete()
    */
   void aboutToRedo();

   /**
    *  Emitted after the redo is performed.
    *
    *  This signal is emitted in the default implementation of redo() after
    *  the executeRedo() method is called.
    *
    *  @see     aboutToRedo()
    */
   void redoComplete();

   /**
    *  Emitted when a session item is deleted and recreated, thereby changing
    *  its ID.
    *
    *  This signal is emitted in subclasses that destroy and recreate session
    *  items during undo and redo execution.  After destroying the session item
    *  the action should maintain the old ID string to use when emitting this
    *  signal after recreating the session item.
    *
    *  @param   oldId
    *           The ID of the deleted session item.
    *  @param   newId
    *           The ID of the recreated session item, obtained with
    *           SessionItem::getId().
    *
    *  @see     updateSessionItem()
    */
   void sessionItemChanged(const std::string& oldId, const std::string& newId);

protected:
   /**
    *  Performs the undo to restore the application to its previous state.
    *
    *  This method must be overridden in subclasses to perform specific changes
    *  to the application.  It is called from the default implementation of the
    *  undo() method.
    */
   virtual void executeUndo() = 0;

   /**
    *  Performs the redo to return the application to its modified state.
    *
    *  This method must be overridden in subclasses to perform specific changes
    *  to the application.  It is called from the default implementation of the
    *  redo() method.
    */
   virtual void executeRedo() = 0;

private:
   UndoAction(const UndoAction& rhs);
   UndoAction& operator=(const UndoAction& rhs);
   std::string mSessionItemId;
   bool mRedo;
};

#endif
