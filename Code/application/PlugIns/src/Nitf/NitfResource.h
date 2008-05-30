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
   class OssimFileObject
   {
   public:
      struct Args
      {
         std::string mFilename;
         std::string mAccess;
         Args(std::string filename, std::string access="r") : mFilename(filename), mAccess(access) {}
      };

      ossimNitfFile* obtainResource(const Args &args) const;
      void releaseResource(const Args &args, ossimNitfFile* pHandler) const;
   };
   
   class OssimFileResource : public Resource<ossimNitfFile, Nitf::OssimFileObject>
   {
   public:
      OssimFileResource(const std::string& filename, const std::string& args = "r") :
         Resource<ossimNitfFile, Nitf::OssimFileObject>(OssimFileObject::Args(filename.c_str(), args.c_str()))
      {
      }
   };

   class OssimImageHandlerObject
   {
   public:
      struct Args
      {
         std::string mFilename;
         Args(const std::string &filename) : mFilename(filename) {}
      };

      ossimImageHandler *obtainResource(const Args &args) const;
      void releaseResource(const Args &args, ossimImageHandler *pHandler);
   };

   class OssimImageHandlerResource : public Resource<ossimImageHandler, Nitf::OssimImageHandlerObject>
   {
   public:
      OssimImageHandlerResource(const std::string &filename = "") : 
         Resource<ossimImageHandler, Nitf::OssimImageHandlerObject>(OssimImageHandlerObject::Args(filename))
      {
      }
   };
}
#endif
