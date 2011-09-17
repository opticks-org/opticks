/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ComplexData.h"
#include "HistogramPlotImp.h"
#include "RasterLayer.h"
#include "Statistics.h"
#include "StatisticsWidget.h"
#include "StringUtilities.h"
#include <QtGui/QVBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QTableWidget>

StatisticsWidget::StatisticsWidget(HistogramPlotImp* pPlot, QWidget* pParent) :
      QWidget(pParent), mpHistogram(pPlot)
{
   mpTitle = new QLabel("Statistics", this);
   mpStats = new QTableWidget(this);
   mpStats->setFrameStyle(QFrame::NoFrame);
   mpStats->setCornerButtonEnabled(false);
   mpStats->setShowGrid(false);
   mpStats->setAlternatingRowColors(true);
   mpStats->setEditTriggers(QTableWidget::NoEditTriggers);
   mpStats->setColumnCount(2);
   mpStats->setRowCount(4);
   mpStats->horizontalHeader()->hide();
   mpStats->verticalHeader()->hide();
   mpStats->setItem(0, 0, new QTableWidgetItem("Minimum Value"));
   mpStats->setItem(1, 0, new QTableWidgetItem("Maximum Value"));
   mpStats->setItem(2, 0, new QTableWidgetItem("Mean Value"));
   mpStats->setItem(3, 0, new QTableWidgetItem("Standard Deviation"));

   QVBoxLayout* pTopLevel = new QVBoxLayout(this);
   pTopLevel->setMargin(0);
   pTopLevel->setSpacing(5);
   pTopLevel->addWidget(mpTitle);
   pTopLevel->addWidget(mpStats, 10);
}

StatisticsWidget::~StatisticsWidget()
{
}

void StatisticsWidget::updateStatistics()
{
   VERIFYNRV(mpHistogram);
   Statistics* pStats = mpHistogram->getStatistics();
   if (pStats != NULL)
   {
      ComplexComponent comp(COMPLEX_MAGNITUDE);
      RasterLayer* pLayer = dynamic_cast<RasterLayer*>(mpHistogram->getLayer());
      if (pLayer != NULL)
      {
         comp = pLayer->getComplexComponent();
      }
      mpTitle->setText(QString("<b>%1 Statistics</b>").arg(QString::fromStdString(StringUtilities::toDisplayString(comp))));
      mpStats->setItem(0, 1, new QTableWidgetItem(QString::number(pStats->getMin(comp))));
      mpStats->setItem(1, 1, new QTableWidgetItem(QString::number(pStats->getMax(comp))));
      mpStats->setItem(2, 1, new QTableWidgetItem(QString::number(pStats->getAverage(comp))));
      mpStats->setItem(3, 1, new QTableWidgetItem(QString::number(pStats->getStandardDeviation(comp))));
   }
}