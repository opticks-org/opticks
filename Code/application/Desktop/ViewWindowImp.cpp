/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ApplicationServices.h"
#include "AppVersion.h"
#include "ViewWindowImp.h"
#include "DesktopServicesImp.h"
#include "PrintPixmap.h"
#include "SessionManagerImp.h"
#include "SystemServicesImp.h"
#include "View.h"
#include "ViewImp.h"
#include "xmlreader.h"

XERCES_CPP_NAMESPACE_USE

using namespace std;

ViewWindowImp::ViewWindowImp(const string& id, const string& windowName) :
   WindowImp(id, windowName)
{
   mpView = NULL;
}

ViewWindowImp::~ViewWindowImp()
{
}

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

View* ViewWindowImp::createView(const QString& strViewName, const ViewType& viewType)
{
   if ((strViewName.isEmpty() == true) || (mpView != NULL))
   {
      return NULL;
   }

   DesktopServices* pDesktop = DesktopServicesImp::instance();
   if (pDesktop != NULL)
   {
      mpView = pDesktop->createView(strViewName.toStdString(), viewType);
   }

   return mpView;
}

View* ViewWindowImp::getView() const
{
   return mpView;
}

void ViewWindowImp::print(bool bSetupDialog)
{
   // Get the window image
   QPixmap windowPixmap;
   if (mpView != NULL)
   {
      QImage windowImage = (dynamic_cast<ViewImp*>(mpView))->getCurrentImage();
      if (windowImage.isNull() == false)
      {
         windowPixmap = QPixmap::fromImage(windowImage);
      }
   }

   SystemServicesImp::instance()->WriteLogInfo(string(APP_NAME) + " is Printing a View");

   if (windowPixmap.isNull() == true)
   {
      QWidget* pWidget = NULL;
      pWidget = getWidget();
      if (pWidget != NULL)
      {
         windowPixmap = QPixmap::grabWidget(pWidget);
      }
   }

   // Print the image
   if (windowPixmap.isNull() == false)
   {
      QWidget* pParent = NULL;

      DesktopServices* pDesktop = DesktopServicesImp::instance();
      if (pDesktop != NULL)
      {
         pParent = pDesktop->getMainWidget();
      }

      PrintPixmap(windowPixmap, bSetupDialog, pParent);
   }
}

void ViewWindowImp::setView(View *pView)
{
   mpView = pView;
}

bool ViewWindowImp::toXml(XMLWriter* pXml) const
{
   if (!WindowImp::toXml(pXml))
   {
      return false;
   }

   // save view id
   if (mpView != NULL)
   {
      pXml->addAttr("viewId", mpView->getId());
   }

   return true;
}

bool ViewWindowImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL)
   {
      return false;
   }

   if (!WindowImp::fromXml(pDocument, version))
   {
      return false;
   }

   DOMElement *pElement = static_cast<DOMElement*>(pDocument);
   if(pElement->hasAttribute(X("viewId")))
   {
      Service<DesktopServices>()->deleteView(mpView);
      setView(dynamic_cast<View*>(SessionManagerImp::instance()->getSessionItem(
         A(pElement->getAttribute(X("viewId"))))));
   }
   else
   {
      setView(NULL);
   }

   return true;
}
