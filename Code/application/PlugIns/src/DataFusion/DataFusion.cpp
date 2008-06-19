/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "DataFusion.h"
#include "DataFusionDlg.h"
#include "DataFusionTools.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "ProgressTracker.h"

#include <string>
using namespace std;

const std::string DataFusion::PLUGIN_NAME = "Data Fusion";

DataFusion::DataFusion() : mbInteractive(true), mpWizardDlg(NULL)
{
   setName(PLUGIN_NAME);
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setDescription("Fuses two images based on one or more tie points");
   setDescriptorId("{4E23B86F-CB71-4f5a-9AB9-0D9646C34F0E}");
   setMenuLocation("[General Algorithms]\\Data &Fusion");
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

DataFusion::~DataFusion()
{
}

bool DataFusion::abort()
{
   DataFusionTools::setAbortFlag(true);
   return ViewerShell::abort();
}

bool DataFusion::getInputSpecification(PlugInArgList* &pArgList)
{
   pArgList = NULL;
   if (mbInteractive)
   {
      // Set up list
      pArgList = mpPlugInManager->getPlugInArgList();
      if (pArgList == NULL)
      {
         return false; // bad arg list -> kick out
      }

      PlugInArg* pArg = NULL;
      pArg = mpPlugInManager->getPlugInArg(); // get me a new plugInArg - using studio's heap
      if (pArg == NULL)
      {
         return false; // bad arg -> kick out
      }
      // get the progress from the arg list
      pArg->setName(ProgressArg()); 
      pArg->setType("Progress"); // we want a Progress object
      pArg->setDefaultValue(NULL);
      pArgList->addArg(*pArg);
   }

   return true;
}

bool DataFusion::getOutputSpecification(PlugInArgList* &pOutputArgs)
{
   pOutputArgs = NULL;
   return true;
}

bool DataFusion::setBatch()
{
   mbInteractive = false;
   return mbInteractive;
}

bool DataFusion::setInteractive()
{
   mbInteractive = true;
   return mbInteractive;
}

bool DataFusion::execute(PlugInArgList* pInputArgs, PlugInArgList* pOutputArgs)
{
   // Get the progress value from the input arg list
   Progress* pProgress = NULL;
   if (pInputArgs != NULL)
   {
      PlugInArg* pArg = NULL;
      if ((pInputArgs->getArg(ProgressArg(), pArg)) && (pArg != NULL))
      {
         pProgress = reinterpret_cast<Progress*>(pArg->getActualValue());
      }
   }

   // Invoke the wizard dialog
   if (mbInteractive)
   {
      mProgressTracker.initialize(pProgress, "Executing DataFusion Plug-In", "app",
                                  "B7E3EF13-70D1-4a45-A95E-0126373C3191");

      if (mpWizardDlg == NULL)
      {
         Service<DesktopServices> pDesktop;

         mpWizardDlg = new DataFusionDlg(this, mProgressTracker, pDesktop->getMainWidget());
         if (mpWizardDlg == NULL)
         {
            return false;
         }
      }

      mpWizardDlg->show();
   }

   return mbInteractive;
}

QWidget* DataFusion::getWidget() const
{
   return mpWizardDlg;
}
