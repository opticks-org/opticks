/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "ConvertToBsqPage.h"
#include "ConvertToBsqPager.h"
#include "DataAccessorImpl.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"

#include <limits>
#include <memory>

ConvertToBsqPager::ConvertToBsqPager(RasterElement* pRaster) :
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

ConvertToBsqPager::~ConvertToBsqPager()
{}

void ConvertToBsqPager::releasePage(RasterPage* pPage)
{
   // Check that pPage is the correct type before deleting it.
   delete dynamic_cast<ConvertToBsqPage*>(pPage);
}

int ConvertToBsqPager::getSupportedRequestVersion() const
{
   return 1;
}

RasterPage* ConvertToBsqPager::getPage(DataRequest* pOriginalRequest, DimensionDescriptor startRow,
   DimensionDescriptor startColumn, DimensionDescriptor startBand)
{
   VERIFYRV(pOriginalRequest != NULL, NULL);
   if (pOriginalRequest->getWritable())
   {
      return NULL;
   }

   InterleaveFormatType requestedType = pOriginalRequest->getInterleaveFormat();
   DimensionDescriptor stopRow = pOriginalRequest->getStopRow();
   DimensionDescriptor stopColumn = pOriginalRequest->getStopColumn();
   DimensionDescriptor stopBand = pOriginalRequest->getStopBand();

   unsigned int concurrentRows = std::min(pOriginalRequest->getConcurrentRows(),
      stopRow.getActiveNumber() - startRow.getActiveNumber() + 1);
   unsigned int concurrentBands = pOriginalRequest->getConcurrentBands();

   VERIFY(requestedType == BSQ);
   VERIFY(startBand == stopBand && concurrentBands == 1);

   VERIFY(mpRaster != NULL);
   const RasterDataDescriptor* pDd = dynamic_cast<const RasterDataDescriptor*>(mpRaster->getDataDescriptor());
   VERIFY(pDd != NULL);
   InterleaveFormatType interleave = pDd->getInterleaveFormat();

   VERIFY(interleave == BIL || interleave == BIP);

   unsigned int numRows = pDd->getRowCount();
   unsigned int numCols = pDd->getColumnCount();
   unsigned int numBands = pDd->getBandCount();

   if (startRow.getActiveNumber() >= numRows || stopRow.getActiveNumber() >= numRows ||
      startColumn.getActiveNumber() >= numCols || stopColumn.getActiveNumber() >= numCols ||
      startBand.getActiveNumber() >= numBands || stopBand.getActiveNumber() >= numBands)
   {
      return NULL;
   }

   unsigned int cols = stopColumn.getActiveNumber() - startColumn.getActiveNumber() + 1;
   std::auto_ptr<ConvertToBsqPage> pPage(new ConvertToBsqPage(concurrentRows, cols, mBytesPerElement));
   unsigned char* pDst = reinterpret_cast<unsigned char*>(pPage->getRawData());
   if (pDst == NULL)
   {
      return NULL;
   }

   FactoryResource<DataRequest> pRequest;
   pRequest->setRows(startRow, stopRow);
   pRequest->setColumns(startColumn, stopColumn, cols);
   pRequest->setBands(startBand, startBand, 1);
   DataAccessor da = mpRaster->getDataAccessor(pRequest.release());

   if (interleave == BIP)
   {
      for (unsigned int row = 0; row < concurrentRows; ++row)
      {
         for (unsigned int col = 0; col < cols; ++col)
         {
            if (da.isValid() == false)
            {
               return NULL;
            }

            memcpy(pDst, da->getColumn(), mBytesPerElement);
            pDst += mBytesPerElement;
            da->nextColumn();
         }

         da->nextRow();
      }
   }
   else if (interleave == BIL)
   {
      for (unsigned int row = 0; row < concurrentRows; ++row)
      {
         if (da.isValid() == false)
         {
            return NULL;
         }

         memcpy(pDst, da->getRow(), mBytesPerElement * cols);
         pDst += mBytesPerElement * cols;
         da->nextRow();
      }
   }

   return pPage.release();
}
