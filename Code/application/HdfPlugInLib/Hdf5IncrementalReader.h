/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef HDF5_ICREMENTAL_READER_H
#define HDF5_ICREMENTAL_READER_H

#include "Hdf5CustomReader.h"

#include <string>
#include <vector>

#include <hdf5.h>

/**
 * This class provides the ability to incrementally read
 * values from a HDF5 dataset without having to read the
 * entire dataset into memory at once as Hdf5Dataset::readData()
 * requires.
 */
class Hdf5IncrementalReader
{
public:
   /**
    * Construct an instance of this reader to read data
    * from the provided HDF5 dataset.
    *
    * @param dataSet
    *        A HDF5 dataset handle to an already open HDF5 dataset.
    *        This dataset handle will not be closed by the reader.
    */
   Hdf5IncrementalReader(hid_t dataSet);

   /**
    * Destroys the Hdf5IncrementalReader.  The HDF5 dataset
    * handle provided in the constructor is NOT closed.
    */
   virtual ~Hdf5IncrementalReader();

   /**
    * Selects points in the dataset that will be read when calling
    * readSelecteData(). This method is a wrapper for
    * H5Sselect_hyperslab().  Please reference the
    * documentation in the HDF5 library for more details.
    *
    * @param operation
    *        Please see H5Sselect_hyperslab() documentation.
    * @param pStart
    *        Please see H5Sselect_hyperslab() documentation.
    * @param pStride
    *        Please see H5Sselect_hyperslab() documentation.
    * @param pCount
    *        Please see H5Sselect_hyperslab() documentation.
    * @param pBlock
    *        Please see H5Sselect_hyperslab() documentation.
    *
    * @return Returns false if an invalid HDF5 dataset was provided or
    *         H5Sselect_hyperslab() returns a negative value.
    */
   bool selectHyperslab(H5S_seloper_t operation,
      const hsize_t* pStart,
      const hsize_t* pStride,
      const hsize_t* pCount,
      const hsize_t* pBlock);

   /**
    * Selects points in the dataset that will be read when calling
    * readSelecteData(). This method is a wrapper for
    * H5Sselect_elements().  Please reference the
    * documentation in the HDF5 library for more details.
    *
    * @param operation
    *        Please see H5Sselect_elements() documentation.
    * @param num_elements
    *        Please see H5Sselect_elements() documentation.
    * @param pCoord
    *        Please see H5Sselect_elements() documentation.
    *
    * @return Returns false if an invalid HDF5 dataset was provided or
    *         H5Sselect_elements() returns a negative value.
    */
   bool selectElements(H5S_seloper_t operation, const size_t num_elements, const hsize_t ** pCoord);

   /**
    * Selects all points in the dataset, so that they
    * will be read when calling readSelectedData(). This
    * method is a wrapper for
    * H5Sselect_all().  Please reference the
    * documentation in the HDF5 library for more details.
    *
    * @return Returns false if an invalid HDF5 dataset was provided or
    *         H5Sselect_all() returns a negative value.
    */
   bool selectAll();

   /**
    * Selects no points in the dataset, so that nothing
    * will be read when calling readSelectedData(). This
    * method is a wrapper for
    * H5Sselect_none().  Please reference the
    * documentation in the HDF5 library for more details.
    *
    * @return Returns false if an invalid HDF5 dataset was provided or
    *         H5Sselect_none() returns a negative value.
    */
   bool selectNone();

   /**
    * Returns true if the current selection is valid
    * for the dataset.  This method is a wrapper for
    * H5Sselect_valid().
    *
    * @return Returns false if an invalid HDF5 dataset was provided
    *         or H5Sselect_valid() returns a value of 0 or less.
    */
   bool isSelectionValid() const;

   /**
    * Returns a handle to the HDF5 dataset that was provided in
    * the constructor.
    *
    * @return Returns a handle to the HDF5 dataset being read this
    *         reader.
    */
   hid_t getDataSet() const;

   /**
    * Returns a handle to the HDF5 dataspace that will be used
    * as the selection when reading data out of the HDF5 dataset.
    *
    * @return Returns a handle to the HDF5 dataspace that defines
    *         the selected data.
    */
   hid_t getSelectionDataSpace() const;

   /**
    * Returns a count of the number of points that are selected
    * in the HDF5 dataset for reading.  This method is a wrapper
    * for H5Sget_select_npoints() using the dataspace handle 
    * returned from getSelectionDataSpace().
    *
    * @returns Returns -1 if an invalid HDF5 dataset was provided,
    *          otherwise the return value of H5Sget_select_npoints().
    */
   hssize_t getNumberOfSelectedPoints() const;

   /**
    * Reads the selected data from the HDF dataset as the given
    * type. The caller of this function
    * is responsible for deleting the returned memory.
    *
    * @return Returns the parsed value.
    *         If the data could not be parsed by the Hdf5CustomReader instance
    *         then NULL will be returned.
    */
   template<typename T>
   T* readSelectedData() const
   {
      DO_IF(mDataset < 0, return NULL);
      DO_IF(mDataspace < 0, return NULL);
      Hdf5TypeResource type(H5Dget_type(mDataset));
      DO_IF(*type < 0, return NULL);
      std::auto_ptr<Hdf5CustomReader> pReader(createHdf5CustomReader<T>(*type));
      DO_IF(pReader.get() == NULL || !pReader->isValid(), return NULL);

      hssize_t numberOfPoints = getNumberOfSelectedPoints();
      DO_IF(numberOfPoints <= 0, return NULL);
      bool supported = false;
      vector<hsize_t> dimensions;
      if (pReader->getSupportedDimensionality() == 1)
      {
         dimensions.push_back(numberOfPoints);
         supported = true;
      }
      else if (pReader->getSupportedDimensionality() == 0 && numberOfPoints == 1)
      {
         supported = true;
      }
      DO_IF(!supported, return NULL);
      DO_IF(!pReader->setReadDataSpace(dimensions), return NULL);
      void* pData = pReader->getReadBuffer();
      Hdf5TypeResource memType(pReader->getReadMemoryType());
      hsize_t readDataSpaceDims = numberOfPoints;
      Hdf5DataSpaceResource readDataSpace(H5Screate_simple(1, &readDataSpaceDims, NULL));
      int readStatus = H5Dread(mDataset, *memType, *readDataSpace, mDataspace, H5P_DEFAULT, pData);
      DO_IF(readStatus < 0, return NULL);
      return reinterpret_cast<T*>(pReader->getValue());
   }

protected:
   hid_t mDataset;
   hid_t mDataspace;
};

#endif