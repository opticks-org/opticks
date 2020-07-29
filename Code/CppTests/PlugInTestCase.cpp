/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "Executable.h"
#include "PlugIn.h"
#include "PlugInTestCase.h"
#include "PlugInManagerServicesImp.h"
#include "PlugInArgList.h"

using namespace std;

bool PlugInTestCase::runPlugIn( string fileName, PlugInArgList *pInputArgList, PlugInArgList *pOutputArgList, bool runInBatch )
{
   bool success = false;

   Executable* pExecutable = getPlugIn(fileName, runInBatch);
   if (pExecutable != NULL)
   {
      success = pExecutable->execute(pInputArgList, pOutputArgList);
   }

   return success;
}

Executable* PlugInTestCase::getPlugIn(string fileName, bool runInBatch)
{
   Executable* pExecutable = NULL;

   PlugIn* pPlugIn = PlugInManagerServicesImp::instance()->createPlugIn(fileName);
   if (pPlugIn != NULL)
   {
      pExecutable = dynamic_cast<Executable*>(pPlugIn);
      if (pExecutable != NULL)
      {
         if (runInBatch)
         {
            pExecutable->setBatch();
         }
         else
         {
            pExecutable->setInteractive();
         }
      }
   }

   return pExecutable;
}

PlugInArgList *PlugInTestCase::getInputArgList( string fileName, bool runInBatch )
{
   return getArgList(fileName, runInBatch, &Executable::getInputSpecification);
}

PlugInArgList *PlugInTestCase::getOutputArgList( string fileName, bool runInBatch )
{
   return getArgList(fileName, runInBatch, &Executable::getOutputSpecification);
}

PlugInArgList* PlugInTestCase::getArgList(string fileName, bool runInBatch,
                                          bool (Executable::*getSpecification)(PlugInArgList*&))
{
   PlugInArgList *pArgList = NULL;

   Executable* pExecutable = getPlugIn(fileName, runInBatch);
   if (pExecutable != NULL)
   {
      (pExecutable->*getSpecification)(pArgList);
   }

   return pArgList;
}
