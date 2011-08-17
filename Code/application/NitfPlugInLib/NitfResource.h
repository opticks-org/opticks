/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef NITFRESOURCE_H
#define NITFRESOURCE_H

#include "MessageLogResource.h"

#include <string>

class ossimNitfFile;
class ossimImageHandler;
class Progress;

namespace Nitf
{
   /**
    *  \cond INTERNAL
    * The %OssimFileObject is a trait object for use with the %Resource template.
    *
    * It provides capability for opening and closing NITF files with OSSIM.
    *
    * @see OssimFileResource
    */
   class OssimFileObject
   {
   public:
      struct Args
      {
         Args(std::string filename, std::string access = "r") :
            mFilename(filename),
            mAccess(access) {}

         std::string mFilename;
         std::string mAccess;
      };

      ossimNitfFile* obtainResource(const Args &args) const;
      void releaseResource(const Args& args, ossimNitfFile* pFile) const;
   };
   /// \endcond

   /**
    * This is a %Resource class that opens and closes NITF files with OSSIM.
   */
   class OssimFileResource : public Resource<ossimNitfFile, Nitf::OssimFileObject>
   {
   public:
      /**
       * Construct an object to represent a file to be opened with OSSIM.
       *
       * @param   filename
       *          The name of the file to open.
       * @param   args
       *          The access mode to open the file with.  These match the modes
       *          used with fopen().
       */
      OssimFileResource(const std::string& filename, const std::string& args = "r") :
         Resource<ossimNitfFile, Nitf::OssimFileObject>(OssimFileObject::Args(filename.c_str(), args.c_str())) {}
   };

   /**
    *  \cond INTERNAL
    * The %OssimImageHandlerObject is a trait object for use with the %Resource template.
    *
    * It provides capability for handling images with OSSIM.
    *
    * @see OssimFileResource
    */
   class OssimImageHandlerObject
   {
   public:
      struct Args
      {
         Args(const std::string& filename) :
            mFilename(filename) {}

         std::string mFilename;
      };

      ossimImageHandler* obtainResource(const Args& args) const;
      void releaseResource(const Args& args, ossimImageHandler* pHandler);
   };
   /// \endcond

   /**
    * This is a %Resource class that opens and closes images with OSSIM.
    */
   class OssimImageHandlerResource : public Resource<ossimImageHandler, Nitf::OssimImageHandlerObject>
   {
   public:
      /**
       * Construct an object to represent images within a file to be opened with OSSIM.
       *
       * @param   filename
       *          The name of the file to open.
       */
      OssimImageHandlerResource(const std::string& filename = "") : 
         Resource<ossimImageHandler, Nitf::OssimImageHandlerObject>(OssimImageHandlerObject::Args(filename)) {}
   };
}
#endif
