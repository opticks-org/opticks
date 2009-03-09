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
#include "NitfMod26aParser.h"
#include "NitfUtilities.h"
#include "ObjectResource.h"

#include <sstream>
#include <string>

using namespace std;
using namespace Nitf;
using namespace Nitf::TRE;


Nitf::Mod26aParser::Mod26aParser()
{
   setName("MOD26A");
   setDescriptorId("{56BDF79B-785A-4755-938D-24FEDF10003E}");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

bool Nitf::Mod26aParser::runAllTests(Progress* pProgress, ostream& failure)
{
   static const string data(
      "M"                       // FIELD1
      "99"                      // FIELD2
      "999999.999"              // FIELD3
      "9.999"                   // FIELD4
      "9.99"                    // FIELD5
      "9999.999"                // FIELD6
      "9999.999"                // FIELD7
      "9999.999"                // FIELD8
      "9"                       // FIELD9
      "T"                       // FIELD10
      );

   static const string data_error4(
      "M"                       // FIELD1
      "-1"                      // FIELD2         // min == 0
      "999999.999"              // FIELD3
      "9.999"                   // FIELD4
      "9.99"                    // FIELD5
      "9999.999"                // FIELD6
      "9999.999"                // FIELD7
      "9999.999"                // FIELD8
      "9"                       // FIELD9
      "T"                       // FIELD10
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
   stringstream input2;
   input2.write(data.c_str(), data.size());
   input2 << "1";          // Add one more byte; valid as alphanumberic or numeric
   numBytes = input2.str().size();
   success = toDynamicObject(input2, numBytes, *treDO.get(), errorMessage);
   if (success == true)     // negative test so success must == false.
   {
      failure << "Error: Negative test of 1 extra byte failed\n";
      treDO->clear();
      return false;
   }

   // start of test 3 - Negative test: 1 byte short in input stream
   stringstream input3;
   input3.write(data.c_str(), data.size()-1);
   numBytes = input3.str().size();
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
   if (success)
   {
      failure << "Error: Negative test with LNSTRT = data out of range failed: did not return false\n";
      treDO->clear();
      return false;
   }

   treDO->clear();

   if (success && !errorMessage.empty())
   {
      failure << errorMessage;
   }

   return (status != INVALID);
}

bool Nitf::Mod26aParser::toDynamicObject(istream &input, size_t numBytes, DynamicObject &output,
   string &errorMessage) const
{
   vector<char> buf;
   bool ok(true);
   bool success(true);

#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : Allow all blanks for classified fields " \
   "since we do not have descriptions of them (dadkins)")
   readField<string>(input, output, success, MOD26A::FIELD1, 1, errorMessage, buf, true);
   readField<unsigned int>(input, output, success, MOD26A::FIELD2, 2, errorMessage, buf, true);
   readField<double>(input, output, success, MOD26A::FIELD3, 10, errorMessage, buf, true);
   readField<double>(input, output, success, MOD26A::FIELD4, 5, errorMessage, buf, true);
   readField<double>(input, output, success, MOD26A::FIELD5, 4, errorMessage, buf, true);
   readField<double>(input, output, success, MOD26A::FIELD6, 8, errorMessage, buf, true);
   readField<double>(input, output, success, MOD26A::FIELD7, 8, errorMessage, buf, true);
   readField<double>(input, output, success, MOD26A::FIELD8, 8, errorMessage, buf, true);
   readField<unsigned int>(input, output, success, MOD26A::FIELD9, 1, errorMessage, buf, true);
   readField<string>(input, output, success, MOD26A::FIELD10, 1, errorMessage, buf, true);

   size_t numRead = input.tellg();
   if (numRead != numBytes)
   {
      numReadErrMsg(numRead, numBytes, errorMessage);
      return false;
   }

   return success;
}


Nitf::TreState Nitf::Mod26aParser::isTreValid(const DynamicObject& tre, ostream& reporter) const
{
   TreState status(VALID);
   set<string>             testSet;
   unsigned int numFields = 0;

   testSet.clear();

   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, MOD26A::FIELD1, testSet, true, true, false));

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, MOD26A::FIELD2, 0U, 99U));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, MOD26A::FIELD3, 0.0F, 999999.9991F));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, MOD26A::FIELD4, 0.0F, 9.9991F));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, MOD26A::FIELD5, 0.0F, 9.991F));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, MOD26A::FIELD6, 0.0F, 9999.9991F));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, MOD26A::FIELD7, 0.0F, 9999.9991F));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, MOD26A::FIELD8, 0.0F, 9999.9991F));

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, MOD26A::FIELD9, 0U, 9U));

   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, MOD26A::FIELD10, testSet, true, true, false));

   unsigned int totalFields = tre.getNumAttributes();
   if (status != INVALID && totalFields != numFields)
   {
      reporter << "Total fields in the Dynamic Object(" << totalFields
         << ") did not match the number found(" << numFields << ") ";
      status = INVALID;
   }

   if (status == INVALID)
   {
      reporter << " INVALID fields found in the MOD26A TAG/SDE\n" ;
   }
   else if (status == SUSPECT)
   {
      reporter << " SUSPECT fields found in the MOD26A TAG/SDE\n" ;
   }

   return status;
}


bool Nitf::Mod26aParser::fromDynamicObject(const DynamicObject& input, ostream& output, size_t& numBytesWritten,
   string &errorMessage) const
{
   size_t sizeIn = max(static_cast<ostream::pos_type>(0), output.tellp());
   size_t sizeOut(sizeIn);

   try
   {
      output << sizeString(dv_cast<string>(input.getAttribute(MOD26A::FIELD1)), 1);
      output << toString(dv_cast<unsigned int>(input.getAttribute(MOD26A::FIELD2)), 2, -1);
      output << toString(dv_cast<double>(input.getAttribute(MOD26A::FIELD3)), 10, 3);
      output << toString(dv_cast<double>(input.getAttribute(MOD26A::FIELD4)), 5, 3);
      output << toString(dv_cast<double>(input.getAttribute(MOD26A::FIELD5)), 4, 2);
      output << toString(dv_cast<double>(input.getAttribute(MOD26A::FIELD6)), 8, 3);
      output << toString(dv_cast<double>(input.getAttribute(MOD26A::FIELD7)), 8, 3);
      output << toString(dv_cast<double>(input.getAttribute(MOD26A::FIELD8)), 8, 3);
      output << toString(dv_cast<unsigned int>(input.getAttribute(MOD26A::FIELD9)), 1, -1);
      output << sizeString(dv_cast<string>(input.getAttribute(MOD26A::FIELD10)), 1);
   }
   catch (const bad_cast&)
   {
      return false;
   }

   sizeOut = output.tellp();
   numBytesWritten = sizeOut - sizeIn;
   return true;
}
