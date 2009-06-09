/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef HDF5CUSTOMREADER_H
#define HDF5CUSTOMREADER_H

#include "Hdf5Resource.h"

#include <hdf5.h>

#include <vector>

/**
 * This interface should be implemented to allow a custom
 * type to be read by the HdfPlugInLib.  In addition
 * to implementing this custom interface, a template
 * specialization of createHdf5CustomReader() needs
 * to be created for the given type.  This class
 * has a highly defined calling order for the functions
 * which is defined below:
 *    - An Hdf5CustomReader instance is created by
 *      invoking createHdf5CustomReader().
 *    - Hdf5CustomReader::isValid() is called and
 *      if the return value is false, the Hdf5CustomReader
 *      instance is deleted.
 *    - Hdf5CustomReader::getSupportedDimensionality() is called
 *      and if the return value does not match the dimensionality
 *      required to read the data in, the Hdf5CustomReader
 *      instance is deleted.
 *    - Hdf5CustomReader::setReadDataSpace() is called and
 *      if the return value is false, the Hdf5CustomReader
 *      instance is deleted..
 *    - Hdf5CustomReader::getReadMemoryType() is called.
 *    - Hdf5CustomReader::getReadBuffer() is called.
 *    - Hdf5CustomReader::getValue() is called.
 *    - The Hdf5CustomReader instance is deleted.
 */
class Hdf5CustomReader
{
public:
   /**
    * Destroys the reader object.
    */
   virtual ~Hdf5CustomReader() {}

   /**
    * Determines whether the reader is valid
    * for the given HDF5 datatype 
    * provided to the createHdf5CustomReader() 
    * function.  If false is returned, this
    * instance is immediately deleted and
    * the read operation is stopped.  If true
    * is returned the read operation continues.
    *
    * @return Return false in your implementation
    *         if the given HDF5 datatype
    *         is NOT supported by this
    *         reader class, otherwise return true.
    */
   virtual bool isValid() const = 0;

   /**
    * Returns the HDF5 dimensionality or rank 
    * supported by this Hdf5CustomReader interface.
    *
    * @return Returns the HDF5 dimensionality or rank
    *         supported by this Hdf5CustomReader.
    */
   virtual unsigned int getSupportedDimensionality() const = 0;

   /**
    * Return the HDF5 datatype, ie. hid_t that
    * represents the in-memory type.  The type
    * returned should be compatible with the
    * void* buffer returned from getReadBuffer().
    * This type will be provided as the memory datatype
    * to either a H5Aread() or H5Dread() call in 
    * the HDF5 library depending on whether data
    * is being read from an HDF5 attribute or an
    * HDF5 dataset.  The hid_t returned from
    * this function will be properly closed
    * by the Hdf5TypeResource object when
    * the read operation is complete.
    *
    * @return Returns the HDF5 datatype that
    *         represents the in-memory type. If
    *         appropriate you can simly return
    *         a H5Tcopy() of the type provided
    *         to createHdf5CustomReader().
    */
   virtual Hdf5TypeResource getReadMemoryType() const = 0;

   /**
    * Sets the dimensions of the data that will
    * be read in from the given dataspace.
    *
    * @param dataSpace
    *        The dimensions and size of each dimension for
    *        the data that will be read in.  An empty
    *        vector indicates scalar data will be read
    *        in, ie. a single value.
    *
    * @return Return true if the provided dimensions are
    *         supported by this reader, false otherwise.
    */
   virtual bool setReadDataSpace(const std::vector<hsize_t>& dataSpace) = 0;

   /**
    * Returns a buffer suitable to pass to the H5Aread()
    * or H5Dread() calls in the HDF5 library.  The type
    * and layout of the buffer are dependent on the HDF5
    * datatype returned from getReadMemoryType().  The
    * size of the buffer is dependent on the value 
    * provided to setReadDataSpace. This
    * pointer should be owned by the Hdf5CustomReader
    * and deleted in the destructor.
    *
    * @return Returns a buffer suitable to pass to
    *         the HDF5 library calls of H5Aread()
    *         or H5Dread().
    */
   virtual void* getReadBuffer() const = 0;

   /**
    * Returns a pointer to data which can
    * be safely cast to T*, where T is the
    * template specialization of
    * createHdf5CustomReader.  The returned
    * data should NOT be owned by the
    * Hdf5CustomReader and will be owned
    * by the caller of this function. The
    * T* returned should be populated from
    * the data that was pushed into the 
    * getReadBuffer() by the H5Aread()
    * or H5Dread() calls.
    *
    * @return Returns a pointer to data
    * which can be safely cast to T*
    * by the caller, where T is
    * template specialization of createHdf5CustomReader().
    */
   virtual void* getValue() const = 0;


};

/**
 * This function should be specialized for any custom types
 * that need to be read using the Hdf5Data::readData() function
 * which is provided for both Hdf5Dataset and Hdf5Attribute.
 * An example is shown below:
 * \code
 * struct mySampleType
 * {
 *   int foo;
 *   int bar;
 * };
 *
 * class mySampleTypeReader; //assume this is declared elsewhere as class mySampleTypeReader : public Hdf5CustomReader
 * //Want to read mySampleType using Hdf5Data::readData function
 * template<>
 * Hdf5CustomReader* createHdf5CustomReader<mySampleType>(hid_t dataType)
 * {
 *    return new mySampleTypeReader(dataType);  
 * }
 *
 * //now to read the data, assume pData is a pointer of type Hdf5Data, ie. Hdf5Dataset or Hdf5Attribute
 * mySampleType* pSampleData = pData->readData<mySampleType>();
 * //the read call above will use your createHdf5CustomReader specialization and your subclass
 * //of Hdf5CustomReader to perform the read from the Hdf5 file and conversion to mySampleType.
 * \endcode
 * 
 *
 * @param dataType
 *        This is the HDF5 datatype of the data that will be read.
 *
 * @return Returns an Hdf5CustomReader implementation that is suitable 
 *         for reading the given type from a HDF5 file.  If the given
 *         dataType is unsupported by the reader either
 *         NULL can be returned from this method or the Hdf5CustomReader
 *         instance returned from this method can return false
 *         from Hdf5CustomReader::isValid().
 */
template<typename T>
Hdf5CustomReader* createHdf5CustomReader(hid_t dataType);

#endif
