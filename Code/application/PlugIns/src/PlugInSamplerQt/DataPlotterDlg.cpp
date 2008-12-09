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
#include "PlotSet.h"
#include "PlotView.h"
#include "PlotWidget.h"
#include "PlotWindow.h"

#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QPushButton>
#include <string>
#include <sstream>

using namespace std;

DataPlotterDlg::DataPlotterDlg(Signature &sig) :
   QDialog(NULL),
   mSig(sig),
   mpPlot(NULL),
   mpPlotSet(NULL),
   mpPlotWindow(NULL)
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

   connect(pCancelButton, SIGNAL(clicked()), this, SLOT(reject()));
   connect(mpAddButton, SIGNAL(clicked()), this, SLOT(addToPlot()));
   connect(pNewButton, SIGNAL(clicked()), this, SLOT(newPlot()));
}

DataPlotterDlg::~DataPlotterDlg()
{
}

void DataPlotterDlg::newPlot()
{
   static int sCount = 1;
   Service<DesktopServices> pDesktop;

   if (mpPlotWindow == NULL)
   {
      mpPlotWindow = static_cast<PlotWindow*>(pDesktop->getWindow("Data Plotter Window", PLOT_WINDOW));
      if (mpPlotWindow == NULL)
      {
         mpPlotWindow = static_cast<PlotWindow*>(pDesktop->createWindow("Data Plotter Window", PLOT_WINDOW));
         if (mpPlotWindow == NULL)
         {
            return;
         }
         mpPlotSet = mpPlotWindow->createPlotSet("Data Plotter Set");
         if (mpPlotSet == NULL)
         {
            pDesktop->deleteWindow(mpPlotWindow);
            return;
         }

         mpPlotWindow->show();
      }
      else
      {
         mpPlotSet = mpPlotWindow->getCurrentPlotSet();
         if (mpPlotSet == NULL)
         {
            mpPlotSet = mpPlotWindow->createPlotSet("Data Plotter Set");
            if (mpPlotSet == NULL)
            {
               return;
            }
         }
      }
   }

   stringstream name;
   name << " Data Plotter Plot " << sCount;
   PlotWidget* pPlot = mpPlotWindow->plotData(mSig, mpXbox->currentText().toStdString(), 
      mpYbox->currentText().toStdString(), name.str());
   if (pPlot == NULL)
   {
      pDesktop->deleteWindow(mpPlotWindow);
      return;
   }
   sCount++;
   mpPlot = pPlot;
   PlotView* pPlotView = mpPlot->getPlot();
   VERIFYNRV(pPlotView != NULL);
   pPlotView->zoomExtents();
   mpAddButton->setEnabled(true);
}

void DataPlotterDlg::addToPlot()
{
   VERIFYNRV(mpPlot != NULL);

   PlotView* pPlotView = mpPlot->getPlot();
   VERIFYNRV(pPlotView != NULL);

   string plotName = pPlotView->getName();
   mpPlotWindow->plotData(mSig, mpXbox->currentText().toStdString(), 
      mpYbox->currentText().toStdString(), plotName);
   pPlotView->zoomExtents();
}
