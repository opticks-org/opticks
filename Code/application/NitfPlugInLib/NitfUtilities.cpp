/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "Classification.h"
#include "DateTime.h"
#include "NitfUtilities.h"
#include "SpecialMetadata.h"
#include "StringUtilities.h"

#include <algorithm>
#include <string>
#include <iomanip>
#include <vector>
#include <boost/lexical_cast.hpp>

using namespace std;
using boost::lexical_cast;

Nitf::TreState Nitf::MaxState(Nitf::TreState stateA, Nitf::TreState stateB)
{
   if (stateA > stateB)
   {
      return stateA;
   }

   return stateB;
}

bool Nitf::bitTest(unsigned int bitMask, unsigned int bit)
{
   return (bitMask >> bit & 0x01);
}

bool Nitf::DtgParseHHMM(const string &fDTG, unsigned short &hour, unsigned short &min)
{
   try
   {
      hour = lexical_cast<unsigned short>(fDTG.substr(0, 2));
      min = lexical_cast<unsigned short>(fDTG.substr(2, 2));
   }
   catch (const boost::bad_lexical_cast&)
   {
      return false;
   }

   if (hour > 23)
   {
      return false;
   }
   if (min > 59)
   {
      return false;
   }

   return true;
}

namespace
{
   //                                              J   F   M   A   M   J   J   A   S   O   N   D
   static const unsigned short maxDayOfMonth[] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
}

bool Nitf::DtgParseCCYYMMDDhhmm(const string &fDTG, 
                          unsigned short &year, 
                          unsigned short &month, 
                          unsigned short &day, 
                          unsigned short &hour, 
                          unsigned short &min)
{
   if (fDTG.size() < 12)
   {
      return false;
   }
   try
   {
      year = lexical_cast<unsigned short>(fDTG.substr(0, 4));
      month = lexical_cast<unsigned short>(fDTG.substr(4, 2));
      day = lexical_cast<unsigned short>(fDTG.substr(6, 2));
      hour = lexical_cast<unsigned short>(fDTG.substr(8, 2));
      min = lexical_cast<unsigned short>(fDTG.substr(10, 2));
   }
   catch (const boost::bad_lexical_cast&)
   {
      return false;
   }

   if (year < 1900 || year > 2100)
   {
      return false;
   }
   if (month < 1 || month > 12)
   {
      return false;
   }
   if (day < 1 || day > maxDayOfMonth[month-1])
   {
      return false;
   }
   if (hour > 23)
   {
      return false;
   }
   if (min > 59)
   {
      return false;
   }

   return true;
}

bool Nitf::DtgParseCCYYMMDDhhmm(const string &fDTG, 
   DateTime *pDateTime)
{
   unsigned short year;
   unsigned short month;
   unsigned short day;
   unsigned short hour;
   unsigned short min;

   if (DtgParseCCYYMMDDhhmm(fDTG, year, month, day, hour, min) == false)
   {
      return false;
   }

   return pDateTime != NULL && pDateTime->set(year, month, day, hour, min, 0);
}

bool Nitf::DtgParseCCYYMMDDhhmmss(const string &fDTG, 
   unsigned short &year, 
   unsigned short &month, 
   unsigned short &day, 
   unsigned short &hour, 
   unsigned short &min,
   unsigned short &sec,
   bool *pDateValid,
   bool *pTimeValid)
{
   if (fDTG.size() < 14)
   {
      if (pTimeValid != NULL)
      {
         *pTimeValid = false;
      }
      if (pDateValid != NULL)
      {
         *pDateValid = false;
      }
      return false;
   }

   bool dateValid = true;
   bool timeValid = true;

   // Set everything to invalid values
   year = 0;
   month = 0;
   day = 0;
   hour = 100;
   min = 100;
   sec = 100;

   try
   {
      year = lexical_cast<unsigned short>(fDTG.substr(0, 4));
      month = lexical_cast<unsigned short>(fDTG.substr(4, 2));
      day = lexical_cast<unsigned short>(fDTG.substr(6, 2));
      hour = lexical_cast<unsigned short>(fDTG.substr(8, 2));
      min = lexical_cast<unsigned short>(fDTG.substr(10, 2));
      sec = lexical_cast<unsigned short>(fDTG.substr(12, 2));
   }
   catch (const boost::bad_lexical_cast&)
   {
      // Do nothing
   }

   if (year < 1900 || year > 2100)
   {
      dateValid = false;
   }
   if (month < 1 || month > 12)
   {
      dateValid = false;
   }
   if (day < 1 || day > maxDayOfMonth[month-1])
   {
      dateValid = false;
   }
   if (hour > 23)
   {
      timeValid = false;
   }
   if (min > 59)
   {
      timeValid = false;
   }
   if (sec > 59)
   {
      timeValid = false;
   }
   if (pDateValid != NULL)
   {
      *pDateValid = dateValid;
   }
   if (pTimeValid != NULL)
   {
      *pTimeValid = timeValid;
   }

   return timeValid && dateValid;
}

bool Nitf::DtgParseCCYYMMDDhhmmss(const string &fDTG, 
   DateTime *pDateTime)
{
   unsigned short year;
   unsigned short month;
   unsigned short day;
   unsigned short hour;
   unsigned short min;
   unsigned short sec;
   bool dateValid;
   bool timeValid;

   bool success = DtgParseCCYYMMDDhhmmss(fDTG, year, month, day, hour, min, sec, &dateValid, &timeValid);
   if (dateValid == true)
   {
      if (timeValid == true)
      {
         if (pDateTime == NULL || pDateTime->set(year, month, day, hour, min, sec) == false)
         {
            success = false;
         }
      }
      else
      {
         if (pDateTime == NULL || pDateTime->set(year, month, day))
         {
            success = false;
         }
      }
   }

   return success;
}

bool Nitf::DtgParseCCYYMMDD(const string &fDTG, 
   unsigned short &year, 
   unsigned short &month, 
   unsigned short &day)
{
   if (fDTG.size() < 8)
   {
      return false;
   }

   try
   {
      year = lexical_cast<unsigned short>(fDTG.substr(0, 4));
      month = lexical_cast<unsigned short>(fDTG.substr(4, 2));
      day = lexical_cast<unsigned short>(fDTG.substr(6, 2));
   }
   catch (const boost::bad_lexical_cast&)
   {
      return false;
   }

   if (year < 1900 || year > 2100)
   {
      return false;
   }
   if (month < 1 || month > 12)
   {
      return false;
   }
   if (day < 1 || day > maxDayOfMonth[month-1])
   {
      return false;
   }

   return true;
}

bool Nitf::DtgParseCCYYMMDD(const string &fDTG, 
   DateTime *pDateTime)
{
   unsigned short year;
   unsigned short month;
   unsigned short day;

   if (DtgParseCCYYMMDD(fDTG, year, month, day) == false)
   {
      return false;
   }

   return pDateTime != NULL && pDateTime->set(year, month, day);
}

bool Nitf::DtgParseDDMMMYY(const string &fDTG, 
   unsigned short &year, 
   unsigned short &month, 
   unsigned short &day)
{  
   if (fDTG.size() < 7)
   {
      return false;
   }

   string mmm;

   try
   {
      day = lexical_cast<unsigned short>(fDTG.substr(0, 2));
      mmm = fDTG.substr(2, 3);
      year = lexical_cast<unsigned short>(fDTG.substr(5, 2));
   }
   catch (const boost::bad_lexical_cast&)
   {
      return false;
   }

   if (year < 48)
   {
      year += 2000;
   }
   else
   {
      year += 1900;
   }

   if (mmm == "JAN")
   {
      month = 1;
   }
   else if (mmm == "FEB")
   {
      month = 2;
   }
   else if (mmm == "MAR")
   {
      month = 3;
   }
   else if (mmm == "APR")
   {
      month = 4;
   }
   else if (mmm == "MAY")
   {
      month = 5;
   }
   else if (mmm == "JUN")
   {
      month = 6;
   }
   else if (mmm == "JUL")
   {
      month = 7;
   }
   else if (mmm == "AUG")
   {
      month = 8;
   }
   else if (mmm == "SEP")
   {
      month = 9;
   }
   else if (mmm == "OCT")
   {
      month = 10;
   }
   else if (mmm == "NOV")
   {
      month = 11;
   }
   else if (mmm == "DEC")
   {
      month = 12;
   }
   else
   {
      return false;
   }

   if (day < 1 || day > maxDayOfMonth[month-1])
   {
      return false;
   }

   return true;
}

bool Nitf::DtgParseDDMMMYY(const string &fDTG, 
   DateTime *pDateTime)
{
   unsigned short year;
   unsigned short month;
   unsigned short day;

   if (DtgParseDDMMMYY(fDTG, year, month, day) == false)
   {
      return false;
   }

   return pDateTime != NULL && pDateTime->set(year, month, day);
}

bool Nitf::DtgParseCCYYMMMDDhhmmss(const string &fDTG, 
   unsigned short &year, 
   unsigned short &month, 
   unsigned short &day,
   unsigned short &hour,
   unsigned short &min,
   unsigned short &sec)
{  
   if (fDTG.size() < 15)
   {
      return false;
   }

   string mmm;
   try
   {
      year = lexical_cast<unsigned short>(fDTG.substr(0, 4));
      mmm = fDTG.substr(4, 3);
      day = lexical_cast<unsigned short>(fDTG.substr(7, 2));
      hour = lexical_cast<unsigned short>(fDTG.substr(9, 2));
      min = lexical_cast<unsigned short>(fDTG.substr(11, 2));
      sec = lexical_cast<unsigned short>(fDTG.substr(13, 2));
   }
   catch (const boost::bad_lexical_cast&)
   {
      return false;
   }

   if (year < 1900 || year > 2100)
   {
      return false;
   }

   if (mmm == "JAN")
   {
      month = 1;
   }
   else if (mmm == "FEB")
   {
      month = 2;
   }
   else if (mmm == "MAR")
   {
      month = 3;
   }
   else if (mmm == "APR")
   {
      month = 4;
   }
   else if (mmm == "MAY")
   {
      month = 5;
   }
   else if (mmm == "JUN")
   {
      month = 6;
   }
   else if (mmm == "JUL")
   {
      month = 7;
   }
   else if (mmm == "AUG")
   {
      month = 8;
   }
   else if (mmm == "SEP")
   {
      month = 9;
   }
   else if (mmm == "OCT")
   {
      month = 10;
   }
   else if (mmm == "NOV")
   {
      month = 11;
   }
   else if (mmm == "DEC")
   {
      month = 12;
   }
   else
   {
      return false;
   }

   if (day < 1 || day > maxDayOfMonth[month-1])
   {
      return false;
   }
   if (hour > 23)
   {
      return false;
   }
   if (min > 59)
   {
      return false;
   }
   if (sec > 59)
   {
      return false;
   }

   return true;
}

bool Nitf::DtgParseCCYYMMMDDhhmmss(const string &fDTG, 
   DateTime *pDateTime)
{
   unsigned short year;
   unsigned short month;
   unsigned short day;
   unsigned short hour;
   unsigned short min;
   unsigned short sec;

   if (DtgParseCCYYMMMDDhhmmss(fDTG, year, month, day, hour, min, sec) == false)
   {
      return false;
   }

   return pDateTime != NULL && pDateTime->set(year, month, day, hour, min, sec);
}

bool Nitf::DtgParseDDHHMMSSZMONYY(const string &fDTG, 
   unsigned short &year, 
   unsigned short &month, 
   unsigned short &day,
   unsigned short &hour,
   unsigned short &min,
   unsigned short &sec)
{
   if (fDTG.size() < 14)
   {
      return false;
   }

   if (fDTG.substr(8, 1) != "Z")
   {
      return false;
   }

   string mmm;
   try
   {
      day = lexical_cast<unsigned short>(fDTG.substr(0, 2));
      hour = lexical_cast<unsigned short>(fDTG.substr(2, 2));
      min = lexical_cast<unsigned short>(fDTG.substr(4, 2));
      sec = lexical_cast<unsigned short>(fDTG.substr(6, 2));
      mmm = fDTG.substr(9, 3);
      year = lexical_cast<unsigned short>(fDTG.substr(12, 2));
   }
   catch (const boost::bad_lexical_cast&)
   {
      return false;
   }

   if (year < 48)
   {
      year += 2000;
   }
   else
   {
      year += 1900;
   }

   if (mmm == "JAN")
   {
      month = 1;
   }
   else if (mmm == "FEB")
   {
      month = 2;
   }
   else if (mmm == "MAR")
   {
      month = 3;
   }
   else if (mmm == "APR")
   {
      month = 4;
   }
   else if (mmm == "MAY")
   {
      month = 5;
   }
   else if (mmm == "JUN")
   {
      month = 6;
   }
   else if (mmm == "JUL")
   {
      month = 7;
   }
   else if (mmm == "AUG")
   {
      month = 8;
   }
   else if (mmm == "SEP")
   {
      month = 9;
   }
   else if (mmm == "OCT")
   {
      month = 10;
   }
   else if (mmm == "NOV")
   {
      month = 11;
   }
   else if (mmm == "DEC")
   {
      month = 12;
   }
   else 
   {
      return false;
   }

   if (day < 1 || day > maxDayOfMonth[month-1])
   {
      return false;
   }
   if (hour > 23)
   {
      return false;
   }
   if (min > 59)
   {
      return false;
   }
   if (sec > 59)
   {
      return false;
   }

   return true;
}

bool Nitf::DtgParseDDHHMMSSZMONYY(const string &fDTG, 
   DateTime *pDateTime)
{
   unsigned short year;
   unsigned short month;
   unsigned short day;
   unsigned short hour;
   unsigned short min;
   unsigned short sec;

   if (DtgParseDDHHMMSSZMONYY(fDTG, year, month, day, hour, min, sec) == false)
   {
      return false;
   }

   return pDateTime != NULL && pDateTime->set(year, month, day, hour, min, sec);
}

bool Nitf::DtgParseYYMMDD(const string &fDTG, 
   unsigned short &year, 
   unsigned short &month, 
   unsigned short &day)
{  
   if (fDTG.size() < 6)
   {
      return false;
   }

   try
   {
      year = lexical_cast<unsigned short>(fDTG.substr(0, 2));
      month = lexical_cast<unsigned short>(fDTG.substr(2, 2));
      day = lexical_cast<unsigned short>(fDTG.substr(4, 2));
   }
   catch (const boost::bad_lexical_cast&)
   {
      return false;
   }

   if (year < 48)
   {
      year += 2000;
   }
   else
   {
      year += 1900;
   }

   if (month < 1 || month > 12)
   {
      return false;
   }

   if (day < 1 || day > maxDayOfMonth[month-1])
   {
      return false;
   }

   return true;
}

bool Nitf::DtgParseYYMMDD(const string &fDTG, 
   DateTime *pDateTime)
{
   unsigned short year;
   unsigned short month;
   unsigned short day;

   if (DtgParseYYMMDD(fDTG, year, month, day) == false)
   {
      return false;
   }

   return pDateTime != NULL && pDateTime->set(year, month, day);
}

bool Nitf::readFromStream(istream& strm, vector<char>& buf, streamsize count, bool bStr)
{
   if (strm.good())
   {
      if (bStr == true)
      {
         ++count;
      }
      buf.resize(count);
      memset(&buf.front(), 0, buf.size());
      if (bStr == true)
      {
         --count;
      }
      strm.read(&buf.front(), count);
      return true;
   }
   return false;
}

namespace
{
   struct InvalidBcsA
   {
      bool operator()(char c)
      {
         return !((c >= 0x20) && (c <= 0x7E));
      }
   };

}

Nitf::TreState Nitf::testTagValidBcsASet( const string &testValue, 
   ostream& reporter, set<string> testSet, bool allBlankOk,
   bool notInSetOk, bool emitMsgNotInSet )
{
   set<string>::iterator inSet = testSet.find(testValue);
   if (inSet == testSet.end())
   {  
      if (StringUtilities::isAllBlank(testValue)) 
      {      
         if (!allBlankOk)
         {
            reporter << " All blank field found when not allowed in field: ";
            return INVALID;
         }
         return VALID;  // found all blank and that is allowed because allBlankOk == true
      }

      if (emitMsgNotInSet)
      {
         reporter << "The value \"" << testValue << "\" was not found in the expected list of values in field: ";
      }

      // Not an all blank string and not in the set
      if (!notInSetOk)
      {
         return SUSPECT;     // extra values not allowed
      }
      
      // Not in Set but extra values are allowed and this is not an all blank string 
      // so see if valid BCS-A characters
      if (find_if (testValue.begin(), testValue.end(), InvalidBcsA()) != testValue.end())
      {
         reporter << "ERROR: A non BCS-A character was found in the field: ";
         return INVALID;
      }
   }
   return VALID;
}

Nitf::TreState Nitf::testTagValidBcsASet(const DynamicObject& tre, 
   ostream& reporter, unsigned int* pNumFields, const string& name, const set<string> &testSet,
   bool allBlankOk, bool notInSetOk, bool emitMsgNotInSet )
{
   string testValue;
   TreState status(INVALID);

   try
   {
      testValue = dv_cast<string>(tre.getAttribute(name));
      status = Nitf::testTagValidBcsASet(testValue, reporter, testSet, allBlankOk, notInSetOk, emitMsgNotInSet);
      if (status != VALID)
      {
         reporter << name;
      }

      if (pNumFields != NULL)
      {
         (*pNumFields)++;
      }
   }
   catch (const bad_cast&)
   {
      reporter << "Field \"" << name << "\" is missing from the Dynamic Object";
   }

   return status;
}

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
string Nitf::sizeString(const string &str, unsigned int size, char fillChar, bool rightJustify)
{
   if (str.size() == size)
   {
      return str;
   }
   

   if (str.size() > size) 
   {
      string outStr(str.substr(0, size - 1));
      return outStr;
   }

   // str must be too short so we need to make it larger with fill
   string outStr;
   stringstream outp;

   if (rightJustify)
   {
      outp << right;
   }
   else
   {
      outp << left;
   }

   outp << setfill(fillChar) << setw(size) << str;
   outStr = outp.str();
   
   return outStr;
}

bool Nitf::isClassificationValidForExport(const Classification& classification, string& errorMessage)
{
   using namespace Nitf;
   std::string err;
   if (classification.isValid(err) == false)
   {
      errorMessage = "Invalid parameter.";
      return false;
   }

   vector<string> invalidFields;
   char* ppFieldNames[] = {"Level", "System", "Codewords", "FileControl", "FileReleasing", "ClassificationReason",
      "DeclassificationType", "DeclassificationDate", "DeclassificationExemption", "FileDowngrade", "DowngradeDate",
      "Description", "Authority", "AuthorityType", "SecuritySourceDate", "SecurityControlNumber", "FileCopyNumber",
      "FileNumberOfCopies" };
   vector<string> fieldNames(&ppFieldNames[0], &ppFieldNames[sizeof(ppFieldNames)/sizeof(ppFieldNames[0])]);
   vector<string>::const_iterator iter;
   for (iter = fieldNames.begin(); iter != fieldNames.end(); ++iter)
   {
      if (isClassificationFieldValidForExport(classification, *iter) == false)
      {
         invalidFields.push_back(*iter);
      }
   }

   if (invalidFields.empty() == true)
   {
      return true;
   }

   errorMessage = "The following fields are invalid: ";
   for (iter = invalidFields.begin(); iter != invalidFields.end(); ++iter)
   {
      errorMessage += *iter + " ";
   }

   return false;
}

bool Nitf::isClassificationFieldValidForExport(const Classification& classification, const string& fieldName)
{
   std::string err;
   if (classification.isValid(err) == false)
   {
      return false;
   }

   string fieldValue;
   vector<string> allowedValues;
   unsigned int maxFieldValueLength = 0;
   if (fieldName == "Level")
   {
      fieldValue = StringUtilities::toUpper(classification.getLevel());
      maxFieldValueLength = 1;
      allowedValues.push_back("T");
      allowedValues.push_back("S");
      allowedValues.push_back("C");
      allowedValues.push_back("R");
      allowedValues.push_back("U");
   }
   else if (fieldName == "System")
   {
      fieldValue = StringUtilities::toUpper(classification.getSystem());
      maxFieldValueLength = 2;
   }
   else if (fieldName == "Codewords")
   {
      fieldValue = StringUtilities::toUpper(classification.getCodewords());
      maxFieldValueLength = 11;
   }
   else if (fieldName == "FileControl")
   {
      fieldValue = StringUtilities::toUpper(classification.getFileControl());
      maxFieldValueLength = 2;
   }
   else if (fieldName == "FileReleasing")
   {
      fieldValue = StringUtilities::toUpper(classification.getFileReleasing());
      maxFieldValueLength = 20;
   }
   else if (fieldName == "ClassificationReason")
   {
      fieldValue = StringUtilities::toUpper(classification.getClassificationReason());
      allowedValues.push_back("");
      allowedValues.push_back("A");
      allowedValues.push_back("B");
      allowedValues.push_back("C");
      allowedValues.push_back("D");
      allowedValues.push_back("E");
      allowedValues.push_back("F");
      allowedValues.push_back("G");
      maxFieldValueLength = 1;
   }
   else if (fieldName == "DeclassificationType")
   {
      fieldValue = StringUtilities::toUpper(classification.getDeclassificationType());
      allowedValues.push_back("");
      allowedValues.push_back("DD");
      allowedValues.push_back("DE");
      allowedValues.push_back("GD");
      allowedValues.push_back("GE");
      allowedValues.push_back("O");
      allowedValues.push_back("X");
      maxFieldValueLength = 2;
   }
   else if (fieldName == "DeclassificationDate")
   {
      const DateTime* pDateTime = classification.getDeclassificationDate();
      return (pDateTime != NULL || pDateTime->isValid());
   }
   else if (fieldName == "DeclassificationExemption")
   {
      fieldValue = StringUtilities::toUpper(classification.getDeclassificationExemption());
      allowedValues.push_back("");
      allowedValues.push_back("X1");
      allowedValues.push_back("X2");
      allowedValues.push_back("X3");
      allowedValues.push_back("X4");
      allowedValues.push_back("X5");
      allowedValues.push_back("X6");
      allowedValues.push_back("X7");
      allowedValues.push_back("X8");
      allowedValues.push_back("X251");
      allowedValues.push_back("X252");
      allowedValues.push_back("X253");
      allowedValues.push_back("X254");
      allowedValues.push_back("X255");
      allowedValues.push_back("X256");
      allowedValues.push_back("X257");
      allowedValues.push_back("X258");
      allowedValues.push_back("X259");
      maxFieldValueLength = 4;
   }
   else if (fieldName == "FileDowngrade")
   {
      fieldValue = StringUtilities::toUpper(classification.getFileDowngrade());
      allowedValues.push_back("");
      allowedValues.push_back("S");
      allowedValues.push_back("C");
      allowedValues.push_back("R");
      maxFieldValueLength = 20;
   }
   else if (fieldName == "DowngradeDate")
   {
      const DateTime* pDateTime = classification.getDowngradeDate();
      return (pDateTime != NULL || pDateTime->isValid());
   }
   else if (fieldName == "Description")
   {
      fieldValue = StringUtilities::toUpper(classification.getDescription());
      maxFieldValueLength = 43;
   }
   else if (fieldName == "Authority")
   {
      fieldValue = StringUtilities::toUpper(classification.getAuthority());
      maxFieldValueLength = 40;
   }
   else if (fieldName == "AuthorityType")
   {
      fieldValue = StringUtilities::toUpper(classification.getAuthorityType());
      allowedValues.push_back("");
      allowedValues.push_back("O");
      allowedValues.push_back("D");
      allowedValues.push_back("M");
      maxFieldValueLength = 1;
   }
   else if (fieldName == "SecuritySourceDate")
   {
      const DateTime* pDateTime = classification.getSecuritySourceDate();
      return (pDateTime != NULL || pDateTime->isValid());
   }
   else if (fieldName == "SecurityControlNumber")
   {
      fieldValue = StringUtilities::toUpper(classification.getSecurityControlNumber());
      maxFieldValueLength = 15;
   }
   else if (fieldName == "FileCopyNumber")
   {
      fieldValue = StringUtilities::toUpper(classification.getFileCopyNumber());
      maxFieldValueLength = 5;
   }
   else if (fieldName == "FileNumberOfCopies")
   {
      fieldValue = StringUtilities::toUpper(classification.getFileNumberOfCopies());
      maxFieldValueLength = 5;
   }

   if (fieldValue.length() > maxFieldValueLength)
   {
      return false;
   }

   if (allowedValues.empty() == false && find(allowedValues.begin(),
      allowedValues.end(), fieldValue) == allowedValues.end())
   {
      return false;
   }

   return true;
}

bool Nitf::updateSpecialMetadata(DynamicObject* pMetadata, vector<double>& centerWavelengths,
   vector<double>& startWavelengths, vector<double>& endWavelengths,
   const vector<double>& fwhms, bool convertFromInverseCentimeters)
{
   if (pMetadata == NULL)
   {
      return false;
   }

   const string centerWavelengthsPath[] = {SPECIAL_METADATA_NAME, BAND_METADATA_NAME,
      CENTER_WAVELENGTHS_METADATA_NAME, END_METADATA_NAME};
   const string startWavelengthsPath[] = {SPECIAL_METADATA_NAME, BAND_METADATA_NAME,
      START_WAVELENGTHS_METADATA_NAME, END_METADATA_NAME};
   const string endWavelengthsPath[] = {SPECIAL_METADATA_NAME, BAND_METADATA_NAME,
      END_WAVELENGTHS_METADATA_NAME, END_METADATA_NAME};

   // Remove any existing wavelength information.
   pMetadata->removeAttributeByPath(centerWavelengthsPath);
   pMetadata->removeAttributeByPath(startWavelengthsPath);
   pMetadata->removeAttributeByPath(endWavelengthsPath);

   vector<double>::const_iterator centerIter;
   vector<double>::const_iterator startIter;
   vector<double>::const_iterator endIter;
   vector<double>::const_iterator fwhmIter;

   // If no centerWavelengths exist, compute them if possible.
   if (centerWavelengths.empty() == true)
   {
      if (startWavelengths.empty() == false)
      {
         if (endWavelengths.empty() == false)
         {
            // Compute centerWavelengths from startWavelengths and endWavelengths.
            if (startWavelengths.size() != endWavelengths.size())
            {
               return false;
            }

            for (startIter = startWavelengths.begin(), endIter = endWavelengths.begin();
               startIter != startWavelengths.end() && endIter != endWavelengths.end();
               ++startIter, ++endIter)
            {
               centerWavelengths.push_back(*startIter + ((*endIter - *startIter) / 2.0));
            }
         }
         else if (fwhms.empty() == false)
         {
            // Compute centerWavelengths from startWavelengths and fwhms.
            if (startWavelengths.size() != fwhms.size())
            {
               return false;
            }

            for (startIter = startWavelengths.begin(), fwhmIter = fwhms.begin();
               startIter != startWavelengths.end() && fwhmIter != fwhms.end();
               ++startIter, ++fwhmIter)
            {
               centerWavelengths.push_back(*startIter + (*fwhmIter / 2.0));
            }
         }
      }
      else if (endWavelengths.empty() == false && fwhms.empty() == false)
      {
         // Compute centerWavelengths from endWavelengths and fwhms.
         if (endWavelengths.size() != fwhms.size())
         {
            return false;
         }

         for (endIter = endWavelengths.begin(), fwhmIter = fwhms.begin();
            endIter != endWavelengths.end() && fwhmIter != fwhms.end();
            ++endIter, ++fwhmIter)
         {
            centerWavelengths.push_back(*endIter - (*fwhmIter / 2.0));
         }
      }
   }

   // If no startWavelengths exist, compute them if possible.
   if (startWavelengths.empty() == true)
   {
      if (centerWavelengths.empty() == false)
      {
         if (fwhms.empty() == false)
         {
            // Compute startWavelengths from centerWavelengths and fwhms.
            if (centerWavelengths.size() != fwhms.size())
            {
               return false;
            }

            for (centerIter = centerWavelengths.begin(), fwhmIter = fwhms.begin();
               centerIter != centerWavelengths.end() && fwhmIter != fwhms.end();
               ++centerIter, ++fwhmIter)
            {
               startWavelengths.push_back(*centerIter - (*fwhmIter / 2.0));
            }
         }
         else if (endWavelengths.empty() == false)
         {
            // Compute startWavelengths from centerWavelengths and endWavelengths.
            if (centerWavelengths.size() != endWavelengths.size())
            {
               return false;
            }

            for (centerIter = centerWavelengths.begin(), endIter = endWavelengths.begin();
               centerIter != centerWavelengths.end() && endIter != endWavelengths.end();
               ++centerIter, ++endIter)
            {
               startWavelengths.push_back(*centerIter - (*endIter - *centerIter));
            }
         }
      }
   }

   // If no endWavelengths exist, compute them if possible.
   if (endWavelengths.empty() == true)
   {
      if (centerWavelengths.empty() == false)
      {
         if (fwhms.empty() == false)
         {
            // Compute endWavelengths from centerWavelengths and fwhms.
            if (centerWavelengths.size() != fwhms.size())
            {
               return false;
            }

            for (centerIter = centerWavelengths.begin(), fwhmIter = fwhms.begin();
               centerIter != centerWavelengths.end() && fwhmIter != fwhms.end();
               ++centerIter, ++fwhmIter)
            {
               endWavelengths.push_back(*centerIter + (*fwhmIter / 2.0));
            }
         }
         else if (startWavelengths.empty() == false)
         {
            // Compute endWavelengths from centerWavelengths and startWavelengths.
            if (centerWavelengths.size() != startWavelengths.size())
            {
               return false;
            }

            for (centerIter = centerWavelengths.begin(), startIter = startWavelengths.begin();
               centerIter != centerWavelengths.end() && startIter != startWavelengths.end();
               ++centerIter, ++startIter)
            {
               endWavelengths.push_back(*centerIter + (*centerIter - *startIter));
            }
         }
      }
   }

   // Write all values to the appropriate special metadata sections.
   bool success = true;
   if (centerWavelengths.empty() == false)
   {
      // Convert all values to microns if necessary.
      if (convertFromInverseCentimeters)
      {
         for (vector<double>::iterator iter = centerWavelengths.begin(); iter != centerWavelengths.end(); ++iter)
         {
            *iter = 10000.0 / *iter;
         }
      }

      success = pMetadata->setAttributeByPath(centerWavelengthsPath, centerWavelengths) && success;
   }

   if (startWavelengths.empty() == false)
   {
      // Convert all values to microns if necessary.
      if (convertFromInverseCentimeters)
      {
         for (vector<double>::iterator iter = startWavelengths.begin(); iter != startWavelengths.end(); ++iter)
         {
            *iter = 10000.0 / *iter;
         }
      }

      success = pMetadata->setAttributeByPath(startWavelengthsPath, startWavelengths) && success;
   }

   if (endWavelengths.empty() == false)
   {
      // Convert all values to microns if necessary.
      if (convertFromInverseCentimeters)
      {
         for (vector<double>::iterator iter = endWavelengths.begin(); iter != endWavelengths.end(); ++iter)
         {
            *iter = 10000.0 / *iter;
         }
      }

      success = pMetadata->setAttributeByPath(endWavelengthsPath, endWavelengths) && success;
   }

   return success;
}
