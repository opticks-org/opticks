/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"
#include "AppVerify.h"
#include "MessageLogResource.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "PlugInResource.h"
#include "Progress.h"
#include "StringUtilities.h"
#include "Tutorial2.h"
#ifdef WIN_API
#include <windows.h>
#else
#include <unistd.h>
#endif

REGISTER_PLUGIN_BASIC(OpticksTutorial, Tutorial2);

Tutorial2::Tutorial2()
{
   setDescriptorId("{1F6316EB-8A03-4034-8B3D-31FADB7AF727}");
   setName("Tutorial 2");
   setDescription("Using resources and services.");
   setCreator("Opticks Community");
   setVersion("Sample");
   setCopyright("Copyright (C) 2008, Ball Aerospace & Technologies Corp.");
   setProductionStatus(false);
   setType("Sample");
   setMenuLocation("[Tutorial]/Tutorial 2");
   setAbortSupported(false);
   allowMultipleInstances(true);
}

Tutorial2::~Tutorial2()
{
}

bool Tutorial2::getInputSpecification(PlugInArgList*& pInArgList)
{
   VERIFY(pInArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pInArgList->addArg<Progress>(Executable::ProgressArg(), NULL, "Progress reporter");
   pInArgList->addArg<int>("Count", 5, "How many times should the plug-in recurse?");
   pInArgList->addArg<int>("Depth", 1, "This is the recursive depth. Not usually set by the initial caller.");
   return true;
}

bool Tutorial2::getOutputSpecification(PlugInArgList*& pOutArgList)
{
   pOutArgList = NULL;
   return true;
}

bool Tutorial2::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Tutorial 2", "app", "A8FEFCB3-5D08-4670-B47E-CC533A932737");
   if (pInArgList == NULL)
   {
      return false;
   }
   Progress* pProgress = pInArgList->getPlugInArgValue<Progress>(Executable::ProgressArg());
   int count;
   int depth;
   pInArgList->getPlugInArgValue("Count", count);
   pInArgList->getPlugInArgValue("Depth", depth);
   if (count < 1 || count >= 10 || depth < 1 || depth >= 10)
   {
      std::string msg = "Count and depth must be between 1 and 9 inclusive.";
      pStep->finalize(Message::Failure, msg);
      if (pProgress != NULL)
      {
         pProgress->updateProgress(msg, 0, ERRORS);
      }

      return false;
   }
   if (depth > count)
   {
      std::string msg = "Depth must be <= count";
      pStep->finalize(Message::Failure, msg);
      if (pProgress != NULL)
      {
         pProgress->updateProgress(msg, 0, ERRORS);
      }

      return false;
   }
   pStep->addProperty("Depth", depth);

   if (pProgress != NULL)
   {
      pProgress->updateProgress("Tutorial 2: Count " + StringUtilities::toDisplayString(count)
         + " Depth " + StringUtilities::toDisplayString(depth), depth * 100 / count, NORMAL);
   }
   // sleep to simulate some work
#ifdef WIN_API
   _sleep(1000);
#else
   sleep(1);
#endif

   if (depth < count)
   {
      ExecutableResource pSubCall("Tutorial 2", std::string(), pProgress, false);
      VERIFY(pSubCall->getPlugIn() != NULL);
      pSubCall->getInArgList().setPlugInArgValue("Count", &count);
      pSubCall->getInArgList().setPlugInArgValue("Depth", &++depth);
      if (!pSubCall->execute())
      {
         // sub-call has already posted an error message
         return false;
      }
   }

   pStep->finalize();
   return true;
}
