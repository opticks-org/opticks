/*
 * The information in this file is
 * Copyright(c) 2009 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QComboBox>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QRadioButton>
#include <QtGui/QSpinBox>
#include <QtGui/QVBoxLayout>

#include "AoiElement.h"
#include "AppVerify.h"
#include "DataDescriptor.h"
#include "FileBrowser.h"
#include "FileDescriptor.h"
#include "ModelServices.h"
#include "RasterElement.h"
#include "CovarianceGui.h"
#include "TypeConverter.h"

CovarianceGui::CovarianceGui(RasterElement* pElement, int rowFactor, int columnFactor,
                                 bool forceRecalculate, bool elementExists, QWidget* pParent) :
   QDialog(pParent),
   mForceRecalculate(forceRecalculate),
   mElementExists(elementExists)
{
   setModal(true);
   setWindowTitle("Covariance Matrix");

   QDialogButtonBox* pButtons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, 
      Qt::Horizontal, this);

   mpCalculationMethodGroup = new QGroupBox("Calculation Method", this);
   mpUseMatrix = new QRadioButton("Use existing Covariance matrix", mpCalculationMethodGroup);
   mpUseFile = new QRadioButton("Load Covariance matrix from file", mpCalculationMethodGroup);
   mpUseSkipFactors = new QRadioButton("Calculate Covariance matrix", mpCalculationMethodGroup);
   mpUseAoi = new QRadioButton("Calculate Covariance matrix over an AOI", mpCalculationMethodGroup);

   mpRowSkipLabel = new QLabel("Row Factor:", this);
   mpColumnSkipLabel = new QLabel("Column Factor:", this);
   mpRowSkip = new QSpinBox(this);
   mpColumnSkip = new QSpinBox(this);
   mpAoi = new QComboBox(this);

   mpFileLabel = new QLabel("Input/Output File");
   mpFile = new FileBrowser(this);
   mpFile->setBrowseCaption("Locate Covariance Matrix File");
   mpFile->setBrowseFileFilters("Covariance Matrix Files (*.cvm)");
   mpFile->setBrowseExistingFile(false);

   mpMessage = new QLabel(this);

   QGridLayout* pGroupLayout = new QGridLayout(mpCalculationMethodGroup);
   pGroupLayout->setMargin(10);
   pGroupLayout->setSpacing(5);
   pGroupLayout->addWidget(mpUseMatrix, 0, 0, 1, 3);
   pGroupLayout->addWidget(mpUseFile, 1, 0, 1, 3);
   pGroupLayout->addWidget(mpUseSkipFactors, 2, 0, 1, 3);
   pGroupLayout->addWidget(mpRowSkipLabel, 3, 1);
   pGroupLayout->addWidget(mpRowSkip, 3, 2);
   pGroupLayout->addWidget(mpColumnSkipLabel, 4, 1);
   pGroupLayout->addWidget(mpColumnSkip, 4, 2);
   pGroupLayout->addWidget(mpUseAoi, 5, 0, 1, 3);
   pGroupLayout->addWidget(mpAoi, 6, 1, 1, 2);
   pGroupLayout->setColumnMinimumWidth(0, 25);

   QVBoxLayout* pTopLevel = new QVBoxLayout(this);
   pTopLevel->setMargin(10);
   pTopLevel->setSpacing(5);
   pTopLevel->addWidget(mpCalculationMethodGroup, 10);
   pTopLevel->addWidget(mpMessage);
   pTopLevel->addWidget(mpFileLabel);
   pTopLevel->addWidget(mpFile);
   pTopLevel->addWidget(pButtons);

   VERIFYNR(connect(pButtons, SIGNAL(accepted()), this, SLOT(accept())));
   VERIFYNR(connect(pButtons, SIGNAL(rejected()), this, SLOT(reject())));
   VERIFYNR(connect(mpFile, SIGNAL(filenameChanged(const QString&)), this, SLOT(updateFilename(const QString&))));
   VERIFYNR(connect(mpUseSkipFactors, SIGNAL(toggled(bool)), mpRowSkipLabel, SLOT(setEnabled(bool))));
   VERIFYNR(connect(mpUseSkipFactors, SIGNAL(toggled(bool)), mpRowSkip, SLOT(setEnabled(bool))));
   VERIFYNR(connect(mpUseSkipFactors, SIGNAL(toggled(bool)), mpColumnSkipLabel, SLOT(setEnabled(bool))));
   VERIFYNR(connect(mpUseSkipFactors, SIGNAL(toggled(bool)), mpColumnSkip, SLOT(setEnabled(bool))));
   VERIFYNR(connect(mpUseAoi, SIGNAL(toggled(bool)), mpAoi, SLOT(setEnabled(bool))));

   // initialize values
   mpRowSkip->setValue(rowFactor);
   mpColumnSkip->setValue(columnFactor);
   std::vector<DataElement*> aois = Service<ModelServices>()->getElements(pElement, 
      TypeConverter::toString<AoiElement>());
   for (std::vector<DataElement*>::const_iterator iter = aois.begin(); iter != aois.end(); ++iter)
   {
      mpAoi->addItem(QString::fromStdString((*iter)->getName()), QVariant::fromValue(reinterpret_cast<void*>(*iter)));
   }
   FileDescriptor* pFd = NULL;
   DataDescriptor* pDd = NULL;
   if (pElement != NULL)
   {
      pDd = pElement->getDataDescriptor();
   }
   if (pDd != NULL)
   {
      pFd = pDd->getFileDescriptor();
   }
   if (pFd != NULL)
   {
      mpFile->setFilename(QString::fromStdString(pFd->getFilename().getFullPathAndName() + ".cvm"));
   }
   mpUseMatrix->setEnabled(mElementExists && !mForceRecalculate);
   updateFilename(mpFile->getFilename());
   mpUseAoi->setEnabled(mpAoi->count() > 0);

   if (mForceRecalculate)
   {
      mpUseSkipFactors->setChecked(true);
   }
   else if (mElementExists)
   {
      mpUseMatrix->setChecked(true);
   }
   else if (mCvmFile.isFile())
   {
      mpUseFile->setChecked(true);
   }
   else
   {
      mpUseSkipFactors->setChecked(true);
   }
   mpRowSkipLabel->setEnabled(mpUseSkipFactors->isChecked());
   mpRowSkip->setEnabled(mpUseSkipFactors->isChecked());
   mpColumnSkipLabel->setEnabled(mpUseSkipFactors->isChecked());
   mpColumnSkip->setEnabled(mpUseSkipFactors->isChecked());
   mpAoi->setEnabled(mpUseAoi->isChecked());
}

int CovarianceGui::getRowFactor() const
{
   if (mpRowSkip->isEnabled())
   {
      return mpRowSkip->value();
   }
   return 1;
}

int CovarianceGui::getColumnFactor() const
{
   if (mpColumnSkip->isEnabled())
   {
      return mpColumnSkip->value();
   }
   return 1;
}

AoiElement* CovarianceGui::getAoi() const
{
   if (mpAoi->isEnabled())
   {
      return reinterpret_cast<AoiElement*>(mpAoi->itemData(mpAoi->currentIndex()).value<void*>());
   }
   return NULL;
}

QString CovarianceGui::getFilename() const
{
   return mpFile->getFilename();
}

bool CovarianceGui::getUseExisting() const
{
   return mpUseMatrix->isChecked();
}

bool CovarianceGui::getUseFile() const
{
   return mpUseFile->isChecked();
}

void CovarianceGui::updateFilename(const QString &filename)
{
   mCvmFile.setFile(filename);
   if (mForceRecalculate)
   {
      mpUseFile->setEnabled(false);
      mpMessage->setText("<b>The Covariance matrix must be recalculated.</b>");
   }
   else if (!mCvmFile.isFile())
   {
      if (mpUseFile->isChecked())
      {
         mpUseSkipFactors->setChecked(true);
      }
      mpUseFile->setEnabled(false);
      mpMessage->setText("<b>The file does not exist.</b>");
   }
   else
   {
      mpUseFile->setEnabled(true);
      mpMessage->clear();
   }
}