/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef HDF4FILE_H
#define HDF4FILE_H

#include <string>
#include <vector>

#include "Hdf4Element.h"

class Hdf4Dataset;
class Hdf4Group;

/**
 * The HDF File is provided as an abstraction of the structure of the HDF File
 * on disk without requiring the HDF libraries installed.
 */
class Hdf4File : public Hdf4Element
{
public:
   /**
    * Creates an Hdf4File with a given filename.
    *
    * @param  name
    *         The name of the HDF file on disk.
    */
   explicit Hdf4File(const std::string& name);

   /**
    * Destroys the Hdf4File object.
    */
   ~Hdf4File();

   /**
    * Returns a pointer to the file's 'root group' object.
    *
    * @return A pointer to the %Hdf4File's root group object. Cannot be NULL.
    */
   virtual Hdf4Group* getRootGroup() const;

private:
   const Hdf4Dataset* mpActiveDataset;
   Hdf4Group* mpRootGroup;
};

#endif
