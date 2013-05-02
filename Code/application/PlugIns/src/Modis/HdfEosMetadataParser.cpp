/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "DataVariant.h"
#include "DynamicObject.h"
#include "HdfEosMetadataParser.h"
#include "ObjectResource.h"
#include "StringUtilities.h"
#include "StringUtilitiesMacros.h"

#include <algorithm>
#include <stack>

#define KEY_VALUE_SEPARATOR   '='
#define BEGIN_PARENTHESIS     '('
#define END_PARENTHESIS       ')'
#define BEGIN_QUOTATION       '\"'
#define END_QUOTATION         '\"'
#define EXPONENTIAL_MARK      'E'
#define DECIMAL_POINT         '.'
#define NEWLINE               '\n'
#define DIGITS                "0123456789"

const std::string WHITESPACE = " \t\r\n";

namespace StringUtilities
{
   BEGIN_ENUM_MAPPING_ALIAS(HdfEosMetadataParser::HdfEosDataType, HdfEosDataType)
      ADD_ENUM_MAPPING(HdfEosMetadataParser::HDF_EOS_VECTOR_STRING, "vector<string>", "vector&lt;string&gt;")
      ADD_ENUM_MAPPING(HdfEosMetadataParser::HDF_EOS_VECTOR_DOUBLE, "vector<double>", "vector&lt;double&gt;")
      ADD_ENUM_MAPPING(HdfEosMetadataParser::HDF_EOS_VECTOR_FLOAT, "vector<float>", "vector&lt;float&gt;")
      ADD_ENUM_MAPPING(HdfEosMetadataParser::HDF_EOS_VECTOR_INT, "vector<int>", "vector&lt;int&gt;")
      ADD_ENUM_MAPPING(HdfEosMetadataParser::HDF_EOS_STRING, "string", "string")
      ADD_ENUM_MAPPING(HdfEosMetadataParser::HDF_EOS_DOUBLE, "double", "double")
      ADD_ENUM_MAPPING(HdfEosMetadataParser::HDF_EOS_FLOAT, "float", "float")
      ADD_ENUM_MAPPING(HdfEosMetadataParser::HDF_EOS_INT, "int", "int")
   END_ENUM_MAPPING()
}

HdfEosMetadataParser::HdfEosMetadataParser() :
   mStringBuffer(std::string()),
   mDoubleBuffer(-1.0),
   mFloatBuffer(-1.0f),
   mIntBuffer(-1)
{
   // Initialize associations of data types to buffer addresses
   mBufferAddresses.clear();
   mBufferAddresses.insert(std::pair<HdfEosDataType, void* const>(HDF_EOS_VECTOR_STRING, &mStringsBuffer));
   mBufferAddresses.insert(std::pair<HdfEosDataType, void* const>(HDF_EOS_VECTOR_DOUBLE, &mDoublesBuffer));
   mBufferAddresses.insert(std::pair<HdfEosDataType, void* const>(HDF_EOS_VECTOR_FLOAT, &mFloatsBuffer));
   mBufferAddresses.insert(std::pair<HdfEosDataType, void* const>(HDF_EOS_VECTOR_INT, &mIntsBuffer));
   mBufferAddresses.insert(std::pair<HdfEosDataType, void* const>(HDF_EOS_STRING, &mStringBuffer));
   mBufferAddresses.insert(std::pair<HdfEosDataType, void* const>(HDF_EOS_DOUBLE, &mDoubleBuffer));
   mBufferAddresses.insert(std::pair<HdfEosDataType, void* const>(HDF_EOS_FLOAT, &mFloatBuffer));
   mBufferAddresses.insert(std::pair<HdfEosDataType, void* const>(HDF_EOS_INT, &mIntBuffer));

   // Initialize associations of datatypes to format strings
   mFormats.clear();
   mFormats.insert(std::pair<HdfEosDataType, std::string>(HDF_EOS_VECTOR_STRING, "%s"));
   mFormats.insert(std::pair<HdfEosDataType, std::string>(HDF_EOS_VECTOR_DOUBLE, "%lg"));
   mFormats.insert(std::pair<HdfEosDataType, std::string>(HDF_EOS_VECTOR_FLOAT, "%f"));
   mFormats.insert(std::pair<HdfEosDataType, std::string>(HDF_EOS_VECTOR_INT, "%d"));
   mFormats.insert(std::pair<HdfEosDataType, std::string>(HDF_EOS_STRING, "%s"));
   mFormats.insert(std::pair<HdfEosDataType, std::string>(HDF_EOS_DOUBLE, "%lg"));
   mFormats.insert(std::pair<HdfEosDataType, std::string>(HDF_EOS_FLOAT, "%f"));
   mFormats.insert(std::pair<HdfEosDataType, std::string>(HDF_EOS_INT, "%d"));

   // Initialize lists of legal data types
   mLegalVectorNumberTypes.clear();
   mLegalVectorNumberTypes.push_back(HDF_EOS_VECTOR_DOUBLE);
   mLegalVectorNumberTypes.push_back(HDF_EOS_VECTOR_FLOAT);
   mLegalVectorNumberTypes.push_back(HDF_EOS_VECTOR_INT);

   mLegalBasicNumberTypes.clear();
   mLegalBasicNumberTypes.push_back(HDF_EOS_DOUBLE);
   mLegalBasicNumberTypes.push_back(HDF_EOS_FLOAT);
   mLegalBasicNumberTypes.push_back(HDF_EOS_INT);

   mLegalVectorTypes = mLegalVectorNumberTypes;
   mLegalVectorTypes.push_back(HDF_EOS_VECTOR_STRING);

   mLegalBasicTypes = mLegalBasicNumberTypes;
   mLegalBasicTypes.push_back(HDF_EOS_STRING);

   // Initialize buffers
   mStringsBuffer.clear();
   mDoublesBuffer.clear();
   mFloatsBuffer.clear();
   mIntsBuffer.clear();
}

HdfEosMetadataParser::~HdfEosMetadataParser()
{}

bool HdfEosMetadataParser::convert(DynamicObject* pMetadata, const std::string& metadataText)
{
   if ((pMetadata == NULL) || (metadataText.empty() == true))
   {
      return false;
   }

   // Create a DynamicObject stack containing the attributes to add instead of setting the attributes
   // values by path so that attribute groups containing no attributes will still be added
   std::stack<DynamicObject*> objects;
   objects.push(pMetadata);

   // Iterate over all key-value pairs contained in the string adding the attributes to the DynamicObject
   std::stack<std::string> classText;
   bool valueKeySet = false;
   bool valueList = false;
   std::string valueText;

   std::string textBlock = metadataText;
   while (textBlock.empty() == false)
   {
      // Extract the next key-value pair from the metadata string
      std::string key;
      std::string value;
      bool isList = false;
      if (getNextKeyValuePair(textBlock, key, value, isList) == false)
      {
         break;
      }

      // Check if a child DynamicObject should be created or removed
      if ((key == "GROUP") || (key == "OBJECT"))
      {
         // Create a new DynamicObject and add it to the stack
         FactoryResource<DynamicObject> pChild;
         objects.push(pChild.release());
      }
      else if ((key == "END_GROUP") || (key == "END_OBJECT"))
      {
         if (classText.empty() == false)
         {
            value += classText.top();
            classText.pop();
         }

         if (valueKeySet == false)
         {
            // The group is complete, so add the entire group to the parent DynamicObject
            DynamicObject* pChild = objects.top();
            VERIFY(pChild != NULL);

            objects.pop();
            VERIFY(objects.empty() == false);

            DynamicObject* pParent = objects.top();
            VERIFY(pParent != NULL);

            DataVariant groupAttribute(*pChild);
            pParent->adoptAttribute(value, groupAttribute);    // Ownership of the child dynamic object is
                                                               // transferred to the parent dynamic object
         }
         else
         {
            // Add the value as an individual attribute to the current DynamicObject
            DynamicObject* pParent = objects.top();
            addAttribute(pParent, value, valueText, valueList);

            // Clear the value
            valueKeySet = false;
            valueList = false;
            valueText.clear();
         }
      }
      else if (key == "CLASS")
      {
         classText.push("." + value);
      }
      else if (key == "VALUE")
      {
         // This value represents the sole content of the current group, so remove (and delete) the
         // group from the stack and set the variable to add the value as an individual attribute
         FactoryResource<DynamicObject> pChild(objects.top());
         objects.pop();

         valueKeySet = true;
         valueList = isList;
         valueText = value;
      }
      else if ((key != "GROUPTYPE") && (key != "END_GROUPTYPE") && (key != "NUM_VAL"))
      {
         // Add an individual attribute to the current DynamicObject
         DynamicObject* pParent = objects.top();
         addAttribute(pParent, key, value, isList);
      }
   }

   return true;
}

bool HdfEosMetadataParser::getNextKeyValuePair(std::string& metadataText, std::string& key, std::string& value,
                                               bool& isList) const
{
   // A key-value pair must contain at least three characters (e.g. "a=1")
   if (metadataText.size() <= 2)
   {
      return false;
   }

   // Check for the presence of a key-value pair based on the separator
   std::string::size_type startIdx = 0;
   std::string::size_type endIdx = metadataText.find(KEY_VALUE_SEPARATOR, 0);
   if (endIdx == std::string::npos)
   {
      return false;
   }

   // Assign the key value
   key = StringUtilities::stripWhitespace(metadataText.substr(startIdx, endIdx - startIdx));

   // Remove the key and separator from the string
   metadataText.erase(startIdx, endIdx - startIdx + 1);

   // Check if there is no value present
   startIdx = metadataText.find_first_not_of(WHITESPACE);
   if (startIdx == std::string::npos)
   {
      // Clear the value, and point to the end of the string
      value.clear();
      endIdx = metadataText.size() - 1;
   }
   else  // There is a value present
   {
      // Check if the value begins with a parenthesis
      if (metadataText[startIdx] == BEGIN_PARENTHESIS)
      {
         // A parenthesis signifies a list
         isList = true;

         // The value ends with a parenthesis
         VERIFY(metadataText.size() > startIdx + 1);

         endIdx = metadataText.find(END_PARENTHESIS, startIdx + 1);
         if (endIdx != std::string::npos)
         {
            value = metadataText.substr(startIdx + 1, (endIdx - 1) - (startIdx + 1) + 1);
         }
         else
         {
            return false;
         }
      }
      // Check if the value begins with a quotation mark
      else if (metadataText[startIdx] == BEGIN_QUOTATION)
      {
         // No parenthesis, so the value is not a list of values
         isList = false;

         // The value ends with a quotation mark
         VERIFY(metadataText.size() > startIdx + 1);

         endIdx = metadataText.find(END_QUOTATION, startIdx + 1);
         if (endIdx != std::string::npos)
         {
            value = metadataText.substr(startIdx + 1, (endIdx - 1) - (startIdx + 1) + 1);
         }
         else
         {
            return false;
         }
      }
      else
      {
         // No parenthesis, so the value is not a list of values
         isList = false;

         //  The value ends with whitespace
         VERIFY(metadataText.size() > startIdx + 1);

         endIdx = metadataText.find_first_of(WHITESPACE, startIdx + 1);
         if (endIdx != std::string::npos)
         {
            value = metadataText.substr(startIdx, endIdx - startIdx + 1);
         }
         else
         {
            value.clear();
            endIdx = metadataText.size() - 1;
         }
      }
   }

   // Remove unneccessary whitespace
   value = StringUtilities::stripWhitespace(value);
   removeNewlineInitiatedWhitespace(value);

   // Remove the value
   metadataText.erase(0, endIdx + 1);

   return true;
}

bool HdfEosMetadataParser::addAttribute(DynamicObject* pDynamicObject, const std::string& dataName,
                                        const std::string& dataValue, bool isList)
{
   if ((pDynamicObject == NULL) || (dataName.empty() == true))
   {
      return false;
   }

   HdfEosDataType dataType;
   const void* pBuffer = NULL;

   if (isList)    // The string value represents a list of data
   {
      // Parse the string value into a vector to determine the data type
      std::vector<std::string> values = convertStringToVector<std::string>(dataValue, NULL, ",", true);
      if (values.empty() == true)
      {
         return false;
      }

      for (std::vector<std::string>::iterator iter = values.begin(); iter != values.end(); ++iter)
      {
         *iter = StringUtilities::stripWhitespace(*iter);
      }

      dataType = determineType(values);
      pBuffer = parseIntoAppropriateBuffer(values, dataType);
   }
   else  // The string value represents a single data value
   {
      dataType = determineType(dataValue);
      pBuffer = parseIntoAppropriateBuffer(dataValue, dataType);
   }

   // Add the attribute to the dynamic object
   if ((dataType.isValid() == true) && (pBuffer != NULL))
   {
      DataVariant attr(StringUtilities::toDisplayString(dataType), pBuffer);
      return pDynamicObject->adoptAttributeByPath(dataName, attr);
   }

   return false;
}

HdfEosMetadataParser::HdfEosDataType HdfEosMetadataParser::determineType(const std::vector<std::string>& values) const
{
   if (values.empty() == true)
   {
      return HdfEosDataType();
   }

   HdfEosDataType dataType;
   for (std::vector<std::string>::const_iterator iter = values.begin(); iter != values.end(); ++iter)
   {
      std::string value = *iter;
      if (value.empty() == false)
      {
         HdfEosDataType currentDataType = determineType(value);
         if (currentDataType == dataType)
         {
            continue;
         }

         if (dataType.isValid() == true)
         {
            return HdfEosDataType();
         }

         dataType = currentDataType;
      }
   }

   // Convert the individual value type into a vector type
   if (dataType.isValid() == true)
   {
      std::string vectorTypeName = "vector<" + StringUtilities::toDisplayString(dataType) + ">";
      HdfEosDataType vectorDataType = StringUtilities::fromDisplayString<HdfEosDataType>(vectorTypeName);
      VERIFYRV(contains(mLegalVectorTypes, vectorDataType) == true, HdfEosDataType());
      return vectorDataType;
   }

   return HdfEosDataType();
}

HdfEosMetadataParser::HdfEosDataType HdfEosMetadataParser::determineType(const std::string& value) const
{
   HdfEosDataType dataType;
   if (value.empty() == true)
   {
      // If the value is empty, treat the value as a string
      dataType = HDF_EOS_STRING;
   }
   else
   {
      // The first character shall not be whitespace
      if (WHITESPACE.find(value[0]) != std::string::npos)
      {
         return HdfEosDataType();
      }

      // Check if a sign is present
      std::string::size_type idx = 0;
      if (value[0] == '-' || value[0] == '+')
      {
         idx = 1;
      }

      // Check if non-number characters are present in the string
      std::string searchValues = std::string(DIGITS);
      searchValues += EXPONENTIAL_MARK;
      searchValues += DECIMAL_POINT;

      idx = value.find_first_not_of(searchValues, idx);
      if (idx != std::string::npos)
      {
         dataType = HDF_EOS_STRING;
      }
      else  // No non-number characters are present
      {
         // Check if an exponential character is present
         idx = value.find(EXPONENTIAL_MARK, 0);
         if (idx != std::string::npos)
         {
            // Check if ONLY ONE exponential character is present
            idx = value.find(EXPONENTIAL_MARK, idx + 1);
            if (idx == std::string::npos)
            {
               // Check if a decimal character is present, after the exponential character
               idx = value.find(DECIMAL_POINT, idx + 1);
               if (idx != std::string::npos)
               {
                  // Check if ONLY ONE decimal character is present
                  idx = value.find(DECIMAL_POINT, idx + 1);
                  if (idx == std::string::npos)
                  {
                     dataType = HDF_EOS_DOUBLE;
                  }
                  else  // MORE THAN ONE decimal character is present
                  {
                     dataType = HDF_EOS_STRING;
                  }
               }
               else  // No decimal characters are present
               {
                  dataType = HDF_EOS_STRING;
               }
            }
            else  //  MORE THAN ONE exponential character is present
            {
               dataType = HDF_EOS_STRING;
            }
         }
         else  // No exponential characters are present
         {
            // Check if a decimal character is present
            idx = value.find(DECIMAL_POINT, 0);
            if (idx != std::string::npos)
            {
               // Check if ONLY ONE decimal character is present
               if (value.find(DECIMAL_POINT, idx + 1) == std::string::npos)
               {
                  // If the number of digits after the decimal is greater than 2, create a double;
                  // otherwise create a float
                  std::string::size_type numDigits = value.length() - idx - 1;
                  if (numDigits > 2)
                  {
                     dataType = HDF_EOS_DOUBLE;
                  }
                  else
                  {
                     dataType = HDF_EOS_FLOAT;
                  }
               }
               else  // MORE THAN ONE decimal character is present
               {
                  dataType = HDF_EOS_STRING;
               }
            }
            else  // No decimal characters are present
            {
               dataType = HDF_EOS_INT;
            }
         }
      }
   }

   VERIFYRV(contains(mLegalBasicTypes, dataType), HdfEosDataType());
   return dataType;
}

const void* const HdfEosMetadataParser::parseIntoAppropriateBuffer(const std::vector<std::string>& values,
                                                                   HdfEosDataType dataType)
{
   if ((values.empty() == true) || (contains(mLegalVectorTypes, dataType) == false))
   {
      return NULL;
   }

   // Determine what type of vector to pass to the conversion function
   const void* pConvertedValues = NULL;
   if (dataType == HDF_EOS_VECTOR_DOUBLE)
   {
      std::vector<double>* pDoubles = NULL;
      parseIntoAppropriateNumberBuffer(values, dataType, pDoubles);
      pConvertedValues = pDoubles;
   }
   else if (dataType == HDF_EOS_VECTOR_FLOAT)
   {
      std::vector<float>* pFloats = NULL;
      parseIntoAppropriateNumberBuffer(values, dataType, pFloats);
      pConvertedValues = pFloats;
   }
   else if (dataType == HDF_EOS_VECTOR_INT)
   {
      std::vector<int>* pInts = NULL;
      parseIntoAppropriateNumberBuffer(values, dataType, pInts);
      pConvertedValues = pInts;
   }
   else if (dataType == HDF_EOS_VECTOR_STRING)
   {
      mStringsBuffer = values;
      pConvertedValues = &mStringsBuffer;
   }

   return pConvertedValues;
}

const void* const HdfEosMetadataParser::parseIntoAppropriateBuffer(const std::string& value, HdfEosDataType dataType)
{
   if (contains(mLegalBasicTypes, dataType) == false)
   {
      return NULL;
   }

   // Determine the appropriate format string
   std::string format = mFormats[dataType];
   VERIFYRV(format.empty() == false, NULL);

   // Determine the correct buffer in which to store the data
   void* pConvertedValue = mBufferAddresses[dataType];
   VERIFYRV(pConvertedValue != NULL, NULL);

   // Check if the value is a string
   if (dataType == HDF_EOS_STRING)
   {
      // Copy the string directly to the buffer
      *(reinterpret_cast<std::string*>(pConvertedValue)) = value;
   }
   else  // It is a number
   {
      // Convert the string value to the appropriate number type and copy it to the buffer
      sscanf(value.c_str(), format.c_str(), pConvertedValue);
   }

   // Return a pointer to the converted value
   return pConvertedValue;
}

template <class Type>
void HdfEosMetadataParser::parseIntoAppropriateNumberBuffer(const std::vector<std::string>& values,
                                                            HdfEosDataType dataType,
                                                            std::vector<Type>*& pConvertedValues)
{
   if ((values.empty() == true) || (contains(mLegalVectorNumberTypes, dataType) == false))
   {
      return;
   }

   // Determine the appropriate format string
   string format = mFormats[dataType];
   VERIFYNRV(format.empty() == false);

   // Determine the correct buffer in which to store the data
   pConvertedValues = reinterpret_cast<std::vector<Type>*>(mBufferAddresses[dataType]);
   VERIFYNRV(pConvertedValues != NULL);

   // Initialize the vector
   pConvertedValues->clear();

   Type convertedValue;
   for (std::vector<string>::const_iterator valuesIter = values.begin(); valuesIter != values.end(); ++valuesIter)
   {
      std::string value = *valuesIter;
      VERIFYNRV(value.empty() == false);

      // Convert the string value to the appropriate number type
      convertedValue = NULL;
      sscanf(value.c_str(), format.c_str(), &convertedValue);
      VERIFYNRV(convertedValue != NULL);
      pConvertedValues->push_back(convertedValue);
   }
}

void HdfEosMetadataParser::removeNewlineInitiatedWhitespace(std::string& str) const
{
   // Remove the leading whitespace for each line of text contained in the string
   std::string::size_type idx = str.find(NEWLINE);
   while (idx != std::string::npos)
   {
      std::string::size_type endIdx = str.find_first_not_of(WHITESPACE, idx);
      if (endIdx != std::string::npos)
      {
         str.erase(idx, endIdx - idx);
      }

      idx = str.find(NEWLINE, idx);
   }
}

template <class Type>
bool HdfEosMetadataParser::contains(const std::vector<Type>& values, const Type& value) const
{
   if (std::find(values.begin(), values.end(), value) == values.end())
   {
      return false;
   }

   return true;
}
