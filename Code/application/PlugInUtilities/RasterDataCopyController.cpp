/*
 * The information in this file is
 * Copyright(c) 2021 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "DataAccessor.h"
#include "DataAccessorImpl.h"
#include "DataRequest.h"
#include "DimensionDescriptor.h"
#include "ObjectResource.h"
#include "RasterDataCopyController.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QEvent>
#include <vector>

const QEvent::Type RasterDataCopyController::CopyChunk = QEvent::User;

RasterDataCopyController::RasterDataCopyController(const RasterElement* pRaster,
                                                   RasterElement* pDestRaster,
                                                   const std::vector<DimensionDescriptor>& selectedRows,
                                                   const std::vector<DimensionDescriptor>& selectedColumns,
                                                   const std::vector<DimensionDescriptor>& selectedBands,
                                                   QObject* pParent) :
   QObject(pParent),
   mpRaster(pRaster),
   mpDestRaster(pDestRaster),
   mRows(selectedRows),
   mColumns(selectedColumns),
   mBands(selectedBands),
   mSrcAcc(NULL, NULL),
   mDstAcc(NULL, NULL)
{
   ENSURE(mpRaster != nullptr);
   ENSURE(mpDestRaster != nullptr);
   const RasterDataDescriptor* pSrcDd = dynamic_cast<const RasterDataDescriptor*>(mpRaster->getDataDescriptor());
   ENSURE(pSrcDd != nullptr);

   Service<DesktopServices>()->createStatusBarProgress(mpProgress.get());

   RasterDataDescriptor* pDstDd = dynamic_cast<RasterDataDescriptor*>(mpDestRaster->getDataDescriptor());
   ENSURE(pDstDd != nullptr);
   mInterleave = pDstDd->getInterleaveFormat();

   switch (mInterleave)
   {
   case BIP:
   {
      if (mBands.size() == pSrcDd->getBandCount())
      {
         // all bands
         if (mColumns.size() == mColumns.back().getActiveNumber() - mColumns.front().getActiveNumber() + 1)
         {
            // full contiguous row at a time
            FactoryResource<DataRequest> pSrcRequest;
            pSrcRequest->setInterleaveFormat(BIP);
            pSrcRequest->setRows(mRows.front(), mRows.back());
            pSrcRequest->setColumns(mColumns.front(), mColumns.back());
            mSrcAcc = mpRaster->getDataAccessor(pSrcRequest.release());

            FactoryResource<DataRequest> pDstRequest;
            pDstRequest->setWritable(true);
            pDstRequest->setInterleaveFormat(BIP);
            mDstAcc = mpDestRaster->getDataAccessor(pDstRequest.release());

            ENSURE(mSrcAcc.isValid() && mDstAcc.isValid());
            mFirstIter = mRows.begin();
            mSecondIter = mColumns.end();

            mChunkSize = pSrcDd->getBytesPerElement() * mBands.size() * mColumns.size();
         }
      }
      break;
   }
   case BIL:
   case BSQ:
   default:
      ENSURE(false);
      break;
   }
}

RasterDataCopyController::~RasterDataCopyController()
{}

void RasterDataCopyController::startCopy()
{
   QCoreApplication::postEvent(this, new QEvent(RasterDataCopyController::CopyChunk));
}

bool RasterDataCopyController::event(QEvent* pEvent)
{
   if (pEvent->type() == RasterDataCopyController::CopyChunk)
   {
      switch (mInterleave)
      {
      case BIP:
      {
         DimensionDescriptor rowDim = *mFirstIter;
         DimensionDescriptor colDim = (mSecondIter != mColumns.end()) ? *mSecondIter : mColumns.front();
         mSrcAcc->toPixel(rowDim.getActiveNumber(), colDim.getActiveNumber());
         VERIFYRV(mSrcAcc.isValid() && mDstAcc.isValid(), true);

         const char* pSrc = reinterpret_cast<const char*>(mSrcAcc->getRow());
         char* pDst = reinterpret_cast<char*>(mDstAcc->getRow());
         memcpy(pDst, pSrc, mChunkSize);
         mDstAcc->nextRow();
         mpProgress->updateProgress("Copying data", (mFirstIter->getActiveNumber() * 100) / mRows.size(), NORMAL);
         if (++mFirstIter == mRows.end())
         {
            deleteLater();
         }
         else
         {
            QCoreApplication::postEvent(this, new QEvent(RasterDataCopyController::CopyChunk));
         }
         break;
      }
      default:
         return true;
      }
      return true;
   }
   return QObject::event(pEvent);
}
