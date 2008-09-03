/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "DateTimeReaderWriter.h"

#include "DateTime.h"
#include "Hdf5CustomReader.h"
#include "Hdf5CustomWriter.h"
#include "Hdf5Utilities.h"
#include "ObjectResource.h"
#include "TypeConverter.h"

#include <limits>
#include <list>
#include <sstream>

using namespace std;

class TempDateTimeTransferStruct
{
public:
   TempDateTimeTransferStruct() {}
   unsigned char mDateValid;
   unsigned char mMonth;
   unsigned char mDay;
   unsigned int mYear;
   unsigned char mTimeValid;
   unsigned char mHour;
   unsigned char mMinute;
   unsigned char mSecond;
};

DateTimeReaderWriter::DateTimeReaderWriter()
: mpReadBuffer(NULL), mpWriteBuffer(NULL), mDataType(-1), mpValue(NULL)
{
}

DateTimeReaderWriter::DateTimeReaderWriter(hid_t dataType)
: mpValue(NULL), mpReadBuffer(NULL), mpWriteBuffer(NULL), mDataType(dataType)
{
   //if data cannot be read, return from constructor before mpValue is set to non-NULL.
   //so that isValid() will return false.
   H5T_class_t type = H5Tget_class(dataType);
   if (type != H5T_COMPOUND)
   {
      return;
   }
   int memberCount = H5Tget_nmembers(dataType);
   if (memberCount != 8)
   {
      return;
   }
   for (int i = 0; i < 8; ++i)
   {
      Hdf5TypeResource memberType(H5Tget_member_type(dataType, i));
      string typeStr = HdfUtilities::hdf5TypeToTypeString(*memberType);
      if (typeStr != TypeConverter::toString<unsigned char>() &&
          typeStr != TypeConverter::toString<unsigned int>() )
      {
         return;
      }
   }

   FactoryResource<DateTime> pDateTime;
   mpValue = pDateTime.release();
}

unsigned int DateTimeReaderWriter::getSupportedDimensionality() const
{
   return 0;
}

DateTimeReaderWriter::~DateTimeReaderWriter()
{
   delete mpReadBuffer;
   delete mpWriteBuffer;
}

Hdf5TypeResource DateTimeReaderWriter::getReadMemoryType() const
{
   Hdf5TypeResource memCompoundType(H5Tcreate(H5T_COMPOUND, sizeof(TempDateTimeTransferStruct)));
   Hdf5TypeResource ucharType(HdfUtilities::getHdf5Type<unsigned char>());
   Hdf5TypeResource uintType(HdfUtilities::getHdf5Type<unsigned int>());
   H5Tinsert(*memCompoundType, "dateValid", HOFFSET(TempDateTimeTransferStruct, mDateValid), *ucharType);
   H5Tinsert(*memCompoundType, "month", HOFFSET(TempDateTimeTransferStruct, mMonth), *ucharType);
   H5Tinsert(*memCompoundType, "day", HOFFSET(TempDateTimeTransferStruct, mDay), *ucharType);
   H5Tinsert(*memCompoundType, "year", HOFFSET(TempDateTimeTransferStruct, mYear), *uintType);
   H5Tinsert(*memCompoundType, "timeValid", HOFFSET(TempDateTimeTransferStruct, mTimeValid), *ucharType);
   H5Tinsert(*memCompoundType, "hour", HOFFSET(TempDateTimeTransferStruct, mHour), *ucharType);
   H5Tinsert(*memCompoundType, "minute", HOFFSET(TempDateTimeTransferStruct, mMinute), *ucharType);
   H5Tinsert(*memCompoundType, "second", HOFFSET(TempDateTimeTransferStruct, mSecond), *ucharType);
   return memCompoundType;
}

Hdf5TypeResource DateTimeReaderWriter::getWriteMemoryType() const
{
   return getReadMemoryType();
}

Hdf5TypeResource DateTimeReaderWriter::getWriteFileType() const
{
   Hdf5TypeResource type(getWriteMemoryType());
   H5Tpack(*type);
   return type;
}

Hdf5DataSpaceResource DateTimeReaderWriter::createDataSpace() const
{
   return Hdf5DataSpaceResource(H5Screate(H5S_SCALAR));
}

bool DateTimeReaderWriter::setReadDataSpace(const vector<hsize_t>& dataSpace)
{
   return dataSpace.empty();
}

void* DateTimeReaderWriter::getReadBuffer() const
{
   if (mpReadBuffer == NULL)
   {
      mpReadBuffer = new TempDateTimeTransferStruct;
   }
   return mpReadBuffer;
}

bool DateTimeReaderWriter::setDataToWrite(void* pObject)
{
   if (pObject == NULL) return false;
   mpValue = reinterpret_cast<DateTime*>(pObject);
   delete mpWriteBuffer;
   mpWriteBuffer = NULL;
   return true;
}

const void* DateTimeReaderWriter::getWriteBuffer() const
{
   if (mpWriteBuffer == NULL)
   {
      mpWriteBuffer = new TempDateTimeTransferStruct;
      mpWriteBuffer->mDateValid = (mpValue->isValid() ? 1 : 0);
      if (mpValue->isValid())
      {
         string formatString = mpValue->getFormattedUtc("%d %m %Y");
         istringstream formatParser;
         formatParser.str(formatString);
         unsigned int day, year, month;
         formatParser >> day;
         formatParser >> month;
         formatParser >> year;
         mpWriteBuffer->mDay = day;
         mpWriteBuffer->mYear = year;
         mpWriteBuffer->mMonth = month;
      }
      else
      {
         mpWriteBuffer->mDay = numeric_limits<unsigned char>::max();
         mpWriteBuffer->mYear = numeric_limits<unsigned int>::max();
         mpWriteBuffer->mMonth = numeric_limits<unsigned char>::max();
      }
      mpWriteBuffer->mTimeValid = (mpValue->isTimeValid() ? 1 : 0);
      if (mpValue->isTimeValid())
      {
         string formatString = mpValue->getFormattedUtc("%H %M %S");
         istringstream formatParser;
         formatParser.str(formatString);
         unsigned int hour, minute, second;
         formatParser >> hour;
         formatParser >> minute;
         formatParser >> second;
         mpWriteBuffer->mHour = hour;
         mpWriteBuffer->mMinute = minute;
         mpWriteBuffer->mSecond = second;
      }
      else
      {
         mpWriteBuffer->mHour = numeric_limits<unsigned char>::max();
         mpWriteBuffer->mMinute = numeric_limits<unsigned char>::max();
         mpWriteBuffer->mSecond = numeric_limits<unsigned char>::max();
      }
   }
   return mpWriteBuffer;
}

void* DateTimeReaderWriter::getValue() const
{
   if (mpReadBuffer != NULL)
   {
      if (mpReadBuffer->mDateValid)
      {
         if (mpReadBuffer->mTimeValid)
         {
            mpValue->set(mpReadBuffer->mYear, mpReadBuffer->mMonth, mpReadBuffer->mDay,
               mpReadBuffer->mHour, mpReadBuffer->mMinute, mpReadBuffer->mSecond);
         }
         else
         {
            mpValue->set(mpReadBuffer->mYear, mpReadBuffer->mMonth, mpReadBuffer->mDay);
         }
      }
   }
   return mpValue;
}

bool DateTimeReaderWriter::isValid() const
{
   return mpValue != NULL;
}

template<>
Hdf5CustomWriter* createHdf5CustomWriter<DateTime>()
{
   return new DateTimeReaderWriter();
}

template<>
Hdf5CustomReader* createHdf5CustomReader<DateTime>(hid_t dataType)
{
   return new DateTimeReaderWriter(dataType);
}
