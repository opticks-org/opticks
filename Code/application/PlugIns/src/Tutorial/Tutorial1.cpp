/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "Progress.h"
#include "Tutorial1.h"

REGISTER_PLUGIN_BASIC(OpticksTutorial, Tutorial1);

Tutorial1::Tutorial1()
{
   setDescriptorId("{5D8F4DD0-9B20-42B1-A060-589DFBC85D00}");
   setName("Tutorial 1");
   setDescription("Creating your first plug-in.");
   setCreator("Opticks Community");
   setVersion("Sample");
   setCopyright("Copyright (C) 2008, Ball Aerospace & Technologies Corp.");
   setProductionStatus(false);
   setType("Sample");
   setMenuLocation("[Tutorial]/Tutorial 1");
   setAbortSupported(false);
}

Tutorial1::~Tutorial1()
{
}

bool Tutorial1::getInputSpecification(PlugInArgList*& pInArgList)
{
   VERIFY(pInArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pInArgList->addArg<Progress>(Executable::ProgressArg(), NULL, "Progress reporter");
   return true;
}

bool Tutorial1::getOutputSpecification(PlugInArgList*& pOutArgList)
{
   pOutArgList = NULL;
   return true;
}

bool Tutorial1::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   if (pInArgList == NULL)
   {
      return false;
   }
   Progress* pProgress = pInArgList->getPlugInArgValue<Progress>(Executable::ProgressArg());
   if (pProgress != NULL)
   {
      pProgress->updateProgress("This demonstrates display of a warning.", 0, WARNING);
      pProgress->updateProgress("This demonstrates display of an error.", 0, ERRORS);
      pProgress->updateProgress("Hello World!", 100, NORMAL);
   }
   return true;
}
