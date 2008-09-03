/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QFrame>
#include <QtGui/QLayout>
#include <QtGui/QPushButton>

#include "NameValueDlg.h"

NameValueDlg::NameValueDlg(QWidget* parent) :
   QDialog(parent)
{
   mpNameLabel = new QLabel("Name:", this);
   mpNameEdit = new QLineEdit(this);

   mpValueLabel = new QLabel("Value:", this);
   mpValueEdit = new QLineEdit(this);

   QFrame* pLine = new QFrame(this);
   pLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   QPushButton* pOK = new QPushButton("&OK", this);
   QPushButton* pCancel = new QPushButton("&Cancel", this);

   // Layout
   QHBoxLayout* pButtonLayout = new QHBoxLayout();
   pButtonLayout->setMargin(0);
   pButtonLayout->setSpacing(5);
   pButtonLayout->addStretch(10);
   pButtonLayout->addWidget(pOK);
   pButtonLayout->addWidget(pCancel);

   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(10);
   pGrid->setSpacing(10);
   pGrid->addWidget(mpNameLabel, 0, 0);
   pGrid->addWidget(mpNameEdit, 0, 1);
   pGrid->addWidget(mpValueLabel, 1, 0);
   pGrid->addWidget(mpValueEdit, 1, 1);
   pGrid->setRowStretch(2, 10);
   pGrid->addWidget(pLine, 3, 0, 1, 2);
   pGrid->addLayout(pButtonLayout, 4, 0, 1, 2);
   pGrid->setColumnStretch(1, 10);

   // Initialization
   setWindowTitle("Select Name and Value");
   setModal(true);
   resize(400, 110);

   // Connections
   connect(pOK, SIGNAL(clicked()), this, SLOT(accept()));
   connect(pCancel, SIGNAL(clicked()), this, SLOT(reject()));
}

NameValueDlg::~NameValueDlg()
{
}

QString NameValueDlg::getName() const
{
   QString strName = mpNameEdit->text();
   return strName;
}

QString NameValueDlg::getValue() const
{
   QString strValue = mpValueEdit->text();
   return strValue;
}

void NameValueDlg::setName(const QString& strName)
{
   mpNameEdit->setText(strName);
}

void NameValueDlg::setNameLabel(const QString& strName)
{
   if (strName.isEmpty() == false)
   {
      mpNameLabel->setText(strName);
   }
}

void NameValueDlg::setValue(const QString& strValue)
{
   mpValueEdit->setText(strValue);
}

void NameValueDlg::setValueLabel(const QString& strValue)
{
   if (strValue.isEmpty() == false)
   {
      mpValueLabel->setText(strValue);
   }
}

 
