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
#include "DynamicObject.h"
#include "FileDescriptor.h"
#include "MessageLogResource.h"
#include "MetadataExporter.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "Progress.h"
#include "xmlwriter.h"

#include <string>

MetadataExporter::MetadataExporter()
{
   setName("Metadata Exporter");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setExtensions("Metadata Files (*.metadata.xml)");
   setDescription("Export Metadata from Data Elements");
   setSubtype("DataElement");
   setDescriptorId("{f48cae61-5827-45b7-8a0e-bf1a2d9f2f62}");
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

MetadataExporter::~MetadataExporter()
{
}

bool MetadataExporter::getInputSpecification(PlugInArgList*& pInArgList)
{
   VERIFY(pInArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   VERIFY(pInArgList->addArg<Progress>(ProgressArg(), NULL));
   VERIFY(pInArgList->addArg<FileDescriptor>(ExportDescriptorArg()));
   VERIFY(pInArgList->addArg<DataElement>(ExportItemArg()));
   return true;
}

bool MetadataExporter::getOutputSpecification(PlugInArgList*& pOutArgList)
{
   pOutArgList = NULL;
   return true;
}

bool MetadataExporter::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   Progress* pProgress = NULL;
   FileDescriptor* pFileDescriptor = NULL;
   DataElement* pElement = NULL;

   StepResource pStep("Export metadata", "app", "{08701b89-565c-4e0a-92ef-9bf22395f902}");

   // get input arguments and log some useful info about them
   { // scope the MessageResource
      MessageResource pMsg("Input arguments", "app", "{5e921da0-6470-44f1-a910-ed12af1e5ebc}");

      pProgress = pInArgList->getPlugInArgValue<Progress>(ProgressArg());
      pMsg->addBooleanProperty("Progress Present", (pProgress != NULL));

      pFileDescriptor = pInArgList->getPlugInArgValue<FileDescriptor>(ExportDescriptorArg());
      if (pFileDescriptor == NULL)
      {
         if (pProgress != NULL)
         {
            pProgress->updateProgress("No file specified", 100, ERRORS);
         }
         pStep->finalize(Message::Failure, "No file specified");
         return false;
      }
      pMsg->addProperty("Destination", pFileDescriptor->getFilename());

      pElement = pInArgList->getPlugInArgValue<DataElement>(ExportItemArg());
      if (pElement == NULL)
      {
         if (pProgress != NULL)
         {
            pProgress->updateProgress("No data element specified", 100, ERRORS);
         }
         pStep->finalize(Message::Failure, "No data element specified");
         return false;
      }
      pMsg->addProperty("Name", pElement->getName());
   }

   if (pProgress != NULL)
   {
      pProgress->updateProgress("Open output file", 10, NORMAL);
   }
   FILE* pFile = fopen(pFileDescriptor->getFilename().getFullPathAndName().c_str(), "w");
   if (pFile == NULL)
   {
      if (pProgress != NULL)
      {
         pProgress->updateProgress("File can not be created", 100, ERRORS);
      }
      pStep->finalize(Message::Failure, "File can not be created");
      return false;
   }

   const DynamicObject* pMetadata = pElement->getMetadata();
   VERIFY(pMetadata);
   if (pProgress != NULL)
   {
      pProgress->updateProgress("Save metadata", 20, NORMAL);
   }
   if (pMetadata->getNumAttributes() == 0)
   {
      if (pProgress != NULL)
      {
         pProgress->updateProgress("Metadata is empty. A file will be created anyway.", 20, WARNING);
      }
      pStep->addMessage("Metadata is empty. A file will be created anyway.", "app", "{29274eb3-c899-4778-ae1e-d267ea0dd346}", true);
   }
   XMLWriter xml("Metadata", Service<MessageLogMgr>()->getLog());
   if (!pMetadata->toXml(&xml))
   {
      if (pProgress != NULL)
      {
         pProgress->updateProgress("Error saving metadata", 100, ERRORS);
      }
      pStep->finalize(Message::Failure, "Error saving metadata");
      return false;
   }
   else
   {
      xml.writeToFile(pFile);
   }
   fclose(pFile);

   if (pProgress != NULL)
   {
      pProgress->updateProgress("Finished saving the metadata", 100, NORMAL);
   }
   pStep->finalize(Message::Success);
   return true;
}
