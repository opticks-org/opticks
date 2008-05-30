/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef HDF_PAGER_H
#define HDF_PAGER_H

#include <string>

#include "CachedPager.h"

/**
 * This base class is a raster pager for all HDF files. It provides a specification
 * for HDF4 and HDF5 pagers.
 *
 * @see Hdf4Pager, Hdf5Pager
 */
class HdfPager : public CachedPager
{
public:
   /**
    * Creates an HdfPager plug-in.
    */
   HdfPager();

   /**
    * Creates an input argument list for the HdfAccessor plug-in.
    *
    * @param  pIn
    *         The input argument list that is created.
    *
    * @return TRUE if the operation succeeds, FALSE otherwise.
    */
   bool getInputSpecification(PlugInArgList*& pIn);

protected:
   /**
    * A constant that represents an invalid handle in all forms of the HDF C API.
    */
   static const int INVALID_HANDLE;

   /**
    * Parses and extracts input arguments.
    *
    * @param  pIn
    *         The argument list to parse. Cannot be NULL.
    *
    * @return TRUE if the values were properly extracted from the argument list, FALSE otherwise.
    */
   bool parseInputArgs(PlugInArgList* pIn);
   
   /**
    * Returns the name of the HDF dataset.
    *
    * The HDF dataset name is one of the input arguments. Gets populated from parseInputArgs(PlugInArgList*).
    *
    * @return  The name of the HDF dataset. For HDF4 data sets, this will be a name. For HDF5 datasets,
    *          this will be the full path and name that uniquely identifies the dataset.
    */
   const std::string& getHdfDatasetName() const { return mHdfName; }

   /**
    * Closes the HDF file.
    *
    * The HDF File is opened in the subclasses.
    */
   virtual void closeFile() = 0;

private:
   std::string mHdfName;
};

#endif
