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
#include "DataRequest.h"
#include "InMemoryPage.h"
#include "InMemoryPager.h"
#include "ModelServices.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"

InMemoryPager::InMemoryPager() : mpData(NULL), mpRaster(NULL)
{
   setName("In Memory Pager");
   setCopyright("Copyright (2006) by Ball Aerospace & Technologies Corp.");
   setCreator("Ball Aerospace & Technologies Corp.");
   setDescription("Provides access to data through a single chunk in memory");
   setDescriptorId("{5EC58609-F1E6-44a6-B322-C15BA10BC741}");
   setVersion(APP_VERSION_NUMBER);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
   setShortDescription("Provides a RAM backing for data");
}

InMemoryPager::~InMemoryPager()
{
   if (mpData != NULL)
   {
      Service<ModelServices> pModel;
      pModel->deleteMemoryBlock(reinterpret_cast<char*>(mpData));
   }
}

bool InMemoryPager::getInputSpecification(PlugInArgList *&pArgList)
{
   Service<PlugInManagerServices> pPlugInMgr;

   pArgList = pPlugInMgr->getPlugInArgList();
   VERIFY(pArgList != NULL);

   VERIFY(pArgList->addArg<RasterElement>("Raster Element"));
   VERIFY(pArgList->addArg<void>("Memory"));

   return true;
}

bool InMemoryPager::execute(PlugInArgList *pInput, PlugInArgList *pOutput)
{
   VERIFY(mpRaster == NULL && mpData == NULL);
   VERIFY(pInput != NULL);

   mpRaster = pInput->getPlugInArgValue<RasterElement>("Raster Element");
   mpData = pInput->getPlugInArgValue<void>("Memory");

   return true;
}

RasterPage *InMemoryPager::getPage(DataRequest *pOriginalRequest, 
      DimensionDescriptor startRow,
      DimensionDescriptor startColumn,
      DimensionDescriptor startBand)
{
   VERIFYRV(mpData != NULL, NULL);
   VERIFYRV(mpRaster != NULL, NULL);
   VERIFYRV(pOriginalRequest != NULL, NULL);

   InterleaveFormatType requestedType = pOriginalRequest->getInterleaveFormat();
   RasterDataDescriptor *pDescriptor = dynamic_cast<RasterDataDescriptor*>(mpRaster->getDataDescriptor());
   VERIFYRV(pDescriptor != NULL, NULL);
   if (pDescriptor->getInterleaveFormat() != requestedType)
   {
      return NULL;
   }

   unsigned int numRows = pDescriptor->getRowCount();
   unsigned int numColumns = pDescriptor->getColumnCount();
   unsigned int numBands = pDescriptor->getBandCount();
   unsigned int bytesPerElement = pDescriptor->getBytesPerElement();

   unsigned int rowNumber = startRow.getActiveNumber();
   unsigned int colNumber = startColumn.getActiveNumber();
   unsigned int bandNumber = startBand.getActiveNumber();

   if (rowNumber >= numRows || colNumber >= numColumns || bandNumber >= numBands)
   {
      return NULL;
   }

   char *pData = reinterpret_cast<char*>(mpData);
   char *pStart = NULL;
   switch (requestedType)
   {
   case BIP:
      {
         size_t minorSize = bytesPerElement;
         size_t middleSize = minorSize * numBands;
         size_t majorSize = middleSize * numColumns;
         pStart = &pData[rowNumber * majorSize + colNumber * middleSize + bandNumber * minorSize];
      }
      break;
   case BSQ:
      {
         size_t minorSize = bytesPerElement;
         size_t middleSize = minorSize * numColumns;
         size_t majorSize = middleSize * numRows;
         pStart = &pData[bandNumber * majorSize + rowNumber * middleSize + colNumber * minorSize];
      }
      break;
   case BIL:
      {
         size_t minorSize = bytesPerElement;
         size_t middleSize = minorSize * numColumns;
         size_t majorSize = middleSize * numBands;
         pStart = &pData[rowNumber * majorSize + bandNumber * middleSize + colNumber * minorSize];
      }
      break;
   default:
      return NULL;
   }

   VERIFYRV(pStart != NULL, NULL);

   return new InMemoryPage(pStart, numRows - rowNumber);
}

void InMemoryPager::releasePage(RasterPage *pPage)
{
   InMemoryPage *pMemPage = dynamic_cast<InMemoryPage*>(pPage);
   if (pMemPage != NULL)
   {
      delete pMemPage;
   }

   return;
}

int InMemoryPager::getSupportedRequestVersion() const
{
   return 1;
}
