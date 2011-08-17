/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef UNDOBUTTON_H
#define UNDOBUTTON_H

#include <QtGui/QListView>
#include <QtGui/QMenu>
#include <QtGui/QStringListModel>
#include <QtGui/QToolButton>
#include <QtGui/QUndoGroup>

class UndoButton : public QToolButton
{
   Q_OBJECT

public:
   UndoButton(bool bUndo, QUndoGroup* pGroup, QWidget* pParent = NULL);
   ~UndoButton();

protected slots:
   void updateModel();
   void executeUndo(const QModelIndex& modelIndex);

private:
   class MenuListView : public QListView
   {
   public:
      MenuListView(QWidget* pParent = NULL) : QListView(pParent) {}
      QSize sizeHint() const;
   };

   bool mUndo;
   QUndoGroup* mpUndoGroup;
   QStringListModel* mpModel;
   MenuListView* mpListView;
   QMenu* mpMenu;
};

#endif
