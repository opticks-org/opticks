/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ModuleManager.h"
#include "PlugInFactory.h"
#include <algorithm>
#include <vector>

const char* ModuleManager::mspName = "ConvolutionFilter";
const char* ModuleManager::mspVersion = "1";
const char* ModuleManager::mspDescription = "Convolve an image kernel with a dataset.";
const char* ModuleManager::mspValidationKey = "none";
const char* ModuleManager::mspUniqueId = "{3C948946-4C15-4C2F-A500-492E32A402F2}";

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
