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
   connect(mpDockAction, SIGNAL(triggered()), this, SLOT(dock()));
   connect(mpUndockAction, SIGNAL(triggered()), this, SLOT(undock()));
   connect(mpShowAction, SIGNAL(triggered()), this, SLOT(show()));
   connect(mpHideAction, SIGNAL(triggered()), this, SLOT(hide()));
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

View* DockWindowImp::createView(const QString& strViewName, const ViewType& viewType)
{
   View* pView = ViewWindowImp::createView(strViewName, viewType);
   if (pView != NULL)
   {
      setWidget(dynamic_cast<ViewImp*>(pView));
   }

   return pView;
}

void DockWindowImp::setWidget(QWidget* pWidget)
{
   if (pWidget == getWidget())
   {
      return;
   }

   if (pWidget != NULL)
   {
      pWidget->setParent(this);
   }

   QDockWidget::setWidget(pWidget);
   notify(SIGNAL_NAME(Subject, Modified));
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
      notify(SIGNAL_NAME(DockWindow, Docked));
   }
}

void DockWindowImp::undock()
{
   if (isFloating() == false)
   {
      setFloating(true);
      notify(SIGNAL_NAME(DockWindow, Undocked));
   }
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

bool DockWindowImp::toXml(XMLWriter* pXml) const
{
   if (!ViewWindowImp::toXml(pXml))
   {
      return false;
   }

   pXml->addAttr("features", static_cast<unsigned int>(features()));
   pXml->addAttr("allowedAreas", static_cast<unsigned int>(allowedAreas()));
   pXml->addAttr("floating", isFloating());
   pXml->addAttr("shown", isVisible());
   // save geometry
   QRect geom = geometry();
   stringstream buf;
   pXml->pushAddPoint(pXml->addElement("Geometry"));
   buf << geom.x() << " " << geom.y();
   pXml->addAttr("pos", buf.str());
   buf.str("");
   buf << geom.width() << " " << geom.height();
   pXml->addAttr("size", buf.str().c_str());
   pXml->popAddPoint();

   return true;
}

bool DockWindowImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL)
   {
      return false;
   }

   if (!ViewWindowImp::fromXml(pDocument, version))
   {
      return false;
   }

   DockWidgetFeatures features;
   Qt::DockWidgetAreas areas;
   bool floating(false);

   DOMElement *pElem = static_cast<DOMElement*>(pDocument);
   if (pElem == NULL)
   {
      return false;
   }

   features = static_cast<DockWidgetFeatures>(StringUtilities::fromXmlString<unsigned int>
      (A(pElem->getAttribute(X("features")))));
   areas = static_cast<Qt::DockWidgetAreas>(StringUtilities::fromXmlString<unsigned int>
      (A(pElem->getAttribute(X("allowedAreas")))));
   floating = StringUtilities::fromXmlString<bool>(A(pElem->getAttribute(X("floating"))));
   bool shown = StringUtilities::fromXmlString<bool>(A(pElem->getAttribute(X("shown"))));
   setFeatures(features);
   setAllowedAreas(areas);
   setFloating(floating);

   // connect view to be restored
   string viewId = A(pElem->getAttribute(X("viewId")));
   if (!viewId.empty())
   {
      View* pView(NULL);
      SessionItem* pItem = Service<SessionManager>()->getSessionItem(viewId);
      pView = dynamic_cast<View*>(pItem);
      if (pView != NULL)
      {
         View* pCurrentView = getView();
         if (pCurrentView != NULL)
         {
            setView(NULL);
            setWidget(NULL);
            Service<DesktopServices> pDesktop;
            pDesktop->deleteView(pCurrentView);
         }
         setWidget(dynamic_cast<ViewImp*>(pView));
      }
   }

   // get sub elements
   for (DOMNode *pChld = pDocument->getFirstChild();
      pChld != NULL;
      pChld = pChld->getNextSibling())
   {
      string name = A(pChld->getNodeName());
      if(XMLString::equals(pChld->getNodeName(), X("Geometry")))
      {
         LocationType pos, size;
         pElem = static_cast<DOMElement*>(pChld);
         if (pElem != NULL)
         {
            XmlReader::StrToLocation(pElem->getAttribute(X("pos")), pos);
            XmlReader::StrToLocation(pElem->getAttribute(X("size")), size);
            resize(size.mX, size.mY);
            move(pos.mX, pos.mY);
         }
      }
   }

   setVisible(shown);

   return true;
}
