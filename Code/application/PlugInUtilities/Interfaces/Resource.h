/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef RESOURCE_H
#define RESOURCE_H

#include <cstdlib>

/**
 * The MemoryObject is a trait object for use with the Resource template. It provides capability for creating and deleting
 * objects from the local heap. A Resource using this trait behaves the same as an auto_ptr. This is the default
 * trait used if a Resource object is declared without specifying the trait class.
 */
template<class T>
class MemoryObject
{
public:
   class Args {};
   T* obtainResource(const Args& args) const
   {
      return new T;
   }

   void releaseResource(const Args& args, T* pObject) const
   {
      delete pObject;
   }
};

/**
 *  This is the basic template that underlies all of the Resource capability.
 *
 * The Resource template is effectively an auto_ptr with a trait describing how to obtain and release
 * the resource. The default trait is MemoryObject, which results in identical syntax and functionality
 * to an auto_ptr. It can also be used to manage FILE *, objects and vectors created via the Object
 * Factory and spectral elements created via ModelServices. The following examples show creation
 * (and upon exit from the code block - destruction) of each of these types.
 * 
 * The code will work in a plug-in or the Studio. Plug-ins must be compiled with _USRDLL defined. The
 * code will not compile otherwise.
 * 
 * @code
 * {
 *    // Allocate a PassThru object from the heap. The PassThru object will be deleted on destruction of the Resource
 *    Resource<struct PassThru> pMyObject;
 *    Resource<struct PassThru> pMyObject2(new PassThru); // alternate syntax to the above line
 *
 *    // Open a file for reading. On destruction of the FileResource, the file will be closed
 *    Resource<FILE,FileObject> pMyFile(FileObject::Args("e:\\application\\configure.in", "r"));
 *    FileResource pMyFile2(FileObject::Args("e:\\application\\configure", "r")); // alternate syntax to the above line
 *    FileResource pMyFile3("e:\\application\\configure.out", "r"); // alternate syntax to the above line
 * 
 *    // Create a Filename object via the ObjectFactory. On destruction of the FactoryResource, the 
 *    // object will be destroyed via the ObjectFactory.
 *    Resource<Filename,FactoryObject> pMyFilename(FactoryArgs("Filename"));
 *    FactoryResource<Filename> pMyFilename2(); // alternate syntax to the above line
 * 
 *    // Create an empty vector of Filename objects via the ObjectFactory.
 *    Resource<vector<Filename*>,FactoryVector> pMyVector(FactoryArgs("Filename"));
 * 
 *    // Create a DataElement via ModelServices. On destruction of the ModelResource, the 
 *    // object will be destroyed via ModelServices.
 *    Resource<AoiElement,ModelObject> pMyAoi(ModelArgs("MyAoi", "AoiElement", mpRasterElement->getName()));
 *    ModelResource<AoiElement> pMyAoi2("MyAoi", mpRasterElement->getName()); // alternate syntax to the above line
 * } // closes all 3 files, deletes the Filename, deletes the vector, destroys the AOI and deletes both PassThru's
 * @endcode
 *
 * Additional traits can be defined as needed. The requirements for a traits class are as follows.
 * 1) It must have a default constructor
 * 2) It must define a nested struct/class/typedef called Args
 * 3) It must define a method called obtainResource taking an Args object as its only non-default argument and returning a value that can be cast to a (T*)
 *     If this call fails, the obtainResource method should throw an exception to prevent the creation of an invalid Resource object.
 * 4) It must define a method called releaseResource taking an Args object and a T* as its only non-default arguments
 * The only requirement on the nested 'Args' struct/class is that it have a copy constructor.
 *
 *  @param   T
 *           The type of the object the Resource wraps
 *  @param   SourceTrait
 *           The type of object to use for obtaining and releasing objects of type T
*/
template <class T, class SourceTrait = MemoryObject<T> >
class Resource
{
protected:
   typedef typename SourceTrait::Args Args;

public:
   /**
    *  Constructs the Resource object.
    *
    *  Constructs the Resource object by storing the args and by calling obtainResource
    *  on the SourceTrait. If no argument is provided, it will construct the Resource using
    *  the SourceTrait's default Args object.
    *
    *  @param   args
    *           The arguments that will be provided to the obtainResource method of the SourceTrait. The type
    *           of this argument is dependent on the SourceTrait.
    */
   explicit Resource(const Args& args = Args()) :
      mArgs(args),
      mOwns(true),
      mpObject(static_cast<T*>(SourceTrait().obtainResource(args))) {}

   /**
    *  Constructs the Resource object.
    *
    *  Constructs the Resource object based on an existing object. Essentially, this wraps the object
    *  in the Resource and assigns the Resource object the responsibility for freeing the underlying object.
    *
    *  @param   pObject
    *           The object to wrap in the Resource.
    *  @param   args
    *           The arguments that would have been provided to the obtainResource method of the SourceTrait. The type
    *           of this argument is dependent on the SourceTrait.
    */
   explicit Resource(T* pObject, const Args& args = Args()) :
      mArgs(args),
      mOwns(true),
      mpObject(pObject) {}

   /**
    *  Copy-constructs the Resource object.
    *
    *  Copy-constructs a Resource object from an existing Resource object. The new Resource object
    *  takes ownership of the wrapped object from the source object.
    *
    *  @param   source
    *           The Resource object to construct from. After this call, the source no longer owns the wrapped object.
    */
   Resource(const Resource<T, SourceTrait>& source) :
      mArgs(source.mArgs),
      mOwns(source.mOwns),
      mpObject(const_cast<T*>(source.release())) {}

   /**
    *  Sets a Resource object to another one.
    *
    *  The assigned object takes ownership of the wrapped object from the
    *  source object.
    *
    *  @param   source
    *           The Resource object to assign from. After this call, the source no longer owns the wrapped object.
    */
   Resource<T, SourceTrait>& operator=(const Resource<T, SourceTrait>& source)
   {
      if (this != &source)
      {
         if (mpObject != source.get())
         {
            destroyIfOwned();
            mOwns = source.mOwns;
         }
         else if (source.mOwns)
         {
            mOwns = true;
         }
         mpObject = const_cast<T*>(source.release());
         mArgs = source.mArgs;
      }
      return *this;
   }

   /**
    *  Destructs a %Resource object.
    *
    *  If the %Resource object owns the wrapped object, the wrapped object will
    *  be freed via the SourceTrait's releaseResource method.
    */
   virtual ~Resource()
   {
      destroyIfOwned();
   }

   /**
    *  Gets the args that were used when the %Resource was created.
    *
    *  @return   The args that were used when the %Resource was created.
    */
   const Args& getArgs() const
   {
      return mArgs;
   }

   /**
    *  Gets the args that were used when the %Resource was created.
    *
    *  @return   The args that were used when the %Resource was created.
    */
   Args& getArgs()
   {
      return mArgs;
   }

   /**
    *  Gets a pointer to the underlying object.
    *
    *  @return   A pointer to the underlying object.
    */
   const T* get() const
   {
      return mpObject;
   }

   /**
    *  Gets a pointer to the underlying object.
    *
    *  @return   A pointer to the underlying object.
    */
   T* get()
   {
      return mpObject;
   }

   /**
    *  Gets a pointer to the underlying object.
    *
    *  @return   A pointer to the underlying object.
    */
   const T* operator->() const
   {
      return get();
   }

   /**
    *  Gets a pointer to the underlying object.
    *
    *  @return   A pointer to the underlying object.
    */
   T* operator->()
   {
      return get();
   }

   /**
    *  Gets a reference to the underlying object.
    *
    *  @return   A reference to the underlying object.
    */
   const T& operator*() const
   {
      return *get();
   }

   /**
    *  Gets a reference to the underlying object.
    *
    *  @return   A reference to the underlying object.
    */
   T& operator*()
   {
      return *get();
   }

   /**
    *  Returns a reference to the indexed object.
    *
    *  Indexes into the underlying array and returns a 
    *  reference to the indexed object. This is only useful
    *  if the SourceTrait's obtainResource and releaseResource
    *  work with arrays of objects. In other cases, calling
    *  this method with an index of other than 0 will cause
    *  undefined behavior.
    *
    *  @param   index
    *           The index of the object in the underlying array.
    *
    *  @return   A reference to the underlying indexed object.
    */
   const T& operator[](int index) const
   {
      return get()[index];
   }

   /**
    *  Returns a reference to the indexed object.
    *
    *  Indexes into the underlying array and returns a 
    *  reference to the indexed object. This is only useful
    *  if the SourceTrait's obtainResource and releaseResource
    *  work with arrays of objects. In other cases, calling
    *  this method with an index of other than 0 will cause
    *  undefined behavior.
    *
    *  @param   index
    *           The index of the object in the underlying array.
    *
    *  @return   A reference to the underlying indexed object.
    */
   T& operator[](int index)
   {
      return get()[index];
   }

   /**
    *  Removes ownership of the underlying resource and returns it.
    *
    *  Gets a pointer to the underlying object and removes
    *  ownership of the underlying object. After this call, the
    *  %Resource object no longer owns the underlying object.
    *
    *  @return   A pointer to the underlying object.
    */
   const T* release() const
   {
      mOwns = false;
      return get();
   }

   /**
    *  Removes ownership of the underlying resource and returns it.
    *
    *  Gets a pointer to the underlying object and removes
    *  ownership of the underlying object. After this call, the
    *  %Resource object no longer owns the underlying object.
    *
    *  @return   A pointer to the underlying object.
    */
   T* release()
   {
      mOwns = false;
      return get();
   }

protected:
   /**
    *  Destroys the underlying resource.
    *
    *  If the underlying object is owned by the %Resource, this
    *  method destroys it via the SourceTrait's releaseResource
    *  method. After destroying it, the %Resource object is empty.
    */
   void destroyIfOwned()
   {
      if (mpObject != NULL && mOwns)
      {
         SourceTrait().releaseResource(mArgs, mpObject);
         mpObject = NULL;
         mOwns = false;
      }
   }

private:
   Args mArgs;
   mutable bool mOwns;
   T* mpObject;
};

/**
 * RAII object to reset a variable.
 */
template<typename T>
class ResetVariableOnDestroy
{
public:
   /**
    * Construct the object.
    *
    * @param variable
    *        The variable to reset to its initial value.
    *
    * @param value
    *        The value to put in the variable until destruction.
    */
   ResetVariableOnDestroy(T& variable, const T& value) :
      mVariable(variable),
      mOriginalValue(variable)
   {
      mVariable = value;
   }

   /**
    * Reset the initial value in the variable.
    */
   ~ResetVariableOnDestroy()
   {
      mVariable = mOriginalValue;
   }

private:
   T& mVariable;
   T mOriginalValue;
};

#endif
