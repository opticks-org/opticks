/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "Aeb.h"
#include "AppVerify.h"
#include "AppVersion.h"
#include "InstallerServices.h"
#include "InstallWizard.h"
#include "InstallWizardCopyFilesPage.h"
#include "Progress.h"

InstallWizardCopyFilesPage::InstallWizardCopyFilesPage(QList<Aeb*>& packageDescriptors, Progress* pProgress, QWidget* pParent) :
   QWizardPage(pParent),
   mPackageDescriptors(packageDescriptors),
   mpProgress(pProgress)
{
   setTitle("Install Files");
   setSubTitle("Click Finish to complete the installation.");
}

InstallWizardCopyFilesPage::~InstallWizardCopyFilesPage()
{
}

bool InstallWizardCopyFilesPage::validatePage()
{
   if (!QWizardPage::validatePage())
   {
      return false;
   }

   if (mPackageDescriptors.isEmpty())
   {
      return true;
   }
   int count = 0;
   if (mpProgress != NULL)
   {
      mpProgress->updateProgress("Installing extensions", 1, NORMAL);
   }
   bool success = false;
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
   if (success && mpProgress != NULL)
   {
      mpProgress->updateProgress("Installation has finished.\nInstalled extensions will be available the next time "
         + std::string(APP_NAME) + " starts.", 100, NORMAL);
   }

   return true;
}
