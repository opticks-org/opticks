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

#include <QtGui/QDialog>

#include "TypesFile.h"

#include <vector>
#include <string>

class QCheckBox;
class QComboBox;
class QLineEdit;
class QRadioButton;
class QSpinBox;

class PcaDlg : public QDialog
{
   Q_OBJECT

public:
   PcaDlg(const std::vector<std::string>& aoiList, unsigned int ulBands, QWidget* parent = 0);
   virtual ~PcaDlg();

   QString getCalcMethod() const;
   QString getTransformFilename() const;

   unsigned int getNumComponents() const;
   EncodingType getOutputDataType() const;
   int getMaxScaleValue() const;
   int getMinScaleValue() const;
   QString getRoiName() const;
   bool selectNumComponentsFromPlot();

public slots:
   virtual void accept();

private slots:
   void browse();
   void updateScaleValues();

private:
   QRadioButton* mpCalculateRadio;
   QComboBox* mpMethodCombo;
   QRadioButton* mpFileRadio;
   QLineEdit* mpFileEdit;
   QSpinBox* mpComponentsSpin;
   QComboBox* mpDataCombo;
   QSpinBox* mpMaxScaleSpin;
   QSpinBox* mpMinScaleSpin;
   QCheckBox* mpRoiCheck;
   QComboBox* mpRoiCombo;
   QCheckBox* mpFromEigenPlot;
};

#endif
