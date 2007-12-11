/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "mbox.h"

#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QPushButton>

MBox::MBox(const QString& strTitle, const QString& strMessage, int type, QWidget* parent) :
   QDialog(parent)
{
   // Text label
   QLabel* pTextLabel = new QLabel(this);
   pTextLabel->setAlignment(Qt::AlignCenter);

   // Check box
   cbAlways = new QCheckBox("Ignore This Error for This Expression", this);

   // Horizontal line
   QFrame* pLine = new QFrame(this);
   pLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   // Buttons
   QPushButton* pOkButton = new QPushButton("&OK", this);
   QPushButton* pCancelButton = new QPushButton("&Cancel", this);

   // Layout
   QHBoxLayout* pButtonLayout = new QHBoxLayout();
   pButtonLayout->setMargin(0);
   pButtonLayout->setSpacing(5);
   pButtonLayout->addStretch();
   pButtonLayout->addWidget(pOkButton);
   pButtonLayout->addWidget(pCancelButton);

   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(10);
   pLayout->setSpacing(5);
   pLayout->addWidget(pTextLabel, 10);
   pLayout->addWidget(cbAlways);
   pLayout->addWidget(pLine);
   pLayout->addLayout(pButtonLayout);

   // Initialization
   setWindowTitle(strTitle);
   setModal(true);

   pTextLabel->setText(strMessage);

   switch (type)
   {
      case MB_OK:
         cbAlways->setEnabled(false);
         pCancelButton->setEnabled(false);
         break;

      case MB_CANCEL:
         cbAlways->setEnabled(false);
         pOkButton->setEnabled(false);
         break;

      case MB_OK_CANCEL:
         cbAlways->setEnabled(false);
         break;

      case MB_OK_CANCEL_ALWAYS:
         cbAlways->setEnabled(true);
         pOkButton->setEnabled(true);
         pCancelButton->setEnabled(true);
         break;

      default:
         break;
   }

   // Connections
   connect(pOkButton, SIGNAL(clicked()), this, SLOT(accept()));
   connect(pCancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

/*  
 *  Destroys the object and frees any allocated resources
 */
MBox::~MBox()
{
    // no need to delete child widgets, Qt does it all for us
}
