/*
 * The information in this file is
 * Copyright(c) 2012 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef JPEG2000UTILITIES_H
#define JPEG2000UTILITIES_H

#include "EnumWrapper.h"
#include "TypesFile.h"

namespace Jpeg2000Utilities
{
   enum Jpeg2000FileTypeEnum
   {
      J2K_CFMT,
      JP2_CFMT
   };
   typedef EnumWrapper<Jpeg2000FileTypeEnum> Jpeg2000FileType;

   int get_file_format(const char* pFilename);
   unsigned int get_num_bands(EncodingType type);

   void reportMessage(const char* pMessage, void* pClientData);
   void reportWarning(const char* pMessage, void* pClientData);
   void reportError(const char* pMessage, void* pClientData);
}

#endif
