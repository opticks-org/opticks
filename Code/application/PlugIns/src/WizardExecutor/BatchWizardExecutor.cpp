/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ApplicationServices.h"
#include "AppVersion.h"
#include "BatchFileParser.h"
#include "BatchWizardExecutor.h"
#include "AppAssert.h"
#include "AppVerify.h"
#include "DateTime.h"
#include "Filename.h"
#include "MessageLogResource.h"
#include "ModelServices.h"
#include "ObjectFactory.h"
#include "ObjectResource.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "StringUtilities.h"
#include "UtilityServices.h"
#include "WizardExecutor.h"
#include "WizardItem.h"
#include "WizardObject.h"
#include "xmlreader.h"

using namespace std;

REGISTER_PLUGIN_BASIC(OpticksWizardExecutor, BatchWizardExecutor);

BatchWizardExecutor::BatchWizardExecutor() :
   mbAbort(false),
   mpProgress(NULL),
   mpStep(NULL),
   mpExecutor(NULL)
{
   setName("Batch Wizard Executor");
   setVersion(APP_VERSION_NUMBER);
   setCreator("Ball Aerospace & Technologies, Corp.");
   setCopyright(APP_COPYRIGHT);
   setShortDescription("Batch Wizard");
   setDescription("Parses an XML file containing a wizard filename and node values and executes the wizard");
   setDescriptorId("{955A6F75-2A17-4d94-8EF7-62F7F12FCFB1}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

BatchWizardExecutor::~BatchWizardExecutor()
{
}

bool BatchWizardExecutor::hasAbort()
{
   bool bSuccess = true;
   if (mpExecutor != NULL)
   {
      bSuccess = mpExecutor->hasAbort();
   }

   return bSuccess;
}

bool BatchWizardExecutor::getInputSpecification(PlugInArgList*& pArgList)
{
   // Set up list
   pArgList = mpPlugInManager->getPlugInArgList();
   VERIFY(pArgList != NULL);

   PlugInArg* pArg = mpPlugInManager->getPlugInArg();      // Progress
   VERIFY(pArg != NULL);
   pArg->setName(ProgressArg());
   pArg->setType("Progress");
   pArg->setDefaultValue(NULL);
   pArgList->addArg(*pArg);

   pArg = mpPlugInManager->getPlugInArg();      // XML Filename
   VERIFY(pArg != NULL);
   pArg->setName("Filename");
   pArg->setType("Filename");
   pArg->setDefaultValue(NULL);
   pArgList->addArg(*pArg);

   return true;
}

bool BatchWizardExecutor::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Run Batch WizardExecutor", "app", "B415A858-820B-4f37-BC8D-37E6BBBC95BD");
   mpStep = pStep.get();

   Service<ModelServices> pModel;
   VERIFY(pModel.get() != NULL);

   if (!extractInputArgs(pInArgList))
   {
      return false;
   }

   pStep->addProperty("batchWizardFile", mFilename);

   BatchFileParser fileParser;
   if (!fileParser.setFile(mFilename))
   {
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(fileParser.getError(), 0, ERRORS);
      }

      pStep->finalize(Message::Failure, fileParser.getError());
      return false;
   }

   BatchWizard* pBatchWizard = fileParser.read();
   while (pBatchWizard != NULL)
   {
      // Initialize the filesets
      pBatchWizard->initializeFilesets(mpObjFact.get());

      // Repeat until done
      string tmp;
      bool bRepeatWizard = pBatchWizard->isRepeating(tmp);

      bool bExecutedOnce = false;
      while ((!bRepeatWizard && !bExecutedOnce) || !pBatchWizard->isComplete())
      {
         // Load the wizard
         string wizardFilename = pBatchWizard->getWizardFilename();

         bool bSuccess = false;
         FactoryResource<WizardObject> pWizard;

         FactoryResource<Filename> pWizardFilename;
         pWizardFilename->setFullPathAndName(wizardFilename);

         XmlReader xml;
         XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* pDocument = xml.parse(pWizardFilename.get());
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
            string message = "Cannot load the wizard from the file: " + wizardFilename;
            delete pBatchWizard;
            if (mpProgress != NULL)
            {
               mpProgress->updateProgress(message, 0, ERRORS);
            }

            pStep->finalize(Message::Failure, message);
            return false;
         }

         // Set the values in the wizard
         vector<Value*> inputValues = pBatchWizard->getInputValues();
         for (vector<Value*>::iterator iter = inputValues.begin(); iter != inputValues.end(); ++iter)
         {
            Value* pValue = *iter;
            if (pValue != NULL)
            {
               string nodeName = pValue->getNodeName();
               string nodeType = pValue->getNodeType();

               string nodeTypeString = nodeType;
               if (nodeType == "File set")
               {
                  nodeTypeString = "Filename";
               }

               WizardNode* pNode = getValueNode(pWizard.get(), nodeName, nodeTypeString);
               if (pNode != NULL)
               {
                  const DataVariant& value = pValue->getValue();
                  if (nodeType == "File set")
                  {
                     string filesetName;
                     value.getValue<string>(filesetName);
                     if (filesetName.empty() == false)
                     {
                        string currentFilename;
                        if (pBatchWizard != NULL)
                        {
                           pBatchWizard->getCurrentFilesetFile(filesetName, currentFilename);
                        }

                        FactoryResource<Filename> pFilename;
                        pFilename->setFullPathAndName(currentFilename);

                        pNode->setValue(pFilename.get());
                     }
                  }
                  else if (value.isValid() == true)
                  {
                     pNode->setValue(value.getPointerToValueAsVoid());
                  }
               }
               else if (mpProgress != NULL)
               {
                  string message = "Could not set the '" + nodeName + "' item value.";
                  mpProgress->updateProgress(message, 0, WARNING);
               }
            }
         }

         // Run the wizard
         bool bWizardSuccess = runWizard(pWizard.get());

         // Cleanup after each run
         if (pBatchWizard->doesCleanup())
         {
            pModel->clear();
         }

         if (mbAbort)
         {
            delete pBatchWizard;
            string message = "Batch Wizard Exector Aborted!";
            if (mpProgress != NULL)
            {
               mpProgress->updateProgress(message, 0, ABORT);
            }

            pStep->finalize(Message::Abort, message);
            return false;
         }

         if (!bWizardSuccess)
         {
            delete pBatchWizard;
            pStep->finalize(Message::Failure);
            return false;
         }

         bExecutedOnce = true;
         pBatchWizard->updateFilesets();
      }

      delete pBatchWizard;
      pBatchWizard = NULL;
      pBatchWizard = fileParser.read();
   }

   pStep->finalize(Message::Success);

   return true;
}

bool BatchWizardExecutor::abort()
{
   mbAbort = true;

   bool bSuccess = true;
   if (mpExecutor != NULL)
   {
      bSuccess = mpExecutor->abort();
   }

   return bSuccess;
}

bool BatchWizardExecutor::extractInputArgs(PlugInArgList* pInArgList)
{
   VERIFY(pInArgList != NULL);

   PlugInArg* pArg = NULL;

   // Progress
   if (!pInArgList->getArg(ProgressArg(), pArg) || (pArg == NULL))
   {
      string message = "Could not read the progress input value!";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(message, 0, ERRORS);
      }

      mpStep->finalize(Message::Failure, message);
      return false;
   }
   mpProgress = pArg->getPlugInArgValue<Progress>();

   // XML filename
   if (!pInArgList->getArg("Filename", pArg) || (pArg == NULL))
   {
      string message = "Could not read the filename input value!";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(message, 0, ERRORS);
      }

      mpStep->finalize(Message::Failure, message);
      return false;
   }
   Filename* pFilename = pArg->getPlugInArgValue<Filename>();
   string filename;
   if (pFilename != NULL)
   {
      filename = pFilename->getFullPathAndName();
   }

   if (filename.empty())
   {
      string message = "The filename input value is invalid!";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(message, 0, ERRORS);
      }

      mpStep->finalize(Message::Failure, message);
      return false;
   }

   mFilename = filename;
   return true;
}

WizardNode* BatchWizardExecutor::getValueNode(const WizardObject* pWizard, const string& nodeName,
                                              const string& nodeType)
{
   VERIFY(pWizard != NULL);

   vector<WizardItem*> items = pWizard->getItems();

   // Loop through all wizard items and find value items.
   // For example, ignore algorithms, importers, etc.
   for (vector<WizardItem*>::iterator itemIter = items.begin(); itemIter != items.end(); ++itemIter)
   {
      WizardItem* pItem = *itemIter;
      if (pItem != NULL)
      {
         string itemType = pItem->getType();
         if (itemType == "Value")
         {
            const vector<WizardNode*>& nodes = pItem->getOutputNodes();

            // Loop through the nodes for this item.
            // Really shouldn't have to do this right now, but
            // may change in the future.  Currently Values only
            // have one node.
            for (vector<WizardNode*>::const_iterator node = nodes.begin(); node != nodes.end(); ++node)
            {
               WizardNode* pNode = *node;
               if (pNode != NULL)
               {
                  string currentName = pNode->getName();
                  string currentType = pNode->getType();

                  if ((currentName == nodeName) && (currentType == nodeType))
                  {
                     return pNode;
                  }
               }
            }
         }
      }
   }

   return NULL;
}

bool BatchWizardExecutor::runWizard(WizardObject* pWizard)
{
   VERIFY(pWizard != NULL);

   mpExecutor = new WizardExecutor;
   VERIFY(mpExecutor != NULL);

   if (pWizard->isBatch())
   {
      mpExecutor->setBatch();
   }
   else
   {
      mpExecutor->setInteractive();
   }

   bool bSuccess = false;

   // Create the input arg list
   PlugInArgList* pIn = NULL;
   mpExecutor->getInputSpecification(pIn);
   if (pIn != NULL)
   {
      // Setup input parameters.
      PlugInArg* pArg = NULL;
      if (pIn->getArg(ProgressArg(), pArg) && (pArg != NULL))
      {
         pArg->setActualValue(mpProgress);
      }

      if (pIn->getArg("Wizard", pArg) && (pArg != NULL))
      {
         pArg->setActualValue(pWizard);
      }

      // Execute the wizard
      bSuccess = mpExecutor->execute(pIn, NULL);

      // Destroy the plug-in arg list
      mpPlugInManager->destroyPlugInArgList(pIn);
   }

   // Clean up
   delete mpExecutor;
   mpExecutor = NULL;

   return bSuccess;
}
