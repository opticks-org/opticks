/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <string>

#include "AppVersion.h"
#include "FeatureClass.h"
#include "FeatureLayerExporter.h"
#include "FileDescriptor.h"
#include "FileResource.h"
#include "MessageLogResource.h"
#include "ModelServices.h"
#include "PlugInArgList.h"
#include "PlugInRegistration.h"
#include "Progress.h"
#include "xmlwriter.h"

REGISTER_PLUGIN_BASIC(OpticksGeographicFeatures, FeatureLayerExporter);

FeatureLayerExporter::FeatureLayerExporter()
{
   setName("Feature Layer Exporter");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setDescriptorId("{A7542E33-B73C-44E4-ADEC-1B260FF79D2E}");
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
   setSubtype("FeatureClass");
}

FeatureLayerExporter::~FeatureLayerExporter()
{}

bool FeatureLayerExporter::getInputSpecification(PlugInArgList*& pArgList)
{
   Service<PlugInManagerServices> pPluginMgr;
   pArgList = pPluginMgr->getPlugInArgList();
   VERIFY(pArgList != NULL);

   VERIFY(pArgList->addArg<Progress>(Executable::ProgressArg(), NULL, Executable::ProgressArgDescription()));
   VERIFY(pArgList->addArg<FileDescriptor>(Exporter::ExportDescriptorArg(), NULL,
      "File descriptor for the output file."));
   VERIFY(pArgList->addArg<Any>(Exporter::ExportItemArg(), NULL, "The FeatureClass to be exported as a shape file"));

   return true;
}

bool FeatureLayerExporter::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Execute Geographic Feature Layer Exporter", "app", "00AC1AF1-BF62-4B4F-8DA9-138B8DDCD3EE");
   std::string msg;
   if (pInArgList == NULL)
   {
      msg = "Input argument list is invalid.";
      pStep->finalize(Message::Failure, msg);
      return false;
   }

   Progress* pProgress = pInArgList->getPlugInArgValue<Progress>(Executable::ProgressArg());

   FileDescriptor* pFileDesc = pInArgList->getPlugInArgValue<FileDescriptor>(Exporter::ExportDescriptorArg());
   if (pFileDesc == NULL)
   {
      msg = "No file specified.";
      if (pProgress != NULL)
      {
         pProgress->updateProgress(msg, 0, ERRORS);
      }
      pStep->finalize(Message::Failure, msg);
      return false;
   }
   const Filename& filename = pFileDesc->getFilename();

   Any* pAny = pInArgList->getPlugInArgValue<Any>(Exporter::ExportItemArg());
   if (pAny == NULL)
   {
      msg = "Input argument list did not include anything to export.";
      if (pProgress != NULL)
      {
         pProgress->updateProgress(msg, 0, ERRORS);
      }
      pStep->finalize(Message::Failure, msg);
      return false;
   }

   FeatureClass* pFeatureClass = model_cast<FeatureClass*>(pAny);
   if (pFeatureClass == NULL)
   {
      msg = "Input argument list did not contain FeatureClass.";
      if (pProgress != NULL)
      {
         pProgress->updateProgress(msg, 0, ERRORS);
      }
      pStep->finalize(Message::Failure, msg);
      return false;
   }

   FileResource pFile(filename.getFullPathAndName().c_str(), "wt");
   if (pFile == NULL)
   {
      msg = "Invalid filename.";
      if (pProgress != NULL)
      {
         pProgress->updateProgress(msg, 0, ERRORS);
      }
      pStep->finalize(Message::Failure, msg);
      return false;
   }

   FactoryResource<DynamicObject> pDynObj = pFeatureClass->toDynamicObject();

   XMLWriter writer("GeoFeatureLayer");
   if (pDynObj->toXml(&writer))
   {
      writer.writeToFile(pFile);
      msg = "Saved: " + filename.getFullPathAndName();
      if (pProgress != NULL)
      {
         pProgress->updateProgress(msg, 100, NORMAL);
      }
      pStep->addProperty("sourceFile", filename.getFullPathAndName());
      pStep->finalize(Message::Success);
   }
   else
   {
      msg = "Error saving: " + filename.getFullPathAndName();
      if (pProgress != NULL)
      {
         pProgress->updateProgress(msg, 0, ERRORS);
      }
      pStep->finalize(Message::Failure, msg);
      return false;
   }

   return true;
}

std::string FeatureLayerExporter::getDefaultExtensions() const
{
   return "Geographic Feature Layer Files (*.geolayer)";
}
