/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QListWidget>
#include <QtGui/QLayout>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>

#include "NameTypeValueDlg.h"

#include "AppVerify.h"
#include "DataVariantEditor.h"
#include "DateTimeImp.h"
#include "FilenameImp.h"
#include "IconImages.h"
#include "StringUtilities.h"
#include "DynamicObjectAdapter.h"
#include "ObjectResource.h"

#include <string>
#include <vector>
using namespace std;

NameTypeValueDlg::NameTypeValueDlg(QWidget* parent) :
   QDialog(parent)
{
   mCurrentType = QString();

   // Name
   QLabel* pNameLabel = new QLabel("Name:", this);
   mpNameEdit = new QLineEdit(this);

   // Type
   QLabel* pTypeLabel = new QLabel("Type:", this);

   // Editor
   mpValueEditor = new DataVariantEditor(this);

   mpTypeList = new QListWidget(this);
   vector<DataVariantEditorDelegate> delegates = mpValueEditor->getDelegates();
   for (vector<DataVariantEditorDelegate>::iterator iter = delegates.begin();
        iter != delegates.end(); ++iter)
   {
      mpTypeList->addItem(QString::fromStdString(iter->getTypeName()));
   }
   mpTypeList->setFixedWidth(180);
   mpTypeList->setCurrentRow(0);

   // Horizontal line
   QFrame* pHLine = new QFrame(this);
   pHLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   // Buttons
   QPushButton* pOK = new QPushButton("&OK", this);
   QPushButton* pCancel = new QPushButton("&Cancel", this);

   // Layout
   QHBoxLayout* pButtonLayout = new QHBoxLayout();
   pButtonLayout->setMargin(0);
   pButtonLayout->setSpacing(5);
   pButtonLayout->addStretch();
   pButtonLayout->addWidget(pOK);
   pButtonLayout->addWidget(pCancel);

   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(10);
   pGrid->setSpacing(5);
   pGrid->addWidget(pNameLabel, 0, 0);
   pGrid->addWidget(mpNameEdit, 1, 0, 1, 2);
   pGrid->setRowMinimumHeight(2, 5);
   pGrid->addWidget(pTypeLabel, 3, 0);
   pGrid->addWidget(mpTypeList, 4, 0);
   pGrid->addWidget(mpValueEditor, 3, 1, 2, 1);
   pGrid->addWidget(pHLine, 5, 0, 1, 2);
   pGrid->setRowMinimumHeight(5, 12);
   pGrid->addLayout(pButtonLayout, 6, 0, 1, 2);
   pGrid->setRowStretch(4, 10);
   pGrid->setColumnStretch(1, 10);

   // Initialization
   resize(450, 300);
   setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

   changeType(0);

   // Connections
   VERIFYNR(connect(mpTypeList, SIGNAL(currentRowChanged(int)), this, SLOT(changeType(int))));
   VERIFYNR(connect(pOK, SIGNAL(clicked()), this, SLOT(accept())));
   VERIFYNR(connect(pCancel, SIGNAL(clicked()), this, SLOT(reject())));
}

NameTypeValueDlg::~NameTypeValueDlg()
{
}

void NameTypeValueDlg::setValue(const QString& strName, const DataVariant& value)
{
   if (value.isValid() == false)
   {
      return;
   }

   // Name
   mpNameEdit->setText(strName);

   // Type
   mCurrentType = value.getTypeName().c_str();

   string type = "";
   if (mCurrentType.isEmpty() == false)
   {
      type = mCurrentType.toStdString();

      for (int i = 0; i < mpTypeList->count(); i++)
      {
         QListWidgetItem* pItem = mpTypeList->item(i);
         if (pItem != NULL)
         {
            QString strCurrentType = pItem->text();
            if (strCurrentType == mCurrentType)
            {
               mpTypeList->setCurrentItem(pItem);
               break;
            }
         }
      }
   }

   mpValueEditor->setValue(value);
}

QString NameTypeValueDlg::getName() const
{
   QString strName = mpNameEdit->text();
   return strName;
}

QString NameTypeValueDlg::getType() const
{
   QString strType;

   QListWidgetItem* pItem = mpTypeList->currentItem();
   if (pItem != NULL)
   {
      strType = pItem->text();
   }

   return strType;
}

const DataVariant &NameTypeValueDlg::getValue()
{
   return mpValueEditor->getValue();
}

void NameTypeValueDlg::changeType(int newTypeRowIndex)
{
   QListWidgetItem* pItem = mpTypeList->item(newTypeRowIndex);
   if (pItem != NULL)
   {
      QString itemText = pItem->text();
      mCurrentType = itemText;
      mpValueEditor->setValue(DataVariant(mCurrentType.toStdString(), NULL), false);
   }
}

void NameTypeValueDlg::accept()
{
   QString strName = getName();
   if (strName.isEmpty() == true)
   {
      QMessageBox::critical(this, windowTitle(), "Please enter a name.");
      return;
   }

   QDialog::accept();
}
