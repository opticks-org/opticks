/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ZIPRESOURCE_H
#define ZIPRESOURCE_H

#include "Resource.h"

#include <string>

#include <unzip.h>

/**
 * The %ZipFileObject is a trait object for use with the %Resource template. 
 *
 * The %ZipFileObject is a trait object for use with the %Resource template. It provides capability for opening
 * and closing zip files.
 * 
 */
class ZipFileObject
{
public:
   /**
    * This is an implementation detail of the %ZipFileObject class. 
    *
    * It is used for passing the parameters required by unzOpen.
    */
   struct Args
   {
      /**
       * The file name of the zip file to be opened.
       */
      std::string mFilename;

      /**
       *  Creates a resource to an Zip file handle.
       *
       *  Auto-closes file when the object goes out of scope.
       *  
       *  @param  filename
       *          The name of the file to open.
       */
      Args(std::string filename) :
         mFilename(filename)
      {}

      /**
       * Default constructor
       */
      Args() :
         mFilename()
      {}
   };

   /**
    * Obtains an zip file handle.
    *
    * See Resource.h for details.
    *
    * @param  args
    *         The arguments for obtaining the resource. Should be of type ZipFileObject::Args.
    * @return Returns a pointer to a handle of the file.
    */
   unzFile* obtainResource(const Args &args) const
   {
      unzFile* pHandle = new unzFile;
      *pHandle = unzOpen(args.mFilename.c_str());
      return pHandle;
   }

   /**
    * Releases an zip file handle.
    *
    * See Resource.h for details.
    *
    * @param  args
    *         The arguments for releasing the resource. Should be of type ZipFileObject::Args.
    * @param  pHandle
    *         A pointer to the handle to be freed.
    */
   void releaseResource(const Args &args, unzFile* pHandle) const
   {
      if (pHandle != NULL)
      {
         unzClose(*pHandle);
      }
   }
};

/**
 *  This is a %Resource class that opens and closes Zip files. 
 *
 *  This is a %Resource class that opens and closes Zip files. It has a conversion
 *  operator to allow a %ZipFileResource object to be used where ever a unzFile
 *  that represents an open Zip File Handle may be used.
 */
class ZipFileResource : public Resource<unzFile, ZipFileObject>
{
public:
   /**
    *  Constructs a Resource object that wraps a unzFile Zip File Handle.
    *
    *  Opens the specified file using unzOpen.
    *
    *  @param   filename
    *           The name of the file to open.
    */
   ZipFileResource(const std::string& filename) :
      Resource<unzFile, ZipFileObject>(Args(filename))
   {
   }

   /**
    * Constructs a Resource object that wraps a unzFile Zip File Handle.
    *
    * This will take ownership of an existing unzFile file handle and 
    * will ensure that it is closed.
    *
    * @param    file
    *           The zip file handle.
    */
   ZipFileResource(unzFile file):
      Resource<unzFile, ZipFileObject>(new unzFile(file), Args())
   {
   }

   /**
    * Default constructor.
    */
   ZipFileResource() :
      Resource<unzFile, ZipFileObject>(NULL, Args())
   {
   }

   /**
    *  Returns a pointer to the underlying unzFile returned by unzOpen.
    *
    *  Returns a pointer to the underlying unzFile. This operator,
    *  used in conjunction with the dereferencing operator,
    *  allows the %ZipFileResource object to be used where ever
    *  a unzFile would normally be used.
    *
    *  @return   A pointer to the underlying unzFile returned by unzOpen.
    */
   operator unzFile*() { return get(); }
};

#endif
