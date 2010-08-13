/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DATA_VARIANT_FACTORY_H
#define DATA_VARIANT_FACTORY_H

#include "Service.h"
#include "XercesIncludes.h"

#include <string>

using XERCES_CPP_NAMESPACE_QUALIFIER DOMNode;
class DataValueWrapper;

/**
 *  \ingroup ServiceModule
 */
class DataVariantFactory
{
public:
   /**
    *  Creates a DataValueWrapper object for use in a DataVariant. This method
    *  is not intended for direct use in client code.
    *
    *  @param   pObject
    *           A pointer to an object to wrap. This object will be passed to
    *           copy constructor of the new object. If it is NULL, the default
    *           constructor will be used.
    *  @param   className
    *           A string containing the class name of the object to be
    *           wrapped. The supported types are documented under DataVariant.
    *           The format of the type string can be either that used by 
    *           type_id or that used by TypeConverter.
    *  @param   strict
    *           If true, a verification error message will be generated if
    *           a wrapper of the given className cannot be constructed.  If
    *           false, no verification error message will be generated.
    *
    *  @return  A pointer to the wrapper.
    *
    *  @see DataVariant::DataVariant()
    */
   virtual DataValueWrapper* createWrapper(const void *pObject, const std::string &className, bool strict = true) = 0;

   /**
    *  Creates a DataValueWrapper object for use in a DataVariant. This method
    *  is not intended for direct use in client code.
    *
    *  @param   pDocument
    *           A pointer to the XML document to read the object from.
    *  @param   version
    *           The version number of the XML file being read.
    *
    *  @return  A pointer to the wrapper.
    *
    *  @see DataVariant::DataVariant()
    */
   virtual DataValueWrapper* createWrapper(DOMNode *pDocument, int version) = 0;

protected:
   /**
    * This will be cleaned up during application close.  Plug-ins do not
    * need to destroy it.
    */
   virtual ~DataVariantFactory() {}
};

#endif
