/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef HDF5_UTILITIES_H
#define HDF5_UTILITIES_H


#include "HdfUtilities.h"

#include "AppVerify.h"
#include "DataVariant.h"
#include "Hdf5CustomWriter.h"
#include "Hdf5Dataset.h"
#include "Hdf5Element.h"
#include "Hdf5Group.h"
#include "TypesFile.h"

#include <hdf5.h>

#include <memory>
#include <string>
#include <vector>

/**
 * A collection of generic utilities provided to simplify use of the application's HDF API.
 */
namespace HdfUtilities
{
   /**
    * Represents a version of H5T_INTEGER type that the HdfPlugInLib does not know how to parse.
    */
   static const std::string UNKNOWN_INTEGER_TYPE = "Unknown HDF5 Integer Type";

   /**
    * Represents a version of H5T_FLOAT type that the HdfPlugInLib does not know how to parse.
    */
   static const std::string UNKNOWN_FLOAT_TYPE = "Unknown HDF5 Float Type";

   /**
    * Represents the H5T_TIME type.  The HdfPlugInLib will not parse data of this type.
    */
   static const std::string TIME_TYPE = "HDF5 Time Type";

   /**
    * Represents the H5T_ARRAY type.  The HdfPlugInLib will not parse data of this type.
    */
   static const std::string ARRAY_TYPE = "HDF5 Array Type";

   /**
    * Represents the H5T_BITFIELD type.  The HdfPlugInLib will not parse data of this type.
    */
   static const std::string BITFIELD_TYPE = "HDF5 Bitfield Type";

   /**
    * Represents the H5T_OPAQUE type.  The HdfPlugInLib will not parse data of this type.
    */
   static const std::string OPAQUE_TYPE = "HDF5 Opaque Type";

   /**
    * Represents the H5T_COMPOUND type.  The HdfPlugInLib will not parse data of this type.
    */
   static const std::string COMPOUND_TYPE = "HDF5 Compound Type";

   /**
    * Represents the H5T_REFERENCE type.  The HdfPlugInLib will not parse data of this type.
    */
   static const std::string REFERENCE_TYPE = "HDF5 Reference Type";

   /**
    * Represents the H5T_ENUM type.  The HdfPlugInLib will not parse data of this type.
    */
   static const std::string ENUM_TYPE = "HDF5 ENUM Type";

   /**
    * Represents the H5T_VLEN type.  The HdfPlugInLib will not parse data of this type.
    */
   static const std::string VLEN_TYPE = "HDF5 VLEN Type";

   /**
    * Converts an HDF5 data type to a string.  This method simply returns a string
    * representation of the type.  It does not take the dataspace into account, ie. the 
    * dimensionality.
    *
    * NOTE: You will need to statically link against the HDF5 libraries.
    *
    * @param  dataTypeId
    *         The value returned from H5Dget_type() or H5Aget_type().
    *
    * @return A string that represents the HDF5 type.  The return value could be one of 
    *         the following:
    *           - TypeConverter::toString<char>();
    *           - TypeConverter::toString<unsigned char>();
    *           - TypeConverter::toString<short>();
    *           - TypeConverter::toString<unsigned short>();
    *           - TypeConverter::toString<int>();
    *           - TypeConverter::toString<unsigned int>();
    *           - TypeConverter::toString<float>();
    *           - TypeConverter::toString<double>();
    *           - TypeConverter::toString<string>();
    *           - HdfUtilities::UNKNOWN_INTEGER_TYPE
    *           - HdfUtilities::UNKNOWN_FLOAT_TYPE
    *           - HdfUtilities::UNKNOWN_TYPE
    *           - HdfUtilities::TIME_TYPE
    *           - HdfUtilities::ARRAY_TYPE
    *           - HdfUtilities::BITFIELD_TYPE
    *           - HdfUtilities::OPAQUE_TYPE
    *           - HdfUtilities::COMPOUND_TYPE
    *           - HdfUtilities::ENUM_TYPE
    *           - HdfUtilities::VLEN_TYPE
    */
   std::string hdf5TypeToTypeString(hid_t dataTypeId);

   /**
    *  Given an open HDF5 Attribute identifier retrieves
    *  the attribute and places it in a variant.
    *
    *  @param  attrId
    *          A handle to the HDF attribute.
    *  @param  var
    *          The variant that the attribute data will be placed in.
    *
    *  @return The number of elements (ie. 5 if a vector of 5 ints) in the attribute.
    */
   bool readHdf5Attribute(hid_t attrId, DataVariant& var);

   /**
    * Given an HDF5 Attribute handle, returns a string representing that attribute.
    *
    * This method only supports attributes supported by StringUtilities::toString()
    * and StringUtilities::fromString() minus the enumerated types in TypesFile.h.
    *
    * Throws an HdfUtilities::Exception if an error occurs.
    *
    * @param  attrId
    *         The attribute id returned by H5Aopen().
    *
    * @return A string representing the attribute. For 1-dimensional vector types,
    *         this returns the list of values: ie. (val1, val2, val3).
    */
   std::string hdf5AttributeToString(hid_t attrId);

   /**
    * Creates an HDF5 attribute with a given name and value.
    *
    * @param dataDescriptor
    *        A handle to the HDF5 dataset or HDF5 group that this attribute should
    *        be attached to.
    * @param attributeName
    *        The name of the attribute that should be created on the HDF5 dataset
    *        or HDF5 group.
    * @param attributeValue
    *        The value that the created attribute should have.  The value will
    *        determine the HDF5 type and HDF5 dataspace of the created attribute.
    *        Specifically, the Hdf5CustomWriter created for T will determine both
    *        the HDF5 type and HDF5 dataspace.
    *
    * @return Returns true if the attribute could be created with the given name
    *         and value successfully, false otherwise.
    */
   template<typename T>
   bool writeAttribute(hid_t dataDescriptor, const std::string& attributeName, const T& attributeValue)
   {
      auto_ptr<Hdf5CustomWriter> pWriter(createHdf5CustomWriter<T>());
      if (pWriter.get() == NULL)
      {
         return false;
      }

      if (!pWriter->setDataToWrite(const_cast<T*>(&attributeValue)))
      {
         return false;
      }
      Hdf5DataSpaceResource spaceId = pWriter->createDataSpace();
      VERIFY(*spaceId >= 0);
      Hdf5TypeResource memTypeId(pWriter->getWriteMemoryType());
      Hdf5TypeResource fileTypeId(pWriter->getWriteFileType());
      if (*memTypeId < 0 || *fileTypeId < 0)
      {
         return false;
      }
      Hdf5AttributeResource attrId(H5Acreate(dataDescriptor, attributeName.c_str(), *fileTypeId, *spaceId, H5P_DEFAULT));
      if (*attrId < 0)
      {
         return false;
      }

      // now write the attribute
      const void* pData = pWriter->getWriteBuffer();
      if (pData != NULL)
      {
         herr_t writeStatus = H5Awrite(*attrId, *memTypeId, pData);
         return writeStatus == 0;
      }
      return false;
   }

   /**
    * Creates an HDF5 dataset with a given name and value.
    *
    * @param fileDescriptor
    *        A handle to the HDF5 file that this dataset should be created in.
    * @param datasetName
    *        The full path to where the HDF5 dataset should be created in
    *        the HDF5 file. This function assumes that any required groups
    *        in the path have already been created.
    * @param datasetValue
    *        The value that the created dataset should have.  The value will
    *        determine the HDF5 type and HDF5 dataspace of the created dataset.
    *        Specifically, the Hdf5CustomWriter created for T will determine both
    *        the HDF5 type and HDF5 dataspace.
    *
    * @return Returns true if the dataset could be created at the given location 
    *         in the file with the given value successfully, false otherwise.
    */
   template<typename T>
   bool writeDataset(hid_t fileDescriptor, const std::string& datasetName, const T& datasetValue)
   {
      std::auto_ptr<Hdf5CustomWriter> pWriter(createHdf5CustomWriter<T>());
      if (pWriter.get() == NULL)
      {
         return false;
      }
      if (!pWriter->setDataToWrite(const_cast<T*>(&datasetValue)))
      {
         return false;
      }
      Hdf5DataSpaceResource spaceId = pWriter->createDataSpace();
      VERIFY(*spaceId >= 0);

      Hdf5TypeResource memTypeId(pWriter->getWriteMemoryType());
      Hdf5TypeResource fileTypeId(pWriter->getWriteFileType());
      if (*memTypeId < 0 || *fileTypeId < 0)
      {
         return false;
      }

      Hdf5DataSetResource dataSet(H5Dcreate(fileDescriptor, datasetName.c_str(),
         *fileTypeId, *spaceId, H5P_DEFAULT));
      if (*dataSet < 0)
      {
         return false;
      }
      const void* pData = pWriter->getWriteBuffer();
      if (pData != NULL)
      {
         herr_t writeStatus = H5Dwrite(*dataSet, *memTypeId, H5S_ALL,
            *spaceId, H5P_DEFAULT, pData);
         return writeStatus == 0;
      }
      return false;
   }

   template<typename T>
   Hdf5TypeResource getHdf5Type();

   /**
    * Creates one or more HDF5 groups with the given names.
    *
    * @param hdfPath
    *        One or more group names, separated by a '/' character, to be created.
    *
    * @param fileDescriptor
    *        A handle to the HDF5 file that the group(s) should be created in.
    *
    * @param bLastItemIsGroup
    *        True if the last item in the string is a group, false otherwise.
    *        If this is false, the text after the final '/' in hdfPath is ignored.
    *
    * @return Returns true if the group(s) were created (or already existed) at the given location(s), false otherwise.
    */
   bool createGroups(const std::string& hdfPath, hid_t fileDescriptor, bool bLastItemIsGroup = false);

}

#endif // _HDF5_H
