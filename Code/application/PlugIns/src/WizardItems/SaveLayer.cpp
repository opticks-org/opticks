/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "AppVerify.h"
#include "DataElement.h"
#include "FileDescriptor.h"
#include "Filename.h"
#include "Layer.h"
#include "LayerList.h"
#include "MessageLogResource.h"
#include "ModelServices.h"
#include "ObjectFactory.h"
#include "ObjectResource.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInResource.h"
#include "SaveLayer.h"
#include "RasterElement.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"

using namespace std;

///////////////
// SaveLayer //
///////////////

SaveLayer::SaveLayer() : mpOutputFilename(NULL), mpElement(NULL)
{
}

SaveLayer::~SaveLayer()
{
}

bool SaveLayer::getInputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;

   if (mbInteractive)
   {
      VERIFY(DesktopItems::getInputSpecification(pArgList) && pArgList != NULL);

      Service<PlugInManagerServices> pPlugInManager;
      VERIFY(pPlugInManager.get() != NULL);

      // Add args
      PlugInArg* pArg = pPlugInManager->getPlugInArg();
      VERIFY(pArg != NULL);
      pArg->setName("Filename");
      pArg->setType("Filename");
      pArg->setDefaultValue(NULL);
      pArgList->addArg(*pArg);

      LayerType eType = getLayerType();
      string modelType = getModelType(eType);
      VERIFY(!modelType.empty());

      pArg = pPlugInManager->getPlugInArg();
      VERIFY(pArg != NULL);
      pArg->setName("Layer Element");
      pArg->setType(modelType);
      pArg->setDefaultValue(NULL);
      pArgList->addArg(*pArg);
   }

   return true;
}

bool SaveLayer::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

bool SaveLayer::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Execute Wizard Item", "app", "DCBBB270-9360-4c96-8CE9-A9D414FC68EE");
   pStep->addProperty("Item", getName());
   mpStep = pStep.get();

   if (!extractInputArgs(pInArgList))
   {
      reportError("Unable to extract input arguments.", "CE17C3AD-05BD-4624-A9AD-9694430E1A6C");
      return false;
   }

   // Check for valid input values
   string filename = "";
   if (mpOutputFilename != NULL)
   {
      filename = mpOutputFilename->getFullPathAndName();
   }

   if (filename.empty())
   {
      reportError("The filename input value is invalid!", "2682BD10-8A8E-4aed-B2D8-7F7B4CC857A4");
      return false;
   }

   if (mpStep != NULL)
   {
      mpStep->addProperty("filename", filename);
   }

   if (mpElement == NULL)
   {
      reportError("The data element input value is invalid!", "CC2017C8-FB19-43c0-B1C6-C70625BFE611");
      return false;
   }

   DataElement* pParent = mpElement->getParent();
   if (mpStep != NULL)
   {
      if (pParent != NULL)
      {
         mpStep->addProperty("dataSet", pParent->getName());
      }
      else
      {
         mpStep->addProperty("dataSet", mpElement->getName());
      }
   }

   // Get the Layer
   Layer* pLayer = NULL;

   vector<Window*> windows;
   Service<DesktopServices> pDesktop;
   VERIFY(pDesktop.get() != NULL);
   pDesktop->getWindows(SPATIAL_DATA_WINDOW, windows);

   for (vector<Window*>::iterator iter = windows.begin(); iter != windows.end(); ++iter)
   {
      SpatialDataWindow* pWindow = static_cast<SpatialDataWindow*>(*iter);
      if (pWindow != NULL)
      {
         SpatialDataView* pCurrentView = pWindow->getSpatialDataView();
         if (pCurrentView != NULL)
         {
            LayerList* pLayerList = pCurrentView->getLayerList();
            if (pLayerList != NULL)
            {
               vector<Layer*> layers;
               pLayerList->getLayers(layers);
               vector<Layer*>::iterator layerIter;
               for (layerIter = layers.begin(); layerIter != layers.end(); ++layerIter)
               {
                  Layer* pCurrentLayer = *layerIter;
                  if (pCurrentLayer != NULL)
                  {
                     if (pCurrentLayer->getDataElement() == mpElement)
                     {
                        pLayer = pCurrentLayer;
                        break;
                     }
                  }
               }
            }
         }
      }
   }

   if (pLayer == NULL)
   {
      reportError("Could not get the layer to save!", "37EBD88F-9752-4b52-8A8A-F1BD9A98E608");
      return false;
   }

   // Get the layer type
   LayerType eType = getLayerType();

   // Save the layer
   FactoryResource<FileDescriptor> pFileDescriptor;
   VERIFY(pFileDescriptor.get() != NULL);
   pFileDescriptor->setFilename(filename);
   ExporterResource exporter("Layer Exporter", pLayer, pFileDescriptor.get());
   VERIFY(exporter->getPlugIn() != NULL);
   bool bSaved = exporter->execute();

   if (!bSaved)
   {
      reportError("Could not save the layer to the file: " + filename, "E2F6878E-E462-409b-AE8A-6E1555198316");
      return false;
   }

   reportComplete();
   return true;
}

bool SaveLayer::extractInputArgs(PlugInArgList* pInArgList)
{
   if (!DesktopItems::extractInputArgs(pInArgList))
   {
      reportError("Unable to extract input arguments.", "3F4C3D60-C1F3-4ff2-AF9C-F67F23D25037");
      return false;
   }

   PlugInArg* pArg = NULL;

   // Filename
   if (!pInArgList->getArg("Filename", pArg) || (pArg == NULL))
   {
      reportError("Could not read the filename input value!", "0C22B05B-ADF8-4d0c-A1AD-8B7193F8DA33");
      return false;
   }
   mpOutputFilename = pArg->getPlugInArgValue<Filename>();

   // Data element
   LayerType eType = getLayerType();

   if (!pInArgList->getArg("Layer Element", pArg) || (pArg == NULL))
   {
      reportError("Could not read the data element input value!", "2AA2FB55-32F7-478b-9920-3696EF55F957");
      return false;
   }
   mpElement = pArg->getPlugInArgValue<DataElement>();

   return true;
}

/////////////
// SaveAoi //
/////////////

SaveAoi::SaveAoi()
{
   setName("Save AOI");
   setVersion(APP_VERSION_NUMBER);
   setCreator("Ball Aerospace & Technologies, Corp.");
   setCopyright(APP_COPYRIGHT);
   setDescription("Saves an existing AOI layer to a file");
   setDescriptorId("{C65A9921-D885-4968-ACC6-8AB914BDFA8F}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

SaveAoi::~SaveAoi()
{
}

LayerType SaveAoi::getLayerType() const
{
   return AOI_LAYER;
}

/////////////////
// SaveGcpList //
/////////////////

SaveGcpList::SaveGcpList()
{
   setName("Save GCP List");
   setVersion(APP_VERSION_NUMBER);
   setCreator("Ball Aerospace & Technologies, Corp.");
   setCopyright(APP_COPYRIGHT);
   setDescription("Saves an existing GCP list layer to a file");
   setDescriptorId("{42A1C8C1-8E9C-4c71-9F57-412812895B01}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

SaveGcpList::~SaveGcpList()
{
}

LayerType SaveGcpList::getLayerType() const
{
   return GCP_LAYER;
}

//////////////////////////
// SaveLayerFromDataSet //
//////////////////////////

SaveLayerFromDataSet::SaveLayerFromDataSet() :
   mpOutputFilename(NULL),
   mpRasterElement(NULL)
{
}

SaveLayerFromDataSet::~SaveLayerFromDataSet()
{
}

bool SaveLayerFromDataSet::getInputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;

   if (mbInteractive)
   {
      VERIFY(DesktopItems::getInputSpecification(pArgList) && pArgList != NULL);
   
      Service<PlugInManagerServices> pPlugInManager;
      VERIFY(pPlugInManager.get() != NULL);

      // Add args
      PlugInArg* pArg = NULL;
      pArg = pPlugInManager->getPlugInArg();
      VERIFY(pArg != NULL);
      pArg->setName("Filename");
      pArg->setType("Filename");
      pArg->setDefaultValue(NULL);
      pArgList->addArg(*pArg);

      pArg = pPlugInManager->getPlugInArg();
      VERIFY(pArg != NULL);
      pArg->setName("Data set");
      pArg->setType("RasterElement");
      pArg->setDefaultValue(NULL);
      pArgList->addArg(*pArg);

      pArg = pPlugInManager->getPlugInArg();
      VERIFY(pArg != NULL);
      LayerType eType = getLayerType();

      pArg->setName("Layer Name");
      pArg->setType("string");
      pArg->setDefaultValue(NULL);
      pArgList->addArg(*pArg);
   }

   return true;
}

bool SaveLayerFromDataSet::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

bool SaveLayerFromDataSet::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Execute Wizard Item", "app", "A1205468-4950-4c8f-9821-60063CC4B31B");
   pStep->addProperty("Item", getName());
   mpStep = pStep.get();

   if (!extractInputArgs(pInArgList))
   {
      reportError("Unable to extract input arguments.", "9A496CD9-5068-4b12-A4C4-AB561CD49523");
      return false;
   }

   // Check for valid input values
   string filename;
   if (mpOutputFilename != NULL)
   {
      filename = mpOutputFilename->getFullPathAndName();
   }

   if (filename.empty())
   {
      reportError(" The filename input value is invalid!", "DA76EB21-7E5A-45aa-A60D-0B99C72585EC");
      return false;
   }

   if (mpStep != NULL)
   {
      mpStep->addProperty("filename", filename);
   }

   if (mpRasterElement == NULL)
   {
      reportError("The data set input value is invalid!", "E11D0EC5-97E6-41a5-8F1F-937290CA102F");
      return false;
   }

   if (mpStep != NULL)
   {
      mpStep->addProperty("dataSet", mpRasterElement->getName());
   }

   if (mLayerName.empty())
   {
      reportError("The layer name input value is invalid!", "0DF331B8-05FF-4178-82D3-9A9CF2851DCF");
      return false;
   }

   if (mpStep != NULL)
   {
      mpStep->addProperty("layerName", mLayerName);
   }

   // Get the view
   SpatialDataView* pView = NULL;

   vector<Window*> windows;
   Service<DesktopServices> pDesktop;
   if (pDesktop.get() != NULL)
   {
      pDesktop->getWindows(SPATIAL_DATA_WINDOW, windows);
   }

   for (vector<Window*>::iterator iter = windows.begin(); iter != windows.end(); ++iter)
   {
      SpatialDataWindow* pWindow = static_cast<SpatialDataWindow*>(*iter);
      if (pWindow != NULL)
      {
         SpatialDataView* pCurrentView = pWindow->getSpatialDataView();
         if (pCurrentView != NULL)
         {
            LayerList* pLayerList = pCurrentView->getLayerList();
            if (pLayerList != NULL)
            {
               RasterElement* pRasterElement = pLayerList->getPrimaryRasterElement();
               if (pRasterElement == mpRasterElement)
               {
                  pView = pCurrentView;
                  break;
               }
            }
         }
      }
   }

   if (pView == NULL)
   {
      reportError("Could not get the view!", "830E3C55-561A-4c49-8269-06E1E04B1BFA");
      return false;
   }

   // Get the spectral element
   LayerType eType = getLayerType();
   string modelType = getModelType(eType);

   DataElement* pElement = NULL;
   Service<ModelServices> pModel;
   if ((pModel.get() != NULL) && !modelType.empty())
   {
      pElement = pModel->getElement(mLayerName, modelType, mpRasterElement);
   }

   // Save the layer
   bool bSaved = false;

   LayerList* pLayerList = pView->getLayerList();
   if (pLayerList != NULL)
   {
      Layer* pLayer = pLayerList->getLayer(eType, pElement, mLayerName.c_str());
      if (pLayer == NULL)
      {
         reportError("Could not get the layer to save!", "02F03D56-7CA8-4052-894D-BFDDFC3A814F");
         return false;
      }

      FactoryResource<FileDescriptor> pFileDescriptor;
      VERIFY(pFileDescriptor.get() != NULL);
      pFileDescriptor->setFilename(filename);
      ExporterResource exporter("Layer Exporter", pLayer, pFileDescriptor.get());
      VERIFY(exporter->getPlugIn() != NULL);
      bSaved = exporter->execute();
   }

   if (!bSaved)
   {
      reportError("Could not save the layer to the file: " + filename, "CAFF2CD5-E6CB-4e90-80E7-87E094F2CB1C");
      return false;
   }

   reportComplete();
   return true;
}

bool SaveLayerFromDataSet::extractInputArgs(PlugInArgList* pInArgList)
{
   if (!DesktopItems::extractInputArgs(pInArgList))
   {
      reportError("Unable to extract input arguments.", "1350D7A3-EAD5-4b85-8DDE-8A3C0089155D");
      return false;
   }

   PlugInArg* pArg = NULL;

   // Filename
   if (!pInArgList->getArg("Filename", pArg) || (pArg == NULL))
   {
      reportError("Could not read the filename input value!", "3BF76C5E-4DB9-49d5-B90F-04A138C11FCE");
      return false;
   }

   mpOutputFilename = pArg->getPlugInArgValue<Filename>();

   // Data Set
   if (!pInArgList->getArg("Data set", pArg) || (pArg == NULL))
   {
      reportError("Could not read the data set input value!", "C9E16D41-0137-463e-936C-605E2DDE0417");
      return false;
   }
   mpRasterElement = pArg->getPlugInArgValue<RasterElement>();

   // Layer name
   LayerType eType = getLayerType();

   if (!pInArgList->getArg("Layer Name", pArg) || (pArg == NULL))
   {
      reportError("Could not read the layer name input value!", "44776565-8B6F-4ec3-9D4E-3D3359C9D23E");
      return false;
   }
   string* pName = pArg->getPlugInArgValue<string>();
   if (pName != NULL)
   {
      mLayerName = *pName;
   }

   return true;
}

////////////////////////
// SaveAoiFromDataSet //
////////////////////////

SaveAoiFromDataSet::SaveAoiFromDataSet()
{
   setName("Save AOI From Data Set");
   setVersion(APP_VERSION_NUMBER);
   setCreator("Ball Aerospace & Technologies, Corp.");
   setCopyright(APP_COPYRIGHT);
   setDescription("Saves an existing AOI layer to a file");
   setDescriptorId("{A5C9CCEC-333B-4fb0-85BF-48825698AE9A}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

SaveAoiFromDataSet::~SaveAoiFromDataSet()
{
}

LayerType SaveAoiFromDataSet::getLayerType() const
{
   return AOI_LAYER;
}

////////////////////
// SaveAnnotation //
////////////////////

SaveAnnotation::SaveAnnotation()
{
   setName("Save Annotation");
   setVersion(APP_VERSION_NUMBER);
   setCreator("Ball Aerospace & Technologies, Corp.");
   setCopyright(APP_COPYRIGHT);
   setDescription("Saves an existing annotation layer to a file");
   setDescriptorId("{88B0B7E3-9289-4cc4-BEFB-B782D4D379C1}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

SaveAnnotation::~SaveAnnotation()
{
}

LayerType SaveAnnotation::getLayerType() const
{
   return ANNOTATION;
}

////////////////////////////
// SaveGcpListFromDataSet //
////////////////////////////

SaveGcpListFromDataSet::SaveGcpListFromDataSet()
{
   setName("Save GCP List From Data Set");
   setVersion(APP_VERSION_NUMBER);
   setCreator("Ball Aerospace & Technologies, Corp.");
   setCopyright(APP_COPYRIGHT);
   setDescription("Saves an existing GCP list layer to a file");
   setDescriptorId("{257790EE-523A-46ae-903C-EC3897502170}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

SaveGcpListFromDataSet::~SaveGcpListFromDataSet()
{
}

LayerType SaveGcpListFromDataSet::getLayerType() const
{
   return GCP_LAYER;
}
