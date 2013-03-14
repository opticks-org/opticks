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
#include "PlugInRegistration.h"
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

REGISTER_PLUGIN(OpticksNitf, Pager, Nitf::Pager);

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
{}

bool Nitf::Pager::getInputSpecification(PlugInArgList*& pArgList)
{
   return (CachedPager::getInputSpecification(pArgList) &&
      pArgList != NULL && pArgList->addArg<int>("Segment Number", 1));
}

bool Nitf::Pager::parseInputArgs(PlugInArgList* pInputArgList)
{
   return (CachedPager::parseInputArgs(pInputArgList) &&
      pInputArgList != NULL && pInputArgList->getPlugInArgValue<int>("Segment Number", mSegment));
}

bool Nitf::Pager::openFile(const string& filename)
{
   mpImageHandler = Nitf::OssimImageHandlerResource(filename);
   return mpImageHandler.get() != NULL;
}

CachedPage::UnitPtr Nitf::Pager::fetchUnit(DataRequest *pOriginalRequest)
{
   VERIFYRV(pOriginalRequest != NULL, CachedPage::UnitPtr());
   DimensionDescriptor startRow = pOriginalRequest->getStartRow();
   DimensionDescriptor stopRow = pOriginalRequest->getStopRow();
   DimensionDescriptor startColumn = pOriginalRequest->getStartColumn();
   DimensionDescriptor stopColumn = pOriginalRequest->getStopColumn();
   DimensionDescriptor startBand = pOriginalRequest->getStartBand();
   DimensionDescriptor stopBand = pOriginalRequest->getStopBand();
   unsigned int concurrentRows = pOriginalRequest->getConcurrentRows();
   unsigned int concurrentColumns = getColumnCount();
   unsigned int concurrentBands = pOriginalRequest->getConcurrentBands();

   unsigned int rowNumber = startRow.getOnDiskNumber();
   unsigned int colNumber = startColumn.getOnDiskNumber();
   unsigned int bandNumber = startBand.getOnDiskNumber();

   VERIFYRV(mpImageHandler.get() != NULL, CachedPage::UnitPtr());
   if (mpImageHandler->canCastTo("ossimNitfTileSource") == false)
   {
      return CachedPage::UnitPtr();
   }

   ossimNitfTileSource* pTileSource = PTR_CAST(ossimNitfTileSource, mpImageHandler.get());
   VERIFYRV(pTileSource != NULL, CachedPage::UnitPtr());
   pTileSource->setExpandLut(false);
   mpImageHandler->setCurrentEntry(mSegment - 1);

   // Try to set the output band list.
   // If it cannot succeed (e.g.: for VQ), this is not an error.
   vector<ossim_uint32> bandList(concurrentBands);
   for (unsigned int band = 0; band < concurrentBands; ++band)
   {
      bandList[band] = bandNumber + band;
   }

   mpImageHandler->setOutputBandList(bandList);

   // The Bounding Rectangle contains NITF Chipping Information.
   ossimIrect br = mpImageHandler->getBoundingRect();

   int minx;
   int miny;
   int maxx;
   int maxy;
   br.getBounds(minx, miny, maxx, maxy);

   ossimIrect region(colNumber + minx, rowNumber + miny,
                     colNumber + minx + concurrentColumns-1, rowNumber + miny + concurrentRows-1);

   const int dstSize = concurrentRows * concurrentColumns * concurrentBands * getBytesPerBand();
   ArrayResource<char> pData(dstSize, true);
   if (pData.get() == NULL)
   {
      return CachedPage::UnitPtr();
   }

   ossimRefPtr<ossimImageData> cubeData = mpImageHandler->getTile(region);
   if (cubeData == NULL || cubeData->getBuf() == NULL)
   {
      return CachedPage::UnitPtr();
   }

   if (concurrentBands == 1)
   {
      // This looks like an innocent optimization. It is not; it is actually a very special case for VQ compression
      // which happens to result in optimization for single band, non-VQ data.
      // Since OSSIM special cases VQ to always have 3 output bands (even when it is really only single band),
      // calling cubeData->unloadTile results in an access violation (copying 3 bands into 1 is a bad idea).
      // So, to work around this, we need to detect when we are dealing with VQ-compressed data. Because OSSIM only
      // supports single band VQ data, all VQ data must be exactly one band. And since single band data in any
      // interleave is equivalent to any other interleave, it also happens to be a very convenient optimization for
      // single band data. And since ossimImageData stores data as BSQ, memcpy is safe to use in this scenario.

      // Since concurrentBands is set to 1, the earlier call to setOutputBandList only requested a single band. In the
      // case of VQ, there will actually be 3 bands, all of which must be identical because ossimNitfTileSource only
      // accepts single-band VQ. In all other cases, the call to setOutputBandList will have restricted cubeData to a
      // single band, meaning that the only valid parameter to cubeData->getBuf is a 0. This parameter cannot be
      // bandNumber because of the earlier call to setOutputBandList (setting it to bandNumber returns a NULL pointer).
      memcpy(pData.get(), cubeData->getBuf(0), dstSize);
   }
   else
   {
      ossimInterleaveType interleave;
      if (pOriginalRequest->getInterleaveFormat() == BSQ)
      {
         interleave = OSSIM_BSQ;
      }
      else if (pOriginalRequest->getInterleaveFormat() == BIL)
      {
         interleave = OSSIM_BIL;
      }
      else if (pOriginalRequest->getInterleaveFormat() == BIP)
      {
         interleave = OSSIM_BIP;
      }
      else
      {
         return CachedPage::UnitPtr();
      }

      cubeData->unloadTile(pData.get(), region, interleave);
   }

   return CachedPage::UnitPtr(new CachedPage::CacheUnit(pData.release(), startRow, concurrentRows,
      dstSize, concurrentBands == 1 ? startBand : CachedPage::CacheUnit::ALL_BANDS));
}
