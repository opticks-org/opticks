/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef NITFPAGER_H
#define NITFPAGER_H

#include "CachedPager.h"
#include "NitfResource.h"
#include "PlugInArg.h"

#include <vector>

class NITF_IMAGE_SEGMENT_BASE;

namespace Nitf
{
   class Pager : public CachedPager
   {
   public:
      Pager();
      virtual ~Pager();

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
      int mSegment; // 1-based segment number
      Nitf::OssimImageHandlerResource mpImageHandler;
      Step* mpStep;
   };
}

#endif
