/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QLayout>

#include "ToolBarImp.h"
#include "AppVerify.h"
#include "DesktopServicesImp.h"
#include "MenuBarImp.h"

XERCES_CPP_NAMESPACE_USE
using namespace std;

ToolBarImp::ToolBarImp(const string& id, const string& name, QWidget* parent) :
   QToolBar(QString::fromStdString(name), parent),
   WindowImp(id, name),
   mpMenuBar(new MenuBarImp(QString::fromStdString(name), this))
{
   // Initialization
   setIconSize(QSize(16, 16));
   addWidget(mpMenuBar);

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
}

void ToolBarImp::hideEvent(QHideEvent* pEvent)
{
   QToolBar::hideEvent(pEvent);
   emit visibilityChanged(false);
}


bool ToolBarImp::toXml(XMLWriter* pXml) const
{
   if (!WindowImp::toXml(pXml))
   {
      return false;
   }

   pXml->addAttr("shown", isVisible());

   return true;
}

bool ToolBarImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL)
   {
      return false;
   }

   if (!WindowImp::fromXml(pDocument, version))
   {
      return false;
   }

   DOMElement *pElem = static_cast<DOMElement*>(pDocument);
   if (pElem == NULL)
   {
      return false;
   }

   bool shown = StringUtilities::fromXmlString<bool>(A(pElem->getAttribute(X("shown"))));
   setVisible(shown);

   return true;
}