/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef STRINGUTILITIESMACROS_H__
#define STRINGUTILITIESMACROS_H__

/** \page stringutilitiesmacros StringUtilities macros
 *  The StringUtilitiesMacros.h file contains definitions for macros which simplify defining type to string mappings.
 *  Basic usage is as follows.
 *  \code
 *  STRINGSTREAM_MAPPING(int)
 *
 *  STRINGSTREAM_MAPPING_PRECISION(float, numeric_limits<float>::digits10)
 *
 *  BEGIN_ENUM_MAPPING(AoiAddMode)
 *  ADD_ENUM_MAPPING(APPEND_AOI, "Append", "Append")
 *  ADD_ENUM_MAPPING(REPLACE_AOI, "Replace", "Replace")
 *  ADD_ENUM_MAPPING(NEW_AOI, "New", "New")
 *  END_ENUM_MAPPING()
 *  \endcode
 *
 *  Advanced configurations, such as using the macros for type to string conversion but specifying a custom string to type conversion are beyond the scope of this document.
 *  If you need such advanced usage, it is suggested that you study the macro definitions in StringUtilitiesMacros.h to determine the best
 *  combination of macros and custom code.
 */

#include "StringUtilities.h"
#include <algorithm>
#include <boost/tuple/tuple.hpp>
#include <limits>
#include <string>
#include <vector>

using namespace std;

template<typename T>
std::string convertVectorToString(const std::vector<T>& vec, bool* pError, std::string sep, bool forDisplay)
{
   bool error = false;
   if (pError != NULL)
   {
      *pError = false;
   }
   string vecString = "";
   for (vector<T>::const_iterator iter = vec.begin(); iter != vec.end(); ++iter)
   {
      string curVal;
      if (forDisplay)
      {
         curVal = StringUtilities::toDisplayString(*iter, &error);
      }
      else
      {
         curVal = StringUtilities::toXmlString(*iter, &error);
      }
      if (error)
      {
         if (pError != NULL)
         {
            *pError = true;
         }
         return "";
      }
      vecString += curVal;
      if ( (iter + 1) != vec.end())
      {
          vecString += sep;
      }
   }
   return vecString;
}

template<class T>
std::vector<T> convertStringToVector(string value, bool* pError, std::string sep, bool forDisplay)
{
   vector<T> retValues;
   bool parsing = true;
   bool error = false;
   string::size_type lastIndex = 0;
   if (pError != NULL)
   {
      *pError = false;
   }
   if (value.empty())
   {
      return retValues;
   }
   while (parsing)
   {
      string::size_type index = value.find(sep, lastIndex);
      if (index == string::npos)
      {
         index = value.size();
      }
      string token = value.substr(lastIndex, (index - lastIndex));
      T parsedValue;
      if (forDisplay)
      {
         parsedValue = StringUtilities::fromDisplayString<T>(token, &error);
      }
      else
      {
         parsedValue = StringUtilities::fromXmlString<T>(token, &error);
      }
      if (error)
      {
         if (pError != NULL)
         {
            *pError = true;
         }
         retValues.clear();
         return retValues;
      }
      retValues.push_back(parsedValue);
      lastIndex = index + sep.size();
      if (index == value.size())
      {
         parsing = false;
      }
   }
   
   return retValues;
}

template<int tupleIndex, typename CompareType>
class FindTupleValue
{
public:
   FindTupleValue(CompareType valueToFind) : mValue(valueToFind)
   {
   }

   template<typename tupleType>
   bool operator()(tupleType compareValue)
   {
      return compareValue.get<tupleIndex>() == mValue;
   }

private:
   CompareType mValue;
};

#define ENUM_MAPPING_TO_DISPLAY_STRING(eName__, eAlias__) \
template<>\
std::string StringUtilities::toDisplayString(const eName__& val, bool* pError)\
{\
   eAlias__##Mapping values = get##eAlias__##Mapping();\
   eAlias__##Mapping::iterator foundValue;\
   foundValue = std::find_if(values.begin(), values.end(), FindTupleValue<0, eName__>(val));\
   if (foundValue != values.end())\
   {\
      if (pError != NULL)\
      {\
         *pError = false;\
      }\
      return foundValue->get<1>();\
   }\
   if (pError != NULL)\
   {\
      *pError = true;\
   }\
   return "";\
}\
template<>\
std::string StringUtilities::toDisplayString(const eName__::EnumType& val, bool* pError)\
{\
   return toDisplayString(EnumWrapper<eName__::EnumType>(val), pError);\
}\

#define ENUM_MAPPING_TO_DISPLAY_STRING_VEC(eName__) \
template<>\
std::string StringUtilities::toDisplayString(const vector<eName__>& vec, bool* pError)\
{\
   return convertVectorToString(vec, pError, ", ", true);\
}\

#define ENUM_MAPPING_TO_XML_STRING(eName__, eAlias__) \
template<>\
std::string StringUtilities::toXmlString(const eName__& val, bool* pError)\
{\
   eAlias__##Mapping values = get##eAlias__##Mapping();\
   eAlias__##Mapping::iterator foundValue;\
   foundValue = std::find_if(values.begin(), values.end(), FindTupleValue<0, eName__>(val));\
   if (foundValue != values.end())\
   {\
      if (pError != NULL)\
      {\
         *pError = false;\
      }\
      return foundValue->get<2>();\
   }\
   if (pError != NULL)\
   {\
      *pError = true;\
   }\
   return "";\
}\
template<>\
std::string StringUtilities::toXmlString(const eName__::EnumType& val, bool* pError)\
{\
   return toXmlString(EnumWrapper<eName__::EnumType>(val), pError);\
}\

#define ENUM_MAPPING_TO_XML_STRING_VEC(eName__) \
template<>\
std::string StringUtilities::toXmlString(const vector<eName__>& vec, bool* pError)\
{\
   return convertVectorToString(vec, pError, ", ", false);\
}\

#define ENUM_MAPPING_FROM_DISPLAY_STRING(eName__, eAlias__) \
template<>\
eName__ StringUtilities::fromDisplayString<eName__>(std::string value, bool* pError)\
{\
   eAlias__##Mapping values = get##eAlias__##Mapping();\
   eAlias__##Mapping::iterator foundValue;\
   foundValue = std::find_if(values.begin(), values.end(), FindTupleValue<1, string>(value));\
   if (foundValue != values.end())\
   {\
      if (pError != NULL)\
      {\
         *pError = false;\
      }\
      return foundValue->get<0>();\
   }\
   if (pError != NULL)\
   {\
      *pError = true;\
   }\
   return eName__();\
}\

#define ENUM_MAPPING_FROM_DISPLAY_STRING_VEC(eName__) \
template<>\
vector<eName__> StringUtilities::fromDisplayString<vector<eName__> >(std::string value, bool* pError)\
{\
   return convertStringToVector<eName__>(value, pError, ", ", true);\
}\

#define ENUM_MAPPING_FROM_XML_STRING(eName__, eAlias__) \
template<>\
eName__ StringUtilities::fromXmlString<eName__>(std::string value, bool* pError)\
{\
   eAlias__##Mapping values = get##eAlias__##Mapping();\
   eAlias__##Mapping::iterator foundValue;\
   foundValue = std::find_if(values.begin(), values.end(), FindTupleValue<2, string>(value));\
   if (foundValue != values.end())\
   {\
      if (pError != NULL)\
      {\
         *pError = false;\
      }\
      return foundValue->get<0>();\
   }\
   if (pError != NULL)\
   {\
      *pError = true;\
   }\
   return eName__();\
}\

#define ENUM_MAPPING_FROM_XML_STRING_VEC(eName__) \
template<>\
vector<eName__> StringUtilities::fromXmlString<vector<eName__> >(std::string value, bool* pError)\
{\
   return convertStringToVector<eName__>(value, pError, ", ", false);\
}\

#define ENUM_MAPPING_PRE_DEFINITIONS(eName__, eAlias__) \
typedef vector<boost::tuple<eName__, string, string> > eAlias__##Mapping;\
const eAlias__##Mapping & get##eAlias__##Mapping();\

#define ENUM_MAPPING_FUNCTION(eAlias__) \
const eAlias__##Mapping& get##eAlias__##Mapping()\
{\
   static eAlias__##Mapping values;\
   if (values.empty())\
   {\

#define ENUM_GET_ALL_VALUES(eName__, eAlias__) \
template<>\
vector<eName__> StringUtilities::getAllEnumValues<eName__>()\
{\
   vector<eName__> retValues;\
   eAlias__##Mapping values = get##eAlias__##Mapping();\
   eAlias__##Mapping::iterator iter;\
   for (iter = values.begin(); iter != values.end(); ++iter)\
   {\
      retValues.push_back(iter->get<0>());\
   }\
   return retValues;\
}\

#define ENUM_GET_ALL_VALUES_DISPLAY_STRING(eName__, eAlias__) \
template<>\
vector<string> StringUtilities::getAllEnumValuesAsDisplayString<eName__>()\
{\
   vector<string> retValues;\
   eAlias__##Mapping values = get##eAlias__##Mapping();\
   eAlias__##Mapping::iterator iter;\
   for (iter = values.begin(); iter != values.end(); ++iter)\
   {\
      retValues.push_back(iter->get<1>());\
   }\
   return retValues;\
}\

#define ENUM_GET_ALL_VALUES_XML_STRING(eName__, eAlias__) \
template<>\
vector<string> StringUtilities::getAllEnumValuesAsXmlString<eName__>()\
{\
   vector<string> retValues;\
   eAlias__##Mapping values = get##eAlias__##Mapping();\
   eAlias__##Mapping::iterator iter;\
   for (iter = values.begin(); iter != values.end(); ++iter)\
   {\
      retValues.push_back(iter->get<2>());\
   }\
   return retValues;\
}\

#define BEGIN_ENUM_MAPPING_ALIAS(eName__, eAlias__) \
ENUM_MAPPING_PRE_DEFINITIONS(eName__, eAlias__) \
ENUM_MAPPING_TO_DISPLAY_STRING(eName__, eAlias__) \
ENUM_MAPPING_TO_DISPLAY_STRING_VEC(eName__) \
ENUM_MAPPING_TO_XML_STRING(eName__, eAlias__) \
ENUM_MAPPING_TO_XML_STRING_VEC(eName__) \
ENUM_MAPPING_FROM_DISPLAY_STRING(eName__, eAlias__) \
ENUM_MAPPING_FROM_DISPLAY_STRING_VEC(eName__) \
ENUM_MAPPING_FROM_XML_STRING(eName__, eAlias__) \
ENUM_MAPPING_FROM_XML_STRING_VEC(eName__) \
ENUM_GET_ALL_VALUES(eName__, eAlias__) \
ENUM_GET_ALL_VALUES_DISPLAY_STRING(eName__, eAlias__) \
ENUM_GET_ALL_VALUES_XML_STRING(eName__, eAlias__) \
ENUM_MAPPING_FUNCTION(eAlias__) \

#define BEGIN_ENUM_MAPPING(eName__) BEGIN_ENUM_MAPPING_ALIAS(eName__, eName__)


#define ADD_ENUM_MAPPING(enumVal, displayValue, xmlValue) values.push_back(boost::make_tuple(enumVal, displayValue, xmlValue));

#define END_ENUM_MAPPING() \
   } \
   return values; \
}

#define STRINGSTREAM_MAPPING_TO_XML_STRING(type__, __precision) \
template<>\
std::string StringUtilities::toXmlString(const type__ & val, bool* pError)\
{\
   stringstream buf;\
   if (__precision != -1)\
   {\
      buf.precision(__precision);\
   }\
   buf << val;\
   if (pError != NULL)\
   {\
      *pError = buf.fail();\
   }\
   return buf.str();\
}\

#define STRINGSTREAM_MAPPING_TO_XML_STRING_CAST(type__, cast__, precision__) \
template<>\
std::string StringUtilities::toXmlString(const type__ & val, bool* pError)\
{\
   stringstream buf;\
   if (precision__ != -1)\
   {\
      buf.precision(precision__);\
   }\
   buf << static_cast<cast__>(val);\
   if (pError != NULL)\
   {\
      *pError = buf.fail();\
   }\
   return buf.str();\
}\

#define STRINGSTREAM_MAPPING_TO_XML_STRING_VEC(type__) \
template<>\
std::string StringUtilities::toXmlString(const vector<type__> & vec, bool* pError)\
{\
   return convertVectorToString(vec, pError, ", ", false);\
}\

#define STRINGSTREAM_MAPPING_TO_DISPLAY_STRING(type__, __precision) \
template<>\
std::string StringUtilities::toDisplayString(const type__ & val, bool* pError)\
{\
   stringstream buf;\
   if (__precision != -1)\
   {\
      buf.precision(__precision);\
   }\
   buf << val;\
   if (pError != NULL)\
   {\
      *pError = buf.fail();\
   }\
   return buf.str();\
}\

#define STRINGSTREAM_MAPPING_TO_DISPLAY_STRING_CAST(type__, cast__, precision__) \
template<>\
std::string StringUtilities::toDisplayString(const type__ & val, bool* pError)\
{\
   stringstream buf;\
   if (precision__ != -1)\
   {\
      buf.precision(precision__);\
   }\
   buf << static_cast<cast__>(val);\
   if (pError != NULL)\
   {\
      *pError = buf.fail();\
   }\
   return buf.str();\
}\

#define STRINGSTREAM_MAPPING_TO_DISPLAY_STRING_VEC(type__) \
template<>\
std::string StringUtilities::toDisplayString(const vector<type__> & vec, bool* pError)\
{\
   return convertVectorToString(vec, pError, ", ", true);\
}\


#define STRINGSTREAM_MAPPING_FROM_XML_STRING(type__) \
template<>\
type__ StringUtilities::fromXmlString<type__>(std::string value, bool* pError)\
{\
   stringstream buf(value);\
   type__ parsedValue;\
   buf >> parsedValue;\
   if (pError != NULL)\
   {\
      *pError = buf.fail();\
   }\
   return parsedValue;\
}\

#define STRINGSTREAM_MAPPING_FROM_XML_STRING_CAST(type__, cast__) \
template<>\
type__ StringUtilities::fromXmlString<type__>(std::string value, bool* pError)\
{\
   stringstream buf(value);\
   cast__ parsedValue;\
   buf >> parsedValue;\
   if (pError != NULL)\
   {\
      *pError = buf.fail() || parsedValue > std::numeric_limits<type__>::max()\
         || parsedValue < std::numeric_limits<type__>::min();\
   }\
   if (parsedValue > std::numeric_limits<type__>::max())\
   {\
      return std::numeric_limits<type__>::max();\
   }\
   if (parsedValue < std::numeric_limits<type__>::min())\
   {\
      return std::numeric_limits<type__>::min();\
   }\
   return static_cast<type__>(parsedValue);\
}\

#define STRINGSTREAM_MAPPING_FROM_XML_STRING_VEC(type__) \
template<>\
vector<type__> StringUtilities::fromXmlString<vector<type__> >(std::string value, bool* pError)\
{\
   return convertStringToVector<type__>(value, pError, ", ", false);\
}\

#define STRINGSTREAM_MAPPING_FROM_DISPLAY_STRING(type__) \
template<>\
type__ StringUtilities::fromDisplayString<type__>(std::string value, bool* pError)\
{\
   stringstream buf(value);\
   type__ parsedValue;\
   buf >> parsedValue;\
   if (pError != NULL)\
   {\
      *pError = buf.fail();\
   }\
   return parsedValue;\
}\

#define STRINGSTREAM_MAPPING_FROM_DISPLAY_STRING_CAST(type__, cast__) \
template<>\
type__ StringUtilities::fromDisplayString<type__>(std::string value, bool* pError)\
{\
   stringstream buf(value);\
   cast__ parsedValue;\
   buf >> parsedValue;\
   if (pError != NULL)\
   {\
      *pError = buf.fail() || parsedValue > std::numeric_limits<type__>::max()\
         || parsedValue < std::numeric_limits<type__>::min();\
   }\
   if (parsedValue > std::numeric_limits<type__>::max())\
   {\
      return std::numeric_limits<type__>::max();\
   }\
   if (parsedValue < std::numeric_limits<type__>::min())\
   {\
      return std::numeric_limits<type__>::min();\
   }\
   return static_cast<type__>(parsedValue);\
}\

#define STRINGSTREAM_MAPPING_FROM_DISPLAY_STRING_VEC(type__) \
template<>\
vector<type__> StringUtilities::fromDisplayString<vector<type__> >(std::string value, bool* pError)\
{\
   return convertStringToVector<type__>(value, pError, ", ", true);\
}\

#define STRINGSTREAM_MAPPING_PRECISION(type__, precision__) \
STRINGSTREAM_MAPPING_TO_XML_STRING(type__, precision__) \
STRINGSTREAM_MAPPING_TO_XML_STRING_VEC(type__) \
STRINGSTREAM_MAPPING_TO_DISPLAY_STRING(type__, precision__) \
STRINGSTREAM_MAPPING_TO_DISPLAY_STRING_VEC(type__) \
STRINGSTREAM_MAPPING_FROM_XML_STRING(type__) \
STRINGSTREAM_MAPPING_FROM_XML_STRING_VEC(type__) \
STRINGSTREAM_MAPPING_FROM_DISPLAY_STRING(type__) \
STRINGSTREAM_MAPPING_FROM_DISPLAY_STRING_VEC(type__) \

#define STRINGSTREAM_MAPPING(__type) \
STRINGSTREAM_MAPPING_PRECISION(__type, -1) \

#define STRINGSTREAM_MAPPING_PRECISION_CAST(type__, cast__, precision__) \
STRINGSTREAM_MAPPING_TO_XML_STRING_CAST(type__, cast__, precision__) \
STRINGSTREAM_MAPPING_TO_XML_STRING_VEC(type__) \
STRINGSTREAM_MAPPING_TO_DISPLAY_STRING_CAST(type__, cast__, precision__) \
STRINGSTREAM_MAPPING_TO_DISPLAY_STRING_VEC(type__) \
STRINGSTREAM_MAPPING_FROM_XML_STRING_CAST(type__, cast__) \
STRINGSTREAM_MAPPING_FROM_XML_STRING_VEC(type__) \
STRINGSTREAM_MAPPING_FROM_DISPLAY_STRING_CAST(type__, cast__) \
STRINGSTREAM_MAPPING_FROM_DISPLAY_STRING_VEC(type__) \

#define STRINGSTREAM_MAPPING_CAST(__type, __cast) \
STRINGSTREAM_MAPPING_PRECISION_CAST(__type, __cast, -1)

#define TO_DISPLAY_POINTER_VARIATIONS(interface__) \
template<>\
std::string StringUtilities::toDisplayString<interface__>(const interface__ & val, bool* pError)\
{\
   return toDisplayString<const interface__ *>(&val, pError);\
}\
template<>\
std::string StringUtilities::toDisplayString<interface__ *>(interface__ * const & val, bool* pError)\
{\
   return toDisplayString<const interface__*>(val, pError);\
}\

#define TO_XML_POINTER_VARIATIONS(interface__) \
template<>\
std::string StringUtilities::toXmlString<interface__>(const interface__ & val, bool* pError)\
{\
   return toXmlString<const interface__ *>(&val, pError);\
}\
template<>\
std::string StringUtilities::toXmlString<interface__ *>(interface__ * const & val, bool* pError)\
{\
   return toXmlString<const interface__*>(val, pError);\
}

#endif
