/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QIcon>
#include <QtGui/QCloseEvent>
#include <QtGui/QMdiArea>
#include <QtGui/QMessageBox>

#include "ApplicationWindow.h"
#include "ContextMenuActions.h"
#include "DesktopServices.h"
#include "ProductView.h"
#include "View.h"
#include "WorkspaceWindow.h"
#include "WorkspaceWindowImp.h"
#include "xmlreader.h"

#include <algorithm>
#include <sstream>

XERCES_CPP_NAMESPACE_USE
using namespace std;

WorkspaceWindowImp::WorkspaceWindowImp(const string& id, const string& windowName, QWidget* parent) :
   QMdiSubWindow(parent),
   ViewWindowImp(id, windowName),
   mpApplicationServices(Service<ApplicationServices>().get(), SIGNAL_NAME(ApplicationServices, SessionClosed),
      Slot(this, &WorkspaceWindowImp::sessionClosed)),
   mConfirmOnClose(true)
{
   setWindowTitle(QString::fromStdString(windowName));
   setAttribute(Qt::WA_DeleteOnClose);
   setFocusPolicy(Qt::StrongFocus);

   mpActiveAction = new QAction("Activate", this);
   mpActiveAction->setStatusTip("Display the window on top of all other workspace windows.");
   mpActiveAction->setAutoRepeat(false);

   VERIFYNR(connect(mpActiveAction, SIGNAL(triggered()), this, SLOT(activate())));
}

WorkspaceWindowImp::~WorkspaceWindowImp()
{
   View* pView = getView();
   if (pView != NULL)
   {
      pView->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &WorkspaceWindowImp::viewDeleted));
   }
}

list<ContextMenuAction> WorkspaceWindowImp::getContextMenuActions() const
{
   list<ContextMenuAction> menuActions = ViewWindowImp::getContextMenuActions();

   Service<DesktopServices> pDesktop;
   if (this != dynamic_cast<WorkspaceWindowImp*>(pDesktop->getCurrentWorkspaceWindow()))
   {
      menuActions.push_back(ContextMenuAction(mpActiveAction, APP_WORKSPACEWINDOW_ACTIVATE_ACTION));
   }

   View* pView = getView();
   if (pView != NULL)
   {
      list<ContextMenuAction> viewActions = pView->getContextMenuActions();
      if (viewActions.empty() == false)
      {
         copy(viewActions.begin(), viewActions.end(), back_inserter(menuActions));
      }
   }

   return menuActions;
}

const string& WorkspaceWindowImp::getObjectType() const
{
   static string sType("WorkspaceWindowImp");
   return sType;
}

bool WorkspaceWindowImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "WorkspaceWindow"))
   {
      return true;
   }

   return ViewWindowImp::isKindOf(className);
}

void WorkspaceWindowImp::viewDeleted(Subject& subject, const string& signal, const boost::any& value)
{
   View* pView = dynamic_cast<View*>(&subject);
   if (pView == NULL)
   {
      return;
   }

   if (pView == getView())
   {
      mConfirmOnClose = false;
      close();
   }
}

void WorkspaceWindowImp::sessionClosed(Subject& subject, const string& signal, const boost::any& value)
{
   Q_UNUSED(subject);
   Q_UNUSED(signal);
   Q_UNUSED(value);
   mConfirmOnClose = false;
}

void WorkspaceWindowImp::setName(const string& windowName)
{
   if (windowName.empty() == true)
   {
      return;
   }

   ViewWindowImp::setName(windowName);
   setWindowTitle(QString::fromStdString(windowName));
}

WindowType WorkspaceWindowImp::getWindowType() const
{
   return WORKSPACE_WINDOW;
}

View* WorkspaceWindowImp::getActiveView() const
{
   View* pView = getView();
   if (pView != NULL)
   {
      ProductView* pProductView = dynamic_cast<ProductView*>(pView);
      if (pProductView != NULL)
      {
         View* pEditView = pProductView->getActiveEditView();
         if (pEditView != NULL)
         {
            pView = pEditView;
         }
      }
   }

   return pView;
}

void WorkspaceWindowImp::setWidget(QWidget* pWidget)
{
   QWidget* pCurrentWidget = getWidget();
   if (pWidget == pCurrentWidget)
   {
      return;
   }

   // Detach from the current view
   View* pCurrentView = dynamic_cast<View*>(pCurrentWidget);
   if (pCurrentView != NULL)
   {
      pCurrentView->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &WorkspaceWindowImp::viewDeleted));
   }

   // Delete the previous widget
   if (pCurrentWidget != NULL)
   {
      delete pCurrentWidget;
   }

   // Set the new window widget
   notify(SIGNAL_NAME(ViewWindow, AboutToSetWidget));
   QMdiSubWindow::setWidget(pWidget);
   notify(SIGNAL_NAME(ViewWindow, WidgetSet), boost::any(pWidget));

   // Update the window icon
   QIcon windowIcon = pWidget->windowIcon();
   if (windowIcon.isNull() == false)
   {
      setIcon(windowIcon);
      setWindowIcon(windowIcon);
   }

   // Attach to the new view
   View* pView = dynamic_cast<View*>(pWidget);
   if (pView != NULL)
   {
      pView->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &WorkspaceWindowImp::viewDeleted));
   }
}

QWidget* WorkspaceWindowImp::getWidget() const
{
   return QMdiSubWindow::widget();
}

void WorkspaceWindowImp::minimize()
{
   showMinimized();
}

void WorkspaceWindowImp::maximize()
{
   showMaximized();
}

void WorkspaceWindowImp::fullScreen()
{
   showFullScreen();
}

void WorkspaceWindowImp::restore()
{
   showNormal();
}

QSize WorkspaceWindowImp::sizeHint() const
{
   int windowWidth = 450;
   int windowHeight = 300;
   WindowSizeType eWindowSize = WorkspaceWindow::getSettingWindowSize();

   QMdiArea* pMdiArea = mdiArea();
   if (eWindowSize == FIXED_SIZE)
   {
      windowWidth = WorkspaceWindow::getSettingWindowWidth();
      windowHeight = WorkspaceWindow::getSettingWindowHeight();
   }
   else if (eWindowSize == WORKSPACE_PERCENTAGE)
   {
      if (pMdiArea != NULL)
      {
         int iPercentage = static_cast<int>(WorkspaceWindow::getSettingWindowPercentage());
         windowWidth = pMdiArea->width() * iPercentage / 100;
         windowHeight = pMdiArea->height() * iPercentage / 100;
      }
   }

   if (pMdiArea != NULL)
   {
      int workspaceWidth = pMdiArea->width();
      if (windowWidth > workspaceWidth)
      {
         windowWidth = workspaceWidth;
      }

      int workspaceHeight = pMdiArea->height();
      if (windowHeight > workspaceHeight)
      {
         windowHeight = workspaceHeight;
      }
   }

   return QSize(windowWidth, windowHeight);
}

void WorkspaceWindowImp::closeEvent(QCloseEvent* pEvent)
{
   if (WorkspaceWindow::getSettingConfirmClose() == true && mConfirmOnClose == true)
   {
      int button = QMessageBox::question(this, "Confirm Close", "Are you sure that you want to close \"" +
         windowTitle() + "\"?", "Yes", "No", QString(), 0, 1);
      if (button == 1)
      {
         pEvent->ignore();
         return;
      }
   }

   ApplicationWindow* pAppWindow = dynamic_cast<ApplicationWindow*>(Service<DesktopServices>()->getMainWidget());
   if (pAppWindow != NULL)
   {
      pAppWindow->removeWindow(dynamic_cast<Window*>(this));
   }

   QMdiSubWindow::closeEvent(pEvent);
}

void WorkspaceWindowImp::changeEvent(QEvent* pEvent)
{
   // This is a workaround for QMdiSubWindow which does not save (or make available) the window restore position.
   // It uses an undocumented method, QWindowStateChangeEvent::isOverride(), to only capture changes which do not
   // specify the "override" flag, which is a flag used mainly for Qt 3 compatibility (see qmdisubwindow.cpp).
   if (pEvent != NULL && pEvent->type() == QEvent::WindowStateChange)
   {
      QWindowStateChangeEvent* pWindowStateChangeEvent = dynamic_cast<QWindowStateChangeEvent*>(pEvent);
      if (pWindowStateChangeEvent != NULL && !pWindowStateChangeEvent->isOverride())
      {
         // If going from a normal state to a minimized, maximized, or fullscreen state, store the geometry so it can
         // later be persisted during session save/restore. Note that this does NOT do a bitwise OR for oldState to
         // prevent overwriting the restore size and position when transitioning from (e.g.) minimized to maximized.
         Qt::WindowStates oldState = pWindowStateChangeEvent->oldState();
         if (oldState == Qt::WindowNoState || oldState == Qt::WindowActive)
         {
            Qt::WindowStates newState = windowState();
            if ((newState & Qt::WindowMinimized) ||
               (newState & Qt::WindowMaximized) ||
               (newState & Qt::WindowFullScreen))
            {
               mRestoreSize = size();
               mRestorePos = pos();
            }
         }
      }
   }

   QMdiSubWindow::changeEvent(pEvent);
}

bool WorkspaceWindowImp::toXml(XMLWriter* pXml) const
{
   if (!ViewWindowImp::toXml(pXml))
   {
      return false;
   }

   // This is a workaround since QMdiSubWindow does not save the window restore position in saveGeometry().
   QSize frame = size();
   QPoint location = pos();
   if (isMinimized() || isMaximized() || isFullScreen())
   {
      frame = mRestoreSize;
      location = mRestorePos;
   }

   stringstream buf;
   pXml->pushAddPoint(pXml->addElement("Geometry"));
   buf << location.x() << " " << location.y();
   pXml->addAttr("pos", buf.str());
   buf.str("");
   buf << frame.width() << " " << frame.height();
   pXml->addAttr("size", buf.str());
   pXml->addAttr("minimized", isMinimized());
   pXml->addAttr("maximized", isMaximized());
   pXml->popAddPoint();
   return true;
}

bool WorkspaceWindowImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if ((pDocument == NULL) || (ViewWindowImp::fromXml(pDocument, version) == false))
   {
      return false;
   }

   // make sure that the zoom percentage toolbar item, etc. are properly connected to the window and view.
   activate();

   // restore the workspace window geometry
   DOMElement* pConfig = dynamic_cast<DOMElement*>(findChildNode(pDocument, "Geometry"));
   if (pConfig)
   {
      LocationType pos;
      LocationType size;
      XmlReader::StrToLocation(pConfig->getAttribute(X("pos")), pos);
      XmlReader::StrToLocation(pConfig->getAttribute(X("size")), size);
      resize(size.mX, size.mY);
      move(pos.mX, pos.mY);

      bool isMinimized = StringUtilities::fromXmlString<bool>(A(pConfig->getAttribute(X("minimized"))));
      if (isMinimized)
      {
         minimize();
      }

      bool isMaximized = StringUtilities::fromXmlString<bool>(A(pConfig->getAttribute(X("maximized"))));
      if (isMaximized)
      {
         maximize();
      }
   }

   return true;
}

void WorkspaceWindowImp::activate()
{
   Service<DesktopServices>()->setCurrentWorkspaceWindow(dynamic_cast<WorkspaceWindow*>(this));
}
