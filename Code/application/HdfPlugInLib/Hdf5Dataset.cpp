/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#include "Hdf5Dataset.h"

#include "ComplexData.h"
#include "Hdf5File.h"
#include "Hdf5Utilities.h"
#include "TypeConverter.h"
#include "TypesFile.h"

using namespace std;

Hdf5Dataset::Hdf5Dataset(Hdf5Element* pParent, const string& name)
 : Hdf5Element(pParent, name)
{
}

Hdf5Dataset::~Hdf5Dataset()
{
}

bool Hdf5Dataset::isCompoundDataset() const
{
   return getTypeName() == HdfUtilities::COMPOUND_TYPE;
}

void Hdf5Dataset::getDataEncoding(EncodingType& encoding) const
{
   if (getTypeName() == TypeConverter::toString<char>())
   {
      encoding = INT1SBYTE;
   }
   else if (getTypeName() == TypeConverter::toString<unsigned char>())
   {
      encoding = INT1UBYTE;
   }
   else if (getTypeName() == TypeConverter::toString<short>())
   {
      encoding = INT2SBYTES;
   }
   else if (getTypeName() == TypeConverter::toString<unsigned short>())
   {
      encoding = INT2UBYTES;
   }
   else if (getTypeName() == TypeConverter::toString<int>())
   {
      encoding = INT4SBYTES;
   }
   else if (getTypeName() == TypeConverter::toString<unsigned int>())
   {
      encoding = INT4UBYTES;
   }
   else if (getTypeName() == TypeConverter::toString<float>())
   {
      encoding = FLT4BYTES;
   }
   else if (getTypeName() == TypeConverter::toString<double>())
   {
      encoding = FLT8BYTES;
   }
   else if (getTypeName() == TypeConverter::toString<IntegerComplex>())
   {
      encoding = INT4SCOMPLEX;
   }
   else if (getTypeName() == TypeConverter::toString<FloatComplex>())
   {
      encoding = FLT8COMPLEX;
   }
   else
   {
      encoding = UNKNOWN;
   }
}

/**
 * This class is fully defined in the .cpp instead of
 * .h which is in violation of the coding standard.
 * This was done because it is an implementation detail of
 * the Hdf5Dataset
 * and should not be publicly exposed to consumers of the
 * Hdf5Dataset class.
 */
class Hdf5DatasetReader : public Hdf5Data::DataReader
{
public:
   Hdf5DatasetReader(const Hdf5Dataset* pDataset) :
      mFileMustBeClosed(false),
      mValid(false),
      mFile(-1),
      mDataset(-1),
      mType(-1),
      mDataSpace(-1)
   {
      Hdf5File* pFile = pDataset->getFile();
      DO_IF(pFile == NULL, return);

      mFile = pFile->getFileHandle();
      if (mFile < 0)
      {
         string filename = pFile->getFilename();
         mFile = H5Fopen(filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
         DO_IF(mFile < 0, return);
         mFileMustBeClosed = true;
      }

      mDataset = H5Dopen(mFile, pDataset->getFullPathAndName().c_str());
      DO_IF(mDataset < 0, return);
      mDataSpace = H5Dget_space(mDataset);
      DO_IF(mDataSpace < 0, return);
      mType = H5Dget_type(mDataset);
      DO_IF(mType < 0, return);
      mValid = true;
   }
   ~Hdf5DatasetReader()
   {
      if (mDataSpace >= 0)
      {
         H5Sclose(mDataSpace);
      }
      if (mType >= 0)
      {
         H5Tclose(mType);
      }
      if (mDataset >= 0)
      {
         H5Dclose(mDataset);
      }
      if (mFile >= 0 && mFileMustBeClosed)
      {
         H5Fclose(mFile);
      }
   }

   bool isValid()
   {
      return mValid;
   }

   hid_t getType()
   {
      return mType;
   }
   hid_t getDataSpace()
   {
      return mDataSpace;
   }
   void* readData(Hdf5CustomReader* pReader)
   {
      DO_IF(pReader == NULL || !pReader->isValid(), return NULL);

      hsize_t sizeArray[H5S_MAX_RANK];
      int num_dimensions = H5Sget_simple_extent_dims(mDataSpace, sizeArray, NULL);
      DO_IF(num_dimensions != pReader->getSupportedDimensionality(), return NULL);

      vector<hsize_t> dimensions;
      for (int i = 0; i < num_dimensions; ++i)
      {
         dimensions.push_back(sizeArray[i]);
      }
      DO_IF(!pReader->setReadDataSpace(dimensions), return NULL);
      void* pData = pReader->getReadBuffer();
      Hdf5TypeResource memType(pReader->getReadMemoryType());
      int readStatus = H5Dread(mDataset, *memType, H5S_ALL, mDataSpace, H5P_DEFAULT, pData);
      DO_IF(readStatus < 0, return NULL);
      return pReader->getValue();
   }
private:
   bool mFileMustBeClosed;
   bool mValid;
   hid_t mFile;
   hid_t mDataset;
   hid_t mType;
   hid_t mDataSpace;
};

Hdf5Data::DataReader* Hdf5Dataset::createReader() const
{
   Hdf5DatasetReader* pReader = new Hdf5DatasetReader(this);
   if (pReader->isValid())
   {
      return pReader;
   }

   delete pReader;
   return NULL;
}
