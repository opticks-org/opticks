/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ADDPLOTDLG_H
#define ADDPLOTDLG_H

#include <QtCore/QStringList>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QRadioButton>

#include "DesktopServices.h"
#include "ModelServices.h"
#include "TypesFile.h"

class PlotWidget;

class AddPlotDlg : public QDialog
{
   Q_OBJECT

public:
   AddPlotDlg(QWidget* pParent = 0);
   ~AddPlotDlg();

   QString getName() const;
   PlotType getType() const;
   QString getAxisText(const AxisPosition& eAxis) const;
   bool useGridlines() const;
   bool useLegend() const;
   void setPlot(PlotWidget* pPlot) const;

protected slots:
   void updateDataWidgets(QAbstractButton* pButton);
   void accept();

private:
   Service<DesktopServices> mpDesktop;
   Service<ModelServices> mpModel;

   QLineEdit* mpNameEdit;
   QButtonGroup* mpTypeGroup;
   QRadioButton* mpCartesianRadio;
   QRadioButton* mpHistogramRadio;
   QRadioButton* mpSignatureRadio;
   QRadioButton* mpPolarRadio;
   QCheckBox* mpGridlinesCheck;
   QCheckBox* mpLegendCheck;
   QWidget* mpCartesianDataWidget;
   QLineEdit* mpXAxisEdit;
   QLineEdit* mpYAxisEdit;
   QLabel* mpDataLabel;
   QComboBox* mpDataCombo;

   QStringList mAoiNames;
};

#endif
