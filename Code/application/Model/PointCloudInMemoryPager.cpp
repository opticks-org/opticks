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
#include "ModelServices.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PointCloudDataDescriptor.h"
#include "PointCloudDataRequest.h"
#include "PointCloudInMemoryPager.h"
#include "PointDataBlock.h"
#include <limits>

class InMemoryDataBlock : public PointDataBlock
{
public:
   InMemoryDataBlock(void* pData, uint32_t pointCount) :
      mpRawData(pData),
      mPointCount(pointCount)
   {      
   }

   virtual void* getRawData()
   {
      return mpRawData;
   }

   virtual uint32_t getNumPoints()
   {
      return mPointCount;
   }

private:
   void* mpRawData;
   uint32_t mPointCount;
};

PointCloudInMemoryPager::PointCloudInMemoryPager() :
   mpData(NULL),
   mPointSizeInBytes(0),
   mPointCount(0)
{
   setName("PointCloud In Memory Pager");
   setCopyright("Copyright (2011) by Ball Aerospace & Technologies Corp.");
   setCreator("Ball Aerospace & Technologies Corp.");
   setDescription("Provides access to data through a single chunk in memory");
   setDescriptorId("{55113CB7-226F-4D6F-AFAD-743924EA16CF}");
   setVersion(APP_VERSION_NUMBER);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
   setShortDescription("Provides a RAM backing for data");
}

PointCloudInMemoryPager::~PointCloudInMemoryPager()
{
   if (mpData != NULL)
   {
      Service<ModelServices> pModel;
      pModel->deleteMemoryBlock(reinterpret_cast<char*>(mpData));
   }
}

bool PointCloudInMemoryPager::initialize(PointCloudDataDescriptor* pDescriptor, uint32_t numPoints)
{
   mPointSizeInBytes = pDescriptor->getPointSizeInBytes();
   uint64_t totalSize = mPointSizeInBytes * numPoints;
   if (totalSize > std::numeric_limits<size_t>::max())
   {
      return false;
   }
   mpData = Service<ModelServices>()->getMemoryBlock(static_cast<size_t>(totalSize));
   mPointCount = numPoints;
   return mpData != NULL;
}

bool PointCloudInMemoryPager::getInputSpecification(PlugInArgList *&pArgList)
{
   pArgList = NULL;
   return true;
}

bool PointCloudInMemoryPager::execute(PlugInArgList* pInput, PlugInArgList* pOutput)
{
   return true;
}

PointDataBlock* PointCloudInMemoryPager::getPointBlock(uint32_t startIndex, uint32_t numPoints, PointCloudDataRequest* pOriginalRequest)
{
   bool writable = false;
   if (startIndex + numPoints >= mPointCount)
   {
      return NULL;
   }
   if (pOriginalRequest != NULL)
   {
      writable = pOriginalRequest->getWritable();
   }
   return new InMemoryDataBlock(mpData + (startIndex * mPointSizeInBytes), mPointCount - startIndex);
}

void PointCloudInMemoryPager::releasePointBlock(PointDataBlock* pBlock)
{
   InMemoryDataBlock* pMemBlock = dynamic_cast<InMemoryDataBlock*>(pBlock);
   delete pMemBlock;
}
