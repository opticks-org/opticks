/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "DataVariant.h"
#include "DateTime.h"
#include "NitfStdidbParser.h"
#include "NitfUtilities.h"
#include "NitfConstants.h"
#include "ObjectResource.h"
#include "PlugInRegistration.h"

#include <sstream>

using namespace std;
using namespace Nitf;
using namespace Nitf::TRE;

#include <boost/algorithm/string/case_conv.hpp>

REGISTER_PLUGIN(OpticksNitfCommonTre, StdidbParser, Nitf::StdidbParser());

Nitf::StdidbParser::StdidbParser()
{
   setName("STDIDB");
   setDescriptorId("{2203DDA4-282A-4ccb-8CC0-8167C761F51F}");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

bool Nitf::StdidbParser::runAllTests(Progress* pProgress, ostream& failure)
{
   static const string data(
      "11SEP01"                 // FIELD1
      "1234"                    // FIELD2
      "02"                      // FIELD3
      "123"                     // FIELD4
      "12"                      // FIELD5
      "12"                      // FIELD6
      "123"                     // FIELD7
      " "                       // FIELD8
      "12"                      // FIELD9
      "12345"                   // FIELD10
      "22"                      // FIELD11
      "21"                      // FIELD12
      "54321"                   // FIELD13
      "US"                      // FIELD14
      "1234"                    // FIELD15
      "1234E12345N"             // FIELD16
      "0815Z11SEP01"                   // FIELD17
      );

   static const string data_error4(
      "11SEP01"                 // FIELD1
      "1234"                    // FIELD2
      "02"                      // FIELD3
      "123"                     // FIELD4
      "12"                      // FIELD5
      "12"                      // FIELD6
      "123"                     // FIELD7
      " "                       // FIELD8
      "00"                      // FIELD9       // ERROR data out of range
      "12345"                   // FIELD10
      "22"                      // FIELD11
      "21"                      // FIELD12
      "54321"                   // FIELD13
      "US"                      // FIELD14
      "1234"                    // FIELD15
      "1234E12345N"             // FIELD16
      "0815Z11SEP01"                   // FIELD17
      );

   FactoryResource<DynamicObject> treDO;
   size_t numBytes(0);

   istringstream input(data);
   numBytes = data.size();
   string errorMessage;

   bool success = toDynamicObject(input, numBytes, *treDO.get(), errorMessage);

   if (!errorMessage.empty())
   {
      failure << errorMessage << endl;
      errorMessage.clear();
   }

   TreState status(INVALID);
   if (success)
   {
      status = isTreValid(*treDO.get(), failure);
   }

   treDO->clear();

   if (status == INVALID)
   {
      return false;
   }

   // Start of test 2 - Negative test: 1 extra byte in input stream
   stringstream input2(data);
   input2 << "1";          // Add one more byte; valid as alphanumberic or numeric
   numBytes = data.size() + 1;
   success = toDynamicObject(input2, numBytes, *treDO.get(), errorMessage);
   if (success == true)     // negative test so success must == false.
   {
      failure << "Error: Negative test of 1 extra byte failed\n";
      treDO->clear();
      return false;
   }

   // start of test 3 - Negative test: 1 byte short in input stream
   string negdata3(data);        // data for test 3 not the 3rd data set
   negdata3.resize(data.size()-1);
   stringstream input3(negdata3);
   numBytes = negdata3.size();
   success = toDynamicObject(input3, numBytes, *treDO.get(), errorMessage);
   if (success == true)  // negative test so success must == false.
   {
      failure << "Error: Negative test of 1 byte short failed\n";
      treDO->clear();
      return false;
   }

   treDO->clear();

   // Start of test 4 - SUSPECT test
   stringstream input4(data_error4);
   numBytes = input4.str().size();

   success = toDynamicObject(input4, numBytes, *treDO.get(), errorMessage);

   status = INVALID;
   if (success)
   {
      stringstream tmpStream;
      status = isTreValid(*treDO.get(), tmpStream);
      if (status != SUSPECT)
      {
         failure << "Error: Negative test with FIELD9 data out of range failed: did not return SUSPECT\n";
         failure << tmpStream.str();
         treDO->clear();
         return false;
      }
      status = VALID;
   }

   treDO->clear();

   return (status != INVALID);
}

bool Nitf::StdidbParser::toDynamicObject(istream& input, size_t numBytes, DynamicObject& output,
   string &errorMessage) const
{
   vector<char> buf;
   bool ok(true);

   bool success = readFromStream(input, buf, 7);  // DDMMMYY

   FactoryResource<DateTime> appDTG;

   string dtg;
   dtg.resize(8);
   memcpy(&dtg[0], &buf[0], 8);

   unsigned short year(0);
   unsigned short month(0);
   unsigned short day(0);

   if (success)
   {
      success = DtgParseDDMMMYY(dtg, year, month, day);
   }

   if (success)
   {
      success = appDTG->set(year, month, day);
      if (success)
      {
         success = output.setAttribute(STDIDB::FIELD1, *appDTG.get());
      }
   }
   if (!success)
   {
      errorMessage += "Parsing " + STDIDB::FIELD1 + " failed\n";
   }

   readField<string>(input, output, success, STDIDB::FIELD2, 4, errorMessage, buf);
   readField<string>(input, output, success, STDIDB::FIELD3, 2, errorMessage, buf);
   readField<int>(input, output, success, STDIDB::FIELD4, 3, errorMessage, buf);
   readField<string>(input, output, success, STDIDB::FIELD5, 2, errorMessage, buf);
   readField<int>(input, output, success, STDIDB::FIELD6, 2, errorMessage, buf);
   readField<string>(input, output, success, STDIDB::FIELD7, 3, errorMessage, buf);
   readField<string>(input, output, success, STDIDB::FIELD8, 1, errorMessage, buf, true);
   readField<int>(input, output, success, STDIDB::FIELD9, 2, errorMessage, buf);
   readField<int>(input, output, success, STDIDB::FIELD10, 5, errorMessage, buf);
   readField<string>(input, output, success, STDIDB::FIELD11, 2, errorMessage, buf);
   readField<int>(input, output, success, STDIDB::FIELD12, 2, errorMessage, buf);
   readField<int>(input, output, success, STDIDB::FIELD13, 5, errorMessage, buf);
   readField<string>(input, output, success, STDIDB::FIELD14, 2, errorMessage, buf, true);
   readField<int>(input, output, success, STDIDB::FIELD15, 4, errorMessage, buf, true);
   readField<string>(input, output, success, STDIDB::FIELD16, 11, errorMessage, buf, true);

   if (success)
   {
      success = success && readFromStream(input, buf, 5);  // FIELD17
      if (!success)
      {
         errorMessage += "Reading " + STDIDB::FIELD17 + " failed\n";
      }
   }

   string str;
   str.resize(6);
   memcpy(&str[0], &buf[0], 6);
   unsigned short hour(0);
   unsigned short min(0);
   unsigned short sec(0);
   if (success)
   {
      success = DtgParseHHMM(str, hour, min);
      if (!success)
      {
         errorMessage += "Parsing " + STDIDB::FIELD17 + " failed\n";
      }
   }

   if (success)
   {
      success = readFromStream(input, buf, 7);  // FIELD17
      if (!success)
      {
         errorMessage += "Reading " + STDIDB::FIELD17 + " failed\n";
      }

      if (success)
      {
         memcpy(&dtg[0], &buf[0], 8);
         success = DtgParseDDMMMYY(dtg, year, month, day);
         if (!success)
         {
            errorMessage += "Parsing " + STDIDB::FIELD17 + " part failed\n";
         }
      }
   }

   if (success)
   {
      success = appDTG->set(year, month, day, hour, min, sec);
      if (success)
      {
         success = output.setAttribute(STDIDB::FIELD17, *appDTG.get());
      }
      if (!success)
      {
         errorMessage += "setting " + STDIDB::FIELD17 + " failed\n";
      }
   }

   size_t numRead = input.tellg();
   if (numRead != numBytes)
   {
      numReadErrMsg(numRead, numBytes, errorMessage);
      return false;
   }

   return success;
}

Nitf::TreState Nitf::StdidbParser::isTreValid(const DynamicObject& tre, ostream& reporter) const
{
   TreState status(VALID);
   set<string>             testSet;
   unsigned int numFields = 0;

   if (status != INVALID)
   {
      // Just check that the field exists. Assume the DateTime class will contain a vaild data/FIELD17
      string fieldName = STDIDB::FIELD1;

      if (tre.getAttribute(fieldName).getPointerToValue<DateTime>() == NULL)
      {
         reporter << "Field \"" << fieldName << "\" missing from the Dynamic Object";
         status = INVALID;
      }
      else
      {
         ++numFields;
      }
   }

   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, STDIDB::FIELD2, testSet, false, true, false));

   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, STDIDB::FIELD3, testSet, false, true, false));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, STDIDB::FIELD4, 1, 999));

   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, STDIDB::FIELD5, testSet, false, true, false));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, STDIDB::FIELD6, 0, 99));

   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, STDIDB::FIELD7, testSet, true, true, false));

   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, STDIDB::FIELD8, testSet, true, false, true));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, STDIDB::FIELD9, 1, 99));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, STDIDB::FIELD10, 1, 99999));

   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, STDIDB::FIELD11, testSet, false, true, false));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, STDIDB::FIELD12, 1, 99));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, STDIDB::FIELD13, 1, 99999));

   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, STDIDB::FIELD14, testSet, true, true, false));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, STDIDB::FIELD15, 1, 9999));

   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, STDIDB::FIELD16, testSet, false, true, false));

   // Just check that the field exists. Assume the DateTime class will contain vaild data
   string fieldName = STDIDB::FIELD17;

   FactoryResource<DateTime> appDTG;
   if (tre.getAttribute(fieldName).getPointerToValue<DateTime>() == NULL)
   {
      reporter << "Field \"" << fieldName << "\" missing from the Dynamic Object";
      status = INVALID;
   }
   else
   {
      ++numFields;
   }

   unsigned int totalFields = tre.getNumAttributes();
   if (status != INVALID && totalFields != numFields)
   {
      reporter << "Total fields in the Dynamic Object(" <<
         totalFields << ") did not match the number found(" << numFields << ") ";
      status = INVALID;
   }

   if (status == INVALID)
   {
      reporter << " INVALID fields found in the STDIDB TAG/SDE\n" ;
   }
   else if (status == SUSPECT)
   {
      reporter << " SUSPECT fields found in the STDIDB TAG/SDE\n" ;
   }

   return status;
}

bool Nitf::StdidbParser::fromDynamicObject(const DynamicObject& input, ostream& output, size_t& numBytesWritten,
   string &errorMessage) const
{
   size_t sizeIn = max(static_cast<ostream::pos_type>(0), output.tellp());
   size_t sizeOut(sizeIn);

   try
   {
      const DateTime* pMissionDTG = dv_cast<DateTime>(&input.getAttribute(STDIDB::FIELD1));
      if (pMissionDTG == NULL)
      {
         return false;
      }

      // put date in form DDMMMYY for this TAG    see: strfFIELD17() for format info
      string ddmmmyy = pMissionDTG->getFormattedUtc("%d%b%y");
      boost::to_upper(ddmmmyy);
      output << sizeString(ddmmmyy, 7);

      output << sizeString(dv_cast<string>(input.getAttribute(STDIDB::FIELD2)), 4);
      output << sizeString(dv_cast<string>(input.getAttribute(STDIDB::FIELD3)), 2);
      output << toString(dv_cast<int>(input.getAttribute(STDIDB::FIELD4)), 3);
      output << sizeString(dv_cast<string>(input.getAttribute(STDIDB::FIELD5)), 2);
      output << toString(dv_cast<int>(input.getAttribute(STDIDB::FIELD6)), 2);
      output << sizeString(dv_cast<string>(input.getAttribute(STDIDB::FIELD7)), 3);
      output << sizeString(dv_cast<string>(input.getAttribute(STDIDB::FIELD8)), 1);
      output << toString(dv_cast<int>(input.getAttribute(STDIDB::FIELD9)), 2);
      output << toString(dv_cast<int>(input.getAttribute(STDIDB::FIELD10)), 5);
      output << sizeString(dv_cast<string>(input.getAttribute(STDIDB::FIELD11)), 2);
      output << toString(dv_cast<int>(input.getAttribute(STDIDB::FIELD12)), 2);
      output << toString(dv_cast<int>(input.getAttribute(STDIDB::FIELD13)), 5);
      output << sizeString(dv_cast<string>(input.getAttribute(STDIDB::FIELD14)), 2);
      output << toString(dv_cast<int>(input.getAttribute(STDIDB::FIELD15)), 4);
      output << sizeString(dv_cast<string>(input.getAttribute(STDIDB::FIELD16)), 11);

      // NOTE: the create DATE and FIELD17 fields are combined into one.

      const DateTime* pCreateDTG = dv_cast<DateTime>(&input.getAttribute(STDIDB::FIELD17));
      if (pCreateDTG == NULL)
      {
         return false;
      }

      string mission_FIELD17 = pMissionDTG->getFormattedUtc("%H%M") + "Z";    // FIELD17
      output << sizeString(mission_FIELD17, 5);

      ddmmmyy = pCreateDTG->getFormattedUtc("%d%b%y");     // FIELD17 cont
      boost::to_upper(ddmmmyy);
      output << sizeString(ddmmmyy, 7);

   }
   catch (const bad_cast&)
   {
      return false;
   }

   sizeOut = output.tellp();
   numBytesWritten = sizeOut - sizeIn;
   return true;
}
