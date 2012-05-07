/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "Aeb.h"

#include "AebIo.h"
#include "AppConfig.h"
#include "ConfigurationSettings.h"
#include "InstallerServices.h"
#include "ProductView.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QUrl>

namespace
{
   bool andAccum(bool a, bool b) { return a && b; }
};

Aeb::Aeb() : mHidden(false), mpIcon(NULL)
{
}

Aeb::~Aeb()
{
   delete mpIcon;
}

bool Aeb::validate() const
{
   if (!mId.isValid() || !mVersion.isValid() || mName.empty() || mTargetApplication.empty())
   {
      return false;
   }
   if (mLicenses.size() != static_cast<int>(mLicenseURLs.size()))
   {
      return false;
   }
   std::string ignoreMsg;
   return checkTargetApplication(ignoreMsg);
}

bool Aeb::meetsRequirements(std::string& errMsg) const
{
   Service<InstallerServices> iservices;
   // validate requires
   for (std::vector<std::pair<AebRequirement, AebRequirement> >::const_iterator req = mRequires.begin(); req != mRequires.end(); ++req)
   {
      // first is the target application...if the is the target application and the extension requirement is valid
      if (!req->second.isValid() || (req->first.isValid() &&
                        (req->first.getId() != AebId::applicationId() || !req->first.meets(AebVersion::appVersion()))))
      {
         continue;
      }
      // if the required extension is not present or does not meet the version requirements then fail
      const Aeb* pExt = iservices->getAeb(req->second.getId());
      if (pExt == NULL || !pExt->validate())
      {
         const Aeb* pPendingExt = iservices->getPendingAebInstall(req->second.getId());
         if (pPendingExt == NULL || !pPendingExt->validate())
         {
            errMsg = "A required extension [" + static_cast<std::string>(req->second.getId()) + "] is not installed.";
            return false;
         }
         else if (!req->second.meets(pPendingExt->getVersion()))
         {
            errMsg = "A required extension [" + pPendingExt->getName() +
               "] is scheduled for install but does not meet the version requirements. [" + pPendingExt->getVersion().toString() + "]";
            return false;
         }
      }
      else if (!req->second.meets(pExt->getVersion()))
      {
         errMsg = "A required extension [" + pExt->getName() +
            "] is present but does not meet the version requirements. [" + pExt->getVersion().toString() + "]";
         return false;
      }
   }
   // validate incompatibles
   for (std::vector<std::pair<AebRequirement, AebRequirement> >::const_iterator inc = mIncompatibles.begin(); inc != mIncompatibles.end(); ++inc)
   {
      // first is the target application...if the is the target application and the extension requirement is valid
      if (!inc->second.isValid() || (inc->first.isValid() &&
                        (inc->first.getId() != AebId::applicationId() || !inc->first.meets(AebVersion::appVersion()))))
      {
         continue;
      }
      // if the incompatible extension is present and meets the version requirements then fail
      const Aeb* pExt = iservices->getAeb(inc->second.getId());
      if (pExt != NULL && pExt->validate() && inc->second.meets(pExt->getVersion()))
      {
         errMsg = "An extension [" + pExt->getName() + "] is installed which is incompatible with this extension.";
         return false;
      }
   }
   return true;
}

bool Aeb::isIncompatible(const Aeb& extension) const
{
   if (!extension.validate())
   {
      return false;
   }
   // validate incompatibles
   for (std::vector<std::pair<AebRequirement, AebRequirement> >::const_iterator inc = mIncompatibles.begin(); inc != mIncompatibles.end(); ++inc)
   {
      // first is the target application...if the is the target application and the extension requirement is valid
      if (!inc->second.isValid() || (inc->first.isValid() &&
                        (inc->first.getId() != AebId::applicationId() || !inc->first.meets(AebVersion::appVersion()))))
      {
         continue;
      }
      // if the incompatible extension is present and meets the version requirements then fail
      if (inc->second.getId() == extension.getId() && inc->second.meets(extension.getVersion()))
      {
          return true;
      }
   }
   return false;
}

bool Aeb::checkTargetApplication(std::string& errMsg) const
{
   errMsg.clear();
   bool appFound = false;
   for (std::vector<AebRequirement>::const_iterator app = mTargetApplication.begin(); app != mTargetApplication.end(); ++app)
   {
      if (app->isValid() && app->getId() == AebId::applicationId())
      {
         appFound = app->meets(AebVersion::appVersion());
         if (!appFound)
         {
            errMsg = "Extension will not install on this version of Opticks.  Extension can only be "
               "installed on Opticks versions between " + app->getMin().toString() +
               " and " + app->getMax().toString() + ". The application version number is " + 
               AebVersion::appVersion().toString() + ".";
         }
         else
         {
            break;
         }
      }
   }
   if (!appFound && errMsg.empty())
   {
      errMsg = "Extension does not list Opticks as a valid target application.";
   }
   return appFound;
}

std::string Aeb::getFilename() const
{
   return mAebFile.absoluteFilePath().toStdString();
}

AebId Aeb::getId() const
{
   return mId;
}

AebVersion Aeb::getVersion() const
{
   return mVersion;
}

std::string Aeb::getName() const
{
   return mName;
}

std::string Aeb::getDescription() const
{
   return mDescription;
}

std::string Aeb::getCreator() const
{
   return mCreator;
}

const std::vector<std::string>& Aeb::getDevelopers() const
{
   return mDevelopers;
}

const std::vector<std::string>& Aeb::getTranslators() const
{
   return mTranslators;
}

const std::vector<std::string>& Aeb::getContributors() const
{
   return mContributors;
}

std::string Aeb::getHomepageURL() const
{
   return mHomepageURL;
}

std::string Aeb::getIconURL() const
{
   return mIconURL;
}

std::vector<std::string> Aeb::getLicenseURLs() const
{
   return mLicenseURLs;
}

bool Aeb::isHidden() const
{
   return mHidden;
}

const std::vector<AebPlatform>& Aeb::getPlatforms() const
{
   return mPlatforms;
}

const std::vector<std::pair<AebRequirement, AebRequirement> >& Aeb::getRequires() const
{
   return mRequires;
}

const std::vector<std::pair<AebRequirement, AebRequirement> >& Aeb::getIncompatibles() const
{
   return mIncompatibles;
}

std::string Aeb::getUpdateURL() const
{
   return mUpdateURL;
}

std::string Aeb::getUpdateKey() const
{
   return mUpdateKey;
}

std::vector<std::string> Aeb::getSplashScreenURLs() const
{
   return mSplashScreenURLs;
}

std::map<std::string, std::string> Aeb::getHelpEntries() const
{
   return mHelpEntries;
}

QPixmap Aeb::getIcon() const
{
   return (mpIcon != NULL) ? *mpIcon : QPixmap();
}

QStringList Aeb::getLicenses() const
{
   return mLicenses;
}

void Aeb::getContentDestinations(const QList<const AebEntry*>& sources, QList<QString>& contentDests) const
{
   Service<ConfigurationSettings> pSettings;
   QDir appHomePath = QString::fromStdString(pSettings->getHome());
   QDir plugInPath = QString::fromStdString(pSettings->getPlugInPath());
   QDir templatePath = QString::fromStdString(ProductView::getSettingTemplatePath()->getFullPathAndName());
   QDir supportPath = QString::fromStdString(pSettings->getSettingSupportFilesPath()->getFullPathAndName());
   QDir wizardPath = QString::fromStdString(pSettings->getSettingWizardPath()->getFullPathAndName());
   QDir extPath = QString::fromStdString(pSettings->getSettingExtensionFilesPath()->getFullPathAndName());

   for (QList<const AebEntry*>::const_iterator iter = sources.begin();
        iter != sources.end();
        ++iter)
   {
      QString filePath = (*iter)->getFilePath();
      QStringList parts = filePath.split("/", QString::SkipEmptyParts);
      if (parts[0] == "content")
      {
         parts.takeFirst();
      }
      else if (parts[0] == "platform")
      {
         parts.takeFirst();
         parts.takeFirst();
      }

      QString top = parts.takeFirst();
      QDir dest;
      if (top == "Bin")
      {
         dest = QDir(QCoreApplication::applicationDirPath());
      }
      else if (top == "Doc" || top == "Help" || top == "DefaultSettings")
      {
         dest = appHomePath;
         dest.setPath(dest.absoluteFilePath(top));
      }
      else if (top == "PlugIns")
      {
         dest = plugInPath;
      }
      else if (top == "Templates")
      {
         dest = templatePath;
      }
      else if (top == "SupportFiles")
      {
         dest = supportPath;
      }
      else if (top == "Wizards")
      {
         dest = wizardPath;
      }
      else
      {
         break;
      }
      contentDests.push_back(dest.absoluteFilePath(parts.join("/")));
   }
}
