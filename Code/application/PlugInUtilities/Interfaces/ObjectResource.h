/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef OBJECTRESOURCE_H
#define OBJECTRESOURCE_H

#include <string>

#include "ApplicationServices.h"
#include "ModelServices.h"
#include "ObjectFactory.h"
#include "Resource.h"
#include "TypeConverter.h"

/**
 * This is a base class for Traits to the %Resource template. 
 *
 * This is a base class for Traits to the %Resource template. It provides a quick
 * method for getting the ObjectFactory pointer that the derived classes need.
 * It also contains the Args class.
 * 
 * @see FactoryResource
 */
class FactoryAllocator
{
public:
   /**
   * This is an implementation detail of the %FactoryAllocator class. 
   *
   * This is an implementation detail of the %FactoryAllocator class. It is used 
   * for passing the parameters required by ObjectFactory::createObject.
   */
   class Args
   {
   public:
      std::string mType;
      Args(std::string type) : mType(type) {}
      Args(const char* type) : mType(type) {}
   };
protected:
   ObjectFactory* getFactory() const
   {
      Service<ApplicationServices> pApplication;
      return pApplication->getObjectFactory();
   }
};
/**
 * The %FactoryObject is a trait object for use with the %Resource template. 
 *
 * The %FactoryObject is a trait object for use with the %Resource template. It provides capability for
 * getting and returning objects from the ObjectFactory.
 * 
 * @see FactoryResource
 */
class FactoryObject : public FactoryAllocator
{
public:
   void* obtainResource(const Args& args) const
   { 
      return getFactory()->createObject(args.mType.c_str()); 
   }
   void releaseResource(const Args& args, void* pObject) const
   { 
      getFactory()->destroyObject(pObject, args.mType.c_str());
   }
};
/**
 * The %FactoryVector is a trait object for use with the %Resource template. 
 *
 * The %FactoryVector is a trait object for use with the %Resource template. It provides capability for
 * getting and returning vectors of objects from the ObjectFactory.
 * 
 * @see FactoryResource
 */
class FactoryVector : public FactoryAllocator
{
public:
   void* obtainResource(const Args& args) const
   { 
      return getFactory()->createObjectVector(args.mType.c_str()); 
   }
   void releaseResource(const Args& args, void* pVector) const
   { 
      getFactory()->destroyObjectVector(pVector, args.mType.c_str());
   }
};

typedef FactoryAllocator::Args FactoryArgs;

/**
 *  This is a %Resource class that wraps an object from the ObjectFactory. 
 *
 *  This is a %Resource class that wraps an object from the ObjectFactory. 
 *  When the %FactoryResource object goes out of scope, the object
 *  will be returned to the ObjectFactory.
 *
 *  @code
 *  void addPixels(AoiElement *pAoi)
 *  {
 *     FactoryResource<BitMask> pMask; // gets BitMask from the ObjectFactory
 *     for (int i=0; i<100; ++i)
 *        for (int j=0; j<100; ++j)
 *           if ((i^j) & 0x1) pMask->setPixel (i, j);
 *     pAoi->addPoints (pMask.get ());
 *  } // BitMask returned to ObjectFactory here
 *  @endcode
 *
 *  @see ObjectFactory
 */
template<class T>
class FactoryResource : public Resource<T, FactoryObject>
{
public:
   explicit FactoryResource() :
      Resource<T, FactoryObject>(TypeConverter::toString<T>())
   {
   }

   explicit FactoryResource<T>(T* pObject) :
      Resource<T, FactoryObject>(pObject, typename Resource<T, FactoryObject>::Args(TypeConverter::toString<T>()))
   {
   }
};

/**
 * The %ModelObject is a trait object for use with the %Resource template. 
 *
 * The %ModelObject is a trait object for use with the %Resource template. It provides capability for
 * getting and returning objects from ModelServices.
 * 
 * @see ModelResource
 */
class ModelObject
{
public:
   /**
    * This is an implementation detail of the %ModelObject class.
    *
    * This is an implementation detail of the %ModelObject class. It is used 
    * for passing the parameters required by ModelServices::create.
    */
   class Args
   {
   public:
      Args() :
         mpDescriptor(NULL),
         mpParent(NULL)
      {
      }

      Args(std::string name, std::string type, DataElement* pParent = NULL) :
         mpDescriptor(NULL),
         mName(name),
         mType(type),
         mpParent(pParent)
      {
      }

      Args(DataDescriptor* pDescriptor) :
         mpDescriptor(pDescriptor),
         mpParent(NULL)
      {
      }

      DataDescriptor* mpDescriptor;
      std::string mName;
      std::string mType;
      DataElement* mpParent;
   };

   void* obtainResource(const Args& args) const
   {
      Service<ModelServices> pModel;
      DataDescriptor* pDescriptor = args.mpDescriptor;
      if (pDescriptor == NULL)
      {
         pDescriptor = pModel->createDataDescriptor(args.mName.c_str(), args.mType.c_str(),
            args.mpParent);
      }
      if (pDescriptor != NULL)
      {
         DataElement* pElement = pModel->createElement(pDescriptor);
         pModel->destroyDataDescriptor(pDescriptor);
         return pElement;
      }

      return NULL;
   }
   void releaseResource(const Args& args, DataElement* pObject) const
   {
      Service<ModelServices>()->destroyElement(pObject);
   }
};
typedef ModelObject::Args ModelArgs;

/**
 *  This is a %Resource class that wraps an object from ModelServices. 
 *
 *  This is a %Resource class that wraps an object from ModelServices. 
 *  When the %ModelResource object goes out of scope, the object
 *  will be returned to ModelServices.
 *
 *  @code
 *  void addPixels(AoiElement *pAoi)
 *  {
 *     ModelResource<AoiElement> pAoi2("MyAoi");
 *     for (int i=0; i<100; ++i)
 *        for (int j=0; j<100; ++j)
 *           if ((i^j) & 0x1) pAoi2->addPoint (i, j);
 *     pAoi->merge (pAoi2.get ());
 *  } // AOI returned to ModelServices here
 *  @endcode
 *
 *  @see ModelServices
 */
template<class T>
class ModelResource : public Resource<T, ModelObject>
{
public:
   explicit ModelResource(std::string name, DataElement* pParent = NULL, std::string type = std::string()) :
      Resource<T, ModelObject>(
         typename Resource<T, ModelObject>::Args(name, type.empty() ? TypeConverter::toString<T>() : type, pParent))
   {
   }

   /**
    * Create a ModelResource for the given descriptor.
    *
    * @param pDescriptor
    *        The DataDescriptor which describes the desired DataElement.
    *        ModelResource takes ownership of this object.  It is not
    *        safe to dereference pDescriptor after creating the ModelResource.
    */
   explicit ModelResource(DataDescriptor *pDescriptor) :
      Resource<T, ModelObject>(typename Resource<T, ModelObject>::Args(pDescriptor))
   {
   }

   /**
    * Create a ModelResource which owns the provided element and will
    * destroy it when the resource is destroyed.
    *
    * @param pElement the element that will be owned by this resource.
    */
   explicit ModelResource(T* pElement) :
      Resource<T, ModelObject>(pElement, typename Resource<T, ModelObject>::Args())
   {
   }
};

class DataDescriptor;

class DataDescriptorObject
{
public:
   typedef ModelObject::Args Args;

   void* obtainResource(const Args& args) const
   {
      if (args.mpDescriptor == NULL)
      {
         return Service<ModelServices>()->createDataDescriptor(args.mName.c_str(), args.mType.c_str(),
                                                                     args.mpParent);
      }

      return args.mpDescriptor;
   }
   void releaseResource(const Args& args, DataDescriptor* pObject) const
   {
      Service<ModelServices>()->destroyDataDescriptor(pObject);
   }
};

template<class T>
class DataDescriptorResource : public Resource<T, DataDescriptorObject>
{
public:
   explicit DataDescriptorResource(std::string name, std::string dataDescriptorType, DataElement* pParent = NULL) :
      Resource<T, DataDescriptorObject>(
         typename Resource<T, DataDescriptorObject>::Args(name, dataDescriptorType, pParent))
   {
   }

   explicit DataDescriptorResource(T* pDescriptor) :
      Resource<T, DataDescriptorObject>(typename Resource<T, DataDescriptorObject>::Args(pDescriptor))
   {
   }
};

class ImportDescriptorObject
{
public:
   class Args
   {
   public:
      Args() :
         mpDescriptor(NULL),
         mpDataDescriptor(NULL),
         mpParent(NULL),
         mImported(false)
      {
      }

      Args(std::string name, std::string type, DataElement* pParent, bool imported) :
         mpDescriptor(NULL),
         mpDataDescriptor(NULL),
         mName(name),
         mType(type),
         mpParent(pParent),
         mImported(imported)
      {
      }

      Args(std::string name, std::string type, const std::vector<std::string>& parent, bool imported) :
         mpDescriptor(NULL),
         mpDataDescriptor(NULL),
         mName(name),
         mType(type),
         mpParent(NULL),
         mParent(parent),
         mImported(imported)
      {
      }

      Args(ImportDescriptor* pDescriptor) :
         mpDescriptor(pDescriptor),
         mpDataDescriptor(NULL),
         mpParent(NULL),
         mImported(false)
      {
      }

      Args(DataDescriptor* pDescriptor, bool imported) :
         mpDescriptor(NULL),
         mpDataDescriptor(pDescriptor),
         mpParent(NULL),
         mImported(imported)
      {
      }

      ImportDescriptor* mpDescriptor;
      DataDescriptor* mpDataDescriptor;
      std::string mName;
      std::string mType;
      DataElement* mpParent;
      std::vector<std::string> mParent;
      bool mImported;
   };

   void* obtainResource(const Args& args) const
   {
      if (args.mpDescriptor == NULL && args.mpDataDescriptor == NULL)
      {
         if (args.mpParent == NULL && !args.mParent.empty())
         {
            return Service<ModelServices>()->createImportDescriptor(args.mName, args.mType,
                                                                     args.mParent, args.mImported);
         }
         return Service<ModelServices>()->createImportDescriptor(args.mName, args.mType,
                                                                     args.mpParent, args.mImported);
      }
      else if (args.mpDescriptor == NULL)
      {
         return Service<ModelServices>()->createImportDescriptor(args.mpDataDescriptor, args.mImported);
      }

      return args.mpDescriptor;
   }
   void releaseResource(const Args &args, ImportDescriptor *pObject) const
   {
      Service<ModelServices>()->destroyImportDescriptor(pObject);
   }
};

class ImportDescriptorResource : public Resource<ImportDescriptor, ImportDescriptorObject>
{
public:
   explicit ImportDescriptorResource(std::string name, std::string dataDescriptorType, DataElement* pParent = NULL,
                                     bool imported = true) :
      Resource<ImportDescriptor, ImportDescriptorObject>(Args(name, dataDescriptorType, pParent, imported))
   {
   }

   explicit ImportDescriptorResource(std::string name, std::string dataDescriptorType,
                                     const std::vector<std::string>& parent, bool imported = true) :
      Resource<ImportDescriptor, ImportDescriptorObject>(Args(name, dataDescriptorType, parent, imported))
   {
   }

   explicit ImportDescriptorResource(ImportDescriptor* pDescriptor) :
      Resource<ImportDescriptor, ImportDescriptorObject>(Args(pDescriptor))
   {
   }

   explicit ImportDescriptorResource(DataDescriptor* pDescriptor, bool imported = true) :
      Resource<ImportDescriptor, ImportDescriptorObject>(Args(pDescriptor, imported))
   {
   }
};

/**
 * This is an implementation detail of the ObjectArray class. 
 *
 * This is an implementation detail of the ObjectArray class. It is used 
 * for passing the size parameter required by new[].
 */
class ObjectArrayArgs
{
public:
   ObjectArrayArgs(int size, bool noThrow) :
      mSize(size),
      mNoThrow(noThrow)
   {
   }

   int mSize;
   bool mNoThrow;
};

/**
 * The %ObjectArray is a trait object for use with the %Resource template. 
 *
 * The %ObjectArray is a trait object for use with the %Resource template.
 * It provides capability for creating and deleting arrays of objects from the local heap.
 * 
 * @see ArrayResource
 */
template<class T>
class ObjectArray
{
public:
   typedef ObjectArrayArgs Args;
   T* obtainResource(const Args& args) const 
   {
      if (args.mSize != 0)
      {
         if (args.mNoThrow)
         {
            return new (std::nothrow) T[args.mSize];
         }

         return new T[args.mSize];
      }

      return NULL;
   }

   void releaseResource(const Args& args, T* pObject) const
   {
      delete [] pObject;
   }
};

typedef ObjectArrayArgs ArrayArgs;

/**
 *  This is a %Resource class that wraps an array of objects on the local heap.
 *
 *  This is a %Resource class that wraps an array of objects on the local heap.
 *  It is in effect a %Resource<T,MemoryObject> except that the ObjectArray
 *  SourceTrait uses the [] versions of new and delete. It also supports a
 *  range checking indexing method called 'at' that functions similarly to
 *  the method of the same name on std::vector and std::deque. Since it
 *  uses new[], it will use the default constructor to make the objects of
 *  type T. The primary reasons for using an %ArrayResource in place of a
 *  std::vector is for the different ownership semantics, and that a copy
 *  is not made when ownership is transfered. An %ArrayResource can be
 *  returned from a method without making a copy of the contained data.
 *
 *  @code
 *  {
 *     ArrayResource<double> pArray(12); // allocate an array of 12 doubles
 *     for (int i=0; i<12; ++i)
 *        pArray = cos(3.141592654/12.0 * i);
 *  } // delete [] called on pArray here
 *  @endcode
 *
 *  @see ObjectFactory
 */
template<class T>
class ArrayResource : public Resource<T, ObjectArray<T> >
{
public:
   /**
    *  Constructs the %Resource object based on a newly allocated heap array.
    *
    *  Essentially, this allocates and wraps an array
    *  in the Resource and assigns the Resource object the responsibility for freeing the array.
    *
    *  @param   args
    *           The arguments that would have been provided to the obtainResource method of the ObjectArray class.
    */
   explicit ArrayResource(const typename Resource<T, ObjectArray<T> >::Args& args) :
      Resource<T, ObjectArray<T> >(args)
   {
   }

   /**
    *  Constructs the %Resource object based on a newly allocated heap array.
    *
    *  Essentially, this allocates and wraps an array
    *  in the Resource and assigns the Resource object the responsibility for freeing the array.
    *
    *  @param   size
    *           The number of objects in the array.
    *  @param   noThrow
    *           The behavior for a failed allocation.
    *           \c True to return \c NULL from get(), \c false to throw \c std::bad_alloc from this constructor.
    */
   explicit ArrayResource(int size, bool noThrow = false) :
      Resource<T, ObjectArray<T> >(typename Resource<T, ObjectArray<T> >::Args(size, noThrow))
   {
   }

   /**
    *  Constructs the %Resource object based on an existing heap array.
    *
    *  Essentially, this wraps the array
    *  in the Resource and assigns the Resource object the responsibility for freeing the array.
    *
    *  @param   pObject
    *           The array to wrap in the Resource.
    *  @param   args
    *           The arguments that would have been provided to the obtainResource method of the ObjectArray class.
    */
   ArrayResource(T* pObject, const typename Resource<T, ObjectArray<T> >::Args& args) :
      Resource<T, ObjectArray<T> >(pObject, args)
   {
   }

   /**
    *  Constructs the %Resource object based on an existing heap array.
    *
    *  Essentially, this wraps the array
    *  in the Resource and assigns the Resource object the responsibility for freeing the array.
    *
    *  @param   pObject
    *           The array to wrap in the Resource.
    *  @param   size
    *           The number of objects in the array.
    *  @param   noThrow
    *           The behavior for a failed allocation.
    *           \c True to return \c NULL from get(), \c false to throw \c std::bad_alloc from this constructor.
    */
   ArrayResource(T* pObject, int size, bool noThrow = false) :
      Resource<T, ObjectArray<T> >(pObject, typename Resource<T, ObjectArray<T> >::Args(size, noThrow))
   {
   }

   ArrayResource(const Resource<T, ObjectArray<T> >& source) :
      Resource<T, ObjectArray<T> >(source)
   {
   }

   /**
    *  Returns the number of objects contained in the underlying array.
    *
    *  @return   The number of objects contained in the underlying array.
    */
   int size() const
   {
      return Resource<T, ObjectArray<T> >::getArgs().mSize;
   }

   /**
    *  Returns a reference to the indexed object.
    *
    *  Indexes into the underlying array and returns a
    *  reference to the indexed object. If the index is out
    *  of bounds, based on the Args object, it will throw
    *  an exception.
    *
    *  @param   index
    *           The index of the object in the underlying array.
    *
    *  @return   A reference to the underlying indexed object.
    */
   T& at(int index) const
   {
      if (index >= size())
      {
         throw std::exception("Range Error in ArrayResource");
      }

      return Resource<T, ObjectArray<T> >::operator[](index);
   }
};

#endif
