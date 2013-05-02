/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef HDFEOSMETADATAPARSER_H
#define HDFEOSMETADATAPARSER_H

#include "EnumWrapper.h"

#include <map>
#include <string>
#include <vector>

class DynamicObject;

/**
 *  Converts string representation of HDF EOS metadata into a DynamicObject
 *  representation.
 *
 *  @see     DynamicObject
 */
class HdfEosMetadataParser
{
public:
   HdfEosMetadataParser();
   ~HdfEosMetadataParser();

   enum HdfEosDataTypeEnum
   {
      HDF_EOS_VECTOR_STRING,
      HDF_EOS_VECTOR_DOUBLE,
      HDF_EOS_VECTOR_FLOAT,
      HDF_EOS_VECTOR_INT,
      HDF_EOS_STRING,
      HDF_EOS_DOUBLE,
      HDF_EOS_FLOAT,
      HDF_EOS_INT
   };

   typedef EnumWrapper<HdfEosDataTypeEnum> HdfEosDataType;

   /**
    *  Populates a DynamicObject with attributes obtained from an HDF EOS
    *  metadata string.
    *
    *  @param   pMetadata
    *           A pointer to the DynamicObject that should contain the metadata
    *           attributes.  This method does nothing and returns \c false if
    *           \c NULL is passed in.
    *  @param   metadataText
    *           The string representation of an HDF EOS metadata value
    *           containing attributes that should be added to \em pMetadata.
    *           This method does nothing and returns \c false if an empty string
    *           is passed in.
    *
    *  @return  Returns \c true if attributes were successfully added to the
    *           DynamicObject from the metadata string; otherwise returns
    *           \c false.
    */
   bool convert(DynamicObject* pMetadata, const std::string& metadataText);

protected:
   /**
    *  Retrieves a key-value pair from metadata text.
    *
    *  This method finds and removes the next key-value pair from a string
    *  representation of the HDF EOS metadata.  This method also determines
    *  whether or not the value represents a list of multiple values.
    *
    *  @param   metadataText
    *           On input, \em metadataText contains a string representation of
    *           HDF EOS metadata.  On return, \em textBlock will contain the
    *           same data as when input, but will no longer contain the
    *           key-value pair that was parsed.  For example, if five key-value
    *           pairs exist in \em metadataText on input, four key-value pairs
    *           will remain in \em metadataText on return.
    *  @param   key
    *           On input, \em key is ignored.  On return \em key will contain a
    *           string representation of the next attribute key parsed from
    *           \em metadataText.
    *  @param   value
    *           On input, \em value is ignored.  On return \em value will
    *           contain a string representation of the next attribute value
    *           parsed from \em metadataText.
    *  @param   isList
    *           On input, \em isList is ignored.  On return \em isList specifies
    *           whether or not \em value actually represents a list of multiple
    *           values.
    *
    *  @return  Returns \c true if a key-value pair was successfully extracted
    *           from the given string; otherwise returns \c false.
    */
   bool getNextKeyValuePair(std::string& metadataText, std::string& key, std::string& value, bool& isList) const;

   /**
    *  Adds an attribute key-value pair to a DynamicObject.
    *
    *  This method adds a string representation of a HDF EOS metatdata
    *  key-value pair to the specified DynamicObject.
    *
    *  @param   pDynamicObject
    *           A pointer to a valid DynamicObject in which to add the
    *           attribute.  This method does nothing and returns \c false if
    *           \c NULL is passed in.
    *  @param   dataName
    *           The name of the attribute to add to the DynamicObject.  This
    *           method does nothing and returns \c false if an empty string is
    *           passed in.
    *  @param   dataValue
    *           The string representation of the attribute value to add to the
    *           DynamicObject.
    *  @param   isList
    *           Specifies whether the attribute to be added contains a list of
    *           sub-values.
    *
    *  @return  Returns \c true if the attribute was successfully added to the
    *           DynamicObject; otherwise returns \c false.
    */
   bool addAttribute(DynamicObject* pDynamicObject, const std::string& dataName, const std::string& dataValue,
      bool isList);

   /**
    *  Determines the data type of a value.
    *
    *  This method analyzes the string representation of a collection of data
    *  values, to determine what type of data the collection is representing.
    *
    *  @param   values
    *           On input, \em values holds the string representation of a
    *           collection of data values.
    *
    *  @return  Returns the data type specifying what type of data the
    *           collection of data values represents.
    */
   HdfEosDataType determineType(const std::vector<std::string>& values) const;

   /**
    *  Determines the data type of a value.
    *
    *  This method analyzes the string representation of data value, to
    *  determine what type of data the value represents.
    *
    *  @param   value
    *           On input, \em value holds the string representation of a
    *           data value.
    *
    *  @return  Returns the data type specifying what type of data the data
    *           value represents.
    */
   HdfEosDataType determineType(const std::string& value) const;

   /**
    *  Parses the string representation into the appropriate buffer.
    *
    *  This method converts the string representation of data, into the
    *  specified type, and stores the result in the appropriate buffer.
    *
    *  @param   values
    *           The collection of values, in string representation, to be
    *           converted.
    *  @param   dataType
    *           The type of data to convert to.
    *
    *  @return  Returns a void pointer to a collection of the specified type.
    *           The collection will contain the converted values.  Subsequent
    *           calls to this method destroys the results of previous calls to
    *           this method.
    */
   const void* const parseIntoAppropriateBuffer(const std::vector<std::string>& values, HdfEosDataType dataType);

   /**
    *  Parses the string representation into the appropriate buffer.
    *
    *  This method converts the string representation of data, into the
    *  specified type, and stores in the result in the appropriate buffer.
    *
    *  @param   value
    *           The string representation of the data to be converted.
    *  @param   dataType
    *           The type of data to convert to.
    *
    *  @return  Returns a void pointer to a the converted value.  The converted
    *           value is stored in a class member buffer variable.  Subsequent
    *           calls to this method destroy the results of previous calls to
    *           this method.
    */
   const void* const parseIntoAppropriateBuffer(const std::string& value, HdfEosDataType dataType);

   /**
    *  Parses the string representation into the appropriate number buffer.
    *
    *  This method converts the string representation of data, into the
    *  specified number type, and stores the result in the appropriate number
    *  buffer.
    *
    *  Subsequent calls to this method destroys the results of previous calls
    *  to this method.
    *
    *  @param   values
    *           The string representation of the data to be converted.
    *  @param   dataType
    *           The type of data to convert to.
    *  @param   pConvertedValues
    *           On return, \em pConvertedValues will point to the string
    *           representation of the converted data, overwritting any previous
    *           conversions that were performed.
    */
   template <class Type>
   void parseIntoAppropriateNumberBuffer(const std::vector<std::string>& values, HdfEosDataType dataType,
      std::vector<Type>*& pConvertedValues);

   /**
    *  Removes newline initiated whitespace from text blocks.
    *
    *  This method removes all white space between newline characters and other
    *  non-whitespace characters.
    *
    *  @param   str
    *           On input, \em str is a block of text.  On return, \em str is the
    *           same block of text, but without any whitespace between newline
    *           characters and other non-whitespace characters.
    */
   void removeNewlineInitiatedWhitespace(std::string& str) const;

   /**
    *  Checks to see if a value is contained within a vector.
    *
    *  This method will determine whether or not a value is contained within a
    *  vector.
    *
    *  @param   values
    *           The vector of values to search.
    *  @param   value
    *           The specific value for which the vector will be searched.
    *
    *  @return  Returns \c true if the given value is contained within the given
    *           vector; otherwise returns \c false.
    */
   template <class Type>
   bool contains(const std::vector<Type>& values, const Type& value) const;

private:
   HdfEosMetadataParser(const HdfEosMetadataParser& rhs);
   HdfEosMetadataParser& operator=(const HdfEosMetadataParser& rhs);

   std::map<HdfEosDataType, void* const> mBufferAddresses;
   std::map<HdfEosDataType, std::string> mFormats;

   std::vector<HdfEosDataType> mLegalVectorNumberTypes;
   std::vector<HdfEosDataType> mLegalBasicNumberTypes;
   std::vector<HdfEosDataType> mLegalVectorTypes;
   std::vector<HdfEosDataType> mLegalBasicTypes;

   std::vector<std::string> mStringsBuffer;
   std::vector<double> mDoublesBuffer;
   std::vector<float> mFloatsBuffer;
   std::vector<int> mIntsBuffer;
   std::string mStringBuffer;
   double mDoubleBuffer;
   float mFloatBuffer;
   int mIntBuffer;
};

#endif
