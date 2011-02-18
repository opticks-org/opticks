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
#include "Classification.h"
#include "DataDescriptor.h"
#include "DesktopServices.h"
#include "DynamicObject.h"
#include "FileDescriptor.h"
#include "FileResource.h"
#include "Georeference.h"
#include "ImportDescriptor.h"
#include "NitfConstants.h"
#include "NitfImageSubheader.h"
#include "NitfImporter.h"
#include "NitfMetadataParsing.h"
#include "NitfPager.h"
#include "NitfResource.h"
#include "NitfUtilities.h"
#include "ObjectResource.h"
#include "PlugInArgList.h"
#include "PlugInRegistration.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterFileDescriptor.h"
#include "RasterPager.h"
#include "RasterUtilities.h"
#include "Resource.h"
#include "SpatialDataView.h"
#include "SpecialMetadata.h"
#include "StringUtilities.h"
#include "TypesFile.h"

#include <ossim/base/ossimConstants.h>
#include <ossim/support_data/ossimNitfFile.h>
#include <ossim/support_data/ossimNitfFileHeaderV2_X.h>
#include <ossim/support_data/ossimNitfImageHeaderV2_X.h>
#include <ossim/imaging/ossimNitfTileSource.h>

#include <sstream>
#include <vector>

using namespace std;

#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : TODO: The NULL pix value for OSSIM_PARTIAL " \
   "is a bad value (leckels)")

REGISTER_PLUGIN(OpticksNitf, NitfImporter, Nitf::NitfImporter);

Nitf::NitfImporter::NitfImporter()
{
   setName("NITF Importer");
   setExtensions("NITF Files (*.ntf *.NTF *.nitf *.NITF *.r0 *.R0)");
   setDescriptorId("{2130D292-2647-4e98-BEF1-BA743234148C}");
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

Nitf::NitfImporter::~NitfImporter()
{}

vector<ImportDescriptor*> Nitf::NitfImporter::getImportDescriptors(const string &filename)
{
   vector<ImportDescriptor*> retval;

   if (filename.empty())
   {
      return retval;
   }

   Nitf::OssimFileResource pFile(filename);
   if (pFile.get() == NULL)
   {
      return retval;
   }

   Nitf::OssimImageHandlerResource pHandler(filename);
   if (pHandler.get() == NULL || pHandler->canCastTo("ossimNitfTileSource") == false)
   {
      return retval;
   }

   ossimNitfFileHeaderV2_X* pFileHeader =
      PTR_CAST(ossimNitfFileHeaderV2_X, pFile->getHeader().get());
   if (pFileHeader == NULL)
   {
      return retval;
   }

   // Not all segments are importable.  This is generally due to the segment
   // using an unsupported compression format.  Only generate descriptors
   // for the importable segments.
   vector<ossim_uint32> importableImageSegments;
   pHandler->getEntryList(importableImageSegments);

   // Create map of TRE parsers.
   // The sole purpose of this map is to force DLLs to stay loaded while the metadata is being imported.
   std::map<std::string, TrePlugInResource> parsers;

   for (vector<ossim_uint32>::iterator segmentIter = importableImageSegments.begin();
        segmentIter != importableImageSegments.end(); ++segmentIter)
   {
      // Do not call pHandler->setCurrentEntry as it is a very expensive operation
      // which causes up to a several second delay on files with many large images.
      const ossim_uint32& currentIndex = *segmentIter;
      ossimNitfImageHeaderV2_X* pImgHeader =
         PTR_CAST(ossimNitfImageHeaderV2_X, pFile->getNewImageHeader(currentIndex));
      if (pImgHeader == NULL)
      {
         continue;
      }

      EncodingType dataType = ossimImageHeaderToEncodingType(pImgHeader);
      if (dataType.isValid() == false)
      {
         continue;
      }

      stringstream imageNameStream;
      imageNameStream << "I" << currentIndex+1;
      string imageName = imageNameStream.str();
      ImportDescriptorResource pImportDescriptor(filename + "-" + imageName, "RasterElement", NULL);
      VERIFYRV(pImportDescriptor.get() != NULL, retval);

      RasterDataDescriptor* pDd = dynamic_cast<RasterDataDescriptor*>(pImportDescriptor->getDataDescriptor());
      VERIFYRV(pDd != NULL, retval);

      vector<DimensionDescriptor> bands = 
         RasterUtilities::generateDimensionVector(pImgHeader->getNumberOfBands(), true, false, true);
      pDd->setBands(bands);

      vector<DimensionDescriptor> rows = 
         RasterUtilities::generateDimensionVector(pImgHeader->getNumberOfRows(), true, false, true);
      pDd->setRows(rows);

      vector<DimensionDescriptor> cols = 
         RasterUtilities::generateDimensionVector(pImgHeader->getNumberOfCols(), true, false, true);
      pDd->setColumns(cols);

      if (pImgHeader->getIMode() == "P")
      {
         pDd->setInterleaveFormat(BIP);
      }
      else if (pImgHeader->getIMode() == "R")
      {
         pDd->setInterleaveFormat(BIL);
      }
      else
      {
         pDd->setInterleaveFormat(BSQ);
      }

      pDd->setDataType(dataType);
      pDd->setValidDataTypes(vector<EncodingType>(1, dataType));
      pDd->setProcessingLocation(IN_MEMORY);

      RasterFileDescriptor* pFd = dynamic_cast<RasterFileDescriptor*>(
         RasterUtilities::generateAndSetFileDescriptor(pDd, filename, imageName, LITTLE_ENDIAN_ORDER));

      string errorMessage;
      if (Nitf::importMetadata(currentIndex + 1, pFile, pFileHeader, pImgHeader, pDd, parsers, errorMessage) == true)
      {
         if (pImgHeader->hasTransparentCode() == true)
         {
            vector<int> badValues;
            badValues.push_back(static_cast<int>(pImgHeader->getTransparentCode()));
            pDd->setBadValues(badValues);
         }

         // If red, green, OR blue bands are valid, set the display mode to RGB.
         if (pDd->getDisplayBand(RED).isValid() == true ||
             pDd->getDisplayBand(GREEN).isValid() == true ||
             pDd->getDisplayBand(BLUE).isValid() == true)
         {
            pDd->setDisplayMode(RGB_MODE);
         }
         // Otherwise, if the gray band is valid, set the display mode to GRAYSCALE.
         else if (pDd->getDisplayBand(GRAY).isValid() == true)
         {
            pDd->setDisplayMode(GRAYSCALE_MODE);
         }
         // Otherwise, if at least 3 bands are available, set the display mode to RGB,
         // and set the first three bands to red, green, and blue respectively.
         else if (bands.size() >= 3)
         {
            pDd->setDisplayBand(RED, bands[0]);
            pDd->setDisplayBand(GREEN, bands[1]);
            pDd->setDisplayBand(BLUE, bands[2]);
            pDd->setDisplayMode(RGB_MODE);
         }
         // Otherwise, if at least 1 band is available, set the display mode to GRAYSCALE,
         // and set the first band to GRAY.
         else if (bands.empty() == false)
         {
            pDd->setDisplayBand(GRAY, bands[0]);
            pDd->setDisplayMode(GRAYSCALE_MODE);
         }
         else
         {
            continue;
         }

         mParseMessages[imageName] = errorMessage;
         retval.push_back(pImportDescriptor.release());
      }
   }

   return retval;
}

unsigned char Nitf::NitfImporter::getFileAffinity(const string& filename)
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

SpatialDataView* Nitf::NitfImporter::createView() const
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

PlugIn* Nitf::NitfImporter::getGeoreferencePlugIn() const
{
   RasterElement* pRaster = getRasterElement();
   if (pRaster != NULL)
   {
      ExecutableResource geoPlugIn("RPC Georeference", string(), getProgress(), true);

      const Georeference* pGeoPlugIn = dynamic_cast<const Georeference*>(geoPlugIn->getPlugIn());
      if (pGeoPlugIn != NULL)
      {
         if (pGeoPlugIn->canHandleRasterElement(pRaster) == true)
         {
            return geoPlugIn->releasePlugIn();
         }
      }
   }

   return RasterElementImporterShell::getGeoreferencePlugIn();
}

bool Nitf::NitfImporter::createRasterPager(RasterElement *pRaster) const
{
   VERIFY(pRaster != NULL);

   DataDescriptor* pDd = pRaster->getDataDescriptor();
   VERIFY(pDd != NULL);
   FileDescriptor* pFd = pDd->getFileDescriptor();
   VERIFY(pFd != NULL);

   const string& datasetLocation = pFd->getDatasetLocation();
   if (datasetLocation.empty() == true)
   {
      return false;
   }

   stringstream imageNameStream(datasetLocation.substr(1));
   int imageSegment;
   imageNameStream >> imageSegment;

   FactoryResource<Filename> pFn;
   pFn->setFullPathAndName(pFd->getFilename());

   ExecutableResource pPlugIn("NitfPager");
   pPlugIn->getInArgList().setPlugInArgValue("Segment Number", &imageSegment);
   pPlugIn->getInArgList().setPlugInArgValue(Nitf::Pager::PagedElementArg(), pRaster);
   pPlugIn->getInArgList().setPlugInArgValue(Nitf::Pager::PagedFilenameArg(), pFn.get());

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

bool Nitf::NitfImporter::validateDefaultOnDiskReadOnly(const DataDescriptor* pDescriptor, string& errorMessage) const
{
   const RasterDataDescriptor* pRasterDescriptor = dynamic_cast<const RasterDataDescriptor*>(pDescriptor);
   if (pRasterDescriptor == NULL)
   {
      errorMessage = "The data descriptor is invalid!";
      return false;
   }
   if (pDescriptor->getProcessingLocation() != ON_DISK_READ_ONLY)
   {
      // this method only checks Nitf's on-disk read-only
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

   return true;
}

EncodingType Nitf::NitfImporter::ossimImageHeaderToEncodingType(ossimNitfImageHeaderV2_X* pImgHeader)
{
   VERIFYRV(pImgHeader != NULL, EncodingType());

   // Use NBPP not ABPP as is done in ossimNitfTileSource.cpp.
   const ossim_int32 bitsPerPixel = pImgHeader->getBitsPerPixelPerBand();
   const ossimString pixelValueType = pImgHeader->getPixelValueType().upcase();
   switch (bitsPerPixel)
   {
      case 8:
      {
         if (pixelValueType == "INT")
         {
            return INT1UBYTE;
         }

         if (pixelValueType == "SI")
         {
            return INT1SBYTE;
         }

         break;
      }
      case 16:
      {
         if (pixelValueType == "INT")
         {
            return INT2UBYTES;
         }

         if (pixelValueType == "SI")
         {
            return INT2SBYTES;
         }

         break;
      }
      case 32:
      {
         if(pixelValueType == "R")
         {
            return FLT4BYTES;
         }

         if (pixelValueType == "INT")
         {
            return INT4UBYTES;
         }

         if (pixelValueType == "SI")
         {
            return INT4SBYTES;
         }

         break;
      }
      case 64:
      {
         if (pixelValueType == "R")
         {
            return FLT8BYTES;
         }

         break;
      }
      default:
      {
         if (pImgHeader->isCompressed())
         {
            if (bitsPerPixel < 8)
            {
               return INT1UBYTE;
            }

            if (bitsPerPixel < 16)
            {
               return INT2UBYTES;
            }

            if (bitsPerPixel < 32)
            {
               return FLT4BYTES;
            }
         }

         break;
      }
   }

   return EncodingType();
}

bool Nitf::NitfImporter::validate(const DataDescriptor* pDescriptor, string& errorMessage) const
{
   if (RasterElementImporterShell::validate(pDescriptor, errorMessage) == false)
   {
      return false;
   }

   if (pDescriptor == NULL)
   {
      errorMessage += "No descriptor available.";
      return false;
   }

   const DynamicObject* const pMetadata = pDescriptor->getMetadata();
   if (pMetadata == NULL)
   {
      errorMessage += "No metadata available.";
      return false;
   }

   const FileDescriptor* const pFileDescriptor = pDescriptor->getFileDescriptor();
   if (pFileDescriptor == NULL)
   {
      errorMessage += "No file descriptor available.";
      return false;
   }

   map<string, string>::const_iterator iter = mParseMessages.find(pFileDescriptor->getDatasetLocation());
   if (iter != mParseMessages.end() && iter->second.empty() == false)
   {
      errorMessage += iter->second;
   }

   const qint64 actualSize = QFile(QString::fromStdString(pFileDescriptor->getFilename().getFullPathAndName())).size();
   const qint64 maxSize = numeric_limits<ossim_uint64>::max() & numeric_limits<std::streamoff>::max();
   if (actualSize > maxSize)
   {
      errorMessage += "This file exceeds the maximum supported size for this platform.\n"
         "Data at the end of the file may be missing or incorrect.\n"
         "IMPORT DATA FROM THIS FILE WITH THE KNOWLEDGE THAT IT IS NOT FULLY SUPPORTED.\n";
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
         errorMessage += "The ICORDS is not a supported value.\n";
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
      errorMessage += "The lookup table will not be applied.\n";
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
         errorMessage += "THIS FILE CONTAINS INVALID CLASSIFICATION INFORMATION!\n"
            "The image has a higher classification level than the file.\n"
            "Update the Classification information before proceeding.\n";
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
               errorMessage += "THIS FILE CONTAINS INVALID CLASSIFICATION INFORMATION!\n" +
                  desStr.str() + " has a higher classification level than the file.\n"
                  "Update the Classification information before proceeding.\n";
            }
         }
      }
   }

   return true;
}

bool Nitf::NitfImporter::runOperationalTests(Progress* pProgress, ostream& failure)
{
   return runAllTests(pProgress, failure);
}

bool Nitf::NitfImporter::runAllTests(Progress* pProgress, ostream& failure)
{
   FactoryResource<DynamicObject> pMetadata;
   vector<double> centerConst;
   centerConst.push_back(10);
   centerConst.push_back(20);
   centerConst.push_back(30);

   vector<double> startConst;
   startConst.push_back(5);
   startConst.push_back(19.75);
   startConst.push_back(26.85);

   vector<double> endConst;
   endConst.push_back(15);
   endConst.push_back(20.25);
   endConst.push_back(33.15);

   vector<double> fwhmConst;
   fwhmConst.push_back(10);
   fwhmConst.push_back(.5);
   fwhmConst.push_back(6.3);

   vector<double> center;
   vector<double> start;
   vector<double> end;
   vector<double> fwhm;

   // center/fwhm --> start/end
   center = centerConst;
   start.clear();
   end.clear();
   fwhm = fwhmConst;
   updateSpecialMetadata(pMetadata.get(), center, start, end, fwhm);
   if (start != startConst || end != endConst)
   {
      failure << "Failed to compute start/end wavelengths from center/fwhm" << endl;
      return false;
   }

   // center/start --> end
   center = centerConst;
   start = startConst;
   end.clear();
   fwhm.clear();
   updateSpecialMetadata(pMetadata.get(), center, start, end, fwhm);
   if (end != endConst)
   {
      failure << "Failed to compute end wavelengths from center/start" << endl;
      return false;
   }

   // center/end --> start
   center = centerConst;
   start.clear();
   end = endConst;
   fwhm.clear();
   updateSpecialMetadata(pMetadata.get(), center, start, end, fwhm);
   if (start != startConst)
   {
      failure << "Failed to compute start wavelengths from center/end" << endl;
      return false;
   }

   // start/fwhm --> center/end
   center.clear();
   start = startConst;
   end.clear();
   fwhm = fwhmConst;
   updateSpecialMetadata(pMetadata.get(), center, start, end, fwhm);
   if (center != centerConst || end != endConst)
   {
      failure << "Failed to compute center/end wavelengths from start/fwhm" << endl;
      return false;
   }

   // start/end --> center
   center.clear();
   start = startConst;
   end = endConst;
   fwhm.clear();
   updateSpecialMetadata(pMetadata.get(), center, start, end, fwhm);
   if (center != centerConst)
   {
      failure << "Failed to compute center wavelengths from start/end" << endl;
      return false;
   }

   // end/fwhm --> center/start
   center.clear();
   start.clear();
   end = endConst;
   fwhm = fwhmConst;
   updateSpecialMetadata(pMetadata.get(), center, start, end, fwhm);
   if (center != centerConst || start != startConst)
   {
      failure << "Failed to compute center/start wavelengths from end/fwhm" << endl;
      return false;
   }

   return true;
}
