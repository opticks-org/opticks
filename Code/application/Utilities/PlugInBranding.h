/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef PLUGINBRANDING_H
#define PLUGINBRANDING_H

#include "FilenameImp.h"

#include <string>

class PlugInBranding
{
public:
   PlugInBranding();
   ~PlugInBranding();

   const Filename* getSplashScreenImage() const;
   void setSplashScreenImage(const Filename* pFilename);
   const std::string& getTitle() const;
   void setTitle(const std::string& title);
   const std::string& getDescription() const;
   void setDescription(const std::string& description);
   const std::string& getVersion() const;
   void setVersion(const std::string& version);
   const std::string& getLicense() const;
   void setLicense(const std::string& license);
   const std::string& getHelpTitle() const;
   void setHelpTitle(const std::string& title);
   const Filename* getHelpWebpage() const;
   void setHelpWebpage(const Filename* pWebpage);

   static const std::vector<PlugInBranding>& getBrandings();

protected:
   FilenameImp mFilename;
   std::string mTitle;
   std::string mDescription;
   std::string mVersion;
   std::string mLicense;
   std::string mHelpTitle;
   FilenameImp mHelpWebpage;
};

#endif
