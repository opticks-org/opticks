/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QWidget>

#include "ViewerShell.h"
#include "DesktopServices.h"
#include "PlugInCallback.h"
#include "Slot.h"
#include "PlugInManagerServices.h"

ViewerShell::ViewerShell()
{
   setType(PlugInManagerServices::ViewerType());
   destroyAfterExecute(false);
   Service<DesktopServices>()->attach(SIGNAL_NAME(DesktopServices, ApplicationWindowClosed), Slot(this, &ViewerShell::cleanupWidget));
}

ViewerShell::~ViewerShell()
{
   Service<DesktopServices>()->detach(SIGNAL_NAME(DesktopServices, ApplicationWindowClosed), Slot(this, &ViewerShell::cleanupWidget));
}

bool ViewerShell::getInputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

bool ViewerShell::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

class ViewerCallback : public PlugInCallback
{
public:
   ViewerCallback(PlugIn* pPlugIn, QWidget* pViewer)
   {
      mpPlugIn = pPlugIn;
      mpViewer = pViewer;
   }

   ~ViewerCallback()
   {
   }

   virtual void operator() ()
   {
      if (mpViewer != NULL)
      {
         mpViewer->hide();
         delete mpViewer;
      }
   }

   virtual bool finish()
   {
      return true;
   }

   virtual PlugIn* getPlugIn() const
   {
      return mpPlugIn;
   }

   virtual Progress* progress() const
   {
      return NULL;
   }

private:
   PlugIn* mpPlugIn;
   QWidget* mpViewer;
};

bool ViewerShell::abort()
{
   bool bSuccess = false;

   Service<DesktopServices> pDesktop;
   ViewerCallback* pCallback = new ViewerCallback(this, getWidget());
   if (pCallback != NULL)
   {
      bSuccess = pDesktop->registerCallback(BACKGROUND_COMPLETE, pCallback);
   }

   return bSuccess;
}

void ViewerShell::cleanupWidget(Subject &subject, const std::string &signal, const boost::any &data)
{
   QWidget *pWidget = getWidget();
   if(pWidget != NULL)
   {
      pWidget->hide();
      delete pWidget;
   }
}
