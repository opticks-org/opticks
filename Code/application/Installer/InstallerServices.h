/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef INSTALLERSERVICES_H__
#define INSTALLERSERVICES_H__

#include "Aeb.h"
#include "Service.h"
#include <list>
#include <map>
#include <string>

class Progress;
class ReferenceCountDatabase;

class InstallerServices
{
public:
   virtual bool installExtension(const std::string& aebFile, Progress* pProgress = NULL) = 0;
   virtual bool uninstallExtension(const std::string& extensionId, std::string& errMsg) = 0;
   virtual void setPendingInstall(const std::vector<std::string>& aebFilenames = std::vector<std::string>()) = 0;
   virtual std::list<const Aeb*> getPendingInstall() const = 0;
   virtual const Aeb* getPendingAebInstall(const std::string& aebId) const = 0;
   virtual const Aeb* getAeb(const std::string& aebId) const = 0;
   virtual std::list<const Aeb*> getAebs() const = 0;
   virtual bool processPending(Progress* pProgress = NULL) = 0;
   virtual std::vector<std::string> getSplashScreenPaths() const = 0;
   virtual std::map<std::string, std::string> getHelpEntries() const = 0;
   virtual ReferenceCountDatabase& getReferenceCountDatabase() = 0;
   virtual bool isPendingUninstall(const AebId& extension) const = 0;

protected:
   virtual ~InstallerServices() {};
};

template<> InstallerServices* Service<InstallerServices>::get() const;

#endif
