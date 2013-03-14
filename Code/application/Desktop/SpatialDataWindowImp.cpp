/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ChippingWindow.h"
#include "OverviewWindow.h"
#include "SessionManager.h"
#include "SpatialDataViewImp.h"
#include "SpatialDataWindowImp.h"

using namespace std;

SpatialDataWindowImp::SpatialDataWindowImp(const string& id, const string& windowName, QWidget* parent) :
   WorkspaceWindowImp(id, windowName, parent),
   mpOverview(NULL)
{
   createView(windowName, SPATIAL_DATA_VIEW);
}

SpatialDataWindowImp::~SpatialDataWindowImp()
{}

const string& SpatialDataWindowImp::getObjectType() const
{
   static string type("SpatialDataWindowImp");
   return type;
}

bool SpatialDataWindowImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "SpatialDataWindow"))
   {
      return true;
   }

   return WorkspaceWindowImp::isKindOf(className);
}

WindowType SpatialDataWindowImp::getWindowType() const
{
   return SPATIAL_DATA_WINDOW;
}

View* SpatialDataWindowImp::createView(const string& viewName, const ViewType& viewType)
{
   if (getView() == NULL)
   {
      return WorkspaceWindowImp::createView(viewName, viewType);
   }

   return NULL;
}

void SpatialDataWindowImp::setWidget(QWidget* pWidget)
{
   if ((getWidget() == NULL) || (Service<SessionManager>()->isSessionLoading() == true))
   {
      WorkspaceWindowImp::setWidget(pWidget);
   }
}

SpatialDataView* SpatialDataWindowImp::getSpatialDataView() const
{
   SpatialDataView* pView = dynamic_cast<SpatialDataView*>(getView());
   return pView;
}

void SpatialDataWindowImp::exportSubset()
{
   SpatialDataView* pView = getSpatialDataView();
   if (pView != NULL)
   {
      ChippingWindow chipWindow(pView, this);
      chipWindow.exec();
   }
}

void SpatialDataWindowImp::showOverviewWindow(bool bShow)
{
   if ((mpOverview == NULL) && (bShow == true))
   {
      SpatialDataViewImp* pView = dynamic_cast<SpatialDataViewImp*>(getView());
      if (pView != NULL)
      {
         mpOverview = new OverviewWindow(pView, this);
         if (mpOverview != NULL)
         {
            connect(mpOverview, SIGNAL(visibilityChanged(bool)), this, SIGNAL(overviewVisibilityChanged(bool)));
         }
      }
   }

   if (mpOverview == NULL)
   {
      return;
   }

   mpOverview->setVisible(bShow);
}

bool SpatialDataWindowImp::isOverviewWindowShown() const
{
   bool bShown = false;
   if (mpOverview != NULL)
   {
      bShown = mpOverview->isVisible();
   }

   return bShown;
}
