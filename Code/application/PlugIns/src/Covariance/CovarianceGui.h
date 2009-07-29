/*
 * The information in this file is
 * Copyright(c) 2009 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef COVARIANCEGUI_H
#define COVARIANCEGUI_H

#include <QtCore/QFileInfo>
#include <QtCore/QString>
#include <QtGui/QDialog>

class AoiElement;
class FileBrowser;
class QComboBox;
class QGroupBox;
class QLabel;
class QRadioButton;
class QSpinBox;
class RasterElement;

class CovarianceGui : public QDialog
{
   Q_OBJECT

public:
   CovarianceGui(RasterElement* pElement, int rowFactor, int columnFactor, bool forceRecalculate,
      bool elementExists, QWidget* pParent = 0);

   int getRowFactor() const;
   int getColumnFactor() const;
   AoiElement* getAoi() const;
   QString getFilename() const;
   bool getUseExisting() const;
   bool getUseFile() const;

private slots:
   void updateFilename(const QString& filename);

private:
   bool mForceRecalculate;
   bool mElementExists;
   QFileInfo mCvmFile;

   QGroupBox* mpCalculationMethodGroup;
   QRadioButton* mpUseMatrix;
   QRadioButton* mpUseFile;
   QRadioButton* mpUseSkipFactors;
   QRadioButton* mpUseAoi;

   QLabel* mpRowSkipLabel;
   QLabel* mpColumnSkipLabel;
   QSpinBox* mpRowSkip;
   QSpinBox* mpColumnSkip;
   QComboBox* mpAoi;

   QLabel* mpFileLabel;
   FileBrowser* mpFile;

   QLabel* mpMessage;
};

#endif
