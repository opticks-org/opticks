/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#include "HdfPager.h"

#include "AppVerify.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"

using namespace std;

const int HdfPager::INVALID_HANDLE = -1;

HdfPager::HdfPager()
{
}

bool HdfPager::getInputSpecification(PlugInArgList*& pIn)
{
   Service<PlugInManagerServices> pPlugInManager;
   VERIFY(pPlugInManager.get() != NULL);
   bool success = CachedPager::getInputSpecification(pIn);
   if (success && pIn != NULL)
   {
      PlugInArg* pArg = pPlugInManager->getPlugInArg();
      VERIFY(pArg != NULL);
      pArg->setName("HDF Name");
      pArg->setType("string");
      pArg->setDefaultValue(NULL);
      pIn->addArg(*pArg);
   }
   else
   {
      pPlugInManager->destroyPlugInArgList(pIn);
      pIn = NULL;
      success = false;
   }
   return success;
}

bool HdfPager::parseInputArgs(PlugInArgList* pIn)
{
   CachedPager::parseInputArgs(pIn);
   bool success = false;
   if (pIn != NULL)
   {
      PlugInArg* pArg = NULL;
      if (pIn->getArg("HDF Name", pArg) && pArg != NULL)
      {
         string* pValue = reinterpret_cast<string*>(pArg->getActualValue());
         if (pValue != NULL)
         {
            mHdfName = *pValue;
            success = true;
         }
      }
   }
   return success;
}
