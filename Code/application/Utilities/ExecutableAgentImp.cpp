/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ApplicationServices.h"
#include "DesktopServices.h"
#include "Executable.h"
#include "ExecutableAgentImp.h"
#include "GraphicLayer.h"
#include "LayerList.h"
#include "PlugIn.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "ProductView.h"
#include "Progress.h"
#include "RasterElement.h"
#include "SessionResource.h"
#include "SpatialDataView.h"
#include "Testable.h"
#include "UtilityServices.h"
#include "WorkspaceWindow.h"

#include <string>
#include <vector>
using namespace std;

ExecutableAgentImp::ExecutableAgentImp() :
   mInstantiated(false),
   mBatch(false),
   mSupportsRequestedBatchSetting(false),
   mpInArgList(NULL),
   mpOutArgList(NULL),
   mpProgress(NULL),
   mProgressDialog(false)
{
}

ExecutableAgentImp::~ExecutableAgentImp()
{
   clearArgLists();
}

void ExecutableAgentImp::instantiate(Progress* pProgress, bool batch)
{
   if (mInstantiated) throw logic_error("ExecutableAgent can not be instantiated twice!");
   mInstantiated = true;

   mPlugIn = PlugInResource();
   mpInArgList = NULL;
   mpOutArgList = NULL;
   mpProgress = pProgress;
   mBatch = batch;
   mSupportsRequestedBatchSetting = false;
   mMenuCommand = "";
}

void ExecutableAgentImp::instantiate(const string& plugInName, const string& menuCommand,
   Progress* pProgress, bool batch)
{
   if (mInstantiated) throw logic_error("ExecutableAgent can not be instantiated twice!");
   mInstantiated = true;

   mPlugIn = PlugInResource(plugInName);
   mpInArgList = NULL;
   mpOutArgList = NULL;
   mpProgress = pProgress;
   mBatch = batch;
   mSupportsRequestedBatchSetting = false;
   mMenuCommand = menuCommand;

   if (mpProgress == NULL)
   {
      Service<PlugInManagerServices> pPlugInManager;
      if (pPlugInManager.get() != NULL)
      {
         mpProgress = pPlugInManager->getProgress(getPlugIn());
      }
   }
}

void ExecutableAgentImp::instantiate(PlugIn* pPlugIn, const string& menuCommand,
   Progress* pProgress, bool batch)
{
   if (mInstantiated) throw logic_error("ExecutableAgent can not be instantiated twice!");
   mInstantiated = true;

   mPlugIn = PlugInResource(pPlugIn);
   mpInArgList = NULL;
   mpOutArgList = NULL;
   mpProgress = pProgress;
   mBatch = batch;
   mSupportsRequestedBatchSetting = false;
   mMenuCommand = menuCommand;

   if (mpProgress == NULL)
   {
      Service<PlugInManagerServices> pPlugInManager;
      if (pPlugInManager.get() != NULL)
      {
         mpProgress = pPlugInManager->getProgress(getPlugIn());
      }
   }
}

void ExecutableAgentImp::setPlugIn(const string& plugInName)
{
   checkInstantiate();
   clearArgLists();
   if (mpProgress != NULL)
   {
      Service<UtilityServices> pServices;
      pServices->destroyProgress(mpProgress);
      mpProgress = NULL; 
   }

   // Free the existing plug-in
   mPlugIn = PlugInResource();

   if (plugInName.empty() == false)
   {
      PlugInResource plugIn(plugInName);
      mPlugIn = plugIn;

      Service<PlugInManagerServices> pPlugInManager;
      if (pPlugInManager.get() != NULL)
      {
         mpProgress = pPlugInManager->getProgress(getPlugIn());
      }
   }
}

void ExecutableAgentImp::setPlugIn(PlugIn* pPlugIn)
{
   checkInstantiate();
   clearArgLists();
   if (mpProgress != NULL)
   {
      Service<UtilityServices> pServices;
      pServices->destroyProgress(mpProgress);
      mpProgress = NULL; 
   }

   // Free the existing plug-in
   mPlugIn = PlugInResource();

   if (pPlugIn != NULL)
   {
      PlugInResource plugIn(pPlugIn);
      mPlugIn = plugIn;

      Service<PlugInManagerServices> pPlugInManager;
      if (pPlugInManager.get() != NULL)
      {
         mpProgress = pPlugInManager->getProgress(getPlugIn());
      }
   }
}

Progress* ExecutableAgentImp::getProgress() const
{
   checkInstantiate();
   return mpProgress;
}

void ExecutableAgentImp::createProgressDialog(bool bCreate)
{
   checkInstantiate();

   Service<ApplicationServices> pApp;
   if (pApp->isInteractive() == true)
   {
      mProgressDialog = bCreate;
   }
}

void ExecutableAgentImp::createProgressDialog()
{
   checkInstantiate();
   if (mProgressDialog == false)
   {
      return;
   }

   const PlugIn* pPlugIn = getPlugIn();
   if (pPlugIn != NULL)
   {
      Service<DesktopServices> pDesktop;

      string name = pPlugIn->getName();
      pDesktop->createProgressDialog(name, getProgress());
   }
}

bool ExecutableAgentImp::isProgressDialogCreated() const
{
   checkInstantiate();
   return mProgressDialog;
}

bool ExecutableAgentImp::execute()
{
   checkInstantiate();
   createProgressDialog();
   return executePlugIn();
}

bool ExecutableAgentImp::executePlugIn()
{
   checkInstantiate();
   PlugIn* pPlugIn = getPlugIn();
   Executable* pExecutable = dynamic_cast<Executable*>(pPlugIn);
   if ((pExecutable == NULL) || (pPlugIn == NULL))
   {
      return false;
   }

   bool bSuccess = false;
   clearArgLists(false, true);

   PlugInArgList &inArgList = getPopulatedInArgList();
   PlugInArgList &outArgList = getOutArgList();
   { // scope the lifetime of the lock
      SessionSaveLock lock;
      if (mSupportsRequestedBatchSetting)
      {
         bSuccess = pExecutable->execute(&inArgList, &outArgList);
      }
      else
      {
         //plug-in doesn't support the batch or interactive mode requested
         bSuccess = false;
      }
   }
   if (bSuccess == true)
   {
      bool bBackground = pExecutable->isBackground();
      if ((bBackground == true) && (mBatch == false))
      {
         Progress* pProgress = outArgList.getPlugInArgValue<Progress>(Executable::ProgressArg());
         bool isValidBkgPlugIn = pProgress != NULL;
         Service<DesktopServices> pDesktop;
         if (pDesktop.get() != NULL && isValidBkgPlugIn)
         {
            pDesktop->addBackgroundPlugIn(pPlugIn, pProgress);
         }
         else
         {
            // use input arg Progress if available to notify user of error
            Progress* pInputProgress = inArgList.getPlugInArgValue<Progress>(Executable::ProgressArg());
            if (pInputProgress != NULL)
            {
               string errMsg = pPlugIn->getName();
               errMsg += " is not a valid background plug-in.\n";
               errMsg += "It must provide a Progress object in the output argument list.";
               pInputProgress->updateProgress(errMsg, 0, ERRORS);
            }
            pExecutable->abort();
            bSuccess = false;
         }
      }

      bool bDestroy = pExecutable->isDestroyedAfterExecute();
      if ((bDestroy == false) || (bBackground == true))
      {
         releasePlugIn();
      }
   }

   clearArgLists(true, false);
   return bSuccess;
}

bool ExecutableAgentImp::abort()
{
   checkInstantiate();
   bool bSuccess = false;

   Executable* pExecutable = dynamic_cast<Executable*>(getPlugIn());
   if (pExecutable != NULL)
   {
      bSuccess = pExecutable->abort();
   }

   return bSuccess;
}

bool ExecutableAgentImp::runAllTests(Progress *pProgress, ostream &output)
{
   checkInstantiate();
   bool success = false;

   Testable* pTestable = dynamic_cast<Testable*>(getPlugIn());
   if (pTestable != NULL)
   {
      success = pTestable->runAllTests(pProgress, output);
   }

   return success;
}

void ExecutableAgentImp::checkInstantiate() const
{
   if (!mInstantiated)
   {
      throw logic_error("Attempting to call methods on an un-instantiated instance of the agent.");
   }
}

bool ExecutableAgentImp::getInstantiated() const
{
   return mInstantiated;
}

PlugInArgList& ExecutableAgentImp::getInArgList() const
{
   checkInstantiate();
   const_cast<ExecutableAgentImp*>(this)->setupArgList();
   return *mpInArgList;
}

PlugInArgList &ExecutableAgentImp::getPopulatedInArgList()
{
   checkInstantiate();
   const_cast<ExecutableAgentImp*>(this)->setupArgList();
   const_cast<ExecutableAgentImp*>(this)->populateArgValues(mpInArgList);
   return *mpInArgList;
}

PlugInArgList& ExecutableAgentImp::getOutArgList() const
{
   checkInstantiate();
   const_cast<ExecutableAgentImp*>(this)->setupArgList();
   return *mpOutArgList;
}

const PlugIn* ExecutableAgentImp::getPlugIn() const
{
   checkInstantiate();
   return mPlugIn.get();
}

PlugIn* ExecutableAgentImp::getPlugIn()
{
   checkInstantiate();
   return mPlugIn.get();
}

PlugIn* ExecutableAgentImp::releasePlugIn()
{
   checkInstantiate();
   return mPlugIn.release();
}

void ExecutableAgentImp::clearArgLists(bool clearInputArgList, bool clearOutputArgList)
{
   checkInstantiate();
   Service<PlugInManagerServices> pPlugInMgr;
   if ((mpInArgList != NULL) && (clearInputArgList))
   {
      pPlugInMgr->destroyPlugInArgList(mpInArgList);
      mpInArgList = NULL;
   }
   if ((mpOutArgList != NULL) && (clearOutputArgList))
   {
      pPlugInMgr->destroyPlugInArgList(mpOutArgList);
      mpOutArgList = NULL;
   }
}

void ExecutableAgentImp::setupArgList()
{
   checkInstantiate();
   Executable* pExecutable = dynamic_cast<Executable*>(getPlugIn());
   if (pExecutable == NULL)
   {
      return;
   }

   if ((mpInArgList == NULL) || (mpOutArgList == NULL))
   {
      if (mBatch == true)
      {
         mSupportsRequestedBatchSetting = pExecutable->setBatch();
      }
      else
      {
         mSupportsRequestedBatchSetting = pExecutable->setInteractive();
      }
   }

   if (mpInArgList == NULL)
   {
      pExecutable->getInputSpecification(mpInArgList);
   }

   if (mpOutArgList == NULL)
   {
      pExecutable->getOutputSpecification(mpOutArgList);
   }

   Service<PlugInManagerServices> pPims;
   if (mpInArgList == NULL)
   {
      mpInArgList = pPims->getPlugInArgList();
   }
   if (mpOutArgList == NULL)
   {
      mpOutArgList = pPims->getPlugInArgList();
   }

   VERIFYNRV(mpInArgList != NULL);
   VERIFYNRV(mpOutArgList != NULL);
}

void ExecutableAgentImp::populateArgValues(PlugInArgList *pArgList)
{
   checkInstantiate();
   if (pArgList != NULL)
   {
      // Window
      PlugInArg *pWindowArg = NULL;
      pArgList->getArg(Executable::WindowArg(), pWindowArg);

      WorkspaceWindow *pWindow = NULL;
      string windowType = TypeConverter::toString<Window>();
      if (pWindowArg != NULL)
      {
         windowType = pWindowArg->getType();
         if (pWindowArg->isActualSet() == true)
         {
            pWindow = reinterpret_cast<WorkspaceWindow*>(pWindowArg->getActualValue());
         }
      }

      if (pWindow == NULL)
      {
         Service<DesktopServices> pDesktop;
         pWindow = pDesktop->getCurrentWorkspaceWindow();
         if (pWindow != NULL && pWindow->isKindOf(windowType) && pWindowArg != NULL)
         {
            if (pWindowArg->isActualSet() == false)
            {
               pWindowArg->setActualValue(pWindow);
            }
         }
      }

      // View
      PlugInArg *pViewArg = NULL;
      pArgList->getArg(Executable::ViewArg(), pViewArg);

      View *pView = NULL;
      string viewType = TypeConverter::toString<View>();
      if (pViewArg != NULL)
      {
         viewType = pViewArg->getType();
         if (pViewArg->isActualSet() == true)
         {
            pView = reinterpret_cast<View*>(pViewArg->getActualValue());
         }
      }

      if (pView == NULL)
      {
         if (pWindow != NULL)
         {
            pView = pWindow->getView();
            if (pView == NULL || !pView->isKindOf(viewType))
            {
               pView = pWindow->getActiveView();
            }
         }
         
         if (pView != NULL && pView->isKindOf(viewType) && pViewArg != NULL)
         {
            if (pViewArg->isActualSet() == false)
            {
               pViewArg->setActualValue(pView);
            }
         }
      }

      // Layer
      PlugInArg *pLayerArg = NULL;
      pArgList->getArg(Executable::LayerArg(), pLayerArg);

      Layer *pLayer = NULL;
      string layerType = TypeConverter::toString<Layer>();
      if (pLayerArg != NULL)
      {
         layerType = pLayerArg->getType();
         if (pLayerArg->isActualSet() == true)
         {
            pLayer = reinterpret_cast<Layer*>(pLayerArg->getActualValue());
         }
      }

      if (pLayer == NULL)
      {
         SpatialDataView* pSpatialDataView = dynamic_cast<SpatialDataView*>(pView);
         if (pSpatialDataView != NULL)
         {
            pLayer = pSpatialDataView->getActiveLayer();
         }

         ProductView* pProductView = dynamic_cast<ProductView*>(pView);
         if (pProductView != NULL)
         {
            pLayer = pProductView->getActiveLayer();
         }

         if (pLayer != NULL && pLayer->isKindOf(layerType) && pLayerArg != NULL)
         {
            if (pLayerArg->isActualSet() == false)
            {
               pLayerArg->setActualValue(pLayer);
            }
         }
      }

      // Data Element
      PlugInArg *pElementArg = NULL;
      pArgList->getArg(Executable::DataElementArg(), pElementArg);

      DataElement *pElement = NULL;
      string elementType = TypeConverter::toString<DataElement>();
      if (pElementArg != NULL)
      {
         elementType = pElementArg->getType();
         if (pElementArg->isActualSet() == true)
         {
            pElement = reinterpret_cast<DataElement*>(pElementArg->getActualValue());
         }
      }

      if (pElement == NULL)
      {
         RasterElement *pPrimaryRaster = NULL;
         // Prefer the primary RasterElement of the view
         SpatialDataView *pSpatialDataView = dynamic_cast<SpatialDataView*>(pView);
         if (pSpatialDataView != NULL)
         {
            LayerList* pLayerList = pSpatialDataView->getLayerList();
            if (pLayerList != NULL)
            {
               pPrimaryRaster = pLayerList->getPrimaryRasterElement();
               if (pPrimaryRaster != NULL)
               {
                  if (pPrimaryRaster->isKindOf(elementType) == true)
                  {
                     pElement = pPrimaryRaster;
                  }
               }
            }
         }

         // Try the element of the current layer
         if (pElement == NULL && pLayer != NULL)
         {
            DataElement *pLayerElement = pLayer->getDataElement();
            if (pLayerElement != NULL && pLayerElement->isKindOf(elementType))
            {
               pElement = pLayerElement;
            }
         }

         if (pElement == NULL && pPrimaryRaster != NULL)
         {
            // Check if there are elements with the same parent
            Service<ModelServices> pModel;
            vector<DataElement*> elements = pModel->getElements(pPrimaryRaster, elementType);
            if (elements.empty() == false)
            {
               pElement = elements.front();
            }
         }

         if (pElement != NULL && pElementArg != NULL)
         {
            if (pElementArg->isActualSet() == false)
            {
               pElementArg->setActualValue(pElement);
            }
         }
      }

      // Progress
      PlugInArg* pProgressArg = NULL;
      pArgList->getArg(Executable::ProgressArg(), pProgressArg);
      if (pProgressArg != NULL)
      {
         if (pProgressArg->isActualSet() == false)
         {
            pProgressArg->setPlugInArgValue(mpProgress);
         }
      }

      // Menu command
      PlugInArg* pMenuCommandArg = NULL;
      pArgList->getArg(Executable::MenuCommandArg(), pMenuCommandArg);
      if (pMenuCommandArg != NULL)
      {
         if (pMenuCommandArg->isActualSet() == false)
         {
            pMenuCommandArg->setPlugInArgValue(&mMenuCommand);
         }
      }
   }
}

bool ExecutableAgentImp::isBatch() const
{
   return mBatch;
}
