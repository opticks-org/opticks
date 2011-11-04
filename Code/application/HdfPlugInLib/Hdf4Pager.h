/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef HDF4PAGER_H
#define HDF4PAGER_H

#include "AppConfig.h"

#include "HdfPager.h"

#include <hdf.h>
#include <mfhdf.h>

/**
 * This class is an on-disk accessor for HDF4 files.
 *
 * Under current conditions, no one will need to derive from Hdf4Pager unless
 * multiple HDF datasets correspond to a single data cube or the
 * data interleave is BIL, which is currently not natively supported.
 */
class Hdf4Pager : public HdfPager
{
public:
   /**
    * Creates an RasterPager for HDF4 data.
    *
    * If you create a subclass, make sure to call setName() to something unique. Two plug-ins with
    * the same name will cause problems.
    */
   Hdf4Pager();

   /**
    * Destroys the RasterPager for HDF4 data.
    *
    * Closes the file and dataset that was opened by openFile().
    */
   ~Hdf4Pager();

private:
   Hdf4Pager& operator=(const Hdf4Pager& rhs);

   // file and data handles.
   int mFileHandle;
   int32 mDataHandle;

   /**
    * Opens the HDF4 file and dataset.
    *
    * Opens the HDF4 file by calling SDstart(filename.c_str()).
    * Opens the HDF4 data handle by calling getHdfDatasetName() and SDnametoindex(datasetName)
    * and SDselect on the resulting dataset index.
    */
   bool openFile(const std::string& filename);

   /**
    * Closes the HDF4 dataset and file handles.
    */
   void closeFile();

   /**
    *  Fetches a CacheUnit from an HDF4 file.
    */
   CachedPage::UnitPtr fetchUnit(DataRequest *pOriginalRequest);

};

#endif
