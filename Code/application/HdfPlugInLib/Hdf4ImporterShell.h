/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef HDF4IMPORTERSHELL_H
#define HDF4IMPORTERSHELL_H

#include "AppConfig.h"

#include "HdfImporterShell.h"

#include <string>

class Hdf4Dataset;
class Hdf4File;
class RasterElement;

/**
 * \ingroup ShellModule
 * The base class of an importer that loads data in HDF Version 4 format.
 *
 * The HDF4 Importer Shell is a class designed to wrap the HDF4 libraries and
 * provide a convenient means of importing simple (header, metadata, 1 cube)
 * HDF4 data. The shell class is highly customizable, but to use
 * these features requires (a) downloading the HDF4 v4.2r1 precompiled libraries
 * from <A HREF=ftp://ftp.ncsa.uiuc.edu/HDF/HDF/HDF4.2r1/bin/>*</A>
 *
 * To link a derived plugin under Windows, link to hm421md.lib/hd421md.lib (DEBUG)
 * or hm421m.lib/hd421m.lib (RELEASE).
 * To link a derived plugin under Solaris, link to libmfhdf.a, libdf.a, libjpeg.a,
 * and libudport.a.
 */
class Hdf4ImporterShell : public HdfImporterShell
{
protected:
   /**
    * Constructs the Hdf4ImporterShell object to load file data in HDF Version 4 format.
    */
   Hdf4ImporterShell();

   /**
    * Populates a representation of the HDF File.
    *
    * @param  hdfFile
    *         An EMPTY Hdf4File object to populate.
    *
    * @return TRUE if the %Hdf4File object was successfully populated, otherwise FALSE.
    */
   virtual bool getFileData(Hdf4File& hdfFile) const;

   /**
    *  Loads entire Hdf4Dataset into the studio's memory block and returns the pointer to it.
    *
    *  Calls ModelServices::getMemoryBlock() to fetch the memory.
    *
    *  @param  hdfFile
    *          A const reference to the Hdf4File that contains the dataset to load into memory.
    *  @param  dataset
    *          The dataset to load into memory from the Hdf4File.
    *
    *  @return A pointer to memory in the studio's memory space with the loaded data. If
    *          passed in NULL, this method will allocate enough memory for reading.
    */
   void* loadDatasetFromFile(const Hdf4File& hdfFile, const Hdf4Dataset& dataset) const;

private:
   /**
    *  Creates a RasterPager plug-in and sets it in the RasterElement.
    *
    *  This method can be overridden for importers that do not have a one-to-one
    *  correspondence between an HDF dataset and a RasterElement (e.g. MODIS).
    *  Importers that load one HDF dataset as a RasterElement can use the
    *  default implementation provided.
    *
    *  Called from HdfImporterShell::createRasterPagerPlugIn(const std::string&, RasterElement&).
    *
    *  @param   pRaster
    *           A pointer to the RasterElement for placing the raster pager
    *           plug-in.  This method does nothing and returns \c false if
    *           \c NULL is passed in.
    *
    *  @return  Returns \c true if pager plug-in is successfully created and set
    *           in the given raster element; otherwise returns \c false.
    */
   virtual bool createRasterPager(RasterElement* pRaster) const;
};

#endif
