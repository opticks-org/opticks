/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#include "MemoryMappedPage.h"
#include "MemoryMappedMatrixView.h"

MemoryMappedPage::MemoryMappedPage()
:  mNumColumns(0), mNumRows(0), mInterlineBytes(0), mpRawCubePointer(NULL), mpMatrixView(NULL)
{
}

MemoryMappedPage::~MemoryMappedPage()
{
   if (mpMatrixView != NULL)
   {
      delete mpMatrixView;
   }
}

void* MemoryMappedPage::getRawData()
{
   return mpRawCubePointer;
}

void MemoryMappedPage::setRawData(char* data)
{
   mpRawCubePointer = data;
}

unsigned int MemoryMappedPage::getNumRows()
{
   return mNumRows;
}

void MemoryMappedPage::setNumRows(unsigned int val)
{
   mNumRows = val;
}

unsigned int MemoryMappedPage::getNumColumns()
{
   return mNumColumns;
}

void MemoryMappedPage::setNumColumns(unsigned int val)
{
   mNumColumns = val;
}

void MemoryMappedPage::setInterlineBytes(unsigned int val)
{
   mInterlineBytes = val;
}

unsigned int MemoryMappedPage::getNumBands()
{
   return 0;
}

unsigned int MemoryMappedPage::getInterlineBytes()
{
   return mInterlineBytes;
}

void MemoryMappedPage::setMemoryMappedMatrixView(MemoryMappedMatrixView* pView)
{
   if (pView == mpMatrixView) return;

   if (mpMatrixView != NULL)
   {
      delete mpMatrixView;
   }
   mpMatrixView = pView;
}

MemoryMappedMatrixView* MemoryMappedPage::getMemoryMappedMatrixView()
{
   return mpMatrixView;
}
