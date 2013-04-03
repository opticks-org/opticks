/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QContextMenuEvent>
#include <QtGui/QMenu>

#include "AppAssert.h"
#include "AppVerify.h"
#include "DockWindowImp.h"
#include "ContextMenuAction.h"
#include "ContextMenuActions.h"
#include "ContextMenuImp.h"
#include "DesktopServicesImp.h"
#include "DockWindow.h"
#include "SessionManager.h"
#include "View.h"
#include "ViewImp.h"
#include "xmlreader.h"

#include <sstream>

XERCES_CPP_NAMESPACE_USE
using namespace std;

DockWindowImp::DockWindowImp(const string& id, const string& windowName, QWidget* parent) :
   QDockWidget(QString::fromStdString(windowName), parent),
   ViewWindowImp(id, windowName)
{
   // Context menu actions
   string shortcutContext = "Dock Window";

   DesktopServicesImp* pDesktop = DesktopServicesImp::instance();
   REQUIRE(pDesktop != NULL);

   mpDockAction = new QAction("&Dock", this);
   mpDockAction->setAutoRepeat(false);
   mpDockAction->setStatusTip("Attaches the window to the main application window");
   pDesktop->initializeAction(mpDockAction, shortcutContext);

   mpUndockAction = new QAction("&Undock", this);
   mpUndockAction->setAutoRepeat(false);
   mpUndockAction->setStatusTip("Floats the window on the desktop");
   pDesktop->initializeAction(mpUndockAction, shortcutContext);

   mpShowAction = new QAction("&Show", this);
   mpShowAction->setAutoRepeat(false);
   mpShowAction->setStatusTip("Shows the window");
   pDesktop->initializeAction(mpShowAction, shortcutContext);

   mpHideAction = new QAction("&Hide", this);
   mpHideAction->setAutoRepeat(false);
   mpHideAction->setStatusTip("Hides the window");
   pDesktop->initializeAction(mpHideAction, shortcutContext);

   mpSeparatorAction = new QAction(this);
   mpSeparatorAction->setSeparator(true);

   // Initialization
   setFeatures(QDockWidget::AllDockWidgetFeatures);
   setFocusPolicy(Qt::StrongFocus);
   setContextMenuPolicy(Qt::DefaultContextMenu);

   // Connections
   VERIFYNR(connect(mpDockAction, SIGNAL(triggered()), this, SLOT(dock())));
   VERIFYNR(connect(mpUndockAction, SIGNAL(triggered()), this, SLOT(undock())));
   VERIFYNR(connect(mpShowAction, SIGNAL(triggered()), this, SLOT(show())));
   VERIFYNR(connect(mpHideAction, SIGNAL(triggered()), this, SLOT(hide())));
   VERIFYNR(connect(this, SIGNAL(topLevelChanged(bool)), this, SLOT(undocked(bool))));
}

DockWindowImp::~DockWindowImp()
{
   Service<DesktopServices> pDesktop;
   detach(SIGNAL_NAME(DockWindow, AboutToShowContextMenu),
      Signal(pDesktop.get(), SIGNAL_NAME(DesktopServices, AboutToShowContextMenu)));
}

const string& DockWindowImp::getObjectType() const
{
   static string type("DockWindowImp");
   return type;
}

bool DockWindowImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "DockWindow"))
   {
      return true;
   }

   return ViewWindowImp::isKindOf(className);
}

WindowType DockWindowImp::getWindowType() const
{
   return DOCK_WINDOW;
}

void DockWindowImp::setWidget(QWidget* pWidget)
{
   QWidget* pCurrentWidget = getWidget();
   if (pWidget == pCurrentWidget)
   {
      return;
   }

   // Set the dock window as the parent of the widget
   if (pWidget != NULL)
   {
      pWidget->setParent(this);
   }

   // Set the new window widget
   notify(SIGNAL_NAME(ViewWindow, AboutToSetWidget));
   QDockWidget::setWidget(pWidget);
   notify(SIGNAL_NAME(ViewWindow, WidgetSet), boost::any(pWidget));

   // Update the window icon
   QIcon windowIcon = pWidget->windowIcon();
   if (windowIcon.isNull() == false)
   {
      setIcon(windowIcon);
      setWindowIcon(windowIcon);
   }

   // Delete the previous widget
   if (pCurrentWidget != NULL)
   {
      delete pCurrentWidget;
   }
}

QWidget* DockWindowImp::getWidget() const
{
   return widget();
}

list<ContextMenuAction> DockWindowImp::getContextMenuActions() const
{
   list<ContextMenuAction> menuActions = SessionItemImp::getContextMenuActions();
   if (isVisible() == true)
   {
      menuActions.push_back(ContextMenuAction(mpHideAction, APP_DOCKWINDOW_HIDE_ACTION));
   }
   else
   {
      menuActions.push_back(ContextMenuAction(mpShowAction, APP_DOCKWINDOW_SHOW_ACTION));
   }

   if (isFloating() == true)
   {
      menuActions.push_back(ContextMenuAction(mpDockAction, APP_DOCKWINDOW_DOCK_ACTION));
   }
   else
   {
      menuActions.push_back(ContextMenuAction(mpUndockAction, APP_DOCKWINDOW_UNDOCK_ACTION));
   }

   return menuActions;
}

void DockWindowImp::dock()
{
   if (isFloating() == true)
   {
      setFloating(false);
   }
}

void DockWindowImp::undock()
{
   if (isFloating() == false)
   {
      setFloating(true);
   }
}

void DockWindowImp::setVisible(bool visible)
{
   QDockWidget::setVisible(visible);
   if (visible == true)
   {
      raise();
   }
}

void DockWindowImp::undocked(bool isUndocked)
{
   if (isUndocked == true)
   {
      notify(SIGNAL_NAME(DockWindow, Undocked));
   }
   else
   {
      notify(SIGNAL_NAME(DockWindow, Docked));
   }
}

bool DockWindowImp::isShown() const
{
   return isVisible();
}

bool DockWindowImp::event(QEvent* pEvent)
{
   if (pEvent != NULL)
   {
      if (pEvent->type() == QEvent::Polish)
      {
         Service<DesktopServices> pDesktop;
         attach(SIGNAL_NAME(DockWindow, AboutToShowContextMenu),
            Signal(pDesktop.get(), SIGNAL_NAME(DesktopServices, AboutToShowContextMenu)));
      }
   }

   return QDockWidget::event(pEvent);
}

void DockWindowImp::showEvent(QShowEvent* pEvent)
{
   QDockWidget::showEvent(pEvent);
   emit visibilityChanged(true);
   notify(SIGNAL_NAME(DockWindow, Shown));
}

void DockWindowImp::hideEvent(QHideEvent* pEvent)
{
   QDockWidget::hideEvent(pEvent);
   emit visibilityChanged(false);
   notify(SIGNAL_NAME(DockWindow, Hidden));
}

void DockWindowImp::contextMenuEvent(QContextMenuEvent* pEvent)
{
   if (pEvent != NULL)
   {
      // Create the context menu
      const QPoint& mouseLocation = pEvent->globalPos();
      list<ContextMenuAction> defaultActions = getContextMenuActions();

      vector<SessionItem*> sessionItems;
      sessionItems.push_back(dynamic_cast<SessionItem*>(this));

      ContextMenuImp menu(sessionItems, mouseLocation, defaultActions, this);

      // Notify to allow additional actions to be added
      notify(SIGNAL_NAME(DockWindow, AboutToShowContextMenu), boost::any(static_cast<ContextMenu*>(&menu)));

      // Invoke the menu
      if (menu.show() == true)
      {
         return;
      }
   }

   QDockWidget::contextMenuEvent(pEvent);
}

void DockWindowImp::saveState() const
{
   string dockName = getName();
   if (dockName.empty() == false)
   {
      Service<ConfigurationSettings> pSettings;
      string settingPathBase = "DockWindow/" + dockName;
      Service<DesktopServices> pDesktop;
      DockWindow* pDockWindow = dynamic_cast<DockWindow*>(const_cast<DockWindowImp*>(this));
      int iArea = static_cast<int>(pDesktop->getDockWindowArea(*pDockWindow));
      pSettings->setSetting(settingPathBase + "/DockArea", iArea);

      QByteArray dockState = saveGeometry().toBase64();
      string stateData = dockState.data();
      pSettings->setSetting(settingPathBase + "/Geometry", stateData);
   }
}

void DockWindowImp::restoreState()
{
   string dockName = getName();
   if (dockName.empty() == false)
   {
      Service<ConfigurationSettings> pSettings;
      string settingPathBase = "DockWindow/" + dockName;

      DataVariant dockArea = pSettings->getSetting(settingPathBase + "/DockArea");
      if (dockArea.isValid() && dockArea.getTypeName() == "int")
      {
         int iArea = dv_cast<int>(dockArea);
         DockWindowAreaType eDockArea = static_cast<DockWindowAreaTypeEnum>(iArea);
         Service<DesktopServices> pDesktop;
         pDesktop->setDockWindowArea(dynamic_cast<DockWindow*>(this), eDockArea);
      }

      DataVariant dockGeom = pSettings->getSetting(settingPathBase + "/Geometry");
      if (dockGeom.isValid() && dockGeom.getTypeName() == "string")
      {
         string stateData = dv_cast<string>(dockGeom);
         QByteArray dockState(stateData.c_str(), stateData.size());
         restoreGeometry(QByteArray::fromBase64(dockState));
      }
   }
}
