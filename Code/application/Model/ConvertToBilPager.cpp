/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "ConvertToBilPage.h"
#include "ConvertToBilPager.h"
#include "DataAccessorImpl.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"

#include <memory>

ConvertToBilPager::ConvertToBilPager(RasterElement* pRaster) :
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

ConvertToBilPager::~ConvertToBilPager()
{}

void ConvertToBilPager::releasePage(RasterPage* pPage)
{
   // Check that pPage is the correct type before deleting it.
   delete dynamic_cast<ConvertToBilPage*>(pPage);
}

int ConvertToBilPager::getSupportedRequestVersion() const
{
   return 1;
}

RasterPage* ConvertToBilPager::getPage(DataRequest* pOriginalRequest, DimensionDescriptor startRow,
   DimensionDescriptor startColumn, DimensionDescriptor startBand)
{
   VERIFY(pOriginalRequest != NULL);
   if (pOriginalRequest->getWritable())
   {
      return NULL;
   }

   DimensionDescriptor stopRow = pOriginalRequest->getStopRow();
   DimensionDescriptor stopColumn = pOriginalRequest->getStopColumn();
   DimensionDescriptor stopBand = pOriginalRequest->getStopBand();
   VERIFY(pOriginalRequest->getInterleaveFormat() == BIL);

   VERIFY(mpRaster != NULL);
   const RasterDataDescriptor* pDd = dynamic_cast<const RasterDataDescriptor*>(mpRaster->getDataDescriptor());
   VERIFY(pDd != NULL);
   InterleaveFormatType interleave = pDd->getInterleaveFormat();
   VERIFY(interleave == BIP || interleave == BSQ);

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
   std::auto_ptr<ConvertToBilPage> pPage(new ConvertToBilPage(rows, cols, bands, mBytesPerElement));
   if (pPage->getRawData() == NULL)
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
         pRequest->setBands(*iter, DimensionDescriptor());

         DataAccessor da = mpRaster->getDataAccessor(pRequest.release());
         unsigned char* pDst = reinterpret_cast<unsigned char*>(pPage->getRawData()) + (band * cols * mBytesPerElement);
         for (unsigned int row = 0; row < rows; ++row)
         {
            if (da.isValid() == false)
            {
               return NULL;
            }

            memcpy(pDst, da->getRow(), mBytesPerElement * cols);
            pDst += bands * cols * mBytesPerElement;
            da->nextRow();
         }
      }
   }
   else if (interleave == BIP)
   {
      // Optimize for CachedPager subclasses by iterating row, column, band instead of band, row, column.
      FactoryResource<DataRequest> pRequest;
      pRequest->setRows(startRow, stopRow, 1);
      pRequest->setColumns(startColumn, stopColumn, cols);
      pRequest->setBands(*iter, DimensionDescriptor());

      DataAccessor da = mpRaster->getDataAccessor(pRequest.release());
      for (unsigned int row = 0; row < rows; ++row)
      {
         for (unsigned int col = 0; col < cols; ++col)
         {
            if (da.isValid() == false)
            {
               return NULL;
            }

            unsigned char* pDst = reinterpret_cast<unsigned char*>(pPage->getRawData()) +
               (row * cols * bands + col) * mBytesPerElement;
            unsigned char* pSrc = reinterpret_cast<unsigned char*>(da->getColumn());
            for (unsigned int band = 0; band < bands; ++band)
            {
               memcpy(pDst, pSrc, mBytesPerElement);
               pDst += cols * mBytesPerElement;
               pSrc += mBytesPerElement;
            }

            da->nextColumn();
         }

         da->nextRow();
      }
   }

   return pPage.release();
}
