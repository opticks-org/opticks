/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QString>
#include <QtGui/QLabel>
#include <QtGui/QRadioButton>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>

#include "Aeb.h"
#include "AppVerify.h"
#include "InstallWizardLicensePage.h"

InstallWizardLicensePage::InstallWizardLicensePage(const Aeb* pPackageDescriptor,
                                                   unsigned int licenseNum,
                                                   const QString& license,
                                                   bool isHtml,
                                                   QWidget* pParent) :
   QWizardPage(pParent)
{
   setTitle("License Agreement");
   setSubTitle("Please read the License Agreement below.");

   QLabel* pName = new QLabel(QString::fromStdString(pPackageDescriptor->getName()), this);
   QTextEdit* pLicense = new QTextEdit(this);
   pLicense->setReadOnly(true);
   if (isHtml)
   {
      pLicense->setHtml(license);
   }
   else
   {
      pLicense->setPlainText(license);
   }

   QRadioButton* pAccept = new QRadioButton("I Accept this License Agreement", this);
   QRadioButton* pReject = new QRadioButton("I Reject this License Agreement", this);
   pReject->setChecked(true);

   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setSpacing(5);
   pLayout->setMargin(10);
   pLayout->addWidget(pName);
   pLayout->addWidget(pLicense);
   pLayout->addWidget(pAccept);
   pLayout->addWidget(pReject);

   // Require pAccept to be checked before enabling the Next button
   registerField(QString("license %1*").arg(licenseNum), pAccept);
}

InstallWizardLicensePage::~InstallWizardLicensePage()
{
}
