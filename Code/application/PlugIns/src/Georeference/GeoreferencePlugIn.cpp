/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <math.h>

#include <QtGui/QMessageBox>

#include "ApplicationServices.h"
#include "AppVersion.h"
#include "AppVerify.h"
#include "DataDescriptor.h"
#include "DesktopServices.h"
#include "Filename.h"
#include "GeoreferenceDlg.h"
#include "GeoreferencePlugIn.h"
#include "LatLonLayer.h"
#include "LayerList.h"
#include "MessageLogResource.h"
#include "ModelServices.h"
#include "ObjectFactory.h"
#include "ObjectResource.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInDescriptor.h"
#include "PlugInManagerServices.h"
#include "PlugInResource.h"
#include "RasterElement.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "UtilityServices.h"

#include <algorithm>
#include <boost/bind.hpp>

GeoreferencePlugIn::GeoreferencePlugIn()
{
   setName("Georeference");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setMenuLocation("[Geo]\\Georeference");
   setDescriptorId("{EB9F4BE5-4E9E-4f54-BC16-A2B33B535CEA}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);

   mResultsName = "GEO_RESULTS";
   mDisplayLayer = false;
   
   mpProgress = NULL;
   mpRaster = NULL;
   mpView = NULL;
}

GeoreferencePlugIn::~GeoreferencePlugIn()
{
}

bool GeoreferencePlugIn::execute(PlugInArgList* pInParam, PlugInArgList* pOutParam )
{
   StepResource pStep("Run GeoReference", "app", "DE61E049-5B44-41A8-BE10-6DA2FFE23E0C");
   pStep->addProperty("name", getName());

   mpProgress = pInParam->getPlugInArgValue<Progress>(ProgressArg());
   mpRaster = pInParam->getPlugInArgValue<RasterElement>(DataElementArg());
   mpView = pInParam->getPlugInArgValue<SpatialDataView>(ViewArg());

   if(mpRaster == NULL)
   {
      mMessageText = "No data cube passed to the plug-in.";
      if(mpProgress)
      {
         mpProgress->updateProgress(mMessageText, 0, ERRORS);
      }
      pStep->finalize(Message::Failure, mMessageText);
      return false;
   }

   string sourceDatasetName = mpRaster->getFilename();
   pStep->addProperty("sourceDataset", sourceDatasetName);

   GeocoordType eType = GEOCOORD_LATLON;

   PlugIn* pGeoPlugin = NULL;
   if(!isBatch())
   {
      preparePluginVectors();

      // Create dialog and setup pointers to desktop services
      // pPlugin->execute() must be called before dlgGeo is destroyed
      // this->cleanUpPluginVectors() must be called after dlgGeo is destroyed
      FactoryResource<Filename> fn;
      if(fn.get() == NULL)
      {
         // the dialog could not be properly created
         pStep->finalize(Message::Failure, "Unable to allocate memory for a Filename object!");
         return false;
      }
      fn->setFullPathAndName(sourceDatasetName);
      QString geoDlgTitle = QString("Georeference ") + QString::fromStdString(fn->getFileName());
      GeoreferenceDlg *pDlgGeo = new (nothrow) GeoreferenceDlg(geoDlgTitle, mlstPluginNames, mlstPluginWidgets,
         mpDesktop->getMainWidget());
      if(pDlgGeo == NULL)
      {
         pStep->finalize(Message::Failure, "GeoreferencePlugIn could not allocate memory for the gui");

         cleanUpPluginVectors();
         return false;
      }

      pDlgGeo->setResultsName(mResultsName);

      bool bInputValid = false;
      int iResult;
      while(!bInputValid)
      {
         // Display dialog
         iResult = pDlgGeo->exec();
         // validate input
         if(iResult == QDialog::Accepted)
         {
            int iGeoIndex = pDlgGeo->getGeorefAlgorithmIndex();
            if(iGeoIndex > -1)
            {
               pGeoPlugin = mlstPlugins[iGeoIndex];

               Georeference* pGeoreference = dynamic_cast<Georeference*>(pGeoPlugin);
               if (pGeoPlugin != NULL)
               {
                  bInputValid = pGeoreference->validateGuiInput();
                  if(!bInputValid)
                  {
                     QMessageBox::critical(pDlgGeo, "Georeference", "The current input is not valid "
                        "for the selected Georeference plug-in!");
                  }
               }
               else
               {
                  QMessageBox::critical(pDlgGeo, "Georeference", "The selected Georeference plug-in "
                     "could not be found!");
               }
            }
            else
            {
               QMessageBox::critical(pDlgGeo, "Georeference", "Please select a valid Georeference plug-in!");
            }
         }
         else // canceled
         {
            bInputValid = true;
         }
      }
      if(iResult == QDialog::Rejected)
      {
         pStep->finalize(Message::Failure, "Georeferencing canceled");
         delete pDlgGeo;
         pDlgGeo = NULL;

         cleanUpPluginVectors();
         return false;
      }

      // Get results
      mResultsName = pDlgGeo->getResultsName();
      eType = pDlgGeo->getGeocoordType();
      unsigned int uiGeoIndex = pDlgGeo->getGeorefAlgorithmIndex();

      if(uiGeoIndex < mlstPlugins.size())
      {
         pGeoPlugin = mlstPlugins[uiGeoIndex];
         mlstPlugins[uiGeoIndex] = NULL;
      }
      else
      {
         mMessageText = "GeoreferencePlugIn: Plugin selected could not be found";
         if(mpProgress)
         {
            mpProgress->updateProgress(mMessageText, 0, ERRORS);
         }
         pStep->finalize(Message::Failure, mMessageText);
         delete pDlgGeo;
         pDlgGeo = NULL;

         cleanUpPluginVectors();
         return false;
      }

      if(pGeoPlugin == NULL)
      {
         mMessageText = "Plugin selected could not be found";
         if(mpProgress)
         {
            mpProgress->updateProgress(mMessageText, 0, ERRORS);
         }
         pStep->finalize(Message::Failure, mMessageText);
         delete pDlgGeo;
         pDlgGeo = NULL;

         cleanUpPluginVectors();
         return false;
      }

      // Execute the selected plugin
      Executable* pGeoExecutable = dynamic_cast<Executable*>(pGeoPlugin);
      if(pGeoExecutable != NULL)
      {
         PlugInArgList *pInArgList(NULL);
         PlugInArgList *pOutArgList(NULL);
         pGeoExecutable->setBatch();
         pGeoExecutable->getInputSpecification(pInArgList);
         pGeoExecutable->getOutputSpecification(pOutArgList);

         pInArgList->setPlugInArgValue(DataElementArg(), mpRaster);
         pInArgList->setPlugInArgValue(ProgressArg(), mpProgress);
         pInArgList->setPlugInArgValueLoose(ViewArg(), mpView);
         
         bool bSuccess = pGeoExecutable->execute(pInArgList, pOutArgList);

         mpPluginManager->destroyPlugInArgList(pInArgList);
         mpPluginManager->destroyPlugInArgList(pOutArgList);
         delete pDlgGeo;
         pDlgGeo = NULL;

         cleanUpPluginVectors();
         if(!bSuccess)
         {
            mpPluginManager->destroyPlugIn(pGeoPlugin);
            mMessageText = "Selected plug-in has failed to execute.";
            if(mpProgress)
            {
               mpProgress->updateProgress(mMessageText, 0, ERRORS);
            }
            pStep->finalize(Message::Failure, mMessageText);
            return false;
         }
      }
      else
      {
         mMessageText = "Georeference plugin selection has failed";
         if(mpProgress)
         {
            mpProgress->updateProgress(mMessageText, 0, ERRORS);
         }
         pStep->finalize(Message::Failure, mMessageText);
         delete pDlgGeo;
         pDlgGeo = NULL;

         cleanUpPluginVectors();
         return false;
      }

      delete pDlgGeo;
      pDlgGeo = NULL;

      cleanUpPluginVectors();
   }
   else
   {
      if(!pInParam->getPlugInArgValue("Display Layer", mDisplayLayer) ||
         !pInParam->getPlugInArgValue("Results Name", mResultsName))
      {
         mMessageText = "No name or Display Layer flag provided for georeferencing layer";
         if(mpProgress)
         {
            mpProgress->updateProgress(mMessageText, 0, ERRORS);
         }
         pStep->finalize(Message::Failure, mMessageText);
         return false;
      }
   }

   pStep->addProperty("resultsName", mResultsName);
   pStep->addProperty("geocoordType", eType);

   if((!isBatch() || mDisplayLayer) && mpView != NULL)
   {
      LayerList* pLayerList = mpView->getLayerList();
      if(pLayerList != NULL)
      {
         LatLonLayer* pLatLonLayer = static_cast<LatLonLayer*>(pLayerList->getLayer(LAT_LONG, mpRaster, mResultsName));
         if(pLatLonLayer == NULL)
         {
            pLatLonLayer = static_cast<LatLonLayer*>(mpView->createLayer(LAT_LONG, mpRaster, mResultsName));
         }

         if(pLatLonLayer != NULL)
         {
            pLatLonLayer->setGeocoordType(eType);
         }
      }
   }

   mMessageText = "Georeference completed.";
   if(mpProgress)
   {
      mpProgress->updateProgress(mMessageText, 100, NORMAL);
   }
   pStep->finalize(Message::Success);

   return true;
}

bool GeoreferencePlugIn::getInputSpecification(PlugInArgList*& pArgList)
{
   // Set up list
   pArgList = mpPluginManager->getPlugInArgList();
   VERIFY(pArgList != NULL);

   VERIFY(pArgList->addArg<Progress>(ProgressArg(), NULL));
   VERIFY(pArgList->addArg<RasterElement>(DataElementArg(), NULL));
   VERIFY(pArgList->addArg<SpatialDataView>(ViewArg(), NULL));

   if(isBatch())
   {
      VERIFY(pArgList->addArg<string>("Results Name", &mResultsName));
      VERIFY(pArgList->addArg<bool>("Display Layer", &mDisplayLayer));
   }

   return true;
}

bool GeoreferencePlugIn::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

void GeoreferencePlugIn::preparePluginVectors()
{
   vector<PlugInDescriptor*> allGeoPluginNames = mpPluginManager->getPlugInDescriptors(PlugInManagerServices::GeoreferenceType());
   
   for(vector<PlugInDescriptor*>::const_iterator plugin = allGeoPluginNames.begin();
                                      plugin != allGeoPluginNames.end();
                                      ++plugin)
   {
      PlugInDescriptor* pDescriptor = *plugin;
      if (pDescriptor == NULL)
      {
         continue;
      }
      PlugInResource pCurrentPlugin(pDescriptor->getName());
      if (pCurrentPlugin.get() == NULL)
      {
         continue;
      }
      Georeference* pCurrentPluginGeo = dynamic_cast<Georeference*>(pCurrentPlugin.get());
      if (pCurrentPluginGeo == NULL)
      {
         continue;
      }
      if(pCurrentPluginGeo->canHandleRasterElement(mpRaster))
      {
         mlstPluginNames.push_back(pDescriptor->getName());
         mlstPlugins.push_back(pCurrentPlugin.release());
         mlstPluginWidgets.push_back(pCurrentPluginGeo->getGui(mpRaster));
      }
   }
}

void GeoreferencePlugIn::cleanUpPluginVectors()
{
   for_each(mlstPlugins.begin(), mlstPlugins.end(),
      boost::bind(&PlugInManagerServices::destroyPlugIn, mpPluginManager.get(), _1));

   mlstPlugins.clear();
   mlstPluginWidgets.clear();
   mlstPluginNames.clear();
}
