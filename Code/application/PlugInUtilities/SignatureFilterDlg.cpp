/*
 * The information in this file is
 * Copyright(c) 2009 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QEvent>
#include <QtGui/QCheckBox>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QFormLayout>
#include <QtGui/QFrame>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QLineEdit>
#include <QtGui/QMessageBox>

#include "AppVerify.h"
#include "FilterWidget.h"
#include "LabeledSection.h"
#include "LabeledSectionGroup.h"
#include "SignatureFilterDlg.h"

SignatureFilterDlg::SignatureFilterDlg(QWidget* pParent) :
   QDialog(pParent)
{
   // Properties widget
   QWidget* pPropertiesWidget = new QWidget(this);
   QLabel* pFilterNameLabel = new QLabel("Name:", pPropertiesWidget);
   mpFilterNameEdit = new QLineEdit(pPropertiesWidget);
   mpLibrarySignatureCheck = new QCheckBox("Apply to library signatures", pPropertiesWidget);
   mpLibrarySignatureCheck->setWhatsThis("If this box is checked, the filter is applied to individual "
      "signatures inside of a library in addition to the library itself.  Otherwise, the filter is applied "
      "to the library as a whole, where its signatures are not checked.");
   mpLibrarySignatureCheck->setChecked(false);

   QGridLayout* pPropertiesGrid = new QGridLayout(pPropertiesWidget);
   pPropertiesGrid->setMargin(0);
   pPropertiesGrid->setSpacing(5);
   pPropertiesGrid->addWidget(pFilterNameLabel, 0, 0);
   pPropertiesGrid->addWidget(mpFilterNameEdit, 0, 1);
   pPropertiesGrid->addWidget(mpLibrarySignatureCheck, 1, 0, 1, 2);
   pPropertiesGrid->setColumnStretch(1, 10);

   LabeledSection* pPropertiesSection = new LabeledSection(pPropertiesWidget, "Properties", this);

   // Pattern widget
   QWidget* pPatternWidget = new QWidget(this);
   mpSignatureNameFilter = new FilterWidget(pPatternWidget);
   mpSignatureNameFilter->setWhatsThis("Only signatures or libraries with this name are displayed.  "
      "If no name is specified, all signatures and libraries match this portion of the filter.");
   mpMetadataNameFilter = new FilterWidget(pPatternWidget);
   mpMetadataNameFilter->setWhatsThis("Only signatures or libraries containing a metadata attribute with this "
      "name are displayed.  If an attribute value is also specified, only signatures or libraries containing a "
      "metadata attribute with this name and the specified value are displayed.  If no name is specified, all "
      "signatures and libraries match this portion of the filter.");
   mpMetadataValueFilter = new FilterWidget(pPatternWidget);
   mpMetadataValueFilter->setWhatsThis("Only signatures or libraries containing a metadata attribute with this "
      "value are displayed.  If an attribute name is also specified, the only signatures or libraries containing "
      "a metadata attribute with the specified name and this value are displayed.  If no value is specified, all "
      "signatures and libraries match this portion of the filter.");

   QFormLayout* pPatternLayout = new QFormLayout(pPatternWidget);
   pPatternLayout->setMargin(0);
   pPatternLayout->setHorizontalSpacing(5);
   pPatternLayout->setVerticalSpacing(10);
   pPatternLayout->addRow("Signature Name:", mpSignatureNameFilter);
   pPatternLayout->addRow("Metadata Attribute Name:", mpMetadataNameFilter);
   pPatternLayout->addRow("Metadata Attribute Value:", mpMetadataValueFilter);

   LabeledSection* pPatternSection = new LabeledSection(pPatternWidget, "Pattern", this);

   // Labeled section group
   LabeledSectionGroup* pSectionGroup = new LabeledSectionGroup(this);
   pSectionGroup->addSection(pPropertiesSection);
   pSectionGroup->addSection(pPatternSection);
   pSectionGroup->addStretch(10);

   // Horizontal line
   QFrame* pLine = new QFrame(this);
   pLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   // Dialog buttons
   QDialogButtonBox* pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
      Qt::Horizontal, this);

   // Layout
   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(10);
   pLayout->setSpacing(5);
   pLayout->addWidget(pSectionGroup, 10);
   pLayout->addWidget(pLine);
   pLayout->addWidget(pButtonBox);

   // Initialization
   setWindowTitle("Signature Filter");
   setModal(true);

   // Connections
   VERIFYNR(connect(pButtonBox, SIGNAL(accepted()), this, SLOT(accept())));
   VERIFYNR(connect(pButtonBox, SIGNAL(rejected()), this, SLOT(reject())));
}

SignatureFilterDlg::~SignatureFilterDlg()
{}

void SignatureFilterDlg::setFilterName(const QString& name)
{
   mpFilterNameEdit->setText(name);
}

void SignatureFilterDlg::setLibrarySignatures(bool librarySignatures)
{
   mpLibrarySignatureCheck->setChecked(librarySignatures);
}

void SignatureFilterDlg::setSignatureNameFilter(const QRegExp& filter)
{
   mpSignatureNameFilter->setFilter(filter);
}

void SignatureFilterDlg::setMetadataNameFilter(const QRegExp& filter)
{
   mpMetadataNameFilter->setFilter(filter);
}

void SignatureFilterDlg::setMetadataValueFilter(const QRegExp& filter)
{
   mpMetadataValueFilter->setFilter(filter);
}

QString SignatureFilterDlg::getFilterName() const
{
   return mpFilterNameEdit->text();
}

bool SignatureFilterDlg::getLibrarySignatures() const
{
   return mpLibrarySignatureCheck->isChecked();
}

QRegExp SignatureFilterDlg::getSignatureNameFilter() const
{
   return mpSignatureNameFilter->getFilter();
}

QRegExp SignatureFilterDlg::getMetadataNameFilter() const
{
   return mpMetadataNameFilter->getFilter();
}

QRegExp SignatureFilterDlg::getMetadataValueFilter() const
{
   return mpMetadataValueFilter->getFilter();
}

void SignatureFilterDlg::accept()
{
   QString filterName = getFilterName();
   if (filterName.isEmpty() == true)
   {
      QMessageBox::critical(this, windowTitle(), "The filter name cannot be empty.");
      return;
   }

   QString name = getSignatureNameFilter().pattern();
   QString metadataName = getMetadataNameFilter().pattern();
   QString metadataValue = getMetadataValueFilter().pattern();

   if ((name.isEmpty() == true) && (metadataName.isEmpty() == true) && (metadataValue.isEmpty() == true))
   {
      QMessageBox::critical(this, windowTitle(), "The filter must specify a non-empty signature name, "
         "metadata attribute name, and/or metadata attribute value pattern.");
      return;
   }

   QDialog::accept();
}

void SignatureFilterDlg::showEvent(QShowEvent* pEvent)
{
   mpFilterNameEdit->selectAll();
   mpFilterNameEdit->setFocus();
   QDialog::showEvent(pEvent);
}
