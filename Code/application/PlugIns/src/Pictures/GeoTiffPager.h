/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GEOTIFFPAGER_H
#define GEOTIFFPAGER_H

#include "DMutex.h"
#include "ModelServices.h"
#include "PlugInManagerServices.h"
#include "RasterPagerShell.h"
#include "tiffio.h"
#include "TypesFile.h"

#include <deque>

class GeoTiffPage;
class RasterElement;

namespace GeoTiffOnDisk
{

class CacheUnit
{
public:
   CacheUnit(const std::vector<unsigned int>& blockNumbers, size_t blockSize);
   ~CacheUnit();

   void get();
   void release();
   unsigned int references() const;
   const std::vector<unsigned int>& blockNumbers() const;
   size_t dataSize() const;
   char* data() const;
   bool isEmpty() const;
   void setIsEmpty(bool v);

private:
   unsigned int mReferenceCount;
   std::vector<unsigned int> mBlockNumbers;
   size_t mDataSize;
   char* mpData;
   Service<ModelServices> mpModelSvcs;
   bool mIsEmpty;
};

class Cache
{
public:
   typedef std::deque<CacheUnit*> cache_t;

public:
   Cache();
   ~Cache();

   void initCacheSize(unsigned int cacheSize);

   CacheUnit* getCacheUnit(unsigned int startBlock, unsigned int endBlock, size_t blockSize);
   CacheUnit* getCacheUnit(std::vector<unsigned int>& blocks, size_t blockSize);

private:
   static bool CacheLocator(std::vector<unsigned int> blockNumbers, CacheUnit* pUnit);
   static bool CacheCleaner(const CacheUnit* pUnit);

   cache_t mCache;
   unsigned int mCacheSize;
   Service<ModelServices> mpModelSvcs;
};

}; // namespace

class GeoTiffPager : public RasterPagerShell
{
public:
   GeoTiffPager();
   ~GeoTiffPager();

   // Executable
   virtual bool getInputSpecification(PlugInArgList*& pArgList);
   virtual bool getOutputSpecification(PlugInArgList*& pArgList);
   virtual bool execute(PlugInArgList* pInputArgList, PlugInArgList* pOutputArgList);

   //RasterPager methods
   RasterPage* getPage(DataRequest* pOriginalRequest, DimensionDescriptor startRow,
      DimensionDescriptor startColumn, DimensionDescriptor startBand);
   void releasePage(RasterPage* pPage);

   int getSupportedRequestVersion() const;

protected:
   GeoTiffPage* getPage(tstrip_t startStrip, tstrip_t endStrip, tsize_t stripSize);

private:
   InterleaveFormatType mInterleave;
   unsigned int mRowCount;
   unsigned int mColumnCount;
   unsigned int mBandCount;
   unsigned int mBytesPerElement;
   TIFF* mpTiff;
   mta::DMutex mMutex;
   Service<PlugInManagerServices> mpPluginSvcs;
   Service<ModelServices> mpModelSvcs;
   GeoTiffOnDisk::Cache mBlockCache;
};

#endif
