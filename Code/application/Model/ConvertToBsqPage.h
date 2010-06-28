/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CONVERTTOBSQPAGE_H
#define CONVERTTOBSQPAGE_H

#include "ObjectResource.h"
#include "RasterPage.h"

/**
 * This class works with ConvertToBsqPager
 * to convert BIP or BIL data to BSQ.
 */
class ConvertToBsqPage : public RasterPage
{
public:
   ConvertToBsqPage(unsigned int rows, unsigned int columns, unsigned int bytesPerElement);
   virtual ~ConvertToBsqPage();

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
};

#endif
