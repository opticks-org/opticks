/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "ComplexData.h"
#include "DataRequest.h"
#include "GdalRasterPager.h"
#include "ObjectResource.h"
#include "PlugInArgList.h"
#include "PlugInRegistration.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterUtilities.h"
#include "switchOnEncoding.h"

#include <gdal_priv.h>
#include <vector>

REGISTER_PLUGIN_BASIC(OpticksGdalImporter, GdalRasterPager);

namespace
{
   GDALDataType encodingTypeToGdalDataType(EncodingType type)
   {
      switch(type)
      {
      case INT1UBYTE:
         return GDT_Byte;
      case INT2UBYTES:
         return GDT_UInt16;
      case INT2SBYTES:
         return GDT_Int16;
      case INT4UBYTES:
         return GDT_UInt32;
      case INT4SBYTES:
         return GDT_Int32;
      case FLT4BYTES:
         return GDT_Float32;
      case FLT8BYTES:
         return GDT_Float64;
      case INT4SCOMPLEX:
         return GDT_CInt16;
      case FLT8COMPLEX:
         return GDT_CFloat32;
      case INT1SBYTE:
         ; // 1 byte signed is not a GDAL type so this will never be reached
      }
      return GDT_Byte;
   }

   size_t gdalDataTypeSize(GDALDataType type)
   {
      switch(type)
      {
      case GDT_Byte:
         return 1;
      case GDT_UInt16:
      case GDT_Int16:
         return 2;
      case GDT_UInt32:
      case GDT_Int32:
      case GDT_Float32:
      case GDT_CInt16:
         return 4;
      case GDT_Float64:
      case GDT_CInt32:
      case GDT_CFloat32:
         return 8;
      case GDT_CFloat64:
         return 16;
      case GDT_TypeCount: // pass through
      case GDT_Unknown:   // pass through
      default:
         break;
      }
      return 0;
   }
}

GdalRasterPager::GdalRasterPager() : mpDataset(NULL)
{
   setName("GDAL Raster Pager");
   setCopyright(APP_COPYRIGHT);
   setCreator("Ball Aerospace & Technologies Corp.");
   setDescription("Provides access to on-disk GDAL data");
   setDescriptorId("{ca3a1d92-abf5-4077-b24e-5209a6d1d2af}");
   setVersion(APP_VERSION_NUMBER);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
   setShortDescription("GDAL pager");
   GDALAllRegister();
}

GdalRasterPager::~GdalRasterPager()
{
}

bool GdalRasterPager::getInputSpecification(PlugInArgList*& pArgList)
{
   if (!CachedPager::getInputSpecification(pArgList))
   {
      return false;
   }
   VERIFY(pArgList->addArg<std::string>("DatasetName", std::string(), std::string()));

   return true;
}

bool GdalRasterPager::parseInputArgs(PlugInArgList *pInputArgList)
{
   if (!CachedPager::parseInputArgs(pInputArgList))
   {
      return false;
   }
   if (!pInputArgList->getPlugInArgValue("DatasetName", mDatasetName))
   {
      return false;
   }
   return true;
}

bool GdalRasterPager::openFile(const std::string& filename)
{
   if (mDatasetName.empty())
   {
      mDatasetName = filename;
   }
   mpDataset.reset(reinterpret_cast<GDALDataset*>(GDALOpen(mDatasetName.c_str(), GA_ReadOnly)));
   return mpDataset.get() != NULL;
}

CachedPage::UnitPtr GdalRasterPager::fetchUnit(DataRequest* pOriginalRequest)
{
   // load entire rows for the band in the request
   // there will be only one band since GDAL is always BSQ.
   // We'll load the rows requested and cache that block
   const RasterDataDescriptor* pDesc = static_cast<const RasterDataDescriptor*>(getRasterElement()->getDataDescriptor());

   // calculate the rows we are loading
   std::vector<DimensionDescriptor> rows = RasterUtilities::subsetDimensionVector(pDesc->getRows(), pOriginalRequest->getStartRow(),
      pOriginalRequest->getStopRow());
   unsigned int numRows = std::min<size_t>(pOriginalRequest->getConcurrentRows(), rows.size());

   // calculate to columns we a loading

//#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : fix this when cached pager properly handles column subsets (tclarke)")
   /** this should work but CachedPager assumes fetchUnit() returns full rows **/
   /*std::vector<DimensionDescriptor> cols = RasterUtilities::subsetDimensionVector(pDesc->getColumns(), pOriginalRequest->getStartColumn(),
      pOriginalRequest->getStopColumn());
   unsigned int numCols = std::min(pOriginalRequest->getConcurrentColumns(), cols.size());*/

   std::vector<DimensionDescriptor> cols = pDesc->getColumns();
   unsigned int numCols = cols.size();

   if (numRows == 0 || numCols == 0)
   {
      return CachedPage::UnitPtr();
   }

   size_t bufSize = numCols * numRows * pDesc->getBytesPerElement();
   ArrayResource<char> pBuffer(bufSize, true);
   if (pBuffer.get() == NULL)
   {
      return CachedPage::UnitPtr();
   }

   GDALRasterBand* pBand = mpDataset->GetRasterBand(pOriginalRequest->getStartBand().getOnDiskNumber() + 1); // 1 based band number
   if (pBand == NULL)
   {
      return CachedPage::UnitPtr();
   }
   GDALDataType effectiveType = encodingTypeToGdalDataType(pDesc->getDataType());

   if (pDesc->getRowSkipFactor() == 0 && pDesc->getColumnSkipFactor() == 0)
   {
      if (pBand->RasterIO(GF_Read, cols.front().getOnDiskNumber(), rows.front().getOnDiskNumber(),
                                   numCols, numRows, pBuffer.get(), numCols, numRows, effectiveType, 0, 0) == CE_Failure)
      {
         return CachedPage::UnitPtr();
      }
   }
   else if (pDesc->getColumnSkipFactor() == 0)
   {
      // read each row
      for (size_t rowIdx = 0; rowIdx < numRows; rowIdx++)
      {
         size_t bufOffset = rowIdx * numCols * pDesc->getBytesPerElement();
         if (pBand->RasterIO(GF_Read, cols.front().getOnDiskNumber(), rows[rowIdx].getOnDiskNumber(),
                                      numCols, 1, pBuffer.get() + bufOffset, numCols, 1, effectiveType, 0, 0) == CE_Failure)
         {
            return CachedPage::UnitPtr();
         }
      }
   }
   else
   {
      unsigned int firstReadCol = cols.front().getOnDiskNumber();
      unsigned int numColsTotal = cols.back().getOnDiskNumber() - firstReadCol + 1;
      unsigned int bytesPerElement = pDesc->getBytesPerElement();
      std::vector<char> tmpBuf(numColsTotal * bytesPerElement);
      // read each pixel
      for (size_t rowIdx = 0; rowIdx < numRows; rowIdx++)
      {
         // read an entire row into a temp buffer
         if (pBand->RasterIO(GF_Read, firstReadCol, rows[rowIdx].getOnDiskNumber(),
                     numColsTotal, 1, &tmpBuf.front(), numColsTotal, 1, effectiveType, 0, 0) == CE_Failure)
         {
            return CachedPage::UnitPtr();
         }

         // copy the pixels needed
         unsigned int bufCol = 0;
         unsigned int colSkip = pDesc->getColumnSkipFactor();
         for (unsigned int curCol = 0; curCol < numColsTotal; curCol += (colSkip + 1))
         {
            size_t bufOffset = (rowIdx * numCols * bytesPerElement) + (bufCol++ * bytesPerElement);
            size_t tmpOffset = curCol * bytesPerElement;
            memcpy(pBuffer.get() + bufOffset, &tmpBuf[tmpOffset], bytesPerElement);
         }
      }
   }

   return CachedPage::UnitPtr(new CachedPage::CacheUnit(
      pBuffer.release(), pOriginalRequest->getStartRow(), numRows, bufSize, pOriginalRequest->getStartBand()));
}
