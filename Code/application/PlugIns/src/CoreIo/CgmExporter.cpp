/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AnnotationLayer.h"
#include "AppVersion.h"
#include "CgmExporter.h"
#include "CgmObject.h"
#include "FileDescriptor.h"
#include "GraphicElement.h"
#include "MessageLogResource.h"
#include "PlugInArgList.h"
#include "Progress.h"

#include <string>
using namespace std;

CgmExporter::CgmExporter()
{
   setName("CGM Exporter");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setExtensions("CGM Files (*.cgm)");
   setSubtype(TypeConverter::toString<AnnotationLayer>());
   setDescription("Export Annotation Layers as CGM files");
   setDescriptorId("{E915CFD0-21D1-4119-B9AE-26860F50E53A}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

CgmExporter::~CgmExporter()
{
}

bool CgmExporter::getInputSpecification(PlugInArgList*& pInArgList)
{
   pInArgList = mpPlugInManager->getPlugInArgList();
   VERIFY(pInArgList != NULL);

   bool success = true;
   success = success && pInArgList->addArg<Progress>(ProgressArg(), NULL);
   success = success && pInArgList->addArg<FileDescriptor>(ExportDescriptorArg(), NULL);
   success = success && pInArgList->addArg<AnnotationLayer>(ExportItemArg(), NULL);

   return success;
}

bool CgmExporter::getOutputSpecification(PlugInArgList*& pOutArgList)
{
   pOutArgList = NULL;
   return true;
}

bool CgmExporter::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   Progress* pProgress = NULL;
   FileDescriptor* pFileDescriptor = NULL;
   AnnotationLayer* pLayer = NULL;

   StepResource pStep("Export layer", "app", "09A23593-7065-4bc6-AF0B-935D2E735926");

   // get input arguments and log some useful info about them
   { // scope the MessageResource
      MessageResource pMsg("Input arguments", "app", "9546476B-1B2B-47db-8956-8299C96CD4A4");

      pProgress = pInArgList->getPlugInArgValue<Progress>(ProgressArg());
      pMsg->addBooleanProperty("Progress Present", (pProgress != NULL));

      pFileDescriptor = pInArgList->getPlugInArgValue<FileDescriptor>(ExportDescriptorArg());
      if (pFileDescriptor == NULL)
      {
         if (pProgress != NULL)
         {
            pProgress->updateProgress("No file specified", 0, ERRORS);
         }
         pStep->finalize(Message::Failure, "No file specified");
         return false;
      }
      pMsg->addProperty("Destination", pFileDescriptor->getFilename());

      pLayer = pInArgList->getPlugInArgValue<AnnotationLayer>(ExportItemArg());
      if (pLayer == NULL)
      {
         if (pProgress != NULL)
         {
            pProgress->updateProgress("No layer specified", 0, ERRORS);
         }
         pStep->finalize(Message::Failure, "No layer specified");
         return false;
      }

      string layerName = pLayer->getName();
      pMsg->addProperty("Name", layerName);
   }

   if (pProgress != NULL)
   {
      pProgress->updateProgress("Exporting Annotation Layer as CGM file.", 0, NORMAL);
   }

   GraphicElement* pElement = dynamic_cast<GraphicElement*>(pLayer->getDataElement());
   if (pElement == NULL)
   {
      if (pProgress != NULL)
      {
         pProgress->updateProgress("Invalid layer provided", 0, ERRORS);
      }
      pStep->finalize(Message::Failure, "Invalid layer provided");
      return false;
   }

   GraphicGroup* pGroup = pElement->getGroup();
   VERIFY(pGroup != NULL);

   CgmObject* pCgm = pGroup->convertToCgm();
   if (pCgm == NULL)
   {
      if (pProgress != NULL)
      {
         pProgress->updateProgress("Cannot convert Annotation Layer into Cgm", 0, ERRORS);
      }
      pStep->finalize(Message::Failure, "Cannot convert Annotation Layer into Cgm");
      return false;
   }

   bool success = pCgm->serializeCgm(pFileDescriptor->getFilename().getFullPathAndName());
   if (!success)
   {
      if (pProgress != NULL)
      {
         pProgress->updateProgress("Unable to save cgm file.", 0, ERRORS);
      }
      pStep->finalize(Message::Failure, "Unable to save cgm file.");
   }
   else
   {
      if (pProgress != NULL)
      {
         pProgress->updateProgress("CGM export complete.", 100, NORMAL);
      }
      pStep->finalize();
   }
   return success;
}
