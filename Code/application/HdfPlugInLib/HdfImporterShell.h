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

#include "ProgressTracker.h"
#include "RasterElementImporterShell.h"

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
   virtual ~HdfImporterShell();

   /**
    *  @copydoc RasterElementImporterShell::validate()
    */
   virtual bool validate(const DataDescriptor* pDescriptor, std::string& errorMessage) const;

protected:
   /**
    *  @copydoc RasterElementImporterShell::getValidationTest()
    *
    *  \par
    *  The following tests are added if the ::ProcessingLocation is
    *  ::ON_DISK_READ_ONLY:
    *  - \link ImporterShell::NO_BAND_FILES NO_BAND_FILES \endlink
    *  - \link ImporterShell::NO_ROW_SUBSETS NO_ROW_SUBSETS \endlink
    *  - \link ImporterShell::NO_COLUMN_SUBSETS NO_COLUMN_SUBSETS \endlink
    *
    *  \par
    *  The following test is removed if the ::ProcessingLocation is
    *  ::ON_DISK_READ_ONLY and the ::InterleaveFormatType is ::BSQ:
    *  - \link ImporterShell::NO_BAND_SUBSETS NO_BAND_SUBSETS \endlink
    */
   virtual int getValidationTest(const DataDescriptor* pDescriptor) const;

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
