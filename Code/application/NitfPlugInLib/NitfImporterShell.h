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

#include "NitfResource.h"
#include "RasterElementImporterShell.h"

#include <openjpeg.h>
#include <ossim/base/ossimConstants.h>

#include <map>
#include <string>

class ossimNitfFileHeaderV2_X;
class ossimNitfImageHeaderV2_X;

namespace Nitf
{
   /**
    *  Base class for NITF importers.
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
       *  @copydoc RasterElementImporterShell::getImportDescriptors()
       *
       *  @default The default implementation iterates over all image segments
       *           found in the NITF file and returns all valid import
       *           descriptors returned by getImportDescriptor() for each image
       *           segment.
       */
      virtual std::vector<ImportDescriptor*> getImportDescriptors(const std::string& filename);

      /**
       *  @copydoc RasterElementImporterShell::getFileAffinity()
       *
       *  @default The default implementation checks for a NITF file header and
       *           returns Importer::CAN_LOAD for all NITF 2.00 and NITF 2.10
       *           files.
       */
      virtual unsigned char getFileAffinity(const std::string& filename);

      /**
       *  @copydoc RasterElementImporterShell::validate()
       *
       *  \par
       *  The default implementation also adds warnings to \em errorMessage when
       *  lookup tables (LUTs) or other issues are detected.
       */
      virtual bool validate(const DataDescriptor* pDescriptor,
         const std::vector<const DataDescriptor*>& importedDescriptors, std::string& errorMessage) const;

      /**
       *  @copydoc RasterElementImporterShell::createView()
       *
       *  @default The default implementation calls the base class
       *           implementation and sets the view background color based on
       *           the value of the File Background Color (FBKGC) field of the
       *           NITF file header.
       */
      virtual SpatialDataView* createView() const;

      /**
       *  @copydoc RasterElementImporterShell::createRasterPager()
       *
       *  @default The default implementation creates an instance of the
       *           "NitfPager" plug-in or the "JPEG2000 Pager" plug-in if the
       *           value in the Image Compression (IC) field of the image
       *           subheader is "C8" or "M8", representing JPEG2000 compression.
       */
      virtual bool createRasterPager(RasterElement* pRaster) const;

      /**
       *  Returns the data type of the specified image segment.
       *
       *  @param   pImgHeader
       *           The parsed subheader for this image segment.
       *
       *  @return  The ::EncodingType to use for the specified image header.
       */
      static EncodingType ossimImageHeaderToEncodingType(const ossimNitfImageHeaderV2_X* pImgHeader);

   protected:
      /**
       *  @copydoc RasterElementImporterShell::getValidationTest()
       *
       *  \par
       *  The following test is added to the base class defaults listed above:
       *  - \link ImporterShell::VALID_METADATA VALID_METADATA \endlink
       *  .
       *  \par
       *  The following tests are added if the ::ProcessingLocation is
       *  ::ON_DISK_READ_ONLY and the value of the Image Compression (IC) field
       *  in the NITF image subheader is not "C8" or "M8" (JPEG2000 compression):
       *  - \link ImporterShell::NO_BAND_FILES NO_BAND_FILES \endlink
       *  - \link ImporterShell::NO_SUBSETS NO_SUBSETS \endlink
       */
      virtual int getValidationTest(const DataDescriptor* pDescriptor) const;

      /**
       *  Allocates and returns an ImportDescriptor for a specified image
       *  segment in a NITF file.
       *
       *  This allocates an ImportDescriptor based on the information contained
       *  in the NITF file header and image subheader, and sets whether the data
       *  set is imported by default.  This method fully populates all fields in
       *  the data descriptor and file descriptor contained in the import
       *  descriptor.  If changes to the defaults are required, the subclass
       *  should override this method instead of getImportDescriptors(), call
       *  this base class implementation, and then make the necessary changes.
       *
       *  @param   filename
       *           The full path and name of the NITF file.
       *  @param   imageSegment
       *           The zero-based index value of the image segment in the NITF
       *           file for which to get an import descriptor.
       *  @param   pFile
       *           The NITF file being loaded.
       *  @param   pFileHeader
       *           The parsed file header for this NITF file.
       *  @param   pImageSubheader
       *           The parsed image subheader for this image segment.
       *
       *  @return  Returns a pointer to a newly created ImportDescriptor or
       *           \c NULL if an import descriptor cannot be created for the
       *           given image segment.  A valid import descriptor will be
       *           returned even if the data set cannot be successfully loaded
       *           due to unsupported content.  If this occurs, the validate()
       *           method will return \c false for this image segment.
       *
       *  @see     getImportDescriptors()
       *
       *  @default The default implementation returns a fully populated import
       *           descriptor for the given image segment.  If the value of the
       *           Image Representation (IREP) field in the image subheader is
       *           something other than "NODISPLY", the import descriptor is set
       *           to be imported by default.
       */
      virtual ImportDescriptor* getImportDescriptor(const std::string& filename, ossim_uint32 imageSegment,
         const Nitf::OssimFileResource& pFile, const ossimNitfFileHeaderV2_X* pFileHeader,
         const ossimNitfImageHeaderV2_X* pImageSubheader);

      /**
       *  Returns the offset in bytes to the beginning of the data of a given
       *  image segment.
       *
       *  @param   filename
       *           The name of the NITF file containing the image segment for
       *           which to get the offset to its data.
       *  @param   imageSegment
       *           The zero-based index value of the image segment in the NITF
       *           file for which to get the offset to its data.
       *
       *  @return  The offset in bytes of the given file to the beginning of the
       *           given image segment data.  A value of 0 is returned if the
       *           given file is not a valid NITF file, or if the given image
       *           segment does not exist in the file.
       *
       *  @see     getImageSize()
       */
      uint64_t getImageOffset(const std::string& filename, unsigned int imageSegment) const;

      /**
       *  Returns the file size in bytes of the data for a given image segment.
       *
       *  @param   filename
       *           The name of the NITF file containing the image segment for
       *           which to get the size of its data.
       *  @param   imageSegment
       *           The zero-based index value of the image segment in the NITF
       *           file for which to get the size of its data.
       *
       *  @return  The size in bytes of the given image segment data as stored
       *           in the given NITF file.  A value of 0 is returned if the
       *           given file is not a valid NITF file, or if the given image
       *           segment does not exist in the file.
       *
       *  @see     getImageOffset()
       */
      uint64_t getImageSize(const std::string& filename, unsigned int imageSegment) const;

   private:
      opj_image_t* getImageInfo(const std::string& filename, unsigned int imageSegment,
         OPJ_CODEC_FORMAT decoderType) const;

      std::map<ossim_uint32, std::string> mParseMessages;
   };
}
#endif
