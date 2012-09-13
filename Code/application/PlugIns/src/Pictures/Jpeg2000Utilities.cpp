/*
 * The information in this file is
 * Copyright(c) 2012 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"
#if defined (JPEG2000_SUPPORT)

#include "Jpeg2000Utilities.h"

#include <QtCore/QFileInfo>

namespace Jpeg2000Utilities
{
   int get_file_format(const char* pFilename)
   {
      QFileInfo info(pFilename);
      if (info.suffix() == "jp2")
      {
         return JP2_CFMT;
      }
      else if (info.suffix() == "j2k")
      {
         return J2K_CFMT;
      }
      return -1;
   }

   unsigned int get_num_bands(EncodingType type)
   {
      // Since OpenJPEG only supports 16-bit lossless,
      // determine the number of 16-bit bands which need
      // to be used to represent the given data type.
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

   void error_callback(const char* msg, void *client_data)
   {
   #ifdef DEBUG
      FILE* stream = (FILE*)client_data;
      fprintf(stream, "[ERROR] %s", msg);
   #endif
   }

   void warning_callback(const char* msg, void* client_data)
   {
   #ifdef DEBUG
      FILE* stream = (FILE*)client_data;
      fprintf(stream, "[WARNING] %s", msg);
   #endif
   }

   void info_callback(const char* msg, void* client_data)
   {
   #ifdef DEBUG
      (void)client_data;
      fprintf(stdout, "[INFO] %s", msg);
   #endif
   }
}

#endif
