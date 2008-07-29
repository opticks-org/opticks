/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MENUBARIMP_H
#define MENUBARIMP_H

#include <QtCore/QList>
#include <QtGui/QAction>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>

#include "MenuBar.h"

class MenuBarImp : public QMenuBar, public MenuBar
{
   Q_OBJECT

public:
   MenuBarImp(const QString& strMenuBarName, QWidget* parent = 0);
   ~MenuBarImp();

   QMenu* addMenu(const std::string& location, QAction* pBefore = NULL);
   QAction* insertMenu(QMenu* pMenu, QMenu* pParentMenu = NULL, QAction* pBefore = NULL);
   QAction* addCommand(const std::string& location, QAction* pBefore = NULL);
   QAction* addCommand(const std::string& location, const std::string& shortcutContext, QAction* pBefore = NULL);
   bool insertCommand(QAction* pCommand, QMenu* pMenu, const std::string& shortcutContext, QAction* pBefore = NULL);

   QAction* getMenuItem(const std::string& location) const;
   QAction* getMenuItem(QMenu* pMenu) const;
   QMenu* getMenu(QAction* pAction) const;
   bool isMenu(QAction* pAction) const;
   bool isCommand(QAction* pAction) const;

   bool removeMenu(QMenu* pMenu);
   bool removeMenuItem(QAction* pAction);

   QList<QAction*> getAllActions() const;
   QList<QAction*> getMenuActions() const;
   QList<QAction*> getCommandActions() const;

protected:
   QAction* getAction(const QString& strMenuItem, QMenu* pMenu) const;

private:
   QString mName;
   QList<QMenu*> mMenus;
};

#endif
