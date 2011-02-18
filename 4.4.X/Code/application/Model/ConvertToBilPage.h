/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CONVERTTOBILPAGE_H
#define CONVERTTOBILPAGE_H

#include "ObjectResource.h"
#include "RasterPage.h"

/**
 * This class works with ConvertToBilPager
 * to convert BIP or BSQ data to BIL.
 */
class ConvertToBilPage : public RasterPage
{
public:
   ConvertToBilPage(unsigned int rows, unsigned int columns, unsigned int bands, unsigned int bytesPerElement);
   virtual ~ConvertToBilPage();

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
