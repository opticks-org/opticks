/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QApplication>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QMouseEvent>
#include <QtGui/QPushButton>

#include <qwt_plot_canvas.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_layout.h>
#include <qwt_scale_engine.h>

#include "EigenPlotDlg.h"

EigenPlotDlg::EigenPlotDlg(QWidget* parent) :
   QDialog(parent),
   mpPlot(NULL),
   mpCurve(NULL),
   mpComponentsSpin(NULL)
{
   // Eigen plot
   mpPlot = new QwtPlot(this);
   mpPlot->installEventFilter(this);
   mpPlot->setAutoFillBackground(true);

   QFont ftAxis = QApplication::font();
   ftAxis.setBold(true);
   ftAxis.setPointSize(10);

   QwtText bottomText("Number of Components");
   bottomText.setFont(ftAxis);
   mpPlot->setAxisTitle(QwtPlot::xBottom, bottomText);

   QwtText leftText("Eigen Values");
   leftText.setFont(ftAxis);
   mpPlot->setAxisTitle(QwtPlot::yLeft, leftText);

   QwtScaleEngine* pLinearScale = mpPlot->axisScaleEngine(QwtPlot::xBottom);
   pLinearScale->setAttribute(QwtScaleEngine::Floating);

   QwtLog10ScaleEngine* pLogScale = new QwtLog10ScaleEngine();
   pLogScale->setAttribute(QwtScaleEngine::Floating);
   mpPlot->setAxisScaleEngine(QwtPlot::yLeft, pLogScale);

   QPalette plotPalette = mpPlot->palette();
   plotPalette.setColor(QPalette::Window, Qt::white);
   mpPlot->setPalette(plotPalette);

   QwtPlotCanvas* pPlotCanvas = mpPlot->canvas();
   pPlotCanvas->setFrameStyle(QFrame::NoFrame);

   QwtPlotLayout* pPlotLayout = mpPlot->plotLayout();
   pPlotLayout->setMargin(5);

   QwtPlotGrid* pPlotGrid = new QwtPlotGrid();
   pPlotGrid->setPen(QPen(Qt::DotLine));
   pPlotGrid->attach(mpPlot);
   mpPlot->replot();

   // Number of components
   QLabel* pComponentsLabel = new QLabel("Number of Components:", this);
   mpComponentsSpin = new QSpinBox(this);
   mpComponentsSpin->setFixedWidth(50);
   mpComponentsSpin->setMinimum(1);

   QLabel* pDescriptionLabel = new QLabel("To set, left click in the plot or enter a value.", this);
   pDescriptionLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
   pDescriptionLabel->setWordWrap(true);

   QFont descriptionFont(pDescriptionLabel->font());
   descriptionFont.setItalic(true);
   pDescriptionLabel->setFont(descriptionFont);

   QHBoxLayout* pComponentsLayout = new QHBoxLayout();
   pComponentsLayout->setMargin(0);
   pComponentsLayout->setSpacing(5);
   pComponentsLayout->addWidget(pComponentsLabel);
   pComponentsLayout->addWidget(mpComponentsSpin);
   pComponentsLayout->addWidget(pDescriptionLabel, 10);

   // Horizontal line
   QFrame* pLine = new QFrame(this);
   pLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   // Buttons
   QPushButton* pOk = new QPushButton("&OK", this);
   QPushButton* pCancel = new QPushButton("&Cancel", this);
   connect(pOk, SIGNAL(clicked()), this, SLOT(accept()));
   connect(pCancel, SIGNAL(clicked()), this, SLOT(reject()));

   QHBoxLayout* pButtonLayout = new QHBoxLayout();
   pButtonLayout->setMargin(0);
   pButtonLayout->setSpacing(5);
   pButtonLayout->addStretch(10);
   pButtonLayout->addWidget(pOk);
   pButtonLayout->addWidget(pCancel);

   // Layout
   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(10);
   pGrid->setSpacing(10);
   pGrid->addWidget(mpPlot, 0, 0);
   pGrid->addLayout(pComponentsLayout, 1, 0);
   pGrid->addWidget(pLine, 2, 0);
   pGrid->addLayout(pButtonLayout, 3, 0);
   pGrid->setRowStretch(0, 10);

   // Initialization
   setWindowTitle("PCA Components");
   setModal(true);
   resize(440, 300);
}

EigenPlotDlg::~EigenPlotDlg()
{
}

int EigenPlotDlg::getNumComponents() const
{
   int iComponents = mpComponentsSpin->value();
   return iComponents;
}

bool EigenPlotDlg::setEigenValues(double* yVals, int numVals)
{
   if (yVals == NULL)
   {
      return false;
   }

   mpComponentsSpin->setMaximum(numVals);

   double* xVals = new double[numVals];
   for (int i = 0; i < numVals; ++i)
   {
      xVals[i] = i + 1.0;
   }

   if (mpCurve != NULL)
   {
      mpCurve->detach();
      delete mpCurve;
   }

   mpCurve = new QwtPlotCurve("Eigen Value Plot");
   mpCurve->setPen(QPen(Qt::black));
   mpCurve->setData(xVals, yVals, numVals);
   mpCurve->attach(mpPlot);

   mpPlot->replot();
   delete[] xVals;

   return true;
}

bool EigenPlotDlg::eventFilter(QObject* pObject, QEvent* pEvent)
{
   if ((pObject != NULL) && (pEvent != NULL))
   {
      if (pObject == mpPlot)
      {
         QEvent::Type eventType = pEvent->type();
         if ((eventType == QEvent::MouseButtonPress) || (eventType == QEvent::MouseMove))
         {
            QMouseEvent* pMouseEvent = static_cast<QMouseEvent*>(pEvent);
            if (mpCurve != NULL)
            {
               QPoint ptMouse = mpPlot->canvas()->mapFromGlobal(pMouseEvent->globalPos());

               // Get the component number and eigen value
               int iIndex = mpCurve->closestPoint(ptMouse);
               int iComponentValue = static_cast<int>(mpCurve->x(iIndex));
               double dEigenValue = mpCurve->y(iIndex);

               // Update the plot axis title
               QString strTitle;
               strTitle.sprintf("Component %d, Eigen Value = %f", iComponentValue, dEigenValue);

               mpPlot->setAxisTitle(QwtPlot::xBottom, strTitle);

               // Update the selected number of components
               mpComponentsSpin->setValue(iComponentValue);
            }
         }
      }
   }

   return QDialog::eventFilter(pObject, pEvent);
}
