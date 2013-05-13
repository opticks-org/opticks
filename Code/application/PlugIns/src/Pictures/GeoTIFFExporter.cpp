/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <math.h>
#include <stdio.h>

#include <geotiff.h>
#include <geovalues.h>
#include <geo_tiffp.h>
#include <geo_keyp.h>

#include "AppVersion.h"
#include "AppVerify.h"
#include "DataAccessorImpl.h"
#include "DimensionDescriptor.h"
#include "DynamicObject.h"
#include "GeoTIFFExporter.h"
#include "GeoTiffExportOptionsWidget.h"
#include "MessageLogResource.h"
#include "OptionsTiffExporter.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "Progress.h"
#include "RasterElement.h"
#include "RasterDataDescriptor.h"
#include "RasterFileDescriptor.h"
#include "StringUtilities.h"

using namespace std;

namespace
{
   int getTiffSampleFormat(EncodingType type)
   {
      switch (type)
      {
      case INT1UBYTE:
      case INT2UBYTES:
      case INT4UBYTES:
         return SAMPLEFORMAT_UINT;
      case INT1SBYTE:
      case INT2SBYTES:
      case INT4SBYTES:
         return SAMPLEFORMAT_INT;
      case FLT4BYTES:
      case FLT8BYTES:
         return SAMPLEFORMAT_IEEEFP;
      default:
         break;
      }
      return SAMPLEFORMAT_VOID;
   }

};

REGISTER_PLUGIN_BASIC(OpticksPictures, GeoTIFFExporter);

GeoTIFFExporter::GeoTIFFExporter() :
   mpStep(NULL),
   mpProgress(NULL),
   mpRaster(NULL),
   mpFileDescriptor(NULL),
   mAbortFlag(false),
   mRowsPerStrip(OptionsTiffExporter::getSettingRowsPerStrip())
{
   setName("GeoTIFF Exporter");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setDescription("Exports raster data in the TIFF file format with GeoTIFF support data if available.");
   setExtensions("TIFF Files (*.tif *.tiff)");
   setSubtype(TypeConverter::toString<RasterElement>());
   setDescriptorId("{6CFD45D6-564B-4c45-B907-FBD3AE8E2FD4}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

GeoTIFFExporter::~GeoTIFFExporter()
{}

bool GeoTIFFExporter::execute(PlugInArgList* pInParam, PlugInArgList* pOutParam)
{
   StepResource pStep("Execute GeoTIFF Exporter", "app", "0ACFD6AC-D673-47e8-A1A7-24B4D6455B86");
   mpStep = pStep.get();

   // Progress
   mpProgress = pInParam->getPlugInArgValue<Progress>(Executable::ProgressArg());

   // Sensor data
   mpRaster = pInParam->getPlugInArgValue<RasterElement>(Exporter::ExportItemArg());
   if (mpRaster == NULL)
   {
      mMessage = "The raster element input value is invalid!";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(mMessage, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, mMessage);
      return false;
   }

   if (isBatch() == true)
   {
      pInParam->getPlugInArgValue("Rows Per Strip", mRowsPerStrip);
   }
   else if (mpOptionWidget.get() != NULL)
   {
      mRowsPerStrip = mpOptionWidget->getRowsPerStrip();
   }

   // Check for complex data
   EncodingType dataType;
   const RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(mpRaster->getDataDescriptor());
   if (pDescriptor != NULL)
   {
      dataType = pDescriptor->getDataType();
   }

   if ((dataType == INT4SCOMPLEX) || (dataType == FLT8COMPLEX))
   {
      mMessage = "Complex data is not supported!";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(mMessage, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, mMessage);
      return false;
   }

   pDescriptor->addToMessageLog(pStep.get());

   // File descriptor
   mpFileDescriptor = pInParam->getPlugInArgValue<RasterFileDescriptor>(Exporter::ExportDescriptorArg());
   if (mpFileDescriptor == NULL)
   {
      mMessage = "The file descriptor input value is invalid!";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(mMessage, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, mMessage);
      return false;
   }

   Message* pMessage = pStep->addMessage("Export File Parameters", "app", "1CB336ED-A1A9-42F2-AE83-7179EF919432");
   mpFileDescriptor->addToMessageLog(pMessage);
   pMessage->finalize();

   mMessage = "Start GeoTIFF Exporter";
   if (mpProgress)
   {
      mpProgress->updateProgress(mMessage, 0, NORMAL);
   }

   string filename = mpFileDescriptor->getFilename();
   mMessage = "File is: " + filename;
   if (mpProgress)
   {
      mpProgress->updateProgress(mMessage, 0, NORMAL);
   }

   TIFF* pOut = XTIFFOpen(filename.c_str(), "w");
   if (pOut == NULL)
   {
      mMessage = "Unable to open GeoTIFF file for writing!  Check folder permissions.";
      if (mpProgress)
      {
         mpProgress->updateProgress(mMessage, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, mMessage);
      return false;
   }

   bool success = writeCube(pOut);
   XTIFFClose(pOut);

   if (success)
   {
      mMessage = "GeoTIFF export complete.";
      if (mpProgress)
      {
         mpProgress->updateProgress(mMessage, 100, NORMAL);
      }

      pStep->finalize(Message::Success);
   }
   else
   {
      pStep->finalize(Message::Failure, mMessage);
      remove(filename.c_str());
   }

   return success;
}

bool GeoTIFFExporter::abort()
{
   mAbortFlag = true;
   return true;
}

bool GeoTIFFExporter::hasAbort()
{
   return true;
}

bool GeoTIFFExporter::getInputSpecification(PlugInArgList*& pArgList)
{
   Service<PlugInManagerServices> pPlugInManager;
   VERIFY(pPlugInManager.get() != NULL);

   pArgList = pPlugInManager->getPlugInArgList();
   VERIFY(pArgList != NULL);

   VERIFY(pArgList->addArg<Progress>(Executable::ProgressArg(), NULL, Executable::ProgressArgDescription()));
   VERIFY(pArgList->addArg<RasterElement>(Exporter::ExportItemArg(), NULL, "Data element to be exported."));
   VERIFY(pArgList->addArg<RasterFileDescriptor>(Exporter::ExportDescriptorArg(), NULL, "File descriptor for the output file."));
   if (isBatch() == true)
   {
      VERIFY(pArgList->addArg<unsigned int>("Rows Per Strip", mRowsPerStrip, "Rows per strip for the TIFF file."));
   }

   return true;
}

bool GeoTIFFExporter::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

QWidget* GeoTIFFExporter::getExportOptionsWidget(const PlugInArgList* pInArgList)
{
   if (mpOptionWidget.get() == NULL)
   {
      mpOptionWidget.reset(new GeoTiffExportOptionsWidget());
   }

   return mpOptionWidget.get();
}

void GeoTIFFExporter::updateProgress(int current, int total, const string& progressString, ReportingLevel level)
{
   if (total <= 0)
   {
      total = 1;
   }

   int percentDone = percentDone = (current * 100 / total);
   if (percentDone >= 100)
   {
      percentDone = 99;
   }

   if (mpProgress)
   {
      mpProgress->updateProgress(progressString, percentDone, level);
   }
}

bool GeoTIFFExporter::writeCube(TIFF* pOut)
{
   if (pOut == NULL)
   {
      return false;
   }

   VERIFY(mpRaster != NULL);
   VERIFY(mpFileDescriptor != NULL);

   const RasterDataDescriptor* pDescriptor = dynamic_cast<const RasterDataDescriptor*>(mpRaster->getDataDescriptor());
   if (pDescriptor == NULL)
   {
      return false;
   }

   int size = 0;
   int row = 0;
   unsigned char* pBuffer = NULL;
   unsigned short numRows = pDescriptor->getRowCount();
   unsigned short numCols = pDescriptor->getColumnCount();
   unsigned short numBands = pDescriptor->getBandCount();
   unsigned short scols = mpFileDescriptor->getColumnCount();
   unsigned short srows = mpFileDescriptor->getRowCount();
   unsigned short sbands = mpFileDescriptor->getBandCount();

   FactoryResource<DataRequest> pRequest;
   pRequest->setInterleaveFormat(BIP);
   DataAccessor accessor = mpRaster->getDataAccessor(pRequest.release());
   if (!accessor.isValid())
   {
      mMessage = "Could not get a valid BIP accessor for this dataset.";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(mMessage, 0, ERRORS);
      }

      return false;
   }

   InterleaveFormatType eInterleave = pDescriptor->getInterleaveFormat();
   if (eInterleave != BIP)
   {
      mMessage = "Data will be saved in BIP format.";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(mMessage, 0, WARNING);
      }
   }

   unsigned int bytesPerElement(pDescriptor->getBytesPerElement());
   size = scols * sbands * bytesPerElement;

   TIFFSetField(pOut, TIFFTAG_IMAGEWIDTH, scols);
   TIFFSetField(pOut, TIFFTAG_IMAGELENGTH, srows);
   TIFFSetField(pOut, TIFFTAG_SAMPLESPERPIXEL, sbands);

   //for this tag, must multiply by # of bytes per data type
   TIFFSetField(pOut, TIFFTAG_BITSPERSAMPLE, static_cast<unsigned short>(bytesPerElement * 8));
   TIFFSetField(pOut, TIFFTAG_SAMPLEFORMAT, static_cast<unsigned short>(
                           getTiffSampleFormat(pDescriptor->getDataType())));

   bool packBits = OptionsTiffExporter::getSettingPackBitsCompression();
   if (mpOptionWidget.get() != NULL)
   {
      packBits = mpOptionWidget->getPackBitsCompression();
   }
   ttag_t compOpt = (packBits ? COMPRESSION_PACKBITS : COMPRESSION_NONE);

   TIFFSetField(pOut, TIFFTAG_COMPRESSION, compOpt);
   TIFFSetField(pOut, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
   TIFFSetField(pOut, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
   TIFFSetField(pOut, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);      //????
   TIFFSetField(pOut, TIFFTAG_ROWSPERSTRIP, mRowsPerStrip);

   //ready to test write
   mMessage = "Writing out GeoTIFF file...";
   if (mpProgress)
   {
      mpProgress->updateProgress( mMessage, 0, NORMAL);
   }

   if ((numRows == srows) && (numCols == scols) && (numBands == sbands))   // full cube write from memory
   {
      for (row = 0; row < srows; row++)
      {
         if (mAbortFlag)
         {
            mMessage = "GeoTIFF export aborted!";
            if (mpProgress != NULL)
            {
               mpProgress->updateProgress(mMessage, 0, ERRORS);
            }

            return false;
         }

         VERIFY(accessor.isValid());
         pBuffer = reinterpret_cast<unsigned char*>(accessor->getRow());
         if (pBuffer != NULL)
         {
            if (TIFFWriteScanline(pOut, pBuffer, row, size) < 0)
            {
               mMessage = "Unable to save GeoTIFF file, check folder permissions.";
               if (mpProgress)
               {
                  mpProgress->updateProgress(mMessage, 0, ERRORS);
               }

               return false;
            }

            updateProgress(row, srows, mMessage, NORMAL);
         }
         accessor->nextRow();
      }
   }
   else // subcube write
   {
      const vector<DimensionDescriptor>& rows = mpFileDescriptor->getRows();
      const vector<DimensionDescriptor>& columns = mpFileDescriptor->getColumns();
      const vector<DimensionDescriptor>& bands = mpFileDescriptor->getBands();

      unsigned int activeRowNumber = 0;
      unsigned int rowSize(0);
      unsigned int outRow(0);
      for (unsigned int r = 0; r < srows; ++r)
      {
         if (mAbortFlag == true)
         {
            mMessage = "GeoTIFF export aborted!";
            if (mpProgress != NULL)
            {
               mpProgress->updateProgress(mMessage, 0, ERRORS);
            }

            return false;
         }

         DimensionDescriptor rowDim = rows[r];
         if (rowDim.isActiveNumberValid())
         {
            // Skip to the next row
            for (; activeRowNumber < rowDim.getActiveNumber(); ++activeRowNumber)
            {
               accessor->nextRow();
            }

            VERIFY(accessor.isValid());

            vector<char> rowData(size);
            if (rowData.empty())
            {
               mMessage = "Error GeoTIFFExporter008: Unable to allocate row buffer.";
               if (mpProgress)
               {
                  mpProgress->updateProgress(mMessage, 0, ERRORS);
               }

               return false;
            }

            unsigned int activeColumnNumber = 0;

            char* pExportData = &(rowData.front());
            for (unsigned int c = 0; c < scols; ++c)
            {
               DimensionDescriptor columnDim = columns[c];
               if (columnDim.isActiveNumberValid())
               {
                  // Skip to the next column
                  for (; activeColumnNumber < columnDim.getActiveNumber(); ++activeColumnNumber)
                  {
                     accessor->nextColumn();
                  }

                  char* pCurrentPixel = reinterpret_cast<char*>(accessor->getColumn());

                  unsigned int activeBandNumber = 0;
                  for (unsigned int b = 0; b < sbands; ++b)
                  {
                     DimensionDescriptor bandDim = bands[b];
                     if (bandDim.isActiveNumberValid())
                     {
                        // Skip to the next band
                        for (; activeBandNumber < bandDim.getActiveNumber(); ++activeBandNumber)
                        {
                           pCurrentPixel += bytesPerElement;
                        }

                        memcpy(pExportData, pCurrentPixel, bytesPerElement);
                        pExportData += bytesPerElement;
                        rowSize += bytesPerElement;
                     }
                  }
               }
               else // this column is not included, go to next column 
               {
                  accessor->nextColumn();
               }
            }

            if (rowSize > 0)
            {
               // write here
               if (TIFFWriteScanline(pOut, &rowData[0], outRow, size) < 0)
               {
                  mMessage = "Error GeoTIFFExporter006: Unable to save GeoTIFF file, check folder permissions.";
                  if (mpProgress)
                  {
                     mpProgress->updateProgress(mMessage, 0, ERRORS);
                  }

                  return false;
               }

               updateProgress(outRow++, srows, mMessage, NORMAL);
            }
         }
         else // this row is not included, go to next row
         {
            accessor->nextRow();
         }
      }
   }

   //assumed everything has been done correctly up to now
   //copy over Geo ref info if there are any, else
   //try to look for world file in same directory and apply
   if (!(applyWorldFile(pOut)))
   {
      if (!(CreateGeoTIFF(pOut)))
      {
         //no geo info found, where is it located?
         mMessage = "Geo data is unavailable and will not be written to the output file!";
         updateProgress(srows, srows, mMessage, WARNING);
         if (mpStep != NULL)
         {
            mpStep->addMessage(mMessage, "app", "9C1E7ADE-ADC4-468c-B15E-FEB53D5FEF5B", true);
         }
      }
   }

   return true;
}

bool GeoTIFFExporter::CreateGeoTIFF(TIFF* pOut)
{
   if ((mpFileDescriptor == NULL) || (mpRaster == NULL) || (mpRaster->isGeoreferenced() == false))
   {
      return false;
   }

   // Get the exported row and column extents
   const vector<DimensionDescriptor>& rows = mpFileDescriptor->getRows();
   const vector<DimensionDescriptor>& columns = mpFileDescriptor->getColumns();

   if ((rows.empty() == true) || (columns.empty() == true))
   {
      return false;
   }

   unsigned int startRow = 0;
   unsigned int endRow = rows.size() - 1;

   DimensionDescriptor startRowDim = rows.front();
   if (startRowDim.isActiveNumberValid() == true)
   {
      startRow = startRowDim.getActiveNumber();
   }

   DimensionDescriptor endRowDim = rows.back();
   if (endRowDim.isActiveNumberValid() == true)
   {
      endRow = endRowDim.getActiveNumber();
   }

   unsigned int startColumn = 0;
   unsigned int endColumn = columns.size() - 1;

   DimensionDescriptor startColumnDim = columns.front();
   if (startColumnDim.isActiveNumberValid() == true)
   {
      startColumn = startColumnDim.getActiveNumber();
   }

   DimensionDescriptor endColumnDim = columns.back();
   if (endColumnDim.isActiveNumberValid() == true)
   {
      endColumn = endColumnDim.getActiveNumber();
   }

   // Open the file for writing GeoTIFF tags
   GTIF* pGtif = GTIFNew(pOut);
   if (pGtif == NULL)
   {
      return false;
   }

   // Get the latitude/longitude values for each corner of a single pixel
   LocationType llPixel(startColumn, startRow);
   LocationType urPixel = llPixel + 1.0;

   LocationType llGeoCoord = mpRaster->convertPixelToGeocoord(llPixel);
   LocationType lrGeoCoord = mpRaster->convertPixelToGeocoord(LocationType(urPixel.mX, llPixel.mY));
   LocationType ulGeoCoord = mpRaster->convertPixelToGeocoord(LocationType(llPixel.mX, urPixel.mY));
   LocationType urGeoCoord = mpRaster->convertPixelToGeocoord(urPixel);

   // Determine if the raster data is orthorectified
   bool isOrthoRectified = false;
   bool hasMetadataTag = false;

   const DynamicObject* pMetadata = mpRaster->getMetadata();
   if (pMetadata != NULL)
   {
      try
      {
         isOrthoRectified = dv_cast<bool>(pMetadata->getAttribute("orthorectified"));
         hasMetadataTag = true;
      }
      catch (bad_cast&)
      {
         // The attribute is not present or is not a bool, so calculate isOrthoRectified
      }
   }

   if (hasMetadataTag == false)
   {
      if ((endRow > startRow) && (endColumn > startColumn))
      {
         // The chip's (0,0)
         LocationType startPixel = llPixel;
         LocationType startGeo = llGeoCoord;

         // The chip's (0,max)
         LocationType rowMax(startPixel.mX, endRow);
         LocationType rowMaxGeo = mpRaster->convertPixelToGeocoord(rowMax);

         // The chip's (max,0)
         LocationType colMax(endColumn, startPixel.mY);
         LocationType colMaxGeo = mpRaster->convertPixelToGeocoord(colMax);

         LocationType rowMaxCheckGeo(rowMaxGeo.mX, startGeo.mY);
         LocationType rowMaxCheck = mpRaster->convertGeocoordToPixel(rowMaxCheckGeo);
         LocationType colMaxCheckGeo(startGeo.mX, colMaxGeo.mY);
         LocationType colMaxCheck = mpRaster->convertGeocoordToPixel(colMaxCheckGeo);

         LocationType deltaRowMax = rowMaxCheck - rowMax;
         LocationType deltaColMax = colMaxCheck - colMax;

         isOrthoRectified = (deltaRowMax.length() < 0.5) && (deltaColMax.length() < 0.5);
      }
   }

   // If the data is orthorectified and the option is set, write out the tie point tag
   // and the latitude/longitude pixel scale tag
   OptionsTiffExporter::TransformationMethod transformationMethod =
      StringUtilities::fromXmlString<OptionsTiffExporter::TransformationMethod>(
      OptionsTiffExporter::getSettingTransformationMethod());
   if (mpOptionWidget.get() != NULL)
   {
      transformationMethod = mpOptionWidget->getTransformationMethod();
   }

   if ((isOrthoRectified == true) && (transformationMethod == OptionsTiffExporter::TIE_POINT_PIXEL_SCALE))
   {
      // Write out the raster origin for the tie point tag, which is the location of the first exported pixel
      double pTiepoints[6] = { 0.0, 0.0, 0.0, llGeoCoord.mY, llGeoCoord.mX, 0.0 };
      TIFFSetField(pOut, TIFFTAG_GEOTIEPOINTS, 6, pTiepoints);

      // The pixel scale tag value can include a negative scale for the latitude and/or longitude (GeoTIFF Format
      // Specification, Version 1.8.2, Section 2.6.1) to account for horizontal or vertical flipping.  A negative
      // latitude scale corresponds to an image with its origin in the lower left or lower right corner, while a
      // positive latitude scale corresponds to an image with its origin in the upper left or upper right corner.
      // Similarly for longitude scale.
      double pPixelSize[3] = { urGeoCoord.mY - llGeoCoord.mY, llGeoCoord.mX - urGeoCoord.mX, 0.0 };
      TIFFSetField(pOut, TIFFTAG_GEOPIXELSCALE, 3, pPixelSize);
   }
   else
   {
      // Write out the transformation matrix tag based on the latitude/longitude of a single pixel
      double a = lrGeoCoord.mY - llGeoCoord.mY;
      double b = ulGeoCoord.mY - llGeoCoord.mY;
      double d = llGeoCoord.mY;
      double e = lrGeoCoord.mX - llGeoCoord.mX;
      double f = ulGeoCoord.mX - llGeoCoord.mX;
      double h = llGeoCoord.mX;

      double tMatrix[16] =
      {
         a,   b,   0.0, d,
         e,   f,   0.0, h,
         0.0, 0.0, 0.0, 0.0,
         0.0, 0.0, 0.0, 1.0
      };

      TIFFSetField(pOut, GTIFF_TRANSMATRIX, 16, tMatrix);
   }

   // Write out other tags to specify a geographic coordinate system
   GTIFKeySet(pGtif, GTRasterTypeGeoKey, TYPE_SHORT, 1, RasterPixelIsArea);
   GTIFKeySet(pGtif, GTModelTypeGeoKey, TYPE_SHORT, 1, ModelGeographic);
   GTIFKeySet(pGtif, GeographicTypeGeoKey, TYPE_SHORT, 1, GCS_WGS_84);

   // Here we violate the GTIF abstraction to retarget on another file.
   // We should just have a function for copying tags from one GTIF object
   // to another.
   pGtif->gt_tif = pOut;
   pGtif->gt_flags |= FLAG_FILE_MODIFIED;

   // Install keys and tags
   GTIFWriteKeys(pGtif);
   GTIFFree(pGtif);

   return true;
}

bool GeoTIFFExporter::applyWorldFile(TIFF *pOut)
{
   FILE* pTfw = NULL;
   double pPixsize[3];
   double xoff;
   double yoff;
   double pTiepoint[6];
   double x_rot;
   double y_rot;
   double pAdfMatrix[16];

   DataDescriptor* pDescriptor = mpRaster->getDataDescriptor();
   if (pDescriptor == NULL)
   {
      return false;
   }

   FileDescriptor* pFileDescriptor = pDescriptor->getFileDescriptor();
   if (pFileDescriptor == NULL)
   {
      return false;
   }

   const Filename& filename = pFileDescriptor->getFilename();
   string path = filename.getPath();
   string title = filename.getTitle();

   std::string worldFilename = path + SLASH + title + ".tfw";

   pTfw = fopen(worldFilename.c_str(), "rt");
   if (pTfw == NULL)
   {
      return false;
   }

   fscanf(pTfw, "%lf", pPixsize + 0);
   fscanf(pTfw, "%lf", &y_rot);
   fscanf(pTfw, "%lf", &x_rot);
   fscanf(pTfw, "%lf", pPixsize + 1);
   fscanf(pTfw, "%lf", &xoff);
   fscanf(pTfw, "%lf", &yoff);
   fclose(pTfw);

   // Write out pixel scale, and tiepoint information.
   if ((x_rot == 0.0) && (y_rot == 0.0))
   {
      pPixsize[2] = 0.0;
      TIFFSetField(pOut, GTIFF_PIXELSCALE, 3, pPixsize);
      pTiepoint[0] = 0.5;
      pTiepoint[1] = 0.5;
      pTiepoint[2] = 0.0;
      pTiepoint[3] = xoff;
      pTiepoint[4] = yoff;
      pTiepoint[5] = 0.0;
      TIFFSetField(pOut, GTIFF_TIEPOINTS, 6, pTiepoint);
   }
   else
   {
      memset(pAdfMatrix, 0, sizeof(double) * 16);
      pAdfMatrix[0] = pPixsize[0];
      pAdfMatrix[1] = x_rot;
      pAdfMatrix[3] = xoff - (pPixsize[0] + x_rot) * 0.5;
      pAdfMatrix[4] = y_rot;
      pAdfMatrix[5] = pPixsize[1];
      pAdfMatrix[7] = yoff - (pPixsize[1] + y_rot) * 0.5;
      pAdfMatrix[15] = 1.0;

      TIFFSetField(pOut, TIFFTAG_GEOTRANSMATRIX, 16, pAdfMatrix);
   }

   return true;
}
