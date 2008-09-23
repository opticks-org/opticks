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
#include <QtGui/QMessageBox>

#include "ApplicationWindow.h"
#include "ContextMenuActions.h"
#include "DesktopServices.h"
#include "OverviewWindow.h"
#include "ProductView.h"
#include "SessionItemSerializer.h"
#include "SessionManager.h"
#include "ViewImp.h"
#include "Workspace.h"
#include "WorkspaceWindow.h"
#include "WorkspaceWindowImp.h"
#include "xmlreader.h"

#include <algorithm>
#include <sstream>

XERCES_CPP_NAMESPACE_USE
using namespace std;

WorkspaceWindowImp::WorkspaceWindowImp(const string& id, const string& windowName, QWidget* parent) :
   QMainWindow(parent),
   ViewWindowImp(id, windowName),
   mSessionClosedReceived(false),
   mpApplicationServices(Service<ApplicationServices>().get(),
                         SIGNAL_NAME(ApplicationServices, SessionClosed),
                         Slot(this, &WorkspaceWindowImp::sessionClosed))
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
   static string type("WorkspaceWindowImp");
   return type;
}

bool WorkspaceWindowImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "WorkspaceWindow"))
   {
      return true;
   }

   return ViewWindowImp::isKindOf(className);
}

void WorkspaceWindowImp::viewDeleted(Subject &subject, const string &signal, const boost::any &value)
{
   View* pDeletedView = dynamic_cast<View*>(&subject);
   if (pDeletedView == NULL)
   {
      return;
   }

   View* pView = getView();
   if (&subject == pView)
   {
      setView(NULL);
      setWidget(NULL);

      Service<DesktopServices> pDesktop;

      ApplicationWindow* pAppWindow = dynamic_cast<ApplicationWindow*>(pDesktop->getMainWidget());
      if (pAppWindow != NULL)
      {
         pAppWindow->removeWindow(dynamic_cast<Window*>(this));
      }

      close();
   }
}

void WorkspaceWindowImp::sessionClosed(Subject &subject, const std::string &signal, const boost::any &value)
{
   Q_UNUSED(subject);
   Q_UNUSED(signal);
   Q_UNUSED(value);
   mSessionClosedReceived = true;
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

View* WorkspaceWindowImp::createView(const QString& strViewName, const ViewType& viewType)
{
   View* pView = ViewWindowImp::createView(strViewName, viewType);
   if (pView != NULL)
   {
      setWidget(dynamic_cast<ViewImp*> (pView));
      pView->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &WorkspaceWindowImp::viewDeleted));
   }

   return pView;
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
   if (pWidget != NULL)
   {
      pWidget->setParent(this);
      setCentralWidget(pWidget);

      QIcon windowIcon = pWidget->windowIcon();
      if (windowIcon.isNull() == false)
      {
         setIcon(windowIcon);
         setWindowIcon(windowIcon);
      }
   }
}

QWidget* WorkspaceWindowImp::getWidget() const
{
   return centralWidget();
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
   int windowHeight = 250;
   WindowSizeType eWindowSize = FIXED_SIZE;

   QWidget* pWorkspace = parentWidget();
   while ((pWorkspace != NULL) && (pWorkspace->inherits("QWorkspace") == false))
   {
      pWorkspace = pWorkspace->parentWidget();
   }

   eWindowSize = WorkspaceWindow::getSettingWindowSize();

   if (eWindowSize == FIXED_SIZE)
   {
      windowWidth = WorkspaceWindow::getSettingWindowWidth();
      windowHeight = WorkspaceWindow::getSettingWindowHeight();
   }
   else if (eWindowSize == WORKSPACE_PERCENTAGE)
   {
      if (pWorkspace != NULL)
      {
         int iPercentage = static_cast<int>(WorkspaceWindow::getSettingWindowPercentage());
         windowWidth = pWorkspace->width() * iPercentage / 100;
         windowHeight = pWorkspace->height() * iPercentage / 100;
      }
   }

   if (pWorkspace != NULL)
   {
      int workspaceWidth = pWorkspace->width();
      if (windowWidth > workspaceWidth)
      {
         windowWidth = workspaceWidth;
      }

      int workspaceHeight = pWorkspace->height();
      if (windowHeight > workspaceHeight)
      {
         windowHeight = workspaceHeight;
      }
   }

   return QSize(windowWidth, windowHeight);
}

void WorkspaceWindowImp::closeEvent(QCloseEvent* pEvent)
{
   if (WorkspaceWindow::getSettingConfirmClose() == true && mSessionClosedReceived == false)
   {
      int button = QMessageBox::question(this, "Confirm Close", 
            "Are you sure that you want to close \"" + windowTitle() + "\"?", "Yes", "No", QString(), 0, 1);

      if (button == 1)
      {
         pEvent->ignore();
         return;
      }
   }

   View* pView = getView();
   if (pView != NULL)
   {
      pView->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &WorkspaceWindowImp::viewDeleted));
   }

   QMainWindow::closeEvent(pEvent);
}

bool WorkspaceWindowImp::toXml(XMLWriter* pXml) const
{
   if (!ViewWindowImp::toXml(pXml))
   {
      return false;
   }

   // need to access parent for geometry of workspace windows
   QWidget* pParent = parentWidget();
   if (pParent != NULL)
   {
      QSize frame;
      QPoint location;
      if (pParent->isMaximized())
      {
         frame = size();
         location = pos();
      }
      else
      {
         frame = pParent->size();
         location = pParent->pos();
      }
      stringstream buf;
      pXml->pushAddPoint(pXml->addElement("Geometry"));
      buf << location.x() << " " << location.y();
      pXml->addAttr("pos", buf.str());
      buf.str("");
      buf << frame.width() << " " << frame.height();
      pXml->addAttr("size", buf.str());
      pXml->addAttr("minimized", pParent->isMinimized());
      pXml->addAttr("maximized", pParent->isMaximized());

      pXml->popAddPoint();
   }

   return true;
}

bool WorkspaceWindowImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL)
   {
      return false;
   }

   View *pCurrentView = getView();
   if(pCurrentView != NULL)
   {
      pCurrentView->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &WorkspaceWindowImp::viewDeleted));
   }
   if (!ViewWindowImp::fromXml(pDocument, version))
   {
      return false;
   }

   QWidget* pParent = parentWidget();
   if (pParent == NULL)
   {
      return false;
   }

   // connect view to be restored
   DOMElement *pElem = static_cast<DOMElement*>(pDocument);
   View *pView = dynamic_cast<View*>(Service<SessionManager>()->getSessionItem(A(pElem->getAttribute(X("viewId")))));
   if (pView != NULL)
   {
      setWidget(dynamic_cast<ViewImp*>(pView));
      pView->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &WorkspaceWindowImp::viewDeleted));
   }

   // make sure that the zoom percentage toolbar item, etc. are properly connected to the window and view.
   ApplicationWindow *pAppWindow = dynamic_cast<ApplicationWindow*>(Service<DesktopServices>()->getMainWidget());
   if(pAppWindow != NULL)
   {
      pAppWindow->setCurrentWorkspaceWindow(dynamic_cast<WorkspaceWindow*>(this));
   }

   // restore the workspace window geometry
   DOMElement *pConfig = dynamic_cast<DOMElement*>(findChildNode(pDocument, "Geometry"));
   if(pConfig)
   {
      LocationType pos;
      LocationType size;
      XmlReader::StrToLocation(pConfig->getAttribute(X("pos")), pos);
      XmlReader::StrToLocation(pConfig->getAttribute(X("size")), size);
      pParent->resize(size.mX, size.mY);
      pParent->move(pos.mX, pos.mY);
      bool isMinimized = StringUtilities::fromXmlString<bool>(
         A(pConfig->getAttribute(X("minimized"))));
      if (isMinimized)
      {
         minimize();
      }
      bool isMaximized = StringUtilities::fromXmlString<bool>(
         A(pConfig->getAttribute(X("maximized"))));
      if (isMaximized)
      {
         maximize();
      }
   }

   return true;
}

void WorkspaceWindowImp::activate()
{
   Service<DesktopServices> pDesktop;

   pDesktop->setCurrentWorkspaceWindow(dynamic_cast<WorkspaceWindow*>(this));
}