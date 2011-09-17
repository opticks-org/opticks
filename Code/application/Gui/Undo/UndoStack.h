/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef UNDOSTACK_H
#define UNDOSTACK_H

#include <QtCore/QList>
#include <QtGui/QUndoCommand>
#include <QtGui/QUndoStack>

#include <string>

class UndoStack : public QUndoStack
{
   Q_OBJECT

public:
   UndoStack(QObject* pParent = NULL);
   ~UndoStack();

   void push(QUndoCommand* pAction);

public slots:
   void updateActions(const std::string& oldId, const std::string& newId);

signals:
   void sessionItemChanged(const std::string& oldId, const std::string& newId);

protected slots:
   void removeAction(QObject* pAction);

private:
   QList<QObject*> mActions;
};

#endif
