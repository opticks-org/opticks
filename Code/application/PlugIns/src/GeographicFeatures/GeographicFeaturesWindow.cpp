/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QAction>

#include "AppVersion.h"
#include "DesktopServices.h"
#include "GeographicFeaturesWidget.h"
#include "GeographicFeaturesWindow.h"
#include "MenuBar.h"
#include "PlugInRegistration.h"
#include "ToolBar.h"

REGISTER_PLUGIN_BASIC(OpticksGeographicFeatures, GeographicFeaturesWindow);

GeographicFeaturesWindow::GeographicFeaturesWindow()
{
   setName("Geographic Features Window");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setDescriptorId("{19585C57-B280-4DB0-AAA2-BD6FBE371597}");
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

GeographicFeaturesWindow::~GeographicFeaturesWindow()
{}

QAction* GeographicFeaturesWindow::createAction()
{
   QAction* pAction = NULL;

   Service<DesktopServices> pDesktop;
   ToolBar* pToolBar = dynamic_cast<ToolBar*>(pDesktop->getWindow("Geographic Features", TOOLBAR));
   if (pToolBar != NULL)
   {
      MenuBar* pMenuBar = pToolBar->getMenuBar();
      if (NULL != pMenuBar)
      {
         pAction = pMenuBar->addCommand("Geographic Features\\Geographic Features Window", getName());
         if (pAction != NULL)
         {
            pAction->setIcon(QIcon(":/GeographicFeatures/icons/GeographicFeaturesWindow"));
            pAction->setToolTip("Displays Geographic Features Window");
            pAction->setStatusTip("Toggles the display of Geographic Features Window");

            pToolBar->addButton(pAction);
         }
      }
   }

   return pAction;

}

QWidget* GeographicFeaturesWindow::createWidget()
{
   return new GeographicFeaturesWidget(Service<DesktopServices>()->getMainWidget());
}
