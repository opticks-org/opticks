/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "ConvertToBipPage.h"
#include "ConvertToBipPager.h"
#include "DataAccessorImpl.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"

#include <algorithm>
#include <memory>

ConvertToBipPager::ConvertToBipPager(RasterElement* pRaster) :
   mpRaster(pRaster),
   mBytesPerElement(0)
{
   if (mpRaster != NULL)
   {
      const RasterDataDescriptor* pDd = dynamic_cast<const RasterDataDescriptor*>(mpRaster->getDataDescriptor());
      if (pDd != NULL)
      {
         mBytesPerElement = pDd->getBytesPerElement();
      }
   }
}

ConvertToBipPager::~ConvertToBipPager()
{}

void ConvertToBipPager::releasePage(RasterPage* pPage)
{
   // Check that pPage is the correct type before deleting it.
   delete dynamic_cast<ConvertToBipPage*>(pPage);
}

int ConvertToBipPager::getSupportedRequestVersion() const
{
   return 1;
}

RasterPage *ConvertToBipPager::getPage(DataRequest* pOriginalRequest, DimensionDescriptor startRow,
   DimensionDescriptor startColumn, DimensionDescriptor startBand)
{
   VERIFY(pOriginalRequest != NULL);
   if (pOriginalRequest->getWritable())
   {
      return NULL;
   }

   VERIFY(mpRaster != NULL);
   const RasterDataDescriptor* pDd = dynamic_cast<const RasterDataDescriptor*>(mpRaster->getDataDescriptor());
   VERIFY(pDd != NULL);
   InterleaveFormatType interleave = pDd->getInterleaveFormat();
   VERIFY(interleave == BIL || interleave == BSQ);

   DimensionDescriptor stopRow = pOriginalRequest->getStopRow();
   DimensionDescriptor stopColumn = pOriginalRequest->getStopColumn();
   DimensionDescriptor stopBand = pOriginalRequest->getStopBand();

   unsigned int numRows = pDd->getRowCount();
   unsigned int numCols = pDd->getColumnCount();
   unsigned int numBands = pDd->getBandCount();
   const std::vector<DimensionDescriptor>& bandVec = pDd->getBands();

   if (startRow.getActiveNumber() >= numRows || stopRow.getActiveNumber() >= numRows ||
      startColumn.getActiveNumber() >= numCols || stopColumn.getActiveNumber() >= numCols ||
      startBand.getActiveNumber() >= numBands || stopBand.getActiveNumber() >= numBands)
   {
      return NULL;
   }

   std::vector<DimensionDescriptor>::const_iterator iter = std::find(bandVec.begin(), bandVec.end(), startBand);
   if (iter == bandVec.end())
   {
      return NULL;
   }

   std::vector<DimensionDescriptor>::const_iterator stopIter = std::find(iter, bandVec.end(), stopBand);
   if (stopIter == bandVec.end())
   {
      return NULL;
   }

   unsigned int cols = stopColumn.getActiveNumber() - startColumn.getActiveNumber() + 1;
   unsigned int rows = std::min(pOriginalRequest->getConcurrentRows(),
      stopRow.getActiveNumber() - startRow.getActiveNumber() + 1);
   unsigned int bands = stopBand.getActiveNumber() - startBand.getActiveNumber() + 1;
   std::auto_ptr<ConvertToBipPage> pPage(new ConvertToBipPage(rows, cols, bands, mBytesPerElement));
   unsigned char* pDst = reinterpret_cast<unsigned char*>(pPage->getRawData());
   if (pDst == NULL)
   {
      return NULL;
   }

   if (interleave == BSQ)
   {
      for (unsigned int band = 0; iter <= stopIter; ++iter, ++band)
      {
         FactoryResource<DataRequest> pRequest;
         pRequest->setRows(startRow, stopRow, 1);
         pRequest->setColumns(startColumn, stopColumn, cols);
         pRequest->setBands(*iter, *iter, 1);

         DataAccessor da = mpRaster->getDataAccessor(pRequest.release());
         for (unsigned int row = 0; row < rows; ++row)
         {
            if (da.isValid() == false)
            {
               return NULL;
            }

            unsigned int cachePos = (row * cols * bands + band) * mBytesPerElement;
            unsigned char* pSrc = reinterpret_cast<unsigned char*>(da->getRow());
            for (unsigned int col = 0; col < cols; ++col)
            {
               memcpy(pDst + cachePos, pSrc, mBytesPerElement);
               cachePos += bands * mBytesPerElement;
               pSrc += mBytesPerElement;
            }

            da->nextRow();
         }
      }
   }
   else if (interleave == BIL)
   {
      // Optimize for CachedPager subclasses by iterating row, band, column instead of band, row, column.
      FactoryResource<DataRequest> pRequest;
      pRequest->setRows(startRow, stopRow, 1);
      pRequest->setColumns(startColumn, stopColumn, cols);
      pRequest->setBands(*iter, DimensionDescriptor());

      DataAccessor da = mpRaster->getDataAccessor(pRequest.release());
      for (unsigned int row = 0; row < rows; ++row)
      {
         if (da.isValid() == false)
         {
            return NULL;
         }

         for (unsigned int band = 0; band < bands; ++band)
         {
            unsigned char* pSrc = reinterpret_cast<unsigned char*>(da->getRow()) +
               (band * da->getConcurrentColumns() * mBytesPerElement);
            unsigned int cachePos = (row * cols * bands + band) * mBytesPerElement;
            for (unsigned int col = 0; col < cols; ++col)
            {
               memcpy(pDst + cachePos, pSrc, mBytesPerElement);
               cachePos += bands * mBytesPerElement;
               pSrc += mBytesPerElement;
            }
         }

         da->nextRow();
      }
   }

   return pPage.release();
}
