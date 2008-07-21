/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "DataDescriptor.h"
#include "FileDescriptor.h"
#include "FeatureClass.h"
#include "FeatureLayerImporter.h"
#include "FeatureProxyConnector.h"
#include "ImportDescriptor.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "Progress.h"
#include "XercesIncludes.h"
#include "xmlreader.h"

#include <algorithm>
#include <cctype>

XERCES_CPP_NAMESPACE_USE

const std::string FeatureLayerImporter::FEATURE_DYNAMIC_OBJECT_ARG = "Feature Dynamic Object";
const std::string FeatureLayerImporter::PLUGIN_NAME = "Geographic Feature Layer Importer";
const std::string FeatureLayerImporter::FILE_PLACEHOLDER = "FilePlaceholder";

FeatureLayerImporter::FeatureLayerImporter() : ShapeFileImporter(), mpFeatureDynObj(NULL)
{
   setName(PLUGIN_NAME);
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setDescriptorId("{6713AF9F-0FAE-45eb-83BF-4A012D41AF7A}");
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
   setExtensions("Geographic feature layer files (*.geolayer)");
   setSubtype("Shape File");
   setAbortSupported(false);
}

FeatureLayerImporter::~FeatureLayerImporter()
{
}

unsigned char FeatureLayerImporter::getFileAffinity(const std::string& filename)
{
   if (filename == FILE_PLACEHOLDER)
   {
      return Importer::CAN_LOAD;
   }

   XmlReader xml(NULL, false);
   XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* pDoc = xml.parse(filename, "GeoFeatureLayer");
   if (pDoc == NULL)
   {
      return Importer::CAN_NOT_LOAD;
   }

   DOMElement* pRootElement = pDoc->getDocumentElement();
   if (pRootElement == NULL)
   {
      return Importer::CAN_NOT_LOAD;
   }

   if (std::string(A(pRootElement->getTagName())) != "GeoFeatureLayer")
   {
      return Importer::CAN_NOT_LOAD;
   }

   return Importer::CAN_LOAD;
}

std::vector<ImportDescriptor*> FeatureLayerImporter::getImportDescriptors(const std::string& filename)
{
   std::vector<ImportDescriptor*> descriptors;

   if (getFileAffinity(filename) != Importer::CAN_NOT_LOAD)
   {
      Service<ModelServices> pModel;

      ImportDescriptor* pImportDescriptor = pModel->createImportDescriptor(filename, "AnnotationElement", NULL);
      VERIFYRV(pImportDescriptor != NULL, descriptors);

      DataDescriptor* pDescriptor = pImportDescriptor->getDataDescriptor();
      VERIFYRV(pDescriptor != NULL, descriptors);

      if (filename != FILE_PLACEHOLDER)
      {
         FactoryResource<FileDescriptor> pFileDescriptor;
         pFileDescriptor->setFilename(filename);
         pDescriptor->setFileDescriptor(pFileDescriptor.get());
      }

      descriptors.push_back(pImportDescriptor);
   }

   return descriptors;
}

bool FeatureLayerImporter::getInputSpecification(PlugInArgList*& pArgList)
{
   if (!ShapeFileImporter::getInputSpecification(pArgList))
   {
      return false;
   }

   pArgList->addArg<DynamicObject>(FEATURE_DYNAMIC_OBJECT_ARG, NULL);

   return true;
}

bool FeatureLayerImporter::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   VERIFY(pInArgList != NULL)
   mpFeatureDynObj = pInArgList->getPlugInArgValue<DynamicObject>(FEATURE_DYNAMIC_OBJECT_ARG);
 
   return ShapeFileImporter::execute(pInArgList, pOutArgList);
}

void FeatureLayerImporter::createFeatureClassIfNeeded(const DataDescriptor *pDescriptor)
{
   if (mpFeatureClass.get() == NULL)
   {
      FactoryResource<DynamicObject> pDynObj;
      VERIFYNRV(pDynObj.get() != NULL);

      DynamicObject *pFeatureDynObj = mpFeatureDynObj;
      
      if (pFeatureDynObj == NULL)
      {

         VERIFYNRV(pDescriptor != NULL);
         const FileDescriptor *pFileDescriptor = pDescriptor->getFileDescriptor();
         // If pFileDescriptor is NULL, then FILE_PLACEHOLDER must have been used.
         // Either mpFeatureDynObj is non-NULL (use that) or the FeatureClass should be default-constructed
         if (pFileDescriptor != NULL)
         {
            pFeatureDynObj = pDynObj.get();

            XmlReader xml(Service<MessageLogMgr>()->getLog(), false);
            XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* pDoc = xml.parse(pFileDescriptor->getFilename());
            if (pDoc == NULL)
            {
               mMessageText = "Could not read file.";
               if(mpProgress) mpProgress->updateProgress(mMessageText, 0, ERRORS);
               if(mpStep) mpStep->finalize(Message::Failure, mMessageText);
               return;
            }

            DOMElement* pRootElement = pDoc->getDocumentElement();
            if (pRootElement == NULL)
            {
               mMessageText = "Could not read file.";
               if(mpProgress) mpProgress->updateProgress(mMessageText, 0, ERRORS);
               if(mpStep) mpStep->finalize(Message::Failure, mMessageText);
               return;
            }

            bool deserialized = false;
            unsigned int formatVersion = atoi(A(pRootElement->getAttribute(X("version"))));
            for(DOMNode *pChild = pRootElement->getFirstChild(); 
               pChild != NULL && !deserialized; pChild = pChild->getNextSibling())
            {
               if(XMLString::equals(pChild->getNodeName(), X("DynamicObject")))
               {
                  deserialized = pDynObj->fromXml(pChild, formatVersion);
               }
            }
            if (!deserialized)
            {
               mMessageText = "Could not read file.";
               if(mpProgress) mpProgress->updateProgress(mMessageText, 0, ERRORS);
               if(mpStep) mpStep->finalize(Message::Failure, mMessageText);
               return;
            }
         }
      }

      std::auto_ptr<FeatureClass> pFeatureClass(new FeatureClass);
      if (pFeatureClass.get() == NULL)
      {
         mMessageText = "Could not read file.";
         if(mpProgress) mpProgress->updateProgress(mMessageText, 0, ERRORS);
         if(mpStep) mpStep->finalize(Message::Failure, mMessageText);
         return;
      }
        
      if (pFeatureDynObj != NULL)
      {
         if (!pFeatureClass->fromDynamicObject(pFeatureDynObj))
         {
            mMessageText = "Could not read file.";
            if(mpProgress) mpProgress->updateProgress(mMessageText, 0, ERRORS);
            if(mpStep) mpStep->finalize(Message::Failure, mMessageText);
            return;
         }
      }
      mpFeatureClass = pFeatureClass;
   }
}

std::vector<ArcProxyLib::ConnectionType> FeatureLayerImporter::getAvailableConnectionTypes()
{
   FeatureProxyConnector *pProxy = FeatureProxyConnector::instance();
   VERIFYRV(pProxy != NULL, std::vector<ArcProxyLib::ConnectionType>());

   return pProxy->getAvailableConnectionTypes();
}
