/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef HDF5RESOURCE_H
#define HDF5RESOURCE_H

#include "AppVerify.h"
#include "Resource.h"
#include <hdf5.h>

#include <string>

/**
 * The %Hdf5FileObject is a trait object for use with the %Resource template. 
 *
 * The %Hdf5FileObject is a trait object for use with the %Resource template. It provides capability for opening
 * and closing HDF5 files.
 * 
 * @see FileObject
 */
class Hdf5FileObject
{
public:
   /**
    * This is an implementation detail of the %Hdf5FileObject class. 
    *
    * It is used for passing the parameters required by H5Fopen.
    */
   struct Args
   {
      /**
       * The file name of the HDF5 file to be opened.
       */
      std::string mFilename;

      /**
       * The access flags for opening the file.
       *
       * Default is to open read only. Other options include read only,
       * read-write, write, truncate. Options may be combined with the
       * | operator. For details, see the HDF5 documentation.
       */
      unsigned int mFlags;

      /**
       * The internal HDF5 access flags.
       *
       * These flags are different than the file-level access flags. They
       * specify what filters are needed to access the data. This should
       * almost always stay at the default, H5P_DEFAULT. For details regarding
       * this parameter, see the H5Fopen function in the HDF5 documentation.
       */
      hid_t mAccess;

      /**
       *  Creates a resource to an HDF5 file handle.
       *
       *  Auto-closes file when the object goes out of scope.
       *  
       *  @param  filename
       *          The name of the file to open.
       *  @param  flags
       *          The open flags for the file access (H5F_ACC_RDONLY, H5F_ACC_RDWR,
       *          H5F_ACC_TRUNC, H5F_ACC_EXCL, H5F_ACC_DEBUG, H5F_ACC_CREAT). These
       *          flags may be ORed together with the operator |. For details
       *          on these flags, see the HDF5 documentation.
       *  @param  access
       *          The internal HDF5 access flags.  These flags are different than
       *          the file-level access flags.  They specify what filters are
       *          needed to access the data.  This should almost always stay at
       *          the default, H5P_DEFAULT.
       */
      Args(std::string filename, unsigned flags = H5F_ACC_RDONLY, hid_t access = H5P_DEFAULT) :
         mFilename(filename), mFlags(flags), mAccess(access)
         {
         }

      /**
       * Default constructor
       */
      Args() : mFilename(), mFlags(0), mAccess(-1)
      {
      }
   };

   /**
    * Obtains an HDF5 file handle.
    *
    * See Resource.h for details.
    *
    * @param  args
    *         The arguments for obtaining the resource. Should be of type Hdf5FileObject::Args.
    * @return Returns a pointer to a handle of the dataset.
    */
   hid_t* obtainResource(const Args &args) const
   {
      int iValid = H5Fis_hdf5(args.mFilename.c_str());
      hid_t* pId = new hid_t;
      VERIFYRV(pId != NULL, NULL);
      *pId = -1; // invalidate the handle
      if (iValid > 0 && pId != NULL)
      {
         *pId = H5Fopen(args.mFilename.c_str(), args.mFlags, args.mAccess);
      }
      return pId;
   }

   /**
    * Releases an HDF5 file handle.
    *
    * See Resource.h for details.
    *
    * @param  args
    *         The arguments for releasing the resource. Should be of type Hdf5FileObject::Args.
    * @param  pHandle
    *         A pointer to the handle to be freed.
    */
   void releaseResource(const Args &args, hid_t* pHandle) const
   {
      if (pHandle != NULL && *pHandle != -1)
      {
         H5Fclose(*pHandle);
         delete pHandle;
      }
   }
};

/**
 *  This is a %Resource class that opens and closes HDF5 files. 
 *
 *  This is a %Resource class that opens and closes HDF5 files. It has a conversion
 *  operator to allow a %Hdf5FileResource object to be used where ever a hid_t
 *  that represents an open HDF5 File Handle may be used.
 *
 *  Example of Usage:
 *  @code
 *  Hdf5FileResource pFile("c:/data/a.h5"); // open for read-only access
 *  Hdf5FileResource pFile2("c:/data/b.h5", H5F_ACC_CREAT | H5F_ACC_RDWR); // create and open for read/write
 *  Hdf5FileResource pFile3("c:/data/c.h5", H5F_ACC_RDWR); // open for read/write access
 *  @endcode
 */
class Hdf5FileResource : public Resource<hid_t, Hdf5FileObject>
{
public:
   /**
    *  Constructs a Resource object that wraps a hid_t HDF5 File Handle.
    *
    *  Opens the specified file using the specified access
    *  modes. The two arguments will ultimately be passed
    *  unmodified to H5Fopen.
    *
    *  @param   filename
    *           The name of the file to open.
    *  @param   flags
    *           The access mode to open the file with. These match
    *           the modes used with H5Fopen.
    *  @param   access
    *           How to open the file. Most users will never need to provide this option.
    *           See the HDF5 documentation for details.
    */
   Hdf5FileResource(const std::string& filename, unsigned flags = H5F_ACC_RDONLY, hid_t access = H5P_DEFAULT) :
      Resource<hid_t, Hdf5FileObject>(Args(filename, flags, access))
   {
   }

   /**
    * Constructs a Resource object that wraps a hid_t HDF5 File Handle.
    *
    * This will take ownership of an existing hid_t file handle and 
    * will ensure that it is closed.
    *
    * @param    file
    *           The HDF5 file handle.
    */
   Hdf5FileResource(hid_t file):
      Resource<hid_t, Hdf5FileObject>(new hid_t(file), Args())
   {
   }

   /**
    * Default constructor.
    */
   Hdf5FileResource() :
      Resource<hid_t, Hdf5FileObject>(NULL, Args())
   {
   }

   /**
    *  Returns a pointer to the underlying hid_t returned by H5Fopen.
    *
    *  Returns a pointer to the underlying hid_t. This operator,
    *  used in conjunction with the dereferencing operator,
    *  allows the %Hdf5FileResource object to be used where ever
    *  a hid_t would normally be used.
    *
    *  @return   A pointer to the underlying hid_t returned by H5Fopen.
    */
   operator hid_t*()
   {
      return get();
   }
};

/**
 *  The %Hdf5DatasetObject is a trait object for use with the %Resource template.
 *
 *  The %Hdf5DatasetObject is a trait object for use with the %Resource template. It provides capability for opening
 *  and closing HDF5 datasets.
 *
 *  @see Hdf5FileObject
 */
class Hdf5DataSetObject
{
public:
   /**
    * This is an implementation detail of the %Hdf5DataSetObject class. 
    *
    * It is used for passing the parameters required by H5Dopen1.
    */
   struct Args
   {
      /**
       * The file handle that represents the file that contains
       * the dataset.
       */
      hid_t mFileHandle;

      /**
       * The full path and name of the dataset within the file.
       *
       * HDF5 provides access to datasets either by requiring
       * opening of each group within the file (similar to
       * performing "cd docs; cd stuff; cd mydir;") and performing
       * a dataset open at the end, OR by providing the full path
       * and name of the dataset (ie. "/docs/stuff/mydir/dataset").
       */
      std::string mFullPathAndName;

      /**
       * Constructs an Args object for an HDF5 Dataset Object.
       *
       * See Resource.h for details.
       *
       * @param  fileHandle
       *         A file handle repsesenting the file that contains the specified
       *         dataset.
       * @param  fullPathAndName
       *         The UNIX-style full path and name of the data element to open.
       */
      Args(hid_t fileHandle, const std::string& fullPathAndName) :
         mFileHandle(fileHandle), mFullPathAndName(fullPathAndName)
      {
      }

      /**
       * Default constructor
       */
      Args() :
         mFileHandle(-1),
         mFullPathAndName()
      {
      }
   };

   /**
    * Obtains an HDF5 dataset handle.
    *
    * See Resource.h for details.
    *
    * @param  args
    *         The arguments for obtaining the resource. Should be of type Hdf5DatasetObject::Args.
    * @return Returns a pointer to a handle of the dataset.
    */
   hid_t* obtainResource(const Args &args) const
   {
      hid_t* pHandle = new (std::nothrow) hid_t;
      if (pHandle != NULL)
      {
         *pHandle = H5Dopen1(args.mFileHandle, args.mFullPathAndName.c_str());
      }
      return pHandle;
   }

   /**
    * Releases an HDF5 dataset handle.
    *
    * See Resource.h for details.
    *
    * @param  args
    *         The arguments for releasing the resource. Should be of type Hdf5DatasetObject::Args.
    * @param  pHandle
    *         A pointer to the handle to be freed.
    */
   void releaseResource(const Args &args, hid_t* pHandle) const
   {
      if (pHandle != NULL && *pHandle != -1)
      {
         H5Dclose(*pHandle);
         delete pHandle;
      }
   }
};

/**
 *  This is a %Resource class that opens and closes HDF5 datasets. 
 *
 *  This is a %Resource class that opens and closes HDF5 datasets. It has a conversion
 *  operator to allow a %Hdf5DatasetResource object to be used where ever a hid_t
 *  that represents an open HDF5 dataset Handle may be used.
 *
 *  Example of Usage:
 *  @code
 *  Hdf5FileResource pFile("c:/data/a.h5"); // open for read-only access
 *  if (pFile.get() != NULL && *pFile > 0) // file exists
 *  {
 *     Hdf5DatasetResource pDataset(*pFile, "/home/dataset/1");
 *     if (pDataset != NULL && *pDataset > 0)
 *     {
 *        // do H5Dread here
 *     }
 *  }
 *  @endcode
 */
class Hdf5DataSetResource : public Resource<hid_t, Hdf5DataSetObject>
{
public:
   /**
    *  Constructs a Resource object that wraps a hid_t HDF5 Dataset Handle.
    *
    *  Opens the specified dataset based on a file handle and the
    *  full path and name to the dataset within the file (ie. "/home/data/cube1").
    *  Calls H5Dopen1().
    *
    *  @param   fileHandle
    *           The HDF5 file handle. @see Hdf5FileResource::get().
    *  @param   datasetName
    *           The full path and name to the HDF5 dataset to open.
    *
    */
   Hdf5DataSetResource(hid_t fileHandle, const std::string& datasetName) :
      Resource<hid_t, Hdf5DataSetObject>(Args(fileHandle, datasetName))
   {
   }

   /**
    * Construct a Resource object that wraps a hid_t HDF5 Dataset Handle
    *
    * This will take ownership of an existing hid_t dataset handle and 
    * will ensure that it is closed.
    *
    * @param    dataset
    *           The HDF5 dataset handle.
    */
   Hdf5DataSetResource(hid_t dataset) :
      Resource<hid_t, Hdf5DataSetObject>(new hid_t(dataset), Args())
   {
   }

   /**
    * Default constructor.
    */
   Hdf5DataSetResource() :
      Resource<hid_t, Hdf5DataSetObject>(NULL, Args())
   {
   }

   /**
    *  Returns a pointer to the underlying hid_t returned by H5Dopen1.
    *
    *  Returns a pointer to the underlying hid_t. This operator,
    *  used in conjunction with the dereferencing operator,
    *  allows the %Hdf5DatasetResource object to be used where ever
    *  a hid_t would normally be used.
    *
    *  @return   A pointer to the underlying hid_t returned by H5Dopen1.
    */
   operator hid_t*()
   {
      return get();
   }
};

/**
 *  The Hdf5AttributeObject is a trait object for use with the %Resource template.
 *
 *  The Hdf5AttributeObject is a trait object for use with the %Resource template.
 *  It provides capability for closing HDF5 Attributes.
 *
 */
class Hdf5AttributeObject
{
public:
   /**
    * This is an implementation detail of the Hdf5AttributeObject class. 
    *
    */
   struct Args
   {
      /**
       * Default constructor
       */
      Args()
      {
      }
   };

   /**
    * Obtains an HDF5 attribute.
    *
    * See Resource.h for details.
    *
    * @param  args
    *         The arguments for obtaining the resource. Should be of type Hdf5AttributeObject::Args.
    *
    * @return Returns NULL
    */
   hid_t* obtainResource(const Args &args) const
   {
      return NULL;
   }

   /**
    * Releases an HDF5 attribute.
    *
    * See Resource.h for details.
    *
    * @param  args
    *         The arguments for releasing the resource. Should be of type Hdf5AttributeObject::Args.
    * @param  pHandle
    *         A pointer to the handle to be freed.
    */
   void releaseResource(const Args &args, hid_t* pHandle) const
   {
      if ((pHandle != NULL) && (*pHandle != -1))
      {
         H5Aclose(*pHandle);
         delete pHandle;
      }
   }
};

/**
 *  This is a Resource class that closes HDF5 attributes. 
 *
 *  This is a Resource class that closes HDF5 attributes. It has a conversion
 *  operator to allow a Hdf5AttributeResource object to be used where ever a hid_t
 *  that represents an open HDF5 attribute Handle may be used.
 *
 */
class Hdf5AttributeResource : public Resource<hid_t, Hdf5AttributeObject>
{
public:
   /**
    * Construct a Resource object that wraps a hid_t HDF5 Attribute Handle
    *
    * This will take ownership of an existing hid_t attribute handle and 
    * will ensure that it is closed.
    *
    * @param    attribute
    *           The HDF5 attribute handle.
    */
   Hdf5AttributeResource(hid_t attribute) :
      Resource<hid_t, Hdf5AttributeObject>(new hid_t(attribute), Args())
   {
   }

   /**
    * Default constructor.
    */
   Hdf5AttributeResource() :
      Resource<hid_t, Hdf5AttributeObject>(NULL, Args())
   {
   }

   /**
    *  Returns a pointer to the underlying hid_t held by this Resource.
    *
    *  Returns a pointer to the underlying hid_t. This operator,
    *  used in conjunction with the dereferencing operator,
    *  allows the Hdf5AttributeResource object to be used where ever
    *  a hid_t would normally be used.
    *
    *  @return   A pointer to the underlying hid_t held by this Resource.
    */
   operator hid_t*()
   {
      return get();
   }
};

/**
 *  The Hdf5TypeObject is a trait object for use with the Resource template.
 *
 *  The Hdf5TypeObject is a trait object for use with the Resource template.
 *  It provides capability for closing HDF5 types.
 *
 */
class Hdf5TypeObject
{
public:
   /**
    * This is an implementation detail of the Hdf5TypeObject class. 
    *
    */
   struct Args
   {
      /**
       * Default constructor
       */
      Args()
      {
      }
   };

   /**
    * Obtains an HDF5 type.
    *
    * See Resource.h for details.
    *
    * @param  args
    *         The arguments for obtaining the resource. Should be of type Hdf5TypeObject::Args.
    *
    * @return Returns NULL
    */
   hid_t* obtainResource(const Args &args) const
   {
      return NULL;
   }

   /**
    * Closes an open HDF5 type.  If the
    * given type is a compound dataset,
    * the close will be recursive on all
    * of the member types.
    *
    * @param handle
    *        The open HDF5 type to close.
    */
   void closeResourceRecursive(hid_t handle) const
   {
      H5T_class_t type = H5Tget_class(handle);
      if (type == H5T_COMPOUND)
      {
         int memberCount = H5Tget_nmembers(handle);
         for (int member = 0; member < memberCount; ++member)
         {
            hid_t memberHandle = H5Tget_member_type(handle, member);
            closeResourceRecursive(memberHandle);
         }
      }
      H5Tclose(handle);
   }

   /**
    * Releases an HDF5 type.
    *
    * See Resource.h for details.
    *
    * @param  args
    *         The arguments for releasing the resource. Should be of type Hdf5TypeObject::Args.
    * @param  pHandle
    *         A pointer to the handle to be freed.
    */
   void releaseResource(const Args &args, hid_t* pHandle) const
   {
      if ((pHandle != NULL) && (*pHandle != -1))
      {
         closeResourceRecursive(*pHandle);
         delete pHandle;
      }
   }
};

/**
 *  This is a Resource class that closes HDF5 types. 
 *
 *  This is a Resource class that closes HDF5 types. It has a conversion
 *  operator to allow a Hdf5TypeResource object to be used where ever a hid_t
 *  that represents an open HDF5 type handle may be used.
 *
 */
class Hdf5TypeResource : public Resource<hid_t, Hdf5TypeObject>
{
public:
   /**
    * Construct a Resource object that wraps a hid_t HDF5 type handle
    *
    * This will take ownership of an existing hid_t type handle and 
    * will ensure that it is closed.
    *
    * @param    type
    *           The HDF5 type handle.
    */
   Hdf5TypeResource(hid_t type) :
      Resource<hid_t, Hdf5TypeObject>(new hid_t(type), Args())
   {
   }

   /**
    * Default constructor.
    */
   Hdf5TypeResource() :
      Resource<hid_t, Hdf5TypeObject>(NULL, Args())
   {
   }

   /**
    *  Returns a pointer to the underlying hid_t held by this Resource.
    *
    *  Returns a pointer to the underlying hid_t. This operator,
    *  used in conjunction with the dereferencing operator,
    *  allows the Hdf5TypeResource object to be used where ever
    *  a hid_t would normally be used.
    *
    *  @return   A pointer to the underlying hid_t held by this Resource.
    */
   operator hid_t*()
   {
      return get();
   }
};

/**
 *  The Hdf5DataSpaceObject is a trait object for use with the Resource template.
 *
 *  The Hdf5DataSpaceObject is a trait object for use with the Resource template.
 *  It provides capability for closing HDF5 dataspaces.
 *
 */
class Hdf5DataSpaceObject
{
public:
   /**
    * This is an implementation detail of the Hdf5DataSpaceObject class. 
    *
    */
   struct Args
   {
      /**
       * Default constructor
       */
      Args()
      {
      }
   };

   /**
    * Obtains an HDF5 dataspace.
    *
    * See Resource.h for details.
    *
    * @param  args
    *         The arguments for obtaining the resource. Should be of type Hdf5DataSpaceObject::Args.
    *
    * @return Returns NULL
    */
   hid_t* obtainResource(const Args &args) const
   {
      return NULL;
   }

   /**
    * Releases an HDF5 dataspace.
    *
    * See Resource.h for details.
    *
    * @param  args
    *         The arguments for releasing the resource. Should be of type Hdf5DataSpaceObject::Args.
    * @param  pHandle
    *         A pointer to the handle to be freed.
    */
   void releaseResource(const Args &args, hid_t* pHandle) const
   {
      if ((pHandle != NULL) && (*pHandle != -1))
      {
         H5Sclose(*pHandle);
         delete pHandle;
      }
   }
};

/**
 *  This is a Resource class that closes HDF5 dataspaces. 
 *
 *  This is a Resource class that closes HDF5 dataspaces. It has a conversion
 *  operator to allow a Hdf5DataSpaceResource object to be used where ever a hid_t
 *  that represents an open HDF5 dataspace handle may be used.
 *
 */
class Hdf5DataSpaceResource : public Resource<hid_t, Hdf5DataSpaceObject>
{
public:
   /**
    * Construct a Resource object that wraps a hid_t HDF5 dataspace handle
    *
    * This will take ownership of an existing hid_t dataspace handle and 
    * will ensure that it is closed.
    *
    * @param    dataspace
    *           The HDF5 dataspace handle.
    */
   Hdf5DataSpaceResource(hid_t dataspace) :
      Resource<hid_t, Hdf5DataSpaceObject>(new hid_t(dataspace), Args())
   {
   }

   /**
    * Default constructor.
    */
   Hdf5DataSpaceResource() :
      Resource<hid_t, Hdf5DataSpaceObject>(NULL, Args())
   {
   }

   /**
    *  Returns a pointer to the underlying hid_t held by this Resource.
    *
    *  Returns a pointer to the underlying hid_t. This operator,
    *  used in conjunction with the dereferencing operator,
    *  allows the Hdf5DataSpaceResource object to be used where ever
    *  a hid_t would normally be used.
    *
    *  @return   A pointer to the underlying hid_t held by this Resource.
    */
   operator hid_t*()
   {
      return get();
   }
};


/**
 *  The Hdf5GroupObject is a trait object for use with the Resource template.
 *
 *  The Hdf5GroupObject is a trait object for use with the Resource template.
 *  It provides capability for closing HDF5 groups.
 *
 */
class Hdf5GroupObject
{
public:
   /**
    * This is an implementation detail of the Hdf5GroupObject class. 
    *
    */
   struct Args
   {
      /**
       * Default constructor
       */
      Args()
      {
      }
   };

   /**
    * Obtains an HDF5 group.
    *
    * See Resource.h for details.
    *
    * @param  args
    *         The arguments for obtaining the resource. Should be of type Hdf5GroupObject::Args.
    *
    * @return Returns NULL
    */
   hid_t* obtainResource(const Args &args) const
   {
      return NULL;
   }

   /**
    * Releases an HDF5 group.
    *
    * See Resource.h for details.
    *
    * @param  args
    *         The arguments for releasing the resource. Should be of type Hdf5GroupObject::Args.
    * @param  pHandle
    *         A pointer to the handle to be freed.
    */
   void releaseResource(const Args &args, hid_t* pHandle) const
   {
      if ((pHandle != NULL) && (*pHandle != -1))
      {
         H5Gclose(*pHandle);
         delete pHandle;
      }
   }
};

/**
 *  This is a Resource class that closes HDF5 groups. 
 *
 *  This is a Resource class that closes HDF5 groups. It has a conversion
 *  operator to allow a Hdf5GroupResource object to be used where ever a hid_t
 *  that represents an open HDF5 group handle may be used.
 *
 */
class Hdf5GroupResource : public Resource<hid_t, Hdf5GroupObject>
{
public:
   /**
    * Construct a Resource object that wraps a hid_t HDF5 group handle
    *
    * This will take ownership of an existing hid_t group handle and 
    * will ensure that it is closed.
    *
    * @param    group
    *           The HDF5 group handle.
    */
   Hdf5GroupResource(hid_t group) :
      Resource<hid_t, Hdf5GroupObject>(new hid_t(group), Args())
   {
   }

   /**
    * Default constructor.
    */
   Hdf5GroupResource() :
      Resource<hid_t, Hdf5GroupObject>(NULL, Args())
   {
   }

   /**
    *  Returns a pointer to the underlying hid_t held by this Resource.
    *
    *  Returns a pointer to the underlying hid_t. This operator,
    *  used in conjunction with the dereferencing operator,
    *  allows the Hdf5GroupResource object to be used where ever
    *  a hid_t would normally be used.
    *
    *  @return   A pointer to the underlying hid_t held by this Resource.
    */
   operator hid_t*()
   {
      return get();
   }
};

/**
 *  The Hdf5ErrorHandlerObject is a trait object for use with the Resource template.
 *
 *  The Hdf5ErrorHandlerObject is a trait object for use with the Resource template.
 *  It provides capability for setting and restoring a custom Hdf5 error handler.
 *
 */
class Hdf5ErrorHandlerObject
{
public:
   /**
    * This is an implementation detail of the Hdf5ErrorHandlerObject class. 
    *
    */
   struct Args
   {
      /**
       *
       */
      Args(H5E_auto1_t errorFunc, void* pClientData) : mErrorFunc(errorFunc), mpClientData(pClientData)
      {
      }

      H5E_auto1_t mErrorFunc;
      void* mpClientData;
      H5E_auto1_t mOriginalErrorFunc;
      void* mpOriginalClientData;
   };

   /**
    * Obtains an HDF5 group.
    *
    * See Resource.h for details.
    *
    * @param  org_args
    *         The arguments for obtaining the resource. Should be of type Hdf5GroupObject::Args.
    *
    * @return Returns NULL
    */
   hid_t* obtainResource(const Args &org_args) const
   {
      Args& args = const_cast<Args&>(org_args);
      H5Eget_auto1(&args.mOriginalErrorFunc, &args.mpOriginalClientData);
      H5Eset_auto1(args.mErrorFunc, args.mpClientData);
      return NULL;
   }

   /**
    * Releases an HDF5 group.
    *
    * See Resource.h for details.
    *
    * @param  args
    *         The arguments for releasing the resource. Should be of type Hdf5GroupObject::Args.
    * @param  pHandle
    *         A pointer to the handle to be freed.
    */
   void releaseResource(const Args &args, hid_t* pHandle) const
   {
      H5Eset_auto1(args.mOriginalErrorFunc, args.mpOriginalClientData);
   }
};

/**
 *  This is a Resource class that sets and restores the Hdf5 error handling function.
 */
class Hdf5ErrorHandlerResource : public Resource<hid_t, Hdf5ErrorHandlerObject>
{
public:
   /**
    * Construct a Resource object.
    */
   Hdf5ErrorHandlerResource(H5E_auto1_t errorFunc, void* pClientData) :
      Resource<hid_t, Hdf5ErrorHandlerObject>(Args(errorFunc, pClientData))
   {
   }
};

#endif
