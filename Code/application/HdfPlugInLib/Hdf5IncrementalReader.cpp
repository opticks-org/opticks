/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "Hdf5IncrementalReader.h"

using namespace std;

Hdf5IncrementalReader::Hdf5IncrementalReader(hid_t dataSet) :
   mDataset(dataSet),
   mDataspace(-1)
{
   if (mDataset > 0)
   {
      mDataspace = H5Dget_space(mDataset);
      selectNone();
   }
}

Hdf5IncrementalReader::~Hdf5IncrementalReader()
{
   if (mDataspace != -1)
   {
      H5Sclose(mDataspace);
   }
}

bool Hdf5IncrementalReader::selectHyperslab(H5S_seloper_t operation,
                                            const hsize_t* pStart,
                                            const hsize_t* pStride,
                                            const hsize_t* pCount,
                                            const hsize_t* pBlock)
{
   DO_IF(mDataspace <= 0, return false);
   return (H5Sselect_hyperslab(mDataspace, operation, pStart, pStride, pCount, pBlock) >= 0);
}

bool Hdf5IncrementalReader::selectElements(H5S_seloper_t operation, const size_t num_elements, const hsize_t ** pCoord)
{
   DO_IF(mDataspace <= 0, return false);
   return (H5Sselect_elements(mDataspace, operation, num_elements, pCoord) >= 0);
}

bool Hdf5IncrementalReader::selectAll()
{
   DO_IF(mDataspace <= 0, return false);
   return (H5Sselect_all(mDataspace) >= 0);
}

bool Hdf5IncrementalReader::selectNone()
{
   DO_IF(mDataspace <= 0, return false);
   return (H5Sselect_none(mDataspace) >= 0);
}

bool Hdf5IncrementalReader::isSelectionValid() const
{
   DO_IF(mDataspace <= 0, return false);
   return (H5Sselect_valid(mDataspace) > 0);
}

hid_t Hdf5IncrementalReader::getDataSet() const
{
   return mDataset;
}

hid_t Hdf5IncrementalReader::getSelectionDataSpace() const
{
   return mDataspace;
}

hssize_t Hdf5IncrementalReader::getNumberOfSelectedPoints() const
{
   DO_IF(mDataspace <= 0, return -1);
   return H5Sget_select_npoints(mDataspace);
}
