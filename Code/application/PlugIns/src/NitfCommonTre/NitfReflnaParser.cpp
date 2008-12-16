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
#include "NitfReflnaParser.h"
#include "NitfUtilities.h"
#include "ObjectResource.h"

#include <sstream>
#include <string>

using namespace std;
using namespace Nitf;
using namespace Nitf::TRE;

Nitf::ReflnaParser::ReflnaParser()
{
   setName("REFLNA");
   setDescriptorId("{9180BE35-E152-4550-BFDF-B35DE4DD815D}");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

bool Nitf::ReflnaParser::runAllTests(Progress* pProgress, ostream& failure)
{
   static const string data(
      "12"                    // FIELD1
      "1234567"               // FIELD2
      "12345678"              // FIELD3
      "1234567"               // FIELD4
      "12345678"              // FIELD5
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

   return (status != INVALID);
}

bool Nitf::ReflnaParser::toDynamicObject(istream& input, size_t numBytes, DynamicObject& output,
   string &errorMessage) const
{
   vector<char> buf;
   bool ok(true);
   bool success(true);

#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : Allow all blanks for classified fields " \
   "since we do not have descriptions of them (dadkins)")
   readField<string>(input, output, success, REFLNA::FIELD1, 2, errorMessage, buf, true);
   readField<string>(input, output, success, REFLNA::FIELD2, 7, errorMessage, buf, true);
   readField<string>(input, output, success, REFLNA::FIELD3, 8, errorMessage, buf, true);
   readField<string>(input, output, success, REFLNA::FIELD4, 7, errorMessage, buf, true);
   readField<string>(input, output, success, REFLNA::FIELD5, 8, errorMessage, buf, true);

   size_t numRead = input.tellg();
   if (numRead != numBytes)
   {
      numReadErrMsg(numRead, numBytes, errorMessage);
      return false;
   }

   return success;
}


Nitf::TreState Nitf::ReflnaParser::isTreValid(const DynamicObject& tre, ostream& reporter) const
{
   TreState status(VALID);
   set<string> testSet;
   unsigned int numFields = 0;

   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, REFLNA::FIELD1, testSet, true, true, false));

   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, REFLNA::FIELD2, testSet, true, true, false));

   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, REFLNA::FIELD3, testSet, true, true, false));

   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, REFLNA::FIELD4, testSet, true, true, false));

   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, REFLNA::FIELD5, testSet, true, true, false));

   unsigned int totalFields = tre.getNumAttributes();
   if (status != INVALID && totalFields != numFields)
   {
      reporter << "Total fields in the Dynamic Object(" <<
         totalFields << ") did not match the number found(" << numFields << ") ";
      status = INVALID;
   }

   if (status == INVALID)
   {
      reporter << " INVALID fields found in the REFLNA TAG/SDE\n" ;
   }
   else if (status == SUSPECT)
   {
      reporter << " SUSPECT fields found in the REFLNA TAG/SDE\n" ;
   }

   return status;
}


bool Nitf::ReflnaParser::fromDynamicObject(const DynamicObject& input, ostream& output, size_t& numBytesWritten,
   string &errorMessage) const
{
   size_t sizeIn = max(static_cast<ostream::pos_type>(0), output.tellp());
   size_t sizeOut(sizeIn);

   try
   {
      output << sizeString(dv_cast<string>(input.getAttribute(REFLNA::FIELD1)), 2);
      output << sizeString(dv_cast<string>(input.getAttribute(REFLNA::FIELD2)), 7);
      output << sizeString(dv_cast<string>(input.getAttribute(REFLNA::FIELD3)), 8);
      output << sizeString(dv_cast<string>(input.getAttribute(REFLNA::FIELD4)), 7);
      output << sizeString(dv_cast<string>(input.getAttribute(REFLNA::FIELD5)), 8);
   }
   catch (const bad_cast&)
   {
      return false;
   }

   sizeOut = output.tellp();
   numBytesWritten = sizeOut - sizeIn;
   return true;
}
