/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "CachedPager.h"
#include "DataDescriptor.h"
#include "DataRequest.h"
#include "DMutex.h"
#include "Filename.h"
#include "ModelServices.h"
#include "ObjectResource.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"

using namespace std;

CachedPager::CachedPager() :
   mCache(10 * 1024 * 1024),
   mpMutex(new mta::DMutex),
   mpDescriptor(NULL),
   mpRaster(NULL),
   mBytesPerBand(0),
   mColumnCount(0),
   mBandCount(0),
   mRowCount(0)
{
}

CachedPager::CachedPager(const size_t cacheSize) :
   mCache(static_cast<const int>(cacheSize)),
   mpMutex(new mta::DMutex),
   mpDescriptor(NULL),
   mpRaster(NULL),
   mBytesPerBand(0),
   mColumnCount(0),
   mBandCount(0),
   mRowCount(0)
{
}

CachedPager::~CachedPager()
{
}

bool CachedPager::getInputSpecification(PlugInArgList *&pArgList)
{
   Service<PlugInManagerServices> pPims;

   pArgList = pPims->getPlugInArgList();
   VERIFY(pArgList != NULL);

   pArgList->addArg<RasterElement>(PagedElementArg(), NULL);
   pArgList->addArg<Filename>(PagedFilenameArg(), NULL);

   return true;
}

bool CachedPager::getOutputSpecification(PlugInArgList *&pArgList)
{
   pArgList = NULL;
   return true;
}

bool CachedPager::execute(PlugInArgList *pInputArgList, PlugInArgList *pOutputArgList)
{
   return parseInputArgs(pInputArgList) && openFile(mFilename);
}

bool CachedPager::parseInputArgs(PlugInArgList *pInputArgList)
{
   if (pInputArgList == NULL)
   {
      return false;
   }

   mpRaster = pInputArgList->getPlugInArgValue<RasterElement>(PagedElementArg());
   if (mpRaster == NULL)
   {
      return false;
   }

   mpDescriptor = dynamic_cast<RasterDataDescriptor*>(mpRaster->getDataDescriptor());
   if (mpDescriptor == NULL)
   {
      return false;
   }

   mBytesPerBand = mpDescriptor->getBytesPerElement();
   mRowCount = mpDescriptor->getRowCount();
   mColumnCount = mpDescriptor->getColumnCount();
   mBandCount = mpDescriptor->getBandCount();

   //Get Filename argument
   Filename* pFilename = pInputArgList->getPlugInArgValue<Filename>(PagedFilenameArg());
   if (pFilename == NULL)
   {
      return false;
   }
   mFilename = pFilename->getFullPathAndName();

   mCache.initialize(mBytesPerBand, mColumnCount, mBandCount);

   return true;
}

RasterPage* CachedPager::getPage(DataRequest *pOriginalRequest,
      DimensionDescriptor startRow, 
      DimensionDescriptor startColumn, 
      DimensionDescriptor startBand)
{
   if (pOriginalRequest == NULL)
   {
      return NULL;
   }
   if (pOriginalRequest->getWritable())
   {
      return NULL;
   }

   VERIFYRV(pOriginalRequest != NULL, NULL);

   mta::MutexLock lock(*mpMutex);


   InterleaveFormatType requestedFormat = pOriginalRequest->getInterleaveFormat();
   DimensionDescriptor stopRow = pOriginalRequest->getStopRow();
   unsigned int concurrentRows = pOriginalRequest->getConcurrentRows();
   DimensionDescriptor stopBand = pOriginalRequest->getStopBand();
   unsigned int concurrentBands = pOriginalRequest->getConcurrentBands();

   if (requestedFormat != mpDescriptor->getInterleaveFormat())
   {
      return NULL;
   }

   CachedPage::UnitPtr pUnit = mCache.getUnit(pOriginalRequest, startRow, startBand);

   DimensionDescriptor cacheStartBand = startBand;
   DimensionDescriptor cacheStopBand = stopBand;
   if (pUnit.get() == NULL) // cache miss
   {
      if (requestedFormat != BSQ)
      {
         cacheStartBand = DimensionDescriptor();
         cacheStopBand = DimensionDescriptor();
         concurrentBands = mBandCount;
      }

      // get a bunch more rows if you can to prevent a cache miss
      if (startRow.getActiveNumber() + concurrentRows < stopRow.getActiveNumber())
      {
         double CHUNK_SIZE = getChunkSize();
         concurrentRows =
            static_cast<unsigned int>(std::max(static_cast<double>(concurrentRows),
                                      CHUNK_SIZE/(concurrentBands*mColumnCount*mBytesPerBand)));
         if (concurrentRows > static_cast<unsigned int>(mRowCount))
         {
            concurrentRows = static_cast<unsigned int>(mRowCount);
         }
      }

      FactoryResource<DataRequest> pNewRequest;
      pNewRequest->setInterleaveFormat(requestedFormat);
      pNewRequest->setRows(startRow, stopRow, concurrentRows);
      // Get full columns
      pNewRequest->setBands(cacheStartBand, cacheStopBand);

      pNewRequest->polish(mpDescriptor);
      pUnit = fetchUnit(pNewRequest.get());
   }

   return mCache.createPage(pUnit, requestedFormat, startRow, startColumn, startBand);
}

void CachedPager::releasePage(RasterPage *pPage)
{
   mta::MutexLock lock(*mpMutex);

   delete dynamic_cast<CachedPage*>(pPage);
}

int CachedPager::getSupportedRequestVersion() const
{
   return 1;
}

const int CachedPager::getBytesPerBand() const
{
   return mBytesPerBand;
}

const int CachedPager::getColumnCount() const
{
   return mColumnCount;
}

const int CachedPager::getBandCount() const
{
   return mBandCount;
}

const RasterElement* CachedPager::getRasterElement() const
{
   return mpRaster;
}

double CachedPager::getChunkSize() const
{
   return 1 * 1024 * 1024;
}
