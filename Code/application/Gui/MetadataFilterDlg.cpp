/*
 * The information in this file is
 * Copyright(c) 2009 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QEvent>
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
#include "MetadataFilterDlg.h"

MetadataFilterDlg::MetadataFilterDlg(QWidget* pParent) :
   QDialog(pParent)
{
   // Properties widget
   QWidget* pPropertiesWidget = new QWidget(this);
   QLabel* pFilterNameLabel = new QLabel("Name:", pPropertiesWidget);
   mpFilterNameEdit = new QLineEdit(pPropertiesWidget);

   QGridLayout* pPropertiesGrid = new QGridLayout(pPropertiesWidget);
   pPropertiesGrid->setMargin(0);
   pPropertiesGrid->setSpacing(5);
   pPropertiesGrid->addWidget(pFilterNameLabel, 0, 0);
   pPropertiesGrid->addWidget(mpFilterNameEdit, 0, 1);
   pPropertiesGrid->setColumnStretch(1, 10);

   LabeledSection* pPropertiesSection = new LabeledSection(pPropertiesWidget, "Properties", this);

   // Pattern widget
   QWidget* pPatternWidget = new QWidget(this);
   mpNameFilter = new FilterWidget(pPatternWidget);
   mpNameFilter->setWhatsThis("Only metadata attributes with this name are displayed.  If an attribute value "
      "is also specified, only attributes with this name and the specified value are displayed.  If no name "
      "is specified, all attributes match this portion of the filter.");
   mpValueFilter = new FilterWidget(pPatternWidget);
   mpValueFilter->setWhatsThis("Only metadata attributes with this value are displayed.  If an attribute name "
      "is also specified, only attributes with the specified name and this value are displayed.  If no value "
      "is specified, all attributes match this portion of the filter.");

   QFormLayout* pPatternLayout = new QFormLayout(pPatternWidget);
   pPatternLayout->setMargin(0);
   pPatternLayout->setHorizontalSpacing(5);
   pPatternLayout->setVerticalSpacing(10);
   pPatternLayout->addRow("Attribute Name:", mpNameFilter);
   pPatternLayout->addRow("Attribute Value:", mpValueFilter);

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
   setWindowTitle("Metadata Filter");
   setModal(true);

   // Connections
   VERIFYNR(connect(pButtonBox, SIGNAL(accepted()), this, SLOT(accept())));
   VERIFYNR(connect(pButtonBox, SIGNAL(rejected()), this, SLOT(reject())));
}

MetadataFilterDlg::~MetadataFilterDlg()
{}

void MetadataFilterDlg::setFilterName(const QString& name)
{
   mpFilterNameEdit->setText(name);
}

void MetadataFilterDlg::setNameFilter(const QRegExp& filter)
{
   mpNameFilter->setFilter(filter);
}

void MetadataFilterDlg::setValueFilter(const QRegExp& filter)
{
   mpValueFilter->setFilter(filter);
}

QString MetadataFilterDlg::getFilterName() const
{
   return mpFilterNameEdit->text();
}

QRegExp MetadataFilterDlg::getNameFilter() const
{
   return mpNameFilter->getFilter();
}

QRegExp MetadataFilterDlg::getValueFilter() const
{
   return mpValueFilter->getFilter();
}

void MetadataFilterDlg::accept()
{
   QString filterName = getFilterName();
   if (filterName.isEmpty() == true)
   {
      QMessageBox::critical(this, windowTitle(), "The filter name cannot be empty.");
      return;
   }

   QString metadataName = getNameFilter().pattern();
   QString metadataValue = getValueFilter().pattern();

   if ((metadataName.isEmpty() == true) && (metadataValue.isEmpty() == true))
   {
      QMessageBox::critical(this, windowTitle(), "The filter must specify a non-empty attribute name and/or "
         "attribute value pattern.");
      return;
   }

   QDialog::accept();
}

void MetadataFilterDlg::showEvent(QShowEvent* pEvent)
{
   mpFilterNameEdit->selectAll();
   mpFilterNameEdit->setFocus();
   QDialog::showEvent(pEvent);
}
