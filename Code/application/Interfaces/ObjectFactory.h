/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef __OBJCTFCT_H
#define __OBJCTFCT_H

#include "Service.h"

#include <string>

/**
 *  \ingroup ServiceModule
 *  Dynamic creation of arbitrary class objects
 *
 *  The ObjectFactory can create and destroy objects of arbitrary classes
 *  dynamically at runtime.
 */
class ObjectFactory
{
public:
   /**
    *  Instantiate an object whose type is determined at runtime.
    *
    *  @param   className
    *           A string containing the class name of the object
    *           to be instantiated.
    *           The valid object types are:
    *           - unsigned char
    *           - char
    *           - unsigned short
    *           - short
    *           - unsigned int
    *           - int
    *           - unsigned long
    *           - long
    *           - float
    *           - double
    *           - bool
    *           - string
    *           - BadValues
    *           - BitMask
    *           - Classification
    *           - DataRequest
    *           - DataVariantAnyData
    *           - DateTime
    *           - DynamicObject
    *           - ExecutableAgent
    *           - ExportAgent
    *           - FileDescriptor
    *           - FileFinder
    *           - Filename
    *           - Font
    *           - ImportAgent
    *           - PointCloudDataRequest
    *           - PointCloudFileDescriptor
    *           - RasterFileDescriptor
    *           - SettableSessionItem
    *           - SignatureFileDescriptor
    *           - Units
    *           - Wavelengths
    *           - WizardObject
    *           .
    *           Vectors may also be created with a string of the form "vector<int>"
    *           for the types listed in createObjectVector().
    *
    *  @return  A pointer to the instantiated object if successful, or NULL.
    */
   virtual void* createObject(const std::string& className) = 0;

   /**
    *  Deallocate an object that was previously allocated by the ObjectFactory,
    *  including vectors.
    *
    *  @param   pObject
    *           A pointer to an object previously allocated by the ObjectFactory.
    *  @param   className
    *           A string containing the class name of the object to be
    *           deallocated.
    */
   virtual void destroyObject(void* pObject, const std::string& className) = 0;

   /**
    *  Instantiate an empty vector of objects whose type is determined at runtime.
    *
    *  @deprecated
    *           This method is deprecated, and may be removed in a future
    *           version.\  Use ObjectFactory::createObject() instead with an
    *           argument of "vector<type>".
    *
    *  @param   className
    *           A string containing the name of the class of the object
    *           to be instantiated.
    *           The valid object types are:
    *           - unsigned char
    *           - char
    *           - unsigned short
    *           - short
    *           - unsigned int
    *           - int
    *           - unsigned long
    *           - long
    *           - float
    *           - double
    *           - bool
    *           - string
    *           - void*
    *           - unsigned char*
    *           - char*
    *           - unsigned short*
    *           - short*
    *           - unsigned int*
    *           - int*
    *           - unsigned long*
    *           - long*
    *           - float*
    *           - double*
    *           - bool*
    *           - string*
    *           - BitMask
    *           - Classification
    *           - DateTime
    *           - DynamicObject
    *           - FileDescriptor
    *           - FileFinder
    *           - Filename
    *           - GraphicObject
    *           - Layer
    *           - PointCloudFileDescriptor
    *           - RasterFileDescriptor
    *           - SignatureFileDescriptor
    *           - Units
    *           - Wavelengths
    *           - WizardObject
    *
    *  @return  A pointer to the instantiated vector if successful, or NULL.
    */
   virtual void* createObjectVector(const std::string& className) = 0;

   /**
    *  Deallocate a vector of objects whose type is determined at runtime.
    *
    *  This method deletes a vector that was previously allocatied by the object
    *  factory.  If the vector contains pointers, the objects that the pointers
    *  point to are not deleted.
    *
    *  @deprecated
    *           This method is deprecated, and may be removed in a future
    *           version.\  Use ObjectFactory::destroyObject() instead with an
    *           argument of "vector<type>".
    *
    *  @param   pVector
    *           A pointer to a vector previously allocated by the ObjectFactory.
    *  @param   className
    *           A string containing the name of the class of the object
    *           to be instantiated.
    */
   virtual void destroyObjectVector(void* pVector, const std::string& className) = 0;

protected:
   /**
    * This will be cleaned up during application close.  Plug-ins do not
    * need to destroy it.
    */
   virtual ~ObjectFactory() {}
};

#endif
