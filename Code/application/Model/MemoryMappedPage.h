/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef MEMORYMAPPEDPAGE_H
#define MEMORYMAPPEDPAGE_H

#include "RasterPage.h"

class MemoryMappedMatrixView;

class MemoryMappedPage : public RasterPage
{
public:
   MemoryMappedPage();
   ~MemoryMappedPage();

   void* getRawData();
   void setRawData(char* data);

   unsigned int getNumRows();
   void setNumRows(unsigned int val);
   unsigned int getNumColumns();
   void setNumColumns(unsigned int val);
   void setInterlineBytes(unsigned int val);
   unsigned int getNumBands();   
   unsigned int getInterlineBytes();

   void setMemoryMappedMatrixView(MemoryMappedMatrixView* pView);
   MemoryMappedMatrixView* getMemoryMappedMatrixView();

private:
   unsigned int mNumRows;
   unsigned int mNumColumns;
   unsigned int mInterlineBytes;
   char* mpRawCubePointer;
   MemoryMappedMatrixView* mpMatrixView;
};

#endif
