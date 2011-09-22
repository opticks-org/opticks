/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AebIo.h"
#include "ConfigurationSettings.h"
#include "InstallerServicesImp.h"
#include "ObjectResource.h"
#include "Progress.h"
#include "Transaction.h"
#include "TransactionLog.h"
#include "TypeConverter.h"

#include <memory>

#include <QtCore/QDir>
#include <QtCore/QDirIterator>
#include <QtCore/QSet>
#include <QtCore/QStack>
#include <QtCore/QUrl>

InstallerServicesImp* InstallerServicesImp::spInstance = NULL;
bool InstallerServicesImp::sDestroyed = false;

template<>
InstallerServices* Service<InstallerServices>::get() const
{
   return InstallerServicesImp::instance();
}

InstallerServicesImp* InstallerServicesImp::instance()
{
   if (spInstance == NULL)
   {
      if (sDestroyed)
      {
         throw std::logic_error("Attempting to use InstallerServices after destroying it.");
      }
      spInstance = new InstallerServicesImp();
   }
   return spInstance;
}

void InstallerServicesImp::destroy()
{
   if (sDestroyed)
   {
      throw std::logic_error("Attempting to destroy InstallerServices after destroying it.");
   }
   delete spInstance;
   spInstance = NULL;
   sDestroyed = true;
}

InstallerServicesImp::InstallerServicesImp()
{
   QDir extensionDir(QString::fromStdString(Service<ConfigurationSettings>()->getSettingExtensionFilesPath()->getFullPathAndName()));
   QStringList subDirs = extensionDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
   foreach (QString subDir, subDirs)
   {
      QDir dir = extensionDir;
      dir.cd(subDir);
      bool needsRemoval = false;
      if (!dir.entryList(QStringList() << "install.rdf").isEmpty())
      {
         std::auto_ptr<Aeb> pExtension(new Aeb());
         AebIo io(*pExtension.get());
         std::string errMsg; // ignored but must be specified
         if (io.fromFile(dir.absoluteFilePath("install.rdf").toStdString(), errMsg) && pExtension->validate())
         {
            AebId id = pExtension->getId();
            mExtensions.insert(std::make_pair(id, pExtension.release()));
         }
         else
         {
            needsRemoval = true;
         }
      }
      else if (!dir.entryInfoList(QStringList() << "transactionlog.txt").isEmpty())
      {
         needsRemoval = true;
      }
      if (needsRemoval)
      {
         // there was a crash during install, uninstall this extension
         QFile pending(extensionDir.absoluteFilePath("pendinguninstall"));
         if (pending.open(QFile::WriteOnly | QFile::Append | QFile::Text))
         {
            pending.write(subDir.toAscii());
            pending.write("\n");
            pending.close();
         }
      }
   }
}

InstallerServicesImp::~InstallerServicesImp()
{
   for (std::map<AebId, Aeb*>::const_iterator iter = mExtensions.begin();
        iter != mExtensions.end();
        ++iter)
   {
      delete iter->second;
   }
   for (std::map<AebId, Aeb*>::const_iterator iter = mPendingInstall.begin();
        iter != mPendingInstall.end();
        ++iter)
   {
      delete iter->second;
   }
}

bool InstallerServicesImp::installExtension(const std::string& aebFile, Progress* pProgress)
{
   std::auto_ptr<Aeb> pExtension(new Aeb());
   AebIo io(*pExtension.get());
   std::string errMsg;
   bool success = io.fromFile(aebFile, errMsg);
   if (!success)
   {
      if (pProgress != NULL)
      {
         pProgress->updateProgress("Unable to load AEB. " + errMsg, 0, ERRORS);
      }
      return false;
   }

   // ensure it's a valid extension and all requirements are met
   if (mExtensions.find(pExtension->getId()) != mExtensions.end())
   {
      if (pProgress != NULL)
      {
         pProgress->updateProgress("This extension is already installed.", 0, ERRORS);
      }
      return false;
   }
   if (!pExtension->validate())
   {
      if (pProgress != NULL)
      {
         pExtension->checkTargetApplication(errMsg); 
         pProgress->updateProgress("This extension is invalid. " + errMsg, 0, ERRORS);
      }
      return false;
   }
   errMsg.clear();
   if (!pExtension->meetsRequirements(errMsg))
   {
      if (pProgress != NULL)
      {
         pProgress->updateProgress(errMsg, 0, ERRORS);
      }
      return false;
   }

   // ensure this will not invalidate any existing extensions
   for (std::map<AebId, Aeb*>::const_iterator ext = mExtensions.begin(); ext != mExtensions.end(); ++ext)
   {
      if (ext->second->isIncompatible(*pExtension.get()))
      {
         if (pProgress != NULL)
         {
            pProgress->updateProgress("An installed extension [" + ext->second->getName() + "] is incompatible.", 0, ERRORS);
         }
         return false;
      }
   }

   // perform the install   
   QDir extensionDir = getExtensionDir(*pExtension.get());
   TransactionLog log(pProgress);
   errMsg.clear();
   const QList<const AebEntry*>& contentPaths = io.getContentPaths(errMsg);
   if (!errMsg.empty())
   {
      if (pProgress != NULL)
      {
         pProgress->updateProgress(errMsg, 0, ERRORS);
      }
      return false;
   }
   QList<QString> contentDests;
   pExtension->getContentDestinations(contentPaths, contentDests);
   if (contentDests.empty())
   {
      //this is only empty if one or more of the content paths
      //was invalid for an Aeb
      if (pProgress != NULL)
      {
         pProgress->updateProgress("The extension contains an invalid content path.", 0, ERRORS);
      }
      return false;
   }
   QList<const AebEntry*>::const_iterator sourceIter;
   QList<QString>::const_iterator destIter;
   for (sourceIter = contentPaths.begin(),
       destIter = contentDests.begin();
       sourceIter != contentPaths.end() && destIter != contentDests.end();
       ++sourceIter, ++destIter)
   {
      std::auto_ptr<Transaction> pTransaction(new Transaction());
      pTransaction->setSource(*sourceIter);
      pTransaction->setAebIo(&io);
      pTransaction->setDestination((*destIter).toStdString());
      if (!log.addTransaction(pTransaction.get()))
      {
         if (pProgress != NULL)
         {
            pProgress->updateProgress("Unable to install a file from the extension.", 0, ERRORS);
         }
         return false;
      }
      pTransaction.release();
   }
   // now add icon and license files
   QString iconPath = QString::fromStdString(pExtension->getIconURL());
   if (!iconPath.isEmpty())
   {
      std::auto_ptr<Transaction> pTransaction(new Transaction());
      QUrl iconUrl(iconPath);
      const AebEntry* pIcon = io.getEntry(iconUrl);
      if (pIcon == NULL)
      {
         if (pProgress != NULL)
         {
            pProgress->updateProgress("Invalid icon file.", 0, ERRORS);
         }
         return false;
      }
      pTransaction->setSource(pIcon);
      pTransaction->setAebIo(&io);
      pTransaction->setDestination(extensionDir.absoluteFilePath(iconUrl.path().remove(0,1)).toStdString());
      if (!log.addTransaction(pTransaction.get()))
      {
         if (pProgress != NULL)
         {
            pProgress->updateProgress("Unable to install a file from the extension.", 0, ERRORS);
         }
         return false;
      }
      pTransaction.release();
   }
   std::vector<std::string> licenseURLs = pExtension->getLicenseURLs();
   for (std::vector<std::string>::const_iterator licenseIter = licenseURLs.begin(); licenseIter != licenseURLs.end(); ++licenseIter)
   {
      QUrl licenseURL(QString::fromStdString(*licenseIter));
      std::auto_ptr<Transaction> pTransaction(new Transaction());
      const AebEntry* pLicense = io.getEntry(licenseURL);
      if (pLicense == NULL)
      {
         if (pProgress != NULL)
         {
            pProgress->updateProgress("Invalid license file.", 0, ERRORS);
         }
         return false;
      }
      pTransaction->setSource(pLicense);
      pTransaction->setAebIo(&io);
      pTransaction->setDestination(extensionDir.absoluteFilePath(licenseURL.path().remove(0, 1)).toStdString());
      if (!log.addTransaction(pTransaction.get()))
      {
         if (pProgress != NULL)
         {
            pProgress->updateProgress("Unable to install a file from the extension.", 0, ERRORS);
         }
         return false;
      }
      pTransaction.release();
   }
   std::vector<std::string> splashScreenURLs = pExtension->getSplashScreenURLs();
   for (std::vector<std::string>::const_iterator splashScreenIter = splashScreenURLs.begin();
      splashScreenIter != splashScreenURLs.end(); ++splashScreenIter)
   {
      QUrl splashScreenURL(QString::fromStdString(*splashScreenIter));
      std::auto_ptr<Transaction> pTransaction(new Transaction());
      const AebEntry* pSplashScreen = io.getEntry(splashScreenURL);
      if (pSplashScreen == NULL)
      {
         if (pProgress != NULL)
         {
            pProgress->updateProgress("Invalid splash screen.", 0, ERRORS);
         }
         return false;
      }
      pTransaction->setSource(pSplashScreen);
      pTransaction->setAebIo(&io);
      QString path = extensionDir.absoluteFilePath(splashScreenURL.path().remove(0, 1));
      pTransaction->setDestination(path.toStdString());
      if (!log.addTransaction(pTransaction.get()))
      {
         if (pProgress != NULL)
         {
            pProgress->updateProgress("Unable to install a file from the extension.", 0, ERRORS);
         }
         return false;
      }
      pTransaction.release();
   }
   if (!extensionDir.exists())
   {
      if (pProgress != NULL)
      {
         pProgress->updateProgress("Unable to register the extension. Check write permission on the Opticks installation directories.",
            0, ERRORS);
      }
      return false;
   }
   if (!log.execute(extensionDir.absoluteFilePath("transactionlog.txt").toStdString(), "Installing " + pExtension->getName()))
   {
      log.rollback();
      extensionDir.remove("transactionlog.txt");
      extensionDir.rmdir(".");
      if (pProgress != NULL)
      {
         pProgress->updateProgress("Error installing the extension.", 0, ERRORS);
      }
      return false;
   }
   if (!io.toFile(extensionDir.absoluteFilePath("install.rdf").toStdString()))
   {
      log.rollback();
      extensionDir.remove("install.rdf");
      extensionDir.remove("transactionlog.txt");
      extensionDir.rmdir(".");
      if (pProgress != NULL)
      {
         pProgress->updateProgress("Error writing the extension metadata file.", 0, ERRORS);
      }
      return false;
   }
   mExtensions.insert(std::make_pair(pExtension->getId(), pExtension.get()));
   pExtension.release();
   return true;
}

bool InstallerServicesImp::uninstallExtension(const std::string& extensionId, std::string& errMsg)
{
   if (mExtensions.find(extensionId) == mExtensions.end())
   {
      errMsg = "Extension is not registered as installed.";
      return false;
   }
   if (mPendingUninstall.find(extensionId) != mPendingUninstall.end())
   {
      return true;
   }

   // check requires to ensure they are all uninstalling as well
   Aeb* pAeb = mExtensions[extensionId];
   for (std::map<AebId, Aeb*>::const_iterator checkAeb = mExtensions.begin(); checkAeb != mExtensions.end(); ++checkAeb)
   {
      if (checkAeb->second == pAeb || mPendingUninstall.find(checkAeb->first) != mPendingUninstall.end())
      {
         continue;
      }
      const std::vector<std::pair<AebRequirement, AebRequirement> >& reqs = checkAeb->second->getRequires();
      for (std::vector<std::pair<AebRequirement, AebRequirement> >::const_iterator req = reqs.begin(); req != reqs.end(); ++req)
      {
         if ((!req->first.isValid() || req->first.meets(AebVersion::appVersion()))
               && req->second.getId() == pAeb->getId() && req->second.meets(pAeb->getVersion()))
         {
            errMsg = "Another extension [" + checkAeb->second->getName() + "] requires this extension. Remove the other first.";
            return false;
         }
      }
   }

   QDir extensionDir(QString::fromStdString(Service<ConfigurationSettings>()->getSettingExtensionFilesPath()->getFullPathAndName()));
   QFile pending(extensionDir.absoluteFilePath("pendinguninstall"));
   if (!pending.open(QFile::WriteOnly | QFile::Append | QFile::Text))
   {
      errMsg = "Unable to register the uninstall. Check write permission on the Opticks installation directories."; 
      return false;
   }
   pending.write(extensionId.c_str());
   pending.write("\n");
   pending.close();
   mPendingUninstall.insert(extensionId);
   return true;
}  

bool InstallerServicesImp::performUninstall(const std::string& extensionId, Progress* pProgress)
{
   if (extensionId.empty())
   {
      // invalid extension id, probably an empty line in the performuninstall file
      return true;
   }
   std::map<AebId, Aeb*>::iterator extension = mExtensions.find(extensionId);
   TransactionLog log(pProgress);

   QDir extensionDir;
   if (extension != mExtensions.end())
   {
      extensionDir = getExtensionDir(*(extension->second));
   }
   else
   {
      extensionDir = QDir(QString::fromStdString(Service<ConfigurationSettings>()->getSettingExtensionFilesPath()->getFullPathAndName()));
      extensionDir.cd(QString::fromStdString(extensionId));
      if (!extensionDir.exists())
      {
         // already uninstalled
         return true;
      }
   }

   if (!log.deserialize(extensionDir.absoluteFilePath("transactionlog.txt").toStdString()))
   {
      if (pProgress != NULL)
      {
         pProgress->updateProgress("Invalid transaction log.", 0, ERRORS);
      }
      return false;
   }
   std::string rollbackMessage;
   if (extension != mExtensions.end())
   {
      rollbackMessage = "Uninstalling extension " + extension->second->getName();
   }
   else
   {
      // partially installed extension...probably an app crash during install
      rollbackMessage = "Recovering from partially installed extension " + extensionId;
   }
   if (!log.rollback(rollbackMessage))
   {
      if (pProgress != NULL)
      {
         pProgress->updateProgress("Unspecified error removing extension. Check write permission on the Opticks installation directories.",
            0, ERRORS);
      }
      return false;
   }
   {
      QDirIterator dirit(extensionDir.absolutePath(), QDir::Files, QDirIterator::Subdirectories);
      while (dirit.hasNext())
      {
         QFile::remove(dirit.next());
      }
   }
   {
      QStack<QString> toRemove;
      QDirIterator dirit(extensionDir.absolutePath(), QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
      while (dirit.hasNext())
      {
         toRemove.push(dirit.next());
      }
      while (!toRemove.empty())
      {
         QDir().rmdir(toRemove.top());
         toRemove.pop();
      }
   }

   // remove the extension's folder
   QString extFolder = extensionDir.canonicalPath();
   extensionDir.cdUp();
   extensionDir.rmdir(extFolder);

   if (extension != mExtensions.end())
   {
      delete extension->second;
      mExtensions.erase(extension);
   }
   return true;
}

void InstallerServicesImp::setPendingInstall(const std::vector<std::string>& aebFilenames)
{
   for (std::map<AebId, Aeb*>::const_iterator iter = mPendingInstall.begin();
        iter != mPendingInstall.end();
        ++iter)
   {
      delete iter->second;
   }
   mPendingInstall.clear();
   for (std::vector<std::string>::const_iterator aebIter = aebFilenames.begin();
        aebIter != aebFilenames.end();
        ++aebIter)
   {
      Resource<Aeb> pExtension;
      AebIo io(*pExtension.get());
      std::string errMsg;
      bool success = io.fromFile(*aebIter, errMsg);
      if (success && pExtension->validate())
      {
         pExtension.release();
         mPendingInstall.insert(std::make_pair(pExtension->getId(), pExtension.get()));
      }
   }
}

std::list<const Aeb*> InstallerServicesImp::getPendingInstall() const
{
   std::list<const Aeb*> aebs;
   for (std::map<AebId, Aeb*>::const_iterator iter = mPendingInstall.begin();
        iter != mPendingInstall.end();
        ++iter)
   {
      aebs.push_back(iter->second);
   }
   return aebs;
}

const Aeb* InstallerServicesImp::getPendingAebInstall(const std::string& aebId) const
{
   AebId id(aebId);
   std::map<AebId, Aeb*>::const_iterator iter = mPendingInstall.find(id);
   if (iter == mPendingInstall.end())
   {
      return NULL;
   }
   return iter->second;
}

const Aeb* InstallerServicesImp::getAeb(const std::string& aebId) const
{
   AebId id(aebId);
   std::map<AebId, Aeb*>::const_iterator iter = mExtensions.find(id);
   if (iter == mExtensions.end())
   {
      return NULL;
   }
   return iter->second;
}

std::list<const Aeb*> InstallerServicesImp::getAebs() const
{
   std::list<const Aeb*> aebs;
   for (std::map<AebId, Aeb*>::const_iterator iter = mExtensions.begin();
        iter != mExtensions.end();
        ++iter)
   {
      aebs.push_back(iter->second);
   }
   return aebs;
}

QDir InstallerServicesImp::getExtensionDir(const Aeb& extension) const
{
   QDir extensionDir(QString::fromStdString(Service<ConfigurationSettings>()->getSettingExtensionFilesPath()->getFullPathAndName()));
   QString id = QString::fromStdString(extension.getId());
   if (!extensionDir.exists(id))
   {
      extensionDir.mkdir(id);
   }
   extensionDir.cd(id);
   return extensionDir;
}

bool InstallerServicesImp::processPending(Progress* pProgress)
{
   QDir extensionDir(QString::fromStdString(Service<ConfigurationSettings>()->getSettingExtensionFilesPath()->getFullPathAndName()));
   QFile pending(extensionDir.absoluteFilePath("pendinguninstall"));
   if (!pending.open(QFile::ReadOnly | QFile::Text))
   {
      return true;
   }
   QByteArray bytes = pending.readAll();
   QStringList extensionIds = QString(bytes).split("\n");
   pending.close();

   bool success = true;
   foreach (QString extensionId, extensionIds)
   {
      if (extensionId == "+all")
      {
         // uninstall all extensions
         while (success && !mExtensions.empty())
         {
            success = performUninstall(mExtensions.begin()->first, pProgress);
         }
         break;
      }
      success = performUninstall(extensionId.toStdString(), pProgress);
   }
   success = success && pending.remove();

   return success;
}

std::vector<std::string> InstallerServicesImp::getSplashScreenPaths() const
{
   std::vector<std::string> paths;
   for (std::map<AebId, Aeb*>::const_iterator iter = mExtensions.begin();
        iter != mExtensions.end();
        ++iter)
   {
      const Aeb& extension = *(iter->second);
      QDir extensionDir = getExtensionDir(extension);
      std::vector<std::string> urls = extension.getSplashScreenURLs();
      for (std::vector<std::string>::const_iterator url = urls.begin(); url != urls.end(); ++url)
      {
         QUrl splashUrl(QString::fromStdString(*url));
         QString path = extensionDir.absoluteFilePath(splashUrl.path().remove(0,1));
         paths.push_back(path.toStdString());
      }
   }
   return paths;
}

std::map<std::string, std::string> InstallerServicesImp::getHelpEntries() const
{
   std::map<std::string, std::string> entries;
   for (std::map<AebId, Aeb*>::const_iterator iter = mExtensions.begin();
        iter != mExtensions.end();
        ++iter)
   {
      const Aeb& extension = *(iter->second);
      QDir extensionDir = getExtensionDir(extension);
      std::map<std::string, std::string> helpEntries = extension.getHelpEntries();
      for (std::map<std::string, std::string>::const_iterator helpEntry = helpEntries.begin();
               helpEntry != helpEntries.end(); ++helpEntry)
      {
         QString dest;
         QUrl destUrl(QString::fromStdString(helpEntry->second));
         if (destUrl.scheme() == "aeb")
         {
            QStringList parts = destUrl.path().split("/", QString::SkipEmptyParts);
            if (parts.size() > 1 && parts.front() == "Help")
            {
               QDir helpHome(QString::fromStdString(Service<ConfigurationSettings>()->getHome()));
               dest = helpHome.absoluteFilePath(parts.join("/"));
            }
            else
            {
               dest = extensionDir.absoluteFilePath(destUrl.path().remove(0,1));
            }
         }
         else if (destUrl.scheme() == "file")
         {
            dest = extensionDir.absoluteFilePath(destUrl.path());
         }
         else
         {
            dest = destUrl.toString();
         }
         entries[helpEntry->first] = dest.toStdString();
      }
   }
   return entries;
}

bool InstallerServicesImp::isPendingUninstall(const AebId& extension) const
{
   return mPendingUninstall.find(extension) != mPendingUninstall.end();
}
