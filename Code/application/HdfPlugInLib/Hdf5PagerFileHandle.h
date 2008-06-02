/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef HDF5_PAGER_FILE_HANDLE
#define HDF5_PAGER_FILE_HANDLE

#include <hdf5.h>

/**
 * This class is pure virtual and provides
 * access to the file handle held by an Hdf5Pager
 * so that any plug-in using the HdfPlugIn can
 * access the currently open file handle of a RasterElement
 * that is using the Hdf5Pager as it's RasterPager instance.
 */
class Hdf5PagerFileHandle
{
public:
   /**
    * Return the HDF5 file handle currently held by the Hdf5Pager instance.
    */
   virtual hid_t getFileHandle() = 0;
};

#endif
