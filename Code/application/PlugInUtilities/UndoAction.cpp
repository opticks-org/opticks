/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QApplication>

#include "SessionItem.h"
#include "SessionManager.h"
#include "UndoAction.h"

using namespace std;

UndoAction::UndoAction(bool bRedoOnAdd) :
   QUndoCommand(NULL),
   mRedo(bRedoOnAdd)
{
}

UndoAction::UndoAction(SessionItem* pItem, bool bRedoOnAdd) :
   QUndoCommand(NULL),
   mRedo(bRedoOnAdd)
{
   setSessionItem(pItem);
}

UndoAction::~UndoAction()
{
}

void UndoAction::setSessionItem(SessionItem* pItem)
{
   mSessionItemId.clear();
   if (pItem != NULL)
   {
      mSessionItemId = pItem->getId();
   }
}

SessionItem* UndoAction::getSessionItem() const
{
   if (mSessionItemId.empty() == false)
   {
      Service<SessionManager> pManager;
      return pManager->getSessionItem(mSessionItemId);
   }

   return NULL;
}

const string& UndoAction::getSessionItemId() const
{
   return mSessionItemId;
}

void UndoAction::updateSessionItem(const string& oldId, const string& newId)
{
   if ((oldId.empty() == false) && (oldId == mSessionItemId))
   {
      mSessionItemId = newId;
   }
}

void UndoAction::undo()
{
   emit aboutToUndo();
   QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
   executeUndo();
   QApplication::restoreOverrideCursor();
   emit undoComplete();
}

void UndoAction::redo()
{
   if (mRedo == true)
   {
      emit aboutToRedo();
      QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
      executeRedo();
      QApplication::restoreOverrideCursor();
      emit redoComplete();
   }
   else
   {
      mRedo = true;
   }
}
