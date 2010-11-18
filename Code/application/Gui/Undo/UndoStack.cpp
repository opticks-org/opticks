/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "UndoAction.h"
#include "UndoStack.h"

using namespace std;

UndoStack::UndoStack(QObject* pParent) :
   QUndoStack(pParent)
{
}

UndoStack::~UndoStack()
{
}

// Override a non-virtual method so that the action is added to the member list
void UndoStack::push(QUndoCommand* pAction)
{
   if (pAction != NULL)
   {
      QObject* pObject = dynamic_cast<QObject*>(pAction);
      if (pObject != NULL)
      {
         mActions.append(pObject);
      }

      QUndoStack::push(pAction);

      UndoAction* pUndoAction = dynamic_cast<UndoAction*>(pAction);
      if (pUndoAction != NULL)
      {
         VERIFYNR(connect(pUndoAction, SIGNAL(sessionItemChanged(const std::string&, const std::string&)), this,
            SLOT(updateActions(const std::string&, const std::string&))));
         VERIFYNR(connect(pUndoAction, SIGNAL(destroyed(QObject*)), this, SLOT(removeAction(QObject*))));
      }
   }
}

void UndoStack::updateActions(const string& oldId, const string& newId)
{
   for (int i = 0; i < mActions.count(); ++i)
   {
      UndoAction* pAction = dynamic_cast<UndoAction*>(mActions[i]);
      if (pAction != NULL)
      {
         pAction->updateSessionItem(oldId, newId);
      }
   }

   emit sessionItemChanged(oldId, newId);
}

void UndoStack::removeAction(QObject* pAction)
{
   if (pAction != NULL)
   {
      int index = mActions.indexOf(pAction);
      if (index != -1)
      {
         mActions.removeAt(index);
      }
   }
}
