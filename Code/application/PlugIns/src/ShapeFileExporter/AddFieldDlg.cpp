/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AddFieldDlg.h"
#include "AppVerify.h"

#include <QtCore/QRegExp>
#include <QtGui/QComboBox>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QLineEdit>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>
#include <QtGui/QValidator>

AddFieldDlg::AddFieldDlg(QWidget* parent) :
   QDialog(parent)
{
   // Name
   QLabel* pNameLabel = new QLabel("Name:", this);
   mpNameEdit = new QLineEdit(this);

   QRegExp regExp("[\\d\\w]{1,11}");
   QRegExpValidator* pValidator = new QRegExpValidator(regExp, this);
   mpNameEdit->setValidator(pValidator);

   // Type
   QLabel* pTypeLabel = new QLabel("Type:", this);
   mpTypeCombo = new QComboBox(this);
   mpTypeCombo->setEditable(false);
   mpTypeCombo->addItem("int");
   mpTypeCombo->addItem("double");
   mpTypeCombo->addItem("string");

   // Value
   QLabel* pValueLabel = new QLabel("Value:", this);
   mpValueEdit = new QLineEdit(this);
   mpIntValidator = new QIntValidator(this);
   mpDoubleValidator = new QDoubleValidator(this);
   mpValueEdit->setValidator(mpIntValidator);

   // Description
   QLabel* pDescriptionLabel = new QLabel(this);
   pDescriptionLabel->setText("Field names are restricted to 11 characters in length and must be\n"
      "alphanumeric or an underscore (_).  Spaces are not allowed.");

   // Horizontal line
   QFrame* pHLine = new QFrame(this);
   pHLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   // Dialog buttons
   QPushButton* pOkButton = new QPushButton("&OK", this);
   QPushButton* pCancelButton = new QPushButton("&Cancel", this);

   // Layout
   QHBoxLayout* pLayout = new QHBoxLayout();
   pLayout->setMargin(0);
   pLayout->setSpacing(5);
   pLayout->addStretch(10);
   pLayout->addWidget(pOkButton);
   pLayout->addWidget(pCancelButton);

   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(10);
   pGrid->setSpacing(10);
   pGrid->addWidget(pNameLabel, 0, 0);
   pGrid->addWidget(mpNameEdit, 0, 1);
   pGrid->addWidget(pTypeLabel, 1, 0);
   pGrid->addWidget(mpTypeCombo, 1, 1, Qt::AlignLeft);
   pGrid->addWidget(pValueLabel, 2, 0);
   pGrid->addWidget(mpValueEdit, 2, 1);
   pGrid->addWidget(pDescriptionLabel, 3, 0, 1, 2, Qt::AlignTop);
   pGrid->addWidget(pHLine, 4, 0, 1, 2);
   pGrid->addLayout(pLayout, 5, 0, 1, 2);
   pGrid->setRowStretch(3, 10);
   pGrid->setColumnStretch(1, 10);

   // Initialization
   setWindowTitle("Add Field");
   setModal(true);

   // Connections
   VERIFYNR(connect(mpTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(updateValue(int))));
   VERIFYNR(connect(pOkButton, SIGNAL(clicked()), this, SLOT(accept())));
   VERIFYNR(connect(pCancelButton, SIGNAL(clicked()), this, SLOT(reject())));
}

AddFieldDlg::~AddFieldDlg()
{}

QString AddFieldDlg::getFieldName() const
{
   return mpNameEdit->text();
}

QString AddFieldDlg::getFieldType() const
{
   return mpTypeCombo->currentText();
}

DataVariant AddFieldDlg::getFieldValue() const
{
   DataVariant value;

   QString valueText = mpValueEdit->text();
   if (valueText.isEmpty() == false)
   {
      QString valueType = getFieldType();
      if (valueType == "int")
      {
         value = DataVariant(valueText.toInt());
      }
      else if (valueType == "double")
      {
         value = DataVariant(valueText.toDouble());
      }
      else if (valueType == "string")
      {
         value = DataVariant(valueText.toStdString());
      }
   }

   return value;
}

void AddFieldDlg::accept()
{
   if (getFieldName().isEmpty())
   {
      QMessageBox::critical(this, windowTitle(), "Please enter a valid field name!");
      return;
   }

   QDialog::accept();
}

void AddFieldDlg::updateValue(int typeIndex)
{
   switch (typeIndex)
   {
   case 0:  // int
      mpValueEdit->setValidator(mpIntValidator);
      break;

   case 1:  // double
      mpValueEdit->setValidator(mpDoubleValidator);
      break;

   case 2:  // string
   default:
      mpValueEdit->setValidator(NULL);
      break;
   }

   if (mpValueEdit->hasAcceptableInput() == false)
   {
      mpValueEdit->clear();
   }
}
