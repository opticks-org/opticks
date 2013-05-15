/*
 * The information in this file is
 * Copyright(c) 2012 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "Jpeg2000Utilities.h"
#include "Progress.h"

#include <QtCore/QFileInfo>

#include <string>

namespace Jpeg2000Utilities
{
   int get_file_format(const char* pFilename)
   {
      QFileInfo info(pFilename);
      if (info.suffix() == "jp2")
      {
         return JP2_CFMT;
      }
      else if ((info.suffix() == "j2k") || (info.suffix() == "j2c") || (info.suffix() == "jpc"))
      {
         return J2K_CFMT;
      }
      return -1;
   }

   unsigned int get_num_bands(EncodingType type)
   {
      // Since OpenJPEG encoding only supports 16-bit lossless, determine the number
      // of 16-bit bands which need to be used to represent the given data type.
      switch (type)
      {
         case INT1UBYTE:    // Fall through
         case INT1SBYTE:    // Fall through
         case INT2UBYTES:   // Fall through
         case INT2SBYTES:
            return 1;
         case INT4UBYTES:   // Fall through
         case INT4SBYTES:   // Fall through
         case FLT4BYTES:
            return 2;
         default:
            return 0; // Unsupported type
      }
   }

   void reportMessage(const char* pMessage, void* pClientData)
   {
      Progress* pProgress = reinterpret_cast<Progress*>(pClientData);
      if ((pProgress != NULL) && (pMessage != NULL))
      {
         std::string currentMessage;
         int percent = 0;
         ReportingLevel level;
         pProgress->getProgress(currentMessage, percent, level);

         pProgress->updateProgress(std::string(pMessage), percent, NORMAL);
      }
   }

   void reportWarning(const char* pMessage, void* pClientData)
   {
      Progress* pProgress = reinterpret_cast<Progress*>(pClientData);
      if ((pProgress != NULL) && (pMessage != NULL))
      {
         pProgress->updateProgress(std::string(pMessage), 0, WARNING);
      }
   }

   void reportError(const char* pMessage, void* pClientData)
   {
      Progress* pProgress = reinterpret_cast<Progress*>(pClientData);
      if ((pProgress != NULL) && (pMessage != NULL))
      {
         pProgress->updateProgress(std::string(pMessage), 0, ERRORS);
      }
   }
}
