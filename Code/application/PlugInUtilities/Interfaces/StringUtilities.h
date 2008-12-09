/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef STRINGUTILITIES_H
#define STRINGUTILITIES_H

#include "LocationType.h"

#include <stdio.h>
#include <string>
#include <vector>

namespace StringUtilities
{
   // constants for the readSTLString class
   const bool STL_APPEND = true;
   const bool STL_OVERWRITE = false;

   /**
    *  Reads readSize number of characters from pInputFile and places them in
    *  string target.
    *
    *  @param   pInputFile
    *           The file from which the string is read.
    *  @param   readSize
    *           The number of bytes to read
    *  @param   target
    *           Where to put the bytes that are read in.
    *  @param   append
    *           Whether to append to the existing string, or to overwrite it.
    *
    *  @return  Returns false if the file does not exist, or if there is a problem
    *           reading the requested bytes from the file. Otherwise, returns true.
    */
   bool readSTLString(FILE* pInputFile, size_t readSize, std::string &target, bool append = STL_OVERWRITE);

   /**
    *  Returns a copy of the string source, with all whitespace stripped from
    *  the left side.
    *
    *  @param   source
    *           The string from which the whitespace will be stripped.
    *  @return  Returns a copy of the string with whitespace removed from the left side.
    */
   std::string stripLeftWhitespace(const std::string& source);

   /**
    *  Returns a copy of the string source, with all whitespace stripped from
    *  the right side.
    *
    *  @param   source
    *           The string from which the whitespace will be stripped.
    *  @return  Returns a copy of the string with whitespace removed from the right side.
    */
   std::string stripRightWhitespace(const std::string& source);

   /**
    *  Returns a copy of the string source, with all whitespace stripped from
    *  both sides.
    *
    *  @param   source
    *           The string from which the whitespace will be stripped.
    *  @return  Returns a copy of the string with whitespace removed from both sides.
    */
   std::string stripWhitespace(const std::string& source);

   /**
    * Returns true if the string consists entirely of space characters.
    *
    * @param source
    *        The string to test
    * @return True if the string contains only space characters or is empty,
    *         false otherwise.
    */
   bool isAllBlank(const std::string &source);

   /**
    *  Converts all uppercase letters to lowercase;
    *
    *  @param   source
    *           The source string for the conversion process.
    *  @return  Returns a copy of the string with all uppercase letters converted to lowercase.
    */
   std::string toLower(const std::string& source);

   /**
    *  Converts all lowercase letters to uppercase;
    *
    *  @param   source
    *           The source string for the conversion process.
    *  @return  Returns a copy of the string with all lowercase letters converted to uppercase.
    */
   std::string toUpper(const std::string& source);

   /**
    *  Breaks up a string into a set of substrings.
    *
    *  This method breaks up a delimited string into its component substrings.
    *  @code
    *  string text("abc/123/def/456");
    *  vector<string> components = split(text, '/');
    *  // components has 4 strings: "abc", "123", "def", "456"
    *  @endcode
    *
    *  @param   source
    *           The string to be split up.
    *
    *  @param   separator
    *           This character will be used as a delimiter to break the source
    *           string up into substrings.
    *
    *  @return  Returns a vector containing the component strings.
    */
   std::vector<std::string> split(const std::string &source, char separator);

   /**
    *  Joins a vector of strings into a single string.
    *
    *  @param source
    *         The strings to join.
    *
    *  @param separator
    *         The string to insert between each source string.
    *
    *  @return The joined string.
    */
   std::string join(const std::vector<std::string> &source, const std::string &separator);

   /**
    *  Converts a LocationType whose values are in D.ddddd Lat/Lon format to a string.
    *
    *  @param   latLonCoords
    *           The source LocationType to be converted
    *  @return  Returns a string displaying the Latitude/Longitude represented by the LocationType passed in.
    */
   std::string latLonToText(LocationType latLonCoords);

   /**
    * Expand the variables in this string and return the expanded result.
    *
    * The syntax for referencing variables is $V(varname), $E(varname) or $C(varname).
    * The $V(varname) syntax only recognizes the following two variables names,
    * APP_HOME or APP_VERSION which be expanded to the location where this application
    * was installed to and the version of this application that is running respectively.
    * For example, $V(APP_HOME) would expand to "C:\Program Files\Application\4.0.0" and 
    * $V(APP_VERSION) would expand to "4.0.0".  The $E(varname) syntax looks for
    * an environment variable defined with the same name and if found substitute's in the value.
    * If the environment variable cannot be found, the $E(varname) is left in the string.
    * The $C(varname) syntax looks for a setting in ConfigurationSettings with the same
    * name and if found substitute's in the value.  If the setting cannot be found, the
    * $C(varname) is left in the string.  If you need to include a $ in the string, please
    * use $$ to escape the $.
    *
    * @param originalString
    *        The string that should be expanded.
    * @param ignoredExpansions
    *        The list of variable expansions that should be ignored.  For example,
    *        providing a vector with 'E' and 'C' in it, would cause $E(varname) and
    *        $C(varname) to be left in the string as is (i.e. no expansion would occur).
    *
    * @return the expanded string.
    */
   std::string expandVariables(const std::string& originalString,
      const std::vector<std::string>& ignoredExpansions = std::vector<std::string>());

   /**
    * Convert the given value into a string.  The
    * resulting string will be in a XML-friendly representation.  In
    * addition, the string representation of the value returned by this
    * method should be subject to less change than the value returned from
    * toDisplayString.
    *
    * @param value
    *        The value to convert into a string.  For a list of types
    *        supported, see \ref stringutilities_types.
    *
    * @param pError
    *        If this value is non-NULL, the bool will be set to true
    *        if there was an error while converting the value into
    *        a string, and false if the conversion was successful.
    *
    * @return Returns the string representation of the given value. If there
    *         was an error, an empty string will be returned.
    */
   template<typename T>
   std::string toXmlString(const T& value, bool* pError = NULL);

   /**
    * Parse the given string into the given type. This method
    * should only be used to parse string values that were returned
    * from toXmlString.  For a list of types supported see
    * \ref stringutilities_types.
    *
    * @param value
    *        The string to parse.
    *
    * @param pError
    *        If this value is non-NULL, the bool will be set to
    *        true if there was an error while parsing the string,
    *        and false if the parse was successful.
    * 
    * @return Returns the parsed value of the given type. If there
    *         was an error, a default constructed value of the 
    *         given type will be returned.  If the type is a pointer
    *         then NULL will be returned.
    */
   template<typename T>
   T fromXmlString(std::string value, bool* pError = NULL);

   /**
    * Convert the given value into a string.  The
    * resulting string will be in a user-friendly representation.
    * The resulting string is intended to be used when the value
    * needs to be converted into a string in order to display to a
    * end-user.  The resulting string should not be expected to
    * remain the same between different versions of the application.
    *
    * @param value
    *        The value to convert into a string.  For a list of types
    *        supported, see \ref stringutilities_types.
    *
    * @param pError
    *        If this value is non-NULL, the bool will be set to true
    *        if there was an error while converting the value into
    *        a string, and false if the conversion was successful.
    *
    * @return Returns the string representation of the given value. If there
    *         was an error, an empty string will be returned.
    */
   template<typename T>
   std::string toDisplayString(const T& value, bool* pError = NULL);

   /**
    * Parse the given string into the given type. This method
    * should only be used to parse string values that were returned
    * from toDisplayString.  For a list of types supported see
    * \ref stringutilities_types.
    *
    * @param value
    *        The string to parse.
    *
    * @param pError
    *        If this value is non-NULL, the bool will be set to
    *        true if there was an error while parsing the string,
    *        and false if the parse was successful.
    * 
    * @return Returns the parsed value of the given type. If there
    *         was an error, a default constructed value of the 
    *         given type will be returned.  If the type is a pointer
    *         then NULL will be returned.
    */
   template<typename T>
   T fromDisplayString(std::string value, bool* pError = NULL);

   /**
    * Provides a list of all enum values for the given enum provided
    * to the template. This method is only supported by the enum types
    * listed in \ref stringutilities_types.
    *
    * @return Returns a list all enum values for the given enum type.
    *
    * @see getAllEnumValuesAsDisplayString(), getAllEnumValuesAsXmlString()
    */
   template<typename T>
   std::vector<T> getAllEnumValues();

   /**
    * Provides a list of display strings for all of the enum values for
    * the given enum type. This method is only supported by the enum types
    * listed in \ref stringutilities_types.
    *
    * @return Returns the result of calling toDisplayString() on every value
    *         in the vector that would be returned by getAllEnumValues(). The
    *         ordering of the vectors returned by this method and getAllEnumValues()
    *         is guaranteed to be the same.
    *
    * @see getAllEnumValues(), getAllEnumValuesAsXmlString()
    */
   template<typename T>
   std::vector<std::string> getAllEnumValuesAsDisplayString();

   /**
    * Provides a list of xml strings for all of the enum values for
    * the given enum type. This method is only supported by the enum types
    * listed in \ref stringutilities_types.
    *
    * @return Returns the result of calling toXmlString() on every value
    *         in the vector that would be returned by getAllEnumValues(). The
    *         ordering of the vectors returned by this method and getAllEnumValues()
    *         is guaranteed to be the same.
    *
    * @see getAllEnumValues(), getAllEnumValuesAsDisplayString()
    */
   template<typename T>
   std::vector<std::string> getAllEnumValuesAsXmlString();

   /**
    * Tokenizes string on spaces, with escape support.
    *
    * This function will start tokenizing a string provided by textIn,
    * or continue tokenizing if the default parameter is used.  Whitespace
    * to the left and right of the token is removed.  Characters
    * may be escaped with the '\\' character.  The following escape codes
    * are understood:
    *
    * - '\\' -- gives a single backslash.
    * - ' ' -- gives a single space character.
    *
    * This function is not thread safe.
    *
    * @param textIn
    *        Input string, of which a local copy is made.  Tokenizing starts
    *        from the beginning of the string, or where left off if the default
    *        parameter is used.
    *
    * @return Returns the first token, with escaped text replaced as appropriate.
    *         This method owns the memory returned.
    */
   const char* escapedToken(const std::string &textIn = std::string());
}

#endif
