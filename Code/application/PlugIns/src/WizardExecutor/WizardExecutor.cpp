/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "AppAssert.h"
#include "AppVerify.h"
#include "DataDescriptor.h"
#include "DataElement.h"
#include "DesktopServices.h"
#include "Executable.h"
#include "FileDescriptor.h"
#include "Filename.h"
#include "Layer.h"
#include "MessageLogResource.h"
#include "ModelServices.h"
#include "ObjectFactory.h"
#include "ObjectResource.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "PlugInResource.h"
#include "Progress.h"
#include "Slot.h"
#include "UtilityServices.h"
#include "View.h"
#include "WizardExecutor.h"
#include "WizardItem.h"
#include "WizardNode.h"
#include "WizardObject.h"
#include "WizardUtilities.h"
#include "xmlreader.h"

#include <QtCore/QDir>
#include <QtGui/QFileDialog>

using namespace std;

REGISTER_PLUGIN_BASIC(OpticksWizardExecutor, WizardExecutor);

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
   if (mpCurrentPlugIn != NULL)
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

   VERIFY(pArgList->addArg<Progress>(Executable::ProgressArg(), NULL));
   VERIFY(pArgList->addArg<WizardObject>("Wizard", NULL));
   VERIFY(pArgList->addArg<Filename>("Filename", NULL));

   return true;
}

bool WizardExecutor::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Run Wizard Executor", "app", "68D30CE9-3B60-4294-ADC2-69A838438542");
   mpStep = pStep.get();

   if (!extractInputArgs(pInArgList))
   {
      return false;
   }

   if (mpWizard == NULL)
   {
      mMessage = "The wizard is invalid!";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(mMessage, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, mMessage);
      return false;
   }

   const vector<WizardItem*>& wizardItems = mpWizard->getItems();
   if (wizardItems.empty() == true)
   {
      mMessage = "There are no items in the wizard to execute.";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(mMessage, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, mMessage);
      return false;
   }

   pStep->addProperty("wizard", mpWizard->getName());

   mpWizard->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &WizardExecutor::wizardDeleted));

   mMessage = "Starting wizard execution";
   if (mpProgress != NULL)
   {
      mpProgress->updateProgress(mMessage, 0, NORMAL);
   }

   pStep->addMessage(mMessage, "app", "2D53370C-DD0D-4160-9BD5-4D46C49A4B7E", true);

   vector<WizardItem*> populateList;
   for (vector<WizardItem*>::const_iterator wiIter = wizardItems.begin(); wiIter != wizardItems.end(); ++wiIter)
   {
      bool bSuccess = false;

      WizardItem* pItem = *wiIter;
      VERIFY(pItem);
      string itemName = pItem->getName();
      string itemType = pItem->getType();

      // build a list of all interactive mode Value items in a row
      // then prompt the user for values on all these items at once
      // finally, continue execution normally
      if (itemType == "Value" && !pItem->getBatchMode())
      {
         populateList.push_back(*wiIter);
         continue;
      }
      if (!WizardUtilities::editItems(populateList, Service<DesktopServices>()->getMainWidget()))
      {
         mMessage = "Wizard cancelled by user.";
         if (mbDeleteWizard)
         {
            mpObjFact->destroyObject(mpWizard, "WizardObject");
         }
         else if (mpWizard != NULL)
         {
            mpWizard->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &WizardExecutor::wizardDeleted));
         }
         if (mpProgress != NULL)
         {
            mpProgress->updateProgress(mMessage, 0, ABORT);
         }
         pStep->finalize(Message::Abort, mMessage);
         return false;
      }
      populateList.push_back(*wiIter);
      for (vector<WizardItem*>::const_iterator plIter = populateList.begin(); plIter != populateList.end(); ++plIter)
      {
         if ((*plIter)->getType() == "Value")
         {
            mMessage = "Executing Value Item: " + (*plIter)->getName();
            pStep->addMessage(mMessage, "app", "9FC4024E-00FA-42cd-8EC3-2AAE84843BA7", true);
            bSuccess = true;
            setConnectedNodeValues(*plIter);
         }
         else
         {
            bSuccess = launchPlugIn(*plIter);
            resetNodeValues(*plIter);
         }
      }
      populateList.clear();

      if (mbAbort)
      {
         resetAllNodeValues();

         mMessage = "Wizard Exector Aborted!";
         pStep->finalize(Message::Abort, mMessage);
         if (mpProgress != NULL)
         {
            string progressMessage;
            int percent = 0;
            ReportingLevel level;
            mpProgress->getProgress(progressMessage, percent, level);

            if (level != ABORT)
            {
               mpProgress->updateProgress(mMessage, 0, ABORT);
            }
         }
         if (mbDeleteWizard)
         {
            mpObjFact->destroyObject(mpWizard, "WizardObject");
         }
         else if (mpWizard != NULL)
         {
            mpWizard->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &WizardExecutor::wizardDeleted));
         }

         return false;
      }

      if (mpWizard == NULL)
      {
         mMessage = "The wizard is no longer valid!  Execution will be terminated.";
         if (mpProgress != NULL)
         {
            mpProgress->updateProgress(mMessage, 0, ERRORS);
         }

         pStep->finalize(Message::Failure, mMessage);
         return false;
      }
      if (!bSuccess)
      {
         resetAllNodeValues();

         if (mpProgress != NULL)
         {
            string progressMessage;
            int percent = 0;
            ReportingLevel level;
            mpProgress->getProgress(progressMessage, percent, level);

            if (level != ERRORS)
            {
               mpProgress->updateProgress("The wizard failed to complete successfully.", 0, ERRORS);
            }
         }

         if (mbDeleteWizard)
         {
            mpObjFact->destroyObject(mpWizard, "WizardObject");
         }
         else if (mpWizard != NULL)
         {
            mpWizard->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &WizardExecutor::wizardDeleted));
         }

         pStep->finalize(Message::Failure);
         return false;
      }
   }
   if (!populateList.empty() && mpProgress != NULL)
   {
      mpProgress->updateProgress(QString("%1 value item(s) ignored. They are not connected to any wizard items.")
         .arg(populateList.size()).toStdString(), 0, WARNING);
   }

   mMessage = "Wizard complete.";
   if (mpProgress != NULL)
   {
      mpProgress->updateProgress(mMessage, 100, NORMAL);
   }

   pStep->finalize(Message::Success);

   if (mbDeleteWizard)
   {
      mpObjFact->destroyObject(mpWizard, "WizardObject");
   }
   else if (mpWizard != NULL)
   {
      mpWizard->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &WizardExecutor::wizardDeleted));
   }

   return true;
}

bool WizardExecutor::abort()
{
   mbAbort = true;

   bool bSuccess = true;
   if (mpCurrentPlugIn != NULL)
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
   if (!pInArgList->getArg("Filename", pArg) || (pArg == NULL))
   {
      mMessage = "Could not read the filename input value!";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(mMessage, 0, ERRORS);
      }

      mpStep->finalize(Message::Failure, mMessage);
      return false;
   }

   Filename* pFilename = pArg->getPlugInArgValue<Filename>();
   string filename;
   if (pFilename == NULL)
   {
      if (mbInteractive)
      {
         QString strDefaultDir = QDir::currentPath();

         QString strFilename = QFileDialog::getOpenFileName(mpDesktop->getMainWidget(), QString(),
            strDefaultDir, "Wizard Files (*.wiz)");
         if (strFilename.isEmpty())
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

   if (filename.empty())
   {
      mMessage = "The filename input value is invalid!";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(mMessage, 0, ERRORS);
      }

      mpStep->finalize(Message::Failure, mMessage);
      return false;
   }

   mpWizard = reinterpret_cast<WizardObject*>(mpObjFact->createObject("WizardObject"));

   if (mpWizard == NULL)
   {
      mMessage = "Could not create the wizard!";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(mMessage, 0, ERRORS);
      }

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
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(mMessage, 0, ERRORS);
      }

      mpStep->finalize(Message::Failure, mMessage);

      mpObjFact->destroyObject(mpWizard, "WizardObject");

      mpWizard = NULL;
      return false;
   }

   mpStep->addProperty("wizardFile", filename);
   mbDeleteWizard = true;
   return true;
}

void WizardExecutor::populatePlugInArgList(PlugInArgList* pArgList, const WizardItem* pItem, bool bInArgs)
{
   if ((pArgList == NULL) || (pItem == NULL))
   {
      return;
   }

   // Get the wizard nodes
   vector<WizardNode*> nodes;

   if (bInArgs)
   {
      nodes = pItem->getInputNodes();
   }
   else
   {
      nodes = pItem->getOutputNodes();
   }

   // Set the plug-in arg actual values from the node values
   for (int i = 0; i < pArgList->getCount(); i++)
   {
      PlugInArg* pArg = NULL;

      if (pArgList->getArg(i, pArg) && (pArg != NULL))
      {
         string argName = pArg->getName();
         string argType = pArg->getType();

         vector<WizardNode*>::iterator iter;
         for (iter = nodes.begin(); iter != nodes.end(); ++iter)
         {
            WizardNode* pNode = *iter;
            if (pNode != NULL)
            {
               string nodeName = pNode->getName();
               string nodeType = pNode->getOriginalType();
               if (nodeType.empty())
               {
                  nodeType = pNode->getType();
               }

               if ((nodeName == argName) && (nodeType == argType))
               {
                  void* pValue = pNode->getValue();
                  if (pValue != NULL)
                  {
                     pArg->setActualValue(pValue);
                  }
                  else if (nodeType == TypeConverter::toString<Progress>() && bInArgs)
                  {
                     // only for input args - bkg plugin must set output arg to its Progress arg
                     pArg->setActualValue(mpProgress);
                  }

                  break;
               }
            }
         }

         if (iter == nodes.end())
         {
            if (argType == TypeConverter::toString<Progress>())
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

   StepResource pStep("Executing " + pItem->getType() + " Item: " + pItem->getName(), "app",
      "6A743B49-618B-44ed-9C5A-B4D67FB809D2");

   bool pluginExecuteStatus = false;

   ExecutableResource pExecutable(pItem->getName(), "", mpProgress, pItem->getBatchMode());

   pExecutable->setAutoArg(false);
   try
   {
      // Set the current plug-in
      mpCurrentPlugIn = dynamic_cast<Executable*>(pExecutable->getPlugIn());

      if (mpCurrentPlugIn != NULL)
      {
          populatePlugInArgList(&pExecutable->getInArgList(), pItem, true);
          populatePlugInArgList(&pExecutable->getOutArgList(), pItem, false);

          pluginExecuteStatus = pExecutable->execute();

          // Execute the plug-in
          if (pluginExecuteStatus)
          {
             setConnectedNodeValues(pItem, &pExecutable->getOutArgList());
             pStep->finalize(Message::Success);
          }
          else
          {
             pStep->finalize(Message::Failure);
          }
      }
      else
      {
         mMessage = "The " + pItem->getName() + " plug-in could not be created! Wizard execution will be terminated.";
         if (mpProgress != NULL)
         {
            mpProgress->updateProgress(mMessage, 0, ERRORS);
         }

         pStep->finalize(Message::Failure, mMessage);
      }

   }
   catch (AssertException exc)
   {
      if (mpProgress != NULL) 
      {
         mpProgress->updateProgress(exc.getText(), 0, ERRORS);
      }
      pStep->finalize(Message::Failure, exc.getText());
      pluginExecuteStatus = false;
   }

   // Cleanup
   mpCurrentPlugIn = NULL;

   return pluginExecuteStatus;
}

void WizardExecutor::setConnectedNodeValues(WizardItem* pItem, PlugInArgList* pOutArgList)
{
   VERIFYNRV(pItem != NULL);

   // Set the connected item input node values
   const vector<WizardNode*>& outputNodes = pItem->getOutputNodes();

   for (vector<WizardNode*>::const_iterator onIter = outputNodes.begin(); onIter != outputNodes.end(); ++onIter)
   {
      WizardNode* pNode = *onIter;
      if (pNode != NULL)
      {
         string nodeName = pNode->getName();
         string nodeType = pNode->getOriginalType();
         if (nodeType.empty())
         {
            nodeType = pNode->getType();
         }

         void* pValue = NULL;

         if (pOutArgList == NULL)
         {
            pValue = pNode->getValue();
         }
         else
         {
            PlugInArg* pArg = NULL;

            if (pOutArgList->getArg(nodeName, pArg) && (pArg != NULL))
            {
               if (pArg->isActualSet())
               {
                  pValue = pArg->getActualValue();
               }
               else if (nodeType == TypeConverter::toString<Progress>())
               {
                  pValue = static_cast<void*>(mpProgress);
               }
               else if (pArg->isDefaultSet())
               {
                  pValue = pArg->getDefaultValue();
               }
            }
         }

         const vector<WizardNode*>& connectedNodes = pNode->getConnectedNodes();

         for (vector<WizardNode*>::const_iterator cnIter = connectedNodes.begin();
                                                 cnIter != connectedNodes.end();
                                                 ++cnIter)
         {
            WizardNode* pConnectedNode = *cnIter;
            if (pConnectedNode != NULL)
            {
               string connectedNodeName = pConnectedNode->getName();
               string connectedNodeType = pConnectedNode->getOriginalType();
               if (connectedNodeType.empty())
               {
                  connectedNodeType = pConnectedNode->getType();
               }

               bool bValidKind = (nodeType == connectedNodeType);

               if (!bValidKind)
               {
                  Service<ModelServices> pModel;

                  bValidKind = pModel->isKindOfElement(nodeType, connectedNodeType);
                  if (!bValidKind)
                  {
                     bool bElement = pModel->isKindOfElement(nodeType, TypeConverter::toString<DataElement>());
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
                        bool bDataDescriptor = pModel->isKindOfDataDescriptor(nodeType,
                           TypeConverter::toString<DataDescriptor>());
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
                           bool bFileDescriptor = pModel->isKindOfFileDescriptor(nodeType,
                              TypeConverter::toString<FileDescriptor>());
                           if ((bFileDescriptor == true) && (pValue != NULL))
                           {
                              bValidKind = (reinterpret_cast<FileDescriptor*>(pValue))->isKindOf(connectedNodeType);
                           }
                        }
                     }
                  }
               }

               if (!bValidKind)
               {
                  bValidKind = mpDesktop->isKindOfView(nodeType, connectedNodeType);
                  if (!bValidKind)
                  {
                     bool bView = mpDesktop->isKindOfView(nodeType, TypeConverter::toString<View>());
                     if ((bView == true) && (pValue != NULL))
                     {
                        bValidKind = reinterpret_cast<View*>(pValue)->isKindOf(connectedNodeType);
                     }
                  }
               }
               if (!bValidKind)
               {
                  bValidKind = mpDesktop->isKindOfLayer(nodeType, connectedNodeType);
                  if (!bValidKind)
                  {
                     bool bLayer = mpDesktop->isKindOfLayer(nodeType, TypeConverter::toString<Layer>());
                     if ((bLayer == true) && (pValue != NULL))
                     {
                        bValidKind = reinterpret_cast<Layer*>(pValue)->isKindOf(connectedNodeType);
                     }
                  }
               }

               if (bValidKind)
               {
                  pConnectedNode->setValue(pValue);
               }
               else
               {
                  string itemName = pItem->getName();
                  string connectedItemName = "";

                  WizardItem* pConnectedItem = pConnectedNode->getItem();
                  if (pConnectedItem != NULL)
                  {
                     string name = pConnectedItem->getName();
                     if (!name.empty())
                     {
                        connectedItemName = " on the " + name + " item";
                     }
                  }

                  mMessage = "Could not set the " + connectedNodeName + " input node value" +
                              connectedItemName + "! The node type is incompatible with the " +
                              nodeName + " connected node type on the " + itemName + " item.";
                  if (mpProgress != NULL)
                  {
                     mpProgress->updateProgress(mMessage, 0, WARNING);
                  }

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

   for (vector<WizardNode*>::const_iterator inIter = inputNodes.begin(); inIter != inputNodes.end(); ++inIter)
   {
      WizardNode* pNode = *inIter;
      if (pNode != NULL)
      {
         pNode->setValue(NULL);
      }
   }

   for (vector<WizardNode*>::const_iterator onIter = outputNodes.begin(); onIter != outputNodes.end(); ++onIter)
   {
      WizardNode* pNode = *onIter;
      if (pNode != NULL)
      {
         pNode->setValue(NULL);
      }
   }
}

void WizardExecutor::resetAllNodeValues()
{
   if (mpWizard == NULL)
   {
      return;
   }

   const vector<WizardItem*>& items = mpWizard->getItems();

   for (vector<WizardItem*>::const_iterator iter = items.begin(); iter != items.end(); ++iter)
   {
      WizardItem* pItem = *iter;
      if (pItem != NULL)
      {
         const string& itemType = pItem->getType();
         if (itemType != "Value")
         {
            resetNodeValues(pItem);
         }
      }
   }
}
