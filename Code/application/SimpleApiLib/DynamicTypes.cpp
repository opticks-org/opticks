/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "DataVariant.h"
#include "DynamicTypes.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "SimpleApiErrors.h"

extern "C"
{
   DataVariant* createDataVariant(const char* pType, const void* pValue)
   {
      if (pType == NULL && pValue == NULL)
      {
         return new DataVariant();
      }
      std::string type(pType);
      DataVariant* pDv = new DataVariant(type, pValue, false);
      if (pDv == NULL)
      {
         setLastError(SIMPLE_WRONG_TYPE);
         return NULL;
      }
      setLastError(SIMPLE_NO_ERROR);
      return pDv;
   }

   DataVariant* createDataVariantFromString(const char* pType, const char* pValue, int xml)
   {
      DataVariant* pDv = new DataVariant();
      if (pDv == NULL)
      {
         setLastError(SIMPLE_WRONG_TYPE);
         return NULL;
      }
      std::string type(pType == NULL ? "" : pType);
      std::string val(pValue == NULL ? "" : pValue);
      DataVariant::Status status;
      if (xml)
      {
         status = pDv->fromXmlString(type, val);
      }
      else
      {
         status = pDv->fromDisplayString(type, val);
      }
      if (status != DataVariant::SUCCESS)
      {
         setLastError(SIMPLE_WRONG_TYPE);
         return 0;
      }
      setLastError(SIMPLE_NO_ERROR);
      return pDv;
   }

   void freeDataVariant(DataVariant* pDv)
   {
      delete pDv;
   }

   int isDataVariantValid(DataVariant* pDv)
   {
      if (pDv == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      setLastError(SIMPLE_NO_ERROR);
      return pDv->isValid();
   }

   uint32_t getDataVariantTypeName(DataVariant* pDv, char* pType, uint32_t typeSize)
   {
      if (pDv == NULL || pType == NULL || !pDv->isValid())
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      std::string type = pDv->getTypeName();
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

   void* getDataVariantValue(DataVariant* pDv)
   {
      if (pDv == NULL || !pDv->isValid())
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return NULL;
      }
      setLastError(SIMPLE_NO_ERROR);
      return pDv->getPointerToValueAsVoid();
   }

   uint32_t getDataVariantValueString(DataVariant* pDv, int xml, char* pValue, uint32_t valueSize)
   {
      if (pDv == NULL || pValue == NULL || !pDv->isValid())
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      DataVariant::Status status;
      std::string val;
      if (xml)
      {
         val = pDv->toXmlString(&status);
      }
      else
      {
         val = pDv->toDisplayString(&status);
      }
      if (status == DataVariant::FAILURE)
      {
         setLastError(SIMPLE_OTHER_FAILURE);
         return 0;
      }
      else if (status == DataVariant::NOT_SUPPORTED)
      {
         setLastError(SIMPLE_WRONG_TYPE);
         return 0;
      }
      if (valueSize == 0)
      {
         return val.size() + 1;
      }
      else if (val.size() > valueSize - 1) // extra char for the null
      {
         setLastError(SIMPLE_BUFFER_SIZE);
         return 0;
      }
      memcpy(pValue, val.c_str(), val.size());
      pValue[val.size()] = '\0';
      setLastError(SIMPLE_NO_ERROR);
      return val.size();
   }

   uint32_t vectorToArray(void* pValue, const char* pType, void** pOutValue)
   {
      if (pValue == NULL || pType == NULL || pOutValue == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      std::string type(pType);
      if (type.substr(0, 6) != "vector")
      {
         setLastError(SIMPLE_WRONG_TYPE);
         return 0;
      }
      // not interested in the exact subtype, only the beginning of the
      // internal data pointer so std::vector<char> should be sufficient
      std::vector<char>* pCharval = reinterpret_cast<std::vector<char>*>(pValue);
      *pOutValue = &pCharval->front();
      setLastError(SIMPLE_NO_ERROR);
      return pCharval->size();
   }

   void freePlugInArgList(PlugInArgList* pPial)
   {
      if (pPial != NULL)
      {
         Service<PlugInManagerServices>()->destroyPlugInArgList(pPial);
      }
   }

   uint32_t getPlugInArgCount(PlugInArgList* pPial)
   {
      if (pPial == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return NULL;
      }
      setLastError(SIMPLE_NO_ERROR);
      return pPial->getCount();
   }

   PlugInArg* getPlugInArgByName(PlugInArgList* pPial, const char* pArgName)
   {
      if (pPial == NULL || pArgName == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return NULL;
      }
      std::string argName(pArgName == NULL ? "" : pArgName);
      PlugInArg* pArg(NULL);
      if (!pPial->getArg(argName, pArg))
      {
         setLastError(SIMPLE_NOT_FOUND);
         return NULL;
      }
      setLastError(SIMPLE_NO_ERROR);
      return pArg;
   }

   PlugInArg* getPlugInArgByIndex(PlugInArgList* pPial, uint32_t argNum)
   {
      if (pPial == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return NULL;
      }
      PlugInArg* pArg(NULL);
      if (!pPial->getArg(argNum, pArg))
      {
         setLastError(SIMPLE_NOT_FOUND);
         return NULL;
      }
      setLastError(SIMPLE_NO_ERROR);
      return pArg;
   }

   uint32_t getPlugInArgName(PlugInArg* pPia, char* pName, uint32_t nameSize)
   {
      if (pPia == NULL || pName == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      std::string name = pPia->getName();
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

   uint32_t getPlugInArgTypeName(PlugInArg* pPia, char* pType, uint32_t typeSize)
   {
      if (pPia == NULL || pType == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      std::string type = pPia->getType();
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

   uint32_t getPlugInArgDescription(PlugInArg* pPia, char* pDesc, uint32_t descSize)
   {
      if (pPia == NULL || pDesc == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      std::string desc = pPia->getDescription();
      if (descSize == 0)
      {
         return desc.size() + 1;
      }
      else if (desc.size() > descSize - 1) // extra char for the null
      {
         setLastError(SIMPLE_BUFFER_SIZE);
         return 0;
      }
      memcpy(pDesc, desc.c_str(), desc.size());
      pDesc[desc.size()] = '\0';
      setLastError(SIMPLE_NO_ERROR);
      return desc.size();
   }

   int isPlugInArgDefaultSet(PlugInArg* pPia)
   {
      if (pPia == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      setLastError(SIMPLE_NO_ERROR);
      return pPia->isDefaultSet();
   }

   int isPlugInArgActualSet(PlugInArg* pPia)
   {
      if (pPia == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      setLastError(SIMPLE_NO_ERROR);
      return pPia->isActualSet();
   }

   void* getPlugInArgDefaultValue(PlugInArg* pPia)
   {
      if (pPia == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return NULL;
      }
      if (!pPia->isDefaultSet())
      {
         setLastError(SIMPLE_NOT_FOUND);
         return NULL;
      }
      setLastError(SIMPLE_NO_ERROR);
      return pPia->getDefaultValue();
   }

   void* getPlugInArgActualValue(PlugInArg* pPia)
   {
      if (pPia == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return NULL;
      }
      if (!pPia->isActualSet())
      {
         setLastError(SIMPLE_NOT_FOUND);
         return NULL;
      }
      setLastError(SIMPLE_NO_ERROR);
      return pPia->getActualValue();
   }

   void* getPlugInArgValue(PlugInArg* pPia)
   {
      if (pPia == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return NULL;
      }
      if (pPia->isActualSet())
      {
         setLastError(SIMPLE_NO_ERROR);
         return pPia->getActualValue();
      }
      else if (pPia->isDefaultSet())
      {
         setLastError(SIMPLE_NO_ERROR);
         return pPia->getDefaultValue();
      }
      setLastError(SIMPLE_NOT_FOUND);
      return NULL;
   }

   void setPlugInArgDefaultValueFromVoid(PlugInArg* pPia, void* pValue)
   {
      if (pPia == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return;
      }
      pPia->setDefaultValue(pValue);
      if (!pPia->isDefaultSet())
      {
         setLastError(SIMPLE_OTHER_FAILURE);
         return;
      }
      setLastError(SIMPLE_NO_ERROR);
   }

   void setPlugInArgDefaultValueFromDataVariant(PlugInArg* pPia, DataVariant* pValue)
   {
      if (pPia == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return;
      }
      void* pVoidValue = NULL;
      if (pValue != NULL && pValue->isValid())
      {
         if (pValue->getTypeName() != pPia->getType())
         {
            setLastError(SIMPLE_WRONG_TYPE);
            return;
         }
         pVoidValue = pValue->getPointerToValueAsVoid();
      }
      pPia->setDefaultValue(pVoidValue);
      if (!pPia->isDefaultSet())
      {
         setLastError(SIMPLE_OTHER_FAILURE);
         return;
      }
      setLastError(SIMPLE_NO_ERROR);
   }

   void setPlugInArgActualValueFromVoid(PlugInArg* pPia, void* pValue)
   {
      if (pPia == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return;
      }
      pPia->setActualValue(pValue);
      if (!pPia->isActualSet())
      {
         setLastError(SIMPLE_OTHER_FAILURE);
         return;
      }
      setLastError(SIMPLE_NO_ERROR);
   }

   void setPlugInArgActualValueFromDataVariant(PlugInArg* pPia, DataVariant* pValue)
   {
      if (pPia == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return;
      }
      void* pVoidValue = NULL;
      if (pValue != NULL && pValue->isValid())
      {
         if (pValue->getTypeName() != pPia->getType())
         {
            setLastError(SIMPLE_WRONG_TYPE);
            return;
         }
         pVoidValue = pValue->getPointerToValueAsVoid();
      }
      pPia->setActualValue(pVoidValue);
      if (!pPia->isActualSet())
      {
         setLastError(SIMPLE_OTHER_FAILURE);
         return;
      }
      setLastError(SIMPLE_NO_ERROR);
   }
}