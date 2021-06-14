/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
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
   /**
    * JPEG2000 container file types.
    */
   enum Jpeg2000FileTypeEnum
   {
      J2K_CFMT,
      JP2_CFMT
   };
   typedef EnumWrapper<Jpeg2000FileTypeEnum> Jpeg2000FileType;

   /**
    * Get the container file type for a specified file.
    *
    * @param pFilename
    *        The file to check.
    * @return A Jpeg2000FileTypeEnum with the type of container file.
    */
   int get_file_format(const char* pFilename);
   
   /**
    * Get the number of 16-bit lossless bands needed to represent a specific encoding.
    *
    * @param type
    *        The encoding type.
    * @return The number of bands needed or 0 for unsupported encodings.
    */
   unsigned int get_num_bands(EncodingType type);

   /**
    * OpenJpeg callback for message logging to an opticks Progress.
    *
    * @param pMessage
    *        The message to log.
    * @param pClientData
    *        The Progress object for logging or NULL if no progress exists.
    */
   void reportMessage(const char* pMessage, void* pClientData);

   /**
    * OpenJpeg callback for warning logging to an opticks Progress.
    *
    * @param pMessage
    *        The message to log.
    * @param pClientData
    *        The Progress object for logging or NULL if no progress exists.
    */
   void reportWarning(const char* pMessage, void* pClientData);

   /**
    * OpenJpeg callback for error logging to an opticks Progress.
    *
    * @param pMessage
    *        The message to log.
    * @param pClientData
    *        The Progress object for logging or NULL if no progress exists.
    */
   void reportError(const char* pMessage, void* pClientData);
}

#endif
