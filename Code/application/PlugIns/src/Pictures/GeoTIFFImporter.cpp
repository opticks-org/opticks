/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "GeoTIFFImporter.h"
#include "AppConfig.h"
#include "AppVerify.h"
#include "DataAccessorImpl.h"
#include "DimensionDescriptor.h"
#include "DynamicObject.h"
#include "Endian.h"
#include "FileResource.h"
#include "ImportDescriptor.h"
#include "MessageLogResource.h"
#include "ModelServices.h"
#include "ObjectFactory.h"
#include "ObjectResource.h"
#include "OptionsTiffImporter.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "PlugInResource.h"
#include "QuickbirdIsd.h"
#include "RasterDataDescriptor.h"
#include "RasterFileDescriptor.h"
#include "RasterUtilities.h"
#include "SpatialDataView.h"

#include <stdio.h>

#include <geotiff.h>
#include <geovalues.h>
#include <geo_normalize.h>
#include <memory>
#include <vector>
#include <xtiffio.h>

#include <errno.h>
#include <QtCore/QFileInfo>
#include <QtCore/QString>
#include <QtCore/QVariant>
using namespace std;

REGISTER_PLUGIN_BASIC(OpticksPictures, GeoTIFFImporter);

namespace
{
   #if TIFFLIB_VERSION != 20060313
      // Unfortunately, the methods in this namespace are tightly coupled to the implementation of libtiff.
      // The crux of the problem is that there does not seem to be a reasonable way to get a TIFFTagValue from the API.
      // The next best option is to call TIFFGetField, which does not act the same for every tag.
      // Because of this, there are many special cases below which are meant to mimic the behavior of TIFFGetField.
      // When upgrading libtiff, check if a better method exists and use it if feasible.
      #error Check for libtiff compatibility.
   #endif

   bool setMetadata(const string& name, DataVariant& value, DynamicObject* pMetadata)
   {
      VERIFY(name.empty() == false && value.isValid() && pMetadata != NULL);
      return pMetadata->adoptAttributeByPath("TIFF/" + name, value);
   }

   template<typename T>
   bool setMetadata(const string& name, uint32 count, void* pValues, DynamicObject* pMetadata)
   {
      if (count == 0 || pValues == NULL)
      {
         return false;
      }

      if (count == 1)
      {
         DataVariant value(*reinterpret_cast<T*>(pValues));
         return setMetadata(name, value, pMetadata);
      }

      vector<T> values(count);
      memcpy(&values[0], pValues, count * sizeof(T));
      DataVariant value(values);
      return setMetadata(name, value, pMetadata);
   }

   template<>
   bool setMetadata<char>(const string& name, uint32 count, void* pValues, DynamicObject* pMetadata)
   {
      // Special case
      // TIFFTAG_INKNAMES is the only tag that should reach this function
      if (count == 0 || pValues == NULL)
      {
         return false;
      }

      char* pString = reinterpret_cast<char*>(pValues);
      for (unsigned int i = 0; i < count; ++i)
      {
         string valueStr(pString);
         DataVariant value(valueStr);
         if (setMetadata(name + QString("-%1").arg(i).toStdString(), value, pMetadata) == false)
         {
            return false;
         }

         pString = strchr(pString, NULL) + 1;
      }

      return true;
   }

   bool setMetadata(const TIFFFieldInfo* pFieldInfo, uint32 count, void* pValues, DynamicObject* pMetadata)
   {
      if (pFieldInfo == NULL || pMetadata == NULL)
      {
         return false;
      }

      const string name(pFieldInfo->field_name);
      switch (pFieldInfo->field_type)
      {
         case TIFF_UNDEFINED: // Fall through to match behavior in tif_dir.c:862
         case TIFF_BYTE:
         {
            return setMetadata<uint8>(name, count, pValues, pMetadata);
         }

         case TIFF_ASCII:
         {
            // Special case
            // Make this appear as a string rather than a vector<char>
            string value(reinterpret_cast<char*>(pValues));
            return setMetadata<string>(name, 1, &value, pMetadata);
         }

         case TIFF_SHORT:
         {
            return setMetadata<uint16>(name, count, pValues, pMetadata);
         }

         case TIFF_LONG: // Fall through to match behavior in tif_dir.c:884
         case TIFF_IFD:
         {
            return setMetadata<uint32>(name, count, pValues, pMetadata);
         }

         case TIFF_RATIONAL:  // Fall through to match behavior in tif_dir.c:894
         case TIFF_SRATIONAL: // Fall through to match behavior in tif_dir.c:895
         case TIFF_FLOAT:
         {
            return setMetadata<float>(name, count, pValues, pMetadata);
         }

         case TIFF_SBYTE:
         {
            return setMetadata<int8>(name, count, pValues, pMetadata);
         }

         case TIFF_SSHORT:
         {
            return setMetadata<int16>(name, count, pValues, pMetadata);
         }

         case TIFF_SLONG:
         {
            return setMetadata<int32>(name, count, pValues, pMetadata);
         }

         case TIFF_DOUBLE:
         {
            return setMetadata<double>(name, count, pValues, pMetadata);
         }

         case TIFF_NOTYPE: // Fall through
         default:
         {
            return false;
         }
      }
   }

   template<typename T>
   bool populateStaticTagMetadata(TIFF* pTiffFile, ttag_t tag,
      const uint32 count, DynamicObject* pMetadata, string& message)
   {
      VERIFY(pTiffFile != NULL && count > 0 && pMetadata != NULL);

      const TIFFFieldInfo* pFieldInfo = TIFFFieldWithTag(pTiffFile, tag);
      VERIFY(pFieldInfo != NULL);

      vector<T> values(count);
      if (TIFFGetField(pTiffFile, pFieldInfo->field_tag, &values[0]) == 0)
      {
         // The tag was not present in the file
         return false;
      }

      // Must call the templated version because some fields are internally inconsistent within libtiff
      // e.g.: TIFFTAG_BITSPERSAMPLE can be TIFF_LONG according to tif_dirinfo.c yet is used as a uint16 in tif_dir.c
      // While it is inefficient to make multiple copies of the vector, this code is only reached by static,
      // non-pointer tags which typically have no more than 2 elements.
      if (setMetadata<T>(pFieldInfo->field_name, count, &values[0], pMetadata) == false)
      {
         message += "Error processing " + string(pFieldInfo->field_name) + "\n";
         return false;
      }

      return true;
   }

   template<typename T>
   bool populateStaticTagMetadataArray1D(TIFF* pTiffFile, ttag_t tag,
      uint32 count, DynamicObject* pMetadata, string& message)
   {
      VERIFY(pTiffFile != NULL && pMetadata != NULL);

      const TIFFFieldInfo* pFieldInfo = TIFFFieldWithTag(pTiffFile, tag);
      VERIFY(pFieldInfo != NULL);

      T* pValue = NULL;
      if (count == 0)
      {
         // Special case
         // The only fields that need this are TIFFTAG_EXTRASAMPLES and TIFFTAG_SUBIFD (tif_dir.c:747, 782)
         // Both of these fields use a uint16 to store the count, so create a temporary uint16
         uint16 count16 = 0;
         if (TIFFGetField(pTiffFile, pFieldInfo->field_tag, &count16, &pValue) == 0)
         {
            // The tag was not present in the file
            return false;
         }

         count = static_cast<uint32>(count16);
      }
      else
      {
         if (TIFFGetField(pTiffFile, pFieldInfo->field_tag, &pValue) == 0)
         {
            // The tag was not present in the file
            return false;
         }
      }

      VERIFY(pValue != NULL && count > 0);

      // Must call the templated version because values must be copied into a vector to be stored in a DataVariant
      if (setMetadata<T>(pFieldInfo->field_name, count, pValue, pMetadata) == false)
      {
         message += "Error processing " + string(pFieldInfo->field_name) + "\n";
         return false;
      }

      return true;
   }

   template<typename T>
   bool populateStaticTagMetadataArray2D(TIFF* pTiffFile, ttag_t tag,
      const uint32 count, DynamicObject* pMetadata, string& message)
   {
      VERIFY(pTiffFile != NULL && count != 0 && pMetadata != NULL);
      const TIFFFieldInfo* pFieldInfo = TIFFFieldWithTag(pTiffFile, tag);
      VERIFY(pFieldInfo != NULL);

      // TIFFTAG_COLORMAP and TIFFTAG_TRANSFERFUNCTION have up to 3 children
      vector<T*> values(3, NULL);
      if (TIFFGetField(pTiffFile, pFieldInfo->field_tag, &values[0], &values[1], &values[2]) == 0)
      {
         // The tag was not present in the file
         return false;
      }

      for (uint32 i = 0; i < values.size() && values[i] != NULL; ++i)
      {
         // Must call the templated version because values must be copied into a vector to be stored in a DataVariant
         if (setMetadata<T>((QString(pFieldInfo->field_name) + QString("-%1").arg(i)).toStdString(),
            count, values[i], pMetadata) == false)
         {
            message += "Error processing " + string(pFieldInfo->field_name) + "\n";
            return false;
         }
      }

      return true;
   }

   bool populateCustomTagMetadata(TIFF* pTiffFile, ttag_t tag, DynamicObject* pMetadata, string& message)
   {
      VERIFY(pTiffFile != NULL && pMetadata != NULL);

      const TIFFFieldInfo* pFieldInfo = TIFFFieldWithTag(pTiffFile, tag);
      VERIFY(pFieldInfo != NULL);

      uint32 count = 0;
      void* pValues = NULL;
      if (pFieldInfo->field_passcount)
      {
         // Special case
         // Use either a uint16* or uint32* based on field_readcount as is done in tif_dir.c:837
         void* pCount = &count;
         uint16 count16 = 0;
         if (pFieldInfo->field_readcount != TIFF_VARIABLE2)
         {
            pCount = &count16;
         }

         if (TIFFGetField(pTiffFile, pFieldInfo->field_tag, pCount, &pValues) == 0)
         {
            // The tag was not present in the file even though libtiff earlier said it was present
            message += "Unable to retrieve information for " + string(pFieldInfo->field_name) + "\n";
            return false;
         }

         if (pFieldInfo->field_readcount != TIFF_VARIABLE2)
         {
            count = count16;
         }
      }
      else
      {
         // Special case
         // TIFFTAG_DOTRANGE requires either 2 or N arguments
         // Since it cannot be known how many arguments to send, display a warning
         if (pFieldInfo->field_tag == TIFFTAG_DOTRANGE)
         {
            message += "Custom field " + string(pFieldInfo->field_name) + " is not supported.\n";
            return false;
         }

         // The data types in this switch must match the ones in tif_dir.c:861
         switch (pFieldInfo->field_type)
         {
            case TIFF_ASCII:
            {
               // Special case
               // Make this appear as a string rather than a vector<char>
               count = 1;
               if (TIFFGetField(pTiffFile, pFieldInfo->field_tag, &pValues) == 0)
               {
                  return false;
               }

               break;
            }

            case TIFF_BYTE:   // Fall through
            case TIFF_UNDEFINED:
            {
               return populateStaticTagMetadata<uint8>(pTiffFile, tag,
                  pFieldInfo->field_readcount, pMetadata, message);
            }

            case TIFF_SBYTE:
            {
               return populateStaticTagMetadata<int8>(pTiffFile, tag,
                  pFieldInfo->field_readcount, pMetadata, message);
            }

            case TIFF_SHORT:
            {
               return populateStaticTagMetadata<uint16>(pTiffFile, tag,
                  pFieldInfo->field_readcount, pMetadata, message);
            }

            case TIFF_SSHORT:
            {
               return populateStaticTagMetadata<int16>(pTiffFile, tag,
                  pFieldInfo->field_readcount, pMetadata, message);
            }

            case TIFF_LONG:   // Fall through
            case TIFF_IFD:
            {
               return populateStaticTagMetadata<uint32>(pTiffFile, tag,
                  pFieldInfo->field_readcount, pMetadata, message);
            }

            case TIFF_SLONG:
            {
               return populateStaticTagMetadata<int32>(pTiffFile, tag,
                  pFieldInfo->field_readcount, pMetadata, message);
            }

            case TIFF_RATIONAL:  // Fall through
            case TIFF_SRATIONAL: // Fall through
            case TIFF_FLOAT:
            {
               return populateStaticTagMetadata<float>(pTiffFile, tag,
                  pFieldInfo->field_readcount, pMetadata, message);
            }

            case TIFF_DOUBLE:
            {
               return populateStaticTagMetadata<double>(pTiffFile, tag,
                  pFieldInfo->field_readcount, pMetadata, message);
            }

            default:
            {
               message += "Invalid tag type for custom field " + string(pFieldInfo->field_name) + "\n";
               return false;
            }
         }
      }

      if (setMetadata(pFieldInfo, count, pValues, pMetadata) == false)
      {
         message += "Error processing custom field " + string(pFieldInfo->field_name) + "\n";
         return false;
      }

      return true;
   }

   void populateTiffMetadata(TIFF* pTiffFile, DynamicObject* pMetadata, string& message)
   {
      message.clear();
      if (pTiffFile == NULL || pMetadata == NULL)
      {
         message += "Cannot populate metadata.\n";
         return;
      }

      // Static tags - these MUST match those called out in tif_dir.c:657-800
      populateStaticTagMetadata<uint32>(pTiffFile, TIFFTAG_SUBFILETYPE, 1, pMetadata, message);
      populateStaticTagMetadata<uint32>(pTiffFile, TIFFTAG_IMAGEWIDTH, 1, pMetadata, message);
      populateStaticTagMetadata<uint32>(pTiffFile, TIFFTAG_IMAGELENGTH, 1, pMetadata, message);
      populateStaticTagMetadata<uint16>(pTiffFile, TIFFTAG_BITSPERSAMPLE, 1, pMetadata, message);
      populateStaticTagMetadata<uint16>(pTiffFile, TIFFTAG_COMPRESSION, 1, pMetadata, message);
      populateStaticTagMetadata<uint16>(pTiffFile, TIFFTAG_PHOTOMETRIC, 1, pMetadata, message);
      populateStaticTagMetadata<uint16>(pTiffFile, TIFFTAG_THRESHHOLDING, 1, pMetadata, message);
      populateStaticTagMetadata<uint16>(pTiffFile, TIFFTAG_FILLORDER, 1, pMetadata, message);
      populateStaticTagMetadata<uint16>(pTiffFile, TIFFTAG_ORIENTATION, 1, pMetadata, message);
      populateStaticTagMetadata<uint16>(pTiffFile, TIFFTAG_SAMPLESPERPIXEL, 1, pMetadata, message);
      populateStaticTagMetadata<uint32>(pTiffFile, TIFFTAG_ROWSPERSTRIP, 1, pMetadata, message);
      populateStaticTagMetadata<uint16>(pTiffFile, TIFFTAG_MINSAMPLEVALUE, 1, pMetadata, message);
      populateStaticTagMetadata<uint16>(pTiffFile, TIFFTAG_MAXSAMPLEVALUE, 1, pMetadata, message);
      populateStaticTagMetadata<double>(pTiffFile, TIFFTAG_SMINSAMPLEVALUE, 1, pMetadata, message);
      populateStaticTagMetadata<double>(pTiffFile, TIFFTAG_SMAXSAMPLEVALUE, 1, pMetadata, message);
      populateStaticTagMetadata<float>(pTiffFile, TIFFTAG_XRESOLUTION, 1, pMetadata, message);
      populateStaticTagMetadata<float>(pTiffFile, TIFFTAG_YRESOLUTION, 1, pMetadata, message);
      populateStaticTagMetadata<uint16>(pTiffFile, TIFFTAG_PLANARCONFIG, 1, pMetadata, message);
      populateStaticTagMetadata<float>(pTiffFile, TIFFTAG_XPOSITION, 1, pMetadata, message);
      populateStaticTagMetadata<float>(pTiffFile, TIFFTAG_YPOSITION, 1, pMetadata, message);
      populateStaticTagMetadata<uint16>(pTiffFile, TIFFTAG_RESOLUTIONUNIT, 1, pMetadata, message);
      populateStaticTagMetadata<uint16>(pTiffFile, TIFFTAG_PAGENUMBER, 2, pMetadata, message);
      populateStaticTagMetadata<uint16>(pTiffFile, TIFFTAG_HALFTONEHINTS, 2, pMetadata, message);
      populateStaticTagMetadata<uint16>(pTiffFile, TIFFTAG_MATTEING, 1, pMetadata, message);
      populateStaticTagMetadata<uint32>(pTiffFile, TIFFTAG_TILEWIDTH, 1, pMetadata, message);
      populateStaticTagMetadata<uint32>(pTiffFile, TIFFTAG_TILELENGTH, 1, pMetadata, message);
      populateStaticTagMetadata<uint32>(pTiffFile, TIFFTAG_TILEDEPTH, 1, pMetadata, message);
      populateStaticTagMetadata<uint16>(pTiffFile, TIFFTAG_DATATYPE, 1, pMetadata, message);
      populateStaticTagMetadata<uint16>(pTiffFile, TIFFTAG_SAMPLEFORMAT, 1, pMetadata, message);
      populateStaticTagMetadata<uint32>(pTiffFile, TIFFTAG_IMAGEDEPTH, 1, pMetadata, message);
      populateStaticTagMetadata<uint16>(pTiffFile, TIFFTAG_YCBCRPOSITIONING, 1, pMetadata, message);
      populateStaticTagMetadata<uint16>(pTiffFile, TIFFTAG_YCBCRSUBSAMPLING, 2, pMetadata, message);

      // More static tags - each of these tags represents a special case in tif_dir.c:657-800

      // Special case
      // Either strips or tiles will be present, but not both
      // In either case, the number is obtained by calling the appropriate TIFFNumberOf function
      if (TIFFIsTiled(pTiffFile) == 0)
      {
         // Strips
         const tstrip_t numStrips(TIFFNumberOfStrips(pTiffFile));
         populateStaticTagMetadataArray1D<uint32>(pTiffFile, TIFFTAG_STRIPOFFSETS, numStrips, pMetadata, message);
         populateStaticTagMetadataArray1D<uint32>(pTiffFile, TIFFTAG_STRIPBYTECOUNTS, numStrips, pMetadata, message);
      }
      else
      {
         // Tiles
         const ttile_t numTiles(TIFFNumberOfTiles(pTiffFile));
         populateStaticTagMetadataArray1D<uint32>(pTiffFile, TIFFTAG_TILEOFFSETS, numTiles, pMetadata, message);
         populateStaticTagMetadataArray1D<uint32>(pTiffFile, TIFFTAG_TILEBYTECOUNTS, numTiles, pMetadata, message);
      }

      // Special case
      // Counts for TIFFTAG_EXTRASAMPLES and TIFFTAG_SUBIFD are obtained by calling TIFFGetField
      populateStaticTagMetadataArray1D<uint16>(pTiffFile, TIFFTAG_EXTRASAMPLES, 0, pMetadata, message);
      populateStaticTagMetadataArray1D<uint32>(pTiffFile, TIFFTAG_SUBIFD, 0, pMetadata, message);

      // Special case
      // TIFFTAG_COLORMAP and TIFFTAG_TRANSFERFUNCTION are uint16** with either 1 or 3 entries
      uint16 bitsPerElement = 0;
      TIFFGetField(pTiffFile, TIFFTAG_BITSPERSAMPLE, &bitsPerElement);
      populateStaticTagMetadataArray2D<uint16>(pTiffFile, TIFFTAG_COLORMAP,
         static_cast<uint32>(1 << bitsPerElement), pMetadata, message);

      populateStaticTagMetadataArray2D<uint16>(pTiffFile, TIFFTAG_TRANSFERFUNCTION, 
         static_cast<uint32>(1 << bitsPerElement), pMetadata, message);

      // Special case
      // TIFFTAG_INKNAMES is a char** to be read in as multiple NULL-terminated strings
      uint16 numberOfInks = 0;
      TIFFGetField(pTiffFile, TIFFTAG_NUMBEROFINKS, &numberOfInks);
      if (numberOfInks > 0)
      {
         if (numberOfInks > 1)
         {
            message += QString("%1 InkNames defined. Only the first will be read.\n").arg(numberOfInks).toStdString();
            numberOfInks = 1;
         }

         populateStaticTagMetadataArray1D<char>(pTiffFile, TIFFTAG_INKNAMES, numberOfInks, pMetadata, message);
      }

      // Iterate over each custom tag, placing it into the metadata
      const int numTags = TIFFGetTagListCount(pTiffFile);
      for (int i = 0; i < numTags; ++i)
      {
         populateCustomTagMetadata(pTiffFile, TIFFGetTagListEntry(pTiffFile, i), pMetadata, message);
      }
   }
}

GeoTIFFImporter::GeoTIFFImporter() : mImportOptionsWidget(NULL)
{
   setName("GeoTIFF Importer");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setExtensions("TIFF files (*.tif)");
   setShortDescription("TIFF");
   setDescriptorId("{F254DD8A-CF70-4835-B958-3E4FFD583E7F}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
   addDependencyCopyright("libgeotiff",
      "Permission is hereby granted, free of charge, to any person obtaining "
      "a copy of this software and associated documentation files (the "
      "\"Software\"), to deal in the Software without restriction, including "
      "without limitation the rights to use, copy, modify, merge, publish, "
      "distribute, sublicense, and/or sell copies of the Software, and to "
      "permit persons to whom the Software is furnished to do so, subject to "
      "the following conditions:<br>"
      "<br>"
      "The above copyright notice and this permission notice shall be "
      "included in all copies or substantial portions of the Software.");
   addDependencyCopyright("libtiff",
      "Copyright (c) 1988-1997 Sam Leffler<br>"
      "Copyright (c) 1991-1997 Silicon Graphics, Inc.<br>"
      "<br>"
      "Permission to use, copy, modify, distribute, and sell this software and "
      "its documentation for any purpose is hereby granted without fee, provided "
      "that (i) the above copyright notices and this permission notice appear in "
      "all copies of the software and related documentation, and (ii) the names of "
      "Sam Leffler and Silicon Graphics may not be used in any advertising or "
      "publicity relating to the software without the specific, prior written "
      "permission of Sam Leffler and Silicon Graphics.<br>"
      "<br>"
      "THE SOFTWARE IS PROVIDED \"AS-IS\" AND WITHOUT WARRANTY OF ANY KIND, "
      "EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY "
      "WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.<br>"
      "<br>"
      "IN NO EVENT SHALL SAM LEFFLER OR SILICON GRAPHICS BE LIABLE FOR "
      "ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, "
      "OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, "
      "WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF "
      "LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE "
      "OF THIS SOFTWARE.");
   addDependencyCopyright("proj4",
      "Copyright (c) 2000, Frank Warmerdam<br>"
      "<br>"
      "Permission is hereby granted, free of charge, to any person obtaining a "
      "copy of this software and associated documentation files (the \"Software\"), "
      "to deal in the Software without restriction, including without limitation "
      "the rights to use, copy, modify, merge, publish, distribute, sublicense, "
      "and/or sell copies of the Software, and to permit persons to whom the "
      "Software is furnished to do so, subject to the following conditions:<br>"
      "<br>"
      "The above copyright notice and this permission notice shall be included "
      "in all copies or substantial portions of the Software.<br>"
      "<br>"
      "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS "
      "OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, "
      "FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL "
      "THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER "
      "LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING "
      "FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER "
      "DEALINGS IN THE SOFTWARE.");
}

GeoTIFFImporter::~GeoTIFFImporter()
{
}

vector<ImportDescriptor*> GeoTIFFImporter::getImportDescriptors(const string& filename)
{
   vector<ImportDescriptor*> descriptors;
   if (filename.empty() == true)
   {
      return descriptors;
   }

   ImportDescriptor* pImportDescriptor = mpModel->createImportDescriptor(filename, "RasterElement", NULL);
   if (pImportDescriptor != NULL)
   {
      RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(pImportDescriptor->getDataDescriptor());
      if (pDescriptor != NULL)
      {
         // Create and set a file descriptor in the data descriptor
         FactoryResource<RasterFileDescriptor> pFileDescriptor;
         if (pFileDescriptor.get() != NULL)
         {
            pFileDescriptor->setFilename(filename);
            pDescriptor->setFileDescriptor(pFileDescriptor.get());
         }

         // Populate the data descriptor from the file
         bool bSuccess = populateDataDescriptor(pDescriptor);
         if (bSuccess == true)
         {
            descriptors.push_back(pImportDescriptor);
         }
         else
         {
            // Delete the import descriptor
            mpModel->destroyImportDescriptor(pImportDescriptor);
         }
      }
   }

   return descriptors;
}

unsigned char GeoTIFFImporter::getFileAffinity(const std::string& filename)
{
   if (getImportDescriptors(filename).empty())
   {
      return Importer::CAN_NOT_LOAD;
   }
   else
   {
      return Importer::CAN_LOAD;
   }
}

bool GeoTIFFImporter::populateDataDescriptor(RasterDataDescriptor* pDescriptor)
{
   if (pDescriptor == NULL)
   {
      return false;
   }

   RasterFileDescriptor* pFileDescriptor = dynamic_cast<RasterFileDescriptor*>
      (pDescriptor->getFileDescriptor());
   if (pFileDescriptor == NULL)
   {
      return false;
   }

   const string& filename = pFileDescriptor->getFilename();
   if (filename.empty() == true)
   {
      return false;
   }

   {
      // Check the first four bytes for TIFF magic number

      //force file to be closed when scope block ends
      FileResource pFile(filename.c_str(), "r");
      if (pFile.get() != NULL)
      {
         const unsigned short tiffBigEndianMagicNumber = 0x4d4d;
         const unsigned short tiffLittleEndianMagicNumber = 0x4949;
         const unsigned short tiffVersionMagicNumber = 42;

         unsigned short fileEndian;
         fread(&fileEndian, sizeof(fileEndian), 1, pFile);

         if ( (fileEndian == tiffBigEndianMagicNumber) || (fileEndian == tiffLittleEndianMagicNumber) )
         {
            unsigned short tiffVersion;
            fread(&tiffVersion, sizeof(tiffVersion), 1, pFile);

            EndianType fileEndianType = (fileEndian == tiffBigEndianMagicNumber ? BIG_ENDIAN_ORDER : LITTLE_ENDIAN_ORDER);
            Endian swapper(fileEndianType);
            swapper.swapBuffer(&tiffVersion, 1);

            if (tiffVersion != tiffVersionMagicNumber)
            {
               return false;
            }
            pFileDescriptor->setEndian(fileEndianType);
         }
         else
         {
            return false;
         }
      }
   }

   TIFF* pTiffFile = XTIFFOpen(filename.c_str(), "r");
   if (pTiffFile == NULL)
   {
      return false;
   }

   // Copy metadata
   populateTiffMetadata(pTiffFile, pDescriptor->getMetadata(), mMetadataMessage);

   // Check for unsupported palette data
   unsigned short photometric = 0;
   TIFFGetField(pTiffFile, TIFFTAG_PHOTOMETRIC, &photometric);

   if (photometric == PHOTOMETRIC_PALETTE)
   {
      XTIFFClose(pTiffFile);
      return false;
   }

   // Rows
   unsigned int numRows = 0;
   TIFFGetField(pTiffFile, TIFFTAG_IMAGELENGTH, &numRows);

   vector<DimensionDescriptor> rows = RasterUtilities::generateDimensionVector(numRows, true, false, true);
   pDescriptor->setRows(rows);
   pFileDescriptor->setRows(rows);

   // Columns
   unsigned int numColumns = 0;
   TIFFGetField(pTiffFile, TIFFTAG_IMAGEWIDTH, &numColumns);

   vector<DimensionDescriptor> columns = RasterUtilities::generateDimensionVector(numColumns, true, false, true);

   pDescriptor->setColumns(columns);
   pFileDescriptor->setColumns(columns);

   // Bands
   unsigned short numBands = 1;
   TIFFGetField(pTiffFile, TIFFTAG_SAMPLESPERPIXEL, &numBands);

   vector<DimensionDescriptor> bands = RasterUtilities::generateDimensionVector(numBands, true, false, true);

   pDescriptor->setBands(bands);
   pFileDescriptor->setBands(bands);

   // Bits per pixel
   unsigned short bitsPerElement = 0;
   TIFFGetField(pTiffFile, TIFFTAG_BITSPERSAMPLE, &bitsPerElement);

   pFileDescriptor->setBitsPerElement(bitsPerElement);

   // Data type
   unsigned short sampleFormat = SAMPLEFORMAT_VOID;
   TIFFGetField(pTiffFile, TIFFTAG_SAMPLEFORMAT, &sampleFormat);

   EncodingType dataType = INT1UBYTE;

   unsigned int bytesPerElement = bitsPerElement / 8;
   switch (bytesPerElement)
   {
      case 1:
         if (sampleFormat == SAMPLEFORMAT_INT)
         {
            dataType = INT1SBYTE;
         }
         else
         {
            dataType = INT1UBYTE;
         }
         break;

      case 2:
         if (sampleFormat == SAMPLEFORMAT_INT)
         {
            dataType = INT2SBYTES;
         }
         else
         {
            dataType = INT2UBYTES;
         }
         break;

      case 4:
         if (sampleFormat == SAMPLEFORMAT_INT)
         {
            dataType = INT4SBYTES;
         }
         else if (sampleFormat == SAMPLEFORMAT_IEEEFP)
         {
            dataType = FLT4BYTES;
         }
         else
         {
            dataType = INT4UBYTES;
         }
         break;

      case 8:
         dataType = FLT8BYTES;
         break;

      default:
         break;
   }

   pDescriptor->setDataType(dataType);

   // Interleave format
   unsigned short planarConfig = 0;
   TIFFGetField(pTiffFile, TIFFTAG_PLANARCONFIG, &planarConfig);

   if (planarConfig == PLANARCONFIG_SEPARATE)
   {
      pFileDescriptor->setInterleaveFormat(BSQ);
   }
   else if (planarConfig == PLANARCONFIG_CONTIG)
   {
      pFileDescriptor->setInterleaveFormat(BIP);
   }

   pDescriptor->setInterleaveFormat(BIP);

   // Bad values
   if ((dataType != FLT4BYTES) && (dataType != FLT8COMPLEX) && (dataType != FLT8BYTES))
   {
      std::vector<int> badValues(1);
      badValues[0] = 0;

      pDescriptor->setBadValues(badValues);
   }

   // Latitude/longitude GCPs
   GTIF* pGeoTiff = GTIFNew(pTiffFile);

   GTIFDefn defn;
   GTIFGetDefn(pGeoTiff, &defn);

   char* pProj4Defn = GTIFGetProj4Defn(&defn);
   if (pProj4Defn != NULL)
   {
      list<GcpPoint> gcps;

      // Upper left
      double x = 0;
      double y = 0;
      if (GTIFImageToPCS(pGeoTiff, &x, &y))
      {
         if (defn.Model != ModelTypeGeographic)
         {
            GTIFProj4ToLatLong(&defn, 1, &x, &y);
         }

         GcpPoint gcp;
         gcp.mPixel.mX = 0.0;
         gcp.mPixel.mY = 0.0;
         gcp.mCoordinate.mX = y;
         gcp.mCoordinate.mY = x;
         gcps.push_back(gcp);
      }

      // Lower left
      x = 0;
      y = numRows - 1;
      if (GTIFImageToPCS(pGeoTiff, &x, &y))
      {
         if (defn.Model != ModelTypeGeographic)
         {
            GTIFProj4ToLatLong(&defn, 1, &x, &y);
         }

         GcpPoint gcp;
         gcp.mPixel.mX = 0.0;
         gcp.mPixel.mY = numRows - 1.0;
         gcp.mCoordinate.mX = y;
         gcp.mCoordinate.mY = x;
         gcps.push_back(gcp);
      }

      // Upper right
      x = numColumns - 1;
      y = 0;
      if (GTIFImageToPCS(pGeoTiff, &x, &y))
      {
         if (defn.Model != ModelTypeGeographic)
         {
            GTIFProj4ToLatLong(&defn, 1, &x, &y);
         }

         GcpPoint gcp;
         gcp.mPixel.mX = numColumns - 1.0;
         gcp.mPixel.mY = 0.0;
         gcp.mCoordinate.mX = y;
         gcp.mCoordinate.mY = x;
         gcps.push_back(gcp);
      }

      // Lower right
      x = numColumns - 1;
      y = numRows - 1;
      if (GTIFImageToPCS(pGeoTiff, &x, &y))
      {
         if (defn.Model != ModelTypeGeographic)
         {
            GTIFProj4ToLatLong(&defn, 1, &x, &y);
         }

         GcpPoint gcp;
         gcp.mPixel.mX = numColumns - 1.0;
         gcp.mPixel.mY = numRows - 1.0;
         gcp.mCoordinate.mX = y;
         gcp.mCoordinate.mY = x;
         gcps.push_back(gcp);
      }

      // Center
      x = (numColumns - 1) / 2;
      y = (numRows - 1) / 2;
      if (GTIFImageToPCS(pGeoTiff, &x, &y))
      {
         if (defn.Model != ModelTypeGeographic)
         {
            GTIFProj4ToLatLong(&defn, 1, &x, &y);
         }

         GcpPoint gcp;
         gcp.mPixel.mX = (numColumns - 1.0) / 2.0;
         gcp.mPixel.mY = (numRows - 1.0) / 2.0;
         gcp.mCoordinate.mX = y;
         gcp.mCoordinate.mY = x;
         gcps.push_back(gcp);
      }

      if (gcps.empty() == false)
      {
         pFileDescriptor->setGcps(gcps);
      }
   }

   // Close the TIFF file
   XTIFFClose(pTiffFile);

   return true;
}

bool GeoTIFFImporter::validateDefaultOnDiskReadOnly(const DataDescriptor* pDescriptor, string& errorMessage) const
{
   const RasterDataDescriptor* pRasterDescriptor = dynamic_cast<const RasterDataDescriptor*>(pDescriptor);
   if (pRasterDescriptor == NULL)
   {
      errorMessage = "The data descriptor is invalid!";
      return false;
   }
   if (pDescriptor->getProcessingLocation() != ON_DISK_READ_ONLY)
   {
      // this method only checks GeoTIFF's on-disk read-only
      return true;
   }

   const RasterFileDescriptor* pFileDescriptor =
      dynamic_cast<const RasterFileDescriptor*>(pRasterDescriptor->getFileDescriptor());
   if (pFileDescriptor == NULL)
   {
      errorMessage = "The file descriptor is invalid!";
      return false;
   }

   // Multiple band files
   const vector<const Filename*>& bandFiles = pFileDescriptor->getBandFiles();
   if (bandFiles.empty() == false)
   {
      errorMessage = "Bands in multiple files are not supported!";
      return false;
   }

   // Interleave format
   InterleaveFormatType fileInterleave = pFileDescriptor->getInterleaveFormat();
   InterleaveFormatType dataInterleave = pRasterDescriptor->getInterleaveFormat();

   // Processing location restrictions
   ProcessingLocation processingLocation = pRasterDescriptor->getProcessingLocation();
   if (processingLocation == ON_DISK_READ_ONLY)
   {
      // Interleave conversions
      if (dataInterleave != fileInterleave)
      {
         errorMessage = "Interleave format conversions are not supported with on-disk read-only processing!";
         return false;
      }

      // Subset
      unsigned int loadedRows = pRasterDescriptor->getRowCount();
      unsigned int loadedColumns = pRasterDescriptor->getColumnCount();
      unsigned int loadedBands = pRasterDescriptor->getBandCount();
      unsigned int fileRows = pFileDescriptor->getRowCount();
      unsigned int fileColumns = pFileDescriptor->getColumnCount();
      unsigned int fileBands = pFileDescriptor->getBandCount();

      if ((loadedRows != fileRows) || (loadedColumns != fileColumns) || (loadedBands != fileBands))
      {
         errorMessage = "Subsets are not supported with on-disk read-only processing!";
         return false;
      }
   }

   return true;
}

bool GeoTIFFImporter::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   if (pInArgList == NULL)
   {
      return false;
   }

   // Create a message log step
   StepResource pStep("Execute GeoTIFF Importer", "app", "76793666-5219-499f-9d2c-8accc11b32fc", "Execute failed");

   // Extract the input args
   bool bSuccess = parseInputArgList(pInArgList);
   if (!bSuccess)
   {
      return false;
   }

   // Update the log and progress with the start of the import
   Progress* pProgress = getProgress();
   if (pProgress != NULL)
   {
      pProgress->updateProgress("GeoTIFF Importer Started", 1, NORMAL);
   }

   loadIsdMetadata(getRasterElement()->getDataDescriptor());
   if (!performImport())
   {
      return false;
   }

   // Create the view
   if (!isBatch() && !Service<SessionManager>()->isSessionLoading())
   {
      SpatialDataView* pView = createView();
      if (pView == NULL)
      {
         pStep->finalize(Message::Failure, "The view could not be created.");
         return false;
      }

      // Add the view to the output arg list
      if (pOutArgList != NULL)
      {
         pOutArgList->setPlugInArgValue("View", pView);
      }
   }

   if (pProgress != NULL)
   {
      pProgress->updateProgress("GeoTIFF Import Complete.", 100, NORMAL);
   }

   pStep->finalize(Message::Success);
   return true;
}

QWidget *GeoTIFFImporter::getImportOptionsWidget(DataDescriptor *pDescriptor)
{
   if (mImportOptionsWidget.get() == NULL)
   {
      QString initialDirectory;
      QString isdFilename;
      const FileDescriptor* pFileDescriptor = (pDescriptor == NULL) ? NULL : pDescriptor->getFileDescriptor();
      if (pFileDescriptor != NULL)
      {
         initialDirectory = QString::fromStdString(pFileDescriptor->getFilename().getPath());
         QFileInfo tiffInfo(QString::fromStdString(pFileDescriptor->getFilename().getFullPathAndName()));
         QFileInfo isdInfo(tiffInfo.absolutePath() + "/" + tiffInfo.completeBaseName() + ".xml");
         if (isdInfo.exists())
         {
            isdFilename = isdInfo.absoluteFilePath();
         }
      }

      OptionsTiffImporter* pWidget = new OptionsTiffImporter(initialDirectory);
      mImportOptionsWidget.reset(pWidget);
      mImportOptionsWidget->setFilename(isdFilename);
   }
   return mImportOptionsWidget.get();
}

void GeoTIFFImporter::loadIsdMetadata(DataDescriptor *pDescriptor)
{
   QString isdFilename;
   if (mImportOptionsWidget.get() != NULL)
   {
      isdFilename = mImportOptionsWidget->getFilename();
   }
   else
   {
      const FileDescriptor* pFileDescriptor = (pDescriptor == NULL) ? NULL : pDescriptor->getFileDescriptor();
      if (pFileDescriptor != NULL)
      {
         QFileInfo tiffInfo(QString::fromStdString(pFileDescriptor->getFilename().getFullPathAndName()));
         QFileInfo isdInfo(tiffInfo.absolutePath() + "/" + tiffInfo.completeBaseName() + ".xml");
         if (isdInfo.exists())
         {
            isdFilename = isdInfo.absoluteFilePath();
         }
      }
   }
   if (isdFilename.isEmpty())
   {
      // don't load any ISD metadata
      return;
   }
   StepResource pStep("Load ISD Metadata", "app", "06b70af8-7ba5-43d6-8a92-826731da7a81");
   QFileInfo isdInfo(isdFilename);
   if (!isdInfo.isFile() || !isdInfo.exists())
   {
      string message = "ISD metadata file " + isdFilename.toStdString() +
         " does not exist.\nMetadata will not be loaded.";
      if (getProgress() != NULL)
      {
         getProgress()->updateProgress(message, 0, WARNING);
      }
      pStep->finalize(Message::Failure, message);
      return;
   }
   QuickbirdIsd isd(isdFilename.toStdString());
   if (!isd.copyToMetadata(pDescriptor->getMetadata()))
   {
      string message = "Unable to parse ISD metadata file " + isdFilename.toStdString() +
         ".\nMetadata will not be loaded.";
      if (getProgress() != NULL)
      {
         getProgress()->updateProgress(message, 0, WARNING);
      }
      pStep->finalize(Message::Failure, message);
   }
   else
   {
      pStep->finalize(Message::Success);
   }
}

bool GeoTIFFImporter::createRasterPager(RasterElement *pRasterElement) const
{
   VERIFY(pRasterElement != NULL);
   DataDescriptor* pDescriptor = pRasterElement->getDataDescriptor();
   VERIFY(pDescriptor != NULL);
   FileDescriptor* pFileDescriptor = pDescriptor->getFileDescriptor();
   VERIFY(pFileDescriptor != NULL);

   const string& filename = pRasterElement->getFilename();
   Progress* pProgress = getProgress();

   StepResource pStep("Create pager for GeoTIFF", "app", "AF6176CD-5B39-4CED-A92E-394E3CD1CD00");

   FactoryResource<Filename> pFilename;
   pFilename->setFullPathAndName(filename);

   ExecutableResource pagerPlugIn("GeoTiffPager", string(), pProgress);
   pagerPlugIn->getInArgList().setPlugInArgValue("Raster Element", pRasterElement);
   pagerPlugIn->getInArgList().setPlugInArgValue("Filename", pFilename.get());
   bool success = pagerPlugIn->execute();

   RasterPager* pPager = dynamic_cast<RasterPager*>(pagerPlugIn->getPlugIn());
   if ((pPager == NULL) || (success == false))
   {
      string message = "Execution of GeoTiffPager failed!";
      if (pProgress != NULL)
      {
         pProgress->updateProgress(message, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, message);
      return false;
   }

   pRasterElement->setPager(pPager);
   pagerPlugIn->releasePlugIn();

   pStep->finalize();
   return true;
}

bool GeoTIFFImporter::validate(const DataDescriptor* pDescriptor, std::string& errorMessage) const
{
   if (RasterElementImporterShell::validate(pDescriptor, errorMessage) == false)
   {
      return false;
   }

   const RasterDataDescriptor* pRasterDescriptor =
      dynamic_cast<const RasterDataDescriptor*>(pDescriptor);
   VERIFY(pRasterDescriptor != NULL);

   const RasterFileDescriptor* pRasterFileDescriptor =
      dynamic_cast<const RasterFileDescriptor*>(pRasterDescriptor->getFileDescriptor());
   VERIFY(pRasterFileDescriptor != NULL);
   if (pRasterFileDescriptor->getInterleaveFormat() == BIL)
   {
      errorMessage = "BIL interleave method not supported";
      return false;
   }

   errorMessage += mMetadataMessage;
   return true;
}
