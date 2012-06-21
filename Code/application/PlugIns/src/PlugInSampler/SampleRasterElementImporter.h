/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SAMPLERASTERELEMENTIMPORTER_H
#define SAMPLERASTERELEMENTIMPORTER_H

#include "RasterElementImporterShell.h"
#include "RasterPage.h"
#include "RasterPager.h"

#include <vector>

class SampleRasterPage : public RasterPage
{
public:
   SampleRasterPage(int row, int col);
   virtual ~SampleRasterPage();

   virtual void* getRawData();
   virtual unsigned int getNumRows();
   virtual unsigned int getNumColumns();
   virtual unsigned int getNumBands();
   virtual unsigned int getInterlineBytes();

private:
   int mRow;
   int mCol;
   std::vector<unsigned char> mData;
};

class SampleRasterPager : public RasterPager
{
public:
   SampleRasterPager();
   virtual ~SampleRasterPager();

   virtual RasterPage* getPage(DataRequest* pOriginalRequest, DimensionDescriptor startRow,
      DimensionDescriptor startColumn, DimensionDescriptor startBand);
   virtual void releasePage(RasterPage* pPage);
   virtual int getSupportedRequestVersion() const;
};

class SampleRasterElementImporter : public RasterElementImporterShell
{
public:
   SampleRasterElementImporter();
   virtual ~SampleRasterElementImporter();

   virtual unsigned char getFileAffinity(const std::string& filename);
   virtual bool isProcessingLocationSupported(ProcessingLocation location) const;
   virtual std::vector<ImportDescriptor*> getImportDescriptors(const std::string& filename);
   virtual bool createRasterPager(RasterElement* pRaster) const;
};

#endif
