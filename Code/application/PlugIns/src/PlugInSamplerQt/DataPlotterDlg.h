/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DATAPLOTTERDLG_H
#define DATAPLOTTERDLG_H

#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QLabel>

#include "TypesFile.h"

class PlotSet;
class PlotWidget;
class PlotWindow;
class Signature;

class DataPlotterDlg : public QDialog
{
   Q_OBJECT

public:
   DataPlotterDlg(Signature &sig);
   ~DataPlotterDlg();

private slots:
   void newPlot();
   void addToPlot();

private:
   QComboBox* mpXbox;
   QComboBox* mpYbox;
   QPushButton* mpAddButton;

   Signature& mSig;
   PlotWidget* mpPlot;
   PlotSet* mpPlotSet;
   PlotWindow* mpPlotWindow;
};

#endif
