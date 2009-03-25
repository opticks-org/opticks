/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */
 
#include "AppConfig.h"
#include "GenericHdf5Importer.h"
#include "HyperionImporter.h"
#include "ModuleManager.h"

// required to include by Hdf5Pager.h
#include <hdf5.h>

// required to include by Hdf4Pager.h
#ifdef HDF4_SUPPORT
#include <hdf.h>
#endif

#include "Hdf4Pager.h"
#include "Hdf5Pager.h"

const char* ModuleManager::mspName = "HDF";
const char* ModuleManager::mspVersion = "1.0.0";
const char* ModuleManager::mspDescription = "Hierarchical Data Format (HDF) PlugIns";
const char* ModuleManager::mspValidationKey = "none";
const char* ModuleManager::mspUniqueId = "{A213B97A-2170-44c6-BCA8-8C5082ADBB7F}";

unsigned int ModuleManager::getTotalPlugIns()
{
#if defined(HDF4_SUPPORT)
   return 4;
#else
   return 2;
#endif
}

PlugIn* ModuleManager::getPlugIn(unsigned int plugInNumber)
{
   PlugIn* pPlugIn = NULL;
   switch (plugInNumber)
   {
   case 0:
      pPlugIn = new GenericHdf5Importer();
      break;

   case 1:
      pPlugIn = new Hdf5Pager();
      break;

#if defined(HDF4_SUPPORT)
   case 2:
      pPlugIn = new HyperionImporter();
      break;

   case 3:
      pPlugIn = new Hdf4Pager();
      break;
#endif

   default:
      break;
   }

   return pPlugIn;
}
