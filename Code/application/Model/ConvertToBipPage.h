/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CONVERTTOBIPPAGE_H
#define CONVERTTOBIPPAGE_H

#include "ObjectResource.h"
#include "RasterPage.h"

/**
 * This class works with ConvertToBipPager
 * to convert BSQ or BIL data to BIP.
 */
class ConvertToBipPage : public RasterPage
{
public:
   ConvertToBipPage(unsigned int rows, unsigned int columns, unsigned int bands, unsigned int bytesPerElement);
   virtual ~ConvertToBipPage();

   // RasterPage methods
   unsigned int getNumBands();
   unsigned int getNumRows();
   unsigned int getNumColumns();
   unsigned int getInterlineBytes();
   void* getRawData();

private:
   ArrayResource<unsigned char> mCache;

   unsigned int mRows;
   unsigned int mColumns;
   unsigned int mBands;
};

#endif
