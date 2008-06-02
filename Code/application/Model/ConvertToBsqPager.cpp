/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ConvertToBsqPager.h"
#include "ConvertToBsqPage.h"
#include "AppVerify.h"
#include "DataAccessorImpl.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"

#include <limits>

ConvertToBsqPager::ConvertToBsqPager(RasterElement *pRaster): 
   mpRaster(pRaster), mBytesPerElement(0), mSkipBytes(std::numeric_limits<unsigned int>::max())
{
   if (mpRaster != NULL)
   {
      const RasterDataDescriptor* pDd =
         dynamic_cast<const RasterDataDescriptor*>(mpRaster->getDataDescriptor());
      if (pDd != NULL)
      {
         mBytesPerElement = pDd->getBytesPerElement();
         InterleaveFormatType interleave = pDd->getInterleaveFormat();
         if (interleave == BIP)
         {
            mSkipBytes = mBytesPerElement * (pDd->getBandCount());
         }
         else if (interleave == BIL)
         {
            mSkipBytes = 0;
         }
      }
   }
}

ConvertToBsqPager::~ConvertToBsqPager(void)
{
}

RasterPage *ConvertToBsqPager::getPage(DataRequest *pOriginalRequest, 
      DimensionDescriptor startRow,
      DimensionDescriptor startColumn,
      DimensionDescriptor startBand)
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

   unsigned int concurrentRows = pOriginalRequest->getConcurrentRows();
   unsigned int concurrentBands = pOriginalRequest->getConcurrentBands();

   VERIFY(requestedType == BSQ);
   VERIFY(startBand == stopBand && concurrentBands == 1);

   VERIFY(mpRaster != NULL);
   const RasterDataDescriptor *pDd = dynamic_cast<const RasterDataDescriptor*>(mpRaster->getDataDescriptor());
   VERIFY(pDd != NULL);
   InterleaveFormatType interleave = pDd->getInterleaveFormat();

   VERIFY(interleave == BIL || interleave == BIP);
   VERIFY(mSkipBytes != std::numeric_limits<unsigned int>::max());

   unsigned int numRows = pDd->getRowCount();
   unsigned int numCols = pDd->getColumnCount();
   unsigned int numBands = pDd->getBandCount();

   if (startRow.getActiveNumber() >= numRows || stopRow.getActiveNumber() >= numRows ||
      startColumn.getActiveNumber() >= numCols || stopColumn.getActiveNumber() >= numCols ||
      startBand.getActiveNumber() >= numBands || stopBand.getActiveNumber() >= numBands)
   {
      return NULL;
   }

   unsigned int cols = stopColumn.getActiveNumber()-startColumn.getActiveNumber()+1;
   ConvertToBsqPage *pPage = new ConvertToBsqPage(concurrentRows,
      cols, mBytesPerElement);


   FactoryResource<DataRequest> pRequest;
   pRequest->setRows(startRow, stopRow, concurrentRows);
   pRequest->setColumns(startColumn, stopColumn, cols);
   pRequest->setBands(startBand, startBand, 1);

   DataAccessor da = mpRaster->getDataAccessor(pRequest.release());

   for (unsigned int j = 0; j < concurrentRows; ++j)
   {
      if (da.isValid())
      {
         pPage->feed(j, mSkipBytes, da->getRow());
      }
      else
      {
         delete pPage;
         return NULL;
      }
      da->nextRow();
   }
   return pPage;
}

void ConvertToBsqPager::releasePage(RasterPage *pPage)
{
   ConvertToBsqPage *pConvertPage = dynamic_cast<ConvertToBsqPage*>(pPage);
   if (pConvertPage != NULL)
   {
      delete pConvertPage;
   }
}

int ConvertToBsqPager::getSupportedRequestVersion() const
{
   return 1;
}
