/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AnnotationLayer.h"
#include "AoiLayer.h"
#include "AppVersion.h"
#include "AppConfig.h"
#include "AppVerify.h"
#include "DataDescriptor.h"
#include "DataElement.h"
#include "DesktopServices.h"
#include "FileDescriptor.h"
#include "FileResource.h"
#include "GcpLayer.h"
#include "MessageLogResource.h"
#include "Layer.h"
#include "LayerExporter.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "StringUtilities.h"
#include "TiePointLayer.h"
#include "UtilityServices.h"
#include "xmlwriter.h"

#include <string>

using namespace std;

LayerExporter::LayerExporter(LayerType layerType)
{

   string userLayerSubclass = 
      StringUtilities::toDisplayString(layerType);
   setName(userLayerSubclass + " Layer Exporter");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);

   string extensions;
   switch (layerType)
   {
   case ANNOTATION:
      mLayerSubclass = TypeConverter::toString<AnnotationLayer>();
      extensions = "Annotation Layers (*.anolayer)";
      break;
   case AOI_LAYER:
      mLayerSubclass = TypeConverter::toString<AoiLayer>();
      extensions = "AOI Layers (*.aoilayer)";
      break;
   case GCP_LAYER:
      mLayerSubclass = TypeConverter::toString<GcpLayer>();
      extensions = "GCP Layers (*.gcplayer)";
      break;
   case TIEPOINT_LAYER:
      mLayerSubclass = TypeConverter::toString<TiePointLayer>();
      extensions = "Tiepoint Layers (*.tielayer)";
      break;
   default:
      break;
   }
   setExtensions(extensions);
   setSubtype(mLayerSubclass);
   setDescription("Export " + userLayerSubclass + " Layers");
   setDescriptorId("{2611EC10-7D17-41c5-AF8C-B41C2A02334D}-" + mLayerSubclass);
   allowMultipleInstances(true);
}

LayerExporter::~LayerExporter()
{
}

bool LayerExporter::getInputSpecification(PlugInArgList*& pInArgList)
{
   pInArgList = mpPlugInManager->getPlugInArgList();
   VERIFY(pInArgList != NULL);

   PlugInArg *pArg;
   VERIFY((pArg = mpPlugInManager->getPlugInArg()) != NULL);
   pArg->setName(ProgressArg());
   pArg->setType("Progress");
   pArg->setDefaultValue(NULL);
   pInArgList->addArg(*pArg);

   VERIFY((pArg = mpPlugInManager->getPlugInArg()) != NULL);
   pArg->setName(ExportDescriptorArg());
   pArg->setType("FileDescriptor");
   pArg->setDefaultValue(NULL);
   pInArgList->addArg(*pArg);

   VERIFY((pArg = mpPlugInManager->getPlugInArg()) != NULL);
   pArg->setName(ExportItemArg());
   pArg->setType(mLayerSubclass);
   pArg->setDefaultValue(NULL);
   pInArgList->addArg(*pArg);

   return true;
}

bool LayerExporter::getOutputSpecification(PlugInArgList*& pOutArgList)
{
   pOutArgList = NULL;
   return true;
}

bool LayerExporter::execute(PlugInArgList *pInArgList, PlugInArgList *pOutArgList)
{
   Progress *pProgress = NULL;
   FileDescriptor *pFileDescriptor = NULL;
   Layer *pLayer = NULL;

   StepResource pStep("Export layer", "app", "D79729F9-C2D8-47F0-BC8D-3257F4744961");

   // get input arguments and log some useful info about them
   { // scope the MessageResource
      MessageResource pMsg("Input arguments", "app", "F81DBC70-FBE6-4D08-8F2F-96E5296DDBCC");

      pProgress = pInArgList->getPlugInArgValue<Progress>(ProgressArg());
      pMsg->addBooleanProperty("Progress Present", (pProgress != NULL));

      pFileDescriptor = pInArgList->getPlugInArgValue<FileDescriptor>(ExportDescriptorArg());
      if(pFileDescriptor == NULL)
      {
         if(pProgress != NULL)
         {
            pProgress->updateProgress("No file specified", 0, ERRORS);
         }
         pStep->finalize(Message::Failure, "No file specified");
         return false;
      }
      pMsg->addProperty("Destination", pFileDescriptor->getFilename());

      pLayer = pInArgList->getPlugInArgValueUnsafe<Layer>(ExportItemArg());
      if(pLayer == NULL)
      {
         if(pProgress != NULL)
         {
            pProgress->updateProgress("No layer specified", 0, ERRORS);
         }
         pStep->finalize(Message::Failure, "No layer specified");
         return false;
      }

      string layerName = pLayer->getName();
      pMsg->addProperty("Name", layerName);
   }

   LayerType ltype = pLayer->getLayerType();
   if(! (ltype == ANNOTATION ||
         ltype == AOI_LAYER ||
         ltype == GCP_LAYER ||
         ltype == GRAPHIC_LAYER ||
         ltype == TIEPOINT_LAYER))
   {
      string msg = "Can not export layers of type ";
      msg += StringUtilities::toDisplayString(pLayer->getLayerType());
      if(pProgress != NULL)
      {
         pProgress->updateProgress(msg, 0, ERRORS);
      }
      pStep->finalize(Message::Failure, msg);
      return false;
   }

   if(pProgress != NULL)
   {
      pProgress->updateProgress("Open output file", 30, NORMAL);
   }
   FileResource pFile(pFileDescriptor->getFilename().getFullPathAndName().c_str(), "w");
   if(pFile.get() == NULL)
   {
      if(pProgress != NULL)
      {
         pProgress->updateProgress("File can not be created", 0, ERRORS);
      }
      pStep->finalize(Message::Failure, "File can not be created");
      return false;
   }

   // before we can save, make sure the element has a valid FileDescriptor
   DataElement *pDataElement = pLayer->getDataElement();
   if(pDataElement != NULL)
   {
      DataDescriptor *pElementDescriptor = pDataElement->getDataDescriptor();
      if(pElementDescriptor->getFileDescriptor() == NULL)
      {
         pElementDescriptor->setFileDescriptor(pFileDescriptor->copy());
      }
   }

   if(pProgress != NULL)
   {
      pProgress->updateProgress("Save layer", 60, NORMAL);
   }
   string elementName;
   elementName = StringUtilities::toXmlString(pLayer->getLayerType());
   if(elementName.empty())
   {
      string msg = "Error serializing layer type.";
      if(pProgress != NULL)
      {
         pProgress->updateProgress(msg, 0, ERRORS);
      }
      pStep->finalize(Message::Failure, msg);
      return false;
   }
   XMLWriter xml(elementName.c_str(),
                  Service<MessageLogMgr>()->getLog());
   if(!pLayer->toXml(&xml))
   {
      if(pProgress != NULL)
      {
         pProgress->updateProgress("Error saving layer", 0, ERRORS);
      }
      pStep->finalize(Message::Failure, "Error saving layer");
      return false;
   }
   else
   {
      xml.writeToFile(pFile);
   }

   if(pProgress != NULL)
   {
      pProgress->updateProgress("Finished saving the layer", 100, NORMAL);
   }
   pStep->finalize(Message::Success);
   return true;
}
