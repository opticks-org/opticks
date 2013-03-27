/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ConvertToBipPage.h"

ConvertToBipPage::ConvertToBipPage(unsigned int rows, unsigned int columns, unsigned int bands,
                                   unsigned int bytesPerElement) :
   mCache(rows * columns * bands * bytesPerElement, true),
   mRows(rows),
   mColumns(columns),
   mBands(bands)
{
}

ConvertToBipPage::~ConvertToBipPage()
{
}

unsigned int ConvertToBipPage::getNumBands()
{
   return mBands;
}

unsigned int ConvertToBipPage::getNumRows()
{
   return mRows;
}

unsigned int ConvertToBipPage::getNumColumns()
{
   return mColumns;
}

unsigned int ConvertToBipPage::getInterlineBytes()
{
   return 0;
}

void* ConvertToBipPage::getRawData()
{
   return mCache.get();
}
