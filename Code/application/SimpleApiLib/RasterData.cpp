/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ApiUtilities.h"
#include "DataAccessor.h"
#include "DataAccessorImpl.h"
#include "DataElement.h"
#include "DataRequest.h"
#include "ModelServices.h"
#include "ObjectResource.h"
#include "RasterData.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterUtilities.h"
#include "SimpleApiErrors.h"
#include "Statistics.h"
#include "switchOnEncoding.h"
#include "TypeConverter.h"

#include <vector>

namespace
{
   template<typename T>
   void copySubcube(T* pData, RasterElement* pElement,
                    unsigned int rowStart, unsigned int rowEnd,
                    unsigned int columnStart, unsigned int columnEnd,
                    unsigned int bandStart, unsigned int bandEnd,
                    bool push, bool& success)
   {
      unsigned int rows = rowEnd - rowStart + 1;
      unsigned int columns = columnEnd - columnStart + 1;
      unsigned int bands = bandEnd - bandStart + 1;
      RasterDataDescriptor* pDesc = static_cast<RasterDataDescriptor*>(pElement->getDataDescriptor());
      unsigned int bytesPerElement = pDesc->getBytesPerElement();
      InterleaveFormatType interleave = pDesc->getInterleaveFormat();
      switch (interleave)
      {
      case BSQ:
         {
            for (unsigned int band = 0; band < bands; ++band)
            {
               FactoryResource<DataRequest> pRequest;
               if (push)
               {
                  pRequest->setWritable(true);
               }
               pRequest->setInterleaveFormat(BSQ);
               pRequest->setRows(pDesc->getActiveRow(rowStart),
                  pDesc->getActiveRow(rowEnd), 1);
               pRequest->setColumns(pDesc->getActiveColumn(columnStart),
                  pDesc->getActiveColumn(columnEnd), columns);
               pRequest->setBands(pDesc->getActiveBand(bandStart + band),
                  pDesc->getActiveBand(bandStart + band), 1);
               DataAccessor daImage = pElement->getDataAccessor(pRequest.release());
               if (!daImage.isValid())
               {
                  success = false;
                  return;
               }
               for (unsigned int row = 0; row < rows; ++row)
               {
                  for (unsigned int col = 0; col < columns; ++col)
                  {
                     daImage->toPixel(rowStart + row, columnStart + col);
                     if (!daImage.isValid())
                     {
                        success = false;
                        return;
                     }
                     if (push)
                     {
                        memcpy(daImage->getColumn(),
                           pData + (columns * rows * band) + (columns * row) + col, bytesPerElement);
                     }
                     else
                     {
                        memcpy(pData + (columns * rows * band) + (columns * row) + col,
                           daImage->getColumn(), bytesPerElement);
                     }
                  }
               }
            }
            break;
         }
      case BIP:
         {
            FactoryResource<DataRequest> pRequest;
            if (push)
            {
               pRequest->setWritable(true);
            }
            pRequest->setInterleaveFormat(BIP);
            pRequest->setRows(pDesc->getActiveRow(rowStart),
               pDesc->getActiveRow(rowEnd), 1);
            pRequest->setColumns(pDesc->getActiveColumn(columnStart),
               pDesc->getActiveColumn(columnEnd), columns);
            DataAccessor daImage = pElement->getDataAccessor(pRequest.release());
            if (!daImage.isValid())
            {
               success = false;
               return;
            }
            for (unsigned int row = 0; row < rows; ++row)
            {
               for (unsigned int col = 0; col < columns; ++col)
               {
                  daImage->toPixel(rowStart + row, columnStart + col);
                  if (!daImage.isValid())
                  {
                     success = false;
                     return;
                  }
                  for (unsigned int band = 0; band < bands; ++band)
                  {
                     if (push)
                     {
                        memcpy(static_cast<T*>(daImage->getColumn()) + (band + bandStart),
                           pData + (row * columns * bands) + (col * bands) + band, bytesPerElement);
                     }
                     else
                     {
                        memcpy(pData + (row * columns * bands) + (col * bands) + band,
                           static_cast<T*>(daImage->getColumn()) + (band + bandStart), bytesPerElement);
                     }
                  }
               }
            }
            break;
         }
      case BIL:
         {
            for (unsigned int row = 0; row < rows; ++row)
            {
               for (unsigned int band = 0; band < bands; ++band)
               {
                  FactoryResource<DataRequest> pRequest;
                  if (push)
                  {
                     pRequest->setWritable(true);
                  }
                  pRequest->setInterleaveFormat(BIL);
                  pRequest->setRows(pDesc->getActiveRow(rowStart + row),
                     pDesc->getActiveRow(rowStart + row), 1);
                  pRequest->setColumns(pDesc->getActiveColumn(columnStart),
                     pDesc->getActiveColumn(columnEnd), columns);
                  pRequest->setBands(pDesc->getActiveBand(bandStart + band),
                     pDesc->getActiveBand(bandStart + band), 1);
                  DataAccessor daImage = pElement->getDataAccessor(pRequest.release());
                  if (!daImage.isValid())
                  {
                     success = false;
                     return;
                  }
                  for (unsigned int col = 0; col < columns; ++col)
                  {
                     daImage->toPixel(rowStart + row, columnStart + col);
                     if (!daImage.isValid())
                     {
                        success = false;
                        return;
                     }
                     if (push)
                     {
                        memcpy(daImage->getColumn(),
                           pData + (row * columns * bands) + (band * columns) + col, bytesPerElement);
                     }
                     else
                     {
                        memcpy(pData + (row * columns * bands) + (band * columns) + col,
                           daImage->getColumn(), bytesPerElement);
                     }
                  }
               }
            }
            break;
         }
      }
   }
}

extern "C"
{

   DataInfo* createDataInfo(DataElement* pElement)
   {
      RasterElement* pRasterElement = dynamic_cast<RasterElement*>(pElement);
      if (pRasterElement == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return NULL;
      }

      RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(pRasterElement->getDataDescriptor());
      if (pDescriptor == NULL)
      {
         setLastError(SIMPLE_OTHER_FAILURE);
         return NULL;
      }

      DataInfo* pDataInfo = new DataInfo;
      pDataInfo->numRows = pDescriptor->getRowCount();
      pDataInfo->numColumns = pDescriptor->getColumnCount();
      pDataInfo->numBands = pDescriptor->getBandCount();
      pDataInfo->encodingType = static_cast<uint32_t>(pDescriptor->getDataType());
      pDataInfo->encodingTypeSize = RasterUtilities::bytesInEncoding(pDescriptor->getDataType());
      pDataInfo->interleaveFormat = static_cast<uint32_t>(pDescriptor->getInterleaveFormat());
      const std::vector<int>& badValues = pDescriptor->getBadValues();
      pDataInfo->numBadValues = badValues.size();
      if (pDataInfo->numBadValues == 0)
      {
         pDataInfo->pBadValues = NULL;
      }
      else
      {
         pDataInfo->pBadValues = new int32_t[pDataInfo->numBadValues];
         memcpy(pDataInfo->pBadValues, &badValues[0], pDataInfo->numBadValues * sizeof(int32_t));
      }

      setLastError(SIMPLE_NO_ERROR);
      return pDataInfo;
   }

   void destroyDataInfo(DataInfo* pDataInfo)
   {
      if (pDataInfo != NULL)
      {
         delete[] pDataInfo->pBadValues;
         delete pDataInfo;
      }
   }

   DataElement* createRasterElement(const char* pName, RasterElementArgs args)
   {
      if (pName == NULL || args.location > 2)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return NULL;
      }

      // Check for an existing element with the name
      if (getDataElement(pName, TypeConverter::toString<RasterElement>(), 0) != NULL)
      {
         setLastError(SIMPLE_EXISTS);
         return NULL;
      }

      RasterElement* pElement = RasterUtilities::createRasterElement(std::string(pName), args.numRows,
         args.numColumns, args.numBands, static_cast<EncodingTypeEnum>(args.encodingType),
         static_cast<InterleaveFormatTypeEnum>(args.interleaveFormat), args.location != 2, args.pParent);
      if (pElement == NULL)
      {
         switch (args.location)
         {
         case 0:
            pElement = RasterUtilities::createRasterElement(std::string(pName), args.numRows,
               args.numColumns, args.numBands, static_cast<EncodingTypeEnum>(args.encodingType),
               static_cast<InterleaveFormatTypeEnum>(args.interleaveFormat), false, args.pParent);
            if (pElement == NULL)
            {
               setLastError(SIMPLE_OTHER_FAILURE);
               return NULL;
            }
            break;
         case 1:
            setLastError(SIMPLE_NO_MEM);
            return NULL;
         case 2:
            setLastError(SIMPLE_OTHER_FAILURE);
            return NULL;
         default:
            setLastError(SIMPLE_BAD_PARAMS);
            return NULL;
         }
      }
      if (args.pBadValues != NULL && args.numBadValues > 0)
      {
         RasterDataDescriptor* pDesc = static_cast<RasterDataDescriptor*>(pElement->getDataDescriptor());
         if (pDesc != NULL)
         {
            std::vector<int> badValues;
            badValues.reserve(args.numBadValues);
            for (uint32_t idx = 0; idx < args.numBadValues; ++idx)
            {
               badValues.push_back(args.pBadValues[idx]);
            }
            pDesc->setBadValues(badValues);

            // set on the statistics objects
            const std::vector<DimensionDescriptor>& allBands = pDesc->getBands();
            for (std::vector<DimensionDescriptor>::const_iterator band = allBands.begin();
                 band != allBands.end(); ++band)
            {
               if (band->isValid())
               {
                  Statistics* pStats = pElement->getStatistics(*band);
                  if (pStats != NULL)
                  {
                     pStats->setBadValues(badValues);
                  }
               }
            }
            pElement->updateData();
         }
      }

      setLastError(SIMPLE_NO_ERROR);
      return pElement;
   }

   void* createDataPointer(DataElement* pElement, DataPointerArgs* pArgs, int* pOwn)
   {
      RasterElement* pRaster = dynamic_cast<RasterElement*>(pElement);
      if (pRaster == NULL || pOwn == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return NULL;
      }
      void* pRawData = pRaster->getRawData();
      if (pArgs == NULL && pRawData != NULL)
      {
         *pOwn = 0;
         setLastError(SIMPLE_NO_ERROR);
         return pRawData;
      }
      *pOwn = 1;
      const RasterDataDescriptor* pDesc = static_cast<const RasterDataDescriptor*>(pRaster->getDataDescriptor());
      DataPointerArgs args = {
         0, pDesc->getRowCount() - 1,
         0, pDesc->getColumnCount() - 1,
         0, pDesc->getBandCount() - 1,
         0 };
      switch (pDesc->getInterleaveFormat())
      {
      case BSQ:
         args.interleaveFormat = 0;
         break;
      case BIP:
         args.interleaveFormat = 1;
         break;
      case BIL:
         args.interleaveFormat = 2;
         break;
      }
      if (pArgs == NULL)
      {
         pArgs = &args;
      }
      unsigned int rowCount = pArgs->rowEnd - pArgs->rowStart + 1;
      unsigned int columnCount = pArgs->columnEnd - pArgs->columnStart + 1;
      unsigned int bandCount = pArgs->bandEnd - pArgs->bandStart + 1;
      pRawData = new (std::nothrow) char[rowCount * columnCount * bandCount * pDesc->getBytesPerElement()];
      if (pRawData == NULL)
      {
         setLastError(SIMPLE_NO_MEM);
         return NULL;
      }
      bool success = true;
      switchOnComplexEncoding(pDesc->getDataType(), copySubcube, pRawData, pRaster,
         pArgs->rowStart, pArgs->rowEnd,
         pArgs->columnStart, pArgs->columnEnd,
         pArgs->bandStart, pArgs->bandEnd, false, success);
      if (!success)
      {
         delete[] pRawData;
         setLastError(SIMPLE_OTHER_FAILURE);
         return NULL;
      }
      setLastError(SIMPLE_NO_ERROR);
      return pRawData;
   }

   void destroyDataPointer(void* pData)
   {
      delete[] pData;
   }

   int copyDataToRasterElement(DataElement* pElement, DataPointerArgs* pArgs, void* pData)
   {
      RasterElement* pRaster = dynamic_cast<RasterElement*>(pElement);
      if (pRaster == NULL || pData == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 1;
      }
      const RasterDataDescriptor* pDesc = static_cast<const RasterDataDescriptor*>(pRaster->getDataDescriptor());
      void* pRawData = pRaster->getRawData();
      if (pArgs == NULL && pRawData != NULL)
      {
         size_t len = pDesc->getRowCount() * pDesc->getColumnCount() * pDesc->getBandCount()
                    * pDesc->getBytesPerElement();
         memcpy(pRawData, pData, len);
         setLastError(SIMPLE_NO_ERROR);
         return 0;
      }
      DataPointerArgs args = {
         0, pDesc->getRowCount() - 1,
         0, pDesc->getColumnCount() - 1,
         0, pDesc->getBandCount() - 1,
         0 };
      switch (pDesc->getInterleaveFormat())
      {
      case BSQ:
         args.interleaveFormat = 0;
         break;
      case BIP:
         args.interleaveFormat = 1;
         break;
      case BIL:
         args.interleaveFormat = 2;
         break;
      }
      if (pArgs == NULL)
      {
         pArgs = &args;
      }
      bool success = true;
      switchOnComplexEncoding(pDesc->getDataType(), copySubcube, pData, pRaster,
         pArgs->rowStart, pArgs->rowEnd,
         pArgs->columnStart, pArgs->columnEnd,
         pArgs->bandStart, pArgs->bandEnd, true, success);
      if (!success)
      {
         setLastError(SIMPLE_OTHER_FAILURE);
         return 1;
      }
      setLastError(SIMPLE_NO_ERROR);
      return 0;
   }

   void updateRasterElement(DataElement* pElement)
   {
      RasterElement* pRasterElement = dynamic_cast<RasterElement*>(pElement);
      if (pRasterElement == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return;
      }
      pRasterElement->updateData();
      setLastError(SIMPLE_NO_ERROR);
   }

   DataAccessorImpl* createDataAccessor(DataElement* pElement, DataAccessorArgs* pArgs)
   {
      RasterElement* pRasterElement = dynamic_cast<RasterElement*>(pElement);
      if (pRasterElement == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return NULL;
      }

      FactoryResource<DataRequest> pRequest;
      RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(pRasterElement->getDataDescriptor());
      if (pArgs != NULL)
      {
         pRequest->setRows(pDescriptor->getActiveRow(pArgs->rowStart),
            pDescriptor->getActiveRow(pArgs->rowEnd),pArgs->concurrentRows);
         pRequest->setColumns(pDescriptor->getActiveColumn(pArgs->columnStart),
            pDescriptor->getActiveColumn(pArgs->columnEnd),pArgs->concurrentColumns);
         pRequest->setBands(pDescriptor->getActiveBand(pArgs->bandStart),
            pDescriptor->getActiveBand(pArgs->bandEnd),pArgs->concurrentBands);
         pRequest->setInterleaveFormat(static_cast<InterleaveFormatTypeEnum>(pArgs->interleaveFormat));
         pRequest->setWritable(pArgs->writable != 0);
      }

      DataAccessor dataAccessor(pRasterElement->getDataAccessor(pRequest.release()));
      if (!dataAccessor.isValid())
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return NULL;
      }
      DataAccessorImpl* pRval = dataAccessor.operator->();
      pRval->incrementRefCount();

      setLastError(SIMPLE_NO_ERROR);
      return pRval;
   }

   void destroyDataAccessor(DataAccessorImpl* pAccessor)
   {
      if (pAccessor != NULL && pAccessor->decrementRefCount() == 0)
      {
         delete pAccessor;
      }
   }

   int isDataAccessorValid(DataAccessorImpl* pAccessor)
   {
      if (pAccessor == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }

      setLastError(SIMPLE_NO_ERROR);
      return pAccessor->isValid();
   }

   void* getDataAccessorRow(DataAccessorImpl* pAccessor)
   {
      if (pAccessor == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
      }

      setLastError(SIMPLE_NO_ERROR);
      return pAccessor->getRow();
   }

   void nextDataAccessorRow(DataAccessorImpl* pAccessor, uint32_t count, int resetColumn)
   {
      if (pAccessor == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return;
      }

      pAccessor->nextRow(count, resetColumn != 0);
      setLastError(SIMPLE_NO_ERROR);
   }

   void* getDataAccessorColumn(DataAccessorImpl* pAccessor)
   {
      if (pAccessor == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return NULL;
      }

      setLastError(SIMPLE_NO_ERROR);
      return pAccessor->getColumn();
   }

   void nextDataAccessorColumn(DataAccessorImpl* pAccessor, uint32_t count)
   {
      if (pAccessor == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return;
      }

      pAccessor->nextColumn(count);
      setLastError(SIMPLE_NO_ERROR);
   }

   void toDataAccessorPixel(DataAccessorImpl* pAccessor, uint32_t row, uint32_t column)
   {
      if (pAccessor == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return;
      }

      pAccessor->toPixel(row, column);
      setLastError(SIMPLE_NO_ERROR);
   }

   RasterElement* getDataAccessorRasterElement(DataAccessorImpl* pAccessor)
   {
      if (pAccessor == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return NULL;
      }

      setLastError(SIMPLE_NO_ERROR);
      return pAccessor->getAssociatedRasterElement();
   }

   uint32_t getDataAccessorRowSize(DataAccessorImpl* pAccessor)
   {
      if (pAccessor == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }

      setLastError(SIMPLE_NO_ERROR);
      return pAccessor->getRowSize();
   }
};
