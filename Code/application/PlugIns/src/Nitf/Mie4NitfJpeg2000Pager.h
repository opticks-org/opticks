/*
 * The information in this file is
 * Copyright(c) 2021 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MIE4NITFJPEG2000PAGER_H
#define MIE4NITFJPEG2000PAGER_H

#include "CachedPager.h"
#include "Mie4NitfPager.h"

#include <boost/tuple/tuple.hpp>
#include <openjpeg.h>
#include <stdio.h>
#include <stdint.h>
#include <string>

class Mie4NitfJpeg2000Pager : public CachedPager
{
public:
   static const std::string const FrameImageSegmentsArg() { return "Frame Image Segments"; }

public:
   typedef std::map<unsigned int, boost::tuple<std::string, uint64_t, uint64_t> > FrameIndexType;

public:
   Mie4NitfJpeg2000Pager();
   virtual ~Mie4NitfJpeg2000Pager();

   virtual bool getInputSpecification(PlugInArgList*& pArgList);
   virtual bool parseInputArgs(PlugInArgList* pArgList);

   virtual bool openFile(const std::string& filename);
   virtual CachedPage::UnitPtr fetchUnit(DataRequest* pOriginalRequest);

protected:
   virtual double getChunkSize() const;

   template <typename Out>
   CachedPage::UnitPtr populateImageData(const DimensionDescriptor& startRow, const DimensionDescriptor& startColumn,
      unsigned int concurrentRows, unsigned int concurrentColumns, uint64_t offset, uint64_t size) const;

   opj_image_t* decodeImage(unsigned int originalStartRow, unsigned int originalStartColumn,
      unsigned int originalStopRow, unsigned int originalStopColumn, int decoderType, uint64_t offset, uint64_t size) const;

private:
   static size_t msMaxCacheSize;
   FrameIndexType mFrameIndex;

   char *mpFilename;
   FILE* mpFile;
};

#endif

