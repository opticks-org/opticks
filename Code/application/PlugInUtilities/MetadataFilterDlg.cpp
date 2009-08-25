/*
 * The information in this file is
 * Copyright(c) 2009 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QCheckBox>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QFrame>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QLineEdit>
#include <QtGui/QMessageBox>

#include "AppVerify.h"
#include "MetadataFilterDlg.h"

MetadataFilterDlg::MetadataFilterDlg(QWidget* pParent) :
   QDialog(pParent)
{
   QLabel* pNameLabel = new QLabel("Name:", this);
   mpNameEdit = new QLineEdit(this);
   mpNameWildcardCheck = new QCheckBox("Enable Wildcarding", this);
   mpNameCaseCheck = new QCheckBox("Case Sensitive", this);

   QLabel* pValueLabel = new QLabel("Value:", this);
   mpValueEdit = new QLineEdit(this);
   mpValueWildcardCheck = new QCheckBox("Enable Wildcarding", this);
   mpValueCaseCheck = new QCheckBox("Case Sensitive", this);

   QFrame* pLine = new QFrame(this);
   pLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   QDialogButtonBox* pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
      Qt::Horizontal, this);

   // Layout
   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(10);
   pGrid->setSpacing(5);
   pGrid->addWidget(pNameLabel, 0, 0);
   pGrid->addWidget(mpNameEdit, 0, 1);
   pGrid->addWidget(mpNameWildcardCheck, 1, 1);
   pGrid->addWidget(mpNameCaseCheck, 2, 1);
   pGrid->setRowMinimumHeight(3, 10);
   pGrid->addWidget(pValueLabel, 4, 0);
   pGrid->addWidget(mpValueEdit, 4, 1);
   pGrid->addWidget(mpValueWildcardCheck, 5, 1);
   pGrid->addWidget(mpValueCaseCheck, 6, 1);
   pGrid->setRowStretch(7, 10);
   pGrid->addWidget(pLine, 8, 0, 1, 2);
   pGrid->addWidget(pButtonBox, 9, 0, 1, 2);
   pGrid->setColumnStretch(1, 10);

   // Initialization
   setWindowTitle("Metadata Filter");
   setModal(true);
   resize(400, 110);

   // Connections
   VERIFYNR(connect(pButtonBox, SIGNAL(accepted()), this, SLOT(accept())));
   VERIFYNR(connect(pButtonBox, SIGNAL(rejected()), this, SLOT(reject())));
}

MetadataFilterDlg::~MetadataFilterDlg()
{}

QRegExp MetadataFilterDlg::getNameFilter() const
{
   QString pattern = mpNameEdit->text();
   Qt::CaseSensitivity caseSensitivity = mpNameCaseCheck->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive;
   QRegExp::PatternSyntax syntax = mpNameWildcardCheck->isChecked() ? QRegExp::Wildcard : QRegExp::FixedString;

   return QRegExp(pattern, caseSensitivity, syntax);
}

QRegExp MetadataFilterDlg::getValueFilter() const
{
   QString pattern = mpValueEdit->text();
   Qt::CaseSensitivity caseSensitivity = mpValueCaseCheck->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive;
   QRegExp::PatternSyntax syntax = mpValueWildcardCheck->isChecked() ? QRegExp::Wildcard : QRegExp::FixedString;

   return QRegExp(pattern, caseSensitivity, syntax);
}

void MetadataFilterDlg::accept()
{
   QString name = mpNameEdit->text();
   QString value = mpValueEdit->text();

   if ((name.isEmpty() == true) && (value.isEmpty() == true))
   {
      QMessageBox::critical(this, windowTitle(), "The metadata filter must specify a non-empty name and/or value.");
      return;
   }

   QDialog::accept();
}
