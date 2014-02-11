/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "AppVerify.h"
#include "Filename.h"
#include "MemoryMappedArray.h"
#include "MemoryMappedArrayView.h"
#include "DMutex.h"
#include "PointCloudDataRequest.h"
#include "PointCloudMemoryMappedPager.h"
#include "PointDataBlock.h"

#include <algorithm>
using namespace std;

class MemoryMappedDataBlock : public PointDataBlock
{
public:
   MemoryMappedDataBlock(void* pData, uint32_t pointCount, MemoryMappedArrayView* pView) :
      mpRawData(pData),
      mPointCount(pointCount),
      mpArrayView(pView)
   {      
   }

   ~MemoryMappedDataBlock()
   {
      delete mpArrayView;
   }

   virtual void* getRawData()
   {
      return mpRawData;
   }

   virtual uint32_t getNumPoints()
   {
      return mPointCount;
   }

   MemoryMappedArrayView* getMemoryMappedArrayView()
   {
      return mpArrayView;
   }

private:
   void* mpRawData;
   uint32_t mPointCount;
   MemoryMappedArrayView* mpArrayView;
};

PointCloudMemoryMappedPager::PointCloudMemoryMappedPager() :
   mMemMapper(NULL),
   mBytesPerElement(0),
   mWritable(false)
{
   setName("PointCloud MemoryMappedPager");
   setCopyright("Copyright (2005) by Ball Aerospace & Technologies Corp.");
   setCreator("Ball Aerospace & Technologies Corp.");
   setDescription("Provides access to on disk data via os-level memory mapping functionality");
   setDescriptorId("{83AC828E-5224-42A5-B75B-7A249B1DEA45}");
   setVersion(APP_VERSION_NUMBER);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
   setShortDescription("Memory maps on disk data");
}

PointCloudMemoryMappedPager::~PointCloudMemoryMappedPager()
{
   if (mMemMapper != NULL)
   {
      for (std::vector<MemoryMappedDataBlock*>::iterator iter = mCurrentlyLeasedPages.begin();
            iter != mCurrentlyLeasedPages.end();
            ++iter)
      {
         MemoryMappedDataBlock* pBlock = *iter;
         if (pBlock != NULL)
         {
            mMemMapper->release(pBlock->getMemoryMappedArrayView());
            delete pBlock;
         }
      }
      delete mMemMapper;
      mMemMapper = NULL;
   }
}

bool PointCloudMemoryMappedPager::getInputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

bool PointCloudMemoryMappedPager::execute(PlugInArgList* pInputArgList, PlugInArgList* pOutputArgList)
{
   return true;
}

bool PointCloudMemoryMappedPager::initialize(const std::string& fileName, uint32_t bytesPerElement, bool writable)
{
   mBytesPerElement = bytesPerElement;
   mWritable = writable;
   try
   {
      mMemMapper = new MemoryMappedArray(fileName, bytesPerElement, 0, !writable);
   }
   catch (std::exception&)
   {
      return false;
   }
   return mMemMapper != NULL;
}

PointDataBlock* PointCloudMemoryMappedPager::getPointBlock(uint32_t startIndex, uint32_t numPoints, PointCloudDataRequest* pOriginalRequest)
{
   bool writable = false;
   if (pOriginalRequest != NULL)
   {
      writable = pOriginalRequest->getWritable();
   }
   if (!mWritable && writable)
   {
      return NULL;
   }
   mta::MutexLock mutex(mMutex);
   unsigned long segmentSize = numPoints * mBytesPerElement;
   MemoryMappedArrayView* pView = mMemMapper->getView(segmentSize);
   VERIFYRV(pView != NULL, NULL);
   char* pRawDataPointer = reinterpret_cast<char*>(pView->getSegmentByIndex(startIndex));
   if (pRawDataPointer == NULL)
   {
      return NULL;
   }
   MemoryMappedDataBlock* pBlock = new MemoryMappedDataBlock(pRawDataPointer, numPoints, pView);
   mCurrentlyLeasedPages.push_back(pBlock);
   return pBlock;
}

void PointCloudMemoryMappedPager::releasePointBlock(PointDataBlock* pBlock)
{
   if (pBlock == NULL)
   {
      return;
   }
   mta::MutexLock mutex(mMutex);
   MemoryMappedDataBlock* pOurBlock = dynamic_cast<MemoryMappedDataBlock*>(pBlock);
   std::vector<MemoryMappedDataBlock*>::iterator foundIter;
   foundIter = std::find(mCurrentlyLeasedPages.begin(), mCurrentlyLeasedPages.end(), pOurBlock);
   if (foundIter != mCurrentlyLeasedPages.end())
   {
      //this instance leased the block, so free the resources
      mMemMapper->release(pOurBlock->getMemoryMappedArrayView());
      mCurrentlyLeasedPages.erase(foundIter);
      delete pOurBlock;
   }
}

