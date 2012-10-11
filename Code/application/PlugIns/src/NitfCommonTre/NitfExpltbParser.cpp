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
#include "NitfConstants.h"
#include "NitfExpltbParser.h"
#include "NitfUtilities.h"
#include "ObjectResource.h"
#include "PlugInRegistration.h"

#include <sstream>

using namespace std;
using namespace Nitf;
using namespace Nitf::TRE;

REGISTER_PLUGIN(OpticksNitfCommonTre, ExpltbParser, Nitf::ExpltbParser());

Nitf::ExpltbParser::ExpltbParser()
{
   setName("EXPLTB");
   setDescriptorId("{746D7743-6FC5-4e67-A0CA-1E92B501A87E}");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

bool Nitf::ExpltbParser::runAllTests(Progress* pProgress, ostream& failure)
{
   static const string data(
      "045.000"                 // ANGLE_TO_NORTH
      "20.000"                  // ANGLE_TO_NORTH_ACCY
      "-30.000"                 // SQUINT_ANGLE
      "44.999"                  // SQUINT_ANGLE_ACCY
      "3SP"                     // MODE
      "                "        // RESERVED1
      "45.00"                   // GRAZE_ANG
      "30.00"                   // GRAZE_ANG_ACCY
      "60.00"                   // SLOPE_ANG
      "HV"                      // POLAR
      "12345"                   // NSAMP
      "0"                       // RESERVED2
      "1"                       // SEQ_NUM
      "Prime ID    "            // PRIME_ID
      "Prime BE       "         // PRIME_BE
      "0"                       // RESERVED3
      "00"                      // N_SEC
      "00"                      // IPR
      );

   static const string data_error4(
      "045.000"                 // ANGLE_TO_NORTH
      "20.000"                  // ANGLE_TO_NORTH_ACCY
      "-30.000"                 // SQUINT_ANGLE
      "44.999"                  // SQUINT_ANGLE_ACCY
      "3SP"                     // MODE
      "                "        // RESERVED1
      "45.00"                   // GRAZE_ANG
      "30.00"                   // GRAZE_ANG_ACCY
      "60.00"                   // SLOPE_ANG
      "JK"                      // POLAR           // ERROR SUSPECT value
      "12345"                   // NSAMP
      "0"                       // RESERVED2
      "1"                       // SEQ_NUM
      "Prime ID    "            // PRIME_ID
      "Prime BE       "         // PRIME_BE
      "0"                       // RESERVED3
      "00"                      // N_SEC
      "00"                      // IPR
      );

   static const string data5(
      "045.000"                 // ANGLE_TO_NORTH
      "20.000"                  // ANGLE_TO_NORTH_ACCY
      "-30.000"                 // SQUINT_ANGLE
      "44.999"                  // SQUINT_ANGLE_ACCY
      "3SP"                     // MODE
      "                "        // RESERVED1
      "45.00"                   // GRAZE_ANG
      "30.00"                   // GRAZE_ANG_ACCY
      "60.00"                   // SLOPE_ANG
      "HV"                      // POLAR
      "12345"                   // NSAMP
      "0"                       // RESERVED2
      " "                       // SEQ_NUM - set to spaces
      "Prime ID    "            // PRIME_ID
      "Prime BE       "         // PRIME_BE
      "0"                       // RESERVED3
      "00"                      // N_SEC
      "00"                      // IPR
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

   // start of test 4 - Negative test: error in data returns SUSPECT
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
         failure << "Error: Negative test with POLAR data out of range failed: did not return SUSPECT\n";
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

bool Nitf::ExpltbParser::toDynamicObject(istream& input, size_t numBytes, DynamicObject& output,
   string &errorMessage) const
{
   vector<char> buf;
   bool success(true);
   readField<double>(input, output, success, EXPLTB::ANGLE_TO_NORTH, 7, errorMessage, buf);
   readField<double>(input, output, success, EXPLTB::ANGLE_TO_NORTH_ACCY, 6, errorMessage, buf);
   readField<double>(input, output, success, EXPLTB::SQUINT_ANGLE, 7, errorMessage, buf);
   readField<double>(input, output, success, EXPLTB::SQUINT_ANGLE_ACCY, 6, errorMessage, buf);
   readField<string>(input, output, success, EXPLTB::MODE, 3, errorMessage, buf);
   readField<string>(input, output, success, EXPLTB::RESERVED1, 16, errorMessage, buf, true);
   readField<double>(input, output, success, EXPLTB::GRAZE_ANG, 5, errorMessage, buf);
   readField<double>(input, output, success, EXPLTB::GRAZE_ANG_ACCY, 5, errorMessage, buf);
   readField<double>(input, output, success, EXPLTB::SLOPE_ANG, 5, errorMessage, buf);
   readField<string>(input, output, success, EXPLTB::POLAR, 2, errorMessage, buf);
   readField<int>(input, output, success, EXPLTB::NSAMP, 5, errorMessage, buf);
   readField<int>(input, output, success, EXPLTB::RESERVED2, 1, errorMessage, buf);
   readField<int>(input, output, success, EXPLTB::SEQ_NUM, 1, errorMessage, buf, true);
   readField<string>(input, output, success, EXPLTB::PRIME_ID, 12, errorMessage, buf, true);
   readField<string>(input, output, success, EXPLTB::PRIME_BE, 15, errorMessage, buf, true);
   readField<int>(input, output, success, EXPLTB::RESERVED3, 1, errorMessage, buf);
   readField<int>(input, output, success, EXPLTB::N_SEC, 2, errorMessage, buf);
   readField<int>(input, output, success, EXPLTB::IPR, 2, errorMessage, buf);

   int64_t numRead = input.tellg();
   if (numRead < 0 || static_cast<uint64_t>(numRead) > std::numeric_limits<size_t>::max() ||
      numRead != static_cast<int64_t>(numBytes))
   {
      numReadErrMsg(numRead, numBytes, errorMessage);
      return false;
   }

   return success;
}


Nitf::TreState Nitf::ExpltbParser::isTreValid(const DynamicObject& tre, ostream& reporter) const
{
   TreState status(VALID);
   set<string>             testSet;
   unsigned int numFields = 0;

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, EXPLTB::ANGLE_TO_NORTH, 0.0, 359.999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, EXPLTB::ANGLE_TO_NORTH_ACCY, 0.0, 44.999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, EXPLTB::SQUINT_ANGLE, -60., 85.0));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, EXPLTB::SQUINT_ANGLE_ACCY, 0.0, 44.999));

   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, EXPLTB::MODE, testSet, false, true, false));

   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, EXPLTB::RESERVED1, testSet, true, false, true));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, EXPLTB::GRAZE_ANG, 0.0, 90.00));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, EXPLTB::GRAZE_ANG_ACCY, 0.0, 90.00));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, EXPLTB::SLOPE_ANG, 0.0, 90.00));

   testSet.clear();
   testSet.insert("HH");
   testSet.insert("HV");
   testSet.insert("VH");
   testSet.insert("VV");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, EXPLTB::POLAR, testSet, false, false, true));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, EXPLTB::NSAMP, 1, 99999));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, EXPLTB::RESERVED2, 0, 0));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, EXPLTB::SEQ_NUM, 0, 6, true));

   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, EXPLTB::PRIME_ID, testSet, true, true, false));

   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, EXPLTB::PRIME_BE, testSet, true, true, false));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, EXPLTB::RESERVED3, 0, 0));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, EXPLTB::N_SEC, 0, 99));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, EXPLTB::IPR, 0, 99));

   unsigned int totalFields = tre.getNumAttributes();
   if (status != INVALID && totalFields != numFields)
   {
      reporter << "Total fields in the Dynamic Object(" <<
         totalFields << ") did not match the number found(" << numFields << ") ";
      status = INVALID;
   }

   if (status == INVALID)
   {
      reporter << " INVALID fields found in the EXPLTB TAG/SDE\n" ;
   }
   else if (status == SUSPECT)
   {
      reporter << " SUSPECT fields found in the EXPLTB TAG/SDE\n" ;
   }

   return status;
}



bool Nitf::ExpltbParser::fromDynamicObject(const DynamicObject& input, ostream& output, size_t& numBytesWritten,
   string &errorMessage) const
{
   if (output.tellp() < 0 || static_cast<uint64_t>(output.tellp()) > std::numeric_limits<size_t>::max())
   {
      return false;
   }
   size_t sizeIn = max<size_t>(0, static_cast<size_t>(output.tellp()));
   size_t sizeOut(sizeIn);

   try
   {
      output << toString(dv_cast<double>(input.getAttribute(EXPLTB::ANGLE_TO_NORTH)), 7, 3);
      output << toString(dv_cast<double>(input.getAttribute(EXPLTB::ANGLE_TO_NORTH_ACCY)), 6, 3);
      output << toString(dv_cast<double>(input.getAttribute(EXPLTB::SQUINT_ANGLE)), 7, 3, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<double>(input.getAttribute(EXPLTB::SQUINT_ANGLE_ACCY)), 6, 3);
      output << sizeString(dv_cast<string>(input.getAttribute(EXPLTB::MODE)), 3);
      output << sizeString(dv_cast<string>(input.getAttribute(EXPLTB::RESERVED1)), 16);
      output << toString(dv_cast<double>(input.getAttribute(EXPLTB::GRAZE_ANG)), 5, 2);
      output << toString(dv_cast<double>(input.getAttribute(EXPLTB::GRAZE_ANG_ACCY)), 5, 2);
      output << toString(dv_cast<double>(input.getAttribute(EXPLTB::SLOPE_ANG)), 5, 2);
      output << sizeString(dv_cast<string>(input.getAttribute(EXPLTB::POLAR)), 2);
      output << toString(dv_cast<int>(input.getAttribute(EXPLTB::NSAMP)), 5);
      output << toString(dv_cast<int>(input.getAttribute(EXPLTB::RESERVED2)), 1);
      output << toString(dv_cast<int>(input.getAttribute(EXPLTB::SEQ_NUM)), 1, -1, ZERO_FILL, false, false, 3, true);
      output << sizeString(dv_cast<string>(input.getAttribute(EXPLTB::PRIME_ID)), 12);
      output << sizeString(dv_cast<string>(input.getAttribute(EXPLTB::PRIME_BE)), 15);
      output << toString(dv_cast<int>(input.getAttribute(EXPLTB::RESERVED3)), 1);
      output << toString(dv_cast<int>(input.getAttribute(EXPLTB::N_SEC)), 2);
      output << toString(dv_cast<int>(input.getAttribute(EXPLTB::IPR)), 2);
   }
   catch (const bad_cast&)
   {
      return false;
   }

   if (output.tellp() < 0 || static_cast<uint64_t>(output.tellp()) > std::numeric_limits<size_t>::max())
   {
      return false;
   }
   sizeOut = static_cast<size_t>(output.tellp());
   numBytesWritten = sizeOut - sizeIn;
   return true;
}
