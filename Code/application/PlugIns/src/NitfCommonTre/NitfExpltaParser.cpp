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
#include "NitfExpltaParser.h"
#include "NitfUtilities.h"
#include "ObjectResource.h"
#include "PlugInRegistration.h"

#include <ostream>
#include <sstream>

using namespace std;
using namespace Nitf;
using namespace Nitf::TRE;

REGISTER_PLUGIN(OpticksNitfCommonTre, ExpltaParser, Nitf::ExpltaParser());

Nitf::ExpltaParser::ExpltaParser()
{
   setName("EXPLTA");
   setDescriptorId("{820D221F-18CF-4fb5-A98C-F7F4ECB32205}");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

bool Nitf::ExpltaParser::runAllTests(Progress* pProgress, ostream& failure)
{
   static const string data(
      "090"                       // ANGLE_TO_NORTH
      "-10"                       // SQUINT_ANGLE
      "1GP"                       // MODE
      "                "          // RESERVED1
      "45"                        // GRAZE_ANG
      "30"                        // SLOPE_ANG
      "HV"                        // POLAR
      "00020"                     // NSAMP
      "0"                         // RESERVED2
      "1"                         // SEQ_NUM
      "TARGET ID   "              // PRIME_ID
      "ENCY ID        "           // PRIME_BE
      "0"                         // RESERVED3
      "01"                        // N_SEC
      "00"                        // IPR
      "01"                        // RESERVED4
      "  "                        // RESERVED5
      "00000"                     // RESERVED6
      "        "                  // RESERVED7
      );


   static const string data_error4(
      "090"                       // ANGLE_TO_NORTH
      "-10"                       // SQUINT_ANGLE
      "1GP"                       // MODE
      "                "          // RESERVED1
      "45"                        // GRAZE_ANG
      "30"                        // SLOPE_ANG
      "JK"                        // POLAR         // ERROR SUSPECT value
      "00020"                     // NSAMP
      "0"                         // RESERVED2
      "1"                         // SEQ_NUM
      "TARGET ID   "              // PRIME_ID
      "ENCY ID        "           // PRIME_BE
      "0"                         // RESERVED3
      "01"                        // N_SEC
      "00"                        // IPR
      "01"                        // RESERVED4
      "  "                        // RESERVED5
      "00000"                     // RESERVED6
      "        "                  // RESERVED7
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
         failure << "Error: Negative test with POLAR data out of range failed: did not return SUSPECT\n";
         failure << tmpStream.str();
         treDO->clear();
         return false;
      }
      status = VALID;
   }

   treDO->clear();

   return (status != INVALID);
}

Nitf::TreState Nitf::ExpltaParser::isTreValid(const DynamicObject& tre, ostream& reporter) const
{
   TreState status = VALID;
   set<string>             testSet;
   unsigned int numFields = 0;

   status = MaxState(status, testTagValueRange<int>(tre, reporter, &numFields, EXPLTA::ANGLE_TO_NORTH, 0U, 359U));
   status = MaxState(status, testTagValueRange<int>(tre, reporter, &numFields, EXPLTA::SQUINT_ANGLE, -60, 85));

   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, EXPLTA::MODE, testSet, false, true, false));

   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, EXPLTA::RESERVED1, testSet, true, false, true));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, EXPLTA::GRAZE_ANG, 0, 90));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, EXPLTA::SLOPE_ANG, 0, 90));

   testSet.clear();
   testSet.insert("HH");
   testSet.insert("HV");
   testSet.insert("VH");
   testSet.insert("VV");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, EXPLTA::POLAR, testSet, false, false, true));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, EXPLTA::NSAMP, 1U, 99999U));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, EXPLTA::RESERVED2, 0U, 0U));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, EXPLTA::SEQ_NUM, 0U, 6U));

   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, EXPLTA::PRIME_ID, testSet, true, true, false));

   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, EXPLTA::PRIME_BE, testSet, true, true, false));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, EXPLTA::RESERVED3, 0U, 0U));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, EXPLTA::N_SEC, 0U, 10U));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, EXPLTA::IPR, 0U, 99U));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, EXPLTA::RESERVED4, 1U, 1U));

   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, EXPLTA::RESERVED5, testSet, true, false, true));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, EXPLTA::RESERVED6, 0U, 0U));

   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, EXPLTA::RESERVED7, testSet, true, false, true));

   unsigned int totalFields = tre.getNumAttributes();
   if (status != INVALID && totalFields != numFields)
   {
      reporter << "Total fields in the Dynamic Object(" <<
         totalFields << ") did not match the number found(" << numFields << ") ";
      status = INVALID;
   }

   if (status == INVALID)
   {
      reporter << " INVALID fields found in the EXPLTA TAG/SDE\n" ;
   }
   else if (status == SUSPECT)
   {
      reporter << " SUSPECT fields found in the EXPLTA TAG/SDE\n" ;
   }

   return status;
}


bool Nitf::ExpltaParser::toDynamicObject(istream& input, size_t numBytes, DynamicObject& output,
   string &errorMessage) const
{
   vector<char> buf;
   bool ok(true);

   bool success = true;

   readField<int>(input, output, success, EXPLTA::ANGLE_TO_NORTH, 3, errorMessage, buf);
   readField<int>(input, output, success, EXPLTA::SQUINT_ANGLE, 3, errorMessage, buf);
   readField<string>(input, output, success, EXPLTA::MODE, 3, errorMessage, buf);
   readField<string>(input, output, success, EXPLTA::RESERVED1, 16, errorMessage, buf, true);
   readField<int>(input, output, success, EXPLTA::GRAZE_ANG, 2, errorMessage, buf);
   readField<int>(input, output, success, EXPLTA::SLOPE_ANG, 2, errorMessage, buf);
   readField<string>(input, output, success, EXPLTA::POLAR, 2, errorMessage, buf);
   readField<int>(input, output, success, EXPLTA::NSAMP, 5, errorMessage, buf);
   readField<int>(input, output, success, EXPLTA::RESERVED2, 1, errorMessage, buf);
   readField<int>(input, output, success, EXPLTA::SEQ_NUM, 1, errorMessage, buf, true);
   readField<string>(input, output, success, EXPLTA::PRIME_ID, 12, errorMessage, buf, true);
   readField<string>(input, output, success, EXPLTA::PRIME_BE, 15, errorMessage, buf, true);
   readField<int>(input, output, success, EXPLTA::RESERVED3, 1, errorMessage, buf);
   readField<int>(input, output, success, EXPLTA::N_SEC, 2, errorMessage, buf, true);
   readField<int>(input, output, success, EXPLTA::IPR, 2, errorMessage, buf, true);
   readField<int>(input, output, success, EXPLTA::RESERVED4, 2, errorMessage, buf);
   readField<string>(input, output, success, EXPLTA::RESERVED5, 2, errorMessage, buf, true);
   readField<int>(input, output, success, EXPLTA::RESERVED6, 5, errorMessage, buf);
   readField<string>(input, output, success, EXPLTA::RESERVED7, 8, errorMessage, buf, true);

   size_t numRead = input.tellg();
   if (numRead != numBytes)
   {
      numReadErrMsg(numRead, numBytes, errorMessage);
      return false;
   }

   return success;
}


bool Nitf::ExpltaParser::fromDynamicObject(const DynamicObject& input, ostream& output, size_t& numBytesWritten,
   string &errorMessage) const
{
   size_t sizeIn = max(static_cast<ostream::pos_type>(0), output.tellp());
   size_t sizeOut(sizeIn);

   try
   {
      output << toString(dv_cast<int>(input.getAttribute(EXPLTA::ANGLE_TO_NORTH)), 3);
      output << toString(dv_cast<int>(input.getAttribute(EXPLTA::SQUINT_ANGLE)), 3, -1, ZERO_FILL, POS_SIGN_TRUE);
      output << sizeString(dv_cast<string>(input.getAttribute(EXPLTA::MODE)), 3);
      output << sizeString(dv_cast<string>(input.getAttribute(EXPLTA::RESERVED1)), 16);
      output << toString(dv_cast<int>(input.getAttribute(EXPLTA::GRAZE_ANG)), 2);
      output << toString(dv_cast<int>(input.getAttribute(EXPLTA::SLOPE_ANG)), 2);
      output << sizeString(dv_cast<string>(input.getAttribute(EXPLTA::POLAR)), 2);
      output << toString(dv_cast<int>(input.getAttribute(EXPLTA::NSAMP)), 5);
      output << toString(dv_cast<int>(input.getAttribute(EXPLTA::RESERVED2)), 1);
      output << toString(dv_cast<int>(input.getAttribute(EXPLTA::SEQ_NUM)), 1);
      output << sizeString(dv_cast<string>(input.getAttribute(EXPLTA::PRIME_ID)), 12);
      output << sizeString(dv_cast<string>(input.getAttribute(EXPLTA::PRIME_BE)), 15);
      output << toString(dv_cast<int>(input.getAttribute(EXPLTA::RESERVED3)), 1);
      output << toString(dv_cast<int>(input.getAttribute(EXPLTA::N_SEC)), 2);
      output << toString(dv_cast<int>(input.getAttribute(EXPLTA::IPR)), 2);
      output << toString(dv_cast<int>(input.getAttribute(EXPLTA::RESERVED4)), 2);
      output << sizeString(dv_cast<string>(input.getAttribute(EXPLTA::RESERVED5)), 2);
      output << toString(dv_cast<int>(input.getAttribute(EXPLTA::RESERVED6)), 5);
      output << sizeString(dv_cast<string>(input.getAttribute(EXPLTA::RESERVED7)), 8);
   }
   catch (const bad_cast&)
   {
      return false;
   }

   sizeOut = output.tellp();
   numBytesWritten = sizeOut - sizeIn;
   return true;
}
