/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QFileDialog>
#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QLineEdit>
#include <QtGui/QMessageBox>
#include <QtGui/QPixmap>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QSpinBox>

#include "AppVerify.h"
#include "PcaDlg.h"
#include "StringUtilities.h"

#include <limits>

using namespace std;

PcaDlg::PcaDlg(const vector<string> &aoiList, unsigned int ulBands, QWidget* parent) :
   QDialog(parent)
{
   // Caption
   setWindowTitle("Principal Component Analysis");

   // Transform
   QGroupBox* pTransformGroup = new QGroupBox("Transform", this);

   mpCalculateRadio = new QRadioButton("Calculate", pTransformGroup);
   mpCalculateRadio->setFocusPolicy(Qt::StrongFocus);
   QLabel* pMethodLabel = new QLabel("Method:", pTransformGroup);

   mpMethodCombo = new QComboBox(pTransformGroup);
   mpMethodCombo->setEditable(false);
   mpMethodCombo->addItem("Second Moment");
   mpMethodCombo->addItem("Covariance");
   mpMethodCombo->addItem("Correlation Coefficient");

   QHBoxLayout* pMethodLayout = new QHBoxLayout();
   pMethodLayout->setMargin(0);
   pMethodLayout->setSpacing(5);
   pMethodLayout->addWidget(pMethodLabel);
   pMethodLayout->addWidget(mpMethodCombo);
   pMethodLayout->addStretch(10);

   mpFileRadio = new QRadioButton("Load From File", pTransformGroup);
   mpFileRadio->setFocusPolicy(Qt::StrongFocus);

   mpFileEdit = new QLineEdit(pTransformGroup);
   mpFileEdit->setMinimumWidth(250);

   QIcon icnBrowse(":/icons/Open");
   QPushButton* pBrowseButton = new QPushButton(icnBrowse, QString(), pTransformGroup);
   pBrowseButton->setFixedWidth(27);
   VERIFYNR(connect(pBrowseButton, SIGNAL(clicked()), this, SLOT(browse())));

   QHBoxLayout* pFileLayout = new QHBoxLayout();
   pFileLayout->setMargin(0);
   pFileLayout->setSpacing(5);
   pFileLayout->addWidget(mpFileEdit, 10);
   pFileLayout->addWidget(pBrowseButton);

   QGridLayout* pTransformGrid = new QGridLayout(pTransformGroup);
   pTransformGrid->setMargin(10);
   pTransformGrid->setSpacing(5);
   pTransformGrid->setColumnMinimumWidth(0, 13);
   pTransformGrid->addWidget(mpCalculateRadio, 0, 0, 1, 2);
   pTransformGrid->addLayout(pMethodLayout, 1, 1);
   pTransformGrid->addWidget(mpFileRadio, 2, 0, 1, 2);
   pTransformGrid->addLayout(pFileLayout, 3, 1);

   VERIFYNR(connect(mpCalculateRadio, SIGNAL(toggled(bool)), pMethodLabel, SLOT(setEnabled(bool))));
   VERIFYNR(connect(mpCalculateRadio, SIGNAL(toggled(bool)), mpMethodCombo, SLOT(setEnabled(bool))));
   VERIFYNR(connect(mpFileRadio, SIGNAL(toggled(bool)), mpFileEdit, SLOT(setEnabled(bool))));
   VERIFYNR(connect(mpFileRadio, SIGNAL(toggled(bool)), pBrowseButton, SLOT(setEnabled(bool))));

   // Number of components
   QLabel* pComponentsLabel = new QLabel("Number of Components:", this);
   mpComponentsSpin = new QSpinBox(this);
   mpComponentsSpin->setMinimum(1);
   mpComponentsSpin->setMaximum(ulBands);
   mpComponentsSpin->setSingleStep(1);
   mpComponentsSpin->setFixedWidth(60);

   mpFromEigenPlot = new QCheckBox("from Eigen Plot", this);
   mpFromEigenPlot->setChecked(false);

   QHBoxLayout* pNumCompLayout = new QHBoxLayout();
   pNumCompLayout->setMargin(0);
   pNumCompLayout->setSpacing(5);
   pNumCompLayout->addWidget(mpComponentsSpin);
   pNumCompLayout->addWidget(mpFromEigenPlot);
   pNumCompLayout->addStretch();

   VERIFYNR(connect(mpFromEigenPlot, SIGNAL(toggled(bool)), mpComponentsSpin, SLOT(setDisabled(bool))));

   // Output data type
   QLabel* pDataLabel = new QLabel("Output Data Type:", this);
   mpDataCombo = new QComboBox(this);
   mpDataCombo->setEditable(false);

   mpDataCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(INT1SBYTE)));
   mpDataCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(INT1UBYTE)));
   mpDataCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(INT2SBYTES)));
   mpDataCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(INT2UBYTES)));
   mpDataCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(INT4SBYTES)));
   mpDataCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(INT4UBYTES)));
   mpDataCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(FLT4BYTES)));
   mpDataCombo->addItem(QString::fromStdString(StringUtilities::toDisplayString(FLT8BYTES)));

   VERIFYNR(connect(mpDataCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(updateScaleValues())));

   // Scale values
   QLabel* pMaxScaleLabel = new QLabel("Scale Max Component Value To:", this);
   mpMaxScaleSpin = new QSpinBox(this);
   mpMaxScaleSpin->setMinimum(0);
   mpMaxScaleSpin->setMaximum(255);
   mpMaxScaleSpin->setSingleStep(1);
   mpMaxScaleSpin->setFixedWidth(85);

   QLabel* pMinScaleLabel = new QLabel("Scale Min Component Value To:", this);
   mpMinScaleSpin = new QSpinBox(this);
   mpMinScaleSpin->setMinimum(0);
   mpMinScaleSpin->setMaximum(255);
   mpMinScaleSpin->setSingleStep(1);
   mpMinScaleSpin->setFixedWidth(85);

   QVBoxLayout* pLayout = new QVBoxLayout();
   pLayout->setMargin(0);
   pLayout->setSpacing(5);
   pLayout->addWidget(pComponentsLabel);
   pLayout->addLayout(pNumCompLayout);
   pLayout->addSpacing(10);
   pLayout->addWidget(pDataLabel);
   pLayout->addWidget(mpDataCombo);
   pLayout->addSpacing(10);
   pLayout->addWidget(pMaxScaleLabel);
   pLayout->addWidget(mpMaxScaleSpin);
   pLayout->addSpacing(10);
   pLayout->addWidget(pMinScaleLabel);
   pLayout->addWidget(mpMinScaleSpin);
   pLayout->addStretch();

   // ROI
   mpRoiCheck = new QCheckBox("Region of Interest (ROI):", this);
   mpRoiCombo = new QComboBox(this);
   mpRoiCombo->setEditable(false);
   mpRoiCombo->setMinimumWidth(150);

   for (vector<string>::const_iterator iter = aoiList.begin(); iter != aoiList.end(); ++iter)
   {
      mpRoiCombo->addItem(QString::fromStdString(*iter));
   }

   QHBoxLayout* pRoiLayout = new QHBoxLayout();
   pRoiLayout->setMargin(0);
   pRoiLayout->setSpacing(5);
   pRoiLayout->addWidget(mpRoiCheck);
   pRoiLayout->addWidget(mpRoiCombo, 10);

   VERIFYNR(connect(mpRoiCheck, SIGNAL(toggled(bool)), mpRoiCombo, SLOT(setEnabled(bool))));

   // Horizontal line
   QFrame* pLine = new QFrame(this);
   pLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   // OK and Cancel buttons
   QPushButton* pOk = new QPushButton("&OK", this);
   pOk->setDefault(true);
   QPushButton* pCancel = new QPushButton("&Cancel", this);

   VERIFYNR(connect(pOk, SIGNAL(clicked()), this, SLOT(accept())));
   VERIFYNR(connect(pCancel, SIGNAL(clicked()), this, SLOT(reject())));

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
   pGrid->addWidget(pTransformGroup, 0, 0);
   pGrid->addLayout(pLayout, 0, 1, 2, 1);
   pGrid->addLayout(pRoiLayout, 1, 0);
   pGrid->addWidget(pLine, 2, 0, 1, 2, Qt::AlignBottom);
   pGrid->addLayout(pButtonLayout, 3, 0, 1, 2);
   pGrid->setRowStretch(2, 10);
   pGrid->setColumnStretch(0, 10);

   // Initialization
   mpCalculateRadio->setChecked(true);
   mpFileEdit->setEnabled(false);
   pBrowseButton->setEnabled(false);
   mpComponentsSpin->setValue(ulBands);
   mpDataCombo->setCurrentIndex(1); // This is connected to updateScaleValues so min and max will update appropriately.
   mpRoiCombo->setEnabled(false);
}

PcaDlg::~PcaDlg()
{}

QString PcaDlg::getCalcMethod() const
{
   QString strMethod;
   if (mpCalculateRadio->isChecked())
   {
      strMethod = mpMethodCombo->currentText();
   }

   return strMethod;
}

QString PcaDlg::getTransformFilename() const
{
   QString strFilename;
   if (mpFileRadio->isChecked())
   {
      strFilename = mpFileEdit->text();
   }

   return strFilename;
}

unsigned int PcaDlg::getNumComponents() const
{
   return static_cast<unsigned int>(mpComponentsSpin->value());
}

EncodingType PcaDlg::getOutputDataType() const
{
   EncodingType eDataType;

   QString strDataType = mpDataCombo->currentText();
   if (!strDataType.isEmpty())
   {
      string dataType = strDataType.toStdString();

      eDataType = StringUtilities::fromDisplayString<EncodingType>(dataType);
   }

   return eDataType;
}

int PcaDlg::getMaxScaleValue() const
{
   return mpMaxScaleSpin->value();
}

int PcaDlg::getMinScaleValue() const
{
   return mpMinScaleSpin->value();
}

QString PcaDlg::getRoiName() const
{
   QString strRoiName;
   if (mpRoiCheck->isChecked())
   {
      strRoiName = mpRoiCombo->currentText();
   }

   return strRoiName;
}

void PcaDlg::accept()
{
   if (getMinScaleValue() >= getMaxScaleValue())
   {
      QMessageBox::critical(this, "PCA", "The minimum scale value must be less than the maximum scale value.");
      return;
   }

   QDialog::accept();
}

void PcaDlg::browse()
{
   QString strFilename = QFileDialog::getOpenFileName(this, "Select PCA Transform File", QString(),
      "PCA files (*.pca*);;All Files (*.*)");
   if (strFilename.isEmpty())
   {
      return;
   }

   FILE* pFile = fopen(strFilename.toStdString().c_str(), "rt");
   if (pFile == NULL)
   {
      QMessageBox::critical(this, "PCA", "Unable to open file:\n" + strFilename);
      return; 
   }

   int lnumBands = 0;
   int lnumComponents = 0;
   int numFieldsRead = 0;
   numFieldsRead = fscanf(pFile, "%d\n", &lnumBands);
   if (numFieldsRead != 1)
   {
      QMessageBox::critical(this, "PCA", "Unable to read from file:\n" + strFilename);
      fclose(pFile);
      return;
   }

   numFieldsRead = fscanf(pFile, "%d\n", &lnumComponents);
   if (numFieldsRead != 1)
   {
      QMessageBox::critical(this, "PCA", "Unable to read from file:\n" + strFilename);
      fclose(pFile);
      return;
   }

   int ulMaxBands = mpComponentsSpin->maximum();
   if (lnumBands != ulMaxBands)
   {
      QString message = QString("File-> %1\ncontains PCA results for %2 bands.\nThere are %3 bands"
         " loaded for this image.").arg(strFilename).arg(lnumBands).arg(ulMaxBands);
      message = "Error: Mismatch on number of bands!\n" + message;
      QMessageBox::critical(this, "PCA", message);
      fclose(pFile);
      return;
   }

   fclose(pFile);
   mpComponentsSpin->setMaximum(lnumComponents);
   mpComponentsSpin->setValue(lnumComponents);
   mpFileEdit->setText(strFilename);
}

void PcaDlg::updateScaleValues()
{
   int ulMaxValue = 0;
   int ulMinValue = 0;

   EncodingType eDataType = getOutputDataType();
   switch (eDataType)
   {
      case INT1UBYTE:
         ulMaxValue = numeric_limits<unsigned char>::max();
         ulMinValue = numeric_limits<unsigned char>::min();
         break;

      case INT1SBYTE:
         ulMaxValue = numeric_limits<char>::max();
         ulMinValue = numeric_limits<char>::min();
         break;

      case INT2UBYTES:
         ulMaxValue = numeric_limits<unsigned short>::max();
         ulMinValue = numeric_limits<unsigned short>::min();
         break;

      case INT2SBYTES:
         ulMaxValue = numeric_limits<short>::max();
         ulMinValue = numeric_limits<short>::min();
         break;

      case INT4UBYTES:
         ulMaxValue = numeric_limits<int>::max();  // Needs to be returned as an int.
         ulMinValue = numeric_limits<unsigned int>::min();
         break;

      case INT4SBYTES:
      case FLT4BYTES:
      case FLT8BYTES:
         ulMaxValue = numeric_limits<int>::max();   // Needs to be returned as an int.
         ulMinValue = numeric_limits<int>::min();   // Needs to be returned as an int.
         break;

      default:
         break;
   }

   mpMaxScaleSpin->setMinimum(ulMinValue);
   mpMaxScaleSpin->setMaximum(ulMaxValue);

   mpMinScaleSpin->setMinimum(ulMinValue);
   mpMaxScaleSpin->setMaximum(ulMaxValue);
   switch (eDataType)
   {
      case FLT4BYTES:
      case FLT8BYTES:
         mpMaxScaleSpin->setValue(1);
         mpMinScaleSpin->setValue(0);
         break;

      default:
         mpMaxScaleSpin->setValue(ulMaxValue);
         mpMinScaleSpin->setValue(ulMinValue);
         break;
   }
}

bool PcaDlg::selectNumComponentsFromPlot()
{
   return mpFromEigenPlot->isChecked();
}
