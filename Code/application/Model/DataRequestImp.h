/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DATAREQUESTIMP_H
#define DATAREQUESTIMP_H

#include "DataRequest.h"
#include "TypesFile.h"
#include "DimensionDescriptor.h"

class DataRequestImp : public DataRequest
{
public:
   DataRequestImp();
   DataRequestImp(const DataRequestImp& rhs);
   ~DataRequestImp();

   DataRequest *copy() const;

   bool validate(const RasterDataDescriptor *pDescriptor) const;
   bool polish(const RasterDataDescriptor *pDescriptor);

   int getRequestVersion(const RasterDataDescriptor *pDescriptor) const;

   InterleaveFormatType getInterleaveFormat() const;
   void setInterleaveFormat(InterleaveFormatType interleave);

   DimensionDescriptor getStartRow() const;
   DimensionDescriptor getStopRow() const;
   unsigned int getConcurrentRows() const;
   void setRows(DimensionDescriptor startRow, DimensionDescriptor stopRow, unsigned int concurrentRows = 0);

   DimensionDescriptor getStartColumn() const;
   DimensionDescriptor getStopColumn() const;
   unsigned int getConcurrentColumns() const;
   void setColumns(DimensionDescriptor startColumn, DimensionDescriptor stopColumn, unsigned int concurrentColumns = 0);

   DimensionDescriptor getStartBand() const;
   DimensionDescriptor getStopBand() const;
   unsigned int getConcurrentBands() const;
   void setBands(DimensionDescriptor startBand, DimensionDescriptor stopBand, unsigned int concurrentBands = 0);

   bool getWritable() const;
   void setWritable(bool writable);

private:
   InterleaveFormatType mInterleave;
   bool mInterleaveDefault;

   DimensionDescriptor mStartRow;
   DimensionDescriptor mStopRow;
   unsigned int mConcurrentRows;
   
   DimensionDescriptor mStartColumn;
   DimensionDescriptor mStopColumn;
   unsigned int mConcurrentColumns;

   DimensionDescriptor mStartBand;
   DimensionDescriptor mStopBand;
   unsigned int mConcurrentBands;

   bool mbWritable;

};

#endif