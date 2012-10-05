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

#include <QtGui/QDialog>

class DockWindow;
class PlotWidget;
class QComboBox;
class QPushButton;
class Signature;

class DataPlotterDlg : public QDialog
{
   Q_OBJECT

public:
   DataPlotterDlg(Signature& sig, QWidget* pParent = NULL);
   virtual ~DataPlotterDlg();

private slots:
   void newPlot();
   void addToPlot();

private:
   DataPlotterDlg(const DataPlotterDlg& rhs);
   DataPlotterDlg& operator=(const DataPlotterDlg& rhs);
   QComboBox* mpXbox;
   QComboBox* mpYbox;
   QPushButton* mpAddButton;

   Signature& mSig;
   DockWindow* mpDockWindow;
   PlotWidget* mpPlotWidget;
};

#endif
