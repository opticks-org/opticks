/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef INSTALLERSERVICESIMP_H__
#define INSTALLERSERVICESIMP_H__

#include "Aeb.h"
#include "InstallerServices.h"
#include "ReferenceCountDatabase.h"
#include <QtCore/QString>
#include <set>

class Progress;

class InstallerServicesImp : public InstallerServices
{
public:
   static InstallerServicesImp* instance();
   static void destroy();

   virtual bool installExtension(const std::string& aebFile, Progress* pProgress = NULL);
   virtual bool uninstallExtension(const std::string& extensionId, std::string& errMsg);
   virtual void setPendingInstall(const std::vector<std::string>& aebFilenames = std::vector<std::string>());
   virtual std::list<const Aeb*> getPendingInstall() const;
   virtual const Aeb* getPendingAebInstall(const std::string& aebId) const;
   virtual const Aeb* getAeb(const std::string& aebId) const;
   virtual std::list<const Aeb*> getAebs() const;
   virtual bool processPending(Progress* pProgress = NULL);
   virtual std::vector<std::string> getSplashScreenPaths() const;
   virtual std::map<std::string, std::string> getHelpEntries() const;
   virtual ReferenceCountDatabase& getReferenceCountDatabase() { return mRefCount; }
   virtual bool isPendingUninstall(const AebId& extension) const;

   QDir getExtensionDir(const Aeb& extension) const;

protected:
   InstallerServicesImp();
   virtual ~InstallerServicesImp();

private:
   bool performUninstall(const std::string& extensionId, Progress* pProgress);

   std::map<AebId, Aeb*> mExtensions;
   std::map<AebId, Aeb*> mPendingInstall;
   ReferenceCountDatabase mRefCount;
   std::set<AebId> mPendingUninstall;

   static InstallerServicesImp* spInstance;
   static bool sDestroyed;
};

#endif