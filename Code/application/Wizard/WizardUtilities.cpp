/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "WizardUtilities.h"
#include "BatchWizard.h"
#include "DataVariant.h"
#include "DateTime.h"
#include "DesktopServices.h"
#include "Filename.h"
#include "FileResource.h"
#include "ObjectResource.h"
#include "PlugInArgList.h"
#include "PlugInResource.h"
#include "Progress.h"
#include "WizardItem.h"
#include "WizardObject.h"
#include "xmlreader.h"
#include "xmlwriter.h"

#include "XercesIncludes.h"

#include <fstream>
using namespace std;

string WizardUtilities::deriveBatchWizardFilename(const string& wizardFilename)
{
   FactoryResource<Filename> pFilename;
   pFilename->setFullPathAndName(wizardFilename);
   string directory = pFilename->getPath();
   string title = pFilename->getTitle();
   return directory + "/" + title + ".batchwiz";
}

bool WizardUtilities::writeBatchWizard(const vector<BatchWizard*>& batchWizards, const string& batchFilename)
{
   if ((batchWizards.empty() == true) || (batchFilename.empty() == true))
   {
      return false;
   }

   Service<MessageLogMgr> pLogMgr;
   XMLWriter writer("batch", pLogMgr->getLog());

   for (vector<BatchWizard*>::const_iterator iter = batchWizards.begin(); iter != batchWizards.end(); ++iter)
   {
      BatchWizard* pBatchWizard = *iter;
      if (pBatchWizard != NULL)
      {
         bool success = pBatchWizard->toXml(&writer);
         if (!success)
         {
            return false;
         }
      }
   }

   FileResource batchFile(batchFilename.c_str(), "wb");
   if (batchFile.get() == NULL)
   {
      return false;
   }

   writer.writeToFile(batchFile);
   return true;
}

BatchWizard* WizardUtilities::createBatchWizardFromWizard(const string& wizardFilename)
{
   FactoryResource<WizardObject> pWizard(readWizard(wizardFilename));
   if (pWizard.get() != NULL)
   {
      return createBatchWizardFromWizard(pWizard.get(), wizardFilename);
   }
   return NULL;
}

BatchWizard* WizardUtilities::createBatchWizardFromWizard(WizardObject* pWizard, const string& wizardFilename)
{
   if (pWizard == NULL)
   {
      return NULL;
   }

   BatchWizard* pBatchWizard = new BatchWizard();
   if (pBatchWizard != NULL)
   {
      pBatchWizard->setWizardFilename(wizardFilename);
      // Values
      vector<WizardItem*> wizItems = pWizard->getItems();
      for (unsigned int i = 0; i < wizItems.size(); i++)
      {
         WizardItem* pItem = wizItems[i];
         if ((pItem != NULL) && (pItem->getType() != "Value"))
         {
            const string& itemName = pItem->getName();

            vector<WizardItem*> inputItems;
            pItem->getConnectedItems(true, inputItems);
            for (unsigned int j = 0; j < inputItems.size(); j++)
            {
               WizardItem* pInputItem = inputItems[j];
               if ((pInputItem != NULL) && (pInputItem->getType() == "Value"))
               {
                  vector<WizardNode*> wizNodes = pInputItem->getOutputNodes();
                  for (unsigned int n = 0; n < wizNodes.size(); n++)
                  {
                     string nodeName = wizNodes[n]->getName();
                     string nodeType = wizNodes[n]->getType();

                     void* pValue = wizNodes[n]->getValue();
                     if (pValue == NULL)
                     {
                        continue;
                     }

                     DataVariant value(nodeType, pValue);
                     pBatchWizard->setInputValue(itemName, nodeName, nodeType, value);
                  }
               }
            }
         }
      }
   }

   return pBatchWizard;
}

WizardObject* WizardUtilities::readWizard(const string& wizardFilename)
{
   if (wizardFilename.empty() == true)
   {
      return NULL;
   }

   FactoryResource<WizardObject> pWizard;

   XmlReader xml;
   FactoryResource<Filename> pFilename;
   pFilename->setFullPathAndName(wizardFilename);
   bool bSuccess = false;

   XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* pDocument = xml.parse(pFilename.get());
   if (pDocument != NULL)
   {
      XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* pRootElement = pDocument->getDocumentElement();
      if (pRootElement != NULL)
      {
         unsigned int version = atoi(A(pRootElement->getAttribute(X("version"))));
         bSuccess = pWizard->fromXml(pRootElement, version);
      }
   }

   if (bSuccess == false)
   {
      return NULL;
   }

   return pWizard.release();
}

void WizardUtilities::runWizard(WizardObject* pWizard)
{
   if (pWizard == NULL)
   {
      return;
   }

   // Create a plug-in resource to ensure that the plug-in is unloaded
   string plugInName = "Wizard Executor";
   bool bBatch = pWizard->isBatch();

   ExecutableResource wizardExecutor(plugInName, string(), NULL, bBatch);
   VERIFYNRV(wizardExecutor.get() != NULL);
   wizardExecutor->getInArgList().setPlugInArgValue("Wizard", pWizard);
   wizardExecutor->createProgressDialog(true);

   // Execute the wizard
   wizardExecutor->execute();
}

bool WizardUtilities::runBatchFiles(const vector<string> &wizardFiles, Progress *pProgress)
{
   ExecutableResource batchWizardExecutor("Batch Wizard Executor", string(), pProgress,
      Service<ApplicationServices>()->isBatch());
   if (batchWizardExecutor->getPlugIn() == NULL)
   {
      if (pProgress != NULL)
      {
         string message = "Could not get the batch wizard executor plug-in.";
         pProgress->updateProgress(message, 0, ERRORS);
      }

      return false;
   }

   unsigned int iCount = wizardFiles.size(); 

   // Launch the batch wizard executor for each xml file
   for (unsigned int i = 0; i < iCount; ++i)
   {
      string filename = wizardFiles[i];
      if (!filename.empty())
      {
         if (pProgress != NULL)
         {
            string message = "Processing batch file: " + filename;
            pProgress->updateProgress(message, i * 100 / iCount, NORMAL);
         }

         FactoryResource<Filename> pFilename;
         VERIFY(pFilename.get() != NULL);
         pFilename->setFullPathAndName(filename);
         batchWizardExecutor->getInArgList().setPlugInArgValue("Filename", pFilename.get());
         bool bSuccess = batchWizardExecutor->execute();
         if (pProgress != NULL)
         {
            if (!bSuccess)
            {
               pProgress->updateProgress("Batch file failed: " + filename, (i + 1) * 100 / iCount, WARNING);
            }
            else
            {
               pProgress->updateProgress("Batch file complete: " + filename, (i + 1) * 100 / iCount, NORMAL);
            }
         }
      }
   }

   if (pProgress != NULL)
   {
      string message = "Processing batch files completed.";
      pProgress->updateProgress(message, 100, NORMAL);
   }

   return true;
}
