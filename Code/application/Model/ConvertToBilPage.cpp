/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ConvertToBilPage.h"

ConvertToBilPage::ConvertToBilPage(unsigned int rows, unsigned int columns, unsigned int bands,
                                   unsigned int bytesPerElement) :
   mCache(rows * columns * bands * bytesPerElement, true),
   mRows(rows),
   mColumns(columns),
   mBands(bands)
{
}

ConvertToBilPage::~ConvertToBilPage()
{
}

unsigned int ConvertToBilPage::getNumBands()
{
   return mBands;
}

unsigned int ConvertToBilPage::getNumRows()
{
   return mRows;
}

unsigned int ConvertToBilPage::getNumColumns()
{
   return mColumns;
}

unsigned int ConvertToBilPage::getInterlineBytes()
{
   return 0;
}

void* ConvertToBilPage::getRawData()
{
   return mCache.get();
}
