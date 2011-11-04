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
#include "NitfConstants.h"
#include "NitfStdidcParser.h"
#include "NitfUtilities.h"
#include "ObjectResource.h"
#include "PlugInRegistration.h"

#include <sstream>
#include <string>

#include <boost/algorithm/string/case_conv.hpp>

using namespace std;
using namespace Nitf;
using namespace Nitf::TRE;

REGISTER_PLUGIN(OpticksNitfCommonTre, StdidcParser, Nitf::StdidcParser());

Nitf::StdidcParser::StdidcParser()
{
   setName("STDIDC");
   setDescriptorId("{ABAE710F-DD0C-486d-908D-F22782166993}");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

bool Nitf::StdidcParser::runAllTests(Progress* pProgress, ostream& failure)
{
   static const string data(
      "20000108180902"                  // ACQUISITION_DATE
      "ABCXYZ_0123456"                  // MISSION
      "01"                              // PASS
      "528"                             // OP_NUM
      "AA"                              // START_SEGMENT
      "00"                              // REPRO_NUM
      "000"                             // REPLAY_REGEN
      " "                               // BLANK_FILL
      "001"                             // START_COLUMN
      "00001"                           // START_ROW
      "AA"                              // END_SEGMENT
      "012"                             // END_COLUMN
      "00027"                           // END_ROW
      "US"                              // COUNTRY
      "0404"                            // WAC
      "3251N11715W"                     // LOCATION
      "     "                           // RESERVED2
      "        "                        // RESERVED3
      );

   static const string data_error4(
      "20000108180902"                  // ACQUISITION_DATE
      "ABCXYZ_0123456"                  // MISSION
      "01"                              // PASS
      "528"                             // OP_NUM
      "AA"                              // START_SEGMENT
      "00"                              // REPRO_NUM
      "000"                             // REPLAY_REGEN
      " "                               // BLANK_FILL
      "001"                             // START_COLUMN
      "00001"                           // START_ROW
      "AA"                              // END_SEGMENT
      "012"                             // END_COLUMN
      "00027"                           // END_ROW
      "US"                              // COUNTRY
      "1867"                            // WAC - max == 1866
      "3251N11715W"                     // LOCATION
      "     "                           // RESERVED2
      "        "                        // RESERVED3
      );

   static const string data5(
      "20000108180902"                  // ACQUISITION_DATE
      "ABCXYZ_0123456"                  // MISSION
      "01"                              // PASS
      "528"                             // OP_NUM
      "AA"                              // START_SEGMENT
      "00"                              // REPRO_NUM
      "000"                             // REPLAY_REGEN
      " "                               // BLANK_FILL
      "001"                             // START_COLUMN
      "00001"                           // START_ROW
      "AA"                              // END_SEGMENT
      "012"                             // END_COLUMN
      "00027"                           // END_ROW
      "US"                              // COUNTRY
      "    "                            // WAC - set to spaces
      "3251N11715W"                     // LOCATION
      "     "                           // RESERVED2
      "        "                        // RESERVED3
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
         failure << "Error: Negative test with data out of range failed: did not return SUSPECT\n";
         failure << tmpStream.str();
         treDO->clear();
         return false;
      }
      status = VALID;
   }

   treDO->clear();

   // Start of test 5 - blanks in optional fields
   errorMessage.clear();
   stringstream input5(data5);
   success = toDynamicObject(input5, input5.str().size(), *treDO.get(), errorMessage);
   if (success == false)
   {
      failure << errorMessage;
      return false;
   }
   else
   {
      stringstream tmpStream;
      status = isTreValid(*treDO.get(), tmpStream);
      if (status != VALID)
      {
         failure << "Error: Test with blank data failed: did not return VALID\n";
         failure << tmpStream.str();
         return false;
      }

      tmpStream.str(string());
      success = fromDynamicObject(*treDO.get(), tmpStream, numBytes, errorMessage);
      if (success == false)
      {
         failure << errorMessage;
         return false;
      }

      if (input5.str() != tmpStream.str())
      {
         failure << "Error: Test with blank data failed: fromDynamicObject returned an unexpected value\n";
         return false;
      }
   }

   treDO->clear();
   return true;
}

bool Nitf::StdidcParser::toDynamicObject(istream& input, size_t numBytes, DynamicObject& output,
   string &errorMessage) const
{
   vector<char> buf;
   bool success(true);

   success = readFromStream(input, buf, 14);  // CCYYMMDDHHMMSS

   FactoryResource<DateTime> appDTG;

   string dtg;
   dtg.resize(15);
   memcpy(&dtg[0], &buf[0], 15);

   bool dateValid(false);
   bool timeValid(false);
   unsigned short year(0);
   unsigned short month(0);
   unsigned short day(0);
   unsigned short hour(0);
   unsigned short min(0);
   unsigned short sec(0);

   if (success)
   {
      success = DtgParseCCYYMMDDhhmmss(dtg, year, month, day, hour, min, sec, &dateValid, &timeValid);
   }

   if (success)
   {
      success = appDTG->set(year, month, day, hour, min, sec);
      if (success)
      {
         success = output.setAttribute(STDIDC::ACQUISITION_DATE, *appDTG.get());
      }
   }
   if (!success)
   {
      errorMessage += "Parsing " + STDIDC::ACQUISITION_DATE + " failed\n";
   }

   readField<string>(input, output, success, STDIDC::MISSION, 14, errorMessage, buf);
   readField<string>(input, output, success, STDIDC::PASS, 2, errorMessage, buf);
   readField<unsigned int>(input, output, success, STDIDC::OP_NUM, 3, errorMessage, buf);
   readField<string>(input, output, success, STDIDC::START_SEGMENT, 2, errorMessage, buf);
   readField<unsigned int>(input, output, success, STDIDC::REPRO_NUM, 2, errorMessage, buf);
   readField<string>(input, output, success, STDIDC::REPLAY_REGEN, 3, errorMessage, buf);
   readField<string>(input, output, success, STDIDC::BLANK_FILL, 1, errorMessage, buf, true);
   readField<unsigned int>(input, output, success, STDIDC::START_COLUMN, 3, errorMessage, buf);
   readField<unsigned int>(input, output, success, STDIDC::START_ROW, 5, errorMessage, buf);
   readField<string>(input, output, success, STDIDC::END_SEGMENT, 2, errorMessage, buf);
   readField<unsigned int>(input, output, success, STDIDC::END_COLUMN, 3, errorMessage, buf);
   readField<unsigned int>(input, output, success, STDIDC::END_ROW, 5, errorMessage, buf);
   readField<string>(input, output, success, STDIDC::COUNTRY, 2, errorMessage, buf, true);
   readField<unsigned int>(input, output, success, STDIDC::WAC, 4, errorMessage, buf, true);
   readField<string>(input, output, success, STDIDC::LOCATION, 11, errorMessage, buf);
   readField<string>(input, output, success, STDIDC::RESERVED2, 5, errorMessage, buf, true);
   readField<string>(input, output, success, STDIDC::RESERVED3, 8, errorMessage, buf, true);

   int64_t numRead = input.tellg();
   if (numRead < 0 || numRead > static_cast<int64_t>(std::numeric_limits<size_t>::max()) ||
      numRead != static_cast<int64_t>(numBytes))
   {
      numReadErrMsg(numRead, numBytes, errorMessage);
      return false;
   }

   return success;
}


Nitf::TreState Nitf::StdidcParser::isTreValid(const DynamicObject& tre, ostream& reporter) const
{
   TreState status(VALID);
   set<string>             testSet;
   unsigned int numFields = 0;

   if (status != INVALID)
   {
      // Just check that the field exists. Assume the DateTime class will contain a vaild date/time
      string fieldName = STDIDC::ACQUISITION_DATE;

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


   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, STDIDC::MISSION, testSet, true, true, false));

   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, STDIDC::PASS, testSet, true, true, false));

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, STDIDC::OP_NUM, 0U, 999U));

   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, STDIDC::START_SEGMENT, testSet, true, true, false));

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, STDIDC::REPRO_NUM, 0U, 99U));

   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, STDIDC::REPLAY_REGEN, testSet, true, true, false));

   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, STDIDC::BLANK_FILL, testSet, true, true, false));

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, STDIDC::START_COLUMN, 1U, 999U));

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, STDIDC::START_ROW, 1U, 99999U));

   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, STDIDC::END_SEGMENT, testSet, true, true, false));

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, STDIDC::END_COLUMN, 1U, 999U));

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, STDIDC::END_ROW, 1U, 99999U));

   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, STDIDC::COUNTRY, testSet, true, true, false));

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, STDIDC::WAC, 1U, 1866U, true));

   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, STDIDC::LOCATION, testSet, true, true, false));

   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, STDIDC::RESERVED2, testSet, true, true, false));

   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, STDIDC::RESERVED3, testSet, true, true, false));

   unsigned int totalFields = tre.getNumAttributes();
   if (status != INVALID && totalFields != numFields)
   {
      reporter << "Total fields in the Dynamic Object(" <<
         totalFields << ") did not match the number found(" << numFields << ") ";
      status = INVALID;
   }

   if (status == INVALID)
   {
      reporter << " INVALID fields found in the STDIDC TAG/SDE\n" ;
   }
   else if (status == SUSPECT)
   {
      reporter << " SUSPECT fields found in the STDIDC TAG/SDE\n" ;
   }

   return status;
}


bool Nitf::StdidcParser::fromDynamicObject(const DynamicObject& input, ostream& output, size_t& numBytesWritten,
   string &errorMessage) const
{
   if (output.tellp() < 0 || output.tellp() > static_cast<int64_t>(std::numeric_limits<size_t>::max()))
   {
      return false;
   }
   size_t sizeIn = max<size_t>(0, static_cast<size_t>(output.tellp()));
   size_t sizeOut(sizeIn);

   try
   {
      const DateTime* pMissionDTG = dv_cast<DateTime>(&input.getAttribute(STDIDC::ACQUISITION_DATE));
      if (pMissionDTG == NULL)
      {
         return false;
      }

      // put date in form CCYYMMDDHHMMSS for this TAG    see: strftime() for format info
      string yyyymmddhhmmss = pMissionDTG->getFormattedUtc("%Y%m%d%H%M%S");
      boost::to_upper(yyyymmddhhmmss);
      output << sizeString(yyyymmddhhmmss, 14);

      output << sizeString(dv_cast<string>(input.getAttribute(STDIDC::MISSION)), 14);
      output << sizeString(dv_cast<string>(input.getAttribute(STDIDC::PASS)), 2);
      output << toString(dv_cast<unsigned int>(input.getAttribute(STDIDC::OP_NUM)), 3, -1);
      output << sizeString(dv_cast<string>(input.getAttribute(STDIDC::START_SEGMENT)), 2);
      output << toString(dv_cast<unsigned int>(input.getAttribute(STDIDC::REPRO_NUM)), 2, -1);
      output << sizeString(dv_cast<string>(input.getAttribute(STDIDC::REPLAY_REGEN)), 3);
      output << sizeString(dv_cast<string>(input.getAttribute(STDIDC::BLANK_FILL)), 1);
      output << toString(dv_cast<unsigned int>(input.getAttribute(STDIDC::START_COLUMN)), 3, -1);
      output << toString(dv_cast<unsigned int>(input.getAttribute(STDIDC::START_ROW)), 5, -1);
      output << sizeString(dv_cast<string>(input.getAttribute(STDIDC::END_SEGMENT)), 2);
      output << toString(dv_cast<unsigned int>(input.getAttribute(STDIDC::END_COLUMN)), 3, -1);
      output << toString(dv_cast<unsigned int>(input.getAttribute(STDIDC::END_ROW)), 5, -1);
      output << sizeString(dv_cast<string>(input.getAttribute(STDIDC::COUNTRY)), 2);
      output << toString(dv_cast<unsigned int>(input.getAttribute(STDIDC::WAC)),
         4, -1, ZERO_FILL, false, false, 3, true);
      output << sizeString(dv_cast<string>(input.getAttribute(STDIDC::LOCATION)), 11);
      output << sizeString(dv_cast<string>(input.getAttribute(STDIDC::RESERVED2)), 5);
      output << sizeString(dv_cast<string>(input.getAttribute(STDIDC::RESERVED3)), 8);
   }
   catch (const bad_cast&)
   {
      return false;
   }

   if (output.tellp() < 0 || output.tellp() > static_cast<int64_t>(std::numeric_limits<size_t>::max()))
   {
      return false;
   }
   sizeOut = static_cast<size_t>(output.tellp());
   numBytesWritten = sizeOut - sizeIn;
   return true;
}
