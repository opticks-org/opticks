/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ConvertToBipPager.h"
#include "ConvertToBipPage.h"
#include "AppVerify.h"
#include "RasterElement.h"
#include "DataAccessorImpl.h"
#include "RasterDataDescriptor.h"

ConvertToBipPager::ConvertToBipPager(RasterElement *pRaster): 
   mpRaster(pRaster), mBytesPerElement(0)
{
   if (mpRaster != NULL)
   {
      const RasterDataDescriptor* pDd =
         dynamic_cast<const RasterDataDescriptor*>(mpRaster->getDataDescriptor());
      if (pDd != NULL)
      {
         mBytesPerElement = pDd->getBytesPerElement();
      }
   }
}

ConvertToBipPager::~ConvertToBipPager(void)
{
}

RasterPage *ConvertToBipPager::getPage(DataRequest *pOriginalRequest, 
      DimensionDescriptor startRow,
      DimensionDescriptor startColumn,
      DimensionDescriptor startBand)
{
   VERIFY(pOriginalRequest != NULL);
   VERIFY(mpRaster != NULL);

   if (pOriginalRequest->getWritable())
   {
      return NULL;
   }

   const RasterDataDescriptor *pDd = dynamic_cast<const RasterDataDescriptor*>(mpRaster->getDataDescriptor());
   VERIFY(pDd != NULL);
   InterleaveFormatType interleave = pDd->getInterleaveFormat();
   VERIFY(interleave == BIL || interleave == BSQ);

   DimensionDescriptor stopRow = pOriginalRequest->getStopRow();
   DimensionDescriptor stopColumn = pOriginalRequest->getStopColumn();
   DimensionDescriptor stopBand = pOriginalRequest->getStopBand();

   unsigned int numRows = pDd->getRowCount();
   unsigned int numCols = pDd->getColumnCount();
   unsigned int numBands = pDd->getBandCount();
   const std::vector<DimensionDescriptor> &bandVec = pDd->getBands();

   if (startRow.getActiveNumber() >= numRows || stopRow.getActiveNumber() >= numRows ||
      startColumn.getActiveNumber() >= numCols || stopColumn.getActiveNumber() >= numCols ||
      startBand.getActiveNumber() >= numBands || stopBand.getActiveNumber() >= numBands)
   {
      return NULL;
   }

   unsigned int cols = stopColumn.getActiveNumber()-startColumn.getActiveNumber()+1;
   unsigned int rows = pOriginalRequest->getConcurrentRows();
   unsigned int bands = stopBand.getActiveNumber()-startBand.getActiveNumber()+1;
   ConvertToBipPage *pPage = new ConvertToBipPage(rows,
      cols, bands, mBytesPerElement);

   unsigned int i = 0;
   for (std::vector<DimensionDescriptor>::const_iterator iter = bandVec.begin();
      iter != bandVec.end(); ++iter, ++i)
   {
      FactoryResource<DataRequest> pRequest;
      pRequest->setRows(startRow, stopRow, 1);
      pRequest->setColumns(startColumn, stopColumn, cols);
      pRequest->setBands(*iter, *iter, 1);

      DataAccessor da = mpRaster->getDataAccessor(pRequest.release());
      for (unsigned int j = 0; j < rows; ++j)
      {

         if (da.isValid())
         {
            pPage->feed(j, i, da->getRow());
         }
         else
         {
            delete pPage;
            return NULL;
         }
         da->nextRow();
      }
   }
   return pPage;
}

void ConvertToBipPager::releasePage(RasterPage *pPage)
{
   ConvertToBipPage *pConvertPage = 
      dynamic_cast<ConvertToBipPage*>(pPage);
   if (pConvertPage != NULL)
   {
      delete pConvertPage;
   }
}

int ConvertToBipPager::getSupportedRequestVersion() const
{
   return 1;
}
