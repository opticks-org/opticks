/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef AEB_H__
#define AEB_H__

#include "AebId.h"
#include "AebPlatform.h"
#include "AebRequirement.h"
#include "AebVersion.h"

#include <boost/utility.hpp>
#include <map>
#include <string>
#include <vector>

#include <QtCore/QFileInfo>
#include <QtCore/QList>
#include <QtCore/QPair>
#include <QtGui/QPixmap>

class AebEntry;

class Aeb : private boost::noncopyable
{
public:

   Aeb();
   bool validate() const;
   bool meetsRequirements(std::string& errMsg) const;
   bool isIncompatible(const Aeb& extension) const;

   std::string getFilename() const;
   AebId getId() const;
   AebVersion getVersion() const;
   std::string getName() const;

   std::string getDescription() const;
   std::string getCreator() const;
   const std::vector<std::string>& getDevelopers() const;
   const std::vector<std::string>& getTranslators() const;
   const std::vector<std::string>& getContributors() const;
   std::string getHomepageURL() const;
   std::string getIconURL() const;
   QPixmap getIcon() const;
   std::vector<std::string> getLicenseURLs() const; // the length of licenseURLs and
   QStringList getLicenses() const;                 // licenses will be the same in a valid AEB.
   void getContentDestinations(const QList<const AebEntry*>& sources, QList<QString>& contentDests) const;
   bool isHidden() const;
   const std::vector<AebPlatform>& getPlatforms() const;
   const std::multimap<AebRequirement, AebRequirement>& getRequires() const;
   const std::multimap<AebRequirement, AebRequirement>& getIncompatibles() const;
   std::string getUpdateURL() const;
   std::string getUpdateKey() const;

   std::vector<std::string> getSplashScreenURLs() const;
   std::map<std::string, std::string> getHelpEntries() const;

private:
   // AEBL required
   AebId mId;
   AebVersion mVersion;
   std::string mName;
   std::vector<AebRequirement> mTargetApplication;
   
   // AEBL optional
   std::string mDescription;
   std::string mCreator;
   std::vector<std::string> mDevelopers;
   std::vector<std::string> mTranslators;
   std::vector<std::string> mContributors;
   std::string mHomepageURL;
   std::string mIconURL;
   std::vector<std::string> mLicenseURLs;
   bool mHidden;
   std::vector<AebPlatform> mPlatforms;
   std::multimap<AebRequirement, AebRequirement> mRequires; // targetApp to requirement
   std::multimap<AebRequirement, AebRequirement> mIncompatibles; // targetApp to incompatible
   std::string mUpdateURL;
   std::string mUpdateKey;

   // Opticks optional
   std::vector<std::string> mSplashScreenURLs;
   std::map<std::string,std::string> mHelpEntries;

   QPixmap mIcon;
   QStringList mLicenses;
   QFileInfo mAebFile;

   friend class AebIo;
};

class AebListResource
{
public:
   ~AebListResource()
   {
      for (std::vector<Aeb*>::iterator iter = mAebs.begin();
           iter != mAebs.end();
           ++iter)
      {
         delete *iter;
      }
   }
   void push_back(Aeb* pAeb)
   {
      mAebs.push_back(pAeb);
   }
   Aeb* back()
   {
      return mAebs.back();
   }
private:
   std::vector<Aeb*> mAebs;
};

#endif
