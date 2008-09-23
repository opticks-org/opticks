/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef INMEMORYPAGE_H
#define INMEMORYPAGE_H

#include "RasterPage.h"

class InMemoryPage : public RasterPage
{
public:
   InMemoryPage(void *pData, unsigned int numRows);
   ~InMemoryPage();

   void *getRawData();
   unsigned int getNumRows();
   unsigned int getNumColumns();
   unsigned int getNumBands();
   unsigned int getInterlineBytes();

private:
   void *mpData;
   unsigned int mNumRows;
};

#endif