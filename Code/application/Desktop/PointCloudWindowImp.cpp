/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ChippingWindow.h"
#include "DesktopServices.h"
#include "HistogramWindowImp.h"
#include "OverviewWindow.h"
#include "PointCloudView.h"
#include "PointCloudWindowImp.h"
#include "View.h"

#include <QtGui/QIcon>

using namespace std;

PointCloudWindowImp::PointCloudWindowImp(const string& id, const string& windowName, QWidget* parent) :
   WorkspaceWindowImp(id, windowName, parent)
{
   setIcon(QIcon(":/icons/PointCloudView"));
   createView(windowName, POINT_CLOUD_VIEW);
}

PointCloudWindowImp::~PointCloudWindowImp()
{
}

const string& PointCloudWindowImp::getObjectType() const
{
   static string type("PointCloudWindowImp");
   return type;
}

bool PointCloudWindowImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "PointCloudWindow"))
   {
      return true;
   }

   return WorkspaceWindowImp::isKindOf(className);
}

WindowType PointCloudWindowImp::getWindowType() const
{
   return POINT_CLOUD_WINDOW;
}

void PointCloudWindowImp::setWidget(QWidget* pWidget)
{
   if (getWidget() == NULL)
   {
      WorkspaceWindowImp::setWidget(pWidget);
   }
}

PointCloudView* PointCloudWindowImp::getPointCloudView() const
{
   PointCloudView* pView = dynamic_cast<PointCloudView*>(getView());
   return pView;
}
