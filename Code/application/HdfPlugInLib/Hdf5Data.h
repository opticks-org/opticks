/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef HDF5DATA_H
#define HDF5DATA_H

#include "Hdf5CustomReader.h"

#include <memory>
#include <string>
#include <vector>

#include <hdf5.h>

/**
 * This class is an abstraction of the data contained within a HDF5 attribute
 * or HDF5 dataset.  This class provides a way to determine the type of
 * data stored within the attribute or dataset along with the dimensions.
 * This class also provides a templated method to extract the contents of
 * the attribute or dataset.
 */
class Hdf5Data
{
public:
   virtual ~Hdf5Data() {}

   /**
    * Gets the type name of this HDF data.
    *
    * @return the type name of this HDF data.
    *
    * @see HdfUtilities::hdf5TypeToTypeString() for more details.
    */
   virtual std::string getTypeName() const;

   /**
    * Sets the type name of this HDF data.
    *
    * @param typeName
    *        The type name of this HDF data.  See HdfUtilities::hdf5TypeToTypeString()
    *        for more details.
    */
   virtual void setTypeName(const std::string& typeName);

   /**
    * Returns the sizes of the dimensions of the HDF data.  If
    * data contains a scalar value, the returned vector will be empty.
    * For example, if the data has 3 dimensions with sizes of 10, 50, 5,
    * the getDimensionSizes().size() method should return 3. The
    * getDimensionSizes()[0] should return 10 and getDimensionSizes()[1]
    * should return 50.
    *
    * @return The sizes of the dimensions of the dataset.
    */
   virtual const std::vector<hsize_t>& getDimensionSizes() const;

   /**
    * Sets the dimension sizes of this HDF data.  If the data
    * contains a scalar value, the provided vector should be empty.
    *
    * @param dimensionSizes
    *        The dimension sizes of the HDF data.
    */
   virtual void setDimensionSizes(const std::vector<hsize_t>& dimensionSizes);

   /**
    * Reads data from the HDF object.  The caller of this function
    * is responsible for deleting the returned memory.
    *
    * @return Returns the parsed value.  If the data was not of the correct
    *         type or dimension, NULL will be returned.
    */
   template<typename T>
   T* readData() const
   {
      std::auto_ptr<DataReader> pDataReader(createReader());
      DO_IF(pDataReader.get() == NULL, return NULL);
      hid_t type = pDataReader->getType();
      DO_IF(type < 0, return NULL);
      hid_t dataSpace = pDataReader->getDataSpace();
      DO_IF(dataSpace < 0, return NULL);
      std::auto_ptr<Hdf5CustomReader> pCustomReader(createHdf5CustomReader<T>(type));
      DO_IF(pCustomReader.get() == NULL, return NULL);
      return reinterpret_cast<T*>(pDataReader->readData(pCustomReader.get()));
   }

   /**
    * \cond INTERNAL
    * This class is not being documented because in order to construct an
    * instance that will be used, you have to subclass this interface and
    * only two classes are currently permitted to do so.  This class
    * is not protected because then the two subclasses would have to
    * show that in their header files as well.
    */
   class DataReader
   {
   public:
      virtual ~DataReader() {}
      virtual hid_t getType() = 0;
      virtual hid_t getDataSpace() = 0;
      virtual void* readData(Hdf5CustomReader* pReader) = 0;
   };
   /// \endcond

protected:
   Hdf5Data();


   virtual DataReader* createReader() const = 0;

   std::string mTypeName;
   std::vector<hsize_t> mDimensionSizes;
};

#endif
