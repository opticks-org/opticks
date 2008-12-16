/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef HDF5IMPORTERSHELL_H
#define HDF5IMPORTERSHELL_H

#include "HdfImporterShell.h"

#include <hdf5.h>

class Hdf5FileResource;
class RasterElement;

/**
 * The base class of an importer that loads data in HDF Version 5 format.
 *
 * The HDF5 Importer Shell is a class designed to wrap the HDF5 libraries and
 * provide a convenient means of importing simple (header, metadata, 1 cube)
 * HDF5 data. The shell class is highly customizable, but to use
 * these features requires downloading the HDF5 v1.6.2 precompiled libraries
 * from <A HREF=ftp://ftp.ncsa.uiuc.edu/HDF/HDF5/prev-releases/hdf5-1.6.5/bin/>*</A>
 * or <A HREF=ftp://ftp.ncsa.uiuc.edu/HDF/HDF5/hdf5-1.6.5/bin/>*</A>.
 *
 * To link a derived plug-in under Windows, define _HDF5USEDLL_ and link to hdf5dll.lib.
 * To link a derived plug-in under Solaris, define _HDF5USEDLL_ and link to libhdf5.so.0.0.0.
 * 
 */
class Hdf5ImporterShell : public HdfImporterShell
{
protected:
   /**
    *  Constructs the Hdf5ImporterShell object to load file data in HDF Version 5 format.
    *
    *  Any class that derives from %Hdf5ImporterShell MUST link against:
    *  szlib.lib, zdll.lib, and hdf5.lib on Windows systems, and
    *  libsz.a, libz.a, and libhdf5.a on Solaris systems.
    */
   Hdf5ImporterShell();

   /**
    *  Destroys the Hdf5ImporterShell object.
    */
   ~Hdf5ImporterShell() {}

   /**
    * Retrieve open HDF5 library handle to the file. This may either involve directly opening the file
    * or retrieving the already open file handle from the Hdf5Pager if the file was loaded on-disk read-only.
    * Never attempt to just open the file using the HDF5 api because they specifically warn against a single process
    * opening the same file twice with the HDF5 api.
    *
    *  @param  fileHandle
    *          On successful return, the fileHandle to use for HDF5 library access.
    *  @param fileResource
    *         An Hdf5FileResource which must remain in scope while fileHandle is being used.
    *         On successful return, the resource may or may not be valid, depending on how pElement was loaded.
    *
    *  @return True if fileHandle is valid, false otherwise.
    */
   bool getFileHandle(hid_t& fileHandle, Hdf5FileResource& fileResource) const;

private:
  /**
    * Creates an RasterPager Plug-In and sets it in the RasterElement.
    *
    * This method can be overridden for importers that do not have a one-to-one correspondence between
    * an HDF dataset and an RasterElement (ie. MODIS). Those that load one HDF dataset as a RasterElement
    * can use the default implementation provided.
    *
    * Called from HdfImporterShell::createRasterPagerPlugIn(const string&, const string&, RasterElement&).
    *
    * @param  pRaster
    *         A pointer to the RasterElement for placing the raster pager plug-in.
    *
    * @return TRUE if the operation succeeds, FALSE otherwise.
    */
   bool createRasterPager(RasterElement *pRaster) const;
};

#endif
