/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "AppVersion.h"
#include "DataRequest.h"
#include "DimensionDescriptor.h"
#include "Mie4NitfJpeg2000Pager.h"
#include "Mie4NitfPager.h"
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

REGISTER_PLUGIN_BASIC(OpticksNitf, Mie4NitfJpeg2000Pager);

size_t Mie4NitfJpeg2000Pager::msMaxCacheSize = 1024 * 1024 * 50; // Specify a cache size (50MB) larger than the default
                                                         // to minimize the number of calls to decode the image

Mie4NitfJpeg2000Pager::Mie4NitfJpeg2000Pager() :
   CachedPager(msMaxCacheSize),
   mpFile(NULL)
{
   setName("MIE4NITF JPEG2000 Pager");
   setCopyright(APP_COPYRIGHT);
   setCreator("Ball Aerospace & Technologies Corp.");
   setDescription("Provides access to on-disk MIE4NITF JPEG2000 data");
   setDescriptorId("{5B2CDFB6-3AAC-4405-AB44-6711DC33C78F}");
   setVersion(APP_VERSION_NUMBER);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

Mie4NitfJpeg2000Pager::~Mie4NitfJpeg2000Pager()
{
   if (mpFile != NULL)
   {
      fclose(mpFile);
      mpFile = NULL;
   }
}

bool Mie4NitfJpeg2000Pager::getInputSpecification(PlugInArgList*& pArgList)
{
   if (CachedPager::getInputSpecification(pArgList) == false)
   {
      return false;
   }

   VERIFY(pArgList != NULL);
   VERIFY(pArgList->addArg<std::vector<unsigned int> >(Nitf::Mie4NitfPager::StartFramesArg()));
   VERIFY(pArgList->addArg<std::vector<std::string> >(Nitf::Mie4NitfPager::FrameFilesArg()));
   VERIFY(pArgList->addArg<std::vector<uint64_t> >(Nitf::Mie4NitfPager::OffsetsArg()));
   VERIFY(pArgList->addArg<std::vector<uint64_t> >(Nitf::Mie4NitfPager::SizesArg()));

   return true;
}

bool Mie4NitfJpeg2000Pager::parseInputArgs(PlugInArgList* pArgList)
{
   if (CachedPager::parseInputArgs(pArgList) == false)
   {
      return false;
   }

   VERIFY(pArgList != NULL);

   std::vector<uint64_t> offset;
   if (!pArgList->getPlugInArgValue(Nitf::Mie4NitfPager::OffsetsArg(), offset))
   {
      return false;
   }

   std::vector<uint64_t> size;
   if (!pArgList->getPlugInArgValue(Nitf::Mie4NitfPager::SizesArg(), size))
   {
      return false;
   }

   std::vector<unsigned int> startFrames;
   if (!pArgList->getPlugInArgValue(Nitf::Mie4NitfPager::StartFramesArg(), startFrames))
   {
      return false;
   }
   std::vector<std::string> frameFiles;
   if (!pArgList->getPlugInArgValue(Nitf::Mie4NitfPager::FrameFilesArg(), frameFiles))
   {
      return false;
   }
   if (startFrames.size() == 0 || startFrames.size() != frameFiles.size() || startFrames.size() != offset.size() || startFrames.size() != size.size())
   {
      return false;
   }
   mFrameIndex.clear();
   auto sf = startFrames.begin();
   auto ff = frameFiles.begin();
   auto os = offset.begin();
   auto ss = size.begin();
   for (; sf != startFrames.end() && ff != frameFiles.end() && os != offset.end() && ss != size.end(); ++sf, ++ff, ++os, ++ss)
   {
      auto val = boost::make_tuple(*ff, *os, *ss);
      mFrameIndex.insert(std::make_pair(*sf, val));
   }

   return true;
}

bool Mie4NitfJpeg2000Pager::openFile(const std::string& filename)
{
   if (mpFile != nullptr)
   {
      fclose(mpFile);
      mpFile = nullptr;
   }
   if (filename.empty() == true)
   {
      return false;
   }

   mpFilename = const_cast<char *>(filename.c_str());
   mpFile = fopen(filename.c_str(), "rb");
   return (mpFile != NULL);
}

CachedPage::UnitPtr Mie4NitfJpeg2000Pager::fetchUnit(DataRequest* pOriginalRequest)
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

   // find the file and image segment corresponding to the band.
   auto frameIndex = mFrameIndex.rbegin();
   for (; frameIndex != mFrameIndex.rend(); ++frameIndex)
   {
      if (frameIndex->first <= startBand.getOnDiskNumber())
      {
         break;
      }
   }
   if (frameIndex == mFrameIndex.rend())
   {
      return CachedPage::UnitPtr();
   }
   auto fname = frameIndex->second.get<0>();
   if (!openFile(fname))
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
      return populateImageData<unsigned char>(startRow, startColumn, concurrentRows, concurrentColumns, frameIndex->second.get<1>(), frameIndex->second.get<2>());

   case INT1SBYTE:
      return populateImageData<signed char>(startRow, startColumn, concurrentRows, concurrentColumns, frameIndex->second.get<1>(), frameIndex->second.get<2>());

   case INT2UBYTES:
      return populateImageData<unsigned short>(startRow, startColumn, concurrentRows, concurrentColumns, frameIndex->second.get<1>(), frameIndex->second.get<2>());

   case INT2SBYTES:
      return populateImageData<signed short>(startRow, startColumn, concurrentRows, concurrentColumns, frameIndex->second.get<1>(), frameIndex->second.get<2>());

   case INT4UBYTES:
      return populateImageData<unsigned int>(startRow, startColumn, concurrentRows, concurrentColumns, frameIndex->second.get<1>(), frameIndex->second.get<2>());

   case INT4SBYTES:
      return populateImageData<signed int>(startRow, startColumn, concurrentRows, concurrentColumns, frameIndex->second.get<1>(), frameIndex->second.get<2>());

   case FLT4BYTES:
      return populateImageData<float>(startRow, startColumn, concurrentRows, concurrentColumns, frameIndex->second.get<1>(), frameIndex->second.get<2>());

   case FLT8BYTES:
      return populateImageData<double>(startRow, startColumn, concurrentRows, concurrentColumns, frameIndex->second.get<1>(), frameIndex->second.get<2>());

   default:
      break;
   }

   return CachedPage::UnitPtr();
}

double Mie4NitfJpeg2000Pager::getChunkSize() const
{
   // Set the chunk size as the maximum cache size to minimize the number of calls to decode the image
   return msMaxCacheSize;
}

template <typename Out>
CachedPage::UnitPtr Mie4NitfJpeg2000Pager::populateImageData(const DimensionDescriptor& startRow,
   const DimensionDescriptor& startColumn,
   unsigned int concurrentRows, unsigned int concurrentColumns,
   uint64_t offset, uint64_t size) const
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
   size_t numPixels = concurrentRows * concurrentColumns * allBands.size();
   size_t numBytes = numPixels * getBytesPerBand();

   std::unique_ptr<Out> pDestination(new(std::nothrow) Out[numPixels]);
   char* pDest = reinterpret_cast<char*>(pDestination.get());
   if (pDest == NULL)
   {
      return CachedPage::UnitPtr();
   }

   memset(pDest, 0, numPixels);

   // Decode the image from the file, first trying the codestream format then the file format
   opj_image_t* pImage = decodeImage(onDiskStartRow, onDiskStartColumn, onDiskStopRow, onDiskStopColumn,
      Jpeg2000Utilities::J2K_CFMT, offset, size);
   if (pImage == NULL)
   {
      pImage = decodeImage(onDiskStartRow, onDiskStartColumn, onDiskStopRow, onDiskStopColumn,
         Jpeg2000Utilities::JP2_CFMT, offset, size);
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
      foreach(QString part, parts)
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
         unsigned int b = 0;  // don't support multi-band frames currentlly
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

   // Destroy the decoded image data
   opj_image_destroy(pImage);

   // Transfer ownership of the resulting data into a new page which will be owned by the caller of this method
   return CachedPage::UnitPtr(new CachedPage::CacheUnit(reinterpret_cast<char*>(pDestination.release()), startRow,
      static_cast<int>(concurrentRows), numBytes));
}

opj_image_t* Mie4NitfJpeg2000Pager::decodeImage(unsigned int originalStartRow, unsigned int originalStartColumn,
   unsigned int originalStopRow, unsigned int originalStopColumn,
   int decoderType, uint64_t offset, uint64_t size) const
{
   // Open a byte stream of the required size
   size_t fileLength = 0;
   if (size > 0)
   {
      fileLength = static_cast<size_t>(size);
   }
   else
   {
      fseek(mpFile, 0, SEEK_END);

      size_t fileSize = static_cast<size_t>(ftell(mpFile));
      if (fileSize <= offset)
      {
         return NULL;
      }

      fileLength = fileSize - static_cast<size_t>(offset);
   }

#ifdef OPJ_STREAM_SEEK_STREAM_FOUND
   // Close filename as opj_stream_create_file_stream() will open it again and multiple fopen() are undefined
   fclose(mpFile);
   mpFile = NULL;
   opj_stream_t* pStream = opj_stream_create_file_stream(mpFilename, fileLength, true);
   if (pStream == NULL)
   {
      return NULL;
   }
   opj_stream_set_user_data_length(pStream, fileLength); // Isn't this redundant?
   opj_stream_seek_stream(pStream, offset);
#else
   // No OpenJpeg opj_stream_seek_stream() function:
   // 1. Seek into mpFile
   // 2. allocate a userData buffer
   // 3. read image data from mpFile into userData
   // 4. create a default opj stream
   // 5. set the stream's userData buffer to the one just allocated and filled.
   if(fseek(mpFile, offset, SEEK_SET))
   {
       perror(strerror(errno)); // How does Opticks handle this sort of error? What MessageLog?
       return NULL;
   }

   errno = 0;
   uchar* userData = static_cast<uchar*>(calloc(fileLength,1));
   if(userData == NULL)
   {
       perror(strerror(errno));  // How does Opticks handle this sort of error? What MessageLog?
       return NULL;
   }

   size_t numBytes(0);
   clearerr(mpFile);
   while((numBytes < fileLength) && !feof(mpFile) && !ferror(mpFile))
   {
       numBytes += fread(userData+numBytes, 1, fileLength-numBytes, mpFile);
   }
   if(ferror(mpFile))
   {
       free(userData);
       return NULL;
   }

   opj_stream_t* pStream = opj_stream_default_create(true); // doesn't do much, not likely to fail
   if (pStream == NULL)
   {
       free(userData);
       return NULL;
   }

   opj_stream_set_user_data(pStream, userData, free);
   opj_stream_set_user_data_length(pStream, std::min(fileLength,numBytes));  // numBytes should always equal fileLength
#endif

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
