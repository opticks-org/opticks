/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CONTEXTMENUIMP_H
#define CONTEXTMENUIMP_H

#include <QtCore/QList>
#include <QtGui/QAction>
#include <QtGui/QMenu>

#include "ContextMenu.h"

#include <string>
#include <vector>

class SessionItem;
class Step;

class ContextMenuImp : public ContextMenu
{
public:
   ContextMenuImp(const std::vector<SessionItem*>& sessionItems, const QPoint& mouseLocation,
      const std::list<ContextMenuAction>& actions, QWidget* pParent, QObject* pActionParent = NULL);
   ~ContextMenuImp();

   using ContextMenu::getSessionItems;
   const std::vector<SessionItem*>& getSessionItems() const;
   const QPoint& getMouseLocation() const;
   QObject* getActionParent() const;

   bool addAction(const ContextMenuAction& menuAction);
   bool addAction(QAction* pAction, const std::string& id);
   bool addActionBefore(QAction* pAction, const std::string& id, const std::string& beforeId);
   bool addActionAfter(QAction* pAction, const std::string& id, const std::string& afterId);
   void removeAction(const std::string& id);
   const QList<ContextMenuAction>& getActions() const;
   QList<ContextMenuAction>& getActions();
   void clear();

   bool show();

protected:
   QAction* getAction(const std::string& id) const;
   void logActions(const std::string& stepName);

private:
   std::vector<SessionItem*> mSessionItems;
   QPoint mMouseLocation;
   QList<ContextMenuAction> mActions;
   QMenu* mpMenu;
   QObject* mpActionParent;
   Step* mpStep;
};

#endif
