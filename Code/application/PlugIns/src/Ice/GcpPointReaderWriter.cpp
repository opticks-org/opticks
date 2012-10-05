/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "GcpPointReaderWriter.h"

#include "GcpList.h"
#include "Hdf5CustomReader.h"
#include "Hdf5CustomWriter.h"
#include "Hdf5Utilities.h"
#include "TypeConverter.h"

#include <list>

using namespace std;

class TempGcpTransferStruct
{
public:
   TempGcpTransferStruct() :
      mPixelX(0.0),
      mPixelY(0.0),
      mGeoX(0.0),
      mGeoY(0.0)
   {
   }

   double mPixelX;
   double mPixelY;
   double mGeoX;
   double mGeoY;
};

GcpPointListReaderWriter::GcpPointListReaderWriter() :
   mpValue(NULL),
   mpWriteBuffer(NULL),
   mpReadBuffer(NULL),
   mValid(false),
   mDataType(-1)
{
}

GcpPointListReaderWriter::GcpPointListReaderWriter(hid_t dataType) :
   mpValue(NULL),
   mpWriteBuffer(NULL),
   mpReadBuffer(NULL),
   mValid(false),
   mDataType(dataType)
{
   //if data cannot be read, return from constructor before setting mValid to true
   //so that isValid() will return false
   H5T_class_t type = H5Tget_class(dataType);
   if (type != H5T_COMPOUND)
   {
      return;
   }
   int memberCount = H5Tget_nmembers(dataType);
   if (memberCount != 4)
   {
      return;
   }
   for (int i = 0; i < 4; ++i)
   {
      Hdf5TypeResource memberType(H5Tget_member_type(dataType, i));
      if (HdfUtilities::hdf5TypeToTypeString(*memberType) != TypeConverter::toString<double>())
      {
         return;
      }
   }

   mValid = true;
}

unsigned int GcpPointListReaderWriter::getSupportedDimensionality() const
{
   return 1;
}

GcpPointListReaderWriter::~GcpPointListReaderWriter()
{
   delete [] mpReadBuffer;
   delete [] mpWriteBuffer;
}

Hdf5TypeResource GcpPointListReaderWriter::getReadMemoryType() const
{
   Hdf5TypeResource memCompoundType(H5Tcreate(H5T_COMPOUND, sizeof(TempGcpTransferStruct)));
   Hdf5TypeResource doubleType(HdfUtilities::getHdf5Type<double>());
   H5Tinsert(*memCompoundType, "pixelX", HOFFSET(TempGcpTransferStruct, mPixelX), *doubleType);
   H5Tinsert(*memCompoundType, "pixelY", HOFFSET(TempGcpTransferStruct, mPixelY), *doubleType);
   H5Tinsert(*memCompoundType, "latitude", HOFFSET(TempGcpTransferStruct, mGeoX), *doubleType);
   H5Tinsert(*memCompoundType, "longitude", HOFFSET(TempGcpTransferStruct, mGeoY), *doubleType);
   return memCompoundType;
}

bool GcpPointListReaderWriter::setDataToWrite(void* pObject)
{
   if (pObject == NULL)
   {
      return false;
   }

   mpValue = reinterpret_cast<list<GcpPoint>*>(pObject);
   delete [] mpWriteBuffer;
   mpWriteBuffer = NULL;
   return true;
}

Hdf5TypeResource GcpPointListReaderWriter::getWriteMemoryType() const
{
   return getReadMemoryType();
}

Hdf5TypeResource GcpPointListReaderWriter::getWriteFileType() const
{
   Hdf5TypeResource type(getWriteMemoryType());
   H5Tpack(*type);
   return type;
}

Hdf5DataSpaceResource GcpPointListReaderWriter::createDataSpace() const
{
   hsize_t attrDims = mpValue->size();
   return Hdf5DataSpaceResource(H5Screate_simple(1, &attrDims, NULL));
}

bool GcpPointListReaderWriter::setReadDataSpace(const vector<hsize_t>& dataSpace)
{
   if (dataSpace.size() == getSupportedDimensionality())
   {
      mDataSpace = dataSpace;
      return true;
   }
   return false;
}

void* GcpPointListReaderWriter::getReadBuffer() const
{
   if (mpReadBuffer == NULL)
   {
      int numberOfGcps = static_cast<int>(mDataSpace.front());
      mpReadBuffer = new TempGcpTransferStruct[numberOfGcps];
   }
   return mpReadBuffer;
}

const void* GcpPointListReaderWriter::getWriteBuffer() const
{
   if (mpWriteBuffer == NULL)
   {
      mpWriteBuffer = new TempGcpTransferStruct[mpValue->size()];
      list<GcpPoint>::size_type i;
      list<GcpPoint>::iterator iter;
      for (i = 0, iter = mpValue->begin();
           iter != mpValue->end();
           ++i, ++iter)
      {
         GcpPoint& point = *iter;
         mpWriteBuffer[i].mPixelX = point.mPixel.mX;
         mpWriteBuffer[i].mPixelY = point.mPixel.mY;
         mpWriteBuffer[i].mGeoX = point.mCoordinate.mX;
         mpWriteBuffer[i].mGeoY = point.mCoordinate.mY;
      }

   }
   return mpWriteBuffer;
}

void* GcpPointListReaderWriter::getValue() const
{
   if (mpReadBuffer != NULL)
   {
      int numberOfGcps = static_cast<int>(mDataSpace.front());
      mpValue = new list<GcpPoint>();
      for (int i = 0; i < numberOfGcps; ++i)
      {
         GcpPoint point;
         point.mPixel.mX = mpReadBuffer[i].mPixelX;
         point.mPixel.mY = mpReadBuffer[i].mPixelY;
         point.mCoordinate.mX = mpReadBuffer[i].mGeoX;
         point.mCoordinate.mY = mpReadBuffer[i].mGeoY;
         mpValue->push_back(point);
      }
   }
   return mpValue;
}

bool GcpPointListReaderWriter::isValid() const
{
   return mValid;
}

template<>
Hdf5CustomWriter* createHdf5CustomWriter<list<GcpPoint> >()
{
   return new GcpPointListReaderWriter();
}

template<>
Hdf5CustomReader* createHdf5CustomReader<list<GcpPoint> >(hid_t dataType)
{
   return new GcpPointListReaderWriter(dataType);
}
