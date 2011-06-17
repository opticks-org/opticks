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
#include "NitfExoptaParser.h"
#include "NitfUtilities.h"
#include "ObjectResource.h"
#include "PlugInRegistration.h"

#include <sstream>
#include <string>

using namespace std;
using namespace Nitf;
using namespace Nitf::TRE;

REGISTER_PLUGIN(OpticksNitfCommonTre, ExoptaParser, Nitf::ExoptaParser());

Nitf::ExoptaParser::ExoptaParser()
{
   setName("EXOPTA");
   setDescriptorId("{4D5A110E-85BD-439d-884C-9A3D336E1E20}");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

bool Nitf::ExoptaParser::runAllTests(Progress* pProgress, ostream& failure)
{
   static const string data(
      "359"                     // ANGLE_TO_NORTH
      "999.9"                   // MEAN_GSD
      "1"                       // RESERVED
      "65535"                   // DYNAMIC_RANGE
      "       "                 // RESERVED2
      "090.0"                   // OBL_ANG
      "-90.0 "                  // ROLL_ANG
      "Prime_id    "            // PRIME_ID
      "Prime_be       "         // PRIME_BE
      "     "                   // RESERVED3
      "250"                     // N_SEC
      "  "                      // RESERVED4
      "0000001"                 // RESERVED5
      "999"                     // N_SEG
      "199999"                  // MAX_LP_SEG
      "            "            // RESERVED6
      "-90.0"                   // SUN_EL
      "999.9"                   // SUN_AZ
      );

   static const string data_error4(
      "359"                     // ANGLE_TO_NORTH
      "999.9"                   // MEAN_GSD
      "1"                       // RESERVED
      "65535"                   // DYNAMIC_RANGE
      "       "                 // RESERVED2
      "090.0"                   // OBL_ANG
      "-090.0"                  // ROLL_ANG
      "Prime_id    "            // PRIME_ID
      "Prime_be       "         // PRIME_BE
      "     "                   // RESERVED3
      "250"                     // N_SEC
      "  "                      // RESERVED4
      "0000001"                 // RESERVED5
      "999"                     // N_SEG
      "199999"                  // MAX_LP_SEG
      "            "            // RESERVED6
      "-90.1"                   // SUN_EL          min == -90.0
      "999.9"                   // SUN_AZ
      );

   static const string data5(
      "359"                     // ANGLE_TO_NORTH
      "999.9"                   // MEAN_GSD
      "1"                       // RESERVED
      "65535"                   // DYNAMIC_RANGE
      "       "                 // RESERVED2
      "90.00"                   // OBL_ANG
      "-90.00"                  // ROLL_ANG
      "Prime_id    "            // PRIME_ID
      "Prime_be       "         // PRIME_BE
      "     "                   // RESERVED3
      "250"                     // N_SEC
      "  "                      // RESERVED4
      "0000001"                 // RESERVED5
      "999"                     // N_SEG
      "199999"                  // MAX_LP_SEG
      "            "            // RESERVED6
      "-90.0"                   // SUN_EL
      "999.9"                   // SUN_AZ
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

bool Nitf::ExoptaParser::toDynamicObject(istream& input, size_t numBytes, DynamicObject& output,
   string &errorMessage) const
{
   vector<char> buf;
   bool ok(true);
   bool success(true);

   readField<unsigned int>(input, output, success, EXOPTA::ANGLE_TO_NORTH, 3, errorMessage, buf, true);
   readField<double>(input, output, success, EXOPTA::MEAN_GSD, 5, errorMessage, buf, true);
   readField<string>(input, output, success, EXOPTA::RESERVED, 1, errorMessage, buf);
   readField<unsigned int>(input, output, success, EXOPTA::DYNAMIC_RANGE, 5, errorMessage, buf, true);
   readField<string>(input, output, success, EXOPTA::RESERVED2, 7, errorMessage, buf, true);
   readField<double>(input, output, success, EXOPTA::OBL_ANG, 5, errorMessage, buf, true);
   readField<double>(input, output, success, EXOPTA::ROLL_ANG, 6, errorMessage, buf, true);
   readField<string>(input, output, success, EXOPTA::PRIME_ID, 12, errorMessage, buf, true);
   readField<string>(input, output, success, EXOPTA::PRIME_BE, 15, errorMessage, buf, true);
   readField<string>(input, output, success, EXOPTA::RESERVED3, 5, errorMessage, buf, true);
   readField<unsigned int>(input, output, success, EXOPTA::N_SEC, 3, errorMessage, buf);
   readField<string>(input, output, success, EXOPTA::RESERVED4, 2, errorMessage, buf, true);
   readField<string>(input, output, success, EXOPTA::RESERVED5, 7, errorMessage, buf);
   readField<unsigned int>(input, output, success, EXOPTA::N_SEG, 3, errorMessage, buf);
   readField<unsigned int>(input, output, success, EXOPTA::MAX_LP_SEG, 6, errorMessage, buf, true);
   readField<string>(input, output, success, EXOPTA::RESERVED6, 12, errorMessage, buf, true);
   readField<double>(input, output, success, EXOPTA::SUN_EL, 5, errorMessage, buf);
   readField<double>(input, output, success, EXOPTA::SUN_AZ, 5, errorMessage, buf);

   size_t numRead = input.tellg();
   if (numRead != numBytes)
   {
      numReadErrMsg(numRead, numBytes, errorMessage);
      return false;
   }

   return success;
}


Nitf::TreState Nitf::ExoptaParser::isTreValid(const DynamicObject& tre, ostream& reporter) const
{
   TreState status(VALID);
   set<string>             testSet;
   unsigned int numFields = 0;

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, EXOPTA::ANGLE_TO_NORTH, 0U, 359U, true));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, EXOPTA::MEAN_GSD, 0.0F, 999.9F, true));

   testSet.clear();
   testSet.insert("1");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, EXOPTA::RESERVED, testSet, true, true, false));

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, EXOPTA::DYNAMIC_RANGE, 0U, 65535U, true));

   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, EXOPTA::RESERVED2, testSet, true, true, false));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, EXOPTA::OBL_ANG, 0.0F, 90.0F, true));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, EXOPTA::ROLL_ANG, -90.1F, 90.1F, true));

   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, EXOPTA::PRIME_ID, testSet, true, true, false));

   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, EXOPTA::PRIME_BE, testSet, true, true, false));

   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, EXOPTA::RESERVED3, testSet, true, true, false));

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, EXOPTA::N_SEC, 0U, 250U));

   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, EXOPTA::RESERVED4, testSet, true, true, false));

   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, EXOPTA::RESERVED5, testSet, true, true, false));

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, EXOPTA::N_SEG, 1U, 999U));

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, EXOPTA::MAX_LP_SEG, 1U, 199999U, true));

   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, EXOPTA::RESERVED6, testSet, true, true, false));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, EXOPTA::SUN_EL, -90.0F, 999.9F));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, EXOPTA::SUN_AZ, 0.0F, 999.9F));

   unsigned int totalFields = tre.getNumAttributes();
   if (status != INVALID && totalFields != numFields)
   {
      reporter << "Total fields in the Dynamic Object(" <<
         totalFields << ") did not match the number found(" << numFields << ") ";
      status = INVALID;
   }

   if (status == INVALID)
   {
      reporter << " INVALID fields found in the EXOPTA TAG/SDE\n" ;
   }
   else if (status == SUSPECT)
   {
      reporter << " SUSPECT fields found in the EXOPTA TAG/SDE\n" ;
   }

   return status;
}


bool Nitf::ExoptaParser::fromDynamicObject(const DynamicObject& input, ostream& output, size_t& numBytesWritten,
   string &errorMessage) const
{
   size_t sizeIn = max(static_cast<ostream::pos_type>(0), output.tellp());
   size_t sizeOut(sizeIn);

   try
   {
      output << toString(dv_cast<unsigned int>(input.getAttribute(EXOPTA::ANGLE_TO_NORTH)), 3, -1,
         ZERO_FILL, false, false, 3, true);
      output << toString(dv_cast<double>(input.getAttribute(EXOPTA::MEAN_GSD)), 5, 1,
         ZERO_FILL, false, false, 3, true);

      // RESERVED has a fixed value of "1" so hard code it for now
      // output << sizeString( dv_cast<string>(input.getAttribute (EXOPTA::RESERVED)), 1);
      output << "1";

      output << toString(dv_cast<unsigned int>(input.getAttribute(EXOPTA::DYNAMIC_RANGE)), 5, -1,
         ZERO_FILL, false, false, 3, true);
      output << sizeString(dv_cast<string>(input.getAttribute(EXOPTA::RESERVED2)), 7);
      output << toString(dv_cast<double>(input.getAttribute(EXOPTA::OBL_ANG)), 5, 2,
         ZERO_FILL, false, false, 3, true);
      output << toString(dv_cast<double>(input.getAttribute(EXOPTA::ROLL_ANG)), 6, 2,
         ZERO_FILL, POS_SIGN_TRUE, false, 3, true);
      output << sizeString(dv_cast<string>(input.getAttribute(EXOPTA::PRIME_ID)), 12);
      output << sizeString(dv_cast<string>(input.getAttribute(EXOPTA::PRIME_BE)), 15);
      output << sizeString(dv_cast<string>(input.getAttribute(EXOPTA::RESERVED3)), 5);
      output << toString(dv_cast<unsigned int>(input.getAttribute(EXOPTA::N_SEC)), 3, -1);
      output << sizeString(dv_cast<string>(input.getAttribute(EXOPTA::RESERVED4)), 2);

      // RESERVED5 has a fixed value of "0000001" so hard code it for now
      // output << sizeString( dv_cast<string>(input.getAttribute (EXOPTA::RESERVED5)), 7);
      output << "0000001";    // RESERVED5 has a fixed value of "0000001"

      output << toString(dv_cast<unsigned int>(input.getAttribute(EXOPTA::N_SEG)), 3, -1);
      output << toString(dv_cast<unsigned int>(input.getAttribute(EXOPTA::MAX_LP_SEG)), 6, -1,
         ZERO_FILL, false, false, 3, true);
      output << sizeString(dv_cast<string>(input.getAttribute(EXOPTA::RESERVED6)), 12);
      output << toString(dv_cast<double>(input.getAttribute(EXOPTA::SUN_EL)), 5, 1, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<double>(input.getAttribute(EXOPTA::SUN_AZ)), 5, 1);
   }
   catch (const bad_cast&)
   {
      return false;
   }

   sizeOut = output.tellp();
   numBytesWritten = sizeOut - sizeIn;
   return true;
}
