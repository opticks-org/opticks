/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QFile>
#include <QtCore/QString>

#include "AppAssert.h"
#include "AppVerify.h"
#include "AppVersion.h"
#include "CachedPager.h"
#include "Classification.h"
#include "DataDescriptor.h"
#include "DesktopServices.h"
#include "DynamicObject.h"
#include "FileDescriptor.h"
#include "FileResource.h"
#include "ImportDescriptor.h"
#include "NitfConstants.h"
#include "NitfImporterShell.h"
#include "NitfMetadataParsing.h"
#include "NitfUtilities.h"
#include "ObjectResource.h"
#include "PlugInArgList.h"
#include "PlugInResource.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterFileDescriptor.h"
#include "RasterPager.h"
#include "RasterUtilities.h"
#include "SpatialDataView.h"
#include "SpecialMetadata.h"
#include "StringUtilities.h"
#include "TypeConverter.h"
#include "TypesFile.h"
#include "UtilityServices.h"

#include <ossim/base/ossimConstants.h>
#include <ossim/support_data/ossimNitfFile.h>
#include <ossim/support_data/ossimNitfFileHeaderV2_X.h>
#include <ossim/support_data/ossimNitfImageHeaderV2_X.h>
#include <ossim/imaging/ossimNitfTileSource.h>

#include <sstream>
#include <vector>

using namespace std;

//#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : TODO: The NULL pix value for OSSIM_PARTIAL " \
//   "is a bad value (leckels)")

Nitf::NitfImporterShell::NitfImporterShell()
{
   setExtensions("NITF Files (*.ntf *.NTF *.nitf *.NITF *.r0 *.R0)");
   addDependencyCopyright("OpenJPEG", Service<UtilityServices>()->getTextFromFile(":/licenses/openjpeg"));
}

Nitf::NitfImporterShell::~NitfImporterShell()
{}

vector<ImportDescriptor*> Nitf::NitfImporterShell::getImportDescriptors(const string &filename)
{
   if (filename.empty() == true)
   {
      return vector<ImportDescriptor*>();
   }

   Nitf::OssimFileResource pNitfFile(filename);
   if (pNitfFile.get() == NULL)
   {
      return vector<ImportDescriptor*>();
   }

   const ossimNitfFileHeaderV2_X* pFileHeader =
      dynamic_cast<const ossimNitfFileHeaderV2_X*>(pNitfFile->getHeader().get());
   if (pFileHeader == NULL)
   {
      return vector<ImportDescriptor*>();
   }

   vector<ImportDescriptor*> importDescriptors;

   ossim_int32 numImageSegments = pFileHeader->getNumberOfImages();
   for (ossim_int32 i = 0; i < numImageSegments; ++i)
   {
      ossimNitfImageHeaderV2_X* pImageHeader = dynamic_cast<ossimNitfImageHeaderV2_X*>(pNitfFile->getNewImageHeader(i));
      if (pImageHeader != NULL)
      {
         ImportDescriptorResource pImportDescriptor(getImportDescriptor(filename, i, pNitfFile, pFileHeader,
            pImageHeader));
         if (pImportDescriptor.get() != NULL)
         {
            importDescriptors.push_back(pImportDescriptor.release());
         }
      }
   }

   return importDescriptors;
}

unsigned char Nitf::NitfImporterShell::getFileAffinity(const string& filename)
{
   // Check that the file exists
   FileResource pFile(filename.c_str(), "rb");
   if (pFile.get() == NULL)
   {
      return Importer::CAN_NOT_LOAD;
   }

   // Check the file header and version (9 bytes plus a terminating NULL)
   char buffer[10];
   memset(buffer, NULL, sizeof(buffer));
   fgets(buffer, sizeof(buffer), pFile);

   if (buffer == Nitf::NITF_METADATA + Nitf::VERSION_02_00 ||
       buffer == Nitf::NITF_METADATA + Nitf::VERSION_02_10)
   {
      return Importer::CAN_LOAD;
   }

   return Importer::CAN_NOT_LOAD;
}

SpatialDataView* Nitf::NitfImporterShell::createView() const
{
   SpatialDataView* pView = RasterElementImporterShell::createView();
   VERIFYRV(pView != NULL, pView);

   RasterElement* pRaster = getRasterElement();
   VERIFYRV(pRaster != NULL, pView);

   const DynamicObject* pMetadata = pRaster->getMetadata();
   VERIFYRV(pMetadata != NULL, pView);

   const string backgroundColorPath[] = { Nitf::NITF_METADATA, Nitf::FILE_HEADER,
      Nitf::FileHeaderFieldNames::BACKGROUND_COLOR, END_METADATA_NAME };

   const DataVariant& dvBackground = pMetadata->getAttributeByPath(backgroundColorPath);
   if (dvBackground.isValid() == true)
   {
      ColorType ctBackground;
      VERIFY(dvBackground.getValue(ctBackground) == true);
      pView->setBackgroundColor(ctBackground);
      pView->refresh();
   }

   return pView;
}

bool Nitf::NitfImporterShell::createRasterPager(RasterElement* pRaster) const
{
   if (pRaster == NULL)
   {
      return false;
   }

   // Get the filename
   string filename = pRaster->getFilename();

   FactoryResource<Filename> pFilename;
   pFilename->setFullPathAndName(filename);

   // Get the image segment corresponding to this raster element
   DataDescriptor* pDescriptor = pRaster->getDataDescriptor();
   VERIFY(pDescriptor != NULL);

   FileDescriptor* pFileDescriptor = pDescriptor->getFileDescriptor();
   VERIFY(pFileDescriptor != NULL);

   const string& datasetLocation = pFileDescriptor->getDatasetLocation();
   if (datasetLocation.empty() == true)
   {
      return false;
   }

   string imageSegmentText = datasetLocation.substr(1);
   unsigned int imageSegment = StringUtilities::fromDisplayString<unsigned int>(imageSegmentText) - 1;

   // Create the resource to execute the pager
   ExecutableResource pPlugIn;

   // Check for J2K compression in the metadata of the raster element being imported and not the given raster
   // element, because the given raster element may be a temporary raster element used by RasterElementImporterShell
   // when the processing location is IN_MEMORY, which does not contain the metadata or the parent information
   // (see RasterUtilities::generateUnchippedRasterDataDescriptor())
   string imageCompression;

   const RasterElement* pElement = getRasterElement();
   if (pElement != NULL)
   {
      const DynamicObject* pMetadata = pElement->getMetadata();
      VERIFYRV(pMetadata, NULL);

      const string attributePath[] =
      {
         Nitf::NITF_METADATA,
         Nitf::IMAGE_SUBHEADER,
         Nitf::ImageSubheaderFieldNames::COMPRESSION,
         END_METADATA_NAME
      };

      imageCompression = pMetadata->getAttributeByPath(attributePath).toDisplayString();
   }

   if ((imageCompression == Nitf::ImageSubheaderFieldValues::IC_C8) ||
      (imageCompression == Nitf::ImageSubheaderFieldValues::IC_M8))
   {
      // Get the offset and size of the image segment in the file
      uint64_t offset = getImageOffset(filename, imageSegment);
      uint64_t size = getImageSize(filename, imageSegment);

      // Use the JPEG2000 pager
      pPlugIn->setPlugIn("JPEG2000 Pager");
      pPlugIn->getInArgList().setPlugInArgValue(CachedPager::PagedElementArg(), pRaster);
      pPlugIn->getInArgList().setPlugInArgValue(CachedPager::PagedFilenameArg(), pFilename.get());
      pPlugIn->getInArgList().setPlugInArgValue("Offset", &offset);
      pPlugIn->getInArgList().setPlugInArgValue("Size", &size);
   }
   else
   {
      // Use the NITF Pager
      pPlugIn->setPlugIn("NitfPager");
      pPlugIn->getInArgList().setPlugInArgValue(CachedPager::PagedElementArg(), pRaster);
      pPlugIn->getInArgList().setPlugInArgValue(CachedPager::PagedFilenameArg(), pFilename.get());
      pPlugIn->getInArgList().setPlugInArgValue("Segment Number", &imageSegment);
   }

   if (pPlugIn->execute() == true)
   {
      RasterPager* pPager = dynamic_cast<RasterPager*>(pPlugIn->getPlugIn());
      if (pPager != NULL)
      {
         pRaster->setPager(pPager);
         pPlugIn->releasePlugIn();
         return true;
      }
   }

   return false;
}

EncodingType Nitf::NitfImporterShell::ossimImageHeaderToEncodingType(const ossimNitfImageHeaderV2_X* pImgHeader)
{
   VERIFYRV(pImgHeader != NULL, EncodingType());

   // Use NBPP not ABPP as is done in ossimNitfTileSource.cpp.
   const ossim_int32 bitsPerPixel = pImgHeader->getBitsPerPixelPerBand();
   const ossimString pixelValueType = pImgHeader->getPixelValueType().upcase();
   if (bitsPerPixel <= 8)
   {
      if ((pixelValueType == "B") || (pixelValueType == "INT"))
      {
         return INT1UBYTE;
      }
      else if (pixelValueType == "SI")
      {
         return INT1SBYTE;
      }
   }
   else if (bitsPerPixel <= 16)
   {
      if (pixelValueType == "INT")
      {
         return INT2UBYTES;
      }
      else if (pixelValueType == "SI")
      {
         return INT2SBYTES;
      }
   }
   else if (bitsPerPixel <= 32)
   {
      if (pixelValueType == "R")
      {
         return FLT4BYTES;
      }
      else if (pixelValueType == "INT")
      {
         return INT4UBYTES;
      }
      else if (pixelValueType == "SI")
      {
         return INT4SBYTES;
      }
   }
   else if (bitsPerPixel <= 64)
   {
      if (pixelValueType == "R")
      {
         return FLT8BYTES;
      }
   }

   return EncodingType();
}

bool Nitf::NitfImporterShell::validate(const DataDescriptor* pDescriptor,
                                       const vector<const DataDescriptor*>& importedDescriptors,
                                       string& errorMessage) const
{
   if (pDescriptor == NULL)
   {
      return false;
   }

   // Get the name of the file being imported
   const FileDescriptor* const pFileDescriptor = pDescriptor->getFileDescriptor();
   VERIFY(pFileDescriptor != NULL);

   string filename = pFileDescriptor->getFilename().getFullPathAndName();

   // Get the image segment being validated
   const string& datasetLocation = pFileDescriptor->getDatasetLocation();
   VERIFY(datasetLocation.empty() == false);

   string imageSegmentText = datasetLocation.substr(1);
   ossim_uint32 imageSegment = StringUtilities::fromDisplayString<unsigned int>(imageSegmentText) - 1;

   // Check for J2K compression, which can be imported without ossim
   const DynamicObject* pMetadata = pDescriptor->getMetadata();
   VERIFY(pMetadata != NULL);

   const string attributePath[] =
   {
      Nitf::NITF_METADATA,
      Nitf::IMAGE_SUBHEADER,
      Nitf::ImageSubheaderFieldNames::COMPRESSION,
      END_METADATA_NAME
   };

   string imageCompression = pMetadata->getAttributeByPath(attributePath).toDisplayString();
   if ((imageCompression != Nitf::ImageSubheaderFieldValues::IC_C8) &&
      (imageCompression != Nitf::ImageSubheaderFieldValues::IC_M8))
   {
      // This image segment does not have J2K compression, so check if it can be imported by ossim
      Nitf::OssimImageHandlerResource pHandler(filename);
      if (pHandler.get() == NULL || pHandler->canCastTo("ossimNitfTileSource") == false)
      {
         errorMessage = "This image segment is not supported by the " + getName() + ".";
         return false;
      }

      vector<ossim_uint32> importableImageSegments;
      pHandler->getEntryList(importableImageSegments);

      vector<ossim_uint32>::iterator segmentIter = find(importableImageSegments.begin(),
         importableImageSegments.end(), imageSegment);
      if (segmentIter == importableImageSegments.end())
      {
         // This image segment cannot be imported by ossim, which is generally due to the image segment
         // using an unsupported compression format
         errorMessage = "This image segment is not supported by the " + getName() + ".";
         return false;
      }
   }

   // Perform the default validation checks
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

   // Add any previously obtained warning messages to the output message
   map<ossim_uint32, string>::const_iterator messageIter = mParseMessages.find(imageSegment);
   if (messageIter != mParseMessages.end() && messageIter->second.empty() == false)
   {
      if (errorMessage.empty() == false)
      {
         errorMessage += "\n";
      }

      errorMessage += messageIter->second;
   }

   // Check for file sizes too large for the current platform
   const qint64 actualSize = QFile(QString::fromStdString(filename)).size();
   const qint64 maxSize = numeric_limits<ossim_uint64>::max() & numeric_limits<streamoff>::max();
   if (actualSize > maxSize)
   {
      if (errorMessage.empty() == false)
      {
         errorMessage += "\n";
      }

      errorMessage += "This file exceeds the maximum supported size for this platform.  "
         "Data at the end of the file may be missing or incorrect.  "
         "IMPORT DATA FROM THIS FILE WITH THE KNOWLEDGE THAT IT IS NOT FULLY SUPPORTED.";
   }

   // warn user if unsupported metadata is in the file
   // NITF 2.0: ICORDS values of U and C are unsupported
   const string versionPathName[] = { Nitf::NITF_METADATA, Nitf::FILE_HEADER,
      Nitf::FileHeaderFieldNames::FILE_VERSION, END_METADATA_NAME };
   if (pMetadata->getAttributeByPath(versionPathName).toDisplayString() == Nitf::VERSION_02_00)
   {
      const string iCordsPath[] = { Nitf::NITF_METADATA, Nitf::IMAGE_SUBHEADER,
         Nitf::ImageSubheaderFieldNames::ICORDS, END_METADATA_NAME };
      string iCords = pMetadata->getAttributeByPath(iCordsPath).toDisplayString();
      if (iCords == Nitf::ImageSubheaderFieldValues::ICORDS_GEOCENTRIC)
      {
      if (errorMessage.empty() == false)
      {
         errorMessage += "\n";
      }

         errorMessage += "The ICORDS is not a supported value.";
      }
   }

   // LUTs are unsupported
   bool hasLut = false;
   const string irepPathName[] = { Nitf::NITF_METADATA, Nitf::IMAGE_SUBHEADER,
      Nitf::ImageSubheaderFieldNames::REPRESENTATION, END_METADATA_NAME };

   string irep;
   const DataVariant& dvIrep = pMetadata->getAttributeByPath(irepPathName);
   if (dvIrep.getValue(irep) == true)
   {
      if (irep == Nitf::ImageSubheaderFieldValues::REPRESENTATION_LUT)
      {
         hasLut = true;
      }
   }

   if (hasLut == false)
   {
      vector<string> irepBands;
      const string irepBandsPathName[] = { Nitf::NITF_METADATA, Nitf::IMAGE_SUBHEADER,
         Nitf::ImageSubheaderFieldNames::BAND_REPRESENTATIONS, END_METADATA_NAME };
      const DataVariant& dvIrepBands = pMetadata->getAttributeByPath(irepBandsPathName);
      if (dvIrepBands.getValue(irepBands) == true)
      {
         for (vector<string>::iterator iter = irepBands.begin(); iter != irepBands.end(); iter++)
         {
            if (*iter == Nitf::ImageSubheaderFieldValues::BAND_REPRESENTATIONS_LUT)
            {
               hasLut = true;
               break;
            }
         }
      }
   }

   if (hasLut == false)
   {
      vector<unsigned int> numLuts;
      const string numLutsPathName[] = { Nitf::NITF_METADATA, Nitf::IMAGE_SUBHEADER,
         Nitf::ImageSubheaderFieldNames::NUMBER_OF_LUTS, END_METADATA_NAME };
      const DataVariant& dvNumLuts = pMetadata->getAttributeByPath(numLutsPathName);
      if (dvNumLuts.getValue(numLuts) == true)
      {
         for (vector<unsigned int>::iterator iter = numLuts.begin(); iter != numLuts.end(); iter++)
         {
            if (*iter > 0)
            {
               hasLut = true;
               break;
            }
         }
      }
   }

   if (hasLut == true)
   {
      if (errorMessage.empty() == false)
      {
         errorMessage += "\n";
      }

      errorMessage += "The lookup table will not be applied.";
   }

   // Check for valid Classification markings. If any level is higher than the file header, display a warning.
   FactoryResource<Classification> pClassification;
   const Classification* pOverallClassification = pDescriptor->getClassification();
   VERIFY(pOverallClassification != NULL);

   // Look in the image subheader.
   string imageClassLevel;
   const string imageClassLevelPathName[] = { Nitf::NITF_METADATA, Nitf::IMAGE_SUBHEADER,
      Nitf::ImageSubheaderFieldNames::SECURITY_LEVEL, END_METADATA_NAME };
   const DataVariant& dvImageClassLevel = pMetadata->getAttributeByPath(imageClassLevelPathName);
   if (dvImageClassLevel.getValue(imageClassLevel) == true)
   {
      pClassification->setLevel(imageClassLevel);
      if (pClassification->hasGreaterLevel(pOverallClassification) == true)
      {
         if (errorMessage.empty() == false)
         {
            errorMessage += "\n";
         }

         errorMessage += "THIS FILE CONTAINS INVALID CLASSIFICATION INFORMATION!  The image has a higher "
            "classification level than the file.  Update the Classification information before proceeding.";
      }
   }

   // Look in each DES subheader.
   int numDes;
   const string numDesPathName[] = { Nitf::NITF_METADATA, Nitf::FILE_HEADER,
      Nitf::FileHeaderFieldNames::NUM_DES, END_METADATA_NAME };
   const DataVariant& dvNumDes = pMetadata->getAttributeByPath(numDesPathName);
   if (dvNumDes.getValue(numDes) == true)
   {
      for (int i = 0; i < numDes; ++i)
      {
         stringstream desStr;
         desStr << "DES_" << setw(3) << setfill('0') << i;

         string desClassLevel;
         const string desClassLevelPathName[] = { Nitf::NITF_METADATA, Nitf::DES_METADATA,
            desStr.str(), Nitf::DesSubheaderFieldNames::SECURITY_LEVEL, END_METADATA_NAME };
         const DataVariant& dvDesClassLevel = pMetadata->getAttributeByPath(desClassLevelPathName);
         if (dvDesClassLevel.getValue(desClassLevel) == true)
         {
            pClassification->setLevel(desClassLevel);
            if (pClassification->hasGreaterLevel(pOverallClassification) == true)
            {
               if (errorMessage.empty() == false)
               {
                  errorMessage += "\n";
               }

               errorMessage += "THIS FILE CONTAINS INVALID CLASSIFICATION INFORMATION!  " + desStr.str() +
                  " has a higher classification level than the file.  Update the Classification information "
                  "before proceeding.";
            }
         }
      }
   }

   return true;
}

int Nitf::NitfImporterShell::getValidationTest(const DataDescriptor* pDescriptor) const
{
   int validationTest = RasterElementImporterShell::getValidationTest(pDescriptor) | VALID_METADATA;
   if (pDescriptor != NULL)
   {
      if (pDescriptor->getProcessingLocation() == ON_DISK_READ_ONLY)
      {
         const DynamicObject* pMetadata = pDescriptor->getMetadata();
         VERIFYRV(pMetadata, validationTest);

         const string compressionPath[] =
         {
            Nitf::NITF_METADATA,
            Nitf::IMAGE_SUBHEADER,
            Nitf::ImageSubheaderFieldNames::COMPRESSION,
            END_METADATA_NAME
         };

         string imageCompression = pMetadata->getAttributeByPath(compressionPath).toDisplayString();
         if ((imageCompression != Nitf::ImageSubheaderFieldValues::IC_C8) &&
            (imageCompression != Nitf::ImageSubheaderFieldValues::IC_M8))
         {
            validationTest |= NO_BAND_FILES | NO_SUBSETS;
         }
      }
   }

   return validationTest;
}

ImportDescriptor* Nitf::NitfImporterShell::getImportDescriptor(const string& filename, ossim_uint32 imageSegment,
                                                               const Nitf::OssimFileResource& pFile,
                                                               const ossimNitfFileHeaderV2_X* pFileHeader,
                                                               const ossimNitfImageHeaderV2_X* pImageSubheader)
{
   if (pImageSubheader == NULL)
   {
      return NULL;
   }

   EncodingType dataType = ossimImageHeaderToEncodingType(pImageSubheader);
   if (dataType.isValid() == false)
   {
      return NULL;
   }

   stringstream imageNameStream;
   imageNameStream << "I" << imageSegment + 1;
   string imageName = imageNameStream.str();

   ImportDescriptorResource pImportDescriptor(filename + "-" + imageName,
      TypeConverter::toString<RasterElement>(), NULL);
   VERIFYRV(pImportDescriptor.get() != NULL, NULL);
   pImportDescriptor->setImported(pImageSubheader->getRepresentation() != "NODISPLY");

   RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(pImportDescriptor->getDataDescriptor());
   VERIFYRV(pDescriptor != NULL, NULL);

   vector<DimensionDescriptor> bands =  RasterUtilities::generateDimensionVector(pImageSubheader->getNumberOfBands(),
      true, false, true);
   pDescriptor->setBands(bands);

   vector<DimensionDescriptor> rows = RasterUtilities::generateDimensionVector(pImageSubheader->getNumberOfRows(),
      true, false, true);
   pDescriptor->setRows(rows);

   vector<DimensionDescriptor> cols = RasterUtilities::generateDimensionVector(pImageSubheader->getNumberOfCols(),
      true, false, true);
   pDescriptor->setColumns(cols);

   if (pImageSubheader->getIMode() == "P")
   {
      pDescriptor->setInterleaveFormat(BIP);
   }
   else if (pImageSubheader->getIMode() == "R")
   {
      pDescriptor->setInterleaveFormat(BIL);
   }
   else
   {
      pDescriptor->setInterleaveFormat(BSQ);
   }

   pDescriptor->setDataType(dataType);
   pDescriptor->setValidDataTypes(vector<EncodingType>(1, dataType));
   pDescriptor->setProcessingLocation(IN_MEMORY);

   map<string, TrePlugInResource> parsers;
   string errorMessage;

   // Set the file descriptor
   RasterFileDescriptor* pFileDescriptor = dynamic_cast<RasterFileDescriptor*>(
      RasterUtilities::generateAndSetFileDescriptor(pDescriptor, filename, imageName, LITTLE_ENDIAN_ORDER));
   if (pFileDescriptor == NULL)
   {
      return NULL;
   }

   // Set the bits per element, which may be different than the data type in the data descriptor,
   // using NBPP instead of ABPP as is done in ossimNitfTileSource.cpp.
   unsigned int bitsPerPixel = static_cast<unsigned int>(pImageSubheader->getBitsPerPixelPerBand());
   pFileDescriptor->setBitsPerElement(bitsPerPixel);

   // Populate the metadata and set applicable values in the data descriptor
   if (Nitf::importMetadata(imageSegment + 1, pFile, pFileHeader, pImageSubheader, pDescriptor, parsers,
      errorMessage) == true)
   {
      const DynamicObject* pMetadata = pDescriptor->getMetadata();
      VERIFYRV(pMetadata, NULL);

      // This only checks the first BANDSB. It is possible to have multiple BANDSB TREs.
      // If someone runs across real data where the bad band info is in another BANDSB TRE
      // this code will need to be modified.
      const string bandsbPath[] =
      {
         Nitf::NITF_METADATA,
         Nitf::TRE_METADATA,
         "BANDSB",
         "0",
         END_METADATA_NAME
      };

      const DynamicObject* pBandsB = dv_cast<DynamicObject>(&pMetadata->getAttributeByPath(bandsbPath));
      if (pBandsB != NULL && pBandsB->getAttribute(Nitf::TRE::BANDSB::BAD_BAND + "#0").isValid())
      {
         const vector<DimensionDescriptor>& curBands = pDescriptor->getBands();
         vector<DimensionDescriptor> newBands;
         for (size_t idx = 0; idx < curBands.size(); ++idx)
         {
            const int* pVal = dv_cast<int>(&pBandsB->getAttribute(
               Nitf::TRE::BANDSB::BAD_BAND + "#" + StringUtilities::toDisplayString(idx)));
            if (pVal == NULL || *pVal == 1) // 0 == invalid or suspect band, 1 = valid band
            {
               newBands.push_back(curBands[idx]);
            }
         }
         pDescriptor->setBands(newBands);
      }

      // Bad values
      if (pImageSubheader->hasTransparentCode() == true)
      {
         vector<int> badValues;
         badValues.push_back(static_cast<int>(pImageSubheader->getTransparentCode()));
         pDescriptor->setBadValues(badValues);
      }

      // If red, green, OR blue bands are valid, set the display mode to RGB.
      if (pDescriptor->getDisplayBand(RED).isValid() == true ||
         pDescriptor->getDisplayBand(GREEN).isValid() == true ||
         pDescriptor->getDisplayBand(BLUE).isValid() == true)
      {
         pDescriptor->setDisplayMode(RGB_MODE);
      }
      // Otherwise, if the gray band is valid, set the display mode to GRAYSCALE.
      else if (pDescriptor->getDisplayBand(GRAY).isValid() == true)
      {
         pDescriptor->setDisplayMode(GRAYSCALE_MODE);
      }
      // Otherwise, if at least 3 bands are available, set the display mode to RGB,
      // and set the first three bands to red, green, and blue respectively.
      else if (bands.size() >= 3)
      {
         pDescriptor->setDisplayBand(RED, bands[0]);
         pDescriptor->setDisplayBand(GREEN, bands[1]);
         pDescriptor->setDisplayBand(BLUE, bands[2]);
         pDescriptor->setDisplayMode(RGB_MODE);
      }
      // Otherwise, if at least 1 band is available, set the display mode to GRAYSCALE,
      // and set the first band to GRAY.
      else if (bands.empty() == false)
      {
         pDescriptor->setDisplayBand(GRAY, bands[0]);
         pDescriptor->setDisplayMode(GRAYSCALE_MODE);
      }
      else
      {
         return NULL;
      }

      // Special initialization for J2K compressed image segments
      const string compressionPath[] =
      {
         Nitf::NITF_METADATA,
         Nitf::IMAGE_SUBHEADER,
         Nitf::ImageSubheaderFieldNames::COMPRESSION,
         END_METADATA_NAME
      };

      string imageCompression = pMetadata->getAttributeByPath(compressionPath).toDisplayString();
      if ((imageCompression == Nitf::ImageSubheaderFieldValues::IC_C8) ||
         (imageCompression == Nitf::ImageSubheaderFieldValues::IC_M8))
      {
         // Per Section 8.1 of the BIIF Profile for JPEG 2000 Version 01.10 (BPJ2K01.10),
         // if the values in the J2K data differ from the values in the image subheader,
         // the J2K values are given precedence.
         opj_image_t* pImage = getImageInfo(filename, imageSegment, OPJ_CODEC_J2K);
         if (pImage == NULL)
         {
            pImage = getImageInfo(filename, imageSegment, OPJ_CODEC_JP2);
         }

         if (pImage != NULL)
         {
            // Bits per element
            unsigned int bitsPerElement = pImage->comps->prec;
            if (bitsPerElement != pFileDescriptor->getBitsPerElement())
            {
               pFileDescriptor->setBitsPerElement(bitsPerElement);
            }

            // Data type
            EncodingType dataType = INT1UBYTE;
            if (bitsPerElement <= 8)
            {
               if (pImage->comps->sgnd)
               {
                  dataType = INT1SBYTE;
               }
               else
               {
                  dataType = INT1UBYTE;
               }
            }
            else if (bitsPerElement <= 16)
            {
               if (pImage->comps->sgnd)
               {
                  dataType = INT2SBYTES;
               }
               else
               {
                  dataType = INT2UBYTES;
               }
            }
            else if (bitsPerElement <= 32)
            {
               if (pImage->comps->sgnd)
               {
                  dataType = INT4SBYTES;
               }
               else
               {
                  dataType = INT4UBYTES;
               }
            }
            else if (bitsPerElement <= 64)
            {
               dataType = FLT8BYTES;
            }

            if (dataType != pDescriptor->getDataType())
            {
               pDescriptor->setDataType(dataType);
            }

            // Rows
            unsigned int numRows = pImage->comps->h;
            if (numRows != pFileDescriptor->getRowCount())
            {
               vector<DimensionDescriptor> rows = RasterUtilities::generateDimensionVector(numRows, true, false, true);
               pDescriptor->setRows(rows);
               pFileDescriptor->setRows(rows);
            }

            // Columns
            unsigned int numColumns = pImage->comps->w;
            if (numColumns != pFileDescriptor->getColumnCount())
            {
               vector<DimensionDescriptor> columns = RasterUtilities::generateDimensionVector(numColumns, true, false,
                  true);
               pDescriptor->setColumns(columns);
               pFileDescriptor->setColumns(columns);
            }

            // Bands
            unsigned int numBands = pImage->numcomps;
            if (numBands != pFileDescriptor->getBandCount())
            {
               vector<DimensionDescriptor> bands = RasterUtilities::generateDimensionVector(numBands, true, false,
                  true);
               pDescriptor->setBands(bands);
               pFileDescriptor->setBands(bands);
            }

            // Cleanup
            opj_image_destroy(pImage);
         }

         // Set the interleave format as BIP, which is the interleave format for J2K data
         pDescriptor->setInterleaveFormat(BIP);
         pFileDescriptor->setInterleaveFormat(BIP);
      }

      mParseMessages[imageSegment] = errorMessage;
   }

   return pImportDescriptor.release();
}

uint64_t Nitf::NitfImporterShell::getImageOffset(const string& filename, unsigned int imageSegment) const
{
   Nitf::OssimFileResource pNitfFile(filename);
   if (pNitfFile.get() == NULL)
   {
      return 0;
   }

   const ossimRefPtr<ossimNitfFileHeader> pFileHeader = pNitfFile->getHeader();
   if (pFileHeader != NULL)
   {
      ossimRefPtr<ossimNitfImageHeader> pImageHeader = pNitfFile->getNewImageHeader(static_cast<long>(imageSegment));
      if (pImageHeader != NULL)
      {
         return static_cast<uint64_t>(pImageHeader->getDataLocation());
      }
   }

   return 0;
}

uint64_t Nitf::NitfImporterShell::getImageSize(const string& filename, unsigned int imageSegment) const
{
   if (filename.empty() == true)
   {
      return 0;
   }

   // Seek to the Length of nth Image Segment (LI) field in the NITF file header to get the image segment data size
   FileResource pFile(filename.c_str(), "rb");
   fseek(pFile.get(), 369 + (16 * imageSegment), SEEK_SET);

   const size_t fieldLength = 10;
   vector<char> buffer(fieldLength);
   fread(&buffer[0], 1, fieldLength, pFile.get());
   string fieldValue(&buffer[0], fieldLength);

   uint64_t size = StringUtilities::fromDisplayString<uint64_t>(fieldValue);
   return size;
}

opj_image_t* Nitf::NitfImporterShell::getImageInfo(const std::string& filename, unsigned int imageSegment,
                                                   OPJ_CODEC_FORMAT decoderType) const
{
   if (filename.empty() == true)
   {
      return NULL;
   }

   // Open a byte stream from the file the size of the J2K compressed data
   FileResource pFile(filename.c_str(), "rb");
   if (pFile.get() == NULL)
   {
      return NULL;
   }

   uint64_t dataOffset = getImageOffset(filename, imageSegment);
   uint64_t dataSize = getImageSize(filename, imageSegment);

   size_t fileLength = 0;
   if (dataSize > 0)
   {
      fileLength = static_cast<size_t>(dataSize);
   }
   else
   {
      fseek(pFile.get(), 0, SEEK_END);
      fileLength = static_cast<size_t>(ftell(pFile.get()));
   }

   opj_stream_t* pStream = opj_stream_create_file_stream(pFile.get(), fileLength, true);
   if (pStream == NULL)
   {
      return NULL;
   }

   opj_stream_set_user_data_length(pStream, fileLength);

   // Seek to the position of the compressed data in the file
   fseek(pFile.get(), static_cast<long>(dataOffset), SEEK_SET);

   // Create the appropriate codec
   opj_codec_t* pCodec = opj_create_decompress(decoderType);
   if (pCodec == NULL)
   {
      opj_stream_destroy(pStream);
      return NULL;
   }

   // Setup the decoding parameters
   opj_dparameters_t parameters;
   opj_set_default_decoder_parameters(&parameters);
   if (opj_setup_decoder(pCodec, &parameters) == OPJ_FALSE)
   {
      opj_stream_destroy(pStream);
      opj_destroy_codec(pCodec);
      return NULL;
   }

   // Read the header info from the stream and fill the image structure
   opj_image_t* pImage = NULL;
   if (opj_read_header(pStream, pCodec, &pImage) == OPJ_FALSE)
   {
      opj_stream_destroy(pStream);
      opj_destroy_codec(pCodec);
      return NULL;
   }

   // Cleanup
   opj_stream_destroy(pStream);
   opj_destroy_codec(pCodec);

   return pImage;
}
