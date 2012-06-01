/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef NITFIMPORTERSHELL_H
#define NITFIMPORTERSHELL_H

#include "RasterElementImporterShell.h"

#include <ossim/base/ossimConstants.h>
#include <string>
#include <map>

class ossimNitfFile;
class ossimNitfFileHeaderV2_X;
class ossimNitfImageHeaderV2_X;

namespace Nitf
{
   /**
    * Base class for NITF importers.
    */
   class NitfImporterShell : public RasterElementImporterShell
   {
   public:
      /**
       *  Creates the NITF importer plug-in.
       *
       *  @see RasterElementImporterShell
       */
      NitfImporterShell();

      /**
       *  Destroys the NITF importer plug-in.
       */
      virtual ~NitfImporterShell();

      /**
       * @copydoc RasterElementImporterShell::getImportDescriptors()
       *
       * @default The default implementation returns image segments which can be imported by this importer.
       */
      virtual std::vector<ImportDescriptor*> getImportDescriptors(const std::string& filename);

      /**
       * @copydoc RasterElementImporterShell::getFileAffinity()
       *
       * @default The default implementation returns Importer::CAN_LOAD for all NITF 2.00 and NITF 2.10 files.
       */
      virtual unsigned char getFileAffinity(const std::string& filename);

      /**
       * @copydoc RasterElementImporterShell::validate()
       * \par
       * The default implementation also returns warnings when lookup tables (LUTs) or other issues are detected.
       */
      virtual bool validate(const DataDescriptor* pDescriptor, std::string& errorMessage) const;

      /**
       * @copydoc RasterElementImporterShell::createView()
       *
       * @default The default implementation sets the background color.
       */
      virtual SpatialDataView* createView() const;

      /**
       * @copydoc RasterElementImporterShell::createRasterPager()
       *
       * @default The default implementation creates an instance of the internal "NitfPager" plug-in.
       */
      virtual bool createRasterPager(RasterElement* pRaster) const;

      /**
       * Return the data type of the specified image.
       *
       * @param pImgHeader
       *        The parsed subheader for this image segment.
       *
       * @return The EncodingType to use for the specified image header.
       */
      static EncodingType ossimImageHeaderToEncodingType(ossimNitfImageHeaderV2_X* pImgHeader);

   protected:
      /**
       *  @copydoc ImporterShell::getValidationTest()
       *
       *  \par
       *  The following raster-specific tests are added to the base class defaults
       *  listed above:
       *  - \link ImporterShell::VALID_CLASSIFICATION VALID_CLASSIFICATION \endlink
       *  - \link ImporterShell::RASTER_SIZE RASTER_SIZE \endlink
       *  - \link ImporterShell::VALID_DATA_TYPE VALID_DATA_TYPE \endlink
       *  - \link ImporterShell::VALID_METADATA VALID_METADATA \endlink
       *  .
       *  \par
       *  Additionally, the following test is added if the ::ProcessingLocation is
       *  ::IN_MEMORY:
       *  - \link ImporterShell::AVAILABLE_MEMORY AVAILABLE_MEMORY \endlink
       *  .
       *  \par
       *  The following tests are added if the ::ProcessingLocation is
       *  ::ON_DISK_READ_ONLY:
       *  - \link ImporterShell::NO_INTERLEAVE_CONVERSIONS NO_INTERLEAVE_CONVERSIONS \endlink
       *  - \link ImporterShell::NO_SUBSETS NO_SUBSETS \endlink
       *  - \link ImporterShell::NO_SKIP_FACTORS NO_SKIP_FACTORS \endlink
       *  - \link ImporterShell::NO_BAND_FILES NO_BAND_FILES \endlink
       *  .
       *  \par
       *  The following tests are added if the ::InterleaveFormatType is not ::BSQ:
       *  - \link ImporterShell::NO_PRE_POST_BAND_BYTES NO_PRE_POST_BAND_BYTES \endlink
       *  - \link ImporterShell::NO_BAND_FILES NO_BAND_FILES \endlink
       *  .
       *  \par
       *  The following test is added if multiple band files are present:
       *  - \link ImporterShell::EXISTING_BAND_FILES EXISTING_BAND_FILES \endlink
       *  .
       */
      virtual int getValidationTest(const DataDescriptor* pDescriptor) const;

      /**
       * Allocate and return an ImportDescriptor for the specified image. This method should allocate an
       * ImportDescriptor (if desired) and set whether it is imported by default. This method should not set any fields
       * in the data descriptor or the file descriptor of the import descriptor. If changes to the defaults are
       * required, the subclass should override getImportDescriptors(), call the base class implementation, and then
       * make the necessary changes.
       *
       * @param filename
       *        The full path and name of the file.
       *
       * @param imageName
       *        The name which will be used for image identification. This is a string of the form "I1", "I2", etc.
       *        Using this name for the import descriptor is optional. This name will be used for the default pager and
       *        image segment.
       *
       * @param pFile
       *        The NITF file being loaded.
       *
       * @param pFileHeader
       *        The parsed header for this NITF file.
       *
       * @param pImageSubheader
       *        The parsed subheader for this image segment.
       *
       * @return A pointer to a newly created ImportDescriptor or \c NULL if no ImportDescriptor should be created.
       *
       * @see getImportDescriptors()
       *
       * @default The default implementation returns a descriptor marked as
       *          imported for all image segments except those marked as NODISPLY.
       */
      virtual ImportDescriptor* getImportDescriptor(const std::string& filename, const std::string& imageName,
         const ossimNitfFile* pFile, const ossimNitfFileHeaderV2_X* pFileHeader,
         const ossimNitfImageHeaderV2_X* pImageSubheader);

   private:
      std::map<std::string, std::string> mParseMessages;
   };
}
#endif
