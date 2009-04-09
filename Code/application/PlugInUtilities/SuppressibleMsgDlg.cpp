/*
 * The information in this file is
 * Copyright(c) 2009 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QCheckBox>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QStyle>
#include <QtGui/QTextEdit>

#include "AppVerify.h"
#include "SuppressibleMsgDlg.h"

SuppressibleMsgDlg::SuppressibleMsgDlg(const std::string& dialogTitle, const std::string& dialogMsg, 
                         MessageType type, bool checkBoxState, QWidget* pParent) : 
   QDialog(pParent)
{
   setWindowTitle(QString::fromStdString(dialogTitle));
   resize(220, 50);
   setModal(true);

   QLabel* pDialogIcon = new QLabel(this);
   switch (type)
   {
   case MESSAGE_INFO:
      {
        pDialogIcon->setPixmap(style()->standardIcon(QStyle::SP_MessageBoxInformation).pixmap(QSize(30, 30)));
        break;
      }
   
   case MESSAGE_WARNING:
      {
         pDialogIcon->setPixmap(style()->standardIcon(QStyle::SP_MessageBoxWarning).pixmap(QSize(30, 30)));
         break;
      }

   case MESSAGE_ERROR:
      {
         pDialogIcon->setPixmap(style()->standardIcon(QStyle::SP_MessageBoxCritical).pixmap(QSize(30, 30)));
         break;
      }

   default:
      break;
   }

   QTextEdit* pMessage = new QTextEdit(this);
   pMessage->setText(QString::fromStdString(dialogMsg));
   pMessage->setReadOnly(true);

   mpDontShow = new QCheckBox("Don't show this again", this);
   mpDontShow->setChecked(checkBoxState);

   // OK Button.
   QPushButton* pOk = new QPushButton("&OK", this);

   QFrame* pLine = new QFrame(this);
   pLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(10);
   pGrid->setSpacing(5);
   pGrid->setColumnStretch(1, 5);
   pGrid->setRowStretch(0, 10);
   pGrid->addWidget(pOk, 3, 0, 1, 2, Qt::AlignRight);
   pGrid->addWidget(pLine, 2, 0, 1, 2);
   pGrid->addWidget(pDialogIcon, 0, 0, Qt::AlignTop);
   pGrid->addWidget(pMessage, 0, 1);
   pGrid->addWidget(mpDontShow, 1, 1);

   VERIFYNRV(connect(pOk, SIGNAL(clicked()), this, SLOT(accept())));
}

SuppressibleMsgDlg::~SuppressibleMsgDlg() { }

bool SuppressibleMsgDlg::getDontShowAgain() const
{
   return mpDontShow->isChecked();
}

void SuppressibleMsgDlg::setDontShowAgain(bool bEnable)
{
   mpDontShow->setChecked(bEnable);
}
