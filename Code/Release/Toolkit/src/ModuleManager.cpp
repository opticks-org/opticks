/**
 * Module Manager 
 *
 * The Module Manager is used to inform the main application about the
 * Plug-Ins within the Module.  It is also used to create, destroy, and provide
 * access to the Plug-Ins.  Plug-In developers edit this class to build
 * a Plug-In Module composed of their Plug-Ins. This is a singleton class.  
 * Only one instance of this class exists at a given time.  Use the instance() 
 * method to get a reference to the class.
 *
 */

#include "ModuleManager.h"
#include "PlugInFactory.h"
#include <algorithm>
#include <vector>

/**
 * This is deprecated, so the value set does not currently matter.
 */
const char* ModuleManager::mspValidationKey = "None";

/**
 * Set this to the name of your module, ie. "Band Math"
 */
const char* ModuleManager::mspName = "";

/**
 * Set this to the version of your module, ie. "1.0"
 */
const char* ModuleManager::mspVersion = "";

/**
 * Set this to a description of your module, ie. "Use to perform math using the bands of a RasterElement"
 */
const char* ModuleManager::mspDescription = "";

/**
 * Set a unique identifier for your module, see SessionItem::getId() for more details
 */
const char* ModuleManager::mspUniqueId = "";

/**
 * The following code configures ModuleManager to use PlugInFactory.
 *
 * This allows you to register a plug-in by including a macro in the plug-in's cpp file.
 * Further modifications to ModuleManager.cpp are not necessary.
 *
 * @see PlugInFactory
 */
std::vector<PlugInFactory*>& factories()
{
   static std::vector<PlugInFactory*> sFactories;
   return sFactories;
}

unsigned int ModuleManager::getTotalPlugIns()
{
   return static_cast<unsigned int>(factories().size());
}

void addFactory(PlugInFactory* pFactory)
{
   if (pFactory != NULL)
   {
      factories().push_back(pFactory);
   }
}

struct FactoryPtrComparator
{
   bool operator()(PlugInFactory* pLhs, PlugInFactory* pRhs)
   {
      if (pLhs == NULL || pRhs == NULL)
      {
         return false;
      }
      size_t leftCount = std::count(pLhs->mName.begin(), pLhs->mName.end(), '_');
      size_t rightCount = std::count(pRhs->mName.begin(), pRhs->mName.end(), '_');
      if (leftCount != rightCount)
      {
         return leftCount > rightCount;
      }
      return pLhs->mName < pRhs->mName;
   }
};

PlugIn* ModuleManager::getPlugIn(unsigned int plugInNumber)
{
   static bool factoriesSorted = false;
   if (!factoriesSorted)
   {
      factoriesSorted = true;
      FactoryPtrComparator comp;
      std::sort(factories().begin(), factories().end(), comp);
   }
   if (plugInNumber >= factories().size())
   {
      return NULL;
   }
   return (*(factories()[plugInNumber]))();
}
