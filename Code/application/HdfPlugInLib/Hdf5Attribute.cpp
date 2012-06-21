/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "Hdf5Attribute.h"

#include <string>
#include <vector>

#include "Hdf5Dataset.h"
#include "Hdf5Element.h"
#include "Hdf5File.h"
#include "Hdf5Group.h"
#include "Hdf5Resource.h"
#include "AppVerify.h"

using namespace std;

Hdf5Attribute::Hdf5Attribute(Hdf5Node* pParent, const string& name, const DataVariant& value) :
Hdf5Node(pParent, name), 
mValue(value)
{
}


Hdf5Attribute::~Hdf5Attribute()
{
}

void Hdf5Attribute::setVariant(const DataVariant &var)
{
   mValue = var;
}

const DataVariant &Hdf5Attribute::getVariant() const
{
   return mValue;
}

DataVariant &Hdf5Attribute::getVariant()
{
   return mValue;
}

/**
 * This class is fully defined in the .cpp instead of
 * .h which is in violation of the coding standard.
 * This was done because it is an implementation detail of
 * the Hdf5Attribute
 * and should not be publicly exposed to consumers of the
 * Hdf5Attribute class.
 */
class Hdf5AttributeReader : public Hdf5Data::DataReader
{
public:
   Hdf5AttributeReader(const Hdf5Attribute* pAttribute) :
      mFileMustBeClosed(false),
      mValid(false),
      mFile(-1),
      mDataset(-1),
      mGroup(-1),
      mAttr(-1),
      mType(-1),
      mDataSpace(-1)
   {
      Hdf5File* pFile = pAttribute->getFile();
      DO_IF(pFile == NULL, return);

      mFile = pFile->getFileHandle();
      if (mFile < 0)
      {
         string filename = pFile->getFilename();
         mFile = H5Fopen(filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
         DO_IF(mFile < 0, return);
         mFileMustBeClosed = true;
      }

      Hdf5Node* pNode = pAttribute->getParent();
      if (dynamic_cast<Hdf5Dataset*>(pNode) != NULL)
      {
         mDataset = H5Dopen1(mFile, pNode->getFullPathAndName().c_str());
         mAttr = H5Aopen_name(mDataset, pAttribute->getName().c_str());
      }
      else if (dynamic_cast<Hdf5Group*>(pNode) != NULL)
      {
         mGroup = H5Gopen1(mFile, pNode->getFullPathAndName().c_str());
         mAttr = H5Aopen_name(mGroup, pAttribute->getName().c_str());
      }
      DO_IF(mAttr < 0, return);
      mDataSpace = H5Aget_space(mAttr);
      DO_IF(mDataSpace < 0, return);
      mType = H5Aget_type(mAttr);
      DO_IF(mType < 0, return);

      mValid = true;
   }
   ~Hdf5AttributeReader()
   {
      if (mDataSpace >= 0)
      {
         H5Sclose(mDataSpace);
      }
      if (mType >= 0)
      {
         H5Tclose(mType);
      }
      if (mAttr >= 0)
      {
         H5Aclose(mAttr);
      }
      if (mGroup >= 0)
      {
         H5Gclose(mGroup);
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
      DO_IF(num_dimensions < 0 || static_cast<unsigned int>(num_dimensions) !=
         pReader->getSupportedDimensionality(), return NULL);

      vector<hsize_t> dimensions;
      for (int i = 0; i < num_dimensions; ++i)
      {
         dimensions.push_back(sizeArray[i]);
      }
      DO_IF(!pReader->setReadDataSpace(dimensions), return NULL);
      void* pData = pReader->getReadBuffer();
      Hdf5TypeResource memType(pReader->getReadMemoryType());
      int readStatus = H5Aread(mAttr, *memType, pData);
      DO_IF(readStatus < 0, return NULL);
      return pReader->getValue();
   }
private:
   bool mFileMustBeClosed;
   bool mValid;
   hid_t mFile;
   hid_t mDataset;
   hid_t mGroup;
   hid_t mAttr;
   hid_t mType;
   hid_t mDataSpace;
};

Hdf5Data::DataReader* Hdf5Attribute::createReader() const
{
   Hdf5AttributeReader* pReader = new Hdf5AttributeReader(this);
   if (pReader->isValid())
   {
      return pReader;
   }

   delete pReader;
   return NULL;
}
