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
#include "NitfSectgaParser.h"
#include "NitfUtilities.h"
#include "ObjectResource.h"
#include "PlugInRegistration.h"

#include <sstream>
#include <string>

using namespace std;
using namespace Nitf;
using namespace Nitf::TRE;

REGISTER_PLUGIN(OpticksNitfCommonTre, SectgaParser, Nitf::SectgaParser());

Nitf::SectgaParser::SectgaParser()
{
   setName("SECTGA");
   setDescriptorId("{8D79488D-061F-4220-86DF-82B91A5297DA}");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

bool Nitf::SectgaParser::runAllTests(Progress* pProgress, ostream& failure)
{
   static const string data(
      "123456789012"                 // SEC_ID
      "123456789012345"              // SEC_BE
      "0"                            // RESERVED001
      );

   static const string data_error4(
      "123456789012"                 // SEC_ID
      "123456789012345"              // SEC_BE
      " "                            // RESERVED001     ERROR: must == "0"
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
      failure << "Error: Negative test with data out of range failed: did not return false\n";
      treDO->clear();
      return false;
   }

   treDO->clear();

   return true;
}

bool Nitf::SectgaParser::toDynamicObject(istream& input, size_t numBytes, DynamicObject& output,
   string &errorMessage) const
{
   vector<char> buf;
   bool success(true);

   readField<string>(input, output, success, SECTGA::SEC_ID, 12, errorMessage, buf, true);
   readField<string>(input, output, success, SECTGA::SEC_BE, 15, errorMessage, buf, true);
   readField<unsigned int>(input, output, success, SECTGA::RESERVED001, 1, errorMessage, buf);

   int64_t numRead = input.tellg();
   if (numRead < 0 || numRead > static_cast<int64_t>(std::numeric_limits<size_t>::max()) ||
      numRead != static_cast<int64_t>(numBytes))
   {
      numReadErrMsg(numRead, numBytes, errorMessage);
      return false;
   }

   return success;
}


Nitf::TreState Nitf::SectgaParser::isTreValid(const DynamicObject& tre, ostream& reporter) const
{
   TreState status(VALID);
   set<string>             testSet;
   unsigned int numFields = 0;

   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, SECTGA::SEC_ID, testSet, true, true, false));

   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, SECTGA::SEC_BE, testSet, true, true, false));

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, SECTGA::RESERVED001, 0U, 0U));

   unsigned int totalFields = tre.getNumAttributes();
   if (status != INVALID && totalFields != numFields)
   {
      reporter << "Total fields in the Dynamic Object(" <<
         totalFields << ") did not match the number found(" << numFields << ") ";
      status = INVALID;
   }

   if (status == INVALID)
   {
      reporter << " INVALID fields found in the SECTGA TAG/SDE\n" ;
   }
   else if (status == SUSPECT)
   {
      reporter << " SUSPECT fields found in the SECTGA TAG/SDE\n" ;
   }

   return status;
}


bool Nitf::SectgaParser::fromDynamicObject(const DynamicObject& input, ostream& output, size_t& numBytesWritten,
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
      output << sizeString(dv_cast<string>(input.getAttribute(SECTGA::SEC_ID)), 12);
      output << sizeString(dv_cast<string>(input.getAttribute(SECTGA::SEC_BE)), 15);
      output << toString(dv_cast<unsigned int>(input.getAttribute(SECTGA::RESERVED001)), 1, -1);
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
