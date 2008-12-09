/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef MEMORYMAPPEDPAGER_H
#define MEMORYMAPPEDPAGER_H

#include "RasterPagerShell.h"
#include "DMutex.h"

#include <vector>
#include <map>

class RasterDataDescriptor;
class MemoryMappedPage;
class MemoryMappedMatrix;

class MemoryMappedPager : public RasterPagerShell
{
public:
   MemoryMappedPager();
   ~MemoryMappedPager();

   // Executable
   virtual bool getInputSpecification(PlugInArgList *&argList);
   virtual bool getOutputSpecification(PlugInArgList *&argList);
   virtual bool execute(PlugInArgList *pInputArgList,
                        PlugInArgList *pOutputArgList);

   //RasterPager methods
   RasterPage* getPage(DataRequest *pOriginalRequest, 
      DimensionDescriptor startRow,
      DimensionDescriptor startColumn,
      DimensionDescriptor startBand);
   void releasePage(RasterPage *pPage);
   int getSupportedRequestVersion() const;


private:
   bool mbUseDataDescriptor;
   const RasterDataDescriptor* mpDataDescriptor;
   bool mSwapEndian;

   // All leased blocks, and the matrices that they came from.
   // Not all matrices will be represented here at any given time.
   std::map<MemoryMappedPage*, MemoryMappedMatrix*> mCurrentlyLeasedPages;
   std::vector<MemoryMappedMatrix*>      mMatrices;
   
   mta::DMutex                           mMutex;

   bool mWritable;
};

#endif
