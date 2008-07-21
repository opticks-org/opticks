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

#include "ModelServices.h"
#include "PlugInManagerServices.h"
#include "RasterPagerShell.h"
#include "tiffio.h"

#include <deque>

class GeoTiffPage;
class Mutex;
class RasterElement;

namespace GeoTiffOnDisk
{

class CacheUnit
{
   unsigned int mReferenceCount;
   std::vector<unsigned int> mBlockNumbers;
   size_t mDataSize;
   char *mpData;
   Service<ModelServices> mpModelSvcs;
   bool mIsEmpty;

public:
   CacheUnit(const std::vector<unsigned int> &blockNumber, size_t blockSize);
   ~CacheUnit();

   void get()                       { mReferenceCount++; }
   void release()                   
   {
      if(mReferenceCount > 0)
      {
         mReferenceCount--;
      }
   }
   unsigned int references() const  { return mReferenceCount; }
   const std::vector<unsigned int> &blockNumbers() const { return mBlockNumbers; }
   size_t dataSize() const          { return mDataSize; }
   char *data() const               { return mpData; }
   bool isEmpty() const             { return mIsEmpty; }
   void setIsEmpty(bool v)          { mIsEmpty = v; }
};

class Cache
{
public:
   typedef std::deque<CacheUnit*> cache_t;

public:
   Cache();
   ~Cache();

   CacheUnit *getCacheUnit(unsigned int startBlock, unsigned int endBlock, size_t blockSize);
   CacheUnit *getCacheUnit(std::vector<unsigned int> &blocks, size_t blockSize);

private:
   static bool Cache::CacheLocator(std::vector<unsigned int> blockNumbers, CacheUnit *pUnit);
   static bool Cache::CacheCleaner(const CacheUnit *pUnit);

   cache_t mCache;
   const unsigned int mCacheSize;
   Service<ModelServices> mpModelSvcs;
};

}; // namespace

class GeoTiffPager : public RasterPagerShell
{
public:
   GeoTiffPager();
   ~GeoTiffPager();

   // Executable
   virtual bool getInputSpecification(PlugInArgList *&pArgList);
   virtual bool getOutputSpecification(PlugInArgList *&pArgList);
   virtual bool execute(PlugInArgList *pInputArgList, 
                        PlugInArgList *pOutputArgList);

   //RasterPager methods
   RasterPage* getPage(DataRequest *pOriginalRequest,
      DimensionDescriptor startRow, 
      DimensionDescriptor startColumn, 
      DimensionDescriptor startBand);
   void releasePage(RasterPage *pPage);

   int getSupportedRequestVersion() const;

protected:
   GeoTiffPage *getPage(tstrip_t startStrip, tstrip_t endStrip, tsize_t stripSize);

private:
   RasterElement *mpRaster;
   TIFF *mpTiff;
   Mutex *mpMutex;
   Service<PlugInManagerServices> mpPluginSvcs;
   Service<ModelServices> mpModelSvcs;
   GeoTiffOnDisk::Cache mBlockCache;
};

#endif
