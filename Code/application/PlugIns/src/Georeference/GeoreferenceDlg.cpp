/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "Georeference.h"
#include "GeoreferenceDlg.h"
#include "GeoreferenceWidget.h"

#include <QtGui/QDialogButtonBox>
#include <QtGui/QFrame>
#include <QtGui/QMessageBox>
#include <QtGui/QVBoxLayout>

#include <string>

GeoreferenceDlg::GeoreferenceDlg(QWidget* pParent) :
   QDialog(pParent)
{
   // Georeference widget
   mpGeorefWidget = new GeoreferenceWidget(this);

   // Horizontal line
   QFrame* pLine = new QFrame(this);
   pLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   // Buttons
   QDialogButtonBox* pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
      Qt::Horizontal, this);

   // Layout
   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(10);
   pLayout->setSpacing(10);
   pLayout->addWidget(mpGeorefWidget, 10);
   pLayout->addWidget(pLine);
   pLayout->addWidget(pButtonBox);

   // Initialization
   setWindowTitle("Georeference");

   // Connections
   VERIFYNR(connect(pButtonBox, SIGNAL(accepted()), this, SLOT(accept())));
   VERIFYNR(connect(pButtonBox, SIGNAL(rejected()), this, SLOT(reject())));
}

GeoreferenceDlg::~GeoreferenceDlg()
{}

void GeoreferenceDlg::setDataDescriptor(RasterDataDescriptor* pDescriptor)
{
   mpGeorefWidget->setDataDescriptor(pDescriptor);
}

RasterDataDescriptor* GeoreferenceDlg::getDataDescriptor()
{
   return mpGeorefWidget->getDataDescriptor();
}

const RasterDataDescriptor* GeoreferenceDlg::getDataDescriptor() const
{
   return mpGeorefWidget->getDataDescriptor();
}

void GeoreferenceDlg::accept()
{
   Georeference* pGeoreference = mpGeorefWidget->getSelectedPlugIn();
   if (pGeoreference == NULL)
   {
      QMessageBox::critical(this, "Georeference", "Please select a georeference plug-in.");
      return;
   }

   const RasterDataDescriptor* pDescriptor = getDataDescriptor();
   std::string errorMessage;

   if (pGeoreference->validate(pDescriptor, errorMessage) == false)
   {
      QMessageBox::critical(this, "Georeference", QString::fromStdString(errorMessage));
      return;
   }
   else if (errorMessage.empty() == false)
   {
      QString message = "The georeference plug-in returned the following warning.  Do you want to continue "
         "with the georeference?\n\n" + QString::fromStdString(errorMessage);
      if (QMessageBox::warning(this, "Georeference", message, QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
      {
         return;
      }
   }

   QDialog::accept();
}
