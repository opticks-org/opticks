/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef POINTCLOUDINMEMORYPAGER_H
#define POINTCLOUDINMEMORYPAGER_H

#include "PointCloudPagerShell.h"

class PointCloudDataDescriptor;
class PointCloudDataRequest;

class PointCloudInMemoryPager : public PointCloudPagerShell
{
public:
   PointCloudInMemoryPager();
   ~PointCloudInMemoryPager();

   bool initialize(PointCloudDataDescriptor* pDescriptor, uint32_t numPoints);
   bool getInputSpecification(PlugInArgList*& pArgList);
   bool execute(PlugInArgList* pInput, PlugInArgList* pOutput);

   virtual PointDataBlock* getPointBlock(uint32_t startIndex, uint32_t numPoints, PointCloudDataRequest* pOriginalRequest);
   virtual void releasePointBlock(PointDataBlock* pBlock);

private:
   char* mpData;
   uint64_t mPointSizeInBytes;
   uint32_t mPointCount;
};

#endif
