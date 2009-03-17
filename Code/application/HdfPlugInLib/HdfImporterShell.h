/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef HDFIMPORTERSHELL_H
#define HDFIMPORTERSHELL_H

#include "RasterElementImporterShell.h"
#include "ProgressTracker.h"

#include <string>
#include <vector>

class RasterElement;

/**
 *  \ingroup ShellModule
 *  The base class for all HDF Importers.
 *
 *  Requires linking against PlugInUtilities.
 */
class HdfImporterShell : public RasterElementImporterShell
{
public:
   /**
    * Constructs the HdfImporterShell object.
    */
   HdfImporterShell();

   /**
    * Destroys the HdfImporterShell object.
    */
   ~HdfImporterShell();

   /**
    *  Determines if the this pager can import ProcessingLocation::ON_DISK_READ_ONLY
    *  with this DataDescriptor.
    *
    *  This implementation checks the following criteria in the
    *  specified order:
    *  - Non-NULL data descriptor
    *  - Non-NULL file descriptor
    *  - No bands in separate files
    *  - The interleave matches the file
    *  - There is no row, column, or band subset.
    *
    *  @param   pDescriptor
    *           The data descriptor to query if it can be successfully
    *           imported.
    *  @param   errorMessage
    *           An error message that is populated with the reason why this importer
    *           cannot load the given data descriptor.
    *
    *  @return  Returns <b>true</b> if the default execute can successfully import
    *           the given data descriptor or the DataDescriptor is not 
    *           ProcessingLocation::ON_DISK_READ_ONLY; otherwise returns <b>false</b>.
    */
   bool validateDefaultOnDiskReadOnly(const DataDescriptor* pDescriptor, std::string& errorMessage) const;

protected:
   /**
    *  Based on the plug-in name and a reference to an RasterElement,
    *  creates the RasterPager Plug-In by calling the private pure virtual function
    *  createRasterPager(RasterElement*).
    *
    *  The dataset loaded is the one specified in the location specified by
    *  FileDescriptor::getDatasetLocation().
    *
    *  @param  pagerName
    *          The name of the RasterPager plug-in. Passed to PlugInManagerServices::createPlugIn().
    *  @param  raster
    *          A reference to the RasterElement for setting the plug-in.
    *
    *  @return TRUE if the operation succeeds. FALSE otherwise.
    */
   bool createRasterPagerPlugIn(const std::string& pagerName,
                                   RasterElement& raster) const;

   /**
    *  Creates an RasterPager Plug-In and sets it in the RasterElement.
    *
    *  This method can be overridden for importers that do not have a one-to-one correspondence between
    *  an HDF dataset and an RasterElement (ie. MODIS). Those that load one HDF dataset as a RasterElement
    *  can use the default implementation provided.
    *
    *  Called from createrRasterPagerPlugIn(const string&, const string&, RasterElement&).
    *
    *  @param  pRaster
    *          A pointer to the RasterElement for placing the raster pager plug-in.
    *
    *  @return TRUE if the operation succeeds, FALSE otherwise.
    */
   virtual bool createRasterPager(RasterElement *pRaster) const = 0;

protected:
   // This is mutable since it's an implementation detail
   // otherwise, any const member function that logs progress
   // will need to const_cast this
   mutable ProgressTracker mProgressTracker;
};

#endif
