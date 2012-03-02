/*
* The information in this file is
* Copyright(c) 2007 Ball Aerospace & Technologies Corporation
* and is subject to the terms and conditions of the
* GNU Lesser General Public License Version 2.1
* The license text is available from   
* http://www.gnu.org/licenses/lgpl.html
*/

#include "AnnotationElement.h"
#include "AoiElement.h"
#include "AoiLayer.h"
#include "Any.h"
#include "ApiUtilities.h"
#include "ClassificationLayer.h"
#include "CustomLayer.h"
#include "DataElementGroup.h"
#include "DesktopServices.h"
#include "GcpLayer.h"
#include "GcpList.h"
#include "LatLonLayer.h"
#include "Layer.h"
#include "LayerList.h"
#include "MeasurementLayer.h"
#include "ModelServices.h"
#include "PlugInRegistration.h"
#include "PlugInResource.h"
#include "PseudocolorLayer.h"
#include "RasterElement.h"
#include "RasterLayer.h"
#include "SessionManager.h"
#include "SignatureLibrary.h"
#include "SimpleApiErrors.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "StringUtilities.h"
#include "TiePointList.h"
#include "TestUtilities.h"
#include "ThresholdLayer.h"
#include "TiePointLayer.h"
#include "TypeConverter.h"

namespace
{
   std::vector<std::string> splitIdentifier(const std::string& name)
   {
      return StringUtilities::split(name, '|');
   }
}

extern "C"
{
   void setHandle(void* pExternal)
   {
      ModuleManager::instance()->setService(reinterpret_cast<External*>(pExternal));
   }

   void* handle()
   {
      return ModuleManager::instance()->getService();
   }

   uint32_t getOpticksVersion(char* pVersion, uint32_t versionSize)
   {
      if (pVersion == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      std::string versionNumber = Service<ConfigurationSettings>()->getVersion();
      if (versionSize == 0)
      {
         return versionNumber.size() + 1;
      }
      else if (versionNumber.size() > versionSize - 1) // extra char for the null
      {
         setLastError(SIMPLE_BUFFER_SIZE);
         return 0;
      }
      memcpy(pVersion, versionNumber.c_str(), versionNumber.size());
      pVersion[versionNumber.size()] = '\0';
      setLastError(SIMPLE_NO_ERROR);
      return versionNumber.size();
   }

   uint32_t getTestDataPath(char* pPath, uint32_t pathSize)
   {
      if (pPath == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      std::string testPath = TestUtilities::getTestDataPath();
      if (pathSize == 0)
      {
         return testPath.size() + 1;
      }
      else if (testPath.size() > pathSize - 1) // extra char for the null
      {
         setLastError(SIMPLE_BUFFER_SIZE);
         return 0;
      }
      memcpy(pPath, testPath.c_str(), testPath.size());
      pPath[testPath.size()] = '\0';
      setLastError(SIMPLE_NO_ERROR);
      return testPath.size();
   }

   uint32_t loadFile(const char* pFilename, int batch)
   {
      if (pFilename == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      ImporterResource import("Auto Importer", std::string(pFilename), NULL, (batch != 0));
      if (!import->execute())
      {
         setLastError(SIMPLE_NOT_FOUND);
         return 0;
      }
      setLastError(SIMPLE_NO_ERROR);
      return import->getImportedElements().size();
   }

   DataElement* getDataElement(const char* pName, const char* pType, int create)
   {
      DataElement* pElement = NULL;
      if (pName == NULL || (pType == NULL && create != 0))
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return NULL;
      }
      const std::string name(pName);
      const std::string type(pType == NULL ? std::string() : pType);
      SessionItem* pSessionItem = Service<SessionManager>()->getSessionItem(name);
      if (pSessionItem != NULL)
      {
         pElement = dynamic_cast<DataElement*>(pSessionItem);
         if (pElement == NULL || (!type.empty()  && !pElement->isKindOf(type)))
         {
            pElement = NULL;
            setLastError(SIMPLE_WRONG_TYPE);
            return NULL;
         }
      }
      else
      {
         std::vector<std::string> id = splitIdentifier(name);
         DataElement* pParent = NULL;
         Service<ModelServices> pModel;
         for (size_t idx = 0; idx < id.size(); ++idx)
         {
            DataElement* pTmp = pModel->getElement(id[idx], (idx == id.size() - 1) ? type : std::string(), pParent);
            if (pTmp == NULL)
            {
               std::vector<DataElement*> children =
                  pModel->getElements(pParent, (idx == id.size() - 1) ? type : std::string());
               for (std::vector<DataElement*>::iterator child = children.begin(); child != children.end(); ++child)
               {
                  if (*child != NULL && (*child)->getDisplayName() == id[idx])
                  {
                     pTmp = *child;
                     break;
                  }
               }
            }
            if (pTmp == NULL && create != 0 && idx == id.size() - 1)
            {
               // Last component not found and creation has been requested
               if (type == TypeConverter::toString<RasterElement>())
               {
                  // Can't use this function to create a RasterElement
                  setLastError(SIMPLE_WRONG_TYPE);
                  return NULL;
               }
               pTmp = Service<ModelServices>()->createElement(id[idx], type, pParent);
            }
            if ((pParent = pTmp) == NULL)
            {
               break;
            }
         }
         if ((pElement = pParent) == NULL)
         {
            setLastError(SIMPLE_NOT_FOUND);
            return NULL;
         }
      }
      setLastError(SIMPLE_NO_ERROR);
      return pElement;
   }

   uint32_t getDataElementName(DataElement* pElement, char* pName, uint32_t nameSize)
   {
      if (pElement == NULL || pName == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      std::string name = pElement->getName();
      if (nameSize == 0)
      {
         return name.size() + 1;
      }
      else if (name.size() > nameSize - 1) // extra char for the null
      {
         setLastError(SIMPLE_BUFFER_SIZE);
         return 0;
      }
      memcpy(pName, name.c_str(), name.size());
      pName[name.size()] = '\0';
      setLastError(SIMPLE_NO_ERROR);
      return name.size();
   }

   uint32_t getDataElementType(DataElement* pElement, char* pType, uint32_t typeSize)
   {
      if (pElement == NULL || pType == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      std::string type = pElement->getType();
      if (typeSize == 0)
      {
         return type.size() + 1;
      }
      else if (type.size() > typeSize - 1) // extra char for the null
      {
         setLastError(SIMPLE_BUFFER_SIZE);
         return 0;
      }
      memcpy(pType, type.c_str(), type.size());
      pType[type.size()] = '\0';
      setLastError(SIMPLE_NO_ERROR);
      return type.size();
   }

   uint32_t getDataElementFilename(DataElement* pElement, char* pFilename, uint32_t filenameSize)
   {
      if (pElement == NULL || pFilename == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      std::string filename = pElement->getFilename();
      if (filenameSize == 0)
      {
         return filename.size() + 1;
      }
      else if (filename.size() > filenameSize - 1) // extra char for the null
      {
         setLastError(SIMPLE_BUFFER_SIZE);
         return 0;
      }
      memcpy(pFilename, filename.c_str(), filename.size());
      pFilename[filename.size()] = '\0';
      setLastError(SIMPLE_NO_ERROR);
      return filename.size();
   }
   
   uint32_t getDataElementChildCount(DataElement* pElement)
   {
      if (pElement == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      std::vector<DataElement*> children = Service<ModelServices>()->getElements(pElement, std::string());
      setLastError(SIMPLE_NO_ERROR);
      return children.size();
   }

   DataElement* getDataElementChild(DataElement* pElement, uint32_t index)
   {
      if (pElement == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return NULL;
      }
      std::vector<DataElement*> children = Service<ModelServices>()->getElements(pElement, std::string());
      if (index >= children.size())
      {
         setLastError(SIMPLE_NOT_FOUND);
         return NULL;
      }
      setLastError(SIMPLE_NO_ERROR);
      return children[index];
   }

   void destroyDataElement(DataElement* pElement)
   {
      Service<ModelServices>()->destroyElement(pElement);
   }

#define CAST_IMP(rval__, typeStr__, type__, val__) if (typeStr__ == #type__) { rval__ = dynamic_cast<type__*>(val__); }
#define CAST_TO_IMP(rval__, typeStr__, type__, val__) if (typeStr__ == #type__) \
   { rval__ = dynamic_cast<DataElement*>(reinterpret_cast<type__*>(val__)); }

   void* castDataElement(DataElement* pElement, const char* pType)
   {
      if (pElement == NULL || pType == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return NULL;
      }
      std::string type(pType);
      void* pSubtype = NULL;
      CAST_IMP(pSubtype, type, DataElement, pElement);
      CAST_IMP(pSubtype, type, Any, pElement);
      CAST_IMP(pSubtype, type, DataElementGroup, pElement);
      CAST_IMP(pSubtype, type, GcpList, pElement);
      CAST_IMP(pSubtype, type, GraphicElement, pElement);
      CAST_IMP(pSubtype, type, AnnotationElement, pElement);
      CAST_IMP(pSubtype, type, AoiElement, pElement);
      CAST_IMP(pSubtype, type, RasterElement, pElement);
      CAST_IMP(pSubtype, type, Signature, pElement);
      CAST_IMP(pSubtype, type, SignatureSet, pElement);
      CAST_IMP(pSubtype, type, SignatureLibrary, pElement);
      CAST_IMP(pSubtype, type, TiePointList, pElement);

      if (pSubtype == NULL)
      {
         setLastError(SIMPLE_WRONG_TYPE);
         return NULL;
      }
      setLastError(SIMPLE_NO_ERROR);
      return pSubtype;
   }

   DataElement* castToDataElement(void* pElement, const char* pType)
   {
      if (pElement == NULL || pType == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return NULL;
      }
      std::string type(pType);
      DataElement* pDe = NULL;
      CAST_TO_IMP(pDe, type, DataElement, pElement);
      CAST_TO_IMP(pDe, type, Any, pElement);
      CAST_TO_IMP(pDe, type, DataElementGroup, pElement);
      CAST_TO_IMP(pDe, type, GcpList, pElement);
      CAST_TO_IMP(pDe, type, GraphicElement, pElement);
      CAST_TO_IMP(pDe, type, AnnotationElement, pElement);
      CAST_TO_IMP(pDe, type, AoiElement, pElement);
      CAST_TO_IMP(pDe, type, RasterElement, pElement);
      CAST_TO_IMP(pDe, type, Signature, pElement);
      CAST_TO_IMP(pDe, type, SignatureSet, pElement);
      CAST_TO_IMP(pDe, type, SignatureLibrary, pElement);
      CAST_TO_IMP(pDe, type, TiePointList, pElement);

      if (pDe == NULL)
      {
         setLastError(SIMPLE_WRONG_TYPE);
         return NULL;
      }
      setLastError(SIMPLE_NO_ERROR);
      return pDe;
   }

   void* castLayer(Layer* pLayer, const char* pType)
   {
      if (pLayer == NULL || pType == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return NULL;
      }
      std::string type(pType);
      void* pSubtype = NULL;
      CAST_IMP(pSubtype, type, Layer, pLayer);
      CAST_IMP(pSubtype, type, GcpLayer, pLayer);
      CAST_IMP(pSubtype, type, GraphicLayer, pLayer);
      CAST_IMP(pSubtype, type, AnnotationLayer, pLayer);
      CAST_IMP(pSubtype, type, ClassificationLayer, pLayer);
      CAST_IMP(pSubtype, type, CustomLayer, pLayer);
      CAST_IMP(pSubtype, type, MeasurementLayer, pLayer);
      CAST_IMP(pSubtype, type, AoiLayer, pLayer);
      CAST_IMP(pSubtype, type, LatLonLayer, pLayer);
      CAST_IMP(pSubtype, type, PseudocolorLayer, pLayer);
      CAST_IMP(pSubtype, type, RasterLayer, pLayer);
      CAST_IMP(pSubtype, type, ThresholdLayer, pLayer);
      CAST_IMP(pSubtype, type, TiePointLayer, pLayer);

      if (pSubtype == NULL)
      {
         setLastError(SIMPLE_WRONG_TYPE);
         return NULL;
      }
      setLastError(SIMPLE_NO_ERROR);
      return pSubtype;
   }

   void copyClassification(DataElement* pCopyFrom, DataElement* pCopyTo)
   {
      if (pCopyTo == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return;
      }
      pCopyTo->copyClassification(pCopyFrom);
      setLastError(SIMPLE_NO_ERROR);
   }
};