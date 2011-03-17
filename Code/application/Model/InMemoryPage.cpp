/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "InMemoryPage.h"

InMemoryPage::InMemoryPage(void *pData, unsigned int numRows) : mpData(pData), mNumRows(numRows)
{
}

InMemoryPage::~InMemoryPage()
{
}

void *InMemoryPage::getRawData()
{
   return mpData;
}

unsigned int InMemoryPage::getNumRows()
{
   return mNumRows;
}

unsigned int InMemoryPage::getNumColumns()
{
   return 0;
}

unsigned int InMemoryPage::getNumBands()
{
   return 0;
}

unsigned int InMemoryPage::getInterlineBytes()
{
   return 0;
}
