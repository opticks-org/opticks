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
#include "NitfUse26aParser.h"
#include "NitfUtilities.h"
#include "ObjectResource.h"
#include "PlugInRegistration.h"

#include <sstream>
#include <string>

using namespace std;
using namespace Nitf;
using namespace Nitf::TRE;

REGISTER_PLUGIN(OpticksNitfCommonTre, Use26aParser, Nitf::Use26aParser());

Nitf::Use26aParser::Use26aParser()
{
   setName("USE26A");
   setDescriptorId("{9A2481CE-B11E-46db-BF50-503EEE6F7E86}");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

bool Nitf::Use26aParser::runAllTests(Progress* pProgress, ostream& failure)
{
   static const string data(
      "270"                     // FIELD1
      "000.0"                   // FIELD2
      "1"                       // FIELD3
      "99999"                   // FIELD4
      "000"                     // FIELD5
      "0"                       // FIELD6
      "8.0"                     // FIELD7
      "17.04"                   // FIELD8
      "-90.00"                  // FIELD9
      "FIELD10     "            // FIELD10
      "FIELD11        "         // FIELD11
      "45.0"                    // FIELD12
      "2"                       // FIELD13
      "250"                     // FIELD14
      "  "                      // FIELD15
      "02"                      // FIELD16
      "00001"                   // FIELD17
      "001"                     // FIELD18
      "000001"                  // FIELD19
      "999999"                  // FIELD20
      "999999"                  // FIELD21
      );

   static const string data_error4(
      "270"                     // FIELD1
      "000.0"                   // FIELD2
      " "                       // FIELD3
      "     "                   // FIELD4   Error: needs to be a number
      "   "                     // FIELD5
      " "                       // FIELD6
      "   "                     // FIELD7
      "17.04"                   // FIELD8
      "-90.01"                  // FIELD9        min = -90.00
      "            "            // FIELD10
      "               "         // FIELD11
      "    "                    // FIELD12
      " "                       // FIELD13
      "   "                     // FIELD14
      "  "                      // FIELD15
      "  "                      // NR
      "00000"                   // RN
      "001"                     // FIELD18
      "000001"                  // FIELD19
      "      "                  // FIELD20
      "      "                  // FIELD21
      );

   FactoryResource<DynamicObject> treDO;
   size_t numBytes(0);

   istringstream input(data);
   numBytes = data.size();

   string errorMessage;
   bool success = toDynamicObject(input, numBytes, *treDO.get(), errorMessage);

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
      failure << "Error: Negative test with LNSTRT = data out of range failed: did not return false\n";
      treDO->clear();
      return false;
   }

   treDO->clear();

   return true;
}

bool Nitf::Use26aParser::toDynamicObject(istream& input, size_t numBytes, DynamicObject& output,
   string &errorMessage) const
{
   vector<char> buf;
   bool ok(true);
   bool success(true);

   readField<unsigned int>(input, output, success, USE26A::FIELD1, 3, errorMessage, buf);
   readField<double>(input, output, success, USE26A::FIELD2, 5, errorMessage, buf);
   readField<unsigned int>(input, output, success, USE26A::FIELD3, 1, errorMessage, buf, true);
   readField<unsigned int>(input, output, success, USE26A::FIELD4, 5, errorMessage, buf, true);
   readField<unsigned int>(input, output, success, USE26A::FIELD5, 3, errorMessage, buf, true);
   readField<unsigned int>(input, output, success, USE26A::FIELD6, 1, errorMessage, buf, true);
   readField<double>(input, output, success, USE26A::FIELD7, 3, errorMessage, buf, true);
   readField<double>(input, output, success, USE26A::FIELD8, 5, errorMessage, buf, true);
   readField<double>(input, output, success, USE26A::FIELD9, 6, errorMessage, buf, true);
   readField<string>(input, output, success, USE26A::FIELD10, 12, errorMessage, buf, true);
   readField<string>(input, output, success, USE26A::FIELD11, 15, errorMessage, buf, true);
   readField<double>(input, output, success, USE26A::FIELD12, 4, errorMessage, buf, true);
   readField<unsigned int>(input, output, success, USE26A::FIELD13, 1, errorMessage, buf, true);
   readField<unsigned int>(input, output, success, USE26A::FIELD14, 3, errorMessage, buf, true);

   // one field of 2 or 2 fields of 1???
   readField<string>(input, output, success, USE26A::FIELD15, 2, errorMessage, buf, true);

   readField<unsigned int>(input, output, success, USE26A::FIELD16, 2, errorMessage, buf);
   readField<unsigned int>(input, output, success, USE26A::FIELD17, 5, errorMessage, buf);
   readField<unsigned int>(input, output, success, USE26A::FIELD18, 3, errorMessage, buf);
   readField<unsigned int>(input, output, success, USE26A::FIELD19, 6, errorMessage, buf, true);
   readField<unsigned int>(input, output, success, USE26A::FIELD20, 6, errorMessage, buf, true);
   readField<unsigned int>(input, output, success, USE26A::FIELD21, 6, errorMessage, buf, true);

   size_t numRead = input.tellg();
   if (numRead != numBytes)
   {
      numReadErrMsg(numRead, numBytes, errorMessage);
      return false;
   }

   return success;
}


Nitf::TreState Nitf::Use26aParser::isTreValid(const DynamicObject& tre, ostream& reporter) const
{
   TreState status(VALID);
   set<string> testSet;
   unsigned int numFields = 0;

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, USE26A::FIELD1, 0U, 359U));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, USE26A::FIELD2, 0.0F, 999.9F));

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, USE26A::FIELD3, 0U, 1U));

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, USE26A::FIELD4, 0U, 99999U));

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, USE26A::FIELD5, 0U, 255U));

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, USE26A::FIELD6, 0U, 1U));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, USE26A::FIELD7, 0.0F, 9.0F));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, USE26A::FIELD8, 0.0F, 90.0F));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, USE26A::FIELD9, -90.0F, 90.0F));

   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, USE26A::FIELD10, testSet, true, true, false));

   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, USE26A::FIELD11, testSet, true, true, false));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, USE26A::FIELD12, 0.0F, 90.0F));

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, USE26A::FIELD13, 0U, 2U));

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, USE26A::FIELD14, 0U, 250U));

   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, USE26A::FIELD15, testSet, true, true, false)); // one field of 2 or 2 fields of 1???

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, USE26A::FIELD16, 2U, 99U));

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, USE26A::FIELD17, 1U, 99999U));

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, USE26A::FIELD18, 1U, 999U));

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, USE26A::FIELD19, 1U, 999999U));

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, USE26A::FIELD20, 1U, 999999U));

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, USE26A::FIELD21, 1U, 999999U));

   unsigned int totalFields = tre.getNumAttributes();
   if (status != INVALID && totalFields != numFields)
   {
      reporter << "Total fields in the Dynamic Object(" <<
         totalFields << ") did not match the number found(" << numFields << ") ";
      status = INVALID;
   }

   if (status == INVALID)
   {
      reporter << " INVALID fields found in the USE26A TAG/SDE\n" ;
   }
   else if (status == SUSPECT)
   {
      reporter << " SUSPECT fields found in the USE26A TAG/SDE\n" ;
   }

   return status;
}


bool Nitf::Use26aParser::fromDynamicObject(const DynamicObject& input, ostream& output, size_t& numBytesWritten,
   string &errorMessage) const
{
   size_t sizeIn = max(static_cast<ostream::pos_type>(0), output.tellp());
   size_t sizeOut(sizeIn);

   try
   {
      output << toString(dv_cast<unsigned int>(input.getAttribute(USE26A::FIELD1)), 3, -1);
      output << toString(dv_cast<double>(input.getAttribute(USE26A::FIELD2)), 5, 1);
      output << toString(dv_cast<unsigned int>(input.getAttribute(USE26A::FIELD3)), 1, -1);
      output << toString(dv_cast<unsigned int>(input.getAttribute(USE26A::FIELD4)), 5, -1);
      output << toString(dv_cast<unsigned int>(input.getAttribute(USE26A::FIELD5)), 3, -1);
      output << toString(dv_cast<unsigned int>(input.getAttribute(USE26A::FIELD6)), 1, -1);
      output << toString(dv_cast<double>(input.getAttribute(USE26A::FIELD7)), 3, 1);
      output << toString(dv_cast<double>(input.getAttribute(USE26A::FIELD8)), 5, 2);
      output << toString(dv_cast<double>(input.getAttribute(USE26A::FIELD9)), 6, 2, ZERO_FILL, POS_SIGN_TRUE);
      output << sizeString(dv_cast<string>(input.getAttribute(USE26A::FIELD10)), 12);
      output << sizeString(dv_cast<string>(input.getAttribute(USE26A::FIELD11)), 15);
      output << toString(dv_cast<double>(input.getAttribute(USE26A::FIELD12)), 4, 1);
      output << toString(dv_cast<unsigned int>(input.getAttribute(USE26A::FIELD13)), 1, -1);
      output << toString(dv_cast<unsigned int>(input.getAttribute(USE26A::FIELD14)), 3, -1);

      // one field of 2 or 2 fields of 1?
      output << sizeString(dv_cast<string>(input.getAttribute(USE26A::FIELD15)), 2);

      output << toString(dv_cast<unsigned int>(input.getAttribute(USE26A::FIELD16)), 2, -1);
      output << toString(dv_cast<unsigned int>(input.getAttribute(USE26A::FIELD17)), 5, -1);
      output << toString(dv_cast<unsigned int>(input.getAttribute(USE26A::FIELD18)), 3, -1);
      output << toString(dv_cast<unsigned int>(input.getAttribute(USE26A::FIELD19)), 6, -1);
      output << toString(dv_cast<unsigned int>(input.getAttribute(USE26A::FIELD20)), 6, -1);
      output << toString(dv_cast<unsigned int>(input.getAttribute(USE26A::FIELD21)), 6, -1);
   }
   catch (const bad_cast&)
   {
      return false;
   }

   sizeOut = output.tellp();
   numBytesWritten = sizeOut - sizeIn;
   return true;
}
