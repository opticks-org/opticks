/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "RasterElement.h"
#include "AppVerify.h"
#include "DataAccessor.h"
#include "DataAccessorImpl.h"
#include "RasterDataDescriptor.h"
#include "RasterFileDescriptor.h"
#include "RasterUtilities.h"
#include "OssimAppMemorySource.h"

#include <ossim/imaging/ossimImageDataFactory.h>
#include <ossim/imaging/ossimImageWriter.h>

OssimAppMemorySource::OssimAppMemorySource(RasterElement& cube, 
   const RasterFileDescriptor &exportDescriptor) : 
   mCube(cube),
   mExportDescriptor(exportDescriptor),
   mColSkipFactor(0),
   mRowSkipFactor(0)
{
   RasterUtilities::determineExportSkipFactor(mExportDescriptor.getColumns(), mColSkipFactor);
   RasterUtilities::determineExportSkipFactor(mExportDescriptor.getRows(), mRowSkipFactor);
}

ossimRefPtr<ossimImageData> OssimAppMemorySource::getTile(const ossimIrect& rect, ossim_uint32 resLevel)
{
   if (resLevel > 0)
   {
      return NULL;
   }

   const RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(mCube.getDataDescriptor());
   VERIFYRV(pDescriptor != NULL, NULL);

   unsigned int bpp = pDescriptor->getBytesPerElement();

   int minx, miny, maxx, maxy;
   rect.getBounds(minx, miny, maxx, maxy);

   VERIFYRV(minx <= maxx && miny <= maxy, NULL);

   int numRows = maxy-miny+1;
   int numCols = maxx-minx+1;

   maxx = min(maxx, static_cast<int>(mExportDescriptor.getColumnCount()-1));
   maxy = min(maxy, static_cast<int>(mExportDescriptor.getRowCount()-1));
   unsigned int bands = mExportDescriptor.getBandCount();

   int occupiedRows = maxy-miny+1;
   int occupiedCols = maxx-minx+1;

   ossimRefPtr<ossimImageData> pData = ossimImageDataFactory::instance()->create(this, getOutputScalarType(),
                                                                                 bands, numCols, numRows);

   if (pData.get() != NULL)
   {
      pData->initialize();

      for (unsigned int b = 0; b < bands; ++b)
      {
         FactoryResource<DataRequest> pRequest;
         if (mColSkipFactor == 0)
         {
            pRequest->setInterleaveFormat(BSQ); // If BSQ and no skip, we can do a single memcpy
         }
         pRequest->setRows(mExportDescriptor.getOnDiskRow(miny), mExportDescriptor.getOnDiskRow(maxy));
         pRequest->setColumns(mExportDescriptor.getOnDiskColumn(minx), 
            mExportDescriptor.getOnDiskColumn(maxx));
         pRequest->setBands(mExportDescriptor.getOnDiskBand(b), mExportDescriptor.getOnDiskBand(b));
         DataAccessor da = mCube.getDataAccessor(pRequest.release());
         if (da.isValid() == false)
         {
            return NULL;
         }

         char* pBuf = static_cast<char*>(pData->getBuf(b));
         if (pBuf == NULL)
         {
            return NULL;
         }

         for (int row = 0; row < occupiedRows; ++row)
         {
            if (da.isValid() == false)
            {
               return NULL;
            }
            if (mColSkipFactor == 0)
            {
               memcpy(pBuf, da->getRow(), occupiedCols*bpp);
            }
            else
            {
               for (int col = 0; col < occupiedCols; ++col)
               {
                  memcpy(pBuf+(col*bpp), da->getColumn(), bpp);
                  da->nextColumn(mColSkipFactor+1);
               }
            }
            pBuf += numCols*bpp;

            da->nextRow(mRowSkipFactor+1, true);
         }
      }

      pData->validate();
   }
   return pData;
}

ossimScalarType OssimAppMemorySource::getOutputScalarType() const
{
   const RasterDataDescriptor* pDescriptor = 
      dynamic_cast<const RasterDataDescriptor*>(mCube.getDataDescriptor());
   VERIFYRV(pDescriptor != NULL, OSSIM_SCALAR_UNKNOWN);
   
#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : TODO: Support Complex Data (leckels)")
   switch(pDescriptor->getDataType())
   {
   case INT1SBYTE:
      return OSSIM_SINT8;
   case INT1UBYTE:
      return OSSIM_UINT8;
   case INT2SBYTES:
      return OSSIM_SINT16;
   case INT2UBYTES:
      return OSSIM_UINT16;
   case INT4SBYTES:
      return OSSIM_SINT32;
   case INT4UBYTES:
      return OSSIM_UINT32;
   case FLT4BYTES:
      return OSSIM_FLOAT32;
   case FLT8BYTES:
      return OSSIM_FLOAT64;
   default:
      return OSSIM_SCALAR_UNKNOWN;
   }
}

ossim_uint32 OssimAppMemorySource::getTileWidth() const
{
   return static_cast<ossim_uint32>(mExportDescriptor.getColumnCount());
}

ossim_uint32 OssimAppMemorySource::getTileHeight() const
{
   return static_cast<ossim_uint32>(mExportDescriptor.getRowCount());
}

ossimIrect OssimAppMemorySource::getBoundingRect(ossim_uint32 resLevel) const
{
   return ossimIrect(0, 0, (getTileWidth()-1)/(resLevel+1), (getTileHeight()-1)/(resLevel+1));
}

ossim_uint32 OssimAppMemorySource::getNumberOfInputBands() const
{
   const RasterDataDescriptor *pDescriptor = 
      dynamic_cast<const RasterDataDescriptor*>(mCube.getDataDescriptor());
   VERIFYRV(pDescriptor != NULL, 0);
   return static_cast<ossim_uint32>(pDescriptor->getBandCount());
}

bool OssimAppMemorySource::canConnectMyInputTo(ossim_int32 inputIndex, const ossimConnectableObject* pObj) const
{
   return pObj != NULL && PTR_CAST(ossimImageWriter, pObj);
}
