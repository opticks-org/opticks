/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#include "StatisticsDlg.h"

#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QPushButton>

using namespace std;

StatisticsDlg::StatisticsDlg(const QString& strCaption, const vector<string>& aoiNames, QWidget* parent) :
   QDialog(parent)
{
   // Subset
   QGroupBox* pSubsetGroup = new QGroupBox("Subset", this);

   // Skip factors
   mpFactorRadio = new QRadioButton("Skip Factors:", pSubsetGroup);
   mpFactorRadio->setFocusPolicy(Qt::StrongFocus);

   QLabel* pRowLabel = new QLabel("Row:", pSubsetGroup);
   mpRowSpin = new QSpinBox(pSubsetGroup);
   mpRowSpin->setFixedWidth(50);
   mpRowSpin->setMinimum(1);

   QLabel* pColumnLabel = new QLabel("Column:", pSubsetGroup);
   mpColumnSpin = new QSpinBox(pSubsetGroup);
   mpColumnSpin->setFixedWidth(50);
   mpColumnSpin->setMinimum(1);

   connect(mpFactorRadio, SIGNAL(toggled(bool)), pRowLabel, SLOT(setEnabled(bool)));
   connect(mpFactorRadio, SIGNAL(toggled(bool)), mpRowSpin, SLOT(setEnabled(bool)));
   connect(mpFactorRadio, SIGNAL(toggled(bool)), pColumnLabel, SLOT(setEnabled(bool)));
   connect(mpFactorRadio, SIGNAL(toggled(bool)), mpColumnSpin, SLOT(setEnabled(bool)));

   // AOI
   mpAoiRadio = new QRadioButton("AOI:", pSubsetGroup);
   mpAoiRadio->setFocusPolicy(Qt::StrongFocus);

   mpAoiCombo = new QComboBox(pSubsetGroup);
   mpAoiCombo->setEditable(false);
   mpAoiCombo->setMinimumWidth(150);

   for(vector<string>::const_iterator iter = aoiNames.begin(); iter != aoiNames.end(); iter++)
   {
      mpAoiCombo->addItem(QString::fromStdString(*iter));
   }

   connect(mpAoiRadio, SIGNAL(toggled(bool)), mpAoiCombo, SLOT(setEnabled(bool)));

   QGridLayout* pSubsetGrid = new QGridLayout(pSubsetGroup);
   pSubsetGrid->setMargin(10);
   pSubsetGrid->setSpacing(5);
   pSubsetGrid->setColumnMinimumWidth(0, 14);
   pSubsetGrid->addWidget(mpFactorRadio, 0, 0, 1, 4);
   pSubsetGrid->addWidget(pRowLabel, 1, 1);
   pSubsetGrid->addWidget(mpRowSpin, 1, 2);
   pSubsetGrid->addWidget(pColumnLabel, 2, 1);
   pSubsetGrid->addWidget(mpColumnSpin, 2, 2);
   pSubsetGrid->addWidget(mpAoiRadio, 3, 0, 1, 4);
   pSubsetGrid->addWidget(mpAoiCombo, 4, 1, 1, 3);
   pSubsetGrid->setColumnStretch(3, 10);

   // Horizontal line
   QFrame* pHLine = new QFrame(this);
   pHLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   // OK and Cancel buttons
   QPushButton* pOk = new QPushButton("&OK", this);
   pOk->setDefault(true);
   QPushButton* pCancel = new QPushButton("&Cancel", this);

   connect(pOk, SIGNAL(clicked()), this, SLOT(accept()));
   connect(pCancel, SIGNAL(clicked()), this, SLOT(reject()));

   QHBoxLayout* pButtonLayout = new QHBoxLayout();
   pButtonLayout->setMargin(0);
   pButtonLayout->setSpacing(5);
   pButtonLayout->addStretch(10);
   pButtonLayout->addWidget(pOk);
   pButtonLayout->addWidget(pCancel);

   // Layout
   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(10);
   pLayout->setSpacing(10);
   pLayout->addWidget(pSubsetGroup);
   pLayout->addStretch();
   pLayout->addWidget(pHLine);
   pLayout->addLayout(pButtonLayout);

   // Initialization
   if(strCaption.isEmpty())
   {
      setWindowTitle("PCA Statistics");
   }
   else
   {
      setWindowTitle(strCaption);
   }

   setModal(true);
   resize(300, 250);
   mpFactorRadio->setChecked(true);
   mpAoiCombo->setEnabled(false);
}

StatisticsDlg::~StatisticsDlg()
{
}

int StatisticsDlg::getRowFactor() const
{
   int iFactor = -1;
   if(mpFactorRadio->isChecked())
   {
      iFactor = mpRowSpin->value();
   }

   return iFactor;
}

int StatisticsDlg::getColumnFactor() const
{
   int iFactor = -1;
   if(mpFactorRadio->isChecked())
   {
      iFactor = mpColumnSpin->value();
   }

   return iFactor;
}

QString StatisticsDlg::getAoiName() const
{
   QString strAoiName;
   if(mpAoiRadio->isChecked())
   {
      strAoiName = mpAoiCombo->currentText();
   }

   return strAoiName;
}
