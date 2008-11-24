/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef HDF5CUSTOMWRITER_H
#define HDF5CUSTOMWRITER_H

#include "Hdf5Resource.h"

/**
 * This interface should be implemented to allow a custom
 * type to be written by the HdfPlugInLib.  In addition
 * to implementing this custom interface, a template
 * specialization of createHdf5CustomWriter() needs
 * to be created for the given type.  This class
 * has a highly defined calling order for the functions
 * which is defined below:
 *    - An Hdf5CustomWriter instance is created by
 *      invoking createHdf5CustomWriter().
 *    - Hdf5CustomWriter::setDataToWrite() is called
 *      and if false is returned, the Hdf5CustomWriter
 *      instance is deleted.
 *    - Hdf5CustomWriter::getWriteFileType(),
 *      Hdf5CustomWriter::getWriteMemoryType() and
 *      Hdf5CustomWriter::createDataSpace() are called in
 *      any order.
 *    - Hdf5CustomWriter::getWriteBuffer() is called.
 *    - The Hdf5CustomWriter instance is deleted.
 */
class Hdf5CustomWriter
{
public:
   /**
    * Destroys the writer object.
    */
   virtual ~Hdf5CustomWriter() {}

   /**
    * Sets the data that should be written by
    * this writer.  If the given data is not
    * supported by this writer, false should
    * be returned.
    *
    * @param pObject
    *        The data to write.  This data
    *        can safely be cast from void* to
    *        T* where T is the type used in 
    *        template specialization of 
    *        createHdf5CustomWriter() that
    *        returned this writer instance.
    *
    * @return Returns true if the data can be written
    * by this writer, false otherwise.
    */
   virtual bool setDataToWrite(void* pObject) = 0;

   /**
    * Return the HDF5 datatype, ie. hid_t that
    * represents the file type. 
    * This type will be provided as the file datatype
    * to either a H5Acreate() or H5Dcreate() call in 
    * the HDF5 library depending on whether data
    * is being written to an HDF5 attribute or an
    * HDF5 dataset.  The hid_t returned from
    * this function will be properly closed
    * by the Hdf5TypeResource object when
    * the write operation is complete.
    *
    * This method and getWriteMemoryType() are provided
    * so that the types returned can vary.  For example
    * the file type could be packed and the memory type
    * unpacked.  Another example would be the file type
    * could be big endian and the memory type little
    * endian.
    * 
    *
    * @return Returns the HDF5 datatype that
    *         represents the file type of the
    *         data being written.
    */
   virtual Hdf5TypeResource getWriteFileType() const = 0;

   /**
    * Return the HDF5 datatype, ie. hid_t that
    * represents the in-memory type.  The type
    * returned should be compatible with the
    * void* buffer returned from getWriteBuffer().
    * This type will be provided as the memory datatype
    * to either a H5Awrite() or H5Dwrite() call in 
    * the HDF5 library depending on whether data
    * is being written to an HDF5 attribute or an
    * HDF5 dataset.  The hid_t returned from
    * this function will be properly closed
    * by the Hdf5TypeResource object when
    * the write operation is complete.
    *
    * This method and getWriteFileType() are provided
    * so that the types returned can vary.  For example
    * the file type could be packed and the memory type
    * unpacked.  Another example would be the file type
    * could be big endian and the memory type little
    * endian.
    *
    * @return Returns the HDF5 datatype that
    *         represents the in-memory type of
    *         the data being written.
    */
   virtual Hdf5TypeResource getWriteMemoryType() const = 0;

   /**
    * Returns the HDF5 dataspace that should be used
    * in the H5Acreate() or H5Dcreate().  The dataspace
    * returned should be compatible with the void* buffer
    * that will be returned from getWriteBuffer().  The
    * dataspace returned should also be compatible
    * with the dimensions of the data that was provided
    * to setDataToWrite().
    * The hid_t returned from this function will be 
    * properly closed by the Hdf5DataSpaceResource object
    * when the write operation is complete.
    *
    * @return Returns the HDF5 dataspace that represents
    *         the data being written.
    */
   virtual Hdf5DataSpaceResource createDataSpace() const = 0;

   /**
    * Returns a buffer suitable to pass to the H5Awrite()
    * or H5Dwrite() calls in the HDF5 library.  The size
    * and layout of the buffer are dependent on the HDF5
    * datatype returned from getWriteMemoryType() and
    * the dataspace returned from createDataSpace().
    * This pointer should be owned by the Hdf5CustomWriter
    * and deleted in the destructor.
    *
    * @return Returns a buffer suitable to pass to
    *         the HDF5 library calls of H5Awrite()
    *         or H5Dwrite().
    */
   virtual const void* getWriteBuffer() const = 0;
};

/**
 * This function should be specialized for any custom types
 * that need to be written using the HdfUtilities::writeAttribute()
 * or HdfUtilities::writeDataset() functions.
 * An example is shown below:
 * \code
 * struct mySampleType
 * {
 *   int foo;
 *   int bar;
 * };
 *
 * class mySampleTypeWriter; //assume this is declared elsewhere as class mySampleTypeWriter : public Hdf5CustomWriter
 * //Want to write mySampleType using Hdf5Utilities::writeAttribute() function.
 * template<>
 * Hdf5CustomWriter* createHdf5CustomWriter<mySampleType>()
 * {
 *    return new mySampleTypeWriter();
 * }
 *
 * //now to write the data as an attribute.
 * //assume groupId is a hid_t of the HDF5 group we want to write the attribute to.
 * mySampleType sampleData;
 * sampleData.foo = 3;
 * sampleData.bar = 100;
 * HdfUtilities::writeAttribute(groupId, "myAttribute", sampleData);
 * //the writeAttribute call above will use your createHdf5CustomWriter specialization and your subclass
 * //of Hdf5CustomWriter to perform the write of mySampleType to the Hdf5 file as an HDF5 attribute.
 *
 * //now to write the data to a dataset
 * //assume fd is hid_t which is an open HDF5 file handle
 * HdfUtilities::writeDataset(fd, "/Level1/Level2/MySampleDataset", sampleData);
 * //the writeDataset call above will use your createHdf5CustomWriter specialization and your subclass
 * //of Hdf5CustomWriter to perform the write of mySampleType to the Hdf5 file as an HDF5 dataset.
 * \endcode
 * 
 * @return Returns an Hdf5CustomWriter implementation that is suitable 
 *         for writing the given type to an HDF5 file.  If the given
 *         value is unsupported by the writer, NULL should be returned.
 */
template<typename T>
Hdf5CustomWriter* createHdf5CustomWriter();

#endif
