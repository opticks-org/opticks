/*
 * The information in this file is
 * Copyright(c) 2012 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"
#if defined (JPEG2000_SUPPORT)

#include "AppVerify.h"
#include "AppVersion.h"
#include "DataAccessorImpl.h"
#include "DimensionDescriptor.h"
#include "FileResource.h"
#include "Jpeg2000Exporter.h"
#include "Jpeg2000Utilities.h"
#include "LayerList.h"
#include "MessageLogResource.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "RasterDataDescriptor.h"
#include "RasterFileDescriptor.h"
#include "RasterLayer.h"
#include "RasterUtilities.h"
#include "SpatialDataView.h"
#include "StringUtilities.h"
#include "UtilityServices.h"

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QString>

// These must be in this order
#include <openjpeg.h>
#include <opj_includes.h>
#include <j2k.h>
#include <jp2.h>

REGISTER_PLUGIN_BASIC(OpticksPictures, Jpeg2000Exporter);

Jpeg2000Exporter::Jpeg2000Exporter()
{
   setName("Jpeg2000 Exporter");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setExtensions("Jpeg2000 files (*.jp2 *.j2k)");
   setShortDescription("Jpeg2000");
   setSubtype(TypeConverter::toString<RasterElement>());
   setDescriptorId("{9DE18FE8-DC11-4BDC-8211-18F70696B01F}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
   addDependencyCopyright("OpenJPEG", Service<UtilityServices>()->getTextFromFile(":/licenses/openjpeg"));
   addDependencyCopyright("proj4", Service<UtilityServices>()->getTextFromFile(":/licenses/proj4"));
}

Jpeg2000Exporter::~Jpeg2000Exporter()
{}

bool Jpeg2000Exporter::getInputSpecification(PlugInArgList*& pArgList)
{
   Service<PlugInManagerServices> pPlugInManager;
   VERIFY(pPlugInManager.get() != NULL);
   
   pArgList = pPlugInManager->getPlugInArgList();
   VERIFY(pArgList != NULL);

   VERIFY(pArgList->addArg<Progress>(Executable::ProgressArg(), NULL, Executable::ProgressArgDescription()));
   VERIFY(pArgList->addArg<RasterElement>(Exporter::ExportItemArg(), NULL, "Data element to be exported."));
   VERIFY(pArgList->addArg<RasterFileDescriptor>(Exporter::ExportDescriptorArg(), NULL, "File descriptor for the output file."));
   VERIFY(pArgList->addArg<View>(Executable::ViewArg(), "View to be exported."));
   return true;
}

bool Jpeg2000Exporter::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Execute JPEG2000 Exporter", "app", "B7618920-ACED-4CDC-858D-15ACA2EB7903", "Export failed");
   VERIFY(pInArgList != NULL);

   // The progress object is used to report progress to the user
   Progress* pProgress = pInArgList->getPlugInArgValue<Progress>(Executable::ProgressArg());
   if (pProgress != NULL)
   {
      pProgress->updateProgress("JPEG2000 Exporter Started", 0, NORMAL);
   }

   // This is the filename to export.
   RasterFileDescriptor* pDestination = pInArgList->getPlugInArgValue<RasterFileDescriptor>(Exporter::ExportDescriptorArg());
   VERIFY(pDestination != NULL);
   pDestination->addToMessageLog(pStep.get());

   // This is the cube to export data from.
   RasterElement* pRaster = pInArgList->getPlugInArgValue<RasterElement>(Exporter::ExportItemArg());
   VERIFY(pRaster != NULL);

   // Only these data types are currently supported.
   RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(pRaster->getDataDescriptor());
   VERIFY(pDescriptor != NULL);

   unsigned int bandFactor = Jpeg2000Utilities::get_num_bands(pDescriptor->getDataType());
   if (bandFactor == 0)
   {
      const std::string message = "Unsupported data type for export.";
      if (pProgress != NULL)
      {
         pProgress->updateProgress(message, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, message);
      return false;
   }
   else if (bandFactor > 1 && pProgress != NULL)
   {
      pProgress->updateProgress("Data being encoded exceeds 16 bits in length."
         "This file will be saved but may not be viewable from other applications.", 10, WARNING);
   }

   // Get dimensions.
   const unsigned int rowCount = pDestination->getRowCount();
   const unsigned int columnCount = pDestination->getColumnCount();
   unsigned int bandCount = pDestination->getBandCount();
   const std::vector<DimensionDescriptor>& exportRows = pDestination->getRows();
   const std::vector<DimensionDescriptor>& exportColumns = pDestination->getColumns();
   const std::vector<DimensionDescriptor>& exportBands = pDestination->getBands();
   VERIFY(exportRows.empty() == false && exportColumns.empty() == false && exportBands.empty() == false);

   // Ignore return values because this function may return false for data which was not subcubed.
   // In this case, use a default skip factor of 0 (i.e.: not subcubed).
   unsigned int rowSkip = 0;
   RasterUtilities::determineExportSkipFactor(exportRows, rowSkip);
   unsigned int columnSkip = 0;
   RasterUtilities::determineExportSkipFactor(exportColumns, columnSkip);

   DimensionDescriptor startRow = pDescriptor->getActiveRow(exportRows.front().getActiveNumber());
   DimensionDescriptor stopRow = pDescriptor->getActiveRow(exportRows.back().getActiveNumber());
   DimensionDescriptor startColumn = pDescriptor->getActiveColumn(exportColumns.front().getActiveNumber());
   DimensionDescriptor stopColumn = pDescriptor->getActiveColumn(exportColumns.back().getActiveNumber());
   DimensionDescriptor startBand = pDescriptor->getActiveBand(exportBands.front().getActiveNumber());
   DimensionDescriptor stopBand = pDescriptor->getActiveBand(exportBands.back().getActiveNumber());
   VERIFY(startRow.isActiveNumberValid() && stopRow.isActiveNumberValid() &&
      startColumn.isActiveNumberValid() && stopColumn.isActiveNumberValid() &&
      startBand.isActiveNumberValid() && stopBand.isActiveNumberValid());

   // As of version 1.5, OpenJPEG only supported lossless compression < 32 bits, so for 32-bit
   // data, add an extra band so that the upper and lower 16 bits can be stored separately.
   // The importer needs to recognize this, which is done by embedding the data type into the filename.
   // The comment field would have been used, but that was not read by OpenJPEG 1.5.
   QFileInfo filename(QString::fromStdString(pDestination->getFilename()));

   // Embed the data type into the file name as the first part of the extension.
   QString dataTypeStr = QString::fromStdString(StringUtilities::toXmlString(pDescriptor->getDataType()));
   if (filename.absoluteFilePath().contains(dataTypeStr, Qt::CaseSensitive) == false)
   {
      filename = QFileInfo(filename.dir(), filename.baseName() + "." + dataTypeStr + "." + filename.completeSuffix());
   }

   // Setup encoding parameters.
   // Most of these parameters are set to default values of image_to_j2k.exe, written by the OpenJPEG group.
   opj_cparameters_t parameters;
   opj_set_default_encoder_parameters(&parameters);
   parameters.cod_format = Jpeg2000Utilities::get_file_format(filename.absoluteFilePath().toStdString().c_str());
   parameters.tcp_mct = bandCount == 3 ? 1 : 0;
   parameters.numresolution = 6;
   parameters.tcp_rates[0] = 0;
   parameters.tcp_numlayers = 1;
   parameters.cp_disto_alloc = 1;
   strncpy(parameters.outfile, filename.absoluteFilePath().toStdString().c_str(), sizeof(parameters.outfile) - 1);

   OPJ_COLOR_SPACE colorSpace = bandCount >= 3 ? CLRSPC_SRGB : CLRSPC_GRAY;
   SpatialDataView* pView = dynamic_cast<SpatialDataView*>(pInArgList->getPlugInArgValue<View>(Executable::ViewArg()));
   if (pView != NULL)
   {
      LayerList* pLayerList = pView->getLayerList();
      if (pLayerList != NULL && pLayerList->getPrimaryRasterElement() == pRaster)
      {
         RasterLayer* pRasterLayer = dynamic_cast<RasterLayer*>(pLayerList->getLayer(RASTER, pRaster));
         if (pRasterLayer != NULL)
         {
            colorSpace = pRasterLayer->getDisplayMode() == RGB_MODE ? CLRSPC_SRGB : CLRSPC_GRAY;
         }
      }
   }

   // Add extra bands for data types larger than 16 bits.
   bandCount *= bandFactor;

   std::vector<opj_image_cmptparm_t> params(bandCount);
   memset(&params[0], 0, sizeof(params[0]) * params.size());
   for (std::vector<opj_image_cmptparm_t>::iterator iter = params.begin(); iter != params.end(); ++iter)
   {
      iter->prec = 16;
      iter->bpp = 32;
      iter->sgnd = 0;
      if (pDescriptor->getDataType() == INT2SBYTES || pDescriptor->getDataType() == INT1SBYTE)
      {
         // 32-bit Windows fails on 0x00008000 if the signed bit is set for INT4SBYTES.
         iter->sgnd = 1;
      }

      iter->dx = 1;
      iter->dy = 1;
      iter->w = columnCount;
      iter->h = rowCount;
   }

   // Copy the data from Opticks into a temporary buffer used by OpenJPEG.
   // This uses a memcpy so that floating-point data can be converted losslessly.
   if (pProgress != NULL)
   {
      pProgress->updateProgress("Copying data", 0, NORMAL);
   }

   opj_image_t* pImage = opj_image_create(bandCount, &params[0], colorSpace);
   pImage->x1 = columnCount;
   pImage->y1 = rowCount;

   // JPEG 2000 data is always stored BIP, so force interleave conversion and subset options here.
   FactoryResource<DataRequest> pRequest;
   pRequest->setRows(startRow, stopRow, 1);
   pRequest->setColumns(startColumn, stopColumn, 1);
   pRequest->setBands(startBand, stopBand);
   pRequest->setInterleaveFormat(BIP);
   DataAccessor da = pRaster->getDataAccessor(pRequest.release());

   // Simple copy of the data.
   // This step does not perform the shifting; it just copies the data
   // into the OpenJPEG structure. Bands which were artifically injected
   // will not be populated in this section of the code.
   for (unsigned int row = 0; row < rowCount; ++row)
   {
      for (unsigned int column = 0; column < columnCount; ++column)
      {
         VERIFY(da.isValid());
         unsigned int offset = row * columnCount + column;
         for (std::vector<DimensionDescriptor>::size_type i = 0; i < exportBands.size(); ++i)
         {
            // Destination:
            //    OpenJPEG data structure (taking into account artificial bands).
            int* pDst = &pImage->comps[i * bandFactor].data[offset];

            // Source:
            //    DataAccessor for the requested exportBand.
            VERIFY(exportBands[i].isActiveNumberValid());
            unsigned char* pSrc = static_cast<unsigned char*>(da->getColumn()) +
               exportBands[i].getActiveNumber() * pDescriptor->getBytesPerElement();

            // Copy the entire element; bit shifting is handled later.
            memcpy(pDst, pSrc, pDescriptor->getBytesPerElement());
         }

         da->nextColumn(static_cast<int>(columnSkip) + 1);
      }

      da->nextRow(static_cast<int>(rowSkip) + 1);
   }

   // Now that the data has been copied, shift the upper bits,
   // moving them into the lower 16 bits of the following band.
   if (bandFactor > 1)
   {
      for (unsigned int band = 0; band < bandCount; band += bandFactor)
      {
         for (unsigned int row = 0; row < rowCount; ++row)
         {
            for (unsigned int column = 0; column < columnCount; ++column)
            {
               unsigned int offset = row * columnCount + column;
               for (unsigned int newBand = 1; newBand < bandFactor; ++newBand)
               {
                  pImage->comps[band + newBand].data[offset] =
                     pImage->comps[band].data[offset] >> (16 * (newBand));

                  // Remove sign extension.
                  pImage->comps[band + newBand].data[offset] &= 0x0000FFFF;
               }

               pImage->comps[band].data[offset] &= 0x0000FFFF;
            }
         }
      }
   }

   opj_cinfo_t* pCinfo = NULL;
   switch (parameters.cod_format)
   {
      case Jpeg2000Utilities::J2K_CFMT:
      {
         pCinfo = opj_create_compress(CODEC_J2K);
         break;
      }

      case Jpeg2000Utilities::JP2_CFMT:
      {
         pCinfo = opj_create_compress(CODEC_JP2);
         break;
      }

      default:
      {
         const std::string message = "Unrecognized format";
         if (pProgress != NULL)
         {
            pProgress->updateProgress(message, 0, ERRORS);
         }

         pStep->finalize(Message::Failure, message);
         return false;
      }
   }

   if (pProgress != NULL)
   {
      pProgress->updateProgress("Encoding data", 10, NORMAL);
   }

   opj_event_mgr_t eventMgr;
   memset(&eventMgr, 0, sizeof(opj_event_mgr_t));
   eventMgr.error_handler = Jpeg2000Utilities::error_callback;
   eventMgr.warning_handler = Jpeg2000Utilities::warning_callback;
   eventMgr.info_handler = Jpeg2000Utilities::info_callback;

   opj_set_event_mgr((opj_common_ptr)pCinfo, &eventMgr, stderr);
   opj_setup_encoder(pCinfo, &parameters, pImage);
   opj_cio_t* pCio = opj_cio_open((opj_common_ptr)pCinfo, NULL, 0);
   if (opj_encode(pCinfo, pCio, pImage, NULL) == 0)
   {
      opj_cio_close(pCio);
      opj_destroy_compress(pCinfo);
      const std::string message = "Encoding failed";
      if (pProgress != NULL)
      {
         pProgress->updateProgress(message, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, message);
      return false;
   }

   if (pProgress != NULL)
   {
      pProgress->updateProgress("Writing file", 80, NORMAL);
   }

   int codestream_length = cio_tell(pCio);
   FileResource pFile(parameters.outfile, "wb");
   if (pFile.get() == NULL)
   {
      opj_cio_close(pCio);
      opj_destroy_compress(pCinfo);
      const std::string message = "Opening the output file for writing failed";
      if (pProgress != NULL)
      {
         pProgress->updateProgress(message, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, message);
      return false;
   }

   if (fwrite(pCio->buffer, 1, codestream_length, pFile) < static_cast<size_t>(codestream_length))
   {
      opj_cio_close(pCio);
      opj_destroy_compress(pCinfo);
      const std::string message = "Writing to the output file failed";
      if (pProgress != NULL)
      {
         pProgress->updateProgress(message, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, message);
      return false;
   }

   opj_cio_close(pCio);
   opj_destroy_compress(pCinfo);

   if (pProgress != NULL)
   {
      pProgress->updateProgress("Export complete.", 100, NORMAL);
   }

   pStep->finalize(Message::Success);
   return true;
}

#endif
