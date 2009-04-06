/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QCloseEvent>
#include <QtGui/QMessageBox>

#include "Aeb.h"
#include "AppAssert.h"
#include "AppVerify.h"
#include "InstallWizard.h"
#include "InstallWizardCopyFilesPage.h"
#include "InstallWizardInfoPage.h"
#include "InstallWizardLicensePage.h"

InstallWizard::InstallWizard(QList<Aeb*>& packageDescriptors, Progress* pProgress, QWidget* pParent) :
   QWizard(pParent)
{
   setWindowTitle("Installation Wizard");
   setModal(true);

   // Information page
   addPage(new InstallWizardInfoPage(packageDescriptors, this));

   // License pages
   foreach (Aeb* pDescriptor, packageDescriptors)
   {
      if (pDescriptor == NULL)
      {
         continue;
      }
      QStringList licenses = pDescriptor->getLicenses();
      std::vector<std::string> licenseUrls = pDescriptor->getLicenseURLs();
      for (unsigned int licenseNum = 0; licenseNum < licenses.size(); licenseNum++)
      {
         QString url = QString::fromStdString(licenseUrls[licenseNum]).toLower();
         bool isHtml = url.endsWith(".html") || url.endsWith(".htm");
         addPage(new InstallWizardLicensePage(pDescriptor, licenseNum, licenses[licenseNum], isHtml, this));
      }
   }

   // CopyFiles page
   addPage(new InstallWizardCopyFilesPage(packageDescriptors, pProgress, this));
}

InstallWizard::~InstallWizard()
{
}

void InstallWizard::reject()
{
   if (close())
   {
      QWizard::reject();
   }
}

void InstallWizard::closeEvent(QCloseEvent* pEvent)
{
   VERIFYNR(pEvent != NULL);
   if (QMessageBox::question(this, "Cancel installation", "Are you sure you want to cancel the installation?",
       QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
   {
      pEvent->ignore();
   }
   else
   {
      QWizard::closeEvent(pEvent);
   }
}
