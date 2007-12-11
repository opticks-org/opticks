/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "Hdf5CustomReader.h"
#include "Hdf5CustomWriter.h"
#include "Hdf5Resource.h"
#include "Hdf5Utilities.h"
#include "TypeConverter.h"

#include <string>
#include <vector>

using namespace std;

/**
 * This class is fully defined in the .cpp instead of
 * .h which is in violation of the coding standard.
 * This was done because it is an implementation detail of
 * the HdfPlugInLib and should not be exposed to 
 * consumers of the HdfPlugInLib.
 */
template<typename T>
class ValueReaderWriter : public Hdf5CustomReader, public Hdf5CustomWriter
{
public:
   ValueReaderWriter() : mpValue(NULL)
   {
   }

   //extraArg is provided because hid_t is an int, which results in ambiguity when T is an int as well.
   ValueReaderWriter(hid_t dataType, void* extraArg) : mpValue(NULL)
   {
      //if data cannot be read, return from constructor before mpValue is set to non-NULL.
      //so that isValid() will return false.
      string parsedType = HdfUtilities::hdf5TypeToTypeString(dataType);
      string requestedType = TypeConverter::toString<T>();
      if (parsedType != requestedType)
      {
         return;
      }

      mpValue = new T();
   }

   unsigned int getSupportedDimensionality() const
   {
      return 0;
   }

   Hdf5TypeResource getReadMemoryType() const
   {
      return HdfUtilities::getHdf5Type<T>();
   }

   bool setDataToWrite(void* pObject)
   {
      if (pObject == NULL) return false;
      mpValue = reinterpret_cast<T*>(pObject);
      return true;
   }

   Hdf5TypeResource getWriteMemoryType() const
   {
      return getReadMemoryType();
   }

   Hdf5TypeResource getWriteFileType() const
   {
      return getReadMemoryType();
   }

   Hdf5DataSpaceResource createDataSpace() const
   {
      return Hdf5DataSpaceResource(H5Screate(H5S_SCALAR));
   }

   bool setReadDataSpace(const std::vector<hsize_t>& dataSpace)
   {
      return dataSpace.empty();
   }

   void* getReadBuffer() const
   {
      return const_cast<void*>(getWriteBuffer());
   }

   const void* getWriteBuffer() const
   {
      return mpValue;
   }

   void* getValue() const
   {
      return mpValue;
   }

   bool isValid() const
   {
      return (mpValue != NULL);
   }

private:
   T* mpValue; //not owned by class
};

/**
 * This class is fully defined in the .cpp instead of
 * .h which is in violation of the coding standard.
 * This was done because it is an implementation detail of
 * the HdfPlugInLib and should not be exposed to 
 * consumers of the HdfPlugInLib.
 */
template<typename T>
class ValueReaderWriter<std::vector<T> > : public Hdf5CustomReader, public Hdf5CustomWriter
{
public:
   ValueReaderWriter() : mValid(false), mpValue(NULL)
   {
   }

   ValueReaderWriter(hid_t dataType) : mpValue(NULL), mValid(false)
   {
      //if data cannot be read, return from constructor before mpValue is set to non-NULL.
      //so that isValid() will return false.
      string parsedType = HdfUtilities::hdf5TypeToTypeString(dataType);
      string requestedType = TypeConverter::toString<T>();
      if (parsedType != requestedType)
      {
         return;
      }
      mValid = true;
   }

   unsigned int getSupportedDimensionality() const
   {
      return 1;
   }

   Hdf5TypeResource getReadMemoryType() const
   {
      return HdfUtilities::getHdf5Type<T>();
   }

   bool setDataToWrite(void* pObject)
   {
      if (pObject == NULL) return false;
      mpValue = reinterpret_cast<vector<T>*>(pObject);
      mValid = true;
      return true;
   }

   Hdf5TypeResource getWriteMemoryType() const
   {
      return getReadMemoryType();
   }

   Hdf5TypeResource getWriteFileType() const
   {
      return getReadMemoryType();
   }

   Hdf5DataSpaceResource createDataSpace() const
   {
      hsize_t attrDims = mpValue->size();
      return Hdf5DataSpaceResource(H5Screate_simple(1, &attrDims, NULL));
   }

   bool setReadDataSpace(const std::vector<hsize_t>& dataSpace)
   {
      if (dataSpace.size() == getSupportedDimensionality())
      {
         delete mpValue;
         mpValue = new vector<T>(static_cast<size_t>(dataSpace.front()));
         return true;
      }
      return false;
   }

   void* getReadBuffer() const
   {
      return const_cast<void*>(getWriteBuffer());
   }

   const void* getWriteBuffer() const
   {
      return &((*mpValue)[0]);
   }

   void* getValue() const
   {
      return mpValue;
   }

   bool isValid() const
   {
      return mValid;
   }
private:
   std::vector<T>* mpValue; //not owned by class
   bool mValid;
};

/**
 * This class is fully defined in the .cpp instead of
 * .h which is in violation of the coding standard.
 * This was done because it is an implementation detail of
 * the HdfPlugInLib and should not be exposed to 
 * consumers of the HdfPlugInLib.
 */
class StringReaderWriter : public Hdf5CustomReader, public Hdf5CustomWriter
{
public:
   StringReaderWriter()
   : mpReadBuffer(NULL), mpWriteBuffer(NULL), mDataType(-1), mpValue(NULL)
   {
   }

   StringReaderWriter(hid_t dataType)
   : mpValue(NULL), mpReadBuffer(NULL), mpWriteBuffer(NULL), mDataType(dataType)
   {
      //if data cannot be read, return from constructor before mpValue is set to non-NULL.
      //so that isValid() will return false.
      string parsedType = HdfUtilities::hdf5TypeToTypeString(dataType);
      string requestedType = TypeConverter::toString<std::string>();
      if (parsedType != requestedType)
      {
         return;
      }

      mpValue = new string();
   }

   unsigned int getSupportedDimensionality() const
   {
      return 0;
   }

   ~StringReaderWriter()
   {
      delete [] mpReadBuffer;
      delete [] mpWriteBuffer;
   }

   Hdf5TypeResource getReadMemoryType() const
   {
      //The Hdf5CustomReader interface is being used.
      //copy the original data type
      //so that if the file was fixed length string, the in-memory
      //type will be a fixed length string.
      return Hdf5TypeResource(H5Tcopy(mDataType));
   }

   bool setDataToWrite(void* pObject)
   {
      if (pObject == NULL) return false;
      mpValue = reinterpret_cast<string*>(pObject);
      delete [] mpWriteBuffer;
      mpWriteBuffer = NULL;
      return true;
   }

   Hdf5TypeResource getWriteMemoryType() const
   {
      //The Hdf5CustomWriter interface is being used and
      //we always write using variable length strings
      return HdfUtilities::getHdf5Type<std::string>();
   }

   Hdf5TypeResource getWriteFileType() const
   {
      return getWriteMemoryType();
   }

   Hdf5DataSpaceResource createDataSpace() const
   {
      return Hdf5DataSpaceResource(H5Screate(H5S_SCALAR));
   }

   bool setReadDataSpace(const std::vector<hsize_t>& dataSpace)
   {
      return dataSpace.empty();
   }

   void* getReadBuffer() const
   {
      htri_t isVariableLengthStr = H5Tis_variable_str(mDataType);
      if (isVariableLengthStr)
      {
         if (mpReadBuffer == NULL)
         {
            mpReadBuffer = new char*[1]; 
         }
         return mpReadBuffer;
      }
      else
      {
         if (mpReadBuffer == NULL)
         {
            size_t size = H5Tget_size(mDataType); // get the size of the type
            mpReadBuffer = new char*[1];
            mpReadBuffer[0] = new char[size + 1]; //create fixed length char* array
            memset(mpReadBuffer[0], 0, size + 1); //set to NULL, so that string is auto null terminated
         }
         return mpReadBuffer[0];
      }
   }

   const void* getWriteBuffer() const
   {
      if (mpWriteBuffer == NULL)
      {
         mpWriteBuffer = new char*[1];
         mpWriteBuffer[0] = const_cast<char*>(mpValue->c_str());
      }
      return mpWriteBuffer;
   }

   void* getValue() const
   {
      if (mpReadBuffer != NULL)
      {
         if (mpReadBuffer[0] == NULL)
         {
            mpValue->clear();
         }
         else
         {
            *mpValue = string(mpReadBuffer[0]);
         }
      }
      return mpValue;
   }
   bool isValid() const
   {
      return (mpValue != NULL);
   }
private:
   std::string* mpValue; //not owned by class
   mutable char** mpWriteBuffer;
   mutable char** mpReadBuffer;
   hid_t mDataType;
};

/**
 * This class is fully defined in the .cpp instead of
 * .h which is in violation of the coding standard.
 * This was done because it is an implementation detail of
 * the HdfPlugInLib and should not be exposed to 
 * consumers of the HdfPlugInLib.
 */
class StringVecReaderWriter : public Hdf5CustomReader, public Hdf5CustomWriter
{
public:
   StringVecReaderWriter()
   : mpReadBuffer(NULL), mpWriteBuffer(NULL), mDataType(-1), mpValue(NULL)
   {
   }

   StringVecReaderWriter(hid_t dataType)
   : mpValue(NULL), mpReadBuffer(NULL), mpWriteBuffer(NULL), mDataType(dataType)
   {
      //if data cannot be read, return from constructor before mpValue is set to non-NULL.
      //so that isValid() will return false.
      string parsedType = HdfUtilities::hdf5TypeToTypeString(dataType);
      string requestedType = TypeConverter::toString<std::string>();
      if (parsedType != requestedType)
      {
         return;
      }

      mpValue = new vector<std::string>();
   }

   unsigned int getSupportedDimensionality() const
   {
      return 1;
   }

   ~StringVecReaderWriter()
   {
      delete [] mpReadBuffer;
      delete [] mpWriteBuffer;
   }

   Hdf5TypeResource getReadMemoryType() const
   {
      //The Hdf5CustomReader interface is being used.
      //copy the original data type
      //so that if the file was fixed length string, the in-memory
      //type will be a fixed length string.
      return Hdf5TypeResource(H5Tcopy(mDataType));
   }

   bool setDataToWrite(void* pObject)
   {
      if (pObject == NULL) return false;
      mpValue = reinterpret_cast<vector<string>*>(pObject);
      delete [] mpWriteBuffer;
      mpWriteBuffer = NULL;
      return true;
   }

   Hdf5TypeResource getWriteMemoryType() const
   {
      //The Hdf5CustomWriter interface is being used and
      //we always write using variable length strings
      return HdfUtilities::getHdf5Type<std::string>();
   }

   Hdf5TypeResource getWriteFileType() const
   {
      return getWriteMemoryType();
   }

   Hdf5DataSpaceResource createDataSpace() const
   {
      hsize_t attrDims = mpValue->size();
      return Hdf5DataSpaceResource(H5Screate_simple(1, &attrDims, NULL));
   }

   bool setReadDataSpace(const std::vector<hsize_t>& dataSpace)
   {
      if (dataSpace.size() == getSupportedDimensionality())
      {
         mDataSpace = dataSpace;
         return true;
      }
      return false;
   }

   void* getReadBuffer() const
   {
      htri_t isVariableLengthStr = H5Tis_variable_str(mDataType);
      if (isVariableLengthStr)
      {
         if (mpReadBuffer == NULL)
         {
            mpReadBuffer = new char*[static_cast<size_t>(mDataSpace.front())]; 
         }
         return mpReadBuffer;
      }
      else
      {
         if (mpReadBuffer == NULL)
         {
#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : Test the fixed length vector string reading code (kstreith)")
            size_t strLen = H5Tget_size(mDataType); // get the size of the type
            mpReadBuffer = new char*[mDataSpace.front() * strLen];
            memset(mpReadBuffer, 0, mDataSpace.front() * (strLen + 1)); //set to NULL, so that string is auto null terminated
         }
         return mpReadBuffer;
      }
   }

   const void* getWriteBuffer() const
   {
      if (mpWriteBuffer == NULL)
      {
         mpWriteBuffer = new char*[mpValue->size()];
         for (vector<std::string>::size_type i = 0; i < mpValue->size(); ++i)
         {
            mpWriteBuffer[i] = const_cast<char*>((*mpValue)[i].c_str());
         }

      }
      return mpWriteBuffer;
   }

   void* getValue() const
   {
      if (mpReadBuffer != NULL)
      {
         int numberOfStrings = static_cast<int>(mDataSpace.front());
         vector<string>& stringValues = *mpValue;
         stringValues.clear();
         stringValues.reserve(numberOfStrings);
         for (int i = 0; i < numberOfStrings; ++i)
         {
            if (mpReadBuffer[i] != NULL)
            {
               stringValues.push_back(string(mpReadBuffer[i]));
            }
            else
            {
               stringValues.push_back(string());
            }
         }
      }
      return mpValue;
   }

   bool isValid() const
   {
      return (mpValue != NULL);
   }

private:
   bool mReadMode;
   std::vector<std::string>* mpValue; //not owned by class
   mutable char** mpReadBuffer;
   mutable char** mpWriteBuffer;
   hid_t mDataType;
   vector<hsize_t> mDataSpace;
};

template<>
Hdf5CustomReader* createHdf5CustomReader<char>(hid_t dataType)
{
   return new ValueReaderWriter<char>(dataType, NULL);
}

template<>
Hdf5CustomReader* createHdf5CustomReader<unsigned char>(hid_t dataType)
{
   return new ValueReaderWriter<unsigned char>(dataType, NULL);
}

template<>
Hdf5CustomReader* createHdf5CustomReader<short>(hid_t dataType)
{
   return new ValueReaderWriter<short>(dataType, NULL);
}

template<>
Hdf5CustomReader* createHdf5CustomReader<unsigned short>(hid_t dataType)
{
   return new ValueReaderWriter<unsigned short>(dataType, NULL);
}

template<>
Hdf5CustomReader* createHdf5CustomReader<int>(hid_t dataType)
{
   return new ValueReaderWriter<int>(dataType, NULL);
}

template<>
Hdf5CustomReader* createHdf5CustomReader<unsigned int>(hid_t dataType)
{
   return new ValueReaderWriter<unsigned int>(dataType, NULL);
}

template<>
Hdf5CustomReader* createHdf5CustomReader<float>(hid_t dataType)
{
   return new ValueReaderWriter<float>(dataType, NULL);
}

template<>
Hdf5CustomReader* createHdf5CustomReader<double>(hid_t dataType)
{
   return new ValueReaderWriter<double>(dataType, NULL);
}

template<>
Hdf5CustomReader* createHdf5CustomReader<string>(hid_t dataType)
{
   return new StringReaderWriter(dataType);
}

template<>
Hdf5CustomReader* createHdf5CustomReader<vector<char> >(hid_t dataType)
{
   return new ValueReaderWriter<vector<char> >(dataType);
}

template<>
Hdf5CustomReader* createHdf5CustomReader<vector<unsigned char> >(hid_t dataType)
{
   return new ValueReaderWriter<vector<unsigned char> >(dataType);
}

template<>
Hdf5CustomReader* createHdf5CustomReader<vector<short> >(hid_t dataType)
{
   return new ValueReaderWriter<vector<short> >(dataType);
}

template<>
Hdf5CustomReader* createHdf5CustomReader<vector<unsigned short> >(hid_t dataType)
{
   return new ValueReaderWriter<vector<unsigned short> >(dataType);
}

template<>
Hdf5CustomReader* createHdf5CustomReader<vector<int> >(hid_t dataType)
{
   return new ValueReaderWriter<vector<int> >(dataType);
}

template<>
Hdf5CustomReader* createHdf5CustomReader<vector<unsigned int> >(hid_t dataType)
{
   return new ValueReaderWriter<vector<unsigned int> >(dataType);
}

template<>
Hdf5CustomReader* createHdf5CustomReader<vector<float> >(hid_t dataType)
{
   return new ValueReaderWriter<vector<float> >(dataType);
}

template<>
Hdf5CustomReader* createHdf5CustomReader<vector<double> >(hid_t dataType)
{
   return new ValueReaderWriter<vector<double> >(dataType);
}

template<>
Hdf5CustomReader* createHdf5CustomReader<vector<string> >(hid_t dataType)
{
   return new StringVecReaderWriter(dataType);
}

template<>
Hdf5CustomWriter* createHdf5CustomWriter<char>()
{
   return new ValueReaderWriter<char>();
}

template<>
Hdf5CustomWriter* createHdf5CustomWriter<unsigned char>()
{
   return new ValueReaderWriter<unsigned char>();
}

template<>
Hdf5CustomWriter* createHdf5CustomWriter<short>()
{
   return new ValueReaderWriter<short>();
}

template<>
Hdf5CustomWriter* createHdf5CustomWriter<unsigned short>()
{
   return new ValueReaderWriter<unsigned short>();
}

template<>
Hdf5CustomWriter* createHdf5CustomWriter<int>()
{
   return new ValueReaderWriter<int>();
}

template<>
Hdf5CustomWriter* createHdf5CustomWriter<unsigned int>()
{
   return new ValueReaderWriter<unsigned int>();
}

template<>
Hdf5CustomWriter* createHdf5CustomWriter<float>()
{
   return new ValueReaderWriter<float>();
}

template<>
Hdf5CustomWriter* createHdf5CustomWriter<double>()
{
   return new ValueReaderWriter<double>();
}

template<>
Hdf5CustomWriter* createHdf5CustomWriter<string>()
{
   return new StringReaderWriter();
}

template<>
Hdf5CustomWriter* createHdf5CustomWriter<vector<char> >()
{
   return new ValueReaderWriter<vector<char> >();
}

template<>
Hdf5CustomWriter* createHdf5CustomWriter<vector<unsigned char> >()
{
   return new ValueReaderWriter<vector<unsigned char> >();
}

template<>
Hdf5CustomWriter* createHdf5CustomWriter<vector<short> >()
{
   return new ValueReaderWriter<vector<short> >();
}

template<>
Hdf5CustomWriter* createHdf5CustomWriter<vector<unsigned short> >()
{
   return new ValueReaderWriter<vector<unsigned short> >();
}

template<>
Hdf5CustomWriter* createHdf5CustomWriter<vector<int> >()
{
   return new ValueReaderWriter<vector<int> >();
}

template<>
Hdf5CustomWriter* createHdf5CustomWriter<vector<unsigned int> >()
{
   return new ValueReaderWriter<vector<unsigned int> >();
}

template<>
Hdf5CustomWriter* createHdf5CustomWriter<vector<float> >()
{
   return new ValueReaderWriter<vector<float> >();
}

template<>
Hdf5CustomWriter* createHdf5CustomWriter<vector<double> >()
{
   return new ValueReaderWriter<vector<double> >();
}

template<>
Hdf5CustomWriter* createHdf5CustomWriter<vector<string> >()
{
   return new StringVecReaderWriter();
}

