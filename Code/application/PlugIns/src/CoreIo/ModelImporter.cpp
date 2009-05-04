/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"
#include "AppVerify.h"
#include "AppVersion.h"
#include "Classification.h"
#include "DataDescriptor.h"
#include "DataElement.h"
#include "DesktopServices.h"
#include "FileDescriptor.h"
#include "ImportDescriptor.h"
#include "Layer.h"
#include "LayerList.h"
#include "MessageLogResource.h"
#include "ModelImporter.h"
#include "ModelServices.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "Progress.h"
#include "RasterElement.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"

#include <QtGui/QCheckBox>
#include <QtGui/QFrame>
#include <QtGui/QHBoxLayout>

using namespace std;
XERCES_CPP_NAMESPACE_USE

map<string,string> ModelImporter::sTypeMap;

REGISTER_PLUGIN_BASIC(OpticksCoreIo, ModelImporter);

ModelImporter::ModelImporter() :
   mpOptionsWidget(NULL),
   mpCheckBox(NULL)
{
   setName("Model Importer");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setExtensions("All Model Elements(*.aoi *.ano *.gcp *.xml);;"
                 "AOI Files (*.aoi);;"
                 "Annotation Files (*.ano);;"
                 "GCP Lists (*.gcp);;"
                 "Tiepoint List (*.tie);;"
                 "XML Files (*.xml)");
   setDescription("Import Model Elements");
   setSubtype("Model Element");
   setDescriptorId("{0F6505F2-9F39-4879-8A53-2633B3F3886B}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);

   // initialize the type map
   // this allows us to load old types into their newer counterparts
   // for example AoiElement can load AOI
   if (sTypeMap.empty())
   {
      sTypeMap["AOI"] = "AoiElement";
      sTypeMap["AOIAdapter"] = "AoiElement";
   }
}

ModelImporter::~ModelImporter()
{
   delete mpOptionsWidget;
}

bool ModelImporter::getInputSpecification(PlugInArgList *&pInArgList)
{
   pInArgList = mpPlugInManager->getPlugInArgList();
   VERIFY(pInArgList != NULL);

   PlugInArg* pArg = NULL;
   VERIFY((pArg = mpPlugInManager->getPlugInArg()) != NULL);
   pArg->setName(ProgressArg());
   pArg->setType("Progress");
   pArg->setDefaultValue(NULL);
   pInArgList->addArg(*pArg);

   VERIFY((pArg = mpPlugInManager->getPlugInArg()) != NULL);
   pArg->setName(ImportElementArg());
   pArg->setType("DataElement");
   pArg->setDefaultValue(NULL);
   pInArgList->addArg(*pArg);

   if (isBatch())
   {
      bool value = false;
      pInArgList->addArg<bool>("CreateLayer", &value);
   }

   return true;
}

bool ModelImporter::getOutputSpecification(PlugInArgList *&pOutArgList)
{
   pOutArgList = NULL;
   return true;
}

vector<ImportDescriptor*> ModelImporter::getImportDescriptors(const string& filename)
{
   return getImportDescriptors(filename, true);
}

vector<ImportDescriptor*> ModelImporter::getImportDescriptors(const string& filename, bool reportErrors)
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
      DOMElement* pRootElement = NULL;
      if (pDoc != NULL)
      {
         pRootElement = pDoc->getDocumentElement();
      }
      if (pRootElement != NULL)
      {
         ImportDescriptor* pImportDescriptor = populateImportDescriptor(pRootElement, filename);
         if (pImportDescriptor != NULL)
         {
            descriptors.push_back(pImportDescriptor);
         }
      }
   }
   
   return descriptors;
}

unsigned char ModelImporter::getFileAffinity(const string& filename)
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

ImportDescriptor* ModelImporter::populateImportDescriptor(DOMElement* pElement, const string& filename)
{
   if (pElement == NULL)
   {
      return NULL;
   }

   if (! (pElement->hasAttribute(X("name")) &&
      pElement->hasAttribute(X("type")) &&
      pElement->hasAttribute(X("version"))))
   {
      return NULL;
   }
   string name = A(pElement->getAttribute(X("name")));
   string type = getTypeSubstitution(A(pElement->getAttribute(X("type"))));
   string dataset;
   if (pElement->hasAttribute(X("dataset")))
   {
      dataset = XmlBase::URLtoPath(pElement->getAttribute(X("dataset")));
   }
   unsigned int version = atoi(A(pElement->getAttribute(X("version"))));

   DOMElement* pClassificationElement = static_cast<DOMElement*>(
         pElement->getElementsByTagName(X("classification"))->item(0));
   FactoryResource<Classification> pClassification;
   bool classificationValid = false;
   if (pClassificationElement != NULL)
   {
      //don't crash because classification is optional
      classificationValid = pClassification->fromXml(pClassificationElement, version);
   }

   FactoryResource<DynamicObject> pMetaData;
   DOMElement* pMetadataElement = static_cast<DOMElement*>(
      pElement->getElementsByTagName(X("metadata"))->item(0));
   bool metaDataValid = false;
   if (pMetadataElement != NULL)
   {
      //don't crash on version 2 files, where metadata is optional
      metaDataValid = pMetaData->fromXml(pMetadataElement, version);
   }

   Service<ModelServices> pModel;

   ImportDescriptor* pImportDescriptor = pModel->createImportDescriptor(name, type, NULL);
   if (pImportDescriptor != NULL)
   {
      DataDescriptor* pDescriptor = pImportDescriptor->getDataDescriptor();
      if (pDescriptor != NULL)
      {
         FactoryResource<FileDescriptor> pFileDescriptor;
         if (pFileDescriptor.get() != NULL)
         {
            pFileDescriptor->setFilename(filename);
            pFileDescriptor->setDatasetLocation(dataset);
            pDescriptor->setFileDescriptor(pFileDescriptor.get());
         }

         if (classificationValid)
         {
            pDescriptor->setClassification(pClassification.get());
         }

         if (metaDataValid)
         {
            pDescriptor->setMetadata(pMetaData.get());
         }
      }
   }

   return pImportDescriptor;
}

bool ModelImporter::execute(PlugInArgList *pInArgList, PlugInArgList *pOutArgList)
{
   Progress* pProgress = NULL;
   DataElement* pElement = NULL;
   bool createLayer = true;
   StepResource pStep("Import model element", "app", "52672A08-89C5-4238-815D-83B573206562");

   // get input arguments and log some useful info about them
   { // scope the MessageResource
      MessageResource pMsg("Input arguments", "app", "9A6416F1-7E25-40BA-B21A-55C382850EC0");

      pProgress = pInArgList->getPlugInArgValue<Progress>(ProgressArg());
      pMsg->addBooleanProperty("Progress Present", (pProgress != NULL));

      pElement = pInArgList->getPlugInArgValue<DataElement>(ImportElementArg());
      if (pElement == NULL)
      {
         if (pProgress != NULL)
         {
            pProgress->updateProgress("No data element", 0, ERRORS);
         }
         pStep->finalize(Message::Failure, "No data element");
         return false;
      }
      pMsg->addProperty("Element name", pElement->getName());

      if (isBatch())
      {
         pInArgList->getPlugInArgValue("CreateLayer", createLayer);
      }
      else if (mpCheckBox != NULL)
      {
         createLayer = mpCheckBox->checkState() == Qt::Checked;
      }
      pMsg->addBooleanProperty("Create layer", createLayer);
   }

   if (pProgress != NULL)
   {
      pProgress->updateProgress((string("Read and parse file ") + pElement->getFilename()), 20, NORMAL);
   }

   // parse the xml
   XmlReader xml(Service<MessageLogMgr>()->getLog());

   XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument *pDomDocument = xml.parse(pElement->getFilename());
   if (pDomDocument == NULL)
   {
      if (pProgress != NULL)
      {
         pProgress->updateProgress("Unable to parse the file", 0, ERRORS);
      }
      pStep->finalize(Message::Failure, "Unable to parse the file");
      return false;
   }
   else
   {
      DOMElement* pRootElement = pDomDocument->getDocumentElement();
      VERIFY(pRootElement != NULL);

      if (pProgress != NULL)
      {
         pProgress->updateProgress("Create the element", 40, NORMAL);
      }

      string name(A(pRootElement->getAttribute(X("name"))));
      string type(getTypeSubstitution(A(pRootElement->getAttribute(X("type")))));
      string datasetName(XmlBase::URLtoPath(pRootElement->getAttribute(X("dataset"))));
      unsigned int formatVersion = atoi(A(pRootElement->getAttribute(X("version"))));

      { // scope the MessageResource
         MessageResource pMsg("Element properties", "app", "599EFB02-FC47-4865-BE51-436179268295");
         pMsg->addProperty("name", name);
         pMsg->addProperty("type", type);
         pMsg->addProperty("dataset name", datasetName);
         pMsg->addProperty("format version", formatVersion);
      }

      if (createLayer)
      {
         if (pProgress != NULL)
         {
            pProgress->updateProgress("Build the layer", 60, NORMAL);
         }

         Layer* pLayer = NULL;
         SpatialDataView* pView = NULL;
         SpatialDataWindow* pWindow = dynamic_cast<SpatialDataWindow*>(mpDesktop->getCurrentWorkspaceWindow());
         if (pWindow != NULL)
         {
            pView = pWindow->getSpatialDataView();
         }
         if (pView != NULL)
         {
            // Reparent the element to the primary raster element
            LayerList* pLayerList = pView->getLayerList();
            if (pLayerList != NULL)
            {
               RasterElement* pParent = pLayerList->getPrimaryRasterElement();
               if (pParent != NULL)
               {
                  if (Service<ModelServices>()->setElementParent(pElement, pParent) == false)
                  {
                     return false;
                  }
               }
            }

            LayerType layerType;
            if (type.find("Annotation") == 0)
            {
               layerType = ANNOTATION;
            }
            else if (type.find("Aoi") == 0)
            {
               layerType = AOI_LAYER;
            }
            else if (type.find("GcpList") == 0)
            {
               layerType = GCP_LAYER;
            }
            else if (type.find("TiePoint") == 0)
            {
               layerType = TIEPOINT_LAYER;
            }

            if (layerType.isValid() == true)
            {
               pLayer = pView->createLayer(layerType, pElement);
            }
         }

         if (pLayer == NULL)
         {
            if (pProgress != NULL)
            {
               pProgress->updateProgress("Warning: Unable to create the layer", 60, WARNING);
            }

            return false;
         }
      }

      if (pProgress != NULL)
      {
         pProgress->updateProgress("Build the element", 70, NORMAL);
      }
      // deserialize the element
      try
      {
         if (pElement->fromXml(pRootElement, formatVersion) == false)
         {
            return false;
         }
      }
      catch (XmlReader::DomParseException&)
      {
         return false;
      }
   }

   pStep->finalize(Message::Success);
   if (pProgress != NULL)
   {
      pProgress->updateProgress("Finished loading the element", 100, NORMAL);
   }

   return true;
}

QWidget *ModelImporter::getImportOptionsWidget(DataDescriptor *pDescriptor)
{
   if (mpOptionsWidget == NULL)
   {
      mpOptionsWidget = new QWidget();
      VERIFYRV(mpOptionsWidget != NULL, NULL);

      QHBoxLayout* pTopLevel = new QHBoxLayout(mpOptionsWidget);
      mpCheckBox = new QCheckBox("Create a layer from this element?", mpOptionsWidget);
      mpCheckBox->setChecked(true);
      pTopLevel->addWidget(mpCheckBox);
   }

   return mpOptionsWidget;
}

string ModelImporter::getTypeSubstitution(string type)
{
   map<string, string>::const_iterator newType = sTypeMap.find(type);
   if (newType != sTypeMap.end())
   {
      return newType->second;
   }
   return type;
}
