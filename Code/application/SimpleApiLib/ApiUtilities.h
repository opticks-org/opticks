/*
* The information in this file is
* Copyright(c) 2007 Ball Aerospace & Technologies Corporation
* and is subject to the terms and conditions of the
* GNU Lesser General Public License Version 2.1
* The license text is available from   
* http://www.gnu.org/licenses/lgpl.html
*/

#ifndef APIUTILITIES_H__
#define APIUTILITIES_H__

#include "AppConfig.h"

class DataElement;
class Layer;
class View;

#ifdef __cplusplus
extern "C"
{
#endif
   /** \addtogroup simple_api
    *  This group of functions represents a datacentric API which is compatible
    *  with C calling conventions and can be used from a variety of programming language
    *  interpreters. Some C++ classes are passed around and may be treated as opaque pointers
    *  but their partial declarations may cause compilation errors. In most cases you
    *  can create a preprocessor definition for the \c class keyword which resolves to
    *  \c typedef \c void* to resolve these compilation problems.
    */
   /*@{*/

   /**
    * @file ApiUtilities.h
    * This file contains general API utilities which don't fit well into another category.
    */

   /**
    * Initialize the services pointer.
    *
    * @param pExternal
    *        Opaque handle obtained from ModuleManager::instance()->getServices() called from
    *        the main Opticks module. This generally needs to be called when loading the
    *        simple API from a DSO. If a static library is used, this may not be needed.
    *
    * @see handle()
    */
   EXPORT_SYMBOL void setHandle(void* pExternal);

   /**
    * Get the services pointer for future initialization.
    *
    * @return An opaque handle which can be passed to setHandle() in another module.
    *         This function should be called from the main Opticks module. The corresponding
    *         setHandle() function is then called from other modules. This generally needs to be
    *         used when loading the simple API from a DSO. If a static library is used, this may not be needed.
    * @see setHandle()
    */
   EXPORT_SYMBOL void* handle();

   /**
    * Get the application version number as a string.
    *
    * @param pVersion
    *        Buffer to store the version string. This will be \c NULL terminated.
    * @param versionSize
    *        The size of the buffer. If the version string is too long an error will be set.
    *        If this is 0, the minimum size of the buffer including the trailing \c NULL will be returned.
    * @return the actual length of the version string.
    */
   EXPORT_SYMBOL uint32_t getOpticksVersion(char* pVersion, uint32_t versionSize);

   /**
    * Get the test data path.
    *
    * @param pPath
    *        Buffer to store the path. This will be \c NULL terminated.
    * @param pathSize
    *        The size of the Buffer. If the name is too long, an error
    *        will be set.
    *        If this is 0, the minimum size of the buffer including the trailing \c NULL will be returned.
    * @return the actual length of the path.
    */
   EXPORT_SYMBOL uint32_t getTestDataPath(char* pPath, uint32_t pathSize);

   /**
    * Load a file using the autoimporter.
    *
    * @param pFilename
    *        The filename to load.
    * @param batch
    *        If zero, load in interactive mode, otherwise load in batch mode.
    * @return The number of datasets which were loaded. A 0 return value may indicate an error.
    */
   EXPORT_SYMBOL uint32_t loadFile(const char* pFilename, int batch);

   /**
    * Get a DataElement pointer given an identifying name.
    *
    * @param pName
    *        First, an attempt will be made to use this as a session ID.
    *        Next, this will be split into an item name path with a | delimiter.
    *        The DataElement tree is traversed until the requested item is found or an error occurs. Each item in the
    *        name path will be treated as a DataElement name. If an item is not found, each child of the current parent
    *        is checked against a display name.
    *
    * @param pType
    *        If not \c NULL or empty, an element of this type will be returned. See TypeConverter for valid type names.
    *
    * @param create
    *        If the element does not exists, should it be created? If zero, an element will not be created. If
    *        non-zero, an attempt will be made to create the element. pType must not be \c NULL if an element is to be
    *        created. Certain elements can't be created this way, most notably RasterElement.
    *
    * @return A pointer to the requested DataElement or \c NULL if the element is not found.
    *         If the element is not found, getLastError() may be queried for information on the error.
    *
    * @see createRasterElement()
    */
   EXPORT_SYMBOL DataElement* getDataElement(const char* pName, const char* pType, int create);

   /**
    * Get all top-level DataElements of a given type.
    *
    * @param pType
    *        If not \c NULL or empty, all top-level DataElements of this type will be returned. If NULL, all top-level DataElements of any type
    *        will be returned. See TypeConverter for valid type names.
    * @param maxLength
    *        The maximum length of the pElements array. pElements must be preallocated to contain at least this many
    *        DataElement pointers. If this is 0, pElements should be \c NULL and the total number of DataElements will be returned.
    * @param pElements
    *         An array of pointers to the DataElements. If inadequate space is available for the number of DataElements, the
    *         array will be truncated. If this is \c NULL no array is set and the number of DataElements will be returned.
    * @return The number of DataElements set in pElements or -1 if there is an error.
    *         getLastError() may be queried for information on the error.
    *         If pElements is \c NULL and maxLength is 0, the total number of DataElements will be returned. This can be
    *         used to allocate pElements.
    * @see getDataElement()
    */
   EXPORT_SYMBOL int getDataElements(const char* pType, int maxLength, DataElement** pElements);

   /**
    * Permanently destroy a DataElement.
    *
    * Suitable for use as a cleanup callback.
    *
    * @param pElement
    *        The DataElement to destroy.
    *        No error checking is performed on this value and a \c NULL will cause a NOOP.
    * @see ModelServices::destroyElement()
    */
   EXPORT_SYMBOL void destroyDataElement(DataElement* pElement);

   /**
    * Get the name of the given DataElement.
    *
    * @param pElement
    *        The DataElement to query.
    * @param pName
    *        Buffer to store the name. This will be \c NULL terminated.
    * @param nameSize
    *        The size of the buffer. If the name is too long, an error will be set.
    *        If this is 0, the minimum size of the buffer including the trailing \c NULL will be returned.
    * @return the actual length of the name.
    */
   EXPORT_SYMBOL uint32_t getDataElementName(DataElement* pElement, char* pName, uint32_t nameSize);

   /**
    * Get the type name of the given DataElement.
    *
    * @param pElement
    *        The DataElement to query.
    * @param pType
    *        Buffer to store the type name. This will be \c NULL terminated.
    * @param typeSize
    *        The size of the buffer. If the type name is too long, an error will be set.
    *        If this is 0, the minimum size of the buffer including the trailing \c NULL will be returned.
    * @return the actual length of the type name.
    */
   EXPORT_SYMBOL uint32_t getDataElementType(DataElement* pElement, char* pType, uint32_t typeSize);

   /**
    * Get the name of the file corresponding to the given DataElement.
    *
    * @param pElement
    *        The DataElement to query.
    * @param pFilename
    *        Buffer to store the filename. This will be \c NULL terminated.
    * @param filenameSize
    *        The size of the Buffer. If the filename is too long, an error
    *        will be set.
    *        If this is 0, the minimum size of the buffer including the trailing \c NULL will be returned.
    * @return the actual length of the filename.
    *
    * @see getDataElement()
    */
   EXPORT_SYMBOL uint32_t getDataElementFilename(DataElement* pElement, char* pFilename, uint32_t filenameSize);

   /**
    * Get the number of child DataElements of a given DataElement.
    *
    * @param pElement
    *        The DataElement to query.
    * @return the number of children.
    */
   EXPORT_SYMBOL uint32_t getDataElementChildCount(DataElement* pElement);

   /**
    * Get a child of a DataElement.
    *
    * @param pElement
    *        The parent DataElement.
    * @param index
    *        The index of the child. This may change as children are added and removed.
    * @return the child DataElement.
    */
   EXPORT_SYMBOL DataElement* getDataElementChild(DataElement* pElement, uint32_t index);

   /**
    * Cast a DataElement to a sub-type.
    *
    * @param pElement
    *        The DataElement to cast.
    * @param pType
    *        The name of the sub-type.
    * @return A void* to the cast element or \c NULL if the cast is invalid.
    */
   EXPORT_SYMBOL void* castDataElement(DataElement* pElement, const char* pType);

   /**
    * Cast a DataElement sub-type to a DataElement.
    *
    * @param pElement
    *        The sub-type to cast as a void*.
    * @param pType
    *        The name of the sub-type.
    * @return A DataElement* or \c NULL if the cast is invalid.
    */
   EXPORT_SYMBOL DataElement* castToDataElement(void* pElement, const char* pType);

   /**
    * Cast a Layer to a sub-type.
    *
    * @param pLayer
    *        The Layer to cast.
    * @param pType
    *        The name of the sub-type.
    * @return A void* to the cast layer or \c NULL if the cast is invalid.
    */
   EXPORT_SYMBOL void* castLayer(Layer* pLayer, const char* pType);

   /**
    * Copy the classification from a data element to another data element.
    *
    * @param pCopyFrom
    *        The DataElement from which the classification is to be copied.
    * @param pCopyTo
    *        The DataElement to which the classification is to be copied.
    */
   EXPORT_SYMBOL void copyClassification(DataElement* pCopyFrom, DataElement* pCopyTo);

   /*@}*/
#ifdef __cplusplus
}
#endif

#endif