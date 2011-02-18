/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */


#include <hdf5.h> // need to do this #include BEFORE Hdf5Utilities
#include <deque>
#include <string>
#include <vector>

#include "AppConfig.h"
#include "AppVerify.h"
#include "ComplexData.h"
#include "Hdf5Attribute.h"
#include "Hdf5Element.h"
#include "Hdf5Group.h"
#include "Hdf5Utilities.h"
#include "StringUtilities.h"

using namespace HdfUtilities;
using namespace std;

namespace
{
template<typename T>
bool readAttribute(hid_t attrId, hid_t attrType, hid_t dataSpace, DataVariant& var)
{
   auto_ptr<Hdf5CustomReader> pReader(createHdf5CustomReader<T>(attrType));
   DO_IF(pReader.get() == NULL || !pReader->isValid(), return false)
   hsize_t sizeArray[H5S_MAX_RANK];
   int num_dimensions = H5Sget_simple_extent_dims(dataSpace, sizeArray, NULL);
   DO_IF(num_dimensions != pReader->getSupportedDimensionality(), return false);

   vector<hsize_t> dimensions;
   for (int i = 0; i < num_dimensions; ++i)
   {
      dimensions.push_back(sizeArray[i]);
   }
   DO_IF(!pReader->setReadDataSpace(dimensions), return false);
   void* pData = pReader->getReadBuffer();
   Hdf5TypeResource memType(pReader->getReadMemoryType());
   herr_t status = H5Aread(attrId, *memType, pData);
   DO_IF(status < 0, return false);
   T* pValue = reinterpret_cast<T*>(pReader->getValue());
   var = *(pValue);
   delete pValue; //delete the value, because the DataVariant creates a deep copy of the value.
   return var.isValid();
}
}

namespace HdfUtilities
{
string hdf5TypeToTypeString(hid_t dataTypeId)
{
   H5T_class_t dataType = H5Tget_class(dataTypeId); // integer or floating point or misc data?
   size_t size = H5Tget_size(dataTypeId); // get the size of the type

   string type = HdfUtilities::UNKNOWN_TYPE;

   switch (dataType)
   {
   case H5T_INTEGER:
      {
         H5T_sign_t sign = H5Tget_sign(dataTypeId);
         
         if (size == 1)
         {
            if (sign == H5T_SGN_NONE)
            {
               type = TypeConverter::toString<unsigned char>();
            }
            else
            {
               type = TypeConverter::toString<char>();
            }
         }
         else if (size == 2)
         {
            if (sign == H5T_SGN_NONE)
            {
               type = TypeConverter::toString<unsigned short>();
            }
            else
            {
               type = TypeConverter::toString<short>();
            }
         }
         else if (size == 4)
         {
            if (sign == H5T_SGN_NONE)
            {
               type = TypeConverter::toString<unsigned int>();
            }
            else
            {
               type = TypeConverter::toString<int>();
            }
         }
         else
         {
            type = HdfUtilities::UNKNOWN_INTEGER_TYPE;
         }
      }
      break;
   case H5T_FLOAT:
      {
         if (size == 4)
         {
            type = TypeConverter::toString<float>();
         }
         else if (size == 8)
         {
            type = TypeConverter::toString<double>();
         }
         else
         {
            type = HdfUtilities::UNKNOWN_FLOAT_TYPE;
         }
      }
      break;
   case H5T_STRING:
      type = TypeConverter::toString<string>();
      break;
   // and now for the types of attributes we don't support - YET
   case H5T_TIME:
      type = HdfUtilities::TIME_TYPE;
      break;
   case H5T_ARRAY:
      type = HdfUtilities::ARRAY_TYPE;
      break;
   case H5T_BITFIELD:
      type = HdfUtilities::BITFIELD_TYPE;
      break;
   case H5T_COMPOUND:
      type = HdfUtilities::COMPOUND_TYPE;
      break;
   case H5T_ENUM:
      type = HdfUtilities::ENUM_TYPE;
      break;
   case H5T_OPAQUE:
      type = HdfUtilities::OPAQUE_TYPE;
      break;
   case H5T_REFERENCE:
      type = HdfUtilities::REFERENCE_TYPE;
      break;
   case H5T_VLEN:
      type = HdfUtilities::VLEN_TYPE;
      break;
   default:
      break;
   }

   if (type == HdfUtilities::COMPOUND_TYPE)
   {
      int real = H5Tget_member_index(dataTypeId, "Real");
      int imaginary = H5Tget_member_index(dataTypeId, "Imaginary");

      if ((real != -1) && (imaginary != -1))
      {
         H5T_class_t membTypeReal = H5Tget_member_class(dataTypeId, real);
         H5T_class_t membTypeImgn = H5Tget_member_class(dataTypeId, imaginary);

         Hdf5TypeResource realType(H5Tget_member_type(dataTypeId, real));
         Hdf5TypeResource imgnType(H5Tget_member_type(dataTypeId, imaginary));

         size_t realSz = H5Tget_size(*realType);
         size_t imgnSz = H5Tget_size(*imgnType);

         if (membTypeReal == H5T_INTEGER && membTypeImgn == H5T_INTEGER && realSz == 2 && imgnSz == 2)
         {
            type = TypeConverter::toString<IntegerComplex>();
         }
         else if (membTypeReal == H5T_FLOAT && membTypeImgn == H5T_FLOAT && realSz == 4 && imgnSz == 4)
         {
            type = TypeConverter::toString<FloatComplex>();
         }
      }
   }

   return type;
}

bool readHdf5Attribute(hid_t attrId, DataVariant& var)
{
   Hdf5TypeResource attrIdType(H5Aget_type(attrId));
   Hdf5DataSpaceResource attrSpace(H5Aget_space(attrId));

   string hdf5Type = HdfUtilities::hdf5TypeToTypeString(*attrIdType);

   hsize_t sizeArray[H5S_MAX_RANK];
   int numDims = H5Sget_simple_extent_dims(*attrSpace, sizeArray, NULL);
   /* numDims = 0 ==> scalar
      numDims = 1 ==> vector
      numDims = 2 ==> matrix

      sizeArray[i] = size of that dimension
      ie. sizeArray[0], numDims = 0 ==> vector of size sizeArray
   */
   size_t numElements = 1;
   for (int i = 0; i < numDims; i++)
      numElements *= static_cast<size_t>(sizeArray[i]);

   bool success = false;
   if (hdf5Type == TypeConverter::toString<unsigned char>())
   {
      if (numDims == 0)
      {
         success = readAttribute<unsigned char>(attrId, *attrIdType, *attrSpace, var);
      }
      else if (numDims == 1)
      {
         success = readAttribute<vector<unsigned char> >(attrId, *attrIdType, *attrSpace, var);
      }
   }
   else if (hdf5Type == TypeConverter::toString<char>())
   {
      if (numDims == 0)
      {
         success = readAttribute<char>(attrId, *attrIdType, *attrSpace, var);
      }
      else if (numDims == 1)
      {
         success = readAttribute<vector<char> >(attrId, *attrIdType, *attrSpace, var);
      }
   }
   else if (hdf5Type == TypeConverter::toString<unsigned short>())
   {
      if (numDims == 0)
      {
         success = readAttribute<unsigned short>(attrId, *attrIdType, *attrSpace, var);
      }
      else if (numDims == 1)
      {
         success = readAttribute<vector<unsigned short> >(attrId, *attrIdType, *attrSpace, var);
      }
   }
   else if (hdf5Type == TypeConverter::toString<short>())
   {
      if (numDims == 0)
      {
         success = readAttribute<short>(attrId, *attrIdType, *attrSpace, var);
      }
      else if (numDims == 1)
      {
         success = readAttribute<vector<short> >(attrId, *attrIdType, *attrSpace, var);
      }
   }
   else if (hdf5Type == TypeConverter::toString<unsigned int>())
   {
      if (numDims == 0)
      {
         success = readAttribute<unsigned int>(attrId, *attrIdType, *attrSpace, var);
      }
      else if (numDims == 1)
      {
         success = readAttribute<vector<unsigned int> >(attrId, *attrIdType, *attrSpace, var);
      }
   }
   else if (hdf5Type == TypeConverter::toString<int>())
   {
      if (numDims == 0)
      {
         success = readAttribute<int>(attrId, *attrIdType, *attrSpace, var);
      }
      else if (numDims == 1)
      {
         success = readAttribute<vector<int> >(attrId, *attrIdType, *attrSpace, var);
      }
   }
   else if (hdf5Type == TypeConverter::toString<float>())
   {
      if (numDims == 0)
      {
         success = readAttribute<float>(attrId, *attrIdType, *attrSpace, var);
      }
      else if (numDims == 1)
      {
         success = readAttribute<vector<float> >(attrId, *attrIdType, *attrSpace, var);
      }
   }
   else if (hdf5Type == TypeConverter::toString<double>())
   {
      if (numDims == 0)
      {
         success = readAttribute<double>(attrId, *attrIdType, *attrSpace, var);
      }
      else if (numDims == 1)
      {
         success = readAttribute<vector<double> >(attrId, *attrIdType, *attrSpace, var);
      }
   }
   else if (hdf5Type == TypeConverter::toString<string>())
   {
      if (numDims == 0)
      {
         success = readAttribute<string>(attrId, *attrIdType, *attrSpace, var);
      }
      else if (numDims == 1)
      {
         success = readAttribute<vector<string> >(attrId, *attrIdType, *attrSpace, var);
      }
   }
   else
   {
      success = false;
   }

   return success;
}

string hdf5AttributeToString(hid_t attrId)
{
   DataVariant var;
   if (HdfUtilities::readHdf5Attribute(attrId, var) && var.isValid())
   {
      return var.toXmlString();
   }
   else
   {
      throw HdfUtilities::Exception("HdfUtilities::readHdf5Attribute() failed to read attribute from file!");
   }
}

template<>
Hdf5TypeResource getHdf5Type<char>()
{
   return Hdf5TypeResource(H5Tcopy(H5T_NATIVE_CHAR));
}

template<>
Hdf5TypeResource getHdf5Type<unsigned char>()
{
   return Hdf5TypeResource(H5Tcopy(H5T_NATIVE_UCHAR));
}

template<>
Hdf5TypeResource getHdf5Type<short>()
{
   return Hdf5TypeResource(H5Tcopy(H5T_NATIVE_SHORT));
}

template<>
Hdf5TypeResource getHdf5Type<unsigned short>()
{
   return Hdf5TypeResource(H5Tcopy(H5T_NATIVE_USHORT));
}

template<>
Hdf5TypeResource getHdf5Type<int>()
{
   return Hdf5TypeResource(H5Tcopy(H5T_NATIVE_INT));
}

template<>
Hdf5TypeResource getHdf5Type<unsigned int>()
{
   return Hdf5TypeResource(H5Tcopy(H5T_NATIVE_UINT));
}

template<>
Hdf5TypeResource getHdf5Type<float>()
{
   return Hdf5TypeResource(H5Tcopy(H5T_NATIVE_FLOAT));
}

template<>
Hdf5TypeResource getHdf5Type<double>()
{
   return Hdf5TypeResource(H5Tcopy(H5T_NATIVE_DOUBLE));
}

template<>
Hdf5TypeResource getHdf5Type<std::string>()
{
   Hdf5TypeResource typeId(H5Tcopy(H5T_C_S1));
   H5Tset_size(*typeId, H5T_VARIABLE);

   return typeId;
}

bool createGroups(const string& hdfPath, hid_t fileDescriptor, bool bLastItemIsGroup)
{
   if (hdfPath.empty() == true)
   {
      return false;
   }

   // grab everything enclosed by the outermost / /
   // create the group up to the next slash
   // pass in the rest between the next / /

   // form a deque of /group1/group2/group3/data1 into a new series of groups so we can add them:
   // /group1
   // /group1/group2
   // /group1/group2/group3
   deque<string> groupsToAdd;

   // create nested groups, unless the last one is a dataset
   string groupString = hdfPath;
   if (bLastItemIsGroup == false) // /group1/group2/dataset ==> pluck off the dataset before proceeding
   {
      string::size_type pos = groupString.find_last_of("/");
      if (pos != string::npos)
      {
         groupString = groupString.substr(0, pos);
      }
   }

   // groupString should resemble '/group1/group2/group3'
   while (groupString.empty() == false && groupString.size() > 1 && groupString[0] == '/')
   {
      groupsToAdd.push_front(groupString);

      string::size_type pos = groupString.find_last_of("/");
      if (pos != string::npos)
      {
         groupString = groupString.substr(0, pos);
      }
   }

   bool bSuccess = !(groupsToAdd.empty());
   /* turn off error handling in Hdf5 library, because H5Gcreate will
      return errors if the group already exists, but there is no HDF5 library
      call to check group existence that doesn't also return errors if
      the group doesn't exist.
    */
   Hdf5ErrorHandlerResource errHandler(NULL, NULL);
   while (groupsToAdd.empty() == false)
   {
      string groupName = groupsToAdd.front();
      groupsToAdd.pop_front();

      Hdf5GroupResource groupId(H5Gcreate(fileDescriptor, groupName.c_str(), 0));
   }

   return bSuccess;
}
}
