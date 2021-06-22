/*
 * The information in this file is
 * Copyright(c) 2021 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef RASTERDATACOPYCONTROLLER_H__
#define RASTERDATACOPYCONTROLLER_H__

#include "DataAccessor.h"
#include "ProgressResource.h"
#include <QtCore/QEvent>
#include <QtCore/QObject>
#include <vector>

class DimensionDescriptor;
class RasterElement;

class RasterDataCopyController : public QObject
{
   Q_OBJECT

public:
   RasterDataCopyController(const RasterElement* pRaster,
      RasterElement* pDestRaster,
      const std::vector<DimensionDescriptor>& selectedRows,
      const std::vector<DimensionDescriptor>& selectedColumns,
      const std::vector<DimensionDescriptor>& selectedBands,
      QObject* pParent=NULL);
   virtual ~RasterDataCopyController();

   void startCopy();

   virtual bool event(QEvent* pEvent) override;

private:
   static const QEvent::Type CopyChunk;
   ProgressResource mpProgress;
   const RasterElement* mpRaster;
   RasterElement* mpDestRaster;
   std::vector<DimensionDescriptor> mRows;
   std::vector<DimensionDescriptor> mColumns;
   std::vector<DimensionDescriptor> mBands;
   InterleaveFormatType mInterleave;  // for convenience
   size_t mChunkSize;

   DataAccessor mSrcAcc;
   DataAccessor mDstAcc;

   std::vector<DimensionDescriptor>::const_iterator mFirstIter;
   std::vector<DimensionDescriptor>::const_iterator mSecondIter;
};

#endif
