/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "AppConfig.h"
#include "AppVerify.h"
#include "DataElement.h"
#include "DesktopServices.h"
#include "MessageLogResource.h"
#include "Layer.h"
#include "LayerImporter.h"
#include "LayerList.h"
#include "ModelImporter.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "Progress.h"
#include "RasterElement.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "StringUtilities.h"
#include "Undo.h"
#include "xmlreader.h"

#include <sstream>
using namespace std;
XERCES_CPP_NAMESPACE_USE

LayerImporter::LayerImporter() : mInteractive(false)
{
   setName("Layer Importer");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setExtensions("All Layers (*.aoilayer *.anolayer *.gcplayer *.tielayer);;"
                 "AOI Layers (*.aoilayer);;"
                 "Annotation Layers (*.anolayer);;"
                 "GCP Layers (*.gcplayer);;"
                 "Tiepoint Layers (*.tielayer);;"
                 "XML Files (*.xml)");
   setDescription("Import Layers");
   setSubtype("Layer");
   setDescriptorId("{7ECEEAF0-CAAA-4b30-B5C1-70A7BAB54333}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

LayerImporter::~LayerImporter()
{
}

bool LayerImporter::getInputSpecification(PlugInArgList *&pInArgList)
{
   pInArgList = mpPlugInManager->getPlugInArgList();
   VERIFY(pInArgList != NULL);

   VERIFY(pInArgList->addArg<Progress>(ProgressArg(), NULL));
   VERIFY(pInArgList->addArg<DataElement>(ImportElementArg(), NULL));
   VERIFY(pInArgList->addArg<SpatialDataView>(ViewArg(), NULL));

   return true;
}

bool LayerImporter::getOutputSpecification(PlugInArgList *&pOutArgList)
{
   pOutArgList = mpPlugInManager->getPlugInArgList();
   VERIFY(pOutArgList != NULL);
   VERIFY(pOutArgList->addArg<Layer>("Layer", NULL));

   return true;
}

vector<ImportDescriptor*> LayerImporter::getImportDescriptors(const string& filename)
{
   return getImportDescriptors(filename, true);
}

vector<ImportDescriptor*> LayerImporter::getImportDescriptors(const string& filename, bool reportErrors)
{
   vector<ImportDescriptor*> descriptors;
   if (!filename.empty())
   {
      MessageLog* pLog = NULL;
      if (reportErrors == true)
      {
         Service<MessageLogMgr> pLogMgr;
         pLog = pLogMgr->getLog();
      }

      XmlReader xml(pLog);
      XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument *pDoc = xml.parse(filename, "metadata");
      DOMElement *pRootElement = NULL;
      if (pDoc != NULL)
      {
         pRootElement = pDoc->getDocumentElement();
      }
      if (pRootElement != NULL)
      {
         for(DOMNode* pChild = pRootElement->getFirstChild();
            pChild != NULL;
            pChild = pChild->getNextSibling())
         {
            if (pChild->getNodeType() == DOMNode::ELEMENT_NODE)
            {
               DOMElement *pChildElement = static_cast<DOMElement*>(pChild);
               string cNodeName = A(pChildElement->getNodeName());

               ImportDescriptor* pImportDescriptor = ModelImporter::populateImportDescriptor(pChildElement, filename);
               if (pImportDescriptor != NULL)
               {
                  descriptors.push_back(pImportDescriptor);
               }
            }
         }
      }
   }

   return descriptors;
}

unsigned char LayerImporter::getFileAffinity(const std::string& filename)
{
   if (getImportDescriptors(filename, false).empty())
   {
      return Importer::CAN_NOT_LOAD;
   }
   else
   {
      return Importer::CAN_LOAD;
   }
}

bool LayerImporter::execute(PlugInArgList *pInArgList, PlugInArgList *pOutArgList)
{
   Layer *pLayer = NULL;
   Progress *pProgress = NULL;
   DataElement *pElement = NULL;
   SpatialDataView* pView = NULL;
   StepResource pStep("Import layer", "app", "DF24688A-6B34-4244-98FF-5FFE2063AC05");

   // get input arguments and log some useful info about them
   { // scope the MessageResource
      MessageResource pMsg("Input arguments", "app", "C0A532DE-0E19-44D3-837C-16ABD267B2C1");

      pProgress = pInArgList->getPlugInArgValue<Progress>(ProgressArg());
      pMsg->addBooleanProperty("Progress Present", (pProgress != NULL));

      pElement = pInArgList->getPlugInArgValue<DataElement>(ImportElementArg());
      if (pElement == NULL)
      {
         if (pProgress != NULL)
         {
            pProgress->updateProgress("No data element", 100, ERRORS);
         }
         pStep->finalize(Message::Failure, "No data element");
         return false;
      }
      pMsg->addProperty("Element name", pElement->getName());
      pView = pInArgList->getPlugInArgValue<SpatialDataView>(ViewArg());
      if (pView != NULL)
      {
         pMsg->addProperty("View name", pView->getName());
      }
   }

   if (pProgress != NULL)
   {
      pProgress->updateProgress((string("Read and parse file ") + pElement->getFilename()),
         20, NORMAL);
   }

   // parse the xml
   XmlReader xml(Service<MessageLogMgr>()->getLog());

   XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument *pDomDocument = xml.parse(pElement->getFilename());
   if (pDomDocument == NULL)
   {
      if (pProgress != NULL)
      {
         pProgress->updateProgress("Unable to parse the file", 100, ERRORS);
      }
      pStep->finalize(Message::Failure, "Unable to parse the file");
      return false;
   }
   else
   {
      DOMElement *pRootElement = pDomDocument->getDocumentElement();
      VERIFY(pRootElement != NULL);

      if (pProgress != NULL)
      {
         pProgress->updateProgress("Create the layer", 40, NORMAL);
      }

      string name(A(pRootElement->getAttribute(X("name"))));
      string type(A(pRootElement->getAttribute(X("type"))));
      unsigned int formatVersion = atoi(A(pRootElement->getAttribute(X("version"))));

      { // scope the MessageResource
         MessageResource pMsg("Layer information", "app", "AA358F7A-107E-456E-8D11-36DDBE5B1645");
         pMsg->addProperty("name", name);
         pMsg->addProperty("type", type);
         pMsg->addProperty("format version", formatVersion);
      }

      if (pView == NULL)
      {
         //no view provided, so find current view
         SpatialDataWindow *pWindow = dynamic_cast<SpatialDataWindow*>(mpDesktop->getCurrentWorkspaceWindow());
         if (pWindow != NULL)
         {
            pView = pWindow->getSpatialDataView();
         }
      }

      if (pView == NULL)
      {
         if (pProgress != NULL)
         {
            pProgress->updateProgress("Could not access the view to create the layer.", 100, ERRORS);
         }

         pStep->finalize(Message::Failure, "Could not access the view to create the layer.");
         return false;
      }
      else
      {
         bool error = false;
         LayerType layerType = StringUtilities::fromXmlString<LayerType>(type, &error);
         if (error == true)
         {
            if (pProgress != NULL)
            {
               pProgress->updateProgress("The layer type is invalid.", 100, ERRORS);
            }

            pStep->finalize(Message::Failure, "The layer type is invalid.");
            return false;
         }
         else
         {
            LayerList* pLayerList = pView->getLayerList();
            if (pLayerList != NULL)
            {
               RasterElement* pNewParentElement = pLayerList->getPrimaryRasterElement();
               if (pNewParentElement != NULL)
               {
                  Service<ModelServices> pModel;
                  if (pModel->setElementParent(pElement, pNewParentElement) == false)
                  {
                     return false;
                  }
               }
            }

            UndoGroup group(pView, "Import " + StringUtilities::toDisplayString(layerType) + " Layer");

            pLayer = pView->createLayer(layerType, pElement);
            if (pLayer == NULL)
            {
               if (pProgress != NULL)
               {
                  pProgress->updateProgress("Unable to create the layer", 100, ERRORS);
               }
               pStep->finalize(Message::Failure, "Unable to create the layer");
               return false;
            }
            else
            {
               if (pProgress != NULL)
               {
                  pProgress->updateProgress("Build the layer", 60, NORMAL);
               }
               // deserialize the layer
               try
               {
                  if (pLayer->fromXml(pRootElement, formatVersion) == false)
                  {
                     return false;
                  }
               }
               catch(XmlReader::DomParseException &)
               {
                  return false;
               }
            }
         }
      }
   }

   pStep->finalize(Message::Success);
   if (pProgress != NULL)
   {
      pProgress->updateProgress("Finished loading the layer", 100, NORMAL);
   }

   // Add the layer to the layer list
   pView->addLayer(pLayer);
   if (pOutArgList != NULL)
   {
      // set the output arguments
      pOutArgList->setPlugInArgValue("Layer", pLayer);
   }

   return true;
}
