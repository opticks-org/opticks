/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "AppVersion.h"
#include "DimensionDescriptor.h"
#include "DynamicObject.h"
#include "Endian.h"
#include "FileResource.h"
#include "GeoTIFFImporter.h"
#include "ImportDescriptor.h"
#include "MessageLogResource.h"
#include "ObjectResource.h"
#include "OptionsTiffImporter.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInRegistration.h"
#include "PlugInResource.h"
#include "QuickbirdIsd.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterFileDescriptor.h"
#include "RasterPager.h"
#include "RasterUtilities.h"
#include "UtilityServices.h"

#include <QtCore/QFileInfo>
#include <QtCore/QString>

#include <errno.h>
#include <geotiff.h>
#include <geovalues.h>
#include <geo_normalize.h>
#include <memory>
#include <stdio.h>
#include <vector>
#include <xtiffio.h>

using namespace std;

REGISTER_PLUGIN_BASIC(OpticksPictures, GeoTIFFImporter);

namespace
{
   #if TIFFLIB_VERSION != 20100615
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
      // TIFFTAG_REFERENCEBLACKWHITE is float* with exactly 6 entries
      populateStaticTagMetadataArray1D<float>(pTiffFile, TIFFTAG_REFERENCEBLACKWHITE, 6, pMetadata, message);

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

GeoTIFFImporter::GeoTIFFImporter() :
   mpImportOptionsWidget(NULL)
{
   setName("GeoTIFF Importer");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setDescription("Imports TIFF and GeoTIFF formatted files.");
   setExtensions("TIFF files (*.tif *.tiff)");
   setDescriptorId("{F254DD8A-CF70-4835-B958-3E4FFD583E7F}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
   addDependencyCopyright("libgeotiff", Service<UtilityServices>()->getTextFromFile(":/licenses/libgeotiff"));
   addDependencyCopyright("libtiff", Service<UtilityServices>()->getTextFromFile(":/licenses/libtiff"));
   addDependencyCopyright("proj4", Service<UtilityServices>()->getTextFromFile(":/licenses/proj4"));
}

GeoTIFFImporter::~GeoTIFFImporter()
{}

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

unsigned char GeoTIFFImporter::getFileAffinity(const string& filename)
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

   // Metadata
   DynamicObject* pMetadata = pDescriptor->getMetadata();
   if (pMetadata != NULL)
   {
      // TIFF tags
      string message;
      populateTiffMetadata(pTiffFile, pDescriptor->getMetadata(), message);
      if (message.empty() == false)
      {
         const string& rasterName = pDescriptor->getName();
         mMetadataMessages[rasterName] = message;
      }

      // ISD metadata
      QuickbirdIsd isd(pMetadata);

      QString isdFilename = isd.getIsdFilename();
      if (isdFilename.isEmpty() == true)
      {
         // An ISD filename has not yet been set, so check for an existing file based on the raster filename
         const FileDescriptor* pFileDescriptor = pDescriptor->getFileDescriptor();
         if (pFileDescriptor != NULL)
         {
            QFileInfo tiffInfo(QString::fromStdString(pFileDescriptor->getFilename().getFullPathAndName()));
            QFileInfo isdInfo(tiffInfo.absolutePath() + "/" + tiffInfo.completeBaseName() + ".xml");
            if (isdInfo.exists() == true)
            {
               isdFilename = isdInfo.absoluteFilePath();
            }
         }
      }

      isd.loadIsdMetadata(isdFilename);
   }

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
   pDescriptor->setValidDataTypes(vector<EncodingType>(1, dataType));

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
      vector<int> badValues(1);
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

      // The pixel coordinate system for GeoTIFF is defined as referring to the
      // top-left corner of the pixel, regardless of whether the format is set
      // as PixelIsArea or PixelIsPoint.  More on the issue can be read at the
      // GeoTIFF FAQ: http://remotesensing.org/geotiff/faq.html#PixelIsPoint

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
         // pixels in GeoTIFF refer to top-left corner of the rastered pixel, so floor the center
         gcp.mPixel.mX = floor((numColumns - 1.0) / 2.0);
         gcp.mPixel.mY = floor((numRows - 1.0) / 2.0);
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

int GeoTIFFImporter::getValidationTest(const DataDescriptor* pDescriptor) const
{
   int validationTest = RasterElementImporterShell::getValidationTest(pDescriptor);
   if (pDescriptor != NULL)
   {
      if (pDescriptor->getProcessingLocation() == ON_DISK_READ_ONLY)
      {
         validationTest |= NO_BAND_FILES | NO_SUBSETS;
      }
   }

   return validationTest;
}

QWidget* GeoTIFFImporter::getImportOptionsWidget(DataDescriptor* pDescriptor)
{
   RasterDataDescriptor* pRasterDescriptor = dynamic_cast<RasterDataDescriptor*>(pDescriptor);
   if (pRasterDescriptor == NULL)
   {
      return NULL;
   }

   // Create the widget
   if (mpImportOptionsWidget.get() == NULL)
   {
      mpImportOptionsWidget.reset(new OptionsTiffImporter());
   }

   // Set the data descriptor into the widget to update the metadata for the given raster data
   mpImportOptionsWidget->setDataDescriptor(pRasterDescriptor);

   return mpImportOptionsWidget.get();
}

bool GeoTIFFImporter::createRasterPager(RasterElement *pRasterElement) const
{
   VERIFY(pRasterElement != NULL);
   RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(pRasterElement->getDataDescriptor());
   VERIFY(pDescriptor != NULL);
   RasterFileDescriptor* pFileDescriptor = dynamic_cast<RasterFileDescriptor*>(pDescriptor->getFileDescriptor());
   VERIFY(pFileDescriptor != NULL);

   const string& filename = pRasterElement->getFilename();
   Progress* pProgress = getProgress();

   StepResource pStep("Create pager for GeoTIFF", "app", "AF6176CD-5B39-4CED-A92E-394E3CD1CD00");

   FactoryResource<Filename> pFilename;
   pFilename->setFullPathAndName(filename);

   ExecutableResource pagerPlugIn("GeoTiffPager", string(), pProgress);
   InterleaveFormatType interleave = pFileDescriptor->getInterleaveFormat();
   unsigned int rowCount = pFileDescriptor->getRowCount();
   unsigned int colCount = pFileDescriptor->getColumnCount();
   unsigned int bandCount = pFileDescriptor->getBandCount();
   unsigned int bytesPerElement = pDescriptor->getBytesPerElement();
   pagerPlugIn->getInArgList().setPlugInArgValue<InterleaveFormatType>("interleave", &interleave);
   pagerPlugIn->getInArgList().setPlugInArgValue<unsigned int>("numRows", &rowCount);
   pagerPlugIn->getInArgList().setPlugInArgValue<unsigned int>("numColumns", &colCount);
   pagerPlugIn->getInArgList().setPlugInArgValue<unsigned int>("numBands", &bandCount);
   pagerPlugIn->getInArgList().setPlugInArgValue<unsigned int>("bytesPerElement", &bytesPerElement);
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

bool GeoTIFFImporter::validate(const DataDescriptor* pDescriptor,
                               const vector<const DataDescriptor*>& importedDescriptors, string& errorMessage) const
{
   if (RasterElementImporterShell::validate(pDescriptor, importedDescriptors, errorMessage) == false)
   {
      ValidationTest errorTest = getValidationError();
      if (errorTest == NO_BAND_FILES)
      {
         const RasterDataDescriptor* pRasterDescriptor = dynamic_cast<const RasterDataDescriptor*>(pDescriptor);
         VERIFY(pRasterDescriptor != NULL);

         if (pRasterDescriptor->getInterleaveFormat() == BSQ)
         {
            errorMessage += "  Bands in multiple files are not supported with on-disk read-only processing.";
         }
      }
      else if ((errorTest == NO_ROW_SUBSETS) || (errorTest == NO_COLUMN_SUBSETS))
      {
         errorMessage = errorMessage.substr(0, errorMessage.length() - 1);
         errorMessage += " with on-disk read-only processing.";
      }

      return false;
   }

   const RasterDataDescriptor* pRasterDescriptor = dynamic_cast<const RasterDataDescriptor*>(pDescriptor);
   VERIFY(pRasterDescriptor != NULL);

   const RasterFileDescriptor* pRasterFileDescriptor =
      dynamic_cast<const RasterFileDescriptor*>(pRasterDescriptor->getFileDescriptor());
   VERIFY(pRasterFileDescriptor != NULL);

   if (pRasterFileDescriptor->getInterleaveFormat() == BIL)
   {
      errorMessage = "BIL interleave method not supported";
      return false;
   }

   const string& rasterName = pRasterDescriptor->getName();

   map<string, string>::const_iterator iter = mMetadataMessages.find(rasterName);
   if (iter != mMetadataMessages.end())
   {
      errorMessage += iter->second;
   }

   return true;
}
