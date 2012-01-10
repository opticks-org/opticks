/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef TYPECONVERTER_H
#define TYPECONVERTER_H

#include <string>
#include <vector>

/**
 * This namespace is a common place to put type to string conversions.
 *
 * To use, simply do the following:
 * @code
 * TypeConverter::toString<int>(); // returns "int"
 * DataElement *pElement;
 * TypeConverter::toString(pElement); // returns "DataElement"
 * TypeConverter::toString<std::vector<int> >(); // returns "vector<int>"
 * TypeConverter::toString<std::vector<DataElement*> >(); // return "vector<DataElement>"
 * @endcode
 */
namespace TypeConverter
{
   /**
    * Generic form of type conversion.
    *
    * This function is explicitly specialized to return type names for those
    * listed in \ref typeconverter_types
    *
    * @return A string containing the type name.
    */
   template<typename T>
   const char* toString();

   /**
    * This function is exactly like the one above it, but may be more convenient
    * where there is already an instance of the type desired.
    *
    * @return A string containing the type name.
    */
   template<typename T>
   const char* toString(const T *)
   {
      return toString<T>();
   }
};


#endif
