/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QDir>
#include <QtGui/QFileDialog>

#include "AppVersion.h"
#include "AppAssert.h"
#include "AppVerify.h"
#include "DataDescriptor.h"
#include "DataElement.h"
#include "DesktopServices.h"
#include "Executable.h"
#include "FileDescriptor.h"
#include "Filename.h"
#include "MessageLogResource.h"
#include "ModelServices.h"
#include "ObjectFactory.h"
#include "ObjectResource.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "Progress.h"
#include "Slot.h"
#include "UtilityServices.h"
#include "View.h"
#include "WizardExecutor.h"
#include "WizardItem.h"
#include "WizardNode.h"
#include "WizardObject.h"
#include "xmlreader.h"

using namespace std;

WizardExecutor::WizardExecutor() :
   mbInteractive(false),
   mbAbort(false),
   mbDeleteWizard(false),
   mpProgress(NULL),
   mpWizard(NULL),
   mpCurrentPlugIn(NULL),
   mpStep(NULL)
{
   setName("Wizard Executor");
   setVersion(APP_VERSION_NUMBER);
   setCreator("Ball Aerospace & Technologies, Corp.");
   setCopyright(APP_COPYRIGHT);
   setShortDescription("Wizard Executor");
   setDescription("Wizard Executor");
   setDescriptorId("{1D312E5F-1501-4445-BFA6-BF9FA5427ABD}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

WizardExecutor::~WizardExecutor()
{
}

void WizardExecutor::wizardDeleted(Subject &subject, const string &signal, const boost::any &v)
{
   if (&subject == mpWizard)
   {
      mpWizard = NULL;
   }
}

bool WizardExecutor::setBatch()
{
   mbInteractive = false;
   return true;
}

bool WizardExecutor::setInteractive()
{
   mbInteractive = true;
   return true;
}

bool WizardExecutor::hasAbort()
{
   bool bSuccess = true;
   if(mpCurrentPlugIn != NULL)
   {
      bSuccess = mpCurrentPlugIn->hasAbort();
   }

   return bSuccess;
}

bool WizardExecutor::getInputSpecification(PlugInArgList*& pArgList)
{
   Service<PlugInManagerServices> pPlugInManager;
   VERIFY(pPlugInManager.get() != NULL);

   // Set up list
   pArgList = pPlugInManager->getPlugInArgList();
   VERIFY(pArgList != NULL);

   PlugInArg* pArg = pPlugInManager->getPlugInArg();      // Progress
   VERIFY(pArg != NULL);
   pArg->setName(ProgressArg());
   pArg->setType("Progress");
   pArg->setDefaultValue(NULL);
   pArgList->addArg(*pArg);

   pArg = pPlugInManager->getPlugInArg();      // Wizard object
   VERIFY(pArg != NULL);
   pArg->setName("Wizard");
   pArg->setType("WizardObject");
   pArg->setDefaultValue(NULL);
   pArgList->addArg(*pArg);

   pArg = pPlugInManager->getPlugInArg();      // Filename
   VERIFY(pArg != NULL);
   pArg->setName("Filename");
   pArg->setType("Filename");
   pArg->setDefaultValue(NULL);
   pArgList->addArg(*pArg);

   return true;
}

bool WizardExecutor::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Run Wizard Executor", "app", "68D30CE9-3B60-4294-ADC2-69A838438542");
   mpStep = pStep.get();
   
   if(!extractInputArgs(pInArgList))
   {
      return false;
   }

   if(mpWizard == NULL)
   {
      mMessage = "The wizard is invalid!";
      if(mpProgress != NULL) mpProgress->updateProgress(mMessage, 0, ERRORS);
      pStep->finalize(Message::Failure, mMessage);
      return false;
   }

   pStep->addProperty("wizard", mpWizard->getName());

   mpWizard->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &WizardExecutor::wizardDeleted));

   mMessage = "Starting wizard execution";
   if(mpProgress != NULL) mpProgress->updateProgress(mMessage, 0, NORMAL);
   pStep->addMessage(mMessage, "app", "2D53370C-DD0D-4160-9BD5-4D46C49A4B7E", true);

   const vector<WizardItem*>& wizardItems = mpWizard->getItems();
   
   for(vector<WizardItem*>::const_iterator wiIter = wizardItems.begin(); wiIter != wizardItems.end(); ++wiIter)
   {
      WizardItem* pItem = *wiIter;
      if(pItem != NULL)
      {
         bool bSuccess = false;

         string itemName = pItem->getName();
         string itemType = pItem->getType();
         mMessage = "Executing " + itemType + " Item: " + itemName;
         pStep->addMessage(mMessage, "app", "9FC4024E-00FA-42cd-8EC3-2AAE84843BA7", true);

         if(itemType == "Value")
         {
            bSuccess = true;
            setConnectedNodeValues(pItem);
         }
         else
         {
            bSuccess = launchPlugIn(pItem);
            resetNodeValues(pItem);
         }

         if(mbAbort)
         {
            resetAllNodeValues();

            mMessage = "Wizard Exector Aborted!";
            pStep->finalize(Message::Abort, mMessage);

            if(mbDeleteWizard)
            {
               mpObjFact->destroyObject(mpWizard, "WizardObject");
            }
            else if(mpWizard != NULL)
            {
               mpWizard->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &WizardExecutor::wizardDeleted));
            }

            return false;
         }

         if(mpWizard == NULL)
         {
            mMessage = "The wizard is no longer valid!  Execution will be terminated.";
            if(mpProgress != NULL) mpProgress->updateProgress(mMessage, 0, ERRORS);
            pStep->finalize(Message::Failure, mMessage);
            return false;
         }

         if(!bSuccess)
         {
            resetAllNodeValues();

            if(mbDeleteWizard)
            {
               mpObjFact->destroyObject(mpWizard, "WizardObject");
            }
            else if(mpWizard != NULL)
            {
               mpWizard->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &WizardExecutor::wizardDeleted));
            }

            pStep->finalize(Message::Failure);
            return false;
         }
      }
   }

   mMessage = "Wizard complete.";
   if(mpProgress != NULL) mpProgress->updateProgress(mMessage, 100, NORMAL);
   pStep->finalize(Message::Success);

   if(mbDeleteWizard)
   {
      mpObjFact->destroyObject(mpWizard, "WizardObject");
   }
   else if(mpWizard != NULL)
   {
      mpWizard->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &WizardExecutor::wizardDeleted));
   }

   return true;
}

bool WizardExecutor::abort()
{
   mbAbort = true;

   bool bSuccess = true;
   if(mpCurrentPlugIn != NULL)
   {
      bSuccess = mpCurrentPlugIn->abort();
   }

   return bSuccess;
}

bool WizardExecutor::extractInputArgs(PlugInArgList* pInArgList)
{
   if (pInArgList == NULL)
   {
      return false;
   }

   PlugInArg* pArg = NULL;

   // Progress
   if ((pInArgList->getArg(ProgressArg(), pArg) == true) && (pArg != NULL))
   {
      mpProgress = pArg->getPlugInArgValue<Progress>();
   }

   // Wizard object
   mpWizard = NULL;
   if ((pInArgList->getArg("Wizard", pArg) == true) && (pArg != NULL))
   {
      mpWizard = pArg->getPlugInArgValue<WizardObject>();
   }

   if (mpWizard != NULL)
   {
      return true;
   }

   // Filename
   if(!pInArgList->getArg("Filename", pArg) || (pArg == NULL))
   {
      mMessage = "Could not read the filename input value!";
      if(mpProgress != NULL) mpProgress->updateProgress(mMessage, 0, ERRORS);
      mpStep->finalize(Message::Failure, mMessage);
      return false;
   }

   Filename* pFilename = pArg->getPlugInArgValue<Filename>();
   string filename;
   if(pFilename == NULL)
   {
      if(mbInteractive)
      {
         QString strDefaultDir = QDir::currentPath();

         QString strFilename = QFileDialog::getOpenFileName(mpDesktop->getMainWidget(), QString(),
            strDefaultDir, "Wizard Files (*.wiz)");
         if(strFilename.isEmpty())
         {
            mpStep->finalize(Message::Failure, "No wizard file specified.");
            return false;
         }

         filename = strFilename.toStdString();
      }
   }
   else
   {
      filename = pFilename->getFullPathAndName();
   }

   if(filename.empty())
   {
      mMessage = "The filename input value is invalid!";
      if(mpProgress != NULL) mpProgress->updateProgress(mMessage, 0, ERRORS);
      mpStep->finalize(Message::Failure, mMessage);
      return false;
   }

   mpWizard = reinterpret_cast<WizardObject*>(mpObjFact->createObject("WizardObject"));

   if (mpWizard == NULL)
   {
      mMessage = "Could not create the wizard!";
      if(mpProgress != NULL) mpProgress->updateProgress(mMessage, 0, ERRORS);
      mpStep->finalize(Message::Failure, mMessage);
      return false;
   }

   bool bSuccess = false;

   FactoryResource<Filename> pWizardFilename;
   pWizardFilename->setFullPathAndName(filename);

   XmlReader xml;
   XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* pDocument = xml.parse(pWizardFilename.get());
   if (pDocument != NULL)
   {
      XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* pRootElement = pDocument->getDocumentElement();
      if (pRootElement != NULL)
      {
         unsigned int version = atoi(A(pRootElement->getAttribute(X("version"))));
         bSuccess = mpWizard->fromXml(pRootElement, version);
      }
   }

   if (bSuccess == false)
   {
      mMessage = "Could not load the wizard from the file!";
      if(mpProgress != NULL) mpProgress->updateProgress(mMessage, 0, ERRORS);
      mpStep->finalize(Message::Failure, mMessage);

      mpObjFact->destroyObject(mpWizard, "WizardObject");

      mpWizard = NULL;
      return false;
   }

   mpStep->addProperty("wizardFile", filename);
   mbDeleteWizard = true;
   return true;
}

void WizardExecutor::populatePlugInArgList(PlugInArgList* pArgList, WizardItem* pItem, bool bInArgs)
{
   if ((pArgList == NULL) || (pItem == NULL))
   {
      return;
   }

   // Get the wizard nodes
   vector<WizardNode*> nodes;

   if(bInArgs)
   {
      nodes = pItem->getInputNodes();
   }
   else
   {
      nodes = pItem->getOutputNodes();
   }

   // Set the plug-in arg actual values from the node values
   for(int i = 0; i < pArgList->getCount(); i++)
   {
      PlugInArg* pArg = NULL;

      if(pArgList->getArg(i, pArg) && (pArg != NULL))
      {
         string argName = pArg->getName();
         string argType = pArg->getType();

         vector<WizardNode*>::iterator iter;
         for(iter = nodes.begin(); iter != nodes.end(); ++iter)
         {
            WizardNode* pNode = *iter;
            if(pNode != NULL)
            {
               string nodeName = pNode->getName();
               string nodeType = pNode->getOriginalType();
               if(nodeType.empty())
               {
                  nodeType = pNode->getType();
               }

               if((nodeName == argName) && (nodeType == argType))
               {
                  void* pValue = pNode->getValue();
                  if(pValue != NULL)
                  {
                     pArg->setActualValue(pValue);
                  }
                  else if(nodeType == "Progress" && bInArgs)
                  {
                     // only for input args - bkg plugin must set output arg to its Progress arg
                     pArg->setActualValue(mpProgress);
                  }

                  break;
               }
            }
         }

         if(iter == nodes.end())
         {
            if(argType == "Progress")
            {
               pArg->setActualValue(mpProgress);
            }
         }
      }
   }
}

bool WizardExecutor::launchPlugIn(WizardItem* pItem)
{
   VERIFY(pItem != NULL);

   bool pluginExecuteStatus = false;
   try
   {
      Service<PlugInManagerServices> pPlugInManager;

      string itemName = pItem->getName();

      PlugIn* pPlugIn = pPlugInManager->createPlugIn(itemName);
      if (pPlugIn == NULL)
      {
         mMessage = "The " + itemName + " plug-in could not be created! Wizard execution will be terminated.";
         if(mpProgress != NULL) mpProgress->updateProgress(mMessage, 0, ERRORS);
         mpStep->finalize(Message::Failure, mMessage);
         return false;
      }

      Executable* pExecutable = dynamic_cast<Executable*>(pPlugIn);
      if (pExecutable == NULL)
      {
         pPlugInManager->destroyPlugIn(pPlugIn);
         return false;
      }

      bool bShouldDestroyPlugin = pExecutable->isDestroyedAfterExecute();

      // Set the execution mode
      bool bBatch = pItem->getBatchMode();
      bool bSupportedMode = true;
      string executeMode;

      if(!bBatch)
      {
         bSupportedMode = pExecutable->setInteractive();
         executeMode = "interactive";
      }
      else
      {
         bSupportedMode = pExecutable->setBatch();
         executeMode = "batch";
      }

      if(!bSupportedMode)
      {
         mMessage = "The " + itemName + " plug-in does not support " + executeMode + " mode!";
         if(mpProgress != NULL) mpProgress->updateProgress(mMessage, 0, ERRORS);
         MessageResource pMsg("Plug-in does not support execution model!", "app",
                                    "2A7C88B5-3A5B-41be-836A-8F5C30FF485D");
         pMsg->addProperty("mode", executeMode);
         pMsg->addProperty("plug-in", itemName);
         mpStep->finalize(Message::Failure, mMessage);
         bShouldDestroyPlugin = true;
      }
      else
      {
         // Build the plug-in arg lists
         PlugInArgList* pInArgList = NULL;
         PlugInArgList* pOutArgList = NULL;
         pExecutable->getInputSpecification(pInArgList);
         pExecutable->getOutputSpecification(pOutArgList);

         populatePlugInArgList(pInArgList, pItem, true);
         populatePlugInArgList(pOutArgList, pItem, false);

         // Set the current plug-in
         mpCurrentPlugIn = pExecutable;

         // Execute the plug-in
         if(pluginExecuteStatus = pExecutable->execute(pInArgList, pOutArgList))
         {
            setConnectedNodeValues(pItem, pOutArgList);
         }

         if(pluginExecuteStatus && pExecutable->isBackground() && bBatch == false)
         {
            // background plug-in must have non-NULL Progress instance in output arg list
            // for it to be added to the Background Window
            Progress  *pReturnedProgress(NULL);
            PlugInArg *pArg(NULL);

            if (pOutArgList != NULL)
            {
               if(pOutArgList->getArg(ProgressArg(), pArg) && (pArg != NULL))
               {
                  pReturnedProgress = pArg->getPlugInArgValue<Progress>();
               }

            }

            if (pReturnedProgress != NULL)
            {
               if (mpDesktop.get() != NULL)
               {
                  mpDesktop->addBackgroundPlugIn(pPlugIn, pReturnedProgress);
                  bShouldDestroyPlugin = false;
               }
            }
            else
            {
               mMessage = "The " + itemName + " background plug-in did not " +
                  "return a valid Progress object and was not added to the Background Plug-In Window.";
               if (mpProgress != NULL) mpProgress->updateProgress(mMessage, 0, ERRORS);
               MessageResource pMsg("Plug-in does not support being run in background!", "app",
                  "C60D7F31-C3C2-4e40-8633-5CC1C8370ADF");
               pMsg->addProperty("plug-in", itemName);
               mpStep->finalize(Message::Failure, mMessage);
               bShouldDestroyPlugin = false;  // need to abort and let plug-in clean up threads
               pExecutable->abort(); 
               pluginExecuteStatus = false;
            }
         }

         pPlugInManager->destroyPlugInArgList(pInArgList);
         pPlugInManager->destroyPlugInArgList(pOutArgList);
      }

      // Cleanup
      mpCurrentPlugIn = NULL;

      if(bShouldDestroyPlugin)
      {
         pPlugInManager->destroyPlugIn(pPlugIn);
      }
   }
   catch(AssertException exc)
   {
      mpStep->finalize(Message::Failure, exc.getText());
      pluginExecuteStatus = false;
   }

   return pluginExecuteStatus;
}

void WizardExecutor::setConnectedNodeValues(WizardItem* pItem, PlugInArgList* pOutArgList)
{
   VERIFYNRV(pItem != NULL);

   // Set the connected item input node values
   const vector<WizardNode*>& outputNodes = pItem->getOutputNodes();

   for(vector<WizardNode*>::const_iterator onIter = outputNodes.begin(); onIter != outputNodes.end(); ++onIter)
   {
      WizardNode* pNode = *onIter;
      if(pNode != NULL)
      {
         string nodeName = pNode->getName();
         string nodeType = pNode->getOriginalType();
         if(nodeType.empty())
         {
            nodeType = pNode->getType();
         }

         void* pValue = NULL;

         if(pOutArgList == NULL)
         {
            pValue = pNode->getValue();
         }
         else
         {
            PlugInArg* pArg = NULL;

            if(pOutArgList->getArg(nodeName, pArg) && (pArg != NULL))
            {
               if(pArg->isActualSet())
               {
                  pValue = pArg->getActualValue();
               }
               else if(nodeType == "Progress")
               {
                  pValue = static_cast<void*>(mpProgress);
               }
               else if(pArg->isDefaultSet())
               {
                  pValue = pArg->getDefaultValue();
               }
            }
         }

         const vector<WizardNode*>& connectedNodes = pNode->getConnectedNodes();

         for(vector<WizardNode*>::const_iterator cnIter = connectedNodes.begin();
                                                 cnIter != connectedNodes.end();
                                                 ++cnIter)
         {
            WizardNode* pConnectedNode = *cnIter;
            if (pConnectedNode != NULL)
            {
               string connectedNodeName = pConnectedNode->getName();
               string connectedNodeType = pConnectedNode->getOriginalType();
               if(connectedNodeType.empty())
               {
                  connectedNodeType = pConnectedNode->getType();
               }

               bool bValidKind = (nodeType == connectedNodeType);

               if(!bValidKind)
               {
                  Service<ModelServices> pModel;

                  bValidKind = pModel->isKindOfElement(nodeType, connectedNodeType);
                  if(!bValidKind)
                  {
                     bool bElement = pModel->isKindOfElement(nodeType, "DataElement");
                     if ((bElement == true) && (pValue != NULL))
                     {
                        bValidKind = (reinterpret_cast<DataElement*>(pValue))->isKindOf(connectedNodeType);
                     }
                  }

                  if (bValidKind == false)
                  {
                     bValidKind = pModel->isKindOfDataDescriptor(nodeType, connectedNodeType);
                     if (bValidKind == false)
                     {
                        bool bDataDescriptor = pModel->isKindOfDataDescriptor(nodeType, "DataDescriptor");
                        if ((bDataDescriptor == true) && (pValue != NULL))
                        {
                           bValidKind = (reinterpret_cast<DataDescriptor*>(pValue))->isKindOf(connectedNodeType);
                        }
                     }

                     if (bValidKind == false)
                     {
                        bValidKind = pModel->isKindOfFileDescriptor(nodeType, connectedNodeType);
                        if (bValidKind == false)
                        {
                           bool bFileDescriptor = pModel->isKindOfFileDescriptor(nodeType, "FileDescriptor");
                           if ((bFileDescriptor == true) && (pValue != NULL))
                           {
                              bValidKind = (reinterpret_cast<FileDescriptor*>(pValue))->isKindOf(connectedNodeType);
                           }
                        }
                     }
                  }
               }

               if(!bValidKind)
               {
                  bValidKind = mpDesktop->isKindOfView(nodeType, connectedNodeType);
                  if(!bValidKind)
                  {
                     bool bView = mpDesktop->isKindOfView(nodeType, "View");
                     if ((bView == true) && (pValue != NULL))
                     {
                        bValidKind = reinterpret_cast<View*>(pValue)->isKindOf(connectedNodeType);
                     }
                  }
               }

               if(bValidKind)
               {
                  pConnectedNode->setValue(pValue);
               }
               else
               {
                  string itemName = pItem->getName();
                  string connectedItemName = "";

                  WizardItem* pConnectedItem = pConnectedNode->getItem();
                  if(pConnectedItem != NULL)
                  {
                     string name = pConnectedItem->getName();
                     if(!name.empty())
                     {
                        connectedItemName = " on the " + name + " item";
                     }
                  }

                  mMessage = "Could not set the " + connectedNodeName + " input node value" +
                              connectedItemName + "! The node type is incompatible with the " +
                              nodeName + " connected node type on the " + itemName + " item.";
                  if(mpProgress != NULL) mpProgress->updateProgress(mMessage, 0, WARNING);
                  mpStep->addMessage(mMessage, "app", "CFA1428A-E382-4b2b-8A23-0C10F8BA97EC", true);
               }
            }
         }
      }
   }
}

void WizardExecutor::resetNodeValues(WizardItem* pItem)
{
   VERIFYNRV(pItem != NULL);

   const vector<WizardNode*>& inputNodes = pItem->getInputNodes();
   const vector<WizardNode*>& outputNodes = pItem->getOutputNodes();

   for(vector<WizardNode*>::const_iterator inIter = inputNodes.begin(); inIter != inputNodes.end(); ++inIter)
   {
      WizardNode* pNode = *inIter;
      if(pNode != NULL)
      {
         pNode->setValue(NULL);
      }
   }

   for(vector<WizardNode*>::const_iterator onIter = outputNodes.begin(); onIter != outputNodes.end(); ++onIter)
   {
      WizardNode* pNode = *onIter;
      if(pNode != NULL)
      {
         pNode->setValue(NULL);
      }
   }
}

void WizardExecutor::resetAllNodeValues()
{
   if(mpWizard == NULL)
   {
      return;
   }

   const vector<WizardItem*>& items = mpWizard->getItems();

   for(vector<WizardItem*>::const_iterator iter = items.begin(); iter != items.end(); ++iter)
   {
      WizardItem* pItem = *iter;
      if(pItem != NULL)
      {
         const string& itemType = pItem->getType();
         if(itemType != "Value")
         {
            resetNodeValues(pItem);
         }
      }
   }
}
