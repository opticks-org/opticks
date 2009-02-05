/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef PCADLG_H
#define PCADLG_H

#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QLineEdit>
#include <QtGui/QRadioButton>
#include <QtGui/QSpinBox>

#include "TypesFile.h"

#include <vector>
#include <string>

class PcaDlg : public QDialog
{
   Q_OBJECT

public:
   bool selectNumComponentsFromPlot();
   PcaDlg(const std::vector<std::string>& aoiList, unsigned int ulBands, QWidget* parent = 0);
   ~PcaDlg();

   QString getCalcMethod() const;
   QString getTransformFilename() const;

   unsigned int getNumComponents() const;
   EncodingType getOutputDataType() const;
   int getMaxScaleValue() const;
   QString getRoiName() const;

protected slots:
   void browse();
   void updateMaxScaleValue();

private:
   QRadioButton* mpCalculateRadio;
   QComboBox* mpMethodCombo;
   QRadioButton* mpFileRadio;
   QLineEdit* mpFileEdit;
   QSpinBox* mpComponentsSpin;
   QComboBox* mpDataCombo;
   QSpinBox* mpScaleSpin;
   QCheckBox* mpRoiCheck;
   QComboBox* mpRoiCombo;
   QCheckBox* mpFromEigenPlot;
};

#endif
