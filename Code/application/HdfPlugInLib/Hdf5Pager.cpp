/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <hdf5.h> // #include this first so Hdf5Pager class is included properly
#include <vector>

#include "ComplexData.h"
#include "AppVerify.h"
#include "AppVersion.h"
#include "ApplicationServices.h"
#include "ConfigurationSettings.h"
#include "DataRequest.h"
#include "DataVariant.h"
#include "Endian.h"
#include "Hdf5Pager.h"
#include "Hdf5Resource.h"
#include "Hdf5Utilities.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterFileDescriptor.h"
#include "Service.h"

using namespace HdfUtilities;
using namespace std;

Hdf5Pager::Hdf5Pager() :
   mFileHandle(INVALID_HANDLE), mDataHandle(INVALID_HANDLE), mFileAccessProperties(H5P_DEFAULT)
{
   setName("Hdf5Pager");
   setDescriptorId("{F3720154-8F3A-43e2-BF36-3A810B59218F}");
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

Hdf5Pager::~Hdf5Pager()
{
   closeFile();
}

bool Hdf5Pager::openFile(const std::string& filename)
{
   // create a file access properties list
   mFileAccessProperties = H5Pcreate(H5P_FILE_ACCESS);
   if (mFileAccessProperties > 0)
   {
      int metaDataElementCount = 0;
      size_t rawDataElementCount = 0;
      size_t rawDataBytes = 0;
      double weight = 1.0;

      // get the current chunk cache properties
      herr_t status = H5Pget_cache(mFileAccessProperties, &metaDataElementCount, &rawDataElementCount,
         &rawDataBytes, &weight);
      if (status >= 0)
      {
         // get the hdf5 chunk cache configuration setting
         rawDataBytes = Hdf5Pager::getSettingCacheSize();

         // set the new chunk cache properties
         status = H5Pset_cache(mFileAccessProperties, metaDataElementCount, rawDataElementCount, rawDataBytes, weight);
         if (status < 0)
         {
            H5Pclose(mFileAccessProperties);
            mFileAccessProperties = H5P_DEFAULT;
         }
      }
      else
      {
         H5Pclose(mFileAccessProperties);
         mFileAccessProperties = H5P_DEFAULT;
      }
   }

   // check to see if a file access properties list has been created
   if (mFileAccessProperties == H5P_DEFAULT)
   {
      // create a file access properties list
      mFileAccessProperties = H5Pcreate(H5P_FILE_ACCESS);
   }

   if (mFileAccessProperties != H5P_DEFAULT)
   {
      // get the hdf5 chunk buffer size configuration setting
      unsigned int chunkBufferSize = Hdf5Pager::getSettingChunkBufferSize();

      // set the new chunk buffer size properties
      if (H5Pset_sieve_buf_size(mFileAccessProperties, chunkBufferSize) < 0)
      {
         H5Pclose(mFileAccessProperties);
         mFileAccessProperties = H5P_DEFAULT;
      }
   }

   mFileHandle = H5Fopen(filename.c_str(), H5F_ACC_RDONLY, mFileAccessProperties);
   if (mFileHandle == INVALID_HANDLE)
   {
      return false;
   }

   const string& hdfFullPathAndName = getHdfDatasetName();

   mDataHandle = H5Dopen(mFileHandle, hdfFullPathAndName.c_str());
   if (mDataHandle == INVALID_HANDLE)
   {
      return false;
   }
   return true;
}

void Hdf5Pager::closeFile()
{
   if (mFileAccessProperties != H5P_DEFAULT)
   {
      H5Pclose(mFileAccessProperties);
   }

   if (mDataHandle != INVALID_HANDLE)
   {
      H5Dclose(mDataHandle);
   }
   if (mFileHandle != INVALID_HANDLE)
   {
      H5Fclose(mFileHandle);
   }
}

hid_t Hdf5Pager::getFileHandle()
{
   return mFileHandle;
}

CachedPage::UnitPtr Hdf5Pager::fetchUnit(DataRequest *pOriginalRequest)
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

   const RasterDataDescriptor* pDescriptor = dynamic_cast<const RasterDataDescriptor*>(
      pRaster->getDataDescriptor());
   VERIFYRV(pDescriptor != NULL, pUnit);

   const RasterFileDescriptor* pFileDescriptor = dynamic_cast<const RasterFileDescriptor*>
      (pDescriptor->getFileDescriptor());
   VERIFYRV(pFileDescriptor != NULL, pUnit);

   EncodingType dataEncoding = pDescriptor->getDataType();
   VERIFYRV(dataEncoding != UNKNOWN, pUnit);

   double bpp = getBytesPerBand();

   InterleaveFormatType fileInterleave = pFileDescriptor->getInterleaveFormat();

   if (requestedFormat != fileInterleave)
   {
      return pUnit; // don't reformat the data
   }

   int columnCount = getColumnCount();
   int bandCount = getBandCount();

   // set up the dataspace for the amount of data to read in
   hsize_t offset[3] = {0, 0, 0}; // the start offset to read in the file
   hsize_t counts[3] = {0, 0, 0}; // how much data to read at a time
   hsize_t dimSpace[3] = {0, 0, 0}; // define a memory dataspace that expresses how much of the file will be read

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
   
   bool success = false;

   switch (fileInterleave)
   {
   case BIP:
      {
         // the offsets change the 'origin' of the read
         offset[0] = startRow.getActiveNumber();
         offset[1] = 0;
         offset[2] = 0;

         dimSpace[0] = concurrentRows;
         dimSpace[1] = columnCount;
         dimSpace[2] = bandCount;

         counts[0] = concurrentRows;
         counts[1] = columnCount;
         counts[2] = bandCount;

         break;
      }
   case BSQ:
      {
         // the offsets change the 'origin' of the read
         offset[0] = startBand.getActiveNumber();
         offset[1] = startRow.getActiveNumber();
         offset[2] = 0;

         dimSpace[0] = concurrentBands;
         dimSpace[1] = concurrentRows;
         dimSpace[2] = columnCount;

         counts[0] = concurrentBands;
         counts[1] = concurrentRows;
         counts[2] = columnCount;

         break;
      }
   case BIL:
      {
         offset[0] = startRow.getActiveNumber();
         offset[1] = 0;
         offset[2] = 0;

         dimSpace[0] = concurrentRows;
         dimSpace[1] = bandCount;
         dimSpace[2] = columnCount;

         counts[0] = concurrentRows;
         counts[1] = bandCount;
         counts[2] = columnCount;

         break;
      }
   default:
      return pUnit;
   }

   size_t pageSize = static_cast<size_t>(bpp*counts[0]*counts[1]*counts[2]);

   ArrayResource<char> pData(pageSize);
   if (pData.get() == NULL)
   {
      return pUnit;
   }

   memset(pData.get(), 0, pageSize);

   Hdf5DataSpaceResource memSpace(H5Screate_simple(3, dimSpace, NULL));

   Hdf5DataSpaceResource dataSpace(H5Dget_space(mDataHandle));
   Hdf5TypeResource dataType(H5Dget_type(mDataHandle));
   Hdf5TypeResource loadedType;
   if (dataEncoding == INT4SCOMPLEX || dataEncoding == FLT8COMPLEX)
   {
      if (dataEncoding == INT4SCOMPLEX)
      {
         loadedType = Hdf5TypeResource(H5Tcreate(H5T_COMPOUND, sizeof(IntegerComplex)));
         H5Tinsert(*loadedType, "Real", HOFFSET(IntegerComplex, mReal), H5T_NATIVE_SHORT);
         H5Tinsert(*loadedType, "Imaginary", offsetof(IntegerComplex, mImaginary), H5T_NATIVE_SHORT);
      }
      else
      {
         loadedType = Hdf5TypeResource(H5Tcreate(H5T_COMPOUND, sizeof(FloatComplex)));
         H5Tinsert(*loadedType, "Real", HOFFSET(FloatComplex, mReal), H5T_NATIVE_FLOAT);
         H5Tinsert(*loadedType, "Imaginary", offsetof(FloatComplex, mImaginary), H5T_NATIVE_FLOAT);
      }
   }
   else
   {
      loadedType = Hdf5TypeResource(H5Tcopy(*dataType));
      if (Endian::getSystemEndian() == LITTLE_ENDIAN)
      {
         H5Tset_order(*loadedType, H5T_ORDER_LE);
      }
      else
      {
         H5Tset_order(*loadedType, H5T_ORDER_BE);
      }
   }

   success = 0 == H5Sselect_hyperslab(*dataSpace, H5S_SELECT_SET, offset, NULL, counts, NULL);
   if (success)
   {
      success = 0 == H5Dread(mDataHandle, *loadedType, *memSpace, *dataSpace, H5P_DEFAULT, pData.get());
   }

   if (success == false)
   {
      return pUnit;
   }

   CachedPage::CacheUnit* pCacheUnit = new CachedPage::CacheUnit(pData.release(), startRow, concurrentRows,
      pageSize, startBand);
   pUnit.reset(pCacheUnit);
   return pUnit;
}
