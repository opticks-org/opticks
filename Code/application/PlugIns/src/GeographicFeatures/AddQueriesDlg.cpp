/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AddQueriesDlg.h"
#include "AppVerify.h"

#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QDialogButtonBox>

AddQueriesDlg::AddQueriesDlg(const std::vector<std::string>& fields, QWidget* pParent) :
   QDialog(pParent)
{
   mpFillColorBox = new QCheckBox(QString("Set unique fill color for each feature"), this);
   mpLineColorBox = new QCheckBox(QString("Set unique line color for each feature"), this);
   QString lineToolTip("Setting a line color also changes the text color");
   mpFillColorBox->setToolTip(lineToolTip);
   QString title("Add Queries");
   setWindowTitle(title);
   QLabel* pAttributeLabel = new QLabel("Field:", this);
   //make the combo boxes
   mpAttributeCombo = new QComboBox(this);
   for (unsigned int i = 0; i < fields.size(); i++)
   {
      mpAttributeCombo->addItem(QString::fromStdString(fields[i]));
   }
   QFrame* pLine = new QFrame(this);
   pLine->setFrameShape(QFrame::HLine);
   pLine->setFrameShadow(QFrame::Sunken);
   QDialogButtonBox* pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
      Qt::Horizontal, this);

   VERIFYNR(connect(pButtonBox, SIGNAL(accepted()), this, SLOT(accept())));
   VERIFYNR(connect(pButtonBox, SIGNAL(rejected()), this, SLOT(reject())));

   QGridLayout* pLayout = new QGridLayout(this);
   pLayout->setMargin(10);
   pLayout->setSpacing(5);
   pLayout->addWidget(pAttributeLabel, 0, 0);
   pLayout->addWidget(mpAttributeCombo, 0, 1);
   pLayout->addWidget(mpFillColorBox, 1, 0, 1, 2);
   pLayout->addWidget(mpLineColorBox, 2, 0, 1, 2);
   pLayout->addWidget(pLine, 3, 0, 1, 2, Qt::AlignBottom);
   pLayout->addWidget(pButtonBox, 4, 0, 1, 2);
   pLayout->setRowStretch(3, 10);
   pLayout->setColumnStretch(1, 10);
}

AddQueriesDlg::~AddQueriesDlg()
{}

std::string AddQueriesDlg::getAttribute() const
{
   std::string query = mpAttributeCombo->currentText().toStdString();
   return query;
}

bool AddQueriesDlg::isFillColorUnique() const
{
   bool bChecked = false;
   if (mpFillColorBox->checkState() == Qt::Checked)
   {
      bChecked = true;
   }
   return bChecked;
}

bool AddQueriesDlg::isLineColorUnique() const
{
   bool bChecked = false;
   if (mpLineColorBox->checkState() == Qt::Checked)
   {
      bChecked = true;
   }
   return bChecked;
}