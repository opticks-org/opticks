/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "WizardItems.h"
#include "AppVerify.h"
#include "MessageLogResource.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "Progress.h"

using namespace std;

WizardItems::WizardItems() : mbInteractive(false), mpStep(NULL), mpProgress(NULL)
{
}

WizardItems::~WizardItems()
{
}

bool WizardItems::hasAbort()
{
   return false;
}

bool WizardItems::getInputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;

   Service<PlugInManagerServices> pPlugInManager;
   VERIFY(pPlugInManager.get() != NULL);

   // Set up list
   pArgList = pPlugInManager->getPlugInArgList();
   VERIFY(pArgList != NULL);

   PlugInArg* pArg = NULL;
   pArg = pPlugInManager->getPlugInArg();      // Progress
   VERIFY(pArg != NULL);
   pArg->setName(ProgressArg());
   pArg->setType("Progress");
   pArg->setDefaultValue(NULL);
   pArgList->addArg(*pArg);

   return true;
}

bool WizardItems::extractInputArgs(PlugInArgList* pInArgList)
{
   if (pInArgList == NULL)
   {
      return false;
   }

   // Progress
   PlugInArg* pArg = NULL;
   if ((pInArgList->getArg(ProgressArg(), pArg) == true) && (pArg != NULL))
   {
      mpProgress = pArg->getPlugInArgValue<Progress>();
   }

   return true;
}

void WizardItems::reportProgress(const string& progressMsg, int iPercent, const string &key)
{
   if(mpProgress != NULL)
   {
      mpProgress->updateProgress(progressMsg, iPercent, NORMAL);
   }

   MessageResource msg("Progress", "app", "2A70C60D-ECBE-48B0-AD58-D9ABD3CCB659");
   msg->addProperty("Message", progressMsg);
}

void WizardItems::reportWarning(const string& warningMsg, const string &key)
{
   if(mpProgress != NULL)
   {
      mpProgress->updateProgress(warningMsg, 0, WARNING);
   }

   MessageResource msg("Warning", "app", "37403DB9-0DCE-446B-92E0-E3FD17438517");
   msg->addProperty("Message", warningMsg);
}

void WizardItems::reportError(const string &errorMsg, const string &key)
{
   if(mpProgress != NULL)
   {
      mpProgress->updateProgress(errorMsg, 0, ERRORS);
   }

   MessageResource msg("Error", "app", "54B3BF7F-C573-42B4-BC34-292B2FF93550");
   msg->addProperty("Message", errorMsg);

   if(mpStep != NULL)
   {
      mpStep->finalize(Message::Failure, errorMsg);
   }
}

void WizardItems::reportComplete()
{
   string plugInName = getName();
 
   if(plugInName.empty())
   {
      plugInName = "WizardItem";
   }

   if(mpProgress != NULL)
   {
      mpProgress->updateProgress((plugInName + " complete!"), 100, NORMAL);
   }
   if(mpStep != NULL)
   {
      mpStep->finalize(Message::Success);
   }
}

Progress* WizardItems::getProgress() const
{
   return mpProgress;
}
