/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <stdio.h>

#include <geotiff.h>
#include <geovalues.h>
#include <geo_normalize.h>
#include <xtiffio.h>

#include "AppVersion.h"
#include "GeoTIFFImporter.h"
#include "AppConfig.h"
#include "AppVerify.h"
#include "DataAccessorImpl.h"
#include "DimensionDescriptor.h"
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
#include "PlugInResource.h"
#include "QuickbirdIsd.h"
#include "RasterDataDescriptor.h"
#include "RasterFileDescriptor.h"
#include "RasterUtilities.h"
#include "SpatialDataView.h"

#include <errno.h>
#include <QtCore/QFileInfo>
#include <QtCore/QString>
#include <QtCore/QVariant>
using namespace std;

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

            EndianType fileEndianType = (fileEndian == tiffBigEndianMagicNumber ? BIG_ENDIAN : LITTLE_ENDIAN);
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

   // Check for unsupported tiled data
   if (TIFFIsTiled(pTiffFile) != 0)
   {
      return false;
   }

   // Check for unsupported palette data
   unsigned short photometric = 0;
   TIFFGetField(pTiffFile, TIFFTAG_PHOTOMETRIC, &photometric);

   if (photometric == PHOTOMETRIC_PALETTE)
   {
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
