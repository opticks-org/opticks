/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "GeoTiffPager.h"
#include "GeoTiffPage.h"

#include "AppVersion.h"
#include "bmutex.h"
#include "AppVerify.h"
#include "DataRequest.h"
#include "Filename.h"
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

using namespace std;

namespace GeoTiffOnDisk
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

CacheUnit *Cache::getCacheUnit(unsigned int startBlock, unsigned int endBlock, size_t blockSize)
{
   // create a vector of block numbers which we require
   vector<unsigned int> blocks(endBlock + 1 - startBlock, 1);
   blocks[0] = startBlock;
   transform(blocks.begin() + 1, blocks.end(), blocks.begin(), blocks.begin() + 1, plus<int>());
   return getCacheUnit(blocks, blockSize);
}

CacheUnit *Cache::getCacheUnit(vector<unsigned int> &blocks, size_t blockSize)
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

bool Cache::CacheLocator(vector<unsigned int> blockNumbers, CacheUnit *pUnit)
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
};

bool Cache::CacheCleaner(const CacheUnit *pUnit)
{
   return (pUnit->references() == 0);
}

}; // namespace GeoTiffOnDisk

REGISTER_PLUGIN_BASIC(OpticksPictures, GeoTiffPager);

GeoTiffPager::GeoTiffPager() :
         RasterPagerShell(),
         mpRaster(NULL),
         mpTiff(NULL),
         mpMutex(new BMutex)
{
   mpMutex->MutexCreate();
   mpMutex->MutexInit();

   setName("GeoTiffPager");
   setCopyright(APP_COPYRIGHT);
   setCreator("Ball Aerospace & Technologies Corp.");
   setDescription("Provides access to on-disk GeoTIFF data");
   setDescriptorId("{ACB77E7A-1EE4-476d-8859-FCDFEAD487EB}");
   setVersion(APP_VERSION_NUMBER);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
   setShortDescription("On-disk GeoTIFF");
}

GeoTiffPager::~GeoTiffPager()
{
   mpMutex->MutexDestroy();
   delete mpMutex;

   if (mpTiff != NULL)
   {
      TIFFClose(mpTiff);
      mpTiff = NULL;
   }
}

bool GeoTiffPager::getInputSpecification(PlugInArgList *&pArgList)
{
   pArgList = mpPluginSvcs->getPlugInArgList();
   VERIFY(pArgList != NULL);

   PlugInArg* pArg = mpPluginSvcs->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName("Raster Element");
   pArg->setType("RasterElement");
   pArg->setDefaultValue(NULL);
   pArgList->addArg(*pArg);

   pArg = mpPluginSvcs->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName("Filename");
   pArg->setType("Filename");
   pArg->setDefaultValue(NULL);
   pArgList->addArg(*pArg);

   return true;
}

bool GeoTiffPager::getOutputSpecification(PlugInArgList *&pArgList)
{
   //----- this plugin has no outputs arguments.
   pArgList = NULL;

   return true;
}

bool GeoTiffPager::execute(PlugInArgList *pInputArgList, PlugInArgList *)
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

   // open the TIFF
   mpTiff = TIFFOpen(pFilename->getFullPathAndName().c_str(), "r");
   return (mpTiff != NULL);
}

RasterPage* GeoTiffPager::getPage(DataRequest *pOriginalRequest,
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

   GeoTiffPage* pPage(NULL);

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

      if ((interleave == BSQ) && (concurrentBands != 1))
      {
         throw string("BSQ data can only be accessed one band at a time.");
      }

      /**
       * possibilities for load
       *
       * BIP with strips
       *     with tiles
       * BSQ with strips
       *     with tiles
       **/

      if (TIFFIsTiled(mpTiff) == 0)
      {
         // the data are stored as strips
         tsize_t stripSize(TIFFStripSize(mpTiff)); // columns * bands * rows in a strip (bands=1 for BSQ)
         if (stripSize <= 0)
         {
            throw string("TIFF file error. Strip size <= 0.");
         }

         // Load in a data block and create a GeoTiffPage for the data.
         // we do this reasonably efficiently by loading complete strips even
         // if that means loading more data than needed

         // how many strips should we load?
         // find the strip that our data block begins in
         // then find the strip that our concurrent data ends in
         tstrip_t startStrip(TIFFComputeStrip(mpTiff, rowNumber, bandNumber));
         tstrip_t endStrip(TIFFComputeStrip(mpTiff, (rowNumber + concurrentRows) - 1, bandNumber));

         // if this data block is already in the cache, retrieve it...otherwise create a new block
         GeoTiffOnDisk::CacheUnit* pCacheUnit(mBlockCache.getCacheUnit(startStrip, endStrip, stripSize));
         if (pCacheUnit == NULL)
         {
            throw string("Can't create a cache unit");
         }
         if (interleave == BIP)
         {
            float rowsPerStrip(static_cast<float>(stripSize / numColumns) / static_cast<float>(numBands) /
               static_cast<float>(bytesPerElement));
            if (rowsPerStrip < 1.0)
            {
               pPage = new GeoTiffPage(pCacheUnit, 0 + (rowNumber * numBands * bytesPerElement) +
                                                           (bandNumber * bytesPerElement),
                                                           0, 0, 0);
            }
            else
            {
               const unsigned int uiRowsPerStrip = static_cast<unsigned int>(rowsPerStrip);
               const unsigned int numRowsOffset = rowNumber % uiRowsPerStrip;
               const unsigned int numRowsAvailable = (uiRowsPerStrip * (endStrip - startStrip + 1)) - numRowsOffset;
               pPage = new GeoTiffPage(pCacheUnit,
                           (numRowsOffset * numColumns * numBands * bytesPerElement) +
                           (colNumber * numBands * bytesPerElement) + (bandNumber * bytesPerElement),
                           numRowsAvailable, 0, 0);
            }
         }
         else if (interleave == BSQ)
         {
            float rowsPerStrip(static_cast<float>(stripSize / numColumns) / static_cast<float>(bytesPerElement));
            if (rowsPerStrip < 1.0)
            {
               pPage = new GeoTiffPage(pCacheUnit, 0 + colNumber * bytesPerElement, 0, 0, 0);
            }
            else
            {
               const unsigned int uiRowsPerStrip = static_cast<unsigned int>(rowsPerStrip);
               const unsigned int numRowsOffset = rowNumber % uiRowsPerStrip;
               const unsigned int numRowsAvailable = (uiRowsPerStrip * (endStrip - startStrip + 1)) - numRowsOffset;
               pPage = new GeoTiffPage(pCacheUnit,
                  (numRowsOffset * numColumns * bytesPerElement) + (colNumber * bytesPerElement),
                  numRowsAvailable, 0, 0);
            }
         }

         if (pPage == NULL)
         {
            throw string("Can't create a data block");
         }
         if (pCacheUnit->isEmpty())
         {
            // we need to load this block
            char* pData(pCacheUnit->data());
            if (pData == NULL)
            {
               throw string("Data block has no data");
            }
            // read in the strips
            char* pBlockPos(pData);
            for (tstrip_t curStrip = startStrip; curStrip <= endStrip; curStrip++)
            {
               tsize_t bytesRead = TIFFReadEncodedStrip(mpTiff, curStrip, pBlockPos, -1);
               if (bytesRead == -1)
               {
                  throw string("Error reading TIFF data");
               }
               pBlockPos += bytesRead;
            }

            pCacheUnit->setIsEmpty(false);
         }
      }
      else
      {
         // The number of pixels in each tile from left to right
         uint32 tileWidth;
         if (TIFFGetField(mpTiff, TIFFTAG_TILEWIDTH, &tileWidth) == 0 || tileWidth == 0)
         {
            throw string("Cannot determine tileWidth");
         }

         // The number of pixels in each tile from top to bottom
         uint32 tileLength;
         if (TIFFGetField(mpTiff, TIFFTAG_TILELENGTH, &tileLength) == 0 || tileLength == 0)
         {
            throw string("Cannot determine tileLength");
         }

         // The number of bytes in each tile
         const tsize_t tileSize(TIFFTileSize(mpTiff));
         if (tileSize <= 0)
         {
            throw string("Cannot determine tileSize");
         }

         // Compute the first and last tile to load.
         // This should ideally use TIFFComputeTile, but there are problems
         // in that method (as of libtiff 3.8.1) so compute them manually here.

         // The number of tiles from left to right
         const uint32 tilesAcross((numColumns + tileWidth - 1) / tileWidth);
         if (tilesAcross == 0)
         {
            throw string("Cannot determine tilesAcross");
         }

         // The number of tiles from top to bottom
         const uint32 tilesDown((numRows + tileLength - 1) / tileLength);
         if (tilesDown == 0)
         {
            throw string("Cannot determine tilesDown");
         }

         // The offset to the first tile in the requested band
         // BIP: Tiles contain all bands, so this is 0
         // BSQ: Tiles contain a single band, so this is number of tiles per band * bandNumber
         // This is stated in the TIFF 6.0 spec on page 68 (in the TileOffsets definition)
         const uint32 tileOffset(interleave == BIP ? 0 : bandNumber * tilesAcross * tilesDown);

         // The 0-based index of the first desired tile
         const ttile_t startTileIndex(rowNumber / tileLength);

         // The tile containing the first column of the first requested row
         const ttile_t startTile(tileOffset + startTileIndex * tilesAcross);
         if (startTile < 0)
         {
            throw string("Cannot determine startTile");
         }

         // The 0-based index of the last desired tile
         const ttile_t endTileIndex((rowNumber + concurrentRows - 1) / tileLength);

         // The tile containing the last column of the last requested row
         const ttile_t endTile(tileOffset + endTileIndex * tilesAcross + tilesAcross - 1);
         if (endTile < 0)
         {
            throw string("Cannot determine endTile");
         }

         // Retrieve a block from the cache
         GeoTiffOnDisk::CacheUnit* pCacheUnit(mBlockCache.getCacheUnit(startTile, endTile, tileSize));
         if (pCacheUnit == NULL)
         {
            throw string("Cannot create a cache unit");
         }

         // The number of bands in pPage
         const unsigned int bandSkip(interleave == BIP ? numBands : 1);

         // The number of columns in pPage
         const unsigned int columnSkip(numColumns);

         // The offset of the first requested data within pPage
         const size_t offset(bytesPerElement *
            ((rowNumber % tileLength) * columnSkip * bandSkip + (interleave == BSQ ? 0 : bandNumber)));

         // The number of rows in pPage
         const unsigned int rowSkip(tileLength * ((endTile - startTile + 1) / tilesAcross));

         // Create a GeoTiffPage based on the computed values
         pPage = new GeoTiffPage(pCacheUnit, offset, rowSkip, columnSkip, bandSkip);
         if (pCacheUnit->isEmpty())
         {
            // Temporary storage for the working tile
            vector<unsigned char> tileData(tileSize);
            for (ttile_t curTile = startTile, tileNum = 0; curTile <= endTile; ++curTile, ++tileNum)
            {
               if (TIFFReadEncodedTile(mpTiff, curTile, &tileData[0], tileSize) != tileSize)
               {
                  throw string("Error reading TIFF data");
               }

               // The starting address of this tile within pPage
               char* pBlockPos(pCacheUnit->data());

               // Increment by one or more rows of tiles
               pBlockPos += tileSize * tilesAcross * bandSkip * bytesPerElement * (tileNum / tilesAcross);

               // Increment by one or more tiles within a row
               pBlockPos += tileWidth * bandSkip * bytesPerElement * (tileNum % tilesAcross);

               // The number of bytes to copy - this might be different for partial tiles (e.g.: at the end of a row)
               size_t numBytesToCopy = tileWidth;
               if ((tileNum + 1) % tilesAcross == 0 && numColumns % tileWidth != 0)
               {
                  numBytesToCopy = numColumns % tileWidth;
               }

               numBytesToCopy *= bandSkip * bytesPerElement;
               for (uint32 row = 0; row < tileLength; ++row)
               {
                  const size_t rowOffset = row * numColumns * bandSkip * bytesPerElement;
                  const size_t tileOffset = row * tileWidth * bandSkip * bytesPerElement;
                  memcpy(pBlockPos + rowOffset, &tileData[tileOffset], numBytesToCopy);
               }
            }

            pCacheUnit->setIsEmpty(false);
         }
      }
   }
   catch (const string& exc)
   {
      delete pPage;
      pPage = NULL;
      if (exc.empty() == false)
      {
         MessageResource pMsg("GeoTIFF Pager Error", "app", "71B84E03-6AF1-4971-9F83-69E3C8BC6BF3");
         pMsg->addProperty("Message", exc);
      }
   }

   //unlock the mutex now
   mpMutex->MutexUnlock();

   return pPage;
}

void GeoTiffPager::releasePage(RasterPage *pPage)
{
   if (pPage == NULL)
   {
      return;
   }

   //ensure only one thread enters this code at a time
   mpMutex->MutexLock();

   GeoTiffPage* pGeoTiffPage = static_cast<GeoTiffPage*>(pPage);
   delete pGeoTiffPage;

   //unlock the mutex now
   mpMutex->MutexUnlock();
}

int GeoTiffPager::getSupportedRequestVersion() const
{
   return 1;
}
