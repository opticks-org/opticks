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
#include "NitfUse00aParser.h"
#include "NitfUtilities.h"
#include "ObjectResource.h"
#include "PlugInRegistration.h"

#include <sstream>
#include <string>

using namespace std;
using namespace Nitf;
using namespace Nitf::TRE;

REGISTER_PLUGIN(OpticksNitfCommonTre, Use00aParser, Nitf::Use00aParser());

Nitf::Use00aParser::Use00aParser()
{
   setName("USE00A");
   setDescriptorId("{92B3A610-4AF8-48d6-BD19-AE09C93E2CD3}");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

bool Nitf::Use00aParser::runAllTests(Progress* pProgress, ostream& failure)
{
   static const string data(
      "270"                     //ANGLE_TO_NORTH
      "000.0"                   // MEAN_GSD
      " "                       // RESERVED1
      "99999"                   // DYNAMIC_RANGE
      "   "                     // RESERVED2
      " "                       // RESERVED3
      "   "                     // RESERVED4
      "17.04"                   // OBL_ANG
      "-90.00"                  // ROLL_ANG
      "            "            // RESERVED5
      "               "         // RESERVED6
      "    "                    // RESERVED7
      " "                       // RESERVED8
      "   "                     // RESERVED9
      " "                       // RESERVED10
      " "                       // RESERVED11
      "00"                      // N_REF
      "00001"                   // REV_NUM
      "001"                     // N_SEG
      "999999"                  // MAX_LP_SEG
      "      "                  // RESERVED12
      "      "                  // RESERVED13
      "+29.5"                   // SUN_EL
      "151.5"                   // SUN_AZ
      );

   static const string data_error4(
      "270"                     // ANGLE_TO_NORTH
      "000.0"                   // MEAN_GSD
      " "                       // RESERVED1
      "     "                   // DYNAMIC_RANGE
      "   "                     // RESERVED2
      " "                       // RESERVED3
      "   "                     // RESERVED4
      "90.04"                   // OBL_ANG - error: needs to be a number from [0, 90]
      "      "                  // ROLL_ANG
      "            "            // RESERVED5
      "               "         // RESERVED6
      "    "                    // RESERVED7
      " "                       // RESERVED8
      "   "                     // RESERVED9
      " "                       // RESERVED10
      " "                       // RESERVED11
      "00"                      // N_REF
      "00001"                   // REV_NUM
      "001"                     // N_SEG
      "      "                  // MAX_LP_SEG
      "      "                  // RESERVED12
      "      "                  // RESERVED13
      "+29.5"                   // SUN_EL
      "151.5"                   // SUN_AZ
      );

   static const string data5(
      "270"                     // ANGLE_TO_NORTH
      "000.0"                   // MEAN_GSD
      " "                       // RESERVED1
      "     "                   // DYNAMIC_RANGE - set to all spaces
      "   "                     // RESERVED2
      " "                       // RESERVED3
      "   "                     // RESERVED4
      "     "                   // OBL_ANG - set to all spaces
      "      "                  // ROLL_ANG - set to all spaces
      "            "            // RESERVED5
      "               "         // RESERVED6
      "    "                    // RESERVED7
      " "                       // RESERVED8
      "   "                     // RESERVED9
      " "                       // RESERVED10
      " "                       // RESERVED11
      "00"                      // N_REF
      "00001"                   // REV_NUM
      "001"                     // N_SEG
      "      "                  // MAX_LP_SEG - set to all spaces
      "      "                  // RESERVED12
      "      "                  // RESERVED13
      "+29.5"                   // SUN_EL
      "151.5"                   // SUN_AZ
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
   if (success == true)
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

bool Nitf::Use00aParser::toDynamicObject(istream& input, size_t numBytes, DynamicObject& output,
   string &errorMessage) const
{
   vector<char> buf;
   bool ok(true);
   bool success(true);

   readField<unsigned int>(input, output, success, USE00A::ANGLE_TO_NORTH, 3, errorMessage, buf);
   readField<double>(input, output, success, USE00A::MEAN_GSD, 5, errorMessage, buf);
   readField<string>(input, output, success, USE00A::RESERVED1, 1, errorMessage, buf, true);
   readField<unsigned int>(input, output, success, USE00A::DYNAMIC_RANGE, 5, errorMessage, buf, true);
   readField<string>(input, output, success, USE00A::RESERVED2, 3, errorMessage, buf, true);
   readField<string>(input, output, success, USE00A::RESERVED3, 1, errorMessage, buf, true);
   readField<string>(input, output, success, USE00A::RESERVED4, 3, errorMessage, buf, true);
   readField<double>(input, output, success, USE00A::OBL_ANG, 5, errorMessage, buf, true);
   readField<double>(input, output, success, USE00A::ROLL_ANG, 6, errorMessage, buf, true);
   readField<string>(input, output, success, USE00A::RESERVED5, 12, errorMessage, buf, true);
   readField<string>(input, output, success, USE00A::RESERVED6, 15, errorMessage, buf, true);
   readField<string>(input, output, success, USE00A::RESERVED7, 4, errorMessage, buf, true);
   readField<string>(input, output, success, USE00A::RESERVED8, 1, errorMessage, buf, true);
   readField<string>(input, output, success, USE00A::RESERVED9, 3, errorMessage, buf, true);
   readField<string>(input, output, success, USE00A::RESERVED10, 1, errorMessage, buf, true);
   readField<string>(input, output, success, USE00A::RESERVED11, 1, errorMessage, buf, true);
   readField<unsigned int>(input, output, success, USE00A::N_REF, 2, errorMessage, buf);
   readField<unsigned int>(input, output, success, USE00A::REV_NUM, 5, errorMessage, buf);
   readField<unsigned int>(input, output, success, USE00A::N_SEG, 3, errorMessage, buf);
   readField<unsigned int>(input, output, success, USE00A::MAX_LP_SEG, 6, errorMessage, buf, true);
   readField<string>(input, output, success, USE00A::RESERVED12, 6, errorMessage, buf, true);
   readField<string>(input, output, success, USE00A::RESERVED13, 6, errorMessage, buf, true);
   readField<double>(input, output, success, USE00A::SUN_EL, 5, errorMessage, buf);
   readField<double>(input, output, success, USE00A::SUN_AZ, 5, errorMessage, buf);

   size_t numRead = input.tellg();
   if (numRead != numBytes)
   {
      numReadErrMsg(numRead, numBytes, errorMessage);
      return false;
   }

   return success;
}


Nitf::TreState Nitf::Use00aParser::isTreValid(const DynamicObject& tre, ostream& reporter) const
{
   TreState status(VALID);
   set<string>             testSet;
   unsigned int numFields(0);

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter, &numFields,
      USE00A::ANGLE_TO_NORTH, 0U, 359U));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, USE00A::MEAN_GSD, 0.0F, 999.9F));

   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, USE00A::RESERVED1, testSet, true, true, false));

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, USE00A::DYNAMIC_RANGE, 0U, 99999U, true));

   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, USE00A::RESERVED2, testSet, true, true, false));

   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, USE00A::RESERVED3, testSet, true, true, false));

   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, USE00A::RESERVED4, testSet, true, true, false));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, USE00A::OBL_ANG, 0.0F, 90.0F, true));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, USE00A::ROLL_ANG, -90.0F, 90.0F, true));

   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, USE00A::RESERVED5, testSet, true, true, false));

   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, USE00A::RESERVED6, testSet, true, true, false));

   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, USE00A::RESERVED7, testSet, true, true, false));

   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, USE00A::RESERVED8, testSet, true, true, false));

   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, USE00A::RESERVED9, testSet, true, true, false));

   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, USE00A::RESERVED10, testSet, true, true, false));

   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, USE00A::RESERVED11, testSet, true, true, false));

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, USE00A::N_REF, 0U, 99U));

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, USE00A::REV_NUM, 1U, 99999U));

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, USE00A::N_SEG, 1U, 999U));

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, USE00A::MAX_LP_SEG, 1U, 999999U, true));

   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, USE00A::RESERVED12, testSet, true, true, false));

   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, USE00A::RESERVED13, testSet, true, true, false));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, USE00A::SUN_EL, -90.0F, 999.9F));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, USE00A::SUN_AZ, -90.0F, 999.9F));

   unsigned int totalFields = tre.getNumAttributes();
   if (status != INVALID && totalFields != numFields)
   {
      reporter << "Total fields in the Dyanmic Object(" <<
         totalFields << ") did not match the number found(" << numFields << ") ";
      status = INVALID;
   }

   if (status == INVALID)
   {
      reporter << " INVALID fields found in the USE00A TAG/SDE\n" ;
   }
   else if (status == SUSPECT)
   {
      reporter << " SUSPECT fields found in the USE00A TAG/SDE\n" ;
   }

   return status;
}


bool Nitf::Use00aParser::fromDynamicObject(const DynamicObject& input, ostream& output, size_t& numBytesWritten,
   string &errorMessage) const
{
   size_t sizeIn = max(static_cast<ostream::pos_type>(0), output.tellp());
   size_t sizeOut(sizeIn);

   try
   {
      output << toString(dv_cast<unsigned int>(input.getAttribute(USE00A::ANGLE_TO_NORTH)), 3, -1);
      output << toString(dv_cast<double>(input.getAttribute(USE00A::MEAN_GSD)), 5, 1);
      output << sizeString(dv_cast<string>(input.getAttribute(USE00A::RESERVED1)), 1);
      output << toString(dv_cast<unsigned int>(input.getAttribute(USE00A::DYNAMIC_RANGE)),
         5, -1, ZERO_FILL, false, false, 3, true);
      output << sizeString(dv_cast<string>(input.getAttribute(USE00A::RESERVED2)), 3);
      output << sizeString(dv_cast<string>(input.getAttribute(USE00A::RESERVED3)), 1);
      output << sizeString(dv_cast<string>(input.getAttribute(USE00A::RESERVED4)), 3);
      output << toString(dv_cast<double>(input.getAttribute(USE00A::OBL_ANG)),
         5, 2, ZERO_FILL, false, false, 3, true);
      output << toString(dv_cast<double>(input.getAttribute(USE00A::ROLL_ANG)),
         6, 2, ZERO_FILL, POS_SIGN_TRUE, false, 3, true);
      output << sizeString(dv_cast<string>(input.getAttribute(USE00A::RESERVED5)), 12);
      output << sizeString(dv_cast<string>(input.getAttribute(USE00A::RESERVED6)), 15);
      output << sizeString(dv_cast<string>(input.getAttribute(USE00A::RESERVED7)), 4);
      output << sizeString(dv_cast<string>(input.getAttribute(USE00A::RESERVED8)), 1);
      output << sizeString(dv_cast<string>(input.getAttribute(USE00A::RESERVED9)), 3);
      output << sizeString(dv_cast<string>(input.getAttribute(USE00A::RESERVED10)), 1);
      output << sizeString(dv_cast<string>(input.getAttribute(USE00A::RESERVED11)), 1);
      output << toString(dv_cast<unsigned int>(input.getAttribute(USE00A::N_REF)), 2, -1);
      output << toString(dv_cast<unsigned int>(input.getAttribute(USE00A::REV_NUM)), 5, -1);
      output << toString(dv_cast<unsigned int>(input.getAttribute(USE00A::N_SEG)), 3, -1);
      output << toString(dv_cast<unsigned int>(input.getAttribute(USE00A::MAX_LP_SEG)),
         6, -1, ZERO_FILL, false, false, 3, true);
      output << sizeString(dv_cast<string>(input.getAttribute(USE00A::RESERVED12)), 6);
      output << sizeString(dv_cast<string>(input.getAttribute(USE00A::RESERVED13)), 6);
      output << toString(dv_cast<double>(input.getAttribute(USE00A::SUN_EL)), 5, 1, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<double>(input.getAttribute(USE00A::SUN_AZ)), 5, 1);
   }
   catch (const bad_cast&)
   {
      return false;
   }

   sizeOut = output.tellp();
   numBytesWritten = sizeOut - sizeIn;
   return true;
}
