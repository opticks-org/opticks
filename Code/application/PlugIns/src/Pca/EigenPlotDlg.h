/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef EIGENPLOTDLG_H
#define EIGENPLOTDLG_H

#include <QtGui/QDialog>
#include <QtGui/QSpinBox>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>

class EigenPlotDlg : public QDialog
{
   Q_OBJECT

public:
   EigenPlotDlg(QWidget* parent = 0);
   ~EigenPlotDlg();

   int getNumComponents() const;
   bool setEigenValues(double* yVals, int numVals);

protected:
   bool eventFilter(QObject* pObject, QEvent* pEvent);

private:
   QwtPlot* mpPlot;
   QwtPlotCurve* mpCurve;
   QSpinBox* mpComponentsSpin;
};

#endif
