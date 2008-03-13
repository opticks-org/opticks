/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef HDF5_INCREMENTAL_WRITER_H
#define HDF5_INCREMENTAL_WRITER_H

#include "Hdf5CustomWriter.h"

#include <string>
#include <vector>

#include <hdf5.h>

/**
 * This class provides the ability to incrementally
 * write values to a HDF5 dataset without having
 * to write values to the entire dataset as 
 * HdfUtilities::writeDataset() requires.
 */
template<typename T>
class Hdf5IncrementalWriter
{
public:
   /**
    * Create the HDF5 dataset that will be written
    * to by this writer.  The dataset will be
    * created in the file with the given name
    * and dimensions.  The HDF5 type of the dataset
    * is determined by the T that this class
    * is templated on.  The HDF5 type of the dataset
    * will be queried from the
    * Hdf5CustomWriter::getWriteFileType() instance
    * that is constructed by calling
    * createCustomHdf5Writer() with the given T.
    * To determine if the HDF5 dataset was created
    * successfully in the file, call isValid()
    * after this object has been constructed.
    *
    * @param fileId
    *        A handle to an open HDF5 file that
    *        the dataset should be written to.
    * @param datasetName
    *        The full path to a location in the file
    *        where the HDF5 datatset should be created.
    *        This writer assumes that any required
    *        groups in the path have already been created
    *        in the file.
    * @param datasetDimensions
    *        A vector specifying the number of dimensions
    *        and the size of the dimensions that should
    *        be used when creating the HDF5 dataset.
    */
   Hdf5IncrementalWriter(hid_t fileId,
      const std::string& datasetName,
      const std::vector<hsize_t>& datasetDimensions)
   {
      mpWriter = createHdf5CustomWriter<T>();
      mDataset = -1;
      mDataspace = -1;
      if (mpWriter == NULL)
      {
         return;
      }
      if (datasetDimensions.empty())
      {
         mDataspace = H5Screate(H5S_SCALAR);
      }
      else
      {      
         mDataspace = H5Screate_simple(datasetDimensions.size(), &(datasetDimensions.front()), NULL);
      }
      if (mDataspace != -1)
      {
         mDataset = H5Dcreate(fileId, datasetName.c_str(),
            *(mpWriter->getWriteFileType()), mDataspace, H5P_DEFAULT);
      }
   }

   /**
    * Destroys the writer instance.  This will also
    * close the HDF5 dataset that was being written
    * to by this writer.
    */
   virtual ~Hdf5IncrementalWriter()
   {
      delete mpWriter;
      if (mDataspace != -1)
      {
         H5Sclose(mDataspace);
      }

      if (mDataset != -1)
      {
         H5Dclose(mDataset);
      }
   }

   /**
    * Returns whether this writer
    * is valid to use.  If this
    * method returns false, you 
    * should not attempt to call
    * any other functions on this writer.
    * This method should be called
    * immediately after the constructor.
    *
    * @return Returns true is this writer 
    *         is valid, false otherwise.
    */
   bool isValid() const
   {
      return mpWriter != NULL && mDataspace != -1 && mDataset != -1;
   }

   /**
    * Returns a HDF5 handle to the dataset
    * that is being written to by this writer.
    *
    * @return Returns a HDF5 handle to the dataset
    *         that is being written to.
    */
   hid_t getDataSet() const
   {
      return mDataset;
   }

   /**
    * Returns the dataspace that represents
    * the selection within the dataset
    * that is being written to.
    *
    * @return Returns a HDF5 dataspace handle
    *         to the selection within the
    *         dataset that is being written to.
    */
   hid_t getDataSpace() const
   {
      return mDataspace;
   }

   /**
    * Provides a convenient way to write data
    * to a single dimensional HDF5 dataset.
    * (ie. a one-dimensional array).  This method
    * will write data to a single dimensional HDF5
    * dataset starting at the row provided.
    *
    * @param startRowNumber
    *        The starting row number in the single
    *        dimensional HDF5 dataset where data
    *        should be written.  This number is zero-based.
    * @param object
    *        The data that should be written to the
    *        HDF5 dataset starting at the given row
    *        number.  This data can be scalar or it
    *        can be single dimensional as well.
    *
    * Returns true if the write was successful, false otherwise.
    */
   bool writeBlock(hsize_t startRowNumber, const T& object)
   {
      DO_IF(!isValid(), return false);
      DO_IF(H5Sget_simple_extent_ndims(mDataspace) != 1, return false);
      DO_IF(!mpWriter->setDataToWrite(const_cast<T*>(&object)), return false);
      Hdf5DataSpaceResource memSpaceId(mpWriter->createDataSpace());
      DO_IF(*memSpaceId < 0, return false);

      hsize_t sizeArray[H5S_MAX_RANK];
      int numdims = H5Sget_simple_extent_dims(*memSpaceId, sizeArray, NULL);
      if (numdims == 0)
      {
         sizeArray[0] = 1;
      }

      hsize_t oneValue = 1;
      DO_IF(H5Sselect_hyperslab(mDataspace, H5S_SELECT_SET, &startRowNumber, &oneValue, sizeArray, NULL) < 0,
         return false);

      Hdf5TypeResource memTypeId(mpWriter->getWriteMemoryType());
      DO_IF(*memTypeId < 0, return false)
      const void* pData = mpWriter->getWriteBuffer();
      if (pData != NULL)
      {
         herr_t writeStatus = H5Dwrite(mDataset, *memTypeId, *memSpaceId,
            mDataspace, H5P_DEFAULT, pData);
         return writeStatus == 0;
      }
      return false;
   }

   /**
    * Writes the specified data to a selection
    * in the dataset defined by a hyperslab.
    * The hyperslab selection is done by using
    * H5Sselect_hyperslab() provided by the
    * HDF5 library.  Please note that this
    * method erases any previous selection in
    * the dataset before performing the hyperslab
    * selection.  The provided data will be
    * written to the selected hyperslab in
    * the HDF5 dataset.
    * 
    * @param pStart
    *        Please see H5Sselect_hyperslab() documentation.
    * @param pStride
    *        Please see H5Sselect_hyperslab() documentation.
    * @param pCount
    *        Please see H5Sselect_hyperslab() documentation.
    * @param pBlock
    *        Please see H5Sselect_hyperslab() documentation.
    * @param object
    *        The data that should be written to the selected
    *        hyperslab in the file.  The provided data
    *        must contain the same number of points as the
    *        selected hyperslab contains.
    *
    * @return Returns true if the hyperslab selection and
    *         write were successful, false otherwise.
    */
   bool writeHyperslab(const hsize_t* pStart,
      const hsize_t* pStride,
      const hsize_t* pCount,
      const hsize_t* pBlock,
      const T& object)
   {
      DO_IF(!isValid(), return false);
      DO_IF(H5Sselect_hyperslab(mDataspace, H5S_SELECT_SET, pStart, pStride, pCount, pBlock) < 0, return false);
      return writeToSelectedData(object);
   }

   /**
    * Writes the specified data to a selection
    * in the dataset defined by a element selection.
    * The element selection is done by using
    * H5Sselect_elements() provided by the
    * HDF5 library.  Please note that this
    * method erases any previous selection in
    * the dataset before performing the element
    * selection.  The provided data will be
    * written to the selected elements in
    * the HDF5 dataset.
    * 
    * @param num_elements
    *        Please see H5Sselect_elements() documentation.
    * @param pCoordinates
    *        Please see H5Sselect_elements() documentation.
    * @param object
    *        The data that should be written to the selected
    *        elements in the file.  The provided data
    *        must contain the same number of points as the
    *        element selection contains.
    *
    * @return Returns true if the element selection and
    *         write were successful, false otherwise.
    */
   bool writeElements(const size_t num_elements,
      const hsize_t ** pCoordinates,
      const T& object)
   {
      DO_IF(!isValid(), return false);
      DO_IF(H5Sselect_elements(mDataspace, H5S_SELECT_SET, num_elements, pCoordinates) < 0, return false);
      return writeToSelectedData(object);
   }

   /**
    * Writes the provided data to the current selection
    * as returned by getDataSpace().  This method should
    * be used if the options provided by writeBlock(),
    * writeHyperslab() and writeElements() is insufficent.
    * In this case, you can get the dataspace by calling
    * getDataSpace() and the perform the selection manually
    * using HDF5 library calls.
    *
    * @param object
    *        The data that should be written to the current
    *        selection in the file.  The provided data
    *        must contain the same number of points as the
    *        current selection contains.
    *
    * @return Returns true if the write was successful,
    *         false otherwise.
    */
   bool writeToSelectedData(const T& object)
   {
      DO_IF(!isValid(), return false);
      DO_IF(!mpWriter->setDataToWrite(const_cast<T*>(&object)), return false);
      Hdf5DataSpaceResource memSpaceId(mpWriter->createDataSpace());
      DO_IF(*memSpaceId < 0, return false);

      DO_IF(H5Sget_select_npoints(mDataspace) != H5Sget_select_npoints(*memSpaceId), return false);

      Hdf5TypeResource memTypeId(mpWriter->getWriteMemoryType());
      DO_IF(*memTypeId < 0, return false)
      const void* pData = mpWriter->getWriteBuffer();
      if (pData != NULL)
      {
         herr_t writeStatus = H5Dwrite(mDataset, *memTypeId, *memSpaceId,
            mDataspace, H5P_DEFAULT, pData);
         return writeStatus == 0;
      }
      return false;
   }

protected:
   hid_t mDataset;
   hid_t mDataspace;
   Hdf5CustomWriter* mpWriter;
};

#endif