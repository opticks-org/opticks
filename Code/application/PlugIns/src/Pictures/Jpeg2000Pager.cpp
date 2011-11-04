/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"
#if defined (JPEG2000_SUPPORT)

#include "AppVerify.h"
#include "AppVersion.h"
#include "bmutex.h"
#include "DataRequest.h"
#include "Filename.h"
#include "Jpeg2000Pager.h"
#include "Jpeg2000Page.h"
#include "MessageLogResource.h"
#include "ModelServices.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterFileDescriptor.h"

#include <functional>
#include <algorithm>

#include <QtCore/QFileInfo>

// These must be in this order
#include <openjpeg.h>
#include <opj_includes.h>
#include <j2k.h>
#include <jp2.h>

using namespace std;

enum Jpeg2000FileTypeEnum {J2K_CFMT, JP2_CFMT};
typedef EnumWrapper<Jpeg2000FileTypeEnum> Jpeg2000FileType;

int getCodec(const char* pFilename)
{
   QFileInfo info(pFilename);
   if (info.suffix() == "jp2")
   {
      return JP2_CFMT;
   }
   else if (info.suffix() == "j2k")
   {
      return J2K_CFMT;
   }
   return -1;
}

void error_callback(const char* msg, void *client_data) {
#ifdef DEBUG
   FILE* stream = (FILE*)client_data;
   fprintf(stream, "[ERROR] %s", msg);
#endif
}

void warning_callback(const char* msg, void* client_data) {
#ifdef DEBUG
   FILE* stream = (FILE*)client_data;
   fprintf(stream, "[WARNING] %s", msg);
#endif
}

void info_callback(const char* msg, void* client_data) {
#ifdef DEBUG
   (void)client_data;
   fprintf(stdout, "[INFO] %s", msg);
#endif
}

namespace Jpeg2000Cache
{

CacheUnit::CacheUnit(const vector<unsigned int>& blockNumbers, size_t blockSize) :
   mReferenceCount(0),
   mBlockNumbers(blockNumbers),
   mDataSize(blockSize),
   mpData(NULL),
   mIsEmpty(true)
{
   mpData = mpModelSvcs->getMemoryBlock(mDataSize);
}

CacheUnit::~CacheUnit()
{
   if (mpData != NULL)
   {
      mpModelSvcs->deleteMemoryBlock(mpData);
      mpData = NULL;
   }
}

void CacheUnit::get()
{
   mReferenceCount++;
}

void CacheUnit::release()
{
   if (mReferenceCount > 0)
   {
      mReferenceCount--;
   }
}

unsigned int CacheUnit::references() const
{
   return mReferenceCount;
}

const vector<unsigned int>& CacheUnit::blockNumbers() const
{
   return mBlockNumbers;
}

size_t CacheUnit::dataSize() const
{
   return mDataSize;
}

char* CacheUnit::data() const
{
   return mpData;
}

bool CacheUnit::isEmpty() const
{
   return mIsEmpty;
}

void CacheUnit::setIsEmpty(bool v)
{
   mIsEmpty = v;
}

Cache::Cache() : mCacheSize(8)
{
}

Cache::~Cache()
{
   for (cache_t::iterator it = mCache.begin(); it != mCache.end(); ++it)
   {
      if (*it != NULL)
      {
         delete *it;
      }
   }
   mCache.clear();
}

CacheUnit* Cache::getCacheUnit(unsigned int startBlock, unsigned int endBlock, size_t blockSize)
{
   // create a vector of block numbers which we require
   vector<unsigned int> blocks(endBlock + 1 - startBlock, 1);
   blocks[0] = startBlock;
   transform(blocks.begin() + 1, blocks.end(), blocks.begin(), blocks.begin() + 1, plus<int>());
   return getCacheUnit(blocks, blockSize);
}

CacheUnit* Cache::getCacheUnit(vector<unsigned int> &blocks, size_t blockSize)
{
   size_t dataSize(blocks.size() * blockSize);

   // find or create the needed cache blocks
   CacheUnit* returnUnit(NULL);
   cache_t::iterator locate_it(find_if(mCache.begin(), mCache.end(), bind1st(ptr_fun(Cache::CacheLocator), blocks)));
   if (locate_it != mCache.end())
   {
      // we found the CacheUnit
      returnUnit = *locate_it;
      mCache.erase(locate_it);
   }
   else
   {
      // we need to create a new CacheUnit
      // is the cache full?
      if (mCache.size() >= mCacheSize)
      {
         // remove the first available unit
         cache_t::iterator clean_it(find_if(mCache.begin(), mCache.end(), Cache::CacheCleaner));
         if (clean_it != mCache.end())
         {
            if (*clean_it != NULL)
            {
               delete *clean_it;
            }
            mCache.erase(clean_it);
         }
      }
      returnUnit = new CacheUnit(blocks, dataSize);
   }
   if (returnUnit != NULL)
   {
      if (returnUnit->data() == NULL)
      {
         delete returnUnit;
         returnUnit = NULL;
      }
      else
      {
         mCache.push_back(returnUnit);
         returnUnit->get();
      }
   }
   return returnUnit;
}

bool Cache::CacheLocator(vector<unsigned int> blockNumbers, CacheUnit* pUnit)
{
   // cases:
   //  does this CacheUnit contain the exact blocks we need?
   //  does this CacheUnit contain more blocks than we need?
   //  does this CacheUnit contain fewer blocks than we need?
   //  does this CacheUnit contain none of the blocks we need?
   // for now, only the exact blocks will be used
   if (blockNumbers.size() != pUnit->blockNumbers().size())
   {
      return false;
   }
   return equal(blockNumbers.begin(), blockNumbers.end(), pUnit->blockNumbers().begin());
}

bool Cache::CacheCleaner(const CacheUnit* pUnit)
{
   return (pUnit->references() == 0);
}

}; // namespace Cache

REGISTER_PLUGIN_BASIC(OpticksPictures, Jpeg2000Pager);

Jpeg2000Pager::Jpeg2000Pager() :
         RasterPagerShell(),
         mpRaster(NULL),
         mpJpeg2000(NULL),
         mpMutex(new BMutex)
{
   mpMutex->MutexCreate();
   mpMutex->MutexInit();

   setName("Jpeg2000Pager");
   setCopyright(APP_COPYRIGHT);
   setCreator("Ball Aerospace & Technologies Corp.");
   setDescription("Provides access to on-disk JPEG2000 data");
   setDescriptorId("{CC0E8FBD-13AB-4b58-A8AC-4B27269C6E11}");
   setVersion(APP_VERSION_NUMBER);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
   setShortDescription("Jpeg2000");
}

Jpeg2000Pager::~Jpeg2000Pager()
{
   mpMutex->MutexDestroy();
   delete mpMutex;

   if (mpJpeg2000 != NULL)
   {
      fclose(mpJpeg2000);
      mpJpeg2000 = NULL;
   }
}

bool Jpeg2000Pager::getInputSpecification(PlugInArgList* &pArgList)
{
   VERIFY((pArgList = mpPluginSvcs->getPlugInArgList()) != NULL);
   VERIFY(pArgList->addArg<RasterElement>("Raster Element", NULL));
   VERIFY(pArgList->addArg<Filename>("Filename", NULL));
   return true;
}

bool Jpeg2000Pager::getOutputSpecification(PlugInArgList *&pArgList)
{
   // This plugin has no output arguments.
   pArgList = NULL;

   return true;
}

bool Jpeg2000Pager::execute(PlugInArgList* pInputArgList, PlugInArgList *)
{
   if ((mpRaster != NULL) || (pInputArgList == NULL))
   {
      return false;
   }

   //Get PlugIn Arguments
   RasterElement* pRaster = pInputArgList->getPlugInArgValue<RasterElement>("Raster Element");
   if (pRaster == NULL)
   {
      return false;
   }

   //Get Filename argument
   Filename* pFilename = pInputArgList->getPlugInArgValue<Filename>("Filename");
   if (pFilename == NULL)
   {
      return false;
   }

   //Done getting PlugIn Arguments
   
   mpRaster = pRaster;
   mDecoderType = getCodec(pFilename->getFileName().c_str());

   // open the JPEG2000
   mpJpeg2000 = fopen((pFilename->getFullPathAndName()).c_str(), "rb");
   return (mpJpeg2000 != NULL);
}

RasterPage* Jpeg2000Pager::getPage(DataRequest* pOriginalRequest,
      DimensionDescriptor startRow, 
      DimensionDescriptor startColumn, 
      DimensionDescriptor startBand)
{
   if (mpRaster == NULL || pOriginalRequest == NULL)
   {
      return NULL;
   }

   if (pOriginalRequest->getWritable())
   {
      return NULL;
   }

   Jpeg2000Page* pPage(NULL);
   opj_dinfo_t* pDinfo = NULL;
   opj_codestream_info_t cstrInfo;
   opj_image_t* pImage = NULL;

   // ensure only one thread enters this code at a time
   mpMutex->MutexLock();

   // we wrap all this in a try/catch so we can have one exit point
   // and ensure that the mutex gets unlocked on an error
   try
   {
      const RasterDataDescriptor* pDescriptor =
         dynamic_cast<const RasterDataDescriptor*>(mpRaster->getDataDescriptor());
      if (pDescriptor == NULL)
      {
         throw string("Invalid Data Descriptor");
      }

      const RasterFileDescriptor* pFileDescriptor =
         dynamic_cast<const RasterFileDescriptor*>(pDescriptor->getFileDescriptor());
      if (pFileDescriptor == NULL)
      {
         throw string("Invalid File Descriptor");
      }

      InterleaveFormatType interleave = pFileDescriptor->getInterleaveFormat();
      unsigned int numBands = pFileDescriptor->getBandCount();
      unsigned int numRows = pFileDescriptor->getRowCount();
      unsigned int numColumns = pFileDescriptor->getColumnCount();
      unsigned int bytesPerElement = pDescriptor->getBytesPerElement();

      unsigned int concurrentRows = pOriginalRequest->getConcurrentRows();
      unsigned int concurrentColumns = pOriginalRequest->getConcurrentColumns();
      unsigned int concurrentBands = pOriginalRequest->getConcurrentBands();

      unsigned int rowNumber = startRow.getOnDiskNumber();
      unsigned int colNumber = startColumn.getOnDiskNumber();
      unsigned int bandNumber = startBand.getOnDiskNumber();

      // make sure the request is valid
      if ((rowNumber >= numRows) || ((rowNumber + concurrentRows) > numRows) ||
         (colNumber >= numColumns) || ((colNumber + concurrentColumns) > numColumns) ||
         (bandNumber >= numBands) || ((bandNumber + concurrentBands) > numBands))
      {
         // Since it is acceptable to increment off the end of the data, do not report an error message
         throw string();
      }

      Jpeg2000Cache::CacheUnit* pCacheUnit(mBlockCache.getCacheUnit(0, numBands - 1,
                                             numRows * numColumns * 4 * numBands));

      // if this data block is already in the cache, retrieve it...otherwise create a new block
      if (pCacheUnit == NULL)
      {
         throw string("Can't create a cache unit");
      }

      if (interleave == BIP)
      {
         pPage = new Jpeg2000Page(pCacheUnit, (rowNumber * numColumns * numBands * bytesPerElement) +
            (colNumber * numBands * bytesPerElement) + (bandNumber * bytesPerElement), numRows, numColumns, numBands);
      }

      if (pCacheUnit->isEmpty())
      {
         // we need to load this block
         char* pData(pCacheUnit->data());
         if (pData == NULL)
         {
            throw string("Data block has no data");
         }
         opj_dparameters_t parameters;
         opj_event_mgr_t eventMgr;
         vector<unsigned char> pSrc(NULL);
         int fileLength;
         opj_cio_t* pCio = NULL;

         // configure the event callbacks
         memset(&eventMgr, 0, sizeof(opj_event_mgr_t));
         eventMgr.error_handler = error_callback;
         eventMgr.warning_handler = warning_callback;
         eventMgr.info_handler = info_callback;

         // set decoding parameters to default values
         opj_set_default_decoder_parameters(&parameters);

         fseek(mpJpeg2000, 0, SEEK_END);
         fileLength = ftell(mpJpeg2000);
         fseek(mpJpeg2000, 0, SEEK_SET);
         pSrc.resize(fileLength);
         fread(&pSrc[0], 1, fileLength, mpJpeg2000);

         // decode the code-stream 
         switch(mDecoderType) 
         {
            case J2K_CFMT: 
               pDinfo = opj_create_decompress(CODEC_J2K);
               break;
            case JP2_CFMT: 
               pDinfo = opj_create_decompress(CODEC_JP2);
               break;
            default:
               break;

         }

         // catch events using our callbacks
         opj_set_event_mgr((opj_common_ptr)pDinfo, &eventMgr, stderr);

         // setup the decoder decoding parameters
         opj_setup_decoder(pDinfo, &parameters);

         // open a byte stream
         pCio = opj_cio_open((opj_common_ptr)pDinfo, &pSrc[0], fileLength);

         // decode the stream and fill the image structure
         pImage = opj_decode_with_info(pDinfo, pCio, &cstrInfo);
         if(!pImage)
         {
            throw string("Invalid JPEG2000 Image");
         }

         // close the byte stream
         opj_cio_close(pCio);

         // populate pData
         for (unsigned int i = 0; i < numRows * numColumns; i++)
         {
            for(unsigned int j = 0; j < numBands; ++j)
            {
//#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : This code will need to changed if Openjpeg is upgraded (mconsidi)")

               // This allows for proper operation when the library puts one-byte data into an unsigned int
               pData[(i * numBands + j)*4] = 0xFF & pImage->comps[j].data[i];
            }
         }
         pCacheUnit->setIsEmpty(false);

         // free remaining structures
         if(pDinfo) 
         {
            opj_destroy_decompress(pDinfo);
         }
         opj_destroy_cstr_info(&cstrInfo);
         opj_image_destroy(pImage);
      }
   }
   catch (const string& exc)
   {
      releasePage(pPage);
      pPage = NULL;
      if(pDinfo) 
      {
         opj_destroy_decompress(pDinfo);
      }
      opj_destroy_cstr_info(&cstrInfo);
      opj_image_destroy(pImage);
      if (exc.empty() == false)
      {
         MessageResource pMsg("Jpeg2000 Pager Error", "app", "CCC48CDE-DF3A-43d6-A750-85BF9BAD25A1");
         pMsg->addProperty("Message", exc);
      }
   }

   //unlock the mutex now
   mpMutex->MutexUnlock();
   return pPage;
}

void Jpeg2000Pager::releasePage(RasterPage* pPage)
{
   if (pPage == NULL)
   {
      return;
   }

   //ensure only one thread enters this code at a time
   mpMutex->MutexLock();

   Jpeg2000Page* pJpeg2000Page = static_cast<Jpeg2000Page*>(pPage);
   delete pJpeg2000Page;

   //unlock the mutex now
   mpMutex->MutexUnlock();
}

int Jpeg2000Pager::getSupportedRequestVersion() const
{
   return 1;
}

#endif 