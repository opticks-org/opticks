/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QPushButton>
#include <QtGui/QTextEdit>

#include "AppVerify.h"
#include "MultiLineTextDialog.h"

using namespace std;

MultiLineTextDialog::MultiLineTextDialog(QWidget *pParent) :
   QDialog(pParent)
{
   // Text edit
   mpEdit = new QTextEdit(this);

   // Horizontal line
   QFrame* pLine = new QFrame(this);
   pLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   // Buttons
   QPushButton* pOK = new QPushButton("&OK", this);
   QPushButton* pCancel = new QPushButton("&Cancel", this);

   // Layout
   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(10);
   pGrid->setSpacing(10);
   pGrid->addWidget(mpEdit, 0, 0, 1, 3);
   pGrid->addWidget(pLine, 1, 0, 1, 3);
   pGrid->addWidget(pOK, 2, 1);
   pGrid->addWidget(pCancel, 2, 2);
   pGrid->setRowStretch(0, 10);
   pGrid->setColumnStretch(0, 10);

   // Initialization
   setWindowTitle("Enter Text");
   setModal(true);

   const string geometry = MultiLineTextDialog::getSettingGeometry();
   if (geometry.empty() == false)
   {
      VERIFYNR(restoreGeometry(QByteArray::fromBase64(QByteArray(geometry.c_str(), geometry.length()))));
   }

   // Connections
   VERIFYNR(connect(pOK, SIGNAL(clicked()), this, SLOT(accept())));
   VERIFYNR(connect(pCancel, SIGNAL(clicked()), this, SLOT(reject())));
}

void MultiLineTextDialog::accept()
{
   MultiLineTextDialog::setSettingGeometry(QString(saveGeometry().toBase64()).toStdString());
   QDialog::accept();
}

QString MultiLineTextDialog::getText() const
{
   QString text;
   if (result() == QDialog::Accepted)
   {
      text = mpEdit->toPlainText();
   }

   return text;
}

void MultiLineTextDialog::setText(const QString& text)
{
   mpEdit->append(text);
}
