/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef INMEMORYPAGER_H
#define INMEMORYPAGER_H

#include "RasterPagerShell.h"

class RasterElement;

class InMemoryPager : public RasterPagerShell
{
public:
   InMemoryPager();
   ~InMemoryPager();

   bool getInputSpecification(PlugInArgList*& pArgList);
   bool execute(PlugInArgList* pInput, PlugInArgList* pOutput);

   RasterPage* getPage(DataRequest* pOriginalRequest, DimensionDescriptor startRow, DimensionDescriptor startColumn,
      DimensionDescriptor startBand);
   void releasePage(RasterPage* pPage);

   int getSupportedRequestVersion() const;

private:
   RasterElement* mpRaster;
   void* mpData;
   bool mbOwner;
};

#endif
