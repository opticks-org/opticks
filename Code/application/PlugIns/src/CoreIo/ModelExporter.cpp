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
#include "DataDescriptor.h"
#include "DataElement.h"
#include "FileDescriptor.h"
#include "MessageLogResource.h"
#include "ModelExporter.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "UtilityServices.h"
#include "xmlwriter.h"

#include <string>
using namespace std;

class AnnotationElement;
class AoiElement;
class GcpList;
class TiePointList;

ModelExporter::ModelExporter(const string& dataElementSubclass) :
   mDataElementSubclass(dataElementSubclass)
{
   string userFriendlyName = mDataElementSubclass;
   string extensions;
   if (mDataElementSubclass == TypeConverter::toString<AoiElement>())
   {
      userFriendlyName = "AOI";
      extensions = "AOI Files (*.aoi)";
   }
   else if (mDataElementSubclass == TypeConverter::toString<AnnotationElement>())
   {
      userFriendlyName = "Annotation";
      extensions = "Annotation Files (*.ano)";
   }
   else if (mDataElementSubclass == TypeConverter::toString<GcpList>())
   {
      userFriendlyName = "GCP List";
      extensions = "GCP Lists (*.gcp)";
   }
   else if (mDataElementSubclass == TypeConverter::toString<TiePointList>())
   {
      userFriendlyName = "Tie Point List";
      extensions = "Tiepoint List (*.tie)";
   }

   setName(userFriendlyName + " Model Exporter");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setExtensions(extensions);
   setDescription("Export " + userFriendlyName + " Model Elements");
   setSubtype(mDataElementSubclass);
   setDescriptorId("{FB7F13DF-A6FB-4a49-94C5-2A8CD5E3AE7F}-" + mDataElementSubclass);
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

ModelExporter::~ModelExporter()
{
}

bool ModelExporter::getInputSpecification(PlugInArgList*& pInArgList)
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
   pArg->setName(ExportDescriptorArg());
   pArg->setType("FileDescriptor");
   pArg->setDefaultValue(NULL);
   pInArgList->addArg(*pArg);

   VERIFY((pArg = mpPlugInManager->getPlugInArg()) != NULL);
   pArg->setName(ExportItemArg());
   pArg->setType(mDataElementSubclass);
   pArg->setDefaultValue(NULL);
   pInArgList->addArg(*pArg);

   return true;
}

bool ModelExporter::getOutputSpecification(PlugInArgList*& pOutArgList)
{
   pOutArgList = NULL;
   return true;
}

bool ModelExporter::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   Progress* pProgress = NULL;
   FileDescriptor* pFileDescriptor = NULL;
   DataElement* pElement = NULL;

   StepResource pStep("Export model element", "app", "2BF48AAB-4832-4694-8583-882A8D584E97");

   // get input arguments and log some useful info about them
   { // scope the MessageResource
      MessageResource pMsg("Input arguments", "app", "1B02F950-2E04-4BEF-8561-BB8D993340F7");

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

      pElement = pInArgList->getPlugInArgValueUnsafe<DataElement>(ExportItemArg());
      if (pElement == NULL)
      {
         if (pProgress != NULL)
         {
            pProgress->updateProgress("No model element specified", 100, ERRORS);
         }
         pStep->finalize(Message::Failure, "No model element specified");
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

   DataDescriptor* pElementDescriptor = pElement->getDataDescriptor();
   VERIFY(pElementDescriptor != NULL);

   if (pProgress != NULL)
   {
      pProgress->updateProgress("Save element", 20, NORMAL);
   }
   string elementName = pElementDescriptor->getType();
   XMLWriter xml(elementName.c_str(), Service<MessageLogMgr>()->getLog());
   if (!pElement->toXml(&xml))
   {
      if (pProgress != NULL)
      {
         pProgress->updateProgress("Error saving model element", 100, ERRORS);
      }
      pStep->finalize(Message::Failure, "Error saving model element");
      return false;
   }
   else
   {
      xml.writeToFile(pFile);
   }
   fclose(pFile);

   if (pProgress != NULL)
   {
      pProgress->updateProgress("Finished saving the model element", 100, NORMAL);
   }
   pStep->finalize(Message::Success);
   return true;
}
