/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#include "Hdf4Dataset.h"

using namespace std;

Hdf4Dataset::Hdf4Dataset(const string& name) : Hdf4Element(name),
   mType(UNSUPPORTED), mCount(0), mBytesPerElement(0)
{
}

Hdf4Dataset::~Hdf4Dataset()
{
}

bool Hdf4Dataset::isCompoundDataset() const
{
   return mType == COMPOUND;
}

void Hdf4Dataset::setDataEncoding(HdfType dataType)
{
   mType = dataType;
}

void Hdf4Dataset::getDataEncoding(EncodingType& encoding) const
{
   switch (mType)
   {
   case INT1SBYTE:
      encoding = ::INT1SBYTE;
      break;
   case INT1UBYTE:
      encoding = ::INT1UBYTE;
      break;
   case INT2SBYTES:
      encoding = ::INT2SBYTES;
      break;
   case INT2UBYTES:
      encoding = ::INT2UBYTES;
      break;
   case INT4SCOMPLEX:
      encoding = ::INT4SCOMPLEX;
      break;
   case INT4SBYTES:
      encoding = ::INT4SBYTES;
      break;
   case INT4UBYTES:
      encoding = ::INT4UBYTES;
      break;
   case FLT4BYTES:
      encoding = ::FLT4BYTES;
      break;
   case FLT8COMPLEX:
      encoding = ::FLT8COMPLEX;
      break;
   case FLT8BYTES:
      encoding = ::FLT8BYTES;
      break;
   case UNSUPPORTED:
   default:
      encoding = ::UNKNOWN;
      break;
   };
}

void Hdf4Dataset::getDataEncoding(HdfType& type) const
{
   type = mType;
}

void Hdf4Dataset::setBytesPerElement(size_t sz)
{
   mBytesPerElement = sz;
}

size_t Hdf4Dataset::getBytesPerElement() const
{
   return mBytesPerElement;
}

void Hdf4Dataset::setCount(size_t count)
{
   mCount = count;
}

size_t Hdf4Dataset::getCount() const
{
   return mCount;
}
