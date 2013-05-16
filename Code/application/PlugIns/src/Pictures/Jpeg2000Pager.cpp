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
#include "DataRequest.h"
#include "DimensionDescriptor.h"
#include "Jpeg2000Pager.h"
#include "Jpeg2000Utilities.h"
#include "ObjectResource.h"
#include "PlugInArgList.h"
#include "PlugInRegistration.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterFileDescriptor.h"
#include "StringUtilities.h"

#include <QtCore/QString>
#include <QtCore/QStringList>

#include <limits>

REGISTER_PLUGIN_BASIC(OpticksPictures, Jpeg2000Pager);

size_t Jpeg2000Pager::msMaxCacheSize = 1024 * 1024 * 50; // Specify a cache size (50MB) larger than the default
                                                         // to minimize the number of calls to decode the image

Jpeg2000Pager::Jpeg2000Pager() :
   CachedPager(msMaxCacheSize),
   mpFile(NULL),
   mOffset(0),
   mSize(0)
{
   setName("JPEG2000 Pager");
   setCopyright(APP_COPYRIGHT);
   setCreator("Ball Aerospace & Technologies Corp.");
   setDescription("Provides access to on-disk JPEG2000 data");
   setDescriptorId("{CC0E8FBD-13AB-4b58-A8AC-4B27269C6E11}");
   setVersion(APP_VERSION_NUMBER);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
   setShortDescription("JPEG2000");
}

Jpeg2000Pager::~Jpeg2000Pager()
{
   if (mpFile != NULL)
   {
      fclose(mpFile);
      mpFile = NULL;
   }
}

std::string Jpeg2000Pager::offsetArg()
{
   static std::string sArgName = "Offset";
   return sArgName;
}

std::string Jpeg2000Pager::sizeArg()
{
   static std::string sArgName = "Size";
   return sArgName;
}

bool Jpeg2000Pager::getInputSpecification(PlugInArgList*& pArgList)
{
   if (CachedPager::getInputSpecification(pArgList) == false)
   {
      return false;
   }

   VERIFY(pArgList != NULL);
   VERIFY(pArgList->addArg<uint64_t>(Jpeg2000Pager::offsetArg(), &mOffset, "The offset into the file in bytes "
      "where the JPEG2000 data begins.  If no value is provided, the data is presumed to be located at the "
      "beginning of the file."));
   VERIFY(pArgList->addArg<uint64_t>(Jpeg2000Pager::sizeArg(), &mSize, "The size in bytes of the JPEG2000 data "
      "contained in the file.  If no value is provided, the data is presumed to continue from the offset to "
      "the end of the file."));

   return true;
}

bool Jpeg2000Pager::parseInputArgs(PlugInArgList* pArgList)
{
   if (CachedPager::parseInputArgs(pArgList) == false)
   {
      return false;
   }

   VERIFY(pArgList != NULL);
   VERIFY(pArgList->getPlugInArgValue<uint64_t>(Jpeg2000Pager::offsetArg(), mOffset));
   VERIFY(pArgList->getPlugInArgValue<uint64_t>(Jpeg2000Pager::sizeArg(), mSize));

   return true;
}

bool Jpeg2000Pager::openFile(const std::string& filename)
{
   if ((mpFile != NULL) || (filename.empty() == true))
   {
      return false;
   }

   mpFile = fopen(filename.c_str(), "rb");
   return (mpFile != NULL);
}

CachedPage::UnitPtr Jpeg2000Pager::fetchUnit(DataRequest* pOriginalRequest)
{
   if (pOriginalRequest == NULL)
   {
      return CachedPage::UnitPtr();
   }

   // Check for interleave conversions, which are not supported by this pager
   const RasterElement* pRaster = getRasterElement();
   VERIFYRV(pRaster != NULL, CachedPage::UnitPtr());

   const RasterDataDescriptor* pDescriptor = dynamic_cast<const RasterDataDescriptor*>(pRaster->getDataDescriptor());
   VERIFYRV(pDescriptor != NULL, CachedPage::UnitPtr());

   const RasterFileDescriptor* pFileDescriptor =
      dynamic_cast<const RasterFileDescriptor*>(pDescriptor->getFileDescriptor());
   VERIFYRV(pFileDescriptor != NULL, CachedPage::UnitPtr());

   if (pFileDescriptor->getBandCount() > 1)
   {
      InterleaveFormatType requestedInterleave = pOriginalRequest->getInterleaveFormat();
      InterleaveFormatType fileInterleave = pFileDescriptor->getInterleaveFormat();
      if (requestedInterleave != fileInterleave)
      {
         return CachedPage::UnitPtr();
      }

      VERIFYRV(requestedInterleave == BIP, CachedPage::UnitPtr());   // The JPEG2000 data is stored BIP
   }

   // Get and validate the extents of the data to be loaded
   DimensionDescriptor startRow = pOriginalRequest->getStartRow();
   DimensionDescriptor stopRow = pOriginalRequest->getStopRow();
   unsigned int concurrentRows = pOriginalRequest->getConcurrentRows();

   DimensionDescriptor startColumn = pOriginalRequest->getStartColumn();
   DimensionDescriptor stopColumn = pOriginalRequest->getStopColumn();
   unsigned int concurrentColumns = pOriginalRequest->getConcurrentColumns();

   DimensionDescriptor startBand = pOriginalRequest->getStartBand();
   DimensionDescriptor stopBand = pOriginalRequest->getStopBand();

   if ((startRow.isOnDiskNumberValid() == false) || (stopRow.isOnDiskNumberValid() == false) ||
      (startColumn.isOnDiskNumberValid() == false) || (stopColumn.isOnDiskNumberValid() == false) ||
      (startBand.isOnDiskNumberValid() == false) || (stopBand.isOnDiskNumberValid() == false))
   {
      return CachedPage::UnitPtr();
   }

   if ((startRow.getOnDiskNumber() > stopRow.getOnDiskNumber()) ||
      (startColumn.getOnDiskNumber() > stopColumn.getOnDiskNumber()) ||
      (startBand.getOnDiskNumber() > stopBand.getOnDiskNumber()))
   {
      return CachedPage::UnitPtr();
   }

   if ((startRow.getActiveNumber() + concurrentRows - 1) > stopRow.getActiveNumber())
   {
      concurrentRows = stopRow.getActiveNumber() - startRow.getActiveNumber() + 1;
   }

   if ((startColumn.getActiveNumber() + concurrentColumns - 1) > stopColumn.getActiveNumber())
   {
      concurrentColumns = stopColumn.getActiveNumber() - startColumn.getActiveNumber() + 1;
   }

   // Populate the image data based on the output data type
   EncodingType outputDataType = pDescriptor->getDataType();
   switch (outputDataType)
   {
   case INT1UBYTE:
      return populateImageData<unsigned char>(startRow, startColumn, concurrentRows, concurrentColumns);

   case INT1SBYTE:
      return populateImageData<signed char>(startRow, startColumn, concurrentRows, concurrentColumns);

   case INT2UBYTES:
      return populateImageData<unsigned short>(startRow, startColumn, concurrentRows, concurrentColumns);

   case INT2SBYTES:
      return populateImageData<signed short>(startRow, startColumn, concurrentRows, concurrentColumns);

   case INT4UBYTES:
      return populateImageData<unsigned int>(startRow, startColumn, concurrentRows, concurrentColumns);

   case INT4SBYTES:
      return populateImageData<signed int>(startRow, startColumn, concurrentRows, concurrentColumns);

   case FLT4BYTES:
      return populateImageData<float>(startRow, startColumn, concurrentRows, concurrentColumns);

   case FLT8BYTES:
      return populateImageData<double>(startRow, startColumn, concurrentRows, concurrentColumns);

   default:
      break;
   }

   return CachedPage::UnitPtr();
}

double Jpeg2000Pager::getChunkSize() const
{
   // Set the chunk size as the maximum cache size to minimize the number of calls to decode the image
   return msMaxCacheSize;
}

template <typename Out>
CachedPage::UnitPtr Jpeg2000Pager::populateImageData(const DimensionDescriptor& startRow,
                                                     const DimensionDescriptor& startColumn,
                                                     unsigned int concurrentRows, unsigned int concurrentColumns) const
{
   VERIFYRV(startRow.isOnDiskNumberValid() == true, CachedPage::UnitPtr());
   VERIFYRV(startColumn.isOnDiskNumberValid() == true, CachedPage::UnitPtr());
   VERIFYRV(concurrentRows > 0, CachedPage::UnitPtr());
   VERIFYRV(concurrentColumns > 0, CachedPage::UnitPtr());

   // Get the rows, colums, and bands to load
   unsigned int onDiskStartRow = startRow.getOnDiskNumber();
   unsigned int onDiskStopRow = onDiskStartRow + concurrentRows;
   unsigned int onDiskStartColumn = startColumn.getOnDiskNumber();
   unsigned int onDiskStopColumn = onDiskStartColumn + concurrentColumns;

   const RasterElement* pRaster = getRasterElement();
   VERIFYRV(pRaster != NULL, CachedPage::UnitPtr());

   const RasterDataDescriptor* pDescriptor = dynamic_cast<const RasterDataDescriptor*>(pRaster->getDataDescriptor());
   VERIFYRV(pDescriptor != NULL, CachedPage::UnitPtr());

   const RasterFileDescriptor* pFileDescriptor =
      dynamic_cast<const RasterFileDescriptor*>(pDescriptor->getFileDescriptor());
   VERIFYRV(pFileDescriptor != NULL, CachedPage::UnitPtr());

   const std::vector<DimensionDescriptor>& allBands = pFileDescriptor->getBands();
   if (allBands.empty() == true)
   {
      return CachedPage::UnitPtr();
   }

   // Create the output data
   unsigned int numPixels = concurrentRows * concurrentColumns * allBands.size();
   unsigned int numBytes = numPixels * getBytesPerBand();

   if (numPixels > static_cast<unsigned int>(std::numeric_limits<int>::max()))   // ArrayResource only allocates up
                                                                                 // to INT_MAX number of values
   {
      return CachedPage::UnitPtr();
   }

   ArrayResource<Out> pDestination(numPixels, true);
   char* pDest = reinterpret_cast<char*>(pDestination.get());
   if (pDest == NULL)
   {
      return CachedPage::UnitPtr();
   }

   memset(pDest, 0, numPixels);

   // Decode the image from the file, first trying the codestream format then the file format
   opj_image_t* pImage = decodeImage(onDiskStartRow, onDiskStartColumn, onDiskStopRow, onDiskStopColumn,
      Jpeg2000Utilities::J2K_CFMT);
   if (pImage == NULL)
   {
      pImage = decodeImage(onDiskStartRow, onDiskStartColumn, onDiskStopRow, onDiskStopColumn,
         Jpeg2000Utilities::JP2_CFMT);
   }

   if (pImage == NULL)
   {
      return CachedPage::UnitPtr();
   }

   // Populate the output image data
   int bandFactor = 1;

   std::string filename = pRaster->getFilename();
   if (filename.empty() == false)
   {
      QStringList parts = QString::fromStdString(filename).split('.');
      foreach (QString part, parts)
      {
         bool error;
         EncodingType dataType = StringUtilities::fromXmlString<EncodingType>(part.toStdString(), &error);
         if (dataType.isValid() == true && error == false)
         {
            int currentBandFactor = Jpeg2000Utilities::get_num_bands(dataType);
            if (currentBandFactor > 0)
            {
               bandFactor = currentBandFactor;
               break;
            }
         }
      }
   }

   const size_t copySize = pDescriptor->getBytesPerElement() / bandFactor;

   for (unsigned int r = 0; r < concurrentRows; ++r)
   {
      for (unsigned int c = 0; c < concurrentColumns; ++c)
      {
         for (std::vector<DimensionDescriptor>::size_type b = 0; b < allBands.size(); ++b)
         {
            for (int f = 0; f < bandFactor; ++f)
            {
               unsigned int componentIndex = b * bandFactor + f;
               int imageWidth = pImage->comps[componentIndex].w;
               int imageHeight = pImage->comps[componentIndex].h;

               int columnSpan = 1;
               if (imageWidth != static_cast<int>(concurrentColumns))
               {
                  columnSpan = pImage->comps[componentIndex].dx;
               }

               int rowSpan = 1;
               if (imageHeight != static_cast<int>(concurrentRows))
               {
                  rowSpan = pImage->comps[componentIndex].dy;
               }

               VERIFYRV(columnSpan > 0, CachedPage::UnitPtr());
               VERIFYRV(rowSpan > 0, CachedPage::UnitPtr());

               if ((r < static_cast<unsigned int>(imageHeight) * rowSpan) &&
                  (c < static_cast<unsigned int>(imageWidth) * columnSpan))
               {
                  int dataIndex = ((r / rowSpan) * imageWidth) + (c / columnSpan);
                  memcpy(pDest, &pImage->comps[componentIndex].data[dataIndex], copySize);
               }

               pDest += copySize; // Increment by up to 16 bits (2 bytes).
            }
         }
      }
   }

   // Destroy the decoded image data
   opj_image_destroy(pImage);

   // Transfer ownership of the resulting data into a new page which will be owned by the caller of this method
   return CachedPage::UnitPtr(new CachedPage::CacheUnit(reinterpret_cast<char*>(pDestination.release()), startRow,
      static_cast<int>(concurrentRows), numBytes));
}

opj_image_t* Jpeg2000Pager::decodeImage(unsigned int originalStartRow, unsigned int originalStartColumn,
                                        unsigned int originalStopRow, unsigned int originalStopColumn,
                                        int decoderType) const
{
   // Open a byte stream of the required size
   size_t fileLength = 0;
   if (mSize > 0)
   {
      fileLength = static_cast<size_t>(mSize);
   }
   else
   {
      fseek(mpFile, 0, SEEK_END);

      size_t fileSize = static_cast<size_t>(ftell(mpFile));
      if (fileSize <= mOffset)
      {
         return NULL;
      }

      fileLength = fileSize - static_cast<size_t>(mOffset);
   }

   opj_stream_t* pStream = opj_stream_create_file_stream(mpFile, fileLength, true);
   if (pStream == NULL)
   {
      return NULL;
   }

   opj_stream_set_user_data_length(pStream, fileLength);

   // Seek to the required position in the file
   fseek(mpFile, static_cast<long>(mOffset), SEEK_SET);

   // Create the appropriate codec
   opj_codec_t* pCodec = NULL;
   switch (decoderType)
   {
   case Jpeg2000Utilities::J2K_CFMT:
      pCodec = opj_create_decompress(OPJ_CODEC_J2K);
      break;

   case Jpeg2000Utilities::JP2_CFMT:
      pCodec = opj_create_decompress(OPJ_CODEC_JP2);
      break;

   default:
      opj_stream_destroy(pStream);
      return NULL;
   }

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

   // Set the portion of the image to decode
   if (opj_set_decode_area(pCodec, pImage, originalStartColumn, originalStartRow, originalStopColumn,
      originalStopRow) == OPJ_FALSE)
   {
      opj_stream_destroy(pStream);
      opj_destroy_codec(pCodec);
      return NULL;
   }

   // Decode the image
   int success = opj_decode(pCodec, pStream, pImage);

   // Cleanup
   opj_stream_destroy(pStream);
   opj_destroy_codec(pCodec);

   if (success == OPJ_FALSE)
   {
      opj_image_destroy(pImage);
      return NULL;
   }

   return pImage;
}
