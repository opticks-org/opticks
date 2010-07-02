/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef FILERESOURCE_H
#define FILERESOURCE_H

#include "AppConfig.h"
#include "Resource.h"
#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <vector>

#if defined(WIN_API)
#include <io.h>
#include <fcntl.h>
#include <limits>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#define O_BINARY 0x0000 // make UNIX stop complaining when given O_BINARY!
#include <fcntl.h>
#include <sys/fcntl.h>
#include <sys/statvfs.h>
#include <unistd.h>
#endif

/**
 * The %FileObject is a trait object for use with the %Resource template.
 *
 * It provides capability for opening and closing files.
 *
 * @see        FileResource
 */
class FileObject
{
public:
   /**
    * This is an implementation detail of the %FileObject class.
    *
    * It is used for passing the parameters required by fopen().
    */
   class Args
   {
   public:
      Args(std::string filename, std::string access = "r", bool deleteOnClose = false) :
         mFilename(filename),
         mAccess(access),
         mDeleteOnClose(deleteOnClose)
      {}

      std::string mFilename;
      std::string mAccess;
      bool mDeleteOnClose;
   };

   FILE* obtainResource(const Args& args) const
   {
      FILE* pFile = fopen(args.mFilename.c_str(), args.mAccess.c_str()); // standard open
      if (args.mAccess.find("r") >= 0 && pFile == NULL)
      { // opening for reads - original failed - check uppercase/lowercase
         std::string fname = args.mFilename.c_str();
         for (size_t i = 0; i < fname.size(); ++i)
         {
            fname[i] = (char) toupper(fname[i]);
         }
         pFile = fopen(fname.c_str(), args.mAccess.c_str());
         if (pFile == NULL)
         { // try lowercase
            for (size_t i = 0; i < fname.size(); ++i)
            {
               fname[i] = (char) tolower(fname[i]);
            }
            pFile = fopen(fname.c_str(), args.mAccess.c_str());
         }
      }
      return pFile;
   }
   void releaseResource(const Args& args, FILE* pStream) const
   {
      if (pStream != NULL)
      {
         fclose(pStream);
      }

      if ((args.mDeleteOnClose) && (args.mFilename.empty() == false))
      {
         remove(args.mFilename.c_str());
      }
   }
};

/**
 * This is a %Resource class that opens and closes files and optionally deletes
 * them after close.
 *
 * This class has a conversion operator to allow a %FileResource object to be
 * used wherever a FILE* would normally be used.
*/
class FileResource : public Resource<FILE, FileObject>
{
public:
   /**
    * Constructs a Resource object that wraps a FILE*.
    *
    * Opens the specified file using the specified access modes.
    *
    * @param   pFilename
    *          The name of the file to open.
    * @param   pAccess
    *          The access mode to open the file with.  These match the modes
    *          used with fopen().
    * @param   deleteOnClose
    *          If \c true, the file will be deleted from the file system after
    *          it is closed.  If \c false, the file will simply be closed.
    */
   FileResource(const char* pFilename, const char* pAccess, bool deleteOnClose = false) :
      Resource<FILE, FileObject>(FileObject::Args(pFilename, pAccess, deleteOnClose))
   {}

   /**
    * Returns a pointer to the underlying FILE.
    *
    * This operator allows the %FileResource object to be used wherever a
    * FILE* would normally be used.
    *
    * @return  A pointer to the underlying FILE.
    */
   operator FILE*()
   {
      return get();
   }

   /**
    * Returns whether the file will be deleted after it is closed.
    *
    * @return  Returns \c true if the file will be deleted after it is closed,
    *          or \c false if the file will simply be closed.
    */
   bool getDeleteOnClose() const
   {
      const FileObject::Args& args = getArgs();
      return args.mDeleteOnClose;
   }

   /**
    * Sets whether the file will be deleted after it is closed.
    *
    * @param   deleteOnClose
    *          If set to \c true, the file will be deleted after it is closed.
    *          If set to \c false, the file will simply close without being
    *          deleted.
    */
   void setDeleteOnClose(bool deleteOnClose)
   {
      FileObject::Args& args = getArgs();
      args.mDeleteOnClose = deleteOnClose;
   }
};

class LargeFileResource
{
public:
   /**
    * Construct a object to represent a large file
    * regardless of platform.
    *
    * @param bResource
    *        if true, this object will own any opened file and
    *        will ensure it is closed when this object is destroyed.
    *        If false, the caller is responsible for closing any opened
    *        file.
    */
   LargeFileResource(bool bResource = true)
   {
      mbOwned = bResource;
      mHandle = -1;
   }

   /**
    * Destructs this object and will close any open file if
    * this object is a resource.
    */
   ~LargeFileResource()
   {
      if (mbOwned)
      {
         LargeFileResource::close();
      }
   }

   /**
    *  Copy-constructs this instance from an existing object. The new
    *  instance takes ownership of the file from the existing object.
    *
    *  @param   source
    *           The object to construct from. After this call, the source no
    *           longer owns the file.
    */
   LargeFileResource(LargeFileResource& source)
   {
      mHandle = source.mHandle;
      mbOwned = source.mbOwned;

      source.release(); // the source no longer owns the file
   }

   /**
    *  Sets this object to another one. The object assigned to
    *  takes ownership of the file from the source object.
    *
    *  @param   source
    *           The object to assign from.
    *           After this call, the source no longer owns the file.
    */
   LargeFileResource& operator=(LargeFileResource& source)
   {
      if (mbOwned) // a file is owned and is being overwritten, so we need some cleanup!
      {
         LargeFileResource::close();
      }
      mHandle = source.mHandle;
      mbOwned = source.mbOwned;

      source.release(); // the source no longer owns the file
      return *this;
   }

   /**
    * Opens the given filename with the given settings.
    *
    * @param filename
    *        The file to be opened.
    * @param openType
    *        Please see the documentation on _open or open64 for details.
    * @param permissionFlag
    *        Please see the documentation on _open or open64 for details.
    *
    * @return true if the file could be opened with the requested settings,
    *         false otherwise.
    */
   bool open(std::string filename, int openType, int permissionFlag)
   {
      if (mbOwned)
      {
         //only close any file we have open if we own it, otherwise let it leak.
         LargeFileResource::close();
      }

#if defined(WIN_API)
      mHandle = _open(filename.c_str(), openType, permissionFlag );
#else
      mHandle = open64(filename.c_str(), openType, permissionFlag | O_LARGEFILE );
#endif

      return validHandle();
   }

   /**
    * Reserve a file of the requested size with the requested name.
    * This file will be open after being reserved and will close any
    * open file that is owned by this object.
    *
    * The file will be reserved using the following open and permission flags:
    *   - O_RDWR | O_CREAT | O_BINARY
    *   - S_IREAD | S_IWRITE | S_IEXEC
    *
    * @param filename
    *        The name of the file to reserve.
    * @param reserveSize
    *        The size of the file in bytes to reserve.
    *
    * @return \c True if the file was successfully reserved, \c false otherwise.
    */
   bool reserve(std::string filename, int64_t reserveSize)
   {
      if (!open(filename, O_RDWR | O_CREAT | O_BINARY, S_IREAD | S_IWRITE | S_IEXEC))
      {
         return false;
      }

#if defined(UNIX_API)
      struct statvfs sbuf;
      if (statvfs(filename.c_str(), &sbuf) ==0)
      {
         // Be sure there is enough space in the file system
         if ((reserveSize + sbuf.f_bsize - 1) / sbuf.f_bsize < sbuf.f_bfree)
         {
            // seek here
            if (seek(reserveSize - 1, SEEK_SET) != (reserveSize - 1) || // seek failed, OR
                write(filename.c_str(), 1) != 1)  // write failed
            {
               return false;
            }
         }
      }
#elif defined(WIN_API)
      if (seek(reserveSize, SEEK_SET) != (reserveSize))
      {
         return false;
      }
      HANDLE winHandle = reinterpret_cast<HANDLE>(_get_osfhandle(mHandle));
      if (!::SetEndOfFile(winHandle))
      {
         return false;
      }
#endif
      return true;
   }

   /**
    * Close the opened file, regardless of file ownership.
    *
    * @return See the documentation on _close or ::close
    */
   int close()
   {
      int retVal = -1;
      if (validHandle())
      {
#if defined(WIN_API)
         retVal = _close(mHandle);
#else
         retVal = ::close(mHandle);
#endif
         mHandle = -1;
      }

      return retVal;
   }

   /**
    * Release ownership of the file.  This means any
    * opened file will not be closed during destruction of this object.
    */
   void release()
   {
      mbOwned = false;
   }

   /**
    * Take ownership of the file.  This means any opened file
    * will be closed during destruction of this object.
    */
   void lock()
   {
      mbOwned = true;  // grab ownership
   }

   /**
    * Create a file with the given name and permissions.
    * This file will be opened after creation and close any
    * open file that is owned by this object.
    *
    * @param filename
    *        The name of the file to create.
    * @param perm
    *        See the documentation on _creat or ::creat
    *
    * @return See the documentation on _creat or ::creat
    */
   int creat(std::string filename, int perm)
   {
      if (mbOwned)
      {
         //only close any file we have open if we own it, otherwise leak it.
         LargeFileResource::close();
      }
#if defined(WIN_API)
      mHandle = _creat(filename.c_str(), perm);
#else
      mHandle = ::creat(filename.c_str(), perm);
#endif

      return mHandle;
   }

   /**
    * Determine if the current position is the end of the file.
    *
    * @return 1 if the position is the end of file, 0 if it is not, -1 on error.
    */
   int eof()
   {
      if (!validHandle())
      {
         return -1;
      }
#if defined(WIN_API)
      return _eof(mHandle);
#else
      return (tell() >= fileLength() ? 1 : 0);
#endif
   }

   /**
    * Determine the length of the opened file in bytes.
    *
    * @return The length of the opened file in bytes,
    *         of -1 if no file is opened.
    */
   int64_t fileLength()
   {
      if (!validHandle())
      {
         return -1;
      }
#if defined(WIN_API)
      return _filelengthi64(mHandle);
#else
      int64_t curPos = tell();
      seek(0, SEEK_END);
      int64_t fileSize = tell();
      seek(curPos, SEEK_SET);
      return fileSize;
#endif
   }

   /**
    * Move the current position in the file the given offset in the
    * given direction for the opened file.
    *
    * @param offset
    *        The number of bytes to move the current position
    *        relative to the origin.
    * @param direction
    *        What the offset is relative to, can be one of the following:
    *        SEEK_SET - beginning of file.
    *        SEEK_CUR - current position in the file.
    *        SEEK_END - end of the file. (all three are defined in stdio.h)
    *
    * @return The new position in the file, or -1 if an error occurred.
    */
   int64_t seek(int64_t offset, int direction)
   {
      if (!validHandle())
      {
         return -1;
      }
#if defined(WIN_API)
      return _lseeki64(mHandle, offset, direction);
#else
      return lseek64(mHandle, offset, direction);
#endif
   }

   /**
    * Reads the given number of bytes from the opened file into the buffer
    * pMem.  The current position in the file is updated.
    *
    * @param pMem
    *        Pointer to the buffer to put the contents of what is read
    *        from the file.  The buffer must be created at least as
    *        large as bytesToRead.
    * @param bytesToRead
    *        The number of bytes to read from the file.
    *
    * @return The number of bytes actually read from the file.  May be less
    *         than bytesToRead if end of file was encountered during read, or
    *         if file was opened in text mode and newline conversions were
    *         performed, -1 on error.
    */
   int64_t read(void* pMem, int64_t bytesToRead)
   {
      if ((pMem == NULL) || (bytesToRead == 0) || !validHandle())
      {
         return -1;
      }
#if defined(WIN_API)
      int64_t bytesRead = 0;
      while (bytesToRead >= std::numeric_limits<unsigned int>::max())
      {
         int64_t r = _read(mHandle, pMem, std::numeric_limits<unsigned int>::max()) ;
         bytesRead += r;
         if (r < std::numeric_limits<unsigned int>::max())
         {
            return bytesRead;
         }
         pMem = reinterpret_cast<char*>(pMem) + std::numeric_limits<unsigned int>::max();
         bytesToRead -= std::numeric_limits<unsigned int>::max();
      }
      bytesRead += _read(mHandle, pMem, static_cast<unsigned int>(bytesToRead));
      return bytesRead;
#else
      return ::read(mHandle, pMem, bytesToRead);
#endif
   }

   /**
    * Returns the current position in the opened file.
    *
    * @return Returns the current position in the opened file, -1 on error.
    */
   int64_t tell()
   {
      if (!validHandle())
      {
         return -1;
      }
#if defined(WIN_API)
      return _telli64(mHandle);
#else
      return lseek64(mHandle, 0, SEEK_CUR);
#endif
   }

   /**
    * Writes the given number of bytes from pMem to the opened file.
    * The curent position in the file is updated.
    *
    * @param pMem
    *        Pointer to the data to be written to the file.  The buffer
    *        must be at least bytesToWrite large.
    * @param bytesToWrite
    *        The number of bytes to copy from pMem into the file.
    *
    * @return the number of bytes actually written to the file, -1 on error.
    */
   int64_t write(const void* pMem, int64_t bytesToWrite)
   {
      if ((pMem == NULL) || (bytesToWrite == 0) || !validHandle())
      {
         return -1;
      }
#if defined(WIN_API)
      int64_t bytesWritten = 0;
      while (bytesToWrite >= std::numeric_limits<unsigned int>::max())
      {
         int64_t wrote = _write(mHandle, pMem, std::numeric_limits<unsigned int>::max()) ;
         bytesWritten += wrote;
         if (wrote < std::numeric_limits<unsigned int>::max())
         {
            return bytesWritten;
         }
         pMem = reinterpret_cast<const char*>(pMem) + std::numeric_limits<unsigned int>::max();
         bytesToWrite -= std::numeric_limits<unsigned int>::max();
      }
      bytesWritten += _write(mHandle, pMem, static_cast<unsigned int>(bytesToWrite));
      return bytesWritten;
#else
      return ::write(mHandle, pMem, bytesToWrite);
#endif
   }

   /**
    * Returns true if this object has a file open.
    *
    * @return True if this object has a file open, false otherwise.
    */
   bool validHandle()
   {
      return (mHandle >= 0);
   }

   /**
    * Read in a line from the file.
    *
    * This is not a buffered read and may be inefficient.
    * This function will not successfully read lines greater than 4k characters in length.
    *
    * @param pError
    *        If NULL, this is ignored. If non-NULL, this will contain false if the
    *        read was successful, true if there was an error.
    *
    * @return The line read in with the newline character stripped.
    */
   std::string readLine(bool* pError = NULL)
   {
      std::string line;
      int64_t pos = tell();
      std::vector<char> buffer(4096, 0);
      int64_t bytesRead = read(&buffer.front(), buffer.size());
      if (bytesRead <= 0)
      {
         if (pError != NULL)
         {
            *pError = true;
         }
         return line;
      }
      std::string tmp(&buffer.front(), static_cast<std::string::size_type>(bytesRead));
      std::string::size_type eolLoc = tmp.find('\n');
      if (eolLoc == std::string::npos)
      {
         if (pError != NULL)
         {
            *pError = false;
         }
         return tmp;
      }
      seek(pos + eolLoc + 1, SEEK_SET);
      if (eolLoc != 0 && tmp[eolLoc-1] == '\r') // DOS-style EOL & non DOS-text I/O
      {
         --eolLoc;
      }
      line = tmp.substr(0, eolLoc);
      if (pError != NULL)
      {
         *pError = false;
      }
      return line;
   }

private:
   int mHandle;
   bool mbOwned;
};

#endif
