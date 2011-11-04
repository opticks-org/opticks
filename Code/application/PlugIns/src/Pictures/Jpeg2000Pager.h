/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef JPEG2000PAGER_H
#define JPEG2000PAGER_H
#include "AppConfig.h"
#if defined (JPEG2000_SUPPORT)

#include "ModelServices.h"
#include "PlugInManagerServices.h"
#include "RasterPagerShell.h"
#include "tiffio.h"

#include <deque>

class Jpeg2000Page;
class Mutex;
class RasterElement;

namespace Jpeg2000Cache
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

      CacheUnit* getCacheUnit(unsigned int startBlock, unsigned int endBlock, size_t blockSize);
      CacheUnit* getCacheUnit(std::vector<unsigned int>& blocks, size_t blockSize);

   private:
      Cache& operator=(const Cache& rhs);

      static bool CacheLocator(std::vector<unsigned int> blockNumbers, CacheUnit* pUnit);
      static bool CacheCleaner(const CacheUnit* pUnit);

      cache_t mCache;
      const unsigned int mCacheSize;
      Service<ModelServices> mpModelSvcs;
   };

}; // namespace

class Jpeg2000Pager : public RasterPagerShell
{
public:
   Jpeg2000Pager();
   virtual ~Jpeg2000Pager();

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
   Jpeg2000Page* getPage(tstrip_t startStrip, tstrip_t endStrip, tsize_t stripSize);

private:
   Jpeg2000Pager& operator=(const Jpeg2000Pager& rhs);

   RasterElement* mpRaster;
   FILE* mpJpeg2000;
   Mutex* mpMutex;
   int mDecoderType;
   Service<PlugInManagerServices> mpPluginSvcs;
   Service<ModelServices> mpModelSvcs;
   Jpeg2000Cache::Cache mBlockCache;
};

#endif
#endif