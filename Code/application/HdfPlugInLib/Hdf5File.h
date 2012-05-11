/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef HDF5FILE_H
#define HDF5FILE_H

#include <hdf5.h>
#include <string>
#include <vector>

#include "Hdf5Node.h"

class Hdf5Group;

/**
 * The HDF File is provided as an abstraction of the structure of the HDF File
 * on disk without requiring the HDF libraries installed.
 */
class Hdf5File : public Hdf5Node
{
public:
   /**
    * Creates an Hdf5File with a given filename.
    *
    * @param  filename
    *         The name of the HDF file on disk.
    *
    * @param  fileHandle
    *         Handle to a previously open file, or -1 if the file is currently unavailable.
    *         This object does NOT assume ownership of fileHandle.
    */
   explicit Hdf5File(const std::string& filename, hid_t fileHandle = -1);
   
   /**
    * Destroys the Hdf5File object.
    */
   ~Hdf5File();

   /**
    * Returns a pointer to the file's 'root group' object. The root group will be empty until readFileData() is called.
    *
    * @return A pointer to the %Hdf5File's root group object. Cannot be NULL.
    *
    * @see readFileData()
    */
   virtual Hdf5Group* getRootGroup() const;

   /**
    * Returns the name of the file.
    *
    * @return The filename provided to the constructor.
    */
   virtual std::string getFilename() const;

   /**
    * Returns the file handle.
    *
    * @return The fileHandle provided to the constructor.
    */
   virtual hid_t getFileHandle() const;

   /**
    * Reads the HDF file at the specified path, opening and closing it if necessary.
    *
    * This method reads the data from the file. If the fileHandle passed to the constructor was valid,
    * then that handle will be used to access the file. If the fileHandle passed to the constructor was not valid,
    * then the file will be opened, read, and closed before returning from this method.
    *
    * @param  groupPath
    *         The desired HDF group to read. This group will be iterated over via H5Giterate.
    *         The results are stored in the 'root group' object with paths relative to the root of the HDF file.
    *
    * @return True on success, false otherwise.
    *
    * @see getRootGroup(), getFileHandle(), getFilename()
    */
   bool readFileData(const std::string& groupPath = "/");

   // Override of Hdf5Node function
   virtual Hdf5File* getFile() const;

private:
   hid_t mFileHandle;
   std::string mFilename;
   Hdf5Group* mpRootGroup;
};

#endif
