/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ConfigurationSettings.h"
#include "DataElement.h"
#include "DynamicObject.h"
#include "Metadata.h"
#include "ObjectResource.h"
#include "SimpleApiErrors.h"

extern "C"
{
   DynamicObject* createDynamicObject()
   {
      FactoryResource<DynamicObject> pDynamicObject;
      if (pDynamicObject.get() == NULL)
      {
         setLastError(SIMPLE_NO_MEM);
         return NULL;
      }
      setLastError(SIMPLE_NO_ERROR);
      return pDynamicObject.release();
   }

   void freeDynamicObject(DynamicObject* pDynamicObject)
   {
      if (pDynamicObject != NULL)
      {
         FactoryResource<DynamicObject> pObj(pDynamicObject);
      }
   }

   DynamicObject* getDataElementMetadata(DataElement* pElement)
   {
      if (pElement == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return NULL;
      }
      setLastError(SIMPLE_NO_ERROR);
      return pElement->getMetadata();
   }

   uint32_t getMetadataAttributeCount(DynamicObject* pMeta)
   {
      if (pMeta == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      setLastError(SIMPLE_NO_ERROR);
      return pMeta->getNumAttributes();
   }

   uint32_t getMetadataAttributeName(DynamicObject* pMeta, uint32_t attributeIndex, char* pName, uint32_t nameSize)
   {
      if (pMeta == NULL || pName == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      std::vector<std::string> names;
      pMeta->getAttributeNames(names);
      if (attributeIndex >= names.size())
      {
         setLastError(SIMPLE_NOT_FOUND);
         return 0;
      }
      if (nameSize == 0)
      {
         return names[attributeIndex].size() + 1;
      }
      else if (names[attributeIndex].size() > nameSize - 1) // extra char for the null
      {
         setLastError(SIMPLE_BUFFER_SIZE);
         return 0;
      }
      memcpy(pName, names[attributeIndex].c_str(), names[attributeIndex].size());
      pName[names[attributeIndex].size()] = '\0';
      setLastError(SIMPLE_NO_ERROR);
      return names[attributeIndex].size();
   }

   DataVariant* getMetadataAttribute(DynamicObject* pMeta, const char* pName)
   {
      if (pMeta == NULL || pName == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return NULL;
      }
      DataVariant& attribute(pMeta->getAttribute(std::string(pName)));
      setLastError(SIMPLE_NO_ERROR);
      return &attribute;
   }

   DataVariant* getMetadataAttributeByPath(DynamicObject* pMeta, const char* pPath)
   {
      if (pMeta == NULL || pPath == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return NULL;
      }
      DataVariant& attribute(pMeta->getAttributeByPath(std::string(pPath)));
      setLastError(SIMPLE_NO_ERROR);
      return &attribute;
   }

   void setMetadataAttribute(DynamicObject* pMeta, const char* pName, DataVariant* pValue)
   {
      if (pMeta == NULL || pName == NULL || pValue == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return;
      }
      if (!pMeta->adoptAttribute(std::string(pName), *pValue))
      {
         setLastError(SIMPLE_OTHER_FAILURE);
         return;
      }
      setLastError(SIMPLE_NO_ERROR);
   }

   void setMetadataAttributeByPath(DynamicObject* pMeta, const char* pPath, DataVariant* pValue)
   {
      if (pMeta == NULL || pPath == NULL || pValue == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return;
      }
      if (!pMeta->adoptAttributeByPath(std::string(pPath), *pValue))
      {
         setLastError(SIMPLE_OTHER_FAILURE);
         return;
      }
      setLastError(SIMPLE_NO_ERROR);
   }

   void removeMetadataAttribute(DynamicObject* pMeta, const char* pName)
   {
      if (pMeta == NULL || pName == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return;
      }
      if (!pMeta->removeAttribute(std::string(pName)))
      {
         setLastError(SIMPLE_OTHER_FAILURE);
         return;
      }
      setLastError(SIMPLE_NO_ERROR);
   }

   void removeMetadataAttributeByPath(DynamicObject* pMeta, const char* pPath)
   {
      if (pMeta == NULL || pPath == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return;
      }
      if (!pMeta->removeAttributeByPath(std::string(pPath)))
      {
         setLastError(SIMPLE_OTHER_FAILURE);
         return;
      }
      setLastError(SIMPLE_NO_ERROR);
   }

   void clearMetadata(DynamicObject* pMeta)
   {
      if (pMeta == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return;
      }
      pMeta->clear();
      setLastError(SIMPLE_NO_ERROR);
   }

   DataVariant* getConfigurationSetting(const char* pSettingKey)
   {
      if (pSettingKey == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return NULL;
      }
      const DataVariant& value(Service<ConfigurationSettings>()->getSetting(std::string(pSettingKey)));
      setLastError(SIMPLE_NO_ERROR);
      return &const_cast<DataVariant&>(value);
   }

   int setConfigurationSetting(const char* pSettingKey, DataVariant* pValue)
   {
      if (pSettingKey == NULL || pValue == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 1;
      }
      if (!Service<ConfigurationSettings>()->adoptSetting(std::string(pSettingKey), *pValue))
      {
         setLastError(SIMPLE_OTHER_FAILURE);
         return 1;
      }
      setLastError(SIMPLE_NO_ERROR);
      return 0;
   }

   int copyConfigurationSetting(const char* pSettingKey, DynamicObject* pDynamicObject)
   {
      if (pSettingKey == NULL || pDynamicObject == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 1;
      }
      Service<ConfigurationSettings>()->copySetting(std::string(pSettingKey), pDynamicObject);
      setLastError(SIMPLE_NO_ERROR);
      return 0;
   }

   int serializeConfigurationSettingDefaults(const char* pFilename, DynamicObject* pDynamicObject)
   {
      if (pFilename == NULL || pDynamicObject == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 1;
      }
      FactoryResource<Filename> pFile;
      pFile->setFullPathAndName(std::string(pFilename));
      if (!Service<ConfigurationSettings>()->serializeAsDefaults(pFile.get(), pDynamicObject))
      {
         setLastError(SIMPLE_OTHER_FAILURE);
         return 1;
      }
      setLastError(SIMPLE_NO_ERROR);
      return 0;
   }
}