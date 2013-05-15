/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef JPEG2000PAGER_H
#define JPEG2000PAGER_H

#include "CachedPager.h"

#include <openjpeg.h>
#include <stdio.h>
#include <string>

class Jpeg2000Pager : public CachedPager
{
public:
   Jpeg2000Pager();
   virtual ~Jpeg2000Pager();

   static std::string offsetArg();
   static std::string sizeArg();

   virtual bool getInputSpecification(PlugInArgList*& pArgList);
   virtual bool parseInputArgs(PlugInArgList* pArgList);

   virtual bool openFile(const std::string& filename);
   virtual CachedPage::UnitPtr fetchUnit(DataRequest* pOriginalRequest);

protected:
   virtual double getChunkSize() const;

   template <typename Out>
   CachedPage::UnitPtr populateImageData(const DimensionDescriptor& startRow, const DimensionDescriptor& startColumn,
      unsigned int concurrentRows, unsigned int concurrentColumns) const;

   opj_image_t* decodeImage(unsigned int originalStartRow, unsigned int originalStartColumn,
      unsigned int originalStopRow, unsigned int originalStopColumn, int decoderType) const;

private:
   static size_t msMaxCacheSize;

   FILE* mpFile;
   uint64_t mOffset;
   uint64_t mSize;
};

#endif
