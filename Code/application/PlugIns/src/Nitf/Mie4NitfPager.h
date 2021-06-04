/*
 * The information in this file is
 * Copyright(c) 2021 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MIE4NITFNITFPAGER_H
#define MIE4NITFNITFPAGER_H

#include "CachedPager.h"
#include "NitfResource.h"
#include "PlugInArg.h"
#include "TypesFile.h"

#include <vector>

class NITF_IMAGE_SEGMENT_BASE;

namespace Nitf
{
   class Mie4NitfPager : public CachedPager
   {
   public:
      static std::string const FrameImageSegmentsArg() { return "Frame Image Segments"; }
      static std::string const StartFramesArg() { return "Start Frames"; }
      static std::string const FrameFilesArg() { return "Frame Files"; }
      static std::string const OffsetsArg() { return "Offsets"; }
      static std::string const SizesArg() { return "Sizes"; }

   public:
      typedef std::map<unsigned int, std::pair<std::string, unsigned int> > FrameIndexType;

   public:
      Mie4NitfPager();
      virtual ~Mie4NitfPager();

      bool getInputSpecification(PlugInArgList *&pArgList);

      bool parseInputArgs(PlugInArgList* pInputArgList);

   private:
      /**
       *  This method should be implemented to open the file and store a file handle to be
       *  closed upon destruction.
       *
       *  Open the file and maintain handles in derived class constructor, close in destructor.
       *
       *  @param   filename
       *           The file name to open.
       *
       *  @return  TRUE if the open succeeds, FALSE otherwise.
       */
      virtual bool openFile(const std::string& filename);

      virtual CachedPage::UnitPtr fetchUnit(DataRequest *pOriginalRequest);

   private:
      Mie4NitfPager& operator=(const Mie4NitfPager& rhs);
      FrameIndexType mFrameIndex;
      std::map<std::string, Nitf::OssimImageHandlerResource> mImageHandlers;
      EncodingType mFileEncoding;
      EncodingType mEncoding;
      float mScaleFactor;
      float mAdditiveFactor;
   };
}

#endif

