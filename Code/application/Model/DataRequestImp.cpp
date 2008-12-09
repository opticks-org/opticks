/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */


#include "AppVerify.h"
#include "DataRequestImp.h"
#include "RasterDataDescriptor.h"
#include "RasterFileDescriptor.h"

DataRequestImp::DataRequestImp() :
   mInterleaveDefault(true),
   mConcurrentRows(0),
   mConcurrentColumns(0),
   mConcurrentBands(0),
   mbWritable(false)
{
}

DataRequestImp::~DataRequestImp()
{
}

DataRequestImp::DataRequestImp(const DataRequestImp &rhs) :
   mInterleave(rhs.mInterleave),
   mInterleaveDefault(rhs.mInterleaveDefault),
   mStartRow(rhs.mStartRow),
   mStopRow(rhs.mStopRow),
   mConcurrentRows(rhs.mConcurrentRows),
   mStartColumn(rhs.mStartColumn),
   mStopColumn(rhs.mStopColumn),
   mConcurrentColumns(rhs.mConcurrentColumns),
   mStartBand(rhs.mStartBand),
   mStopBand(rhs.mStopBand),
   mConcurrentBands(rhs.mConcurrentBands),
   mbWritable(rhs.mbWritable)
{
}

DataRequest *DataRequestImp::copy() const
{
   return new DataRequestImp(*this);
}

bool DataRequestImp::validate(const RasterDataDescriptor *pDescriptor) const
{
   if (pDescriptor == NULL)
   {
      return false;
   }
   unsigned int numRows = pDescriptor->getRowCount();
   unsigned int numColumns = pDescriptor->getColumnCount();
   unsigned int numBands = pDescriptor->getBandCount();
   unsigned int bytesPerElement = pDescriptor->getBytesPerElement();
   unsigned int postLineBytes = 0;

   DimensionDescriptor startRow = getStartRow();
   DimensionDescriptor stopRow = getStopRow();
   unsigned int concurrentRows = getConcurrentRows();
   DimensionDescriptor startColumn = getStartColumn();
   DimensionDescriptor stopColumn = getStopColumn();
   unsigned int concurrentColumns = getConcurrentColumns();
   DimensionDescriptor startBand = getStartBand();
   DimensionDescriptor stopBand = getStopBand();
   unsigned int concurrentBands = getConcurrentBands();

   if (!startRow.isActiveNumberValid() || !stopRow.isActiveNumberValid() ||
      !startColumn.isActiveNumberValid() || !stopColumn.isActiveNumberValid() ||
      !startBand.isActiveNumberValid() || !stopBand.isActiveNumberValid())
   {
      return false;
   }

   //validate all the parameters before continuing
   if (startRow.getActiveNumber() >= numRows || 
      startRow.getActiveNumber() > stopRow.getActiveNumber() ||
      stopRow.getActiveNumber() >= numRows ||
      startColumn.getActiveNumber() >= numColumns ||
      startColumn.getActiveNumber() > stopColumn.getActiveNumber() ||
      stopColumn.getActiveNumber() >= numColumns ||
      startBand.getActiveNumber() >= numBands ||
      startBand.getActiveNumber() > stopBand.getActiveNumber() ||
      stopBand.getActiveNumber() >= numBands ||
      concurrentRows > stopRow.getActiveNumber()-startRow.getActiveNumber()+1 ||
      concurrentColumns > stopColumn.getActiveNumber()-startColumn.getActiveNumber()+1 ||
      concurrentBands > stopBand.getActiveNumber()-startBand.getActiveNumber()+1)
   {
      return false;
   }

   if (getInterleaveFormat() == BSQ)
   {
      // Can only get single-band BSQ accessors
      if (startBand != stopBand || concurrentBands != 1)
      {
         return false;
      }
   }

   return true;
}

bool DataRequestImp::polish(const RasterDataDescriptor *pDescriptor)
{
   if (pDescriptor == NULL)
   {
      return false;
   }

   // interleave

   InterleaveFormatType nativeInterleave = pDescriptor->getInterleaveFormat();
   if (mInterleaveDefault || pDescriptor->getBandCount() == 1)
   {
      mInterleave = nativeInterleave;
   }

   if (mInterleave == BSQ && nativeInterleave == BIL && 
      (mConcurrentRows == 0 || mConcurrentRows == 1) &&
      (mConcurrentBands == 0 || mConcurrentBands == 1))
   {
      mInterleave = BIL; // single row of BIL is the same as a single row of BSQ
   }

   // rows
   if (!mStartRow.isValid())
   {
      mStartRow = pDescriptor->getActiveRow(0);
   }
   if (!mStopRow.isValid())
   {
      mStopRow = pDescriptor->getActiveRow(pDescriptor->getRowCount()-1);
   }
   if (mConcurrentRows == 0)
   {
      mConcurrentRows = 1;
   }

   // columns
   if (!mStartColumn.isValid())
   {
      mStartColumn = pDescriptor->getActiveColumn(0);
   }
   if (!mStopColumn.isValid())
   {
      mStopColumn = pDescriptor->getActiveColumn(pDescriptor->getColumnCount()-1);
   }
   if (mConcurrentColumns == 0)
   {
      mConcurrentColumns = mStopColumn.getActiveNumber() - mStartColumn.getActiveNumber() + 1;
   }

   // bands
   if (!mStartBand.isValid())
   {
      mStartBand = pDescriptor->getActiveBand(0);
   }
   if (!mStopBand.isValid())
   {
      if (mInterleave == BIP || mInterleave == BIL)
      {
         mStopBand = pDescriptor->getActiveBand(pDescriptor->getBandCount()-1);
      }
      else
      {
         mStopBand = mStartBand;
      }
   }
   if (mConcurrentBands == 0)
   {
      mConcurrentBands = mStopBand.getActiveNumber() - mStartBand.getActiveNumber() + 1;
   }

   return true;
}

int DataRequestImp::getRequestVersion(const RasterDataDescriptor *pDescriptor) const
{
   return 1;
}


InterleaveFormatType DataRequestImp::getInterleaveFormat() const
{
   return mInterleave;
}

void DataRequestImp::setInterleaveFormat(InterleaveFormatType interleave)
{
   mInterleave = interleave;
   mInterleaveDefault = false;
}

DimensionDescriptor DataRequestImp::getStartRow() const
{
   return mStartRow;
}

DimensionDescriptor DataRequestImp::getStopRow() const
{
   return mStopRow;
}

unsigned int DataRequestImp::getConcurrentRows() const
{
   return mConcurrentRows;
}

void DataRequestImp::setRows(DimensionDescriptor startRow, DimensionDescriptor stopRow, unsigned int concurrentRows)
{
   mStartRow = startRow;
   mStopRow = stopRow;
   mConcurrentRows = concurrentRows;
}

DimensionDescriptor DataRequestImp::getStartColumn() const
{
   return mStartColumn;
}

DimensionDescriptor DataRequestImp::getStopColumn() const
{
   return mStopColumn;
}

unsigned int DataRequestImp::getConcurrentColumns() const
{
   return mConcurrentColumns;
}

void DataRequestImp::setColumns(DimensionDescriptor startColumn, DimensionDescriptor stopColumn,
                                unsigned int concurrentColumns)
{
   mStartColumn = startColumn;
   mStopColumn = stopColumn;
   mConcurrentColumns = concurrentColumns;
}

DimensionDescriptor DataRequestImp::getStartBand() const
{
   return mStartBand;
}

DimensionDescriptor DataRequestImp::getStopBand() const
{
   return mStopBand;
}

unsigned int DataRequestImp::getConcurrentBands() const
{
   return mConcurrentBands;
}

void DataRequestImp::setBands(DimensionDescriptor startBand, DimensionDescriptor stopBand, unsigned int concurrentBands)
{
   mStartBand = startBand;
   mStopBand = stopBand;
   mConcurrentBands = concurrentBands;
}

bool DataRequestImp::getWritable() const
{
   return mbWritable;
}

void DataRequestImp::setWritable(bool writable)
{
   mbWritable = writable;
}
