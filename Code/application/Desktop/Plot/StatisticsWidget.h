/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef STATISTICSWIDGET_H__
#define STATISTICSWIDGET_H__

#include <QtGui/QWidget>

class HistogramPlotImp;
class QLabel;
class QTableWidget;

class StatisticsWidget : public QWidget
{
   Q_OBJECT

public:
   StatisticsWidget(HistogramPlotImp* pPlot, QWidget* pParent = 0);
   virtual ~StatisticsWidget();

public slots:
   void updateStatistics();

private:
   StatisticsWidget(const StatisticsWidget& rhs);
   StatisticsWidget& operator=(const StatisticsWidget& rhs);

   QLabel* mpTitle;
   QTableWidget* mpStats;
   HistogramPlotImp* mpHistogram;
};

#endif
