/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "DataPlotterDlg.h"
#include "DesktopServices.h"
#include "DockWindow.h"
#include "PlotSet.h"
#include "PlotSetGroup.h"
#include "PlotView.h"
#include "PlotWidget.h"

#include <QtGui/QComboBox>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QPushButton>

#include <string>
#include <sstream>
using namespace std;

DataPlotterDlg::DataPlotterDlg(Signature& sig, QWidget* pParent) :
   QDialog(pParent),
   mSig(sig),
   mpDockWindow(NULL),
   mpPlotWidget(NULL)
{
   // Axis Selectors
   QLabel* pXlabel = new QLabel("X-Axis:", this);
   mpXbox = new QComboBox(this);
   mpXbox->addItem("Band Number");
   mpXbox->addItem("Wavelength");

   QLabel* pYlabel = new QLabel("Y-Axis:", this);
   mpYbox = new QComboBox(this);
   mpYbox->addItem("Radiance");
   mpYbox->addItem("Reflectance");

   // Buttons
   QPushButton* pNewButton = new QPushButton("&New Plot", this);
   mpAddButton = new QPushButton("&Add to Plot", this);
   QPushButton* pCancelButton = new QPushButton("&Close", this);
   mpAddButton->setDisabled(true);

   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(10);
   pGrid->setSpacing(5);
   pGrid->addWidget(pXlabel, 0, 0);
   pGrid->addWidget(pYlabel, 1, 0);
   pGrid->addWidget(mpXbox, 0, 1);
   pGrid->addWidget(mpYbox, 1, 1);
   pGrid->addWidget(pNewButton, 2, 0);
   pGrid->addWidget(mpAddButton, 2, 1);
   pGrid->addWidget(pCancelButton, 3, 1);

   VERIFYNR(connect(pCancelButton, SIGNAL(clicked()), this, SLOT(reject())));
   VERIFYNR(connect(mpAddButton, SIGNAL(clicked()), this, SLOT(addToPlot())));
   VERIFYNR(connect(pNewButton, SIGNAL(clicked()), this, SLOT(newPlot())));
}

DataPlotterDlg::~DataPlotterDlg()
{}

void DataPlotterDlg::newPlot()
{
   static int sCount = 1;
   Service<DesktopServices> pDesktop;

   PlotSetGroup* pPlotSetGroup = NULL;
   if (mpDockWindow == NULL)
   {
      mpDockWindow = static_cast<DockWindow*>(pDesktop->getWindow("Data Plotter Window", DOCK_WINDOW));
      if (mpDockWindow == NULL)
      {
         mpDockWindow = static_cast<DockWindow*>(pDesktop->createWindow("Data Plotter Window", DOCK_WINDOW));
         if (mpDockWindow == NULL)
         {
            return;
         }

         pPlotSetGroup = pDesktop->createPlotSetGroup();
         if (pPlotSetGroup == NULL)
         {
            pDesktop->deleteWindow(mpDockWindow);
            mpDockWindow = NULL;
            return;
         }

         mpDockWindow->setWidget(pPlotSetGroup->getWidget());
      }
   }

   if (pPlotSetGroup == NULL)
   {
      pPlotSetGroup = dynamic_cast<PlotSetGroup*>(mpDockWindow->getWidget());
      if (pPlotSetGroup == NULL)
      {
         return;
      }
   }

   // Ensure the dock window is shown
   mpDockWindow->show();

   // Create a new plot set if necessary
   if (pPlotSetGroup->getCurrentPlotSet() == NULL)
   {
      if (pPlotSetGroup->createPlotSet("Data Plotter Set") == NULL)
      {
         return;
      }
   }

   stringstream name;
   name << " Data Plotter Plot " << sCount;

   mpPlotWidget = pPlotSetGroup->plotData(mSig, mpXbox->currentText().toStdString(),
      mpYbox->currentText().toStdString(), name.str());
   if (mpPlotWidget == NULL)
   {
      pDesktop->deleteWindow(mpDockWindow);
      return;
   }

   sCount++;
   PlotView* pPlotView = mpPlotWidget->getPlot();
   VERIFYNRV(pPlotView != NULL);
   pPlotView->zoomExtents();
   mpAddButton->setEnabled(true);
}

void DataPlotterDlg::addToPlot()
{
   VERIFYNRV(mpDockWindow != NULL);
   VERIFYNRV(mpPlotWidget != NULL);

   PlotSetGroup* pPlotSetGroup = dynamic_cast<PlotSetGroup*>(mpDockWindow->getWidget());
   VERIFYNRV(pPlotSetGroup != NULL);

   PlotView* pPlotView = mpPlotWidget->getPlot();
   VERIFYNRV(pPlotView != NULL);

   const string& plotName = pPlotView->getName();
   pPlotSetGroup->plotData(mSig, mpXbox->currentText().toStdString(), mpYbox->currentText().toStdString(), plotName);

   pPlotView->zoomExtents();
}
