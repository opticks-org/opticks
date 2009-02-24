/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#include <QtCore/QStringList>
#include <QtGui/QFileDialog>

#include "ApplicationServices.h"
#include "AppVersion.h"
#include "AppVerify.h"
#include "DesktopServices.h"
#include "Filename.h"
#include "GetFilename.h"
#include "MessageLogResource.h"
#include "ObjectFactory.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"

#include <string>
using namespace std;

/////////////////
// GetFilename //
/////////////////

GetFilename::GetFilename()
{
}

GetFilename::~GetFilename()
{
}

bool GetFilename::getInputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;

   if (mbInteractive)
   {
      VERIFY(DesktopItems::getInputSpecification(pArgList) && (pArgList != NULL));

      Service<PlugInManagerServices> pPlugInManager;
      VERIFY(pPlugInManager.get() != NULL);

      // Add args
      PlugInArg* pArg = pPlugInManager->getPlugInArg();
      VERIFY(pArg != NULL);
      pArg->setName("Dialog Caption");
      pArg->setType("string");
      pArg->setDefaultValue(NULL);
      pArgList->addArg(*pArg);

      pArg = pPlugInManager->getPlugInArg();
      VERIFY(pArg != NULL);
      pArg->setName("Initial Directory/File");
      pArg->setType("Filename");
      pArg->setDefaultValue(NULL);
      pArgList->addArg(*pArg);

      pArg = pPlugInManager->getPlugInArg();
      VERIFY(pArg != NULL);
      pArg->setName("File Filters");
      pArg->setType("string");
      pArg->setDefaultValue(NULL);
      pArgList->addArg(*pArg);
   }

   return true;
}

bool GetFilename::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;

   if (mbInteractive)
   {
      Service<PlugInManagerServices> pPlugInManager;
      VERIFY(pPlugInManager.get() != NULL);

      // Set up list
      pArgList = pPlugInManager->getPlugInArgList();
      VERIFY(pArgList != NULL);

      // Add args
      PlugInArg* pArg = pPlugInManager->getPlugInArg();
      VERIFY(pArg != NULL);
      pArg->setName("Filename");
      pArg->setType("Filename");
      pArg->setDefaultValue(NULL);
      pArgList->addArg(*pArg);
   }

   return true;
}

bool GetFilename::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Execute Wizard Item", "app", "A8942A47-F5A1-4ff5-8248-04CAFD5632B0");
   pStep->addProperty("Item", getName());
   mpStep = pStep.get();

   if (!extractInputArgs(pInArgList))
   {
      reportError("Unable to extract input arguments.", "DED4E7C6-D38F-465f-93CB-F6F0391FA851");
      return false;
   }

   // Get the filename
   QString strFilename = getFilenameFromUser();
   if (strFilename.isEmpty())
   {
      reportError("No filename selected.", "B121D579-96EF-4b4d-A5B4-3C70B12031EB");
      return false;
   }

   // Set the output value
   if (pOutArgList != NULL)
   {
      PlugInArg* pArg = NULL;
      if (pOutArgList->getArg("Filename", pArg) && (pArg != NULL))
      {
         Filename* pFilename = NULL;
         Service<ApplicationServices> pApplication;
         VERIFY(pApplication.get() != NULL);
         ObjectFactory* pObjFact = pApplication->getObjectFactory();
         if (pObjFact != NULL)
         {
            pFilename = static_cast<Filename*>(pObjFact->createObject("Filename"));
         }

         if (pFilename != NULL)
         {
            pFilename->setFullPathAndName(strFilename.toStdString());
            pArg->setActualValue(pFilename);
         }
         else
         {
            reportError("Could not set the output value!", "542AF154-287C-414d-B5BA-FAF212DF78A2");
            return false;
         }
      }
      else
      {
         reportError("Could not set the output value!", "ECCDD8C1-B3C2-4a62-B1D1-EA705DECE2A3");
         return false;
      }
   }

   reportComplete();
   return true;
}

bool GetFilename::extractInputArgs(PlugInArgList* pInArgList)
{
   if (!DesktopItems::extractInputArgs(pInArgList))
   {
      return false;
   }

   PlugInArg* pArg = NULL;

   // Dialog caption
   if (!pInArgList->getArg("Dialog Caption", pArg) || (pArg == NULL))
   {
      reportError("Could not read the dialog caption input value!", "C7B90CA5-33DC-4358-94C4-9E08885644FC");
      return false;
   }
   string* pCaption = pArg->getPlugInArgValue<string>();
   if (pCaption != NULL && !pCaption->empty())
   {
      mCaption = QString::fromStdString(*pCaption);
   }

   // Initial directory/file
   if (!pInArgList->getArg("Initial Directory/File", pArg) || (pArg == NULL))
   {
      reportError("Could not read the initial directory/file input value!", "D283CE82-D606-4ea6-9E38-2510127668F1");
      return false;
   }
   Filename* pFilename = pArg->getPlugInArgValue<Filename>();
   if (pFilename != NULL)
   {
      mInitialDir = QString::fromStdString(pFilename->getFullPathAndName());
   }

   // File filters
   if (!pInArgList->getArg("File Filters", pArg) || (pArg == NULL))
   {
      reportError("Could not read the file filters input value!", "32E891A0-80EA-4c9c-B34F-3DFFE9CE6766");
      return false;
   }
   string* pFilters = pArg->getPlugInArgValue<string>();
   if (pFilters != NULL && !pFilters->empty())
   {
      mFilters = QString::fromStdString(*pFilters);
   }

   if (mpStep != NULL)
   {
      string filters = "";
      if (!mFilters.isEmpty())
      {
         filters = mFilters.toStdString();
      }

      string initialDir = "";
      if (!mInitialDir.isEmpty())
      {
         initialDir = mInitialDir.toStdString();
      }

      string caption = "";
      if (!mCaption.isEmpty())
      {
         caption = mCaption.toStdString();
      }

      mpStep->addProperty("filters", filters);
      mpStep->addProperty("initialDir", initialDir);
      mpStep->addProperty("caption", caption);
   }

   return true;
}

QString GetFilename::getFilenameFromUser()
{
   return QString();
}

/////////////////////////
// GetExistingFilename //
/////////////////////////

GetExistingFilename::GetExistingFilename()
{
   setName("Get Existing Filename");
   setVersion(APP_VERSION_NUMBER);
   setCreator("Ball Aerospace & Technologies, Corp.");
   setCopyright(APP_COPYRIGHT);
   setDescription("Displays a file selection dialog to get the name of an existing file");
   setDescriptorId("{44859273-0106-4140-97FF-BBB8BB68C366}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

GetExistingFilename::~GetExistingFilename()
{
}

QString GetExistingFilename::getFilenameFromUser()
{
   QString strFilename = QFileDialog::getOpenFileName(mpDesktop->getMainWidget(), mCaption, mInitialDir, mFilters);
   return strFilename;
}

////////////////////
// GetNewFilename //
////////////////////

GetNewFilename::GetNewFilename()
{
   setName("Get New Filename");
   setVersion(APP_VERSION_NUMBER);
   setCreator("Ball Aerospace & Technologies, Corp.");
   setCopyright(APP_COPYRIGHT);
   setDescription("Displays a file selection dialog to get the name of a file to create");
   setDescriptorId("{17DC0027-39E3-4b75-BA20-686A3ADF98CC}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

GetNewFilename::~GetNewFilename()
{
}

QString GetNewFilename::getFilenameFromUser()
{
   QString strFilename = QFileDialog::getSaveFileName(mpDesktop->getMainWidget(), mCaption, mInitialDir, mFilters);
   return strFilename;
}

//////////////////////////
// GetExistingFilenames //
//////////////////////////

GetExistingFilenames::GetExistingFilenames()
{
   setName("Get Existing Filenames");
   setVersion(APP_VERSION_NUMBER);
   setCreator("Ball Aerospace & Technologies, Corp.");
   setCopyright(APP_COPYRIGHT);
   setDescription("Displays a file selection dialog to get multiple existing filenames");
   setDescriptorId("{59566313-4DE2-443d-9604-A627FA51553B}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

GetExistingFilenames::~GetExistingFilenames()
{
}

bool GetExistingFilenames::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;

   if (mbInteractive)
   {
      Service<PlugInManagerServices> pPlugInManager;
      VERIFY(pPlugInManager.get() != NULL);

      // Set up list
      pArgList = pPlugInManager->getPlugInArgList();
      VERIFY(pArgList != NULL);

      // Add args
      PlugInArg* pArg = pPlugInManager->getPlugInArg();
      VERIFY(pArg != NULL);
      pArg->setName("Filenames");
      pArg->setType("vector<Filename>");
      pArg->setDefaultValue(NULL);
      pArgList->addArg(*pArg);
   }

   return true;
}

bool GetExistingFilenames::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Execute Wizard Item", "app", "8B5D1D11-A6CD-4bf7-BAEC-5D736C234E5E");
   pStep->addProperty("Item", getName());
   mpStep = pStep.get();

   if (!extractInputArgs(pInArgList))
   {
      reportError("Unable to extract input arguments.", "5121B5C3-0C10-484a-B62A-0F5126A88676");
      return false;
   }

   Service<DesktopServices> pDesktop;
   VERIFY(pDesktop.get() != NULL);

   // Get the filenames from the user
   QStringList strlFilenames = QFileDialog::getOpenFileNames(pDesktop->getMainWidget(), mCaption,
                                                             mInitialDir, mFilters);
   if (strlFilenames.isEmpty())
   {
      reportError("No filenames selected.", "EFF383ED-7D6B-4607-942D-0B1D4D78B315");
      return false;
   }

   // Create the output vector
   Service<ApplicationServices> pApplication;
   VERIFY(pApplication.get() != NULL);
   ObjectFactory* pObjFact = pApplication->getObjectFactory();
   VERIFY(pObjFact != NULL);

   vector<Filename*> filenames;
   for (int i = 0; i < strlFilenames.count(); i++)
   {
      QString strFilename = strlFilenames[i];
      if (!strFilename.isEmpty())
      {
         Filename* pCurrentFilename = NULL;
         pCurrentFilename = static_cast<Filename*>(pObjFact->createObject("Filename"));

         if (pCurrentFilename != NULL)
         {
            pCurrentFilename->setFullPathAndName(strFilename.toStdString());
            filenames.push_back(pCurrentFilename);
         }
      }
   }

   // Set the output value
   if (pOutArgList != NULL)
   {
      PlugInArg* pArg = NULL;
      if (pOutArgList->getArg("Filenames", pArg) && (pArg != NULL))
      {
         pArg->setActualValue(&filenames);
      }
      else
      {
         reportError("Could not set the output value!", "77B5D277-B791-4073-973C-4F855A34807E");
         return false;
      }
   }

   reportComplete();
   return true;
}
