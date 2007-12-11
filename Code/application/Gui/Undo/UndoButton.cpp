/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QUndoStack>
#include <QtGui/QWidgetAction>

#include "AppVerify.h"
#include "Icons.h"
#include "UndoButton.h"

UndoButton::UndoButton(bool bUndo, QUndoGroup* pGroup, QWidget* pParent) :
   QToolButton(pParent),
   mUndo(bUndo),
   mpUndoGroup(pGroup),
   mpModel(NULL),
   mpListView(NULL),
   mpMenu(NULL)
{
   mpModel = new QStringListModel(this);

   mpListView = new MenuListView(this);
   mpListView->setModel(mpModel);

   QWidgetAction* pWidgetAction = new QWidgetAction(this);
   pWidgetAction->setDefaultWidget(mpListView);

   mpMenu = new QMenu(this);
   mpMenu->addAction(pWidgetAction);

   // Initialization
   QAction* pAction = NULL;
   if (mpUndoGroup != NULL)
   {
      Icons* pIcons = Icons::instance();

      if (mUndo == true)
      {
         pAction = mpUndoGroup->createUndoAction(this);
         pAction->setIcon(pIcons->mUndo);
      }
      else
      {
         pAction = mpUndoGroup->createRedoAction(this);
         pAction->setIcon(pIcons->mRedo);
      }
   }

   setDefaultAction(pAction);
   setMenu(mpMenu);
   setPopupMode(QToolButton::MenuButtonPopup);

   // Connections
   VERIFYNR(connect(mpMenu, SIGNAL(aboutToShow()), this, SLOT(updateModel())));
   VERIFYNR(connect(mpListView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(executeUndo(const QModelIndex&))));
}

UndoButton::~UndoButton()
{
}

void UndoButton::updateModel()
{
   if (mpUndoGroup == NULL)
   {
      return;
   }

   QUndoStack* pStack = mpUndoGroup->activeStack();
   if (pStack != NULL)
   {
      int index = pStack->index();

      QStringList actionList;
      if (mUndo == true)
      {
         for (int i = index - 1; i > -1; --i)
         {
            QString actionText = pStack->text(i);
            if (actionText.isEmpty() == false)
            {
               actionList.append(actionText);
            }
         }
      }
      else
      {
         for (int i = index; i < pStack->count(); ++i)
         {
            QString actionText = pStack->text(i);
            if (actionText.isEmpty() == false)
            {
               actionList.append(actionText);
            }
         }
      }

      mpModel->setStringList(actionList);
   }

   mpListView->scrollToTop();
}

void UndoButton::executeUndo(const QModelIndex& modelIndex)
{
   if ((modelIndex.isValid() == true) && (mpUndoGroup != NULL))
   {
      QUndoStack* pStack = mpUndoGroup->activeStack();
      if (pStack != NULL)
      {
         int currentIndex = pStack->index();
         int listIndex = modelIndex.row() + 1;

         if (mUndo == true)
         {
            pStack->setIndex(currentIndex - listIndex);
         }
         else
         {
            pStack->setIndex(currentIndex + listIndex);
         }
      }
   }

   mpMenu->close();
}

QSize UndoButton::MenuListView::sizeHint() const
{
   int iWidth = sizeHintForColumn(0) + 25;   // Add 25 to provide room for the scroll bar
   int iHeight = 150;

   return QSize(iWidth, iHeight);
}
