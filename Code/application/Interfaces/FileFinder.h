/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef __FILEFNDR_H
#define __FILEFNDR_H

#include "DateTime.h"

#include <string>

/**
 *  Provides capability for file system searches
 *
 *  A platform-independent tool for searching the file system and
 *  querying file attributes.
 *
 *  @see        Filename
 */
class FileFinder
{
public:
   /**
    *  Determine if any files meet client's criteria.
    *
    *  @param   path
    *           The directory to search.
    *
    *  @param   criteria
    *           Search criteria. This is operating system dependent but
    *           generally contains either a filename, a glob, or is empty.
    *           An empty string will search for all files.
    *
    *  @param   includeDirectories
    *           Should subdirectories be includes in the results? If false, only files are included.
    *
    *  @return  True if any files were identified as meeting the 
    *           criteria.
    */
   virtual bool findFile(const std::string& path, const std::string& criteria, bool includeDirectories = false) = 0;

   /**
    *  Designate first/next entry in list of qualified files.
    *
    *  @return  True if another file has qualified and can be
    *           interrogated for status by the application.
    */
   virtual bool findNextFile() = 0;

   /**
    *  Get the length of the current qualified file.
    *
    *  @return  The length, as determined by the operating system,
    *           of the current file.
    */
   virtual double getLength() const = 0;

   /**
    *  Get the filename, without any path information, of the 
    *  current qualified file.
    *
    *  @return  The name of the current file.
    */
   virtual std::string getFileName() const = 0;

   /**
    *  Get the path information of the 
    *  current qualified file.
    *
    *  @return  The path of the current file.
    */
   virtual std::string getFilePath() const = 0;

   /**
    *  Get the filename, without any path or extension information,
    *  of the current qualified file.
    *
    *  @param   fileTitle
    *           The name of the current file.
    *
    *  @return  True if the current file title was successfully set,
    *           otherwise false.
    */
   virtual bool getFileTitle(std::string& fileTitle) const = 0;

   /**
    *  Get the filename and path information, of the 
    *  current qualified file.
    *
    *  @param   fullFilePath
    *           The full path of the current file.
    *
    *  @return  True if the full path of the current file was
    *           successfully set, otherwise false.
    */
   virtual bool getFullPath(std::string& fullFilePath) const = 0;

   /**
    *  Get the last access time of the current qualified file.
    *
    *  @param   dt
    *           The date/time of last access, as determined by the
    *           operating system, of the current file.
    */
   virtual void getLastAccessTime(DateTime& dt) const = 0;

   /**
    *  Get the last modification time of the current qualified file.
    *
    *  @param   dt
    *           The date/time of last change, as determined by the
    *           operating system, of the current file.
    */
   virtual void getLastModificationTime(DateTime& dt) const = 0;

   /**
    *  Determine if the entry is either the current or parent directory
    *
    *  @return  True if the current file is either "." or ".."
    */
   virtual bool isDots() const = 0;

   /**
    *  Determine if the entry is a directory
    *
    *  @return  True if the current file is a directory.
    */
   virtual bool isDirectory() const = 0;

   /**
    *  Determine if the current entry is read-only.
    *
    *  @return  True if the current file is read-only for the current user.
    */
   virtual bool isReadOnly() const = 0;

protected:
   /**
    * This should be destroyed by calling ObjectFactory::destroyObject.
    */
   virtual ~FileFinder() {}
};

#endif   // __FILEFNDR_H
