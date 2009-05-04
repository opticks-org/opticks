/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "CustomMenuPlugIn.h"
#include "FileFinder.h"
#include "Filename.h"
#include "ObjectResource.h"
#include "PlugInRegistration.h"

class CustomMenuPlugInFactory : public DynamicPlugInFactory
{
public:
   void init(bool locate, std::vector<std::string>& dynamicPlugIns)
   {      
      Service<ConfigurationSettings> pSettings;
      std::string searchPath = pSettings->getPlugInPath();
      
      FactoryResource<FileFinder> pFinder;
      pFinder->findFile(searchPath, "*.customplugin", false);
      
      bool foundPlugIn = pFinder->findNextFile();
      std::string plugIn;
      foundPlugIn = foundPlugIn && pFinder->getFileTitle(plugIn);
      while (foundPlugIn)
      {
         std::string fullPath;
         pFinder->getFullPath(fullPath);
         mPlugIns[plugIn] = fullPath;
         if (locate)
         {
            dynamicPlugIns.push_back(plugIn);
         }
         foundPlugIn = pFinder->findNextFile();
         foundPlugIn = foundPlugIn && pFinder->getFileTitle(plugIn);
      }
   }
   PlugIn* createPlugIn(const std::string& name)
   {
      if (mPlugIns.find(name) != mPlugIns.end())
      {
         return new CustomMenuPlugIn(name, mPlugIns[name]);
      }
      return NULL;
   }
private:
   std::map<std::string, std::string> mPlugIns;
};

REGISTER_DYNAMIC_MODULE(OpticksPlugInSampler, CustomMenuPlugInFactory);