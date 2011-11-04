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
#include "NitfMpd26aParser.h"
#include "NitfUtilities.h"
#include "ObjectResource.h"
#include "PlugInRegistration.h"

#include <sstream>
#include <string>

using namespace std;
using namespace Nitf;
using namespace Nitf::TRE;

REGISTER_PLUGIN(OpticksNitfCommonTre, Mpd26aParser, Nitf::Mpd26aParser());

Nitf::Mpd26aParser::Mpd26aParser()
{
   setName("MPD26A");
   setDescriptorId("{5F4E744E-9CA1-4974-9A03-2E41371F4074}");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

bool Nitf::Mpd26aParser::runAllTests(Progress* pProgress, ostream& failure)
{
   static const string data(
      "99999999999"              // FIELD1
      "ab"                       // FIELD2
      "9999999"                  // FIELD3
      "999999.999"               // FIELD4
      "99.999"                   // FIELD5
      "999.99999"                // FIELD6
      "abc"                      // FIELD7
      "9"                        // FIELD8
      "A"                        // FIELD9
      "-99.9"                    // FIELD10
      "999.9"                    // FIELD11
      "0"                        // FIELD12
      "9"                        // FIELD13
      "0999"                     // FIELD14
      "099999999"                // FIELD15
      "-99999999"                // FIELD16
      "099999999"                // FIELD17
      "999999.999"               // FIELD18
      "099999999"                // FIELD19
      "-99999999"                // FIELD20
      "099999999"                // FIELD21
      "-99999999"                // FIELD22
      "099999999"                // FIELD23
      "-99999999"                // FIELD24
      "099999999"                // FIELD25
      "-99999999"                // FIELD26
      "099999999"                // FIELD27
      "-99999999"                // FIELD28
      "099999999"                // FIELD29
      "-99999999"                // FIELD30
      "099999999"                // FIELD31
      "-99999999"                // FIELD32
      "099999999"                // FIELD33
      "-99999999"                // FIELD34
      "099999999"                // FIELD35
      "-99999999"                // FIELD36
      "099999999"                // FIELD37
      "-99999999"                // FIELD38
      "099999999"                // FIELD39
      "-99999999"                // FIELD40
      "099999999"                // FIELD41
      "-99999999"                // FIELD42
      "099999999"                // FIELD43
      "-99999999"                // FIELD44
      "099999999"                // FIELD45
      );

   static const string data_error4(
      "99999999999"              // FIELD1
      "ab"                       // FIELD2
      "9999999"                  // FIELD3
      "999999.999"               // FIELD4
      "99.999"                   // FIELD5
      "999.99999"                // FIELD6
      "abc"                      // FIELD7
      "9"                        // FIELD8
      "A"                        // FIELD9
      "-99.9"                    // FIELD10
      "999.9"                    // FIELD11
      "0"                        // FIELD12
      "9"                        // FIELD13
      "1000"                     // FIELD14    max == 999
      "099999999"                // FIELD15
      "-99999999"                // FIELD16
      "099999999"                // FIELD17
      "999999.999"               // FIELD18
      "099999999"                // FIELD19
      "-99999999"                // FIELD20
      "099999999"                // FIELD21
      "-99999999"                // FIELD22
      "099999999"                // FIELD23
      "-99999999"                // FIELD24
      "099999999"                // FIELD25
      "-99999999"                // FIELD26
      "099999999"                // FIELD27
      "-99999999"                // FIELD28
      "099999999"                // FIELD29
      "-99999999"                // FIELD30
      "099999999"                // FIELD31
      "-99999999"                // FIELD32
      "099999999"                // FIELD33
      "-99999999"                // FIELD34
      "099999999"                // FIELD35
      "-99999999"                // FIELD36
      "099999999"                // FIELD37
      "-99999999"                // FIELD38
      "099999999"                // FIELD39
      "-99999999"                // FIELD40
      "099999999"                // FIELD41
      "-99999999"                // FIELD42
      "099999999"                // FIELD43
      "-99999999"                // FIELD44
      "099999999"                // FIELD45
      );

   static const string data5(
      "99999999999"              // FIELD1
      "ab"                       // FIELD2
      "9999999"                  // FIELD3
      "999999.999"               // FIELD4
      "99.999"                   // FIELD5
      "999.99999"                // FIELD6
      "abc"                      // FIELD7
      "9"                        // FIELD8
      "A"                        // FIELD9
      "-99.9"                    // FIELD10
      "999.9"                    // FIELD11
      "0"                        // FIELD12
      "9"                        // FIELD13
      "-999"                     // FIELD14
      "099999999"                // FIELD15
      "-99999999"                // FIELD16
      "099999999"                // FIELD17
      "999999.999"               // FIELD18
      "099999999"                // FIELD19
      "-99999999"                // FIELD20
      "099999999"                // FIELD21
      "-99999999"                // FIELD22
      "099999999"                // FIELD23
      "-99999999"                // FIELD24
      "099999999"                // FIELD25
      "-99999999"                // FIELD26
      "099999999"                // FIELD27
      "-99999999"                // FIELD28
      "099999999"                // FIELD29
      "-99999999"                // FIELD30
      "099999999"                // FIELD31
      "-99999999"                // FIELD32
      "099999999"                // FIELD33
      "-99999999"                // FIELD34
      "099999999"                // FIELD35
      "-99999999"                // FIELD36
      "099999999"                // FIELD37
      "-99999999"                // FIELD38
      "099999999"                // FIELD39
      "-99999999"                // FIELD40
      "099999999"                // FIELD41
      "-99999999"                // FIELD42
      "099999999"                // FIELD43
      "-99999999"                // FIELD44
      "099999999"                // FIELD45
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
   input2 << "1";          // Add one more byte; valid as alphanumeric or numeric
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

   if (status == INVALID)
   {
      return false;
   }

   istringstream input5(data5);
   numBytes = data5.size();

   errorMessage.clear();
   success = toDynamicObject(input5, numBytes, *treDO.get(), errorMessage);

   if (!errorMessage.empty())
   {
      failure << errorMessage << endl;
      errorMessage.clear();
   }

   if (success)
   {
      status = isTreValid(*treDO.get(), failure);
   }
   else
   {
      status = INVALID;
   }

   if (status == INVALID)
   {
      return false;
   }

   stringstream output5;
   size_t numBytesWritten = 0;
   success = fromDynamicObject(*treDO.get(), output5, numBytesWritten, errorMessage);
   if (!errorMessage.empty())
   {
      failure << errorMessage << endl;
      errorMessage.clear();
   }

   if (success == false || numBytesWritten != numBytes || output5.str() != input5.str())
   {
      failure << "test5 failed" << endl;
      return false;
   }

   treDO->clear();

   return (status != INVALID);
}

bool Nitf::Mpd26aParser::toDynamicObject(istream& input, size_t numBytes, DynamicObject& output,
   string &errorMessage) const
{
   vector<char> buf;
   bool success(true);

   // Allow all blanks for these fields since we do not have descriptions of them.
   readField<string>(input, output, success, MPD26A::FIELD1, 11, errorMessage, buf, true);
   readField<string>(input, output, success, MPD26A::FIELD2, 2, errorMessage, buf, true);
   readField<unsigned int>(input, output, success, MPD26A::FIELD3, 7, errorMessage, buf, true);
   readField<double>(input, output, success, MPD26A::FIELD4, 10, errorMessage, buf, true);
   readField<double>(input, output, success, MPD26A::FIELD5, 6, errorMessage, buf, true);
   readField<double>(input, output, success, MPD26A::FIELD6, 9, errorMessage, buf, true);
   readField<string>(input, output, success, MPD26A::FIELD7, 3, errorMessage, buf, true);
   readField<unsigned int>(input, output, success, MPD26A::FIELD8, 1, errorMessage, buf, true);
   readField<string>(input, output, success, MPD26A::FIELD9, 1, errorMessage, buf, true);
   readField<double>(input, output, success, MPD26A::FIELD10, 5, errorMessage, buf, true);
   readField<double>(input, output, success, MPD26A::FIELD11, 5, errorMessage, buf, true);
   readField<unsigned int>(input, output, success, MPD26A::FIELD12, 1, errorMessage, buf, true);
   readField<unsigned int>(input, output, success, MPD26A::FIELD13, 1, errorMessage, buf, true);
   readField<int>(input, output, success, MPD26A::FIELD14, 4, errorMessage, buf, true);
   readField<int>(input, output, success, MPD26A::FIELD15, 9, errorMessage, buf, true);
   readField<int>(input, output, success, MPD26A::FIELD16, 9, errorMessage, buf, true);
   readField<int>(input, output, success, MPD26A::FIELD17, 9, errorMessage, buf, true);
   readField<double>(input, output, success, MPD26A::FIELD18, 10, errorMessage, buf, true);
   readField<int>(input, output, success, MPD26A::FIELD19, 9, errorMessage, buf, true);
   readField<int>(input, output, success, MPD26A::FIELD20, 9, errorMessage, buf, true);
   readField<int>(input, output, success, MPD26A::FIELD21, 9, errorMessage, buf, true);
   readField<int>(input, output, success, MPD26A::FIELD22, 9, errorMessage, buf, true);
   readField<int>(input, output, success, MPD26A::FIELD23, 9, errorMessage, buf, true);
   readField<int>(input, output, success, MPD26A::FIELD24, 9, errorMessage, buf, true);
   readField<int>(input, output, success, MPD26A::FIELD25, 9, errorMessage, buf, true);
   readField<int>(input, output, success, MPD26A::FIELD26, 9, errorMessage, buf, true);
   readField<int>(input, output, success, MPD26A::FIELD27, 9, errorMessage, buf, true);
   readField<int>(input, output, success, MPD26A::FIELD28, 9, errorMessage, buf, true);
   readField<int>(input, output, success, MPD26A::FIELD29, 9, errorMessage, buf, true);
   readField<int>(input, output, success, MPD26A::FIELD30, 9, errorMessage, buf, true);
   readField<int>(input, output, success, MPD26A::FIELD31, 9, errorMessage, buf, true);
   readField<int>(input, output, success, MPD26A::FIELD32, 9, errorMessage, buf, true);
   readField<int>(input, output, success, MPD26A::FIELD33, 9, errorMessage, buf, true);
   readField<int>(input, output, success, MPD26A::FIELD34, 9, errorMessage, buf, true);
   readField<int>(input, output, success, MPD26A::FIELD35, 9, errorMessage, buf, true);
   readField<int>(input, output, success, MPD26A::FIELD36, 9, errorMessage, buf, true);
   readField<int>(input, output, success, MPD26A::FIELD37, 9, errorMessage, buf, true);
   readField<int>(input, output, success, MPD26A::FIELD38, 9, errorMessage, buf, true);
   readField<int>(input, output, success, MPD26A::FIELD39, 9, errorMessage, buf, true);
   readField<int>(input, output, success, MPD26A::FIELD40, 9, errorMessage, buf, true);
   readField<int>(input, output, success, MPD26A::FIELD41, 9, errorMessage, buf, true);
   readField<int>(input, output, success, MPD26A::FIELD42, 9, errorMessage, buf, true);
   readField<int>(input, output, success, MPD26A::FIELD43, 9, errorMessage, buf, true);
   readField<int>(input, output, success, MPD26A::FIELD44, 9, errorMessage, buf, true);
   readField<int>(input, output, success, MPD26A::FIELD45, 9, errorMessage, buf, true);

   int64_t numRead = input.tellg();
   if (numRead < 0 || numRead > static_cast<int64_t>(std::numeric_limits<size_t>::max()) ||
      numRead != static_cast<int64_t>(numBytes))
   {
      numReadErrMsg(numRead, numBytes, errorMessage);
      return false;
   }

   return success;
}


Nitf::TreState Nitf::Mpd26aParser::isTreValid(const DynamicObject& tre, ostream& reporter) const
{
   TreState status(VALID);
   set<string>             testSet;
   unsigned int numFields = 0;

   testSet.clear();

//#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : FIELD1 should be an int64.  " \
//   "Making it a char string for now. (lbeck)")
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, MPD26A::FIELD1, testSet, true, true, false));

   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, MPD26A::FIELD2, testSet, true, true, false));

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, MPD26A::FIELD3, 0U, 9999999U));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, MPD26A::FIELD4, 0.0F, 999999.999F));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, MPD26A::FIELD5, 0.0F, 99.999F));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, MPD26A::FIELD6, 0.0F, 999.99999F));

   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, MPD26A::FIELD7, testSet, true, true, false));

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, MPD26A::FIELD8, 0U, 9U));

   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, MPD26A::FIELD9, testSet, true, true, false));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, MPD26A::FIELD10, -99.9F, 99.9F));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, MPD26A::FIELD11, 0.0F, 999.9F));

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, MPD26A::FIELD12, 0U, 9U));

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, MPD26A::FIELD13, 0U, 9U));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, MPD26A::FIELD14, -999, 999));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, MPD26A::FIELD15, -99999999, 99999999));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, MPD26A::FIELD16, -99999999, 99999999));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, MPD26A::FIELD17, -99999999, 99999999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, MPD26A::FIELD18, 0.0F, 999999.999F));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, MPD26A::FIELD19, -99999999, 99999999));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, MPD26A::FIELD20, -99999999, 99999999));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, MPD26A::FIELD21, -99999999, 99999999));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, MPD26A::FIELD22, -99999999, 99999999));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, MPD26A::FIELD23, -99999999, 99999999));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, MPD26A::FIELD24, -99999999, 99999999));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, MPD26A::FIELD25, -99999999, 99999999));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, MPD26A::FIELD26, -99999999, 99999999));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, MPD26A::FIELD27, -99999999, 99999999));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, MPD26A::FIELD28, -99999999, 99999999));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, MPD26A::FIELD29, -99999999, 99999999));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, MPD26A::FIELD30, -99999999, 99999999));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, MPD26A::FIELD31, -99999999, 99999999));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, MPD26A::FIELD32, -99999999, 99999999));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, MPD26A::FIELD33, -99999999, 99999999));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, MPD26A::FIELD34, -99999999, 99999999));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, MPD26A::FIELD35, -99999999, 99999999));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, MPD26A::FIELD36, -99999999, 99999999));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, MPD26A::FIELD37, -99999999, 99999999));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, MPD26A::FIELD38, -99999999, 99999999));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, MPD26A::FIELD39, -99999999, 99999999));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, MPD26A::FIELD40, -99999999, 99999999));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, MPD26A::FIELD41, -99999999, 99999999));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, MPD26A::FIELD42, -99999999, 99999999));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, MPD26A::FIELD43, -99999999, 99999999));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, MPD26A::FIELD44, -99999999, 99999999));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, MPD26A::FIELD45, -99999999, 99999999));

   unsigned int totalFields = tre.getNumAttributes();
   if (status != INVALID && totalFields != numFields)
   {
      reporter << "Total fields in the Dynamic Object(" <<
         totalFields << ") did not match the number found(" << numFields << ") ";
      status = INVALID;
   }

   if (status == INVALID)
   {
      reporter << " INVALID fields found in the MPD26A TAG/SDE\n" ;
   }
   else if (status == SUSPECT)
   {
      reporter << " SUSPECT fields found in the MPD26A TAG/SDE\n" ;
   }

   return status;
}


bool Nitf::Mpd26aParser::fromDynamicObject(const DynamicObject& input, ostream& output, size_t& numBytesWritten,
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
      output << sizeString(dv_cast<string>(input.getAttribute(MPD26A::FIELD1)), 11);
      output << sizeString(dv_cast<string>(input.getAttribute(MPD26A::FIELD2)), 2);
      output << toString(dv_cast<unsigned int>(input.getAttribute(MPD26A::FIELD3)), 7, -1);
      output << toString(dv_cast<double>(input.getAttribute(MPD26A::FIELD4)), 10, 3);
      output << toString(dv_cast<double>(input.getAttribute(MPD26A::FIELD5)), 6, 3);
      output << toString(dv_cast<double>(input.getAttribute(MPD26A::FIELD6)), 9, 5);
      output << sizeString(dv_cast<string>(input.getAttribute(MPD26A::FIELD7)), 3);
      output << toString(dv_cast<unsigned int>(input.getAttribute(MPD26A::FIELD8)), 1, -1);
      output << sizeString(dv_cast<string>(input.getAttribute(MPD26A::FIELD9)), 1);
      output << toString(dv_cast<double>(input.getAttribute(MPD26A::FIELD10)), 5, 1, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<double>(input.getAttribute(MPD26A::FIELD11)), 5, 1);
      output << toString(dv_cast<unsigned int>(input.getAttribute(MPD26A::FIELD12)), 1, -1);
      output << toString(dv_cast<unsigned int>(input.getAttribute(MPD26A::FIELD13)), 1, -1);
      output << toString(dv_cast<int>(input.getAttribute(MPD26A::FIELD14)), 4, -1);
      output << toString(dv_cast<int>(input.getAttribute(MPD26A::FIELD15)), 9, -1);
      output << toString(dv_cast<int>(input.getAttribute(MPD26A::FIELD16)), 9, -1);
      output << toString(dv_cast<int>(input.getAttribute(MPD26A::FIELD17)), 9, -1);
      output << toString(dv_cast<double>(input.getAttribute(MPD26A::FIELD18)), 10, 3);
      output << toString(dv_cast<int>(input.getAttribute(MPD26A::FIELD19)), 9, -1);
      output << toString(dv_cast<int>(input.getAttribute(MPD26A::FIELD20)), 9, -1);
      output << toString(dv_cast<int>(input.getAttribute(MPD26A::FIELD21)), 9, -1);
      output << toString(dv_cast<int>(input.getAttribute(MPD26A::FIELD22)), 9, -1);
      output << toString(dv_cast<int>(input.getAttribute(MPD26A::FIELD23)), 9, -1);
      output << toString(dv_cast<int>(input.getAttribute(MPD26A::FIELD24)), 9, -1);
      output << toString(dv_cast<int>(input.getAttribute(MPD26A::FIELD25)), 9, -1);
      output << toString(dv_cast<int>(input.getAttribute(MPD26A::FIELD26)), 9, -1);
      output << toString(dv_cast<int>(input.getAttribute(MPD26A::FIELD27)), 9, -1);
      output << toString(dv_cast<int>(input.getAttribute(MPD26A::FIELD28)), 9, -1);
      output << toString(dv_cast<int>(input.getAttribute(MPD26A::FIELD29)), 9, -1);
      output << toString(dv_cast<int>(input.getAttribute(MPD26A::FIELD30)), 9, -1);
      output << toString(dv_cast<int>(input.getAttribute(MPD26A::FIELD31)), 9, -1);
      output << toString(dv_cast<int>(input.getAttribute(MPD26A::FIELD32)), 9, -1);
      output << toString(dv_cast<int>(input.getAttribute(MPD26A::FIELD33)), 9, -1);
      output << toString(dv_cast<int>(input.getAttribute(MPD26A::FIELD34)), 9, -1);
      output << toString(dv_cast<int>(input.getAttribute(MPD26A::FIELD35)), 9, -1);
      output << toString(dv_cast<int>(input.getAttribute(MPD26A::FIELD36)), 9, -1);
      output << toString(dv_cast<int>(input.getAttribute(MPD26A::FIELD37)), 9, -1);
      output << toString(dv_cast<int>(input.getAttribute(MPD26A::FIELD38)), 9, -1);
      output << toString(dv_cast<int>(input.getAttribute(MPD26A::FIELD39)), 9, -1);
      output << toString(dv_cast<int>(input.getAttribute(MPD26A::FIELD40)), 9, -1);
      output << toString(dv_cast<int>(input.getAttribute(MPD26A::FIELD41)), 9, -1);
      output << toString(dv_cast<int>(input.getAttribute(MPD26A::FIELD42)), 9, -1);
      output << toString(dv_cast<int>(input.getAttribute(MPD26A::FIELD43)), 9, -1);
      output << toString(dv_cast<int>(input.getAttribute(MPD26A::FIELD44)), 9, -1);
      output << toString(dv_cast<int>(input.getAttribute(MPD26A::FIELD45)), 9, -1);
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
