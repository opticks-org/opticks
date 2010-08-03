/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QGridLayout>
#include <QtGui/QLabel>

#include "AppVerify.h"
#include "DesktopServices.h"
#include "GraphicViewWidget.h"
#include "SpatialDataWindow.h"
#include "View.h"

#include <string>
using namespace std;

GraphicViewWidget::GraphicViewWidget(QWidget* pParent) :
   QWidget(pParent)
{
   // View
   QLabel* pViewLabel = new QLabel("View:", this);
   mpViewCombo = new QComboBox(this);
   mpViewCombo->setEditable(false);

   // Layout
   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(0);
   pGrid->setSpacing(5);
   pGrid->addWidget(pViewLabel, 0, 0);
   pGrid->addWidget(mpViewCombo, 0, 1);
   pGrid->setRowStretch(1, 10);
   pGrid->setColumnStretch(1, 10);

   // Initialization
   Service<DesktopServices> pDesktop;

   vector<Window*> windows;
   pDesktop->getWindows(SPATIAL_DATA_WINDOW, windows);
   for (vector<Window*>::iterator iter = windows.begin(); iter != windows.end(); ++iter)
   {
      SpatialDataWindow* pSpatialDataWindow = dynamic_cast<SpatialDataWindow*>(*iter);
      if (pSpatialDataWindow != NULL)
      {
         View* pView = pSpatialDataWindow->getView();
         addView(pView);
      }
   }

   // Connections
   VERIFYNR(connect(mpViewCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(notifyViewChange())));
}

GraphicViewWidget::~GraphicViewWidget()
{
}

View* GraphicViewWidget::getView() const
{
   View* pView = NULL;

   int index = mpViewCombo->currentIndex();
   if ((index > -1) && (index < static_cast<int>(mViews.size())))
   {
      pView = mViews[index];
   }

   return pView;
}

void GraphicViewWidget::setView(View* pView)
{
   if (pView != getView())
   {
      QString viewName;
      if (pView != NULL)
      {
         viewName = QString::fromStdString(pView->getDisplayName());
         if (viewName.isEmpty() == true)
         {
            viewName = QString::fromStdString(pView->getName());
         }
      }

      int index = mpViewCombo->findText(viewName);
      mpViewCombo->setCurrentIndex(index);
   }
}

void GraphicViewWidget::addView(View* pView)
{
   if (pView != NULL)
   {
      vector<View*>::iterator iter;
      for (iter = mViews.begin(); iter != mViews.end(); ++iter)
      {
         View* pCurrentView = *iter;
         if (pCurrentView == pView)
         {
            return;
         }
      }

      string viewName = pView->getDisplayName();
      if (viewName.empty() == true)
      {
         viewName = pView->getName();
      }

      if (viewName.empty() == false)
      {
         mpViewCombo->addItem(QString::fromStdString(viewName));
         mViews.push_back(pView);
      }
   }
}

void GraphicViewWidget::notifyViewChange()
{
   View* pView = getView();
   emit viewChanged(pView);
}
