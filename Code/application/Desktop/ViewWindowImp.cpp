/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "AppVersion.h"
#include "DesktopServices.h"
#include "PrintPixmap.h"
#include "SessionManager.h"
#include "SystemServicesImp.h"
#include "View.h"
#include "ViewImp.h"
#include "ViewWindowImp.h"

XERCES_CPP_NAMESPACE_USE
using namespace std;

ViewWindowImp::ViewWindowImp(const string& id, const string& windowName) :
   WindowImp(id, windowName)
{}

ViewWindowImp::~ViewWindowImp()
{}

const string& ViewWindowImp::getObjectType() const
{
   static string type("ViewWindowImp");
   return type;
}

bool ViewWindowImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "ViewWindow"))
   {
      return true;
   }

   return WindowImp::isKindOf(className);
}

View* ViewWindowImp::createView(const string& viewName, const ViewType& viewType)
{
   if (viewName.empty() == true)
   {
      return NULL;
   }

   Service<DesktopServices> pDesktop;

   View* pView = pDesktop->createView(viewName, viewType);
   if (pView != NULL)
   {
      setWidget(dynamic_cast<ViewImp*>(pView));
      return pView;
   }

   return NULL;
}

View* ViewWindowImp::getView() const
{
   return dynamic_cast<View*>(getWidget());
}

void ViewWindowImp::print(bool bSetupDialog)
{
   // Get the window image
   QPixmap windowPixmap;

   View* pView = getView();
   if (pView != NULL)
   {
      QImage windowImage;
      pView->getCurrentImage(windowImage);
      if (windowImage.isNull() == false)
      {
         windowPixmap = QPixmap::fromImage(windowImage);
      }
   }

   SystemServicesImp::instance()->WriteLogInfo(string(APP_NAME) + " is Printing a View");

   if (windowPixmap.isNull() == true)
   {
      QWidget* pWidget = getWidget();
      if (pWidget != NULL)
      {
         windowPixmap = QPixmap::grabWidget(pWidget);
      }
   }

   // Print the image
   if (windowPixmap.isNull() == false)
   {
      Service<DesktopServices> pDesktop;
      PrintPixmap(windowPixmap, bSetupDialog, pDesktop->getMainWidget());
   }
}

bool ViewWindowImp::toXml(XMLWriter* pXml) const
{
   if ((pXml == NULL) || (WindowImp::toXml(pXml) == false))
   {
      return false;
   }

   View* pView = getView();
   if (pView != NULL)
   {
      pXml->addAttr("viewId", pView->getId());
   }

   return true;
}

bool ViewWindowImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if ((pDocument == NULL) || (WindowImp::fromXml(pDocument, version) == false))
   {
      return false;
   }

   DOMElement* pElement = static_cast<DOMElement*>(pDocument);
   VERIFY(pElement != NULL);

   if (pElement->hasAttribute(X("viewId")) == true)
   {
      Service<SessionManager> pManager;

      ViewImp* pView = dynamic_cast<ViewImp*>(pManager->getSessionItem(A(pElement->getAttribute(X("viewId")))));
      if (pView != NULL)
      {
         setWidget(pView);
      }
   }

   return true;
}
