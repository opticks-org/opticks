/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "StatisticsReaderWriter.h"

#include "Statistics.h"
#include "Hdf5CustomReader.h"
#include "Hdf5CustomWriter.h"
#include "Hdf5Utilities.h"
#include "ObjectResource.h"
#include "TypeConverter.h"

#include <limits>
#include <list>
#include <sstream>

using namespace std;

StatisticsValuesReaderWriter::StatisticsValuesReaderWriter()
: mDataType(-1), mpValue(NULL)
{
}

StatisticsValuesReaderWriter::StatisticsValuesReaderWriter(hid_t dataType)
: mpValue(NULL), mDataType(dataType)
{
   //if data cannot be read, return from constructor before mpValue is set to non-NULL.
   //so that isValid() will return false.
   H5T_class_t type = H5Tget_class(dataType);
   if (type != H5T_COMPOUND)
   {
      return;
   }
   static vector<string> expectedMembers;
   if (expectedMembers.empty())
   {
      expectedMembers.push_back("onDiskNumber");
      expectedMembers.push_back("average");
      expectedMembers.push_back("min");
      expectedMembers.push_back("max");
      expectedMembers.push_back("standardDeviation");
      expectedMembers.push_back("percentiles");
      expectedMembers.push_back("binCenters");
      expectedMembers.push_back("histogramCounts");
      sort(expectedMembers.begin(), expectedMembers.end());
   }
   int memberCount = H5Tget_nmembers(dataType);
   if (memberCount != expectedMembers.size())
   {
      return;
   }
   vector<string> memberNames;
   for (int i = 0; i < memberCount; ++i)
   {
      char* pMemberName = H5Tget_member_name(dataType, i);
      if (pMemberName != NULL)
      {
         memberNames.push_back(string(pMemberName));
      }
#if !defined(DEBUG)
      //In release mode, free the char* that H5Tget_member_name() creates using malloc().
      //We are leaking in debug mode, because currently in debug mode we are linking
      //with the release mode version of the Hdf5 library, which means we have two heaps
      //so asking the debug mode heap to free memory allocated on the release mode heap
      //causes an fatal error.
      free(pMemberName); 
#endif
   }
   sort(memberNames.begin(), memberNames.end());
   if (!equal(expectedMembers.begin(), expectedMembers.end(), memberNames.begin()))
   {
      return;
   }

   mpValue = new StatisticsValues();
}

unsigned int StatisticsValuesReaderWriter::getSupportedDimensionality() const
{
   return 0;
}

StatisticsValuesReaderWriter::~StatisticsValuesReaderWriter()
{
}

Hdf5TypeResource StatisticsValuesReaderWriter::getReadMemoryType() const
{
   Hdf5TypeResource memCompoundType(H5Tcreate(H5T_COMPOUND, sizeof(StatisticsValues)));
   Hdf5TypeResource uintType(HdfUtilities::getHdf5Type<unsigned int>());
   Hdf5TypeResource doubleType(HdfUtilities::getHdf5Type<double>());
   H5Tinsert(*memCompoundType, "onDiskNumber", HOFFSET(StatisticsValues, mOnDiskBandNumber), *uintType);
   H5Tinsert(*memCompoundType, "average", HOFFSET(StatisticsValues, mAverage), *doubleType);
   H5Tinsert(*memCompoundType, "min", HOFFSET(StatisticsValues, mMin), *doubleType);
   H5Tinsert(*memCompoundType, "max", HOFFSET(StatisticsValues, mMax), *doubleType);
   H5Tinsert(*memCompoundType, "standardDeviation", HOFFSET(StatisticsValues, mStandardDeviation), *doubleType);
   Hdf5TypeResource percentileType(HdfUtilities::getHdf5Type<StatisticsValues::percentileType>());
   Hdf5TypeResource variablePercentileType(H5Tvlen_create(*percentileType));
   hsize_t herr = H5Tinsert(*memCompoundType, "percentiles", HOFFSET(StatisticsValues, mpPercentiles), *variablePercentileType);
   Hdf5TypeResource binCenterType(HdfUtilities::getHdf5Type<StatisticsValues::binCenterType>());
   Hdf5TypeResource variableBinCenterType(H5Tvlen_create(*binCenterType));
   herr = H5Tinsert(*memCompoundType, "binCenters", HOFFSET(StatisticsValues, mpBinCenters), *variableBinCenterType);
   Hdf5TypeResource histogramType(HdfUtilities::getHdf5Type<StatisticsValues::histogramType>());
   Hdf5TypeResource variableHistogramTypeType(H5Tvlen_create(*histogramType));
   herr = H5Tinsert(*memCompoundType, "histogramCounts", HOFFSET(StatisticsValues, mpHistogramCounts), *variableHistogramTypeType);
   return memCompoundType;
}

bool StatisticsValuesReaderWriter::setDataToWrite(void* pObject)
{
   if (pObject == NULL) return false;
   mpValue = reinterpret_cast<StatisticsValues*>(pObject);
   return true;
}

Hdf5TypeResource StatisticsValuesReaderWriter::getWriteMemoryType() const
{
   return getReadMemoryType();
}

Hdf5TypeResource StatisticsValuesReaderWriter::getWriteFileType() const
{
   Hdf5TypeResource type(getWriteMemoryType());
   H5Tpack(*type);
   return type;
}

Hdf5DataSpaceResource StatisticsValuesReaderWriter::createDataSpace() const
{
   return Hdf5DataSpaceResource(H5Screate(H5S_SCALAR));
}

bool StatisticsValuesReaderWriter::setReadDataSpace(const vector<hsize_t>& dataSpace)
{
   return dataSpace.empty();
}

void* StatisticsValuesReaderWriter::getReadBuffer() const
{
   return mpValue;
}

const void* StatisticsValuesReaderWriter::getWriteBuffer() const
{
   return mpValue;
}

void* StatisticsValuesReaderWriter::getValue() const
{
   return mpValue;
}

bool StatisticsValuesReaderWriter::isValid() const
{
   return mpValue != NULL;
}

StatisticsMetadataReaderWriter::StatisticsMetadataReaderWriter()
: mDataType(-1), mpValue(NULL)
{
}

StatisticsMetadataReaderWriter::StatisticsMetadataReaderWriter(hid_t dataType)
: mpValue(NULL), mDataType(dataType)
{
   //if data cannot be read, return from constructor before mpValue is set to non-NULL.
   //so that isValid() will return false.
   H5T_class_t type = H5Tget_class(dataType);
   if (type != H5T_COMPOUND)
   {
      return;
   }

   static vector<string> expectedMembers;
   if (expectedMembers.empty())
   {
      expectedMembers.push_back("resolution");
      expectedMembers.push_back("badValues");
      sort(expectedMembers.begin(), expectedMembers.end());
   }
   int memberCount = H5Tget_nmembers(dataType);
   if (memberCount != expectedMembers.size())
   {
      return;
   }
   vector<string> memberNames;
   for (int i = 0; i < memberCount; ++i)
   {
      char* pMemberName = H5Tget_member_name(dataType, i);
      if (pMemberName != NULL)
      {
         memberNames.push_back(string(pMemberName));
      }
#if !defined(DEBUG)
      //In release mode, free the char* that H5Tget_member_name() creates using malloc().
      //We are leaking in debug mode, because currently in debug mode we are linking
      //with the release mode version of the Hdf5 library, which means we have two heaps
      //so asking the debug mode heap to free memory allocated on the release mode heap
      //causes an fatal error.
      free(pMemberName); 
#endif
   }
   sort(memberNames.begin(), memberNames.end());
   if (!equal(expectedMembers.begin(), expectedMembers.end(), memberNames.begin()))
   {
      return;
   }

   mpValue = new StatisticsMetadata();
}

unsigned int StatisticsMetadataReaderWriter::getSupportedDimensionality() const
{
   return 0;
}

StatisticsMetadataReaderWriter::~StatisticsMetadataReaderWriter()
{
}

Hdf5TypeResource StatisticsMetadataReaderWriter::getReadMemoryType() const
{
   Hdf5TypeResource memCompoundType(H5Tcreate(H5T_COMPOUND, sizeof(StatisticsMetadata)));
   Hdf5TypeResource uintType(HdfUtilities::getHdf5Type<unsigned int>());
   hsize_t herr = H5Tinsert(*memCompoundType, "resolution", HOFFSET(StatisticsMetadata, mStatResolution), *uintType);
   Hdf5TypeResource badValueType(HdfUtilities::getHdf5Type<StatisticsMetadata::badValueType>());
   Hdf5TypeResource variableBadValueType(H5Tvlen_create(*badValueType));
   herr = H5Tinsert(*memCompoundType, "badValues", HOFFSET(StatisticsMetadata, mBadValues), *variableBadValueType);
   return memCompoundType;
}

bool StatisticsMetadataReaderWriter::setDataToWrite(void* pObject)
{
   if (pObject == NULL) return false;
   mpValue = reinterpret_cast<StatisticsMetadata*>(pObject);
   return true;
}

Hdf5TypeResource StatisticsMetadataReaderWriter::getWriteMemoryType() const
{
   return getReadMemoryType();
}

Hdf5TypeResource StatisticsMetadataReaderWriter::getWriteFileType() const
{
   Hdf5TypeResource type(getWriteMemoryType());
   H5Tpack(*type);
   return type;
}

Hdf5DataSpaceResource StatisticsMetadataReaderWriter::createDataSpace() const
{
   return Hdf5DataSpaceResource(H5Screate(H5S_SCALAR));
}

bool StatisticsMetadataReaderWriter::setReadDataSpace(const vector<hsize_t>& dataSpace)
{
   return dataSpace.empty();
}

void* StatisticsMetadataReaderWriter::getReadBuffer() const
{
   return mpValue;
}

const void* StatisticsMetadataReaderWriter::getWriteBuffer() const
{
   return mpValue;
}

void* StatisticsMetadataReaderWriter::getValue() const
{
   return mpValue;
}

bool StatisticsMetadataReaderWriter::isValid() const
{
   return mpValue != NULL;
}

template<>
Hdf5CustomWriter* createHdf5CustomWriter<StatisticsValues>()
{
   return new StatisticsValuesReaderWriter();
}

template<>
Hdf5CustomReader* createHdf5CustomReader<StatisticsValues>(hid_t dataType)
{
   return new StatisticsValuesReaderWriter(dataType);
}

template<>
Hdf5CustomWriter* createHdf5CustomWriter<StatisticsMetadata>()
{
   return new StatisticsMetadataReaderWriter();
}

template<>
Hdf5CustomReader* createHdf5CustomReader<StatisticsMetadata>(hid_t dataType)
{
   return new StatisticsMetadataReaderWriter(dataType);
}
