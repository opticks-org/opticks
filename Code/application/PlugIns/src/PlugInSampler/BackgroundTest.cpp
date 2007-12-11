/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "BackgroundTest.h"
#include "bthread.h"
#include "AppConfig.h"
#include "AppVerify.h"
#include "AppVersion.h"
#include "DesktopServices.h"
#include "MessageLogResource.h"
#include "ModuleManager.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"

#include <string>

#if defined(WIN_API)
#include <windows.h>
#elif defined(UNIX_API)
#include <unistd.h>
#else
#error "Platform Unknown."
#endif

using namespace std;

BackgroundTest::BackgroundTest() :
      mInteractive(false),
      mAbort(false),
      mpProgress(NULL),
      mpWorkerThread(NULL)
{
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setName("Background Execution Test");
   setDescription("Demonstrates creation and cleanup of a background execution plug-in.");
   setShortDescription("Background execution demonstration and test");
   setMenuLocation("[Demo]/Background Test");
   setProductionStatus(false);
   setDescriptorId("{D5A8A298-FF83-4fcf-A059-0EAC7AB18E42}");
   allowMultipleInstances(true);
   destroyAfterExecute(false);
}

BackgroundTest::~BackgroundTest()
{
}

bool BackgroundTest::setBatch()
{
   mInteractive = false;
   return true;
}

bool BackgroundTest::setInteractive()
{
   mInteractive = true;
   return true;
}

bool BackgroundTest::getInputSpecification(PlugInArgList*& pArgList)
{
   pArgList = mpPlugInManager->getPlugInArgList();
   VERIFY(pArgList != NULL);

   PlugInArg *pArg = mpPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName(ProgressArg());
   pArg->setType("Progress");
   pArg->setDefaultValue(NULL);
   pArgList->addArg(*pArg);

   return true;
}

bool BackgroundTest::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = mpPlugInManager->getPlugInArgList();
   VERIFY(pArgList != NULL);

   PlugInArg *pArg = mpPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName(ProgressArg());
   pArg->setType("Progress");
   pArg->setDefaultValue(NULL);
   pArgList->addArg(*pArg);

   return true;
}

bool BackgroundTest::execute(PlugInArgList* pInputArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Run background test plug-in", "app", "B75359A2-7053-4d11-BA89-63CEC140E5C6");
   VERIFY((pInputArgList != NULL) && (pOutArgList != NULL));

   // Get the input argument
   //
   PlugInArg *pArg = NULL;
   VERIFY(pInputArgList->getArg(ProgressArg(), pArg) && (pArg != NULL));
   if(pArg->isActualSet())
   {
      mpProgress = static_cast<Progress*>(pArg->getActualValue());
   }
   else if(pArg->isDefaultSet())
   {
      mpProgress = static_cast<Progress*>(pArg->getDefaultValue());
   }
   else
   {
      string msg = "No value set for Progress";
      if(mpProgress != NULL)
      {
         mpProgress->updateProgress(msg, 0, ERRORS);
      }
      pStep->finalize(Message::Failure, msg);
      mpProgress = NULL;
      return false;
   }
   if(mpProgress != NULL)
   {
      mpProgress->updateProgress("Run background test", 0, NORMAL);
   }

 
   // Create a new thread safe progress object
   //
   Progress *pNewProgress = mpUtility->getProgress(true);
   VERIFY(pNewProgress != NULL);
   Progress *pOldProgress = mpProgress;
   mpProgress = pNewProgress;

   // Set the output arg
   //
   VERIFY(pOutArgList->getArg(ProgressArg(), pArg) && (pArg != NULL));
   pArg->setActualValue(mpProgress);

   // Create a worker thread
   //
   mpWorkerThread = new BThread(this, reinterpret_cast<void*>(BackgroundTest::runWorkerThread));
   VERIFY(mpWorkerThread != NULL);
   mpWorkerThread->ThreadInit();
   mpWorkerThread->ThreadLaunch(-5); // lower the priority of this thread to ensure the GUI remains responsive

   // Mark this as a success and return
   //
   if(pOldProgress != NULL)
   {
      pOldProgress->updateProgress("Run background test", 100, NORMAL);
   }
   pStep->finalize(Message::Success);
   return true;
}

bool BackgroundTest::abort()
{
   mAbort = true;
   return true;
}

bool BackgroundTest::hasAbort()
{
   return true;
}

bool BackgroundTest::isBackground() const
{
   return true;
}

void BackgroundTest::runWorkerThread(void *pArg)
{
   if(pArg == NULL)
   {
      return;
   }

   // Execute the worker thread
   //
   BackgroundTest *pSelf = reinterpret_cast<BackgroundTest*>(pArg);
   bool returnValue = pSelf->workerThread();

   // Create a callback to exit the background thread
   //
   PlugInCallback *pCallback = new BackgroundTest::Callback(pSelf,
                                                            pSelf->mpWorkerThread,
                                                            returnValue,
                                                            pSelf->mpProgress);
   Service<DesktopServices> pDesktop;
   if(pCallback != NULL)
   {
      pDesktop->registerCallback(BACKGROUND_COMPLETE, pCallback);
   }
}

bool BackgroundTest::workerThread()
{
   for(unsigned int tick = 0; tick < 50; tick++)
   {
      if(mAbort)
      {
         if(mpProgress != NULL)
         {
            mpProgress->updateProgress("Aborted", 0, ABORT);
         }
         return false;
      }
      if(mpProgress != NULL)
      {
         double percent = 100 * tick / 20.0;
         mpProgress->updateProgress("Working...", static_cast<int>(percent), NORMAL);
      }
#if defined(WIN_API)
      Sleep(500);     // 0.5 seconds
#elif defined(UNIX_API)
      usleep(500000); // 0.5 seconds
#else
#error "Platform Unknown."
#endif
   }

   mpProgress->updateProgress("Finished", 100, NORMAL);

   return true;
}

BackgroundTest::Callback::Callback(BackgroundTest *pPlugin, BThread *pThread, bool returnValue, Progress *pProgress) :
      mpPlugin(pPlugin),
      mpThread(pThread),
      mReturnValue(returnValue),
      mpProgress(pProgress)
{
}

BackgroundTest::Callback::~Callback()
{
   delete mpThread;
}

void BackgroundTest::Callback::operator()()
{
   // blank
}

bool BackgroundTest::Callback::finish()
{
   // wait for thread to finish
   return mpThread->ThreadWait();
}

PlugIn* BackgroundTest::Callback::getPlugIn() const
{
   return dynamic_cast<PlugIn*>(mpPlugin);
}

Progress *BackgroundTest::Callback::progress() const
{
   return mpProgress;
}
