/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef TOOLBARIMP_H
#define TOOLBARIMP_H

#include <QtGui/QAction>
#include <QtGui/QToolBar>

#include "WindowImp.h"
#include "TypesFile.h"

#include <string>
#include <vector>

class MenuBar;
class MenuBarImp;

class ToolBarImp : public QToolBar, public WindowImp
{
   Q_OBJECT

public:
   ToolBarImp(const std::string& id, const std::string& name, QWidget* parent = 0);
   ~ToolBarImp();

   using SessionItemImp::setIcon;

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   WindowType getWindowType() const;
   using WindowImp::setName;

   MenuBar* getMenuBar() const;
   void addButton(QAction* pAction, QAction* pBefore = NULL);
   void addButton(QAction* pAction, const std::string& shortcutContext, QAction* pBefore = NULL);
   QAction* insertWidget(QWidget* pWidget, QAction* pBefore = NULL);
   using QToolBar::insertWidget;
   QAction* addSeparator(QAction* pBefore = NULL);
   std::vector<QAction*> getItems() const;
   void removeItem(QAction* pAction);

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

signals:
   void visibilityChanged(bool bVisible);

protected:
   void showEvent(QShowEvent* pEvent);
   void hideEvent(QHideEvent* pEvent);

private:
   MenuBarImp* mpMenuBar;

   WINDOWIMPDROP_METHODS(WindowImp);
};

#define TOOLBARADAPTER_METHODS(impClass) \
   WINDOWADAPTER_METHODS(impClass) \
   MenuBar* getMenuBar() const \
   { \
      return impClass::getMenuBar(); \
   } \
   void addButton(QAction* pAction, QAction* pBefore = NULL) \
   { \
      return impClass::addButton(pAction, pBefore); \
   } \
   void addButton(QAction* pAction, const std::string& shortcutContext, QAction* pBefore = NULL) \
   { \
      return impClass::addButton(pAction, shortcutContext, pBefore); \
   } \
   QAction* insertWidget(QWidget* pWidget, QAction* pBefore = NULL) \
   { \
      return impClass::insertWidget(pWidget, pBefore); \
   } \
   QAction* addSeparator(QAction* pBefore = NULL) \
   { \
      return impClass::addSeparator(pBefore); \
   } \
   std::vector<QAction*> getItems() const \
   { \
      return impClass::getItems(); \
   } \
   void removeItem(QAction* pAction) \
   { \
      return impClass::removeItem(pAction); \
   }

#endif