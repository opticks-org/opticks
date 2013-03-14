/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef STATISTICSREADERWRITER_H
#define STATISTICSREADERWRITER_H

#include "Hdf5CustomReader.h"
#include "Hdf5CustomWriter.h"
#include "Hdf5Resource.h"
#include "IceUtilities.h"

class StatisticsMetadataFloat
{
public:
   StatisticsMetadataFloat() :
      mStatResolution(0)
   {}

   int mStatResolution;
   hvl_t mBadValues;
   typedef char BadValueType;
};

class StatisticsMetadata
{
public:
   StatisticsMetadata() :
      mStatResolution(0)
   {}

   int mStatResolution;
   hvl_t mBadValues;
   typedef int BadValueType;
};

class StatisticsValues
{
public:
   StatisticsValues() :
      mOnDiskBandNumber(0),
      mAverage(0.0),
      mMin(0.0),
      mMax(0.0),
      mStandardDeviation(0.0)
   {
   }

   unsigned int mOnDiskBandNumber;
   double mAverage;
   double mMin;
   double mMax;
   double mStandardDeviation;
   hvl_t mpPercentiles;
   typedef double PercentileType;
   hvl_t mpBinCenters;
   typedef double BinCenterType;
   hvl_t mpHistogramCounts;
   typedef unsigned int HistogramType;
};

class StatisticsValuesReaderWriter : public Hdf5CustomReader, public Hdf5CustomWriter
{
public:
   StatisticsValuesReaderWriter();
   StatisticsValuesReaderWriter(hid_t dataType);
   ~StatisticsValuesReaderWriter();

   unsigned int getSupportedDimensionality() const;
   Hdf5TypeResource getReadMemoryType() const;
   bool setDataToWrite(void* pObject);
   Hdf5TypeResource getWriteMemoryType() const;
   Hdf5TypeResource getWriteFileType() const;
   Hdf5DataSpaceResource createDataSpace() const;
   bool setReadDataSpace(const std::vector<hsize_t>& dataSpace);
   void* getReadBuffer() const;
   const void* getWriteBuffer() const;
   void* getValue() const;
   bool isValid() const;

private:
   mutable StatisticsValues* mpValue; //not owned by class
   hid_t mDataType;
};

class StatisticsMetadataFloatReaderWriter : public Hdf5CustomReader, public Hdf5CustomWriter
{
public:
   StatisticsMetadataFloatReaderWriter();
   StatisticsMetadataFloatReaderWriter(hid_t dataType);
   ~StatisticsMetadataFloatReaderWriter();

   unsigned int getSupportedDimensionality() const;
   Hdf5TypeResource getReadMemoryType() const;
   bool setDataToWrite(void* pObject);
   Hdf5TypeResource getWriteMemoryType() const;
   Hdf5TypeResource getWriteFileType() const;
   Hdf5DataSpaceResource createDataSpace() const;
   bool setReadDataSpace(const std::vector<hsize_t>& dataSpace);
   void* getReadBuffer() const;
   const void* getWriteBuffer() const;
   void* getValue() const;
   bool isValid() const;

private:
   mutable StatisticsMetadataFloat* mpValue; //not owned by class
   hid_t mDataType;
};

class StatisticsMetadataReaderWriter : public Hdf5CustomReader, public Hdf5CustomWriter
{
public:
   StatisticsMetadataReaderWriter();
   StatisticsMetadataReaderWriter(hid_t dataType);
   ~StatisticsMetadataReaderWriter();

   unsigned int getSupportedDimensionality() const;
   Hdf5TypeResource getReadMemoryType() const;
   bool setDataToWrite(void* pObject);
   Hdf5TypeResource getWriteMemoryType() const;
   Hdf5TypeResource getWriteFileType() const;
   Hdf5DataSpaceResource createDataSpace() const;
   bool setReadDataSpace(const std::vector<hsize_t>& dataSpace);
   void* getReadBuffer() const;
   const void* getWriteBuffer() const;
   void* getValue() const;
   bool isValid() const;

private:
   mutable StatisticsMetadata* mpValue; //not owned by class
   hid_t mDataType;
};

#endif
