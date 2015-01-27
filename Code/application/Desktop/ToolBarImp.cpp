/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QApplication>
#include <QtGui/QKeyEvent>
#include <QtGui/QLayout>

#include "ToolBarImp.h"
#include "AppVerify.h"
#include "ContextMenuAction.h"
#include "ContextMenuActions.h"
#include "DesktopServicesImp.h"
#include "MenuBarImp.h"
#include "ToolBar.h"

XERCES_CPP_NAMESPACE_USE
using namespace std;

ToolBarImp::ToolBarImp(const string& id, const string& name, QWidget* parent) :
   QToolBar(QString::fromStdString(name), parent),
   WindowImp(id, name),
   mpMenuBar(new MenuBarImp(QString::fromStdString(name), this)),
   mMinToolBarWidth(0)
{
   mpShowAction = new QAction("&Show", this);
   mpShowAction->setAutoRepeat(false);
   mpShowAction->setStatusTip("Shows the window");

   mpHideAction = new QAction("&Hide", this);
   mpHideAction->setAutoRepeat(false);
   mpHideAction->setStatusTip("Hides the window");

   // Initialization
   setIconSize(QSize(16, 16));
   addWidget(mpMenuBar);
   setContextMenuPolicy(Qt::DefaultContextMenu);

   VERIFYNR(connect(mpShowAction, SIGNAL(triggered()), this, SLOT(show())));
   VERIFYNR(connect(mpHideAction, SIGNAL(triggered()), this, SLOT(hide())));
   VERIFYNR(connect(this, SIGNAL(topLevelChanged(bool)), this, SLOT(floatToolBar(bool))));

   // Prevent the menu bar from stretching
   QLayout* pLayout = layout();
   if (pLayout != NULL)
   {
      pLayout->setAlignment(mpMenuBar, Qt::AlignLeft | Qt::AlignVCenter);
   }
}

ToolBarImp::~ToolBarImp()
{
}

const string& ToolBarImp::getObjectType() const
{
   static string type("ToolBarImp");
   return type;
}

bool ToolBarImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "ToolBar"))
   {
      return true;
   }

   return WindowImp::isKindOf(className);
}

WindowType ToolBarImp::getWindowType() const
{
   return TOOLBAR;
}

MenuBar* ToolBarImp::getMenuBar() const
{
   return mpMenuBar;
}

void ToolBarImp::addButton(QAction* pAction, QAction* pBefore)
{
   if (pAction == NULL)
   {
      return;
   }

   if (pBefore != NULL)
   {
      insertAction(pBefore, pAction);
   }
   else
   {
      addAction(pAction);
   }

   notify(SIGNAL_NAME(Subject, Modified));
}

void ToolBarImp::addButton(QAction* pAction, const string& shortcutContext, QAction* pBefore)
{
   if (pAction == NULL)
   {
      return;
   }

   // Initialize the keyboard shortcut in the action
   DesktopServicesImp* pDesktop = DesktopServicesImp::instance();
   VERIFYNRV(pDesktop != NULL);

   pDesktop->initializeAction(pAction, shortcutContext);

   // Add the button to the toolbar
   addButton(pAction, pBefore);
}

QAction* ToolBarImp::insertWidget(QWidget* pWidget, QAction* pBefore)
{
   if (pWidget == NULL)
   {
      return NULL;
   }

   QAction* pAction = NULL;
   if (pBefore != NULL)
   {
      pAction = insertWidget(pBefore, pWidget);
   }
   else
   {
      pAction = addWidget(pWidget);
   }

   if (pAction != NULL)
   {
      notify(SIGNAL_NAME(Subject, Modified));
   }

   return pAction;
}

QAction* ToolBarImp::addSeparator(QAction* pBefore)
{
   QAction* pAction = NULL;
   if (pBefore != NULL)
   {
      pAction = insertSeparator(pBefore);
   }
   else
   {
      pAction = QToolBar::addSeparator();
   }

   if (pAction != NULL)
   {
      notify(SIGNAL_NAME(Subject, Modified));
   }

   return pAction;
}

vector<QAction*> ToolBarImp::getItems() const
{
   QList<QAction*> actionList = actions();
   QVector<QAction*> actionVector = actionList.toVector();
   vector<QAction*> items = actionVector.toStdVector();

   return items;
}

void ToolBarImp::removeItem(QAction* pAction)
{
   if (pAction != NULL)
   {
      removeAction(pAction);
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

void ToolBarImp::showEvent(QShowEvent* pEvent)
{
   QToolBar::showEvent(pEvent);
   emit visibilityChanged(true);
   notify(SIGNAL_NAME(ToolBar, Shown));
}

void ToolBarImp::hideEvent(QHideEvent* pEvent)
{
   QToolBar::hideEvent(pEvent);
   emit visibilityChanged(false);
   notify(SIGNAL_NAME(ToolBar, Hidden));
}

list<ContextMenuAction> ToolBarImp::getContextMenuActions() const
{
   list<ContextMenuAction> menuActions = WindowImp::getContextMenuActions();
   if (isVisible() == true)
   {
      menuActions.push_back(ContextMenuAction(mpHideAction, APP_TOOLBAR_HIDE_ACTION));
   }
   else
   {
      menuActions.push_back(ContextMenuAction(mpShowAction, APP_TOOLBAR_SHOW_ACTION));
   }

   return menuActions;
}

bool ToolBarImp::isShown() const
{
   return isVisible();
}

void ToolBarImp::floatToolBar(bool floated)
{
   if (floated)
   {
      mMinToolBarWidth = this->minimumWidth();
      if (QApplication::keyboardModifiers() == Qt::ControlModifier)
      {
         // This allows for toolbars to be wider when floated.  This
         // capability is very useful when toolbars contain sliders
         // (ie: Animation Toolbar, Brightness Toolbar) and user wants
         // the toolbar floated, but doesn't want to give up slider
         // fidelity.
         int width = this->geometry().width();
         this->setMinimumWidth( width * 0.9);
      }
   }
   else
   {
      if (this->minimumWidth() != mMinToolBarWidth)
      {
         this->setMinimumWidth(mMinToolBarWidth);
      }
   }
}

void ToolBarImp::moveEvent(QMoveEvent* pEvent)
{
   if (QApplication::keyboardModifiers() != Qt::ControlModifier)
   {
      if (this->minimumWidth() != mMinToolBarWidth)
      {
         this->setMinimumWidth(mMinToolBarWidth);
      }
   }
   QToolBar::moveEvent(pEvent);
}
