/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ConvertToBsqPage.h"

ConvertToBsqPage::ConvertToBsqPage(unsigned int rows, unsigned int columns, unsigned int bytesPerElement) :
   mCache(rows * columns * bytesPerElement, true),
   mRows(rows),
   mColumns(columns)
{
}

ConvertToBsqPage::~ConvertToBsqPage()
{
}

unsigned int ConvertToBsqPage::getNumBands()
{
   return 1;
}

unsigned int ConvertToBsqPage::getNumRows()
{
   return mRows;
}

unsigned int ConvertToBsqPage::getNumColumns()
{
   return mColumns;
}

unsigned int ConvertToBsqPage::getInterlineBytes()
{
   return 0;
}

void* ConvertToBsqPage::getRawData()
{
   return mCache.get();
}
