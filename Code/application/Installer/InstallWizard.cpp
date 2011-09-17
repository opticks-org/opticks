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
#include "AppVersion.h"
#include "InstallWizard.h"
#include "InstallWizardInfoPage.h"
#include "InstallWizardLicensePage.h"
#include "InstallerServices.h"
#include "Progress.h"

InstallWizard::InstallWizard(QList<Aeb*>& packageDescriptors, Progress* pProgress, QWidget* pParent) :
   QWizard(pParent),
   mPackageDescriptors(packageDescriptors),
   mpProgress(pProgress)
{
   setWindowTitle("Installation Wizard");
   setModal(true);

   // Information page
   addPage(new InstallWizardInfoPage(packageDescriptors, this));

   // License pages
   int licenseId = 0;
   foreach (Aeb* pDescriptor, packageDescriptors)
   {
      if (pDescriptor == NULL)
      {
         continue;
      }
      QStringList licenses = pDescriptor->getLicenses();
      std::vector<std::string> licenseUrls = pDescriptor->getLicenseURLs();
      for (int licenseNum = 0; licenseNum < licenses.size(); ++licenseNum)
      {
         QString url = QString::fromStdString(licenseUrls[licenseNum]).toLower();
         bool isHtml = url.endsWith(".html") || url.endsWith(".htm");
         addPage(new InstallWizardLicensePage(pDescriptor, licenseId, licenses[licenseNum], isHtml, this));
         ++licenseId;
      }
   }

   // CopyFiles page
   QWizardPage* pCopyFilesPage = new QWizardPage(this);
   pCopyFilesPage->setTitle("Install Files");
   pCopyFilesPage->setSubTitle("Click Finish to complete the installation.");
   addPage(pCopyFilesPage);

   VERIFYNRV(connect(this, SIGNAL(accepted()), this, SLOT(install())));
}

InstallWizard::~InstallWizard()
{}

void InstallWizard::install()
{
   if (mPackageDescriptors.isEmpty())
   {
      return;
   }
   int count = 0;
   if (mpProgress != NULL)
   {
      mpProgress->updateProgress("Installing extensions", 1, NORMAL);
   }
   bool success = false;
   std::vector<std::string> pendingInstall;
   foreach (Aeb* pDescriptor, mPackageDescriptors)
   {
      if (pDescriptor == NULL)
      {
         continue;
      }
      pendingInstall.push_back(pDescriptor->getFilename());
   }
   Service<InstallerServices>()->setPendingInstall(pendingInstall);
   foreach (Aeb* pDescriptor, mPackageDescriptors)
   {
      if (pDescriptor == NULL)
      {
         continue;
      }
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress("Installing " + pDescriptor->getName(), 100 * count++ / mPackageDescriptors.size(), NORMAL);
      }
      if (!Service<InstallerServices>()->installExtension(pDescriptor->getFilename(), mpProgress))
      {
         if (mpProgress != NULL)
         {
            mpProgress->updateProgress("Unable to install " + pDescriptor->getName(), 0, ERRORS);
         }
      }
      else
      {
         success = true;
      }
   }
   Service<InstallerServices>()->setPendingInstall();
   if (success && mpProgress != NULL)
   {
      mpProgress->updateProgress("Installation has finished.\nInstalled extensions will be available the next time "
         + std::string(APP_NAME) + " starts.", 100, NORMAL);
   }
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
