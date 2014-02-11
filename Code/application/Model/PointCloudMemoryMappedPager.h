/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef POINTCLOUDMEMORYMAPPEDPAGER_H
#define POINTCLOUDMEMORYMAPPEDPAGER_H

#include "PointCloudPagerShell.h"
#include "DMutex.h"

#include <vector>

class MemoryMappedDataBlock;
class MemoryMappedArray;

class PointCloudMemoryMappedPager : public PointCloudPagerShell
{
public:
   PointCloudMemoryMappedPager();
   ~PointCloudMemoryMappedPager();

   bool initialize(const std::string& fileName, uint32_t bytesPerElement, bool writable);

   virtual bool getInputSpecification(PlugInArgList *&argList);
   virtual bool execute(PlugInArgList *pInputArgList,
                        PlugInArgList *pOutputArgList);

   virtual PointDataBlock* getPointBlock(uint32_t startIndex, uint32_t numPoints, PointCloudDataRequest* pOriginalRequest);
   virtual void releasePointBlock(PointDataBlock* pBlock);

private:
   std::vector<MemoryMappedDataBlock*> mCurrentlyLeasedPages;
   MemoryMappedArray* mMemMapper;   
   mta::DMutex mMutex;
   uint32_t mBytesPerElement;
   bool mWritable;
};

#endif
