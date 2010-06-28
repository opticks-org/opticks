/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"

#include <hdf.h> // #include this first so Hdf4Pager class is included properly
#include <mfhdf.h>
#include <vector>

#include "AppVerify.h"
#include "AppVersion.h"
#include "DataDescriptor.h"
#include "DataRequest.h"
#include "Hdf4Pager.h"
#include "Hdf4Utilities.h"
#include "RasterElement.h"
#include "RasterFileDescriptor.h"

using namespace HdfUtilities;
using namespace std;

static const int INVALID_HANDLE = -1;

Hdf4Pager::Hdf4Pager() :
   mFileHandle(INVALID_HANDLE), mDataHandle(INVALID_HANDLE)
{
   setName("Hdf4Pager");
   setDescriptorId("{DA5E408C-35CC-4f50-B50D-AD0B05174AEA}");
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

Hdf4Pager::~Hdf4Pager()
{
   closeFile();
}

bool Hdf4Pager::openFile(const std::string& filename)
{
   mFileHandle = SDstart(filename.c_str(), DFACC_READ);
   if (mFileHandle == FAIL)
   {
      return false;
   }

   const string& hdfDatasetName = getHdfDatasetName();

   int iIndex = SDnametoindex(mFileHandle, hdfDatasetName.c_str());
   mDataHandle = SDselect(mFileHandle, iIndex);
   if (mDataHandle == FAIL)
   {
      return false;
   }
   return true;
}

void Hdf4Pager::closeFile()
{
   if (mDataHandle != INVALID_HANDLE)
   {
      SDendaccess(mDataHandle);
   }
   if (mFileHandle != INVALID_HANDLE)
   {
      SDend(mFileHandle);
   }
}

CachedPage::UnitPtr Hdf4Pager::fetchUnit(DataRequest *pOriginalRequest)
{
   CachedPage::UnitPtr pUnit;

   VERIFYRV(pOriginalRequest != NULL, pUnit);

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

   if (startRow.getOnDiskNumber() > stopRow.getOnDiskNumber() || 
      startColumn.getOnDiskNumber() > stopColumn.getOnDiskNumber() ||
      startBand.getOnDiskNumber() > stopBand.getOnDiskNumber())
   {
      return pUnit;
   }

   VERIFYRV(mFileHandle != INVALID_HANDLE, pUnit);
   VERIFYRV(mDataHandle != INVALID_HANDLE, pUnit);

   const RasterElement* pRaster = getRasterElement();
   VERIFYRV(pRaster != NULL, pUnit);

   const DataDescriptor* pDescriptor = pRaster->getDataDescriptor();
   VERIFYRV(pDescriptor != NULL, pUnit);

   const RasterFileDescriptor* pFileDescriptor = dynamic_cast<const RasterFileDescriptor*>
      (pDescriptor->getFileDescriptor());
   VERIFYRV(pFileDescriptor != NULL, pUnit);

   double bpp = getBytesPerBand();

   InterleaveFormatType fileInterleave = pFileDescriptor->getInterleaveFormat();
   if (requestedFormat != fileInterleave)
   {
      return pUnit; // don't reformat the data
   }

   int columnCount = getColumnCount();
   int bandCount = getBandCount();

   if (startRow.getActiveNumber() + concurrentRows - 1 >= stopRow.getActiveNumber())
   {
      concurrentRows = stopRow.getActiveNumber()-startRow.getActiveNumber()+1;
   }
   if (startColumn.getActiveNumber() + concurrentColumns - 1 >= stopColumn.getActiveNumber())
   {
      concurrentColumns = stopColumn.getActiveNumber()-startColumn.getActiveNumber()+1;
   }
   if (startBand.getActiveNumber() + concurrentBands - 1 >= stopBand.getActiveNumber())
   {
      concurrentBands = stopBand.getActiveNumber()-startBand.getActiveNumber()+1;
   }

   int32 lStartValue[3] = {0};
   int32 lNumValues[3] = {0};
   switch (fileInterleave)
   {
   case BIP:
      {
         lStartValue[0] = startRow.getActiveNumber();
         lStartValue[1] = 0;
         lStartValue[2] = 0;

         lNumValues[0] = concurrentRows;
         lNumValues[1] = columnCount;
         lNumValues[2] = bandCount;

         break;
      }
   case BSQ:
      {
         lStartValue[0] = startBand.getActiveNumber();
         lStartValue[1] = startRow.getActiveNumber();
         lStartValue[2] = 0;

         lNumValues[0] = 1; // should be concurrentBands, but without a nextBand() function, this should be 1
         lNumValues[1] = concurrentRows;
         lNumValues[2] = columnCount;

         break;
      }
   case BIL:
      {
         lStartValue[0] = startRow.getActiveNumber();
         lStartValue[1] = 0;
         lStartValue[2] = 0;

         lNumValues[0] = concurrentRows;
         lNumValues[1] = bandCount;
         lNumValues[2] = columnCount;

         break;
      }
   default:
      return pUnit;
   }

   int pageSize = static_cast<int>(bpp*lNumValues[0]*lNumValues[1]*lNumValues[2]);

   ArrayResource<char> pData(pageSize, true);
   if (pData.get() == NULL)
   {
      return pUnit;
   }
   memset(pData.get(), 0, pageSize);

   int32 iSuccess = SDreaddata(mDataHandle, lStartValue, NULL, lNumValues, pData.get());

   if (iSuccess == FAIL)
   {
      return pUnit;
   }

   CachedPage::CacheUnit* pCacheUnit = new CachedPage::CacheUnit(pData.release(), startRow, concurrentRows,
      pageSize, (fileInterleave == BSQ ? startBand : CachedPage::CacheUnit::ALL_BANDS));
   pUnit.reset(pCacheUnit);
   return pUnit;
}
