/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef HDF4_DATASET_H
#define HDF4_DATASET_H

#include "EnumWrapper.h"
#include "Hdf4Element.h"
#include "TypesFile.h"

#include <string>

class Hdf4Dataset : public Hdf4Element
{
public:
   /**
    * Types supported by HdfPlugInLib.
    */
   enum HdfTypeEnum
   {
      INT1SBYTE = 0,      /**< char */
      INT1UBYTE = 1,      /**< unsigned char */
      INT2SBYTES = 2,     /**< short */
      INT2UBYTES = 3,     /**< unsigned short */
      INT4SCOMPLEX = 4,   /**< complex short */
      INT4SBYTES = 5,     /**< int */
      INT4UBYTES = 6,     /**< unsigned int */
      FLT4BYTES = 7,      /**< float */
      FLT8COMPLEX = 8,    /**< complex float */
      FLT8BYTES = 9,      /**< double */
      STRING = 10,        /**< std::string */
      COMPOUND = 11,      /**< Compund or VDATA dataset */
      UNSUPPORTED = 12    /**< Indicates a value unsupported by HdfPlugInLib */
   };

   /**
    * @EnumWrapper Hdf4Dataset::HdfTypeEnum.
    */
   typedef EnumWrapper<HdfTypeEnum> HdfType;

   /**
    * Gets whether the dataset is compound or not.
    *
    * Compound datasets have no data encoding (UNKNOWN), 0 per element, and 0 count.
    * If this function returns true, HdfPlugInLib will not have parsed the dataset because
    * the HdfPlugInLib does not support loading compound datasets.
    *
    * @return Returns whether the dataset is a compound dataset (ie. vdata) or not.
    */
   virtual bool isCompoundDataset() const;

   /**
    * Sets the data type for the HDF dataset.
    *
    * For non-native types or compound datasets, returns UNSUPPORTED.
    *
    * @param  dataType
    *         The HdfType that corresponds to dataset's type.
    *         For those types that do not correspond to an HdfType, UNSUPPORTED
    *         should be passed in.
    */
   virtual void setDataEncoding(HdfType dataType);

   /**
    * Gets the data encoding type for the HDF dataset.
    *
    * For non-native types or compound datasets, returns UNKNOWN.
    *
    * @param  encoding
    *         The data encoding type that corresponds to the member string.
    *         For those types that do not correspond to an EncodingType (ie. a compound dataset),
    *         UNKNOWN is returned.
    *
    */
   virtual void getDataEncoding(EncodingType& encoding) const;

   /**
    * Gets the HDF data type for the HDF dataset.
    *
    * For unsupported types or compound datasets, returns UNSUPPORTED.
    *
    * @param  type
    *         The data type that corresponds to the member string.
    *         For those types that do not correspond to an HdfType (ie. a compound dataset),
    *         UNSUPPORTED is returned.
    */
   virtual void getDataEncoding(HdfType& type) const;

   /*
    * Sets the number of bytes per element in the dataset.
    *
    * Typically is only used by HdfImporterShell::getFileData().
    *
    * @param  sz
    *         The number of bytes per element in the dataset.
    */
   virtual void setBytesPerElement(size_t sz);

   /**
    * Returns the number of bytes per element of the dataset.
    *
    * @return The total number of bytes per element of the dataset.
    */
   virtual size_t getBytesPerElement() const;

   /*
    * Sets the total number of elements in the dataset - even if it is multidimensional.
    *
    * Typically is only used by HdfImporterShell::getFileData().
    *
    * @param  count
    *         The total number of elements in the dataset.
    */
   virtual void setCount(size_t count);

   /**
    * Returns the total number of elements in the dataset - even if it is multi-dimensional.
    *
    * @return The total number of elements in the dataset.
    */
   virtual size_t getCount() const;

protected:
   // The Hdf4Group must be a friend so the Hdf4Group can delete and create Hdf4Datasets
   friend class Hdf4Group;

   /**
    * Creates an Hdf4Dataset object that represents the data set object of an HDF file.
    *
    * To create a dataset, use Hdf4Group::addDataset().
    */
   explicit Hdf4Dataset(const std::string& name);

   /**
    * Destroys the Hdf4Dataset object.
    *
    * To destroy an Hdf4Dataset from a group, call Hdf4Group::removeDataset().
    */
   virtual ~Hdf4Dataset();

private:
   HdfType mType;
   size_t mCount;
   size_t mBytesPerElement;
};

#endif
