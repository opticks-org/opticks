/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef WORKSPACEWINDOWIMP_H
#define WORKSPACEWINDOWIMP_H

#include <QtGui/QMainWindow>

#include "ApplicationServices.h"
#include "AttachmentPtr.h"
#include "TypesFile.h"
#include "ViewWindowImp.h"

#include <string>

class OverviewWindow;
class View;

class WorkspaceWindowImp : public QMainWindow, public ViewWindowImp
{
   Q_OBJECT

public:
   WorkspaceWindowImp(const std::string& id, const std::string& windowName, QWidget* parent = 0);
   ~WorkspaceWindowImp();

   using SessionItemImp::setIcon;
   std::list<ContextMenuAction> getContextMenuActions() const;

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   void viewDeleted(Subject &subject, const std::string &signal, const boost::any &value);
   void sessionClosed(Subject &subject, const std::string &signal, const boost::any &value);

   void setName(const std::string& windowName);
   WindowType getWindowType() const;

   View* createView(const QString& strViewName, const ViewType& viewType);
   View* getActiveView() const;
   void setWidget(QWidget* pWidget);
   QWidget* getWidget() const;

   void minimize();
   void maximize();
   void fullScreen();
   void restore();

   QSize sizeHint() const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

protected slots:
   void activate();

protected:
   AttachmentPtr<ApplicationServices> mpApplicationServices;
   virtual void closeEvent(QCloseEvent* pEvent);

private:
   bool mConfirmOnClose;
   QAction* mpActiveAction;

   WINDOWIMPDROP_METHODS(ViewWindowImp);
};

#define WORKSPACEWINDOWADAPTEREXTENSION_CLASSES \
   VIEWWINDOWADAPTEREXTENSION_CLASSES

#define WORKSPACEWINDOWADAPTER_METHODS(impClass) \
   VIEWWINDOWADAPTER_METHODS(impClass) \
   void minimize() \
   { \
      impClass::minimize(); \
   } \
   void maximize() \
   { \
      impClass::maximize(); \
   } \
   void fullScreen() \
   { \
      impClass::fullScreen(); \
   } \
   void restore() \
   { \
      impClass::restore(); \
   } \
   View* getActiveView() const \
   { \
      return impClass::getActiveView(); \
   }

#endif
