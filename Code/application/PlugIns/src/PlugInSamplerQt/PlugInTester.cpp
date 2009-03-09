/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInDescriptor.h"
#include "PlugInManagerServices.h"
#include "PlugInResource.h"
#include "PlugInTester.h"
#include "PlugInSelectorDlg.h"
#include "Progress.h"
#include "Testable.h"

#include <sstream>
#include <string>

using namespace std;

PlugInTester::PlugInTester()
{
   setName("Plug-In Tester");

   setMenuLocation("[Demo]\\T&est Plug-Ins");
   setProductionStatus(false);
   setDescriptorId("{F091EED9-436E-4908-8A23-C71E890F19E3}");
   allowMultipleInstances(false);
}

bool PlugInTester::getInputSpecification(PlugInArgList *&pArgList)
{
   bool bSuccess = false;
   pArgList = mpPlugMgr->getPlugInArgList();
   if (pArgList != NULL)
   {
      PlugInArg* pArg = mpPlugMgr->getPlugInArg();
      if (pArg != NULL)
      {
         pArg->setName(ProgressArg());
         pArg->setType("Progress");
         pArg->setDefaultValue(NULL);
         bSuccess = pArgList->addArg(*pArg);
      }
   }
   return bSuccess;
}

bool PlugInTester::getOutputSpecification(PlugInArgList *&pArgList)
{
   pArgList = NULL;
   return true;
}

bool PlugInTester::execute(PlugInArgList* pInArgs, PlugInArgList* pOutArgs)
{
   VERIFY(pInArgs != NULL);

   Progress* pProgress = NULL;
   PlugInArg* pArg = NULL;
   if (pInArgs != NULL && pInArgs->getArg(ProgressArg(), pArg) && pArg != NULL)
   {
      pProgress = reinterpret_cast<Progress*>(pArg->getActualValue());
   }

   vector<PlugInDescriptor*> allPlugins = mpPlugMgr->getPlugInDescriptors();
   vector<string> testablePlugins;
   for (vector<PlugInDescriptor*>::const_iterator it = allPlugins.begin(); it != allPlugins.end(); ++it)
   {
      PlugInDescriptor* pDescriptor = *it;
      if (pDescriptor == NULL)
      {
         continue;
      }
      if (pDescriptor->isTestable())
      {
         testablePlugins.push_back(pDescriptor->getName());
      }
   }

   string msg;
   bool bSuccess = false;
   PlugInSelectorDlg dlg(mpDesktop->getMainWidget(), testablePlugins);
   int stat = dlg.exec();
   if (stat == QDialog::Accepted)
   {
      const vector<string>& pluginsToTest = dlg.getSelectedPlugins();

      // TODO: Set up a ProgressTracker for each plug-in to test

      vector<string>::const_iterator it;
      for (it = pluginsToTest.begin(); it != pluginsToTest.end(); ++it)
      {
         PlugInResource pPlugIn(*it);
         Testable* pTestable = dynamic_cast<Testable*>(pPlugIn.get());
         if (pTestable == NULL)
         {
            msg += "The Plug-In " + *it + " cannot be created!";
            if (pProgress != NULL)
            {
               pProgress->updateProgress(msg, 0, ERRORS);
            }
            return false;
         }

         msg += "Testing " + *it + "...";
         if (pProgress != NULL)
         {
            pProgress->updateProgress(msg, 0, NORMAL);
         }

         stringstream ostr;
         bSuccess = pTestable->runAllTests(pProgress, ostr);

         msg += "Testing of Plug-In " + *it + " has been completed";
         ReportingLevel lvl = NORMAL;
         if (!bSuccess)
         {
            lvl = ERRORS;
            msg += " with errors!";
         }
         else
         {
            msg += "!";
         }

         if (ostr.str().size() > 0)
         {
            msg += "\n" + ostr.str() + "\n";
         }
         else
         {
            msg += "\n";
         }

         if (pProgress != NULL)
         {
            pProgress->updateProgress(msg, 100, lvl);
         }
      }
   }

   return bSuccess;
}
