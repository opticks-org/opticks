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
#include "CachedPage.h"
#include "DataRequest.h"
#include "NitfPager.h"
#include "NitfUtilities.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "switchOnEncoding.h"

#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimString.h>
#include <ossim/imaging/ossimImageHandlerRegistry.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/imaging/ossimNitfTileSource.h>
#include <ossim/imaging/ossimImageWriterFactoryRegistry.h>
#include <ossim/imaging/ossimBandSelector.h>
#include <ossim/init/ossimInit.h>

#include <QtCore/QString>

Nitf::Pager::Pager() :
   mSegment(1),
   mpStep(NULL)
{
   setCopyright(APP_COPYRIGHT);
   setName("NitfPager");
   setVersion(APP_VERSION_NUMBER);
   setCreator("Ball Aerospace & Technologies Corp.");
   setDescriptorId("{4946AB79-B6DF-4ecd-8DA7-B77B04329C2F}");
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

Nitf::Pager::~Pager()
{
   // Do nothing
}

bool Nitf::Pager::getInputSpecification(PlugInArgList*& pArgList)
{
   Service<PlugInManagerServices> pPims;
   VERIFY(pPims.get() != NULL);

   if (CachedPager::getInputSpecification(pArgList) == true)
   {
      VERIFY(pArgList != NULL);

      if (pArgList->addArg<int>("Segment Number", 1) == false)
      {
         pPims->destroyPlugInArgList(pArgList);
         pArgList = NULL;
         return false;
      }

      return true;
   }

   return false;
}

bool Nitf::Pager::parseInputArgs(PlugInArgList* pInputArgList)
{
   bool success = CachedPager::parseInputArgs(pInputArgList) == true;
   if (success)
   {
      int* pSegment = pInputArgList->getPlugInArgValue<int>("Segment Number");
      if (pSegment == NULL)
      {
         return false;
      }
      mSegment = *pSegment; // guaranteed to be good since has a default value set
   }

   return success;
}

bool Nitf::Pager::openFile(const string& filename)
{
   mpImageHandler = Nitf::OssimImageHandlerResource(filename);
   return mpImageHandler.get() != NULL;
}

CachedPage::UnitPtr Nitf::Pager::fetchUnit(DataRequest *pOriginalRequest)
{
   CachedPage::UnitPtr pUnit;

   VERIFYRV(pOriginalRequest != NULL, pUnit);
#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : TODO currently fails if startColumn != 0 (leckels)")

   InterleaveFormatType requestedFormat = pOriginalRequest->getInterleaveFormat();
   DimensionDescriptor startRow = pOriginalRequest->getStartRow();
   DimensionDescriptor stopRow = pOriginalRequest->getStopRow();
   unsigned int concurrentRows = pOriginalRequest->getConcurrentRows();
   DimensionDescriptor startColumn = pOriginalRequest->getStartColumn();
   DimensionDescriptor stopColumn = pOriginalRequest->getStopColumn();
   unsigned int concurrentColumns = pOriginalRequest->getConcurrentColumns();
   DimensionDescriptor startBand = pOriginalRequest->getStartBand();
   DimensionDescriptor stopBand = pOriginalRequest->getStopBand();
   unsigned int concurrentBands = pOriginalRequest->getConcurrentBands();

   unsigned int rowNumber = startRow.getActiveNumber();
   unsigned int colNumber = startColumn.getActiveNumber();
   unsigned int bandNumber = startBand.getActiveNumber();

   if (requestedFormat != BSQ)
   {
      return pUnit;
   }

   if (rowNumber > stopRow.getActiveNumber() ||
      colNumber > stopColumn.getActiveNumber() ||
      bandNumber > stopBand.getActiveNumber())
   {
      return pUnit;
   }

   // give the entire row & bands of data
   if (concurrentColumns != getColumnCount())
   {
      concurrentColumns = getColumnCount();
   }

   if (requestedFormat == BSQ)
   {
      concurrentBands = 1;
   }
   else if (concurrentBands != getBandCount())
   {
      concurrentBands = getBandCount();
   }

   // can't give more data than there is in the cube
   concurrentRows = min(concurrentRows, stopRow.getActiveNumber() - startRow.getActiveNumber() + 1);

   VERIFYRV(mpImageHandler.get() != NULL, pUnit);

   if (mpImageHandler->canCastTo("ossimNitfTileSource") == false)
   {
      return pUnit;
   }
   ossimNitfTileSource* pTileSource = PTR_CAST(ossimNitfTileSource, mpImageHandler.get());
   if (pTileSource == NULL)
   {
      return pUnit;
   }

   pTileSource->setExpandLut(false);

   mpImageHandler->setCurrentEntry(mSegment-1);
   // the Bounding Rectangle contains NITF Chipping Information.
   ossimIrect br = mpImageHandler->getBoundingRect();

   int minx;
   int miny;
   int maxx;
   int maxy;
   br.getBounds(minx, miny, maxx, maxy);

   ossimIrect region(colNumber + minx, rowNumber + miny,
                     colNumber + minx + concurrentColumns-1, rowNumber + miny + concurrentRows-1);

   ossimRefPtr<ossimImageData> cubeData = mpImageHandler->getTile(region);
   if (cubeData == NULL || cubeData->getBuf(bandNumber) == NULL)
   {
      return pUnit;
   }

   size_t srcSize = static_cast<size_t>(cubeData->getSizePerBandInBytes()) * concurrentBands;
   size_t dstSize = static_cast<size_t>(concurrentRows)*concurrentColumns*concurrentBands*getBytesPerBand();
   VERIFYRV(srcSize >= dstSize && dstSize > 0, pUnit);

   char* pData = new char[dstSize];
   memcpy(pData, cubeData->getBuf(bandNumber), dstSize);

   CachedPage::CacheUnit* pCacheUnit = new CachedPage::CacheUnit(pData, startRow, concurrentRows,
      static_cast<int>(dstSize), startBand);
   pUnit.reset(pCacheUnit);
   return pUnit;
}
