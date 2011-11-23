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
#include "Georeference.h"
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
#include "PlugInRegistration.h"
#include "PlugInResource.h"
#include "RasterElement.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "Undo.h"
#include "UtilityServices.h"

#include <algorithm>
#include <boost/bind.hpp>
#include <memory>

REGISTER_PLUGIN_BASIC(OpticksGeoreference, GeoreferencePlugIn);

GeoreferencePlugIn::GeoreferencePlugIn() :
   mCreateLayer(Georeference::getSettingCreateLatLonLayer()),
   mDisplayLayer(Georeference::getSettingDisplayLatLonLayer()),
   mpProgress(NULL),
   mpRaster(NULL),
   mpView(NULL),
   mResultsName("GEO_RESULTS")
{
   setName("Georeference");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setMenuLocation("[Geo]\\Georeference");
   setDescriptorId("{EB9F4BE5-4E9E-4f54-BC16-A2B33B535CEA}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

GeoreferencePlugIn::~GeoreferencePlugIn()
{
}

bool GeoreferencePlugIn::execute(PlugInArgList* pInParam, PlugInArgList* pOutParam )
{
   StepResource pStep("Run GeoReference", "app", "DE61E049-5B44-41A8-BE10-6DA2FFE23E0C");
   pStep->addProperty("name", getName());

   mpProgress = pInParam->getPlugInArgValue<Progress>(Executable::ProgressArg());
   mpRaster = pInParam->getPlugInArgValue<RasterElement>(Executable::DataElementArg());
   mpView = pInParam->getPlugInArgValue<SpatialDataView>(Executable::ViewArg());

   if (mpRaster == NULL)
   {
      mMessageText = "No data cube passed to the plug-in.";
      if (mpProgress)
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
   if (!isBatch())
   {
      preparePluginVectors();

      // Create dialog and setup pointers to desktop services
      // pPlugin->execute() must be called before dlgGeo is destroyed
      // this->cleanUpPluginVectors() must be called after dlgGeo is destroyed
      FactoryResource<Filename> fn;
      if (fn.get() == NULL)
      {
         // the dialog could not be properly created
         pStep->finalize(Message::Failure, "Unable to allocate memory for a Filename object!");
         return false;
      }
      fn->setFullPathAndName(sourceDatasetName);
      QString geoDlgTitle = QString("Georeference ") + QString::fromStdString(fn->getFileName());

      GeoreferenceDlg dlgGeo(geoDlgTitle, mlstPluginNames, mlstPluginWidgets, mpDesktop->getMainWidget());
      dlgGeo.setResultsName(mResultsName);

      bool bInputValid = false;
      int iResult = QDialog::Rejected;
      while (!bInputValid)
      {
         // Display dialog
         iResult = dlgGeo.exec();
         // validate input
         if (iResult == QDialog::Accepted)
         {
            int iGeoIndex = dlgGeo.getGeorefAlgorithmIndex();
            if (iGeoIndex > -1)
            {
               pGeoPlugin = mlstPlugins[iGeoIndex];

               Georeference* pGeoreference = dynamic_cast<Georeference*>(pGeoPlugin);
               if (pGeoPlugin != NULL)
               {
                  bInputValid = pGeoreference->validateGuiInput();
                  if (!bInputValid)
                  {
                     QMessageBox::critical(&dlgGeo, "Georeference", "The current input is not valid "
                        "for the selected Georeference plug-in!");
                  }
               }
               else
               {
                  QMessageBox::critical(&dlgGeo, "Georeference", "The selected Georeference plug-in "
                     "could not be found!");
               }
            }
            else
            {
               QMessageBox::critical(&dlgGeo, "Georeference", "Please select a valid Georeference plug-in!");
            }
         }
         else // canceled
         {
            bInputValid = true;
         }
      }
      if (iResult == QDialog::Rejected)
      {
         pStep->finalize(Message::Failure, "Georeferencing canceled");
         cleanUpPluginVectors();
         return false;
      }

      // Get results
      mCreateLayer = dlgGeo.getCreateLayer();
      mDisplayLayer = dlgGeo.getDisplayLayer();
      mResultsName = dlgGeo.getResultsName();
      eType = dlgGeo.getGeocoordType();
      unsigned int uiGeoIndex = dlgGeo.getGeorefAlgorithmIndex();

      if (uiGeoIndex < mlstPlugins.size())
      {
         pGeoPlugin = mlstPlugins[uiGeoIndex];
         mlstPlugins[uiGeoIndex] = NULL;
      }
      else
      {
         mMessageText = "GeoreferencePlugIn: Plugin selected could not be found";
         if (mpProgress)
         {
            mpProgress->updateProgress(mMessageText, 0, ERRORS);
         }

         pStep->finalize(Message::Failure, mMessageText);
         cleanUpPluginVectors();
         return false;
      }

      if (pGeoPlugin == NULL)
      {
         mMessageText = "Plugin selected could not be found";
         if (mpProgress)
         {
            mpProgress->updateProgress(mMessageText, 0, ERRORS);
         }

         pStep->finalize(Message::Failure, mMessageText);
         cleanUpPluginVectors();
         return false;
      }

      // Execute the selected plugin
      Executable* pGeoExecutable = dynamic_cast<Executable*>(pGeoPlugin);
      if (pGeoExecutable != NULL)
      {
         PlugInArgList* pInArgList(NULL);
         PlugInArgList* pOutArgList(NULL);
         pGeoExecutable->setBatch();
         pGeoExecutable->getInputSpecification(pInArgList);
         pGeoExecutable->getOutputSpecification(pOutArgList);

         pInArgList->setPlugInArgValue(Executable::DataElementArg(), mpRaster);
         pInArgList->setPlugInArgValue(Executable::ProgressArg(), mpProgress);
         pInArgList->setPlugInArgValueLoose(Executable::ViewArg(), mpView);
         
         bool bSuccess = pGeoExecutable->execute(pInArgList, pOutArgList);

         mpPluginManager->destroyPlugInArgList(pInArgList);
         mpPluginManager->destroyPlugInArgList(pOutArgList);

         cleanUpPluginVectors();
         if (!bSuccess)
         {
            mpPluginManager->destroyPlugIn(pGeoPlugin);
            mMessageText = "Selected plug-in has failed to execute.";
            if (mpProgress)
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
         if (mpProgress)
         {
            mpProgress->updateProgress(mMessageText, 0, ERRORS);
         }

         pStep->finalize(Message::Failure, mMessageText);
         cleanUpPluginVectors();
         return false;
      }

      cleanUpPluginVectors();
   }
   else
   {
      pInParam->getPlugInArgValue("Create Layer", mCreateLayer);
      pInParam->getPlugInArgValue("Display Layer", mDisplayLayer);
      pInParam->getPlugInArgValue("Results Name", mResultsName);
   }

   if (mCreateLayer && mpView != NULL)
   {
      LayerList* pLayerList = mpView->getLayerList();
      if (pLayerList != NULL)
      {
         auto_ptr<UndoGroup> pUndoGroup;
         LatLonLayer* pLatLonLayer = static_cast<LatLonLayer*>(pLayerList->getLayer(LAT_LONG, mpRaster, mResultsName));
         if (pLatLonLayer == NULL)
         {
            pUndoGroup.reset(new UndoGroup(mpView, "Add Lat/Lon Layer"));
            pLatLonLayer = static_cast<LatLonLayer*>(mpView->createLayer(LAT_LONG, mpRaster, mResultsName));
            if (pLatLonLayer == NULL)
            {
               string warningTxt = "Could not create the latitude/longitude layer.";
               if (mResultsName.empty())
               {
                  warningTxt += " The input results name was blank.";
               }
               if (mpProgress != NULL)
               {
                  mpProgress->updateProgress(warningTxt, 99, WARNING);
               }
               pStep->addMessage(warningTxt, "app", "84B6CB10-9902-4ECA-BD9B-0BF6DEDF288D");
            }
         }

         if (pLatLonLayer != NULL)
         {
            pStep->addProperty("resultsName", mResultsName);
            pStep->addProperty("geocoordType", eType);
            pLatLonLayer->setGeocoordType(eType);

            if (pOutParam != NULL)
            {
               pOutParam->setPlugInArgValue<LatLonLayer>("Latitude/Longitude Layer", pLatLonLayer);
            }

            if (mDisplayLayer)
            {
               mpView->showLayer(pLatLonLayer);
            }
            else
            {
               mpView->hideLayer(pLatLonLayer);
            }
         }
      }
   }

   mMessageText = "Georeference completed.";
   if (mpProgress)
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

   VERIFY(pArgList->addArg<Progress>(Executable::ProgressArg(), NULL, Executable::ProgressArgDescription()));
   VERIFY(pArgList->addArg<RasterElement>(Executable::DataElementArg(), NULL, "Data element to be georeferenced."));
   VERIFY(pArgList->addArg<SpatialDataView>(Executable::ViewArg(), NULL, "View in which the georeferencing results are displayed."));

   if (isBatch())
   {
      VERIFY(pArgList->addArg<string>("Results Name", &mResultsName, "Name for the result of the georeferencing."));
      VERIFY(pArgList->addArg<bool>("Create Layer", &mCreateLayer, "Whether to create a lat/lon layer or not."));
      VERIFY(pArgList->addArg<bool>("Display Layer", &mDisplayLayer, "If a lat/lon layer is created, whether to "
         "display the new layer or not."));
   }

   return true;
}

bool GeoreferencePlugIn::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = mpPluginManager->getPlugInArgList();
   VERIFY(pArgList != NULL);
   VERIFY(pArgList->addArg<LatLonLayer>("Latitude/Longitude Layer", NULL, "Latitude/longitude layer resulting from the georeferencing."));
   return true;
}

void GeoreferencePlugIn::preparePluginVectors()
{
   vector<PlugInDescriptor*> allGeoPluginNames =
      mpPluginManager->getPlugInDescriptors(PlugInManagerServices::GeoreferenceType());

   for (vector<PlugInDescriptor*>::const_iterator plugin = allGeoPluginNames.begin();
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
      if (pCurrentPluginGeo->canHandleRasterElement(mpRaster))
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
