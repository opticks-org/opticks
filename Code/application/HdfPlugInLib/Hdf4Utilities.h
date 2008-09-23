/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef HDF4_UTILITIES_H
#define HDF4_UTILITIES_H

#include "AppConfig.h"
#if defined(HDF4_SUPPORT)

#include <hdf.h>
#include <mfhdf.h>
#include "HdfUtilities.h"

#include "Hdf4Element.h"
#include "DataVariant.h"
#include "Resource.h"

#include <string>

namespace HdfUtilities
{
   /**
    * Reads an attribute from an Hdf4Element based on the attribute name.
    *
    * Example usage:
    * @code
    * // assume pElement is a pDataset is a valid, non-NULL Hdf4Dataset
    * unsigned short badValue;
    * if (HdfUtilities::getAttribute(*pDataset, "_FillValue", badValue) == false)
    * {
    *    // error condition
    * }
    * // use _FillValue here
    * @endcode
    *
    * @param  element
    *         The Hdf4Element from which to read the attribute.
    * @param  attributeName
    *         The name of the attribute to read in.
    * @param  actualValue
    *         Contains the actual value contained in the attribute.
    *
    * @return TRUE if the attribute was found and contained valid data, FALSE otherwise.
    */
   template<typename T>
   bool getAttribute(const Hdf4Element& element, const std::string& attributeName, T& actualValue)
   {
      Hdf4Attribute* pAttr = element.getAttribute(attributeName);
      if (pAttr != NULL)
      {
         return pAttr->getVariant().getValue(actualValue);
      }
      return false;
   }

   /**
    *  Converts an HDF4 type to a string that can be used to create an Hdf4Attribute.
    *
    *  @param  type
    *          The HDF4 type (ie. DFNT_CHAR).
    *  @param  count
    *          The number of elements whose data type is 'type'.
    * 
    *  @return A string that represents the HDF5 type in the same way
    *          that DynamicObect::get() expresses types as strings.
    */
   std::string hdf4TypeToString(long type, size_t count);


   /**
    *  Given an open HDF4 Attribute identifier retrieves
    *  the attribute and places it in a variant.
    *
    *  @param  obj_id
    *          The object identifier that the attribute should be read from.
    *  @param  attr_index
    *          The index of the attribute to read.
    *  @param  var
    *          The variant that the attribute data will be placed in.
    *
    *  @return True if the attribute could be read, false otherwise.
    */
   bool readHdf4Attribute(int32 obj_id, int32 attr_index, DataVariant& var);

   /**
    *  The %Hdf4FileObject is a trait object for use with the %Resource template. 
    *
    *  It provides capability for opening and closing HDF4 files.
    * 
    * @see FileResource
    */
   class Hdf4FileObject
   {
   public:
      /**
       * This is an implementation detail of the %Hdf4FileObject class. 
       *
       * It is used for passing the parameters required by SDstart.
       */
      struct Args
      {
         /**
          * The file name of the file that is opened.
          */
         std::string mFilename;

         /**
          * How the file is accessed (read, write, append, truncate, etc.).
          *
          * For details, see the HDF documentation.
          */
         int32 mAccess;

         /**
          * Constructs an Args object for an HDF4 File Object.
          *
          * See Resource.h for details.
          *
          * @param  filename
          *         The file that is opened.
          * @param  access
          *         How the file will be opened (ie. DFACC_READ, DFACC_WRITE, etc.). See the HDF
          *         documentation for details.
          */
         Args(std::string filename, int32 access = DFACC_READ) : mFilename(filename), mAccess(access) {}
      };

      /**
       * Obtains an HDF4 file handle.
       *
       * See Resource.h for details.
       *
       * @param  args
       *         The arguments for obtaining the resource. Should be of type Hdf4FileObject::Args.
       * @return Returns a pointer to a handle of the dataset.
       */
      int32* obtainResource(const Args &args) const;

      /**
       * Releases an HDF4 file handle.
       *
       * See Resource.h for details.
       *
       * @param  args
       *         The arguments for releasing the resource. Should be of type Hdf4FileObject::Args.
       * @param  pHandle
       *         A pointer to the handle to be freed.
       */
      void releaseResource(const Args &args, int32* pHandle) const;
   };

   /**
    *  This is a %Resource class that opens and closes HDF4 files.
    *
    *  It has a conversion operator to allow an %Hdf4FileResource object to be used
    *  where ever a int32* file handle would normally be used.
    *
    *  see Hdf4DatasetResource for code examples.
    */
   class Hdf4FileResource : public Resource<int32, Hdf4FileObject>
   {
   public:
      /**
       *  Creates a resource to an HDF4 file handle.
       *
       *  Auto-closes file when the object goes out of scope.
       *  
       *  @param  pFilename
       *          The name of the file to open.
       *  @param  access
       *          Optional argument for how to access the file
       *          (DFACC_READ, DFACC_WRITE, DFACC_CREATE, DFACC_ALL).
       */
      Hdf4FileResource(const char *pFilename, int32 access = DFACC_READ) :
         Resource<int32, Hdf4FileObject>(Hdf4FileObject::Args(pFilename, access))
      {
      }

      /**
       * Allows for implict conversion of this resource type to an int32*.
       *
       * @return A pointer to an int32 reprsenting the file handle.
       */
      operator int32*() { return get(); }
   };

   /**
    * This is an implementation detail of the %Hdf4DatasetObject class. 
    *
    * It is used for passing the parameters required by SDselect.
    */
   class Hdf4DatasetObject
   {
   public:
      /**
       * This is an implementation detail of the %Hdf4FileObject class. 
       *
       * It is used for passing the parameters required by SDstart.
       */
      struct Args
      {
         /**
          * The file handle that represents the file that contains
          * the dataset.
          */
         int32 mFileHandle;

         /**
          * The index of the dataset.
          *
          * There are two constructors provided, one that takes the name
          * of the dataset, one that takes the index. This value is
          * then initialized appropriately by the constructor.
          */
         int mIndex;

         /**
          * Constructs an Args object for an HDF4 Dataset Object.
          *
          * See Resource.h for details.
          *
          * @param  fileHandle
          *         A file handle repsesenting the file that contains the specified
          *         dataset.
          * @param  dsIndex
          *         The index of the dataset to access.
          */
         Args(int32 fileHandle, int dsIndex) : mFileHandle(fileHandle), mIndex(dsIndex)
         {
         }

         /**
          * Constructs an Args object for an HDF4 Dataset Object.
          *
          * See Resource.h for details.
          *
          * This method is provided for convenience, behaving the
          * same as the constructor above.
          *
          * @param  fileHandle
          *         A file handle repsesenting the file that contains the specified
          *         dataset.
          * @param  datasetName
          *         The name of the dataset to access.
          */
         Args(int32 fileHandle, const std::string& datasetName) : mFileHandle(fileHandle)
         {
            int dsIndex = SDnametoindex(mFileHandle, const_cast<char*>(datasetName.c_str()));
            mIndex = dsIndex;
         }
      };

      /**
       * Obtains an HDF4 dataset handle.
       *
       * See Resource.h for details.
       *
       * @param  args
       *         The arguments for obtaining the resource. Should be of type Hdf4DatasetObject::Args.
       * @return Returns a pointer to a handle of the dataset.
       */
      int32* obtainResource(const Args &args) const;

      /**
       * Releases an HDF4 dataset handle.
       *
       * See Resource.h for details.
       *
       * @param  args
       *         The arguments for releasing the resource. Should be of type Hdf4DatasetObject::Args.
       * @param  pHandle
       *         A pointer to the handle to be freed.
       */
      void releaseResource(const Args &args, int32* pHandle) const;
   };

   /**
    *  This is a %Resource class that opens and closes HDF4 datasets.
    *
    *  It has a conversion operator to allow an %Hdf4DatasetResource object to be used
    *  where ever a int32* data handle would normally be used.
    *
    *  @code
    *  bool readData(const string& filename, int index)
    *  {
    *     Hdf4FileResource file("c:/data/a.hdf");
    *     if (file.get() != NULL && *file != FAIL)
    *     {
    *        Hdf4DatasetResource dataset(*file, index);
    *        if (dataset.get() != NULL && *dataset != FAIL)
    *        {
    *           // do SDreaddata here
    *        }
    *     }
    *  }
    *  @endcode
    */
   class Hdf4DatasetResource : public Resource<int32, Hdf4DatasetObject>
   {
   public:
      /**
       *  Creates a resource to an Hdf4 dataset handle.
       *
       *  Auto-closes data handle when the object goes out of scope.
       *  
       *  @param  fileHandle
       *          A handle to the file that contains the dataset.
       *  @param  datasetName
       *          The name of the dataset to open. This is typically the easiest
       *          way to get a data handle since it takes the result from Hdf4Dataset::getName().
       */
      Hdf4DatasetResource(int32 fileHandle, const std::string& datasetName) :
         Resource<int32, Hdf4DatasetObject>(Hdf4DatasetObject::Args(fileHandle, datasetName))
      {
      }

      /**
       *  Creates a resource to an Hdf4 dataset handle.
       *
       *  Auto-closes data handle when the object goes out of scope.
       *  
       *  @param  fileHandle
       *          A handle to the file that contains the dataset.
       *  @param  dsIndex
       *          The index, if known, of the dataset.
       */
      Hdf4DatasetResource(int32 fileHandle, int dsIndex) :
         Resource<int32, Hdf4DatasetObject>(Hdf4DatasetObject::Args(fileHandle, dsIndex))
      {
      }

      /**
       * Allows for implict conversion of this resource type to an int32*.
       *
       * @return A pointer to an int32 reprsenting the file handle.
       */
      operator int32*() { return get(); }
   };
}

#endif

#endif
