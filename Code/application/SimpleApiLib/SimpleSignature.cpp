/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "Signature.h"
#include "SignatureSet.h"
#include "SimpleApiErrors.h"
#include "SimpleSignature.h"

extern "C"
{
   uint32_t getSignatureDataSetCount(DataElement* pSig)
   {
      Signature* pSignature = dynamic_cast<Signature*>(pSig);
      if (pSignature == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      setLastError(SIMPLE_NO_ERROR);
      return pSignature->getDataNames().size();
   }

   uint32_t getSignatureDataSetName(DataElement* pSig, uint32_t index, char* pName, uint32_t nameSize)
   {
      Signature* pSignature = dynamic_cast<Signature*>(pSig);
      if (pSignature == NULL || pName == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      std::string name;
      std::set<std::string> names = pSignature->getDataNames();
      for (std::set<std::string>::const_iterator nameIter = names.begin(); nameIter != names.end(); ++nameIter)
      {
         if (index == 0)
         {
            name = *nameIter;
            break;
         }
         --index;
      }
      if (name.empty())
      {
         setLastError(SIMPLE_NOT_FOUND);
         return 0;
      }
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

   DataVariant* getSignatureDataSet(DataElement* pSig, const char* pName)
   {
      Signature* pSignature = dynamic_cast<Signature*>(pSig);
      if (pSignature == NULL || pName == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return NULL;
      }
      const DataVariant& res = pSignature->getData(std::string(pName));
      if (!res.isValid())
      {
         setLastError(SIMPLE_NOT_FOUND);
         return NULL;
      }
      setLastError(SIMPLE_NO_ERROR);
      return &const_cast<DataVariant&>(res);
   }

   int setSignatureDataSet(DataElement* pSig, const char* pName, DataVariant* pValue)
   {
      Signature* pSignature = dynamic_cast<Signature*>(pSig);
      if (pSignature == NULL || pName == NULL || pValue == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 1;
      }
      pSignature->adoptData(std::string(pName), *pValue);
      setLastError(SIMPLE_NO_ERROR);
      return 0;
   }

   uint32_t getSignatureSetCount(DataElement* pSet)
   {
      SignatureSet* pSignatureSet = dynamic_cast<SignatureSet*>(pSet);
      if (pSignatureSet == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      setLastError(SIMPLE_NO_ERROR);
      return pSignatureSet->getNumSignatures();
   }

   DataElement* getSignatureSetSignature(DataElement* pSet, uint32_t index)
   {
      SignatureSet* pSignatureSet = dynamic_cast<SignatureSet*>(pSet);
      if (pSignatureSet == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return NULL;
      }
      if (index >= pSignatureSet->getNumSignatures())
      {
         setLastError(SIMPLE_NOT_FOUND);
         return NULL;
      }
      setLastError(SIMPLE_NO_ERROR);
      return dynamic_cast<DataElement*>(pSignatureSet->getSignatures()[index]);
   }
}