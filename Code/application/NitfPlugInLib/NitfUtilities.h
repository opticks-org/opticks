/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef NITF_UTILITIES_H
#define NITF_UTILITIES_H

class Classification;
class DateTime;
#include "DataVariant.h"
#include "DynamicObject.h"
#include "NitfConstants.h"
#include "NitfTreParser.h"
#include "TypesFile.h"
#include "StringUtilities.h"

#include <iomanip>
#include <iostream>
#include <limits>
#include <set>
#include <string>
#include <vector>

#include <boost/lexical_cast.hpp>

namespace Nitf
{   
   /**
    * Determine the least trusted state of the provided TreStates.
    *
    * @param stateA
    *        The first state to compare
    * @param stateB
    *        The second state to compare
    *
    * @return The least trusted of the provided states.
    */
   TreState MaxState(TreState stateA, TreState stateB);

   /**
    * Represents getting the first instance of a tag.
    */
   struct FindFirst
   {
      bool operator()(const DynamicObject& instance)
      {
         return true;
      }
   };

   /**
    * Gets a handle to a TRE out of the NITF Metadata.
    *
    * You can provide a functor to filter out which TRE to fetch (ie. find the first
    * ICHIPB with an OP_COL_11 field that is valid). Example:
    * @code
    * // Get the first instance of the RPC00B TRE; assumes nitfMetadata is the NITF metadata DynamicObject
    * DynamicObject* pDynObj = getTagHandle(nitfMetadata, "RPC00B", FindFirst());
    * @endcode
    *
    * @param  nitfMetadata
    *         The cube's NITF metadata object.
    * @param  treName
    *         The name of the TRE.
    * @param  cond
    *         A functor that determines if the DynamicObject passed in should be fetched.
    *         argument is provided, gets the first instance of the tag.
    * @return A pointer to the tag handle of the given TRE and instance. NULL, if the
    *         TRE does not exist.
    */
   template<class Condition>
   const DynamicObject* getTagHandle(const DynamicObject& nitfMetadata,
      std::string treName, Condition cond = Condition())
   {
      std::string actualType;

      const DynamicObject* pTre = nitfMetadata.getAttribute(Nitf::TRE_METADATA).getPointerToValue<DynamicObject>();
      if (pTre == NULL)
      {
         return NULL;
      }

      // now get the TRE DynamicObject
      pTre = pTre->getAttribute(treName).getPointerToValue<DynamicObject>();
      if (pTre == NULL)
      {
         return NULL;
      }

      const DynamicObject* pFound = NULL;
      std::vector<std::string> instances;
      pTre->getAttributeNames(instances);
      for (std::vector<std::string>::const_iterator it = instances.begin(); it != instances.end(); ++it)
      {
         const DynamicObject* pObj = pTre->getAttribute(*it).getPointerToValue<DynamicObject>();
         if (pObj != NULL && cond(*pObj))
         {
            pFound = pObj;
            break;
         }
      }
      return pFound;
   }

   /**
    * Test a tag against a range
    *
    * @param reporter
    *        ostream to report failures to
    * @param testValue
    *        The value to test
    * @param minValue
    *        The minimum value of the range
    * @param maxValue
    *        The maximum value of the range
    *
    * @return SUSPECT if outside the range, VALID otherwise.
    */
   template<typename T>
   TreState testTagValueRange(std::ostream& reporter, const T &testValue, const T &minValue, const T &maxValue)
   {
      if (testValue < minValue || testValue > maxValue)
      {
         reporter << "SUSPECT field: " << testValue << " should have been between "
            << minValue << " and " << maxValue << ", ";
         return SUSPECT;
      }

      return VALID;
   }

   /**
    * Test a tag within a DynamicObject against a range
    *
    * @param tre
    *        The DynamicObject to get the value from
    * @param reporter
    *        ostream to report failures to
    * @param pNumFields
    *        Number of fields currently read in by the calling parser.
    *        If not NULL, this value is incremented if the field has been read.
    * @param name
    *        The name of the tag within the DynamicObject
    * @param minValue
    *        The minimum value of the range
    * @param maxValue
    *        The maximum value of the range
    *        
    * @return INVALID if the tag is not found, SUSPECT if outside the range, VALID otherwise.
    */
   template<typename T>
   TreState testTagValueRange(const DynamicObject& tre, std::ostream& reporter, 
      unsigned int* pNumFields, const std::string& name, const T &minValue, const T &maxValue)
   {
      try
      {
         T testValue = dv_cast<T>(tre.getAttribute(name));
         TreState state = testTagValueRange(reporter, testValue, minValue, maxValue);
         if (state != VALID)
         {
            reporter << ": field = " << name << ".\n";
         }

         if (pNumFields != NULL)
         {
            (*pNumFields)++;
         }

         return state;
      }
      catch (const std::bad_cast&)
      {
         reporter << "Field " << name << " not found in Dynamic Object from testTagValueRange().\n";
      }
      return INVALID;
   }

   /**
    * Test a tag within a DynamicObject for equivalence
    *
    * @param tre
    *        The DynamicObject to get the value from
    * @param reporter
    *        ostream to report failures to
    * @param name
    *        The name of the tag within the DynamicObject
    * @param eqValue
    *        The value to test equivalence against
    *        
    * @return INVALID if the tag is not found, SUSPECT if not equal, VALID otherwise.
    */
   template<typename T>
   TreState testTagValueEq(const DynamicObject& tre, std::ostream& reporter, const std::string& name, const T &eqValue)
   {
      try
      {
         const T &testValue = dv_cast<T>(tre.getAttribute(name));
         if (testValue == eqValue)
         {
            return VALID;
         }
         reporter << "Field " << name << " = " << testValue << " should have been equal to " << eqValue << ".\n";
         return SUSPECT;
      }
      catch (std::bad_cast e)
      {
         reporter << "Field " << name << " not found in Dynamic Object from testTagValueEq().\n";
         return INVALID;
      }

      return UNTESTED;
   }

   /**
    * Test a tag within a DynamicObject for non-equivalence
    *
    * @param tre
    *        The DynamicObject to get the value from
    * @param reporter
    *        ostream to report failures to
    * @param name
    *        The name of the tag within the DynamicObject
    * @param notEqValue
    *        The value to test non-equivalence against
    *        
    * @return INVALID if the tag is not found, SUSPECT if equal, VALID otherwise.
    */
   template<typename T>
   inline TreState testTagValueNotEq(const DynamicObject& tre, std::ostream& reporter,
      const std::string& name, const T &notEqValue)
   {
      try
      {
         const T &testValue = dv_cast<T>(tre.getAttribute(name));
         if (testValue != notEqValue)
         {
            return VALID;
         }
         reporter << "Field " << name << " = " << testValue <<
            " should have been not equal to " << notEqValue << ".\n";
         return SUSPECT;
      }
      catch (std::bad_cast e)
      {
         reporter << "Field " << name << " not found in Dynamic Object from testTagValueEq().\n";
         return INVALID;
      }

      return UNTESTED;
   }

   /**
    * Combination typecast and endian swap.
    *
    * @paran pBuffer
    *        Buffer containing non-swapped data
    * @param sourceEndian
    *        The endianness of the data in the buffer
    *
    * @return The contents of the buffer, appropriately typed and swapped.
    */
   template<typename T>
   T convertBinary(const void *pBuffer, EndianType sourceEndian)
   {
      T data;
      Endian endian(sourceEndian);
      memcpy(&data, pBuffer, sizeof(T));
      endian.swapValue(data);
      return data;
   }

   /**
    * Test the specified bit.
    *
    * @param bitMask
    *        Mask to tests
    * @param bit
    *        Bit of the mask to test
    *
    * @return True if the given bit is true, false otherwise.
    */
   bool bitTest(unsigned int bitMask, unsigned int bit);

   /**
    * Parse a date-time string of the form HHMM (hour minute).
    *
    * @param fDTG
    *        The string to parse
    * @param hour
    *        Upon successful return, this contains the hours from the string.
    * @param min
    *        Upon successful return, this contains the minutes from the string.
    *
    * @return True if the parse succeeded, false otherwise.
    */
   bool DtgParseHHMM(const std::string &fDTG,
      unsigned short &hour,
      unsigned short &min);

   /**
    * Parse a date-time string of the form CCYYMMDDhhmm.
    *
    * @param fDTG
    *        The string to parse
    * @param year
    *        Upon successful return, this contains the year (including century) from the string.
    * @param month
    *        Upon successful return, this contains the month from the string.
    * @param day
    *        Upon successful return, this contains the day from the string.
    * @param hour
    *        Upon successful return, this contains the hours from the string.
    * @param min
    *        Upon successful return, this contains the minutes from the string.
    *
    * @return True if the parse succeeded, false otherwise.
    */
   bool DtgParseCCYYMMDDhhmm(const std::string &fDTG, 
      unsigned short &year, 
      unsigned short &month, 
      unsigned short &day, 
      unsigned short &hour, 
      unsigned short &min);

   /**
    * Parse a date-time string of the form CCYYMMDDhhmm.
    *
    * @param fDTG
    *        The string to parse
    * @param pDateTime
    *        Upon successful return, this contains the date and time from the string.
    *
    * @return True if the parse succeeded, false otherwise.
    */
   bool DtgParseCCYYMMDDhhmm(const std::string &fDTG, 
      DateTime *pDateTime);

   /**
    * Parse a date-time string of the form CCYYMMDDhhmmss.
    *
    * @param fDTG
    *        The string to parse
    * @param year
    *        Upon successful return, this contains the year (including century) from the string.
    * @param month
    *        Upon successful return, this contains the month from the string.
    * @param day
    *        Upon successful return, this contains the day from the string.
    * @param hour
    *        Upon successful return, this contains the hours from the string.
    * @param min
    *        Upon successful return, this contains the minutes from the string.
    * @param sec
    *        Upon successful return, this contains the seconds from the string.
    * @param pDateValid
    *        If non-NULL, the value pointed at will contain true if the date portion
    *        of the date-time string parsed successfully.
    * @param pTimeValid
    *        If non-NULL, the value pointed at will contain true if the time portion
    *        of the date-time string parsed successfully.
    *
    * @return True if the parse completely succeeded, false otherwise. Note
    *         that even if this returns false, pDateValid and pTimeValid will
    *         be set appropriately.
    */
   bool DtgParseCCYYMMDDhhmmss(const std::string &fDTG, 
      unsigned short &year, 
      unsigned short &month, 
      unsigned short &day, 
      unsigned short &hour, 
      unsigned short &min,
      unsigned short &sec,
      bool *pDateValid = NULL,
      bool *pTimeValid = NULL);

   /**
    * Parse a date-time string of the form CCYYMMDDhhmmss.
    *
    * @param fDTG
    *        The string to parse
    * @param pDateTime
    *        Upon successful return, this contains the date and time from the string.
    *
    * @return True if the parse succeeded, false otherwise.
    *         Note that even if this returns false, pDateTime may contain partially valid data.
    */
   bool DtgParseCCYYMMDDhhmmss(const std::string &fDTG, 
      DateTime *pDateTime);

   /**
    * Parse a date-time string of the form CCYYMMDD.
    *
    * @param fDTG
    *        The string to parse
    * @param year
    *        Upon successful return, this contains the year (including century) from the string.
    * @param month
    *        Upon successful return, this contains the month from the string.
    * @param day
    *        Upon successful return, this contains the day from the string.
    *
    * @return True if the parse succeeded, false otherwise.
    */
   bool DtgParseCCYYMMDD(const std::string &fDTG, 
      unsigned short &year, 
      unsigned short &month, 
      unsigned short &day);

   /**
    * Parse a date-time string of the form CCYYMMDD.
    *
    * @param fDTG
    *        The string to parse
    * @param pDateTime
    *        Upon successful return, this contains the date and time from the string.
    *
    * @return True if the parse succeeded, false otherwise.
    */
   bool DtgParseCCYYMMDD(const std::string &fDTG, 
      DateTime *pDateTime);

   /**
    * Parse a date-time string of the form DDMMMYY, where MMM is a three-letter
    * abbreviation for the month.
    *
    * @param fDTG
    *        The string to parse
    * @param year
    *        Upon successful return, this contains the year (including century) from the string.
    *        The century assumes that any two-digit year less than 48 refers to 2000 + YY, and any
    *        two-digit year greater than or equal to 48 refers to 1900 + YY.
    * @param month
    *        Upon successful return, this contains the 1-based month from the string.
    * @param day
    *        Upon successful return, this contains the day from the string.
    *
    * @return True if the parse succeeded, false otherwise.
    */
   bool DtgParseDDMMMYY(const std::string &fDTG, 
      unsigned short &year, 
      unsigned short &month, 
      unsigned short &day);

   /**
    * Parse a date-time string of the form DDMMMYY, where MMM is a three-letter
    * abbreviation for the month.
    *
    * @param fDTG
    *        The string to parse
    * @param pDateTime
    *        Upon successful return, this contains the date and time from the string.
    *        The century assumes that any two-digit year less than 48 refers to 2000 + YY, and any
    *        two-digit year greater than or equal to 48 refers to 1900 + YY.
    *
    * @return True if the parse succeeded, false otherwise.
    */
   bool DtgParseDDMMMYY(const std::string &fDTG, 
      DateTime *pDateTime);

   /**
    * Parse a date-time string of the form CCYYMMDDhhmmss.
    *
    * @param fDTG
    *        The string to parse
    * @param year
    *        Upon successful return, this contains the year (including century) from the string.
    * @param month
    *        Upon successful return, this contains the month from the string.
    * @param day
    *        Upon successful return, this contains the day from the string.
    * @param hour
    *        Upon successful return, this contains the hours from the string.
    * @param min
    *        Upon successful return, this contains the minutes from the string.
    * @param sec
    *        Upon successful return, this contains the seconds from the string.
    *
    * @return True if the parse succeeded, false otherwise.
    */
   bool DtgParseCCYYMMMDDhhmmss(const std::string &fDTG, 
      unsigned short &year, 
      unsigned short &month, 
      unsigned short &day,
      unsigned short &hour,
      unsigned short &min,
      unsigned short &sec);

   /**
    * Parse a date-time string of the form CCYYMMDDhhmmss.
    *
    * @param fDTG
    *        The string to parse
    * @param pDateTime
    *        Upon successful return, this contains the date and time from the string.
    *
    * @return True if the parse succeeded, false otherwise.
    */
   bool DtgParseCCYYMMMDDhhmmss(const std::string &fDTG, 
      DateTime *pDateTime);

   /**
    * Parse a date-time string of the form DDHHMMSSZMONYY, where MMM is a three-letter
    * abbreviation for the month.
    *
    * @param fDTG
    *        The string to parse
    * @param year
    *        Upon successful return, this contains the year (including century) from the string.
    *        The century assumes that any two-digit year less than 48 refers to 2000 + YY, and any
    *        two-digit year greater than or equal to 48 refers to 1900 + YY.
    * @param month
    *        Upon successful return, this contains the month from the string.
    * @param day
    *        Upon successful return, this contains the day from the string.
    * @param hour
    *        Upon successful return, this contains the hours from the string.
    * @param min
    *        Upon successful return, this contains the minutes from the string.
    * @param sec
    *        Upon successful return, this contains the seconds from the string.
    *
    * @return True if the parse succeeded, false otherwise.
    */
   bool DtgParseDDHHMMSSZMONYY(const std::string &fDTG, 
      unsigned short &year, 
      unsigned short &month, 
      unsigned short &day,
      unsigned short &hour,
      unsigned short &min,
      unsigned short &sec);

   /**
    * Parse a date-time string of the form CCYYMMDDhhmmss, where MMM is a three-letter
    * abbreviation for the month.
    *
    * @param fDTG
    *        The string to parse
    * @param pDateTime
    *        Upon successful return, this contains the date and time from the string.
    *        The century assumes that any two-digit year less than 48 refers to 2000 + YY, and any
    *        two-digit year greater than or equal to 48 refers to 1900 + YY.
    *
    * @return True if the parse succeeded, false otherwise.
    */
   bool DtgParseDDHHMMSSZMONYY(const std::string &fDTG, 
      DateTime *pDateTime);

   /**
    * Parse a date-time string of the form YYMMDD.
    *
    * @param fDTG
    *        The string to parse.
    * @param year
    *        Upon successful return, this contains the year (including century) from the string.
    *        The century assumes that any two-digit year less than 48 refers to 2000 + YY, and any
    *        two-digit year greater than or equal to 48 refers to 1900 + YY.
    * @param month
    *        Upon successful return, this contains the month from the string.
    * @param day
    *        Upon successful return, this contains the day from the string.
    *
    * @return True if the parse succeeded, false otherwise.
    */
   bool DtgParseYYMMDD(const std::string &fDTG, 
      unsigned short &year, 
      unsigned short &month, 
      unsigned short &day);

   /**
    * Parse a date-time string of the form YYMMDD.
    *
    * @param fDTG
    *        The string to parse.
    * @param pDateTime
    *        Upon successful return, this contains the date from the string.
    *        The century assumes that any two-digit year less than 48 refers to 2000 + YY, and any
    *        two-digit year greater than or equal to 48 refers to 1900 + YY.
    *
    * @return True if the parse succeeded, false otherwise.
    */
    bool DtgParseYYMMDD(const std::string &fDTG,
      DateTime *pDateTime);

   /**
    * Test a tag to ensure valid BCS-A strings from within a set.
    *
    * @param testValue
    *        String to test.
    * @param reporter
    *        ostream to report errors to.
    * @param testSet
    *        Set of always allowed strings.
    * @param allBlankOk
    *        If true, it is valid to have an all blank (whitespace) string in testValue.
    * @param notInSetOk
    *        If true, it is valid to have testValue not occur in testSet.
    * @param emitMsgNotInSet
    *        If true, emit a message to reporter when testValue does not occur in testSet.
    *
    * @return This method follows these rules, in this order:
    *           -# If testValue occurs in testSet, returns TreState::VALID.
    *           -# If true is false, and testString is all blank,
    *                 emits a message and returns TreState::INVALID.
    *           -# If emitMsgNotInSet, emit a message
    *           -# If notInSetOk is false, returns TreState::SUSPECT.
    *           -# If testValue contains characters outside of BCS-A,
    *                 emits a message and returns TreState::INVALID.
    */
   TreState testTagValidBcsASet( const std::string &testValue, 
                                 std::ostream& reporter,
                                 std::set<std::string> testSet, 
                                 bool allBlankOk = false,
                                 bool notInSetOk = true,
                                 bool emitMsgNotInSet = false );
   


   /**
    * Test a tag to ensure valid BCS-A strings from within a set.
    *
    * @param tre
    *        DynamicObject to get the test value from
    * @param reporter
    *        ostream to report errors to.
    * @param pNumFields
    *        Number of fields currently read in by the calling parser.
    *        If not NULL, this value is incremented if the field has been read.
    * @param name
    *        The name of the test value within the DynamicObject.
    * @param testSet
    *        Set of always allowed strings.
    * @param allBlankOk
    *        If true, it is valid to have an all blank (whitespace) string in the test value.
    * @param notInSetOk
    *        If true, it is valid to have the test value not occur in testSet.
    * @param emitMsgNotInSet
    *        If true, emit a message to reporter when the test value does not occur in testSet.
    *
    * @return This method follows these rules, in this order:
    *           -# If testValue occurs in testSet, returns TreState::VALID.
    *           -# If true is false, and testString is all blank,
    *                 emits a message and returns TreState::INVALID.
    *           -# If emitMsgNotInSet, emit a message
    *           -# If notInSetOk is false, returns TreState::SUSPECT.
    *           -# If testValue contains characters outside of BCS-A,
    *                 emits a message and returns TreState::INVALID.
    */
   TreState testTagValidBcsASet( const DynamicObject& tre, 
                                 std::ostream& reporter,
                                 unsigned int* pNumFields,
                                 const std::string& name,
                                 const std::set<std::string> &testSet, 
                                 bool allBlankOk = false,
                                 bool notInSetOk = true,
                                 bool emitMsgNotInSet = false );


   /**
    * Reads a number of characters from a stream and places it in a vector of chars.
    *
    * @param  strm
    *         The source stream.
    * @param  buf
    *         The destination vector for the data.
    * @param  count
    *         The number of bytes to read from the stream.
    * @param  bStr
    *         TRUE if the value read will need to be a NULL-terminated string. FALSE if the data will be
    *         treated as binary data.
    * @return TRUE if the operation succeeds and the stream is good, FALSE otherwise.
    */
   bool readFromStream(std::istream& strm, std::vector<char>& buf, std::streamsize count, bool bStr = true);


   /**
    * Cleans up a string for NITF export.  It guarantees the string is characters long.
    * Left or right justified, as specified, if the string needs to be lengthened.
    *
    * @param  str
    *         The string to be cleaned up for NITF export
    * @param  size
    *         The required length of the output string.
    * @param  fillChar
    *         The character to fill with if the string needs to be lenghtened. Default is blank (' ').
    * @param  rightJustify
    *         If true then the string is right justified, left justified otherwise.
    * @return The output string. Guaranteed to be size characters long.
    */
   std::string sizeString(const std::string &str, unsigned int size, char fillChar=' ', bool rightJustify=false);

   /**
    * Converts a numeric to a string.  It only works with numeric intrinsics
    * Guaranteed to be num characters long with a leading '-' if negative or 
    * a leading '+' if positive and posSign == true
    *
    * @param  num
    *         The numeric to be converted to a string.
    * @param  size
    *         The required length of the output string.
    * @param  precision
    *         For floating point the number of digits after the decimal point. 
    *         Default == -1 which means to fit in the maximum within the allocated size.
    * @param  fillChar
    *         The numeric fill character. Default == '0'
    * @param  posSign
    *         If true then output a leading "+" if the num is positive.
    * @param  sciNotation
    *         If true then force scientific notation in the form: "+0.123456E+001"
    * @param  expSize
    *         The number of digets in the exponent. Where 1 <= expSize <= 3
    * @return The numeric value converted to a string. Guaranteed to be num characters 
    *         long with a leading '-' if negative or a leading '+' if positive and posSign == true
    */
   template<typename T>
   std::string toString(T num, unsigned int size, int precision=-1,
      char fillChar='0', bool posSign=false, bool sciNotation=false, int expSize=3)
   {
      std::stringstream   outp;
      std::string         outStr;

      if (precision < 0)
      {
         precision = size;
      }

      if (posSign) 
      {
         outp << std::showpos;
      }

      if (fillChar == '0')
      {
         outp << std::internal;
      }

      outp << std::setfill(fillChar) << std::noboolalpha << std::setw(size);

      if (typeid(num) == typeid(float) || typeid(num) == typeid(double))
      {
         if (sciNotation)
         {
            // The I/O routines give us a 3 digit exponent but some TAGs require less.
            // The minimum size is 7, +0.0E+0. Addition digits are added to either the
            // precision of the mantissia or exponent. +0.0000E+000
            if (size < 7)
            {
               sciNotation = false;
            }
            else 
            {
               if (expSize < 1 || expSize > 3)
               {
                  // We need the abs(num) but the overloaded abs() function is ambiguous 
                  // in this template so use "?" operator on a known type
                  double ans(num);
                  ans = (ans >= 0 ? ans : -ans);

                  expSize = 3;
                  if (ans < 1.0E+10)
                  {
                     expSize = 1;
                  }
                  else if (ans > 1.0E-99 && ans < 1.0E+100)
                  {
                     expSize = 2;
                  }

               }
               int prec = size - 5 - expSize;
               if (prec == -1)
               {
                  prec = 1;
                  expSize = 1;
               }
               else if (prec == 0)
               {
                  prec = 1;
                  expSize = 2;
               }

               outp.setf(ios::uppercase);
               outp << std::scientific << std::setprecision(prec) << num;
            }
         }

         if (!sciNotation)
         {
            outp << std::fixed << std::setprecision(precision) << num;
         }
      }

      // char or unsigned char print out as the ASCII value instead of the numeric
      // so cast it to an int.
      else if (typeid(num) == typeid(char)) 
      {
         outp << static_cast<int>(num);
      }

      // "unsigned" vars do not print the "+" even with "showpos" 
      // so cast it to a signed int to force the "+" 
      // then use a int64 for the extra room to force it positive even 
      // if the sign bit was set.
      else if (!numeric_limits<T>::is_signed) 
      {
         outp << static_cast<long long>(num);
      }

      else 
      {
         outp << num;
      }

      // use ".str()" method instead of ">>" because ">>" left justifies 
      // blank filled strings whereas ".str()" works in every case
      outStr = outp.str();

      if (sciNotation)
      {
         // Find the first digit in the exponent
         const unsigned int eLocation = outStr.rfind("E");
         VERIFYRV(eLocation != string::npos && eLocation < outStr.length() - 2, std::string());
         const unsigned int exponentLocation = eLocation + 2;

         // If there are not enough digits, add them
         while (outStr.length() - exponentLocation < static_cast<unsigned int>(expSize))
         {
            outStr.insert(exponentLocation, "0");
         }

         // If there are too many digits, delete them
         while (outStr.length() - exponentLocation > static_cast<unsigned int>(expSize))
         {
            outStr.erase(exponentLocation, 1);
         }
      }

      // clamp the string length to the required size because << might have
      // made it too long. This will truncate the least significant digits.
      if (outStr.size() != size)
      {
         outStr.resize(size);
      }

      return outStr;
   }


   // General template for dealing with most types
   template<typename T>
   inline T fromBuffer(std::vector<char> &buf, bool &ok, bool allBlankOk)
   {
      if (!numeric_limits<T>::is_signed)
      {
         std::string temp = StringUtilities::stripWhitespace(std::string(&buf.front()));
         if (temp.size() > 0 && temp[0] == '-')
         {
            ok = false;
            T junk = 0;
            return junk;
         }
      }

      try
      {
         string trimmedString = StringUtilities::stripWhitespace(std::string(&buf.front()));
         if (trimmedString.empty() == true)
         {
            ok = allBlankOk;
         }
         else
         {
            return boost::lexical_cast<T>(trimmedString);
         }
      }
      catch (boost::bad_lexical_cast e)
      {
         ok = false;
      }

      T junk = 0;
      return junk;
   }

   // Specialization for the unusual ones
   template <>
   inline std::string fromBuffer<std::string>(std::vector<char> &buf, bool &ok, bool allBlankOk)
   {
      std::string trimmedString = StringUtilities::stripWhitespace(std::string(&buf.front()));
      if (trimmedString.empty() == true)
      {
         ok = allBlankOk;
      }

      return trimmedString;
   }

   // create the error message
   inline bool readFieldErrMsg(std::string &msg, const std::string &name, const std::vector<char> &buf, int len)
   {
      std::string value;
      value.resize(len);
      strcpy(&value[0], &buf[0]);

      msg += "NITF Parse TAG Error: " + name + " = " + value + "\n";
      return true;
   }

   //
   inline bool numReadErrMsg(int numRead, int numBytes, std::string &errorMessage)
   {
      std::string numB, numR;

      try
      {
         numR = boost::lexical_cast<std::string>(numRead);
         numB = boost::lexical_cast<std::string>(numBytes);
      }
      catch (boost::bad_lexical_cast e)
      {
      }

      if (numRead < 0) {
         errorMessage += "Read past end of data. Should have read " + numB + " bytes.\n";
      }
      else {
         errorMessage += "Read " + numR + " bytes. Should have read " + numB + " bytes.\n";
      }

      return true;
   }

   template<typename T>
   inline bool readField(std::istream &input, DynamicObject &output, bool &success,
      const std::string &name, int len, std::string &msg, std::vector<char> &buf, bool allBlankOk = false)
   {
      bool ok(true);
      if (input.good() == false)
      {
         success = false;
      }
      else if (readFromStream(input, buf, len) == false)
      {
         success = false;
      }
      else if (output.setAttribute(name, fromBuffer<T>(buf, ok, allBlankOk)) == false || ok == false)
      {
         success = false;
         readFieldErrMsg(msg, name, buf, len);
      }

      return success;
   }

   /**
    * Determines whether a Classification object is valid for export to a NITF 2.1 file.
    *
    * @param  classification
    *         The Classification object to validate.
    *
    * @param  errorMessage
    *         An error message indicating the nature of the problem.
    *
    * @return True if the given object is valid for NITF 2.1 export, false otherwise.
    */
   bool isClassificationValidForExport(const Classification& classification,
      std::string& errorMessage = std::string());

   /**
    * Determines whether a classification field is valid for export to a NITF 2.1 file.
    *
    * @param  classification
    *         The Classification object to validate.
    *
    * @param  fieldName
    *         The name of the field to validate.
    *
    * @return True if the given field is valid for NITF 2.1 export, false otherwise.
    */
   bool isClassificationFieldValidForExport(const Classification& classification, const std::string& fieldName);
}

#endif
