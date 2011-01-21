/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef HDF5PAGER_H
#define HDF5PAGER_H

#include "HdfPager.h"
#include "Hdf5PagerFileHandle.h"

#include <hdf5.h>

/**
 * This class is an on-disk accessor for HDF5 files.
 *
 * Under current conditions, no one will need to derive from Hdf5Pager unless
 * multiple HDF datasets correspond to a single data cube.
 *
 * This pager should only be used with datasets that have two
 * or three dimensions.  If used with datasets having two
 * dimensions, the band count must be 1 and the interleave format
 * must be BIP.
 */
class Hdf5Pager : public HdfPager, public Hdf5PagerFileHandle
{
public:
   SETTING(CacheSize, Hdf5Pager, unsigned int, 1024 * 1024)
   SETTING(ChunkBufferSize, Hdf5Pager, unsigned int, 64 * 1024)

   /**
    * Creates an RasterPager for HDF5 data.
    *
    * If you create a subclass, make sure to call setName() to something unique. Two plug-ins with
    * the same name will cause problems.
    */
   Hdf5Pager();

   /**
    * Destroys the RasterPager for HDF5 data.
    *
    * Closes the file and dataset that was opened by openFile().
    */
   ~Hdf5Pager();

   /**
    * @copydoc Hdf5PagerFileHandle::getFileHandle()
    */
   hid_t getFileHandle();

private:
   // file and data handles.
   hid_t mFileHandle;
   hid_t mDataHandle;
   hid_t mFileAccessProperties;

   /**
    * Opens the HDF5 file and dataset.
    *
    * Opens the HDF5 file by calling H5Fopen(filename.c_str())
    * Opens the HDF5 data handle by calling getHdfDatasetName() and H5Dopen1 on the resulting dataset name.
    */
   bool openFile(const std::string& filename);

   /**
    * Closes the HDF5 dataset and file handles.
    */
   void closeFile();

   /**
    *  Fetches a cache unit from an HDF5 file.
    */
   CachedPage::UnitPtr fetchUnit(DataRequest *pOriginalRequest);
};

#endif
