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
#include "NitfSensraParser.h"
#include "NitfUtilities.h"
#include "ObjectResource.h"

#include <sstream>
#include <string>

using namespace std;
using namespace Nitf;
using namespace Nitf::TRE;

Nitf::SensraParser::SensraParser()
{
   setName("SENSRA");
   setDescriptorId("{01B5123E-8509-4130-870E-5FE6A3EB9EAE}");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

bool Nitf::SensraParser::runAllTests(Progress* pProgress, ostream& failure)
{
   static const string data(
        "99999999"              // REF_ROW
        "99999999"              // REF_COL
        "abcdef"                // SENSOR_MODEL
        "-45"                   // SENSOR_MOUNT
        "+33.653456-084.423456" // SENSOR_LOC
        "M"                     // SENSOR_ALT_SOURCE
        "-01000"                // SENSOR_ALT
        "m"                     // SENSOR_ALT_UNIT
        "99000"                 // SENSOR_AGL
        "-90.000"               // SENSOR_PITCH
        "-180.000"              // SENSOR_ROLL
        "-180.000"              // SENSOR_YAW
        "-90.000"               // PLATFORM_PITCH
        "-180.000"              // PLATFORM_ROLL
        "359.9"                 // PLATFORM_HDG
        "N"                     // GROUND_SPD_SOURCE
        "9999.9"                // GROUND_SPD
        "k"                     // GRND_SPD_UNIT
        "359.9"                 // GROUND_TRACK
        "-9999"                 // VERT_VEL
        "m"                     // VERT_VEL_UNIT
        "9999"                  // SWATH_FRAMES
        "9999"                  // NUM_SWATHS
        "999"                   // SPOT_NUM
      );

   static const string data_error4(
        "99999999"              // REF_ROW
        "99999999"              // REF_COL
        "abcdef"                // SENSOR_MODEL
        "-45"                   // SENSOR_MOUNT
        "+33.653456-084.423456" // SENSOR_LOC
        "M"                     // SENSOR_ALT_SOURCE
        "-01000"                // SENSOR_ALT
        "m"                     // SENSOR_ALT_UNIT
        "99000"                 // SENSOR_AGL
        "-90.000"               // SENSOR_PITCH
        "-180.111"              // SENSOR_ROLL          min == -180.000
        "-180.000"              // SENSOR_YAW
        "-90.000"               // PLATFORM_PITCH
        "-180.000"              // PLATFORM_ROLL
        "359.9"                 // PLATFORM_HDG
        "N"                     // GROUND_SPD_SOURCE
        "9999.9"                // GROUND_SPD
        "k"                     // GRND_SPD_UNIT
        "359.9"                 // GROUND_TRACK
        "-9999"                 // VERT_VEL
        "m"                     // VERT_VEL_UNIT
        "9999"                  // SWATH_FRAMES
        "9999"                  // NUM_SWATHS
        "999"                   // SPOT_NUM
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
         failure << "Error: Negative test with LNSTRT = data out of range failed: did not return SUSPECT\n";
         failure << tmpStream.str();
         treDO->clear();
         return false;
      }
      status = VALID;
   }

   treDO->clear();

   return (status != INVALID);
}

bool Nitf::SensraParser::toDynamicObject(istream& input, size_t numBytes, DynamicObject& output,
   string &errorMessage) const
{
   vector<char> buf;
   bool ok(true);
   bool success(true);

   readField<unsigned int>(input, output, success, SENSRA::REF_ROW, 8, errorMessage, buf, true);
   readField<unsigned int>(input, output, success, SENSRA::REF_COL, 8, errorMessage, buf, true);
   readField<string>(input, output, success, SENSRA::SENSOR_MODEL, 6, errorMessage, buf, true);
   readField<int>(input, output, success, SENSRA::SENSOR_MOUNT, 3, errorMessage, buf, true);
   readField<string>(input, output, success, SENSRA::SENSOR_LOC, 21, errorMessage, buf, true);
   readField<string>(input, output, success, SENSRA::SENSOR_ALT_SOURCE, 1, errorMessage, buf, true);
   readField<int>(input, output, success, SENSRA::SENSOR_ALT, 6, errorMessage, buf, true);
   readField<string>(input, output, success, SENSRA::SENSOR_ALT_UNIT, 1, errorMessage, buf, true);
   readField<unsigned int>(input, output, success, SENSRA::SENSOR_AGL, 5, errorMessage, buf, true);
   readField<double>(input, output, success, SENSRA::SENSOR_PITCH, 7, errorMessage, buf, true);
   readField<double>(input, output, success, SENSRA::SENSOR_ROLL, 8, errorMessage, buf, true);
   readField<double>(input, output, success, SENSRA::SENSOR_YAW, 8, errorMessage, buf, true);
   readField<double>(input, output, success, SENSRA::PLATFORM_PITCH, 7, errorMessage, buf, true);
   readField<double>(input, output, success, SENSRA::PLATFORM_ROLL, 8, errorMessage, buf, true);
   readField<double>(input, output, success, SENSRA::PLATFORM_HDG, 5, errorMessage, buf, true);
   readField<string>(input, output, success, SENSRA::GROUND_SPD_SOURCE, 1, errorMessage, buf, true);
   readField<double>(input, output, success, SENSRA::GROUND_SPD, 6, errorMessage, buf, true);
   readField<string>(input, output, success, SENSRA::GRND_SPD_UNIT, 1, errorMessage, buf, true);
   readField<double>(input, output, success, SENSRA::GROUND_TRACK, 5, errorMessage, buf, true);
   readField<int>(input, output, success, SENSRA::VERT_VEL, 5, errorMessage, buf, true);
   readField<string>(input, output, success, SENSRA::VERT_VEL_UNIT, 1, errorMessage, buf, true);
   readField<unsigned int>(input, output, success, SENSRA::SWATH_FRAMES, 4, errorMessage, buf, true);
   readField<unsigned int>(input, output, success, SENSRA::NUM_SWATHS, 4, errorMessage, buf, true);
   readField<unsigned int>(input, output, success, SENSRA::SPOT_NUM, 3, errorMessage, buf, true);

   size_t numRead = input.tellg();
   if (numRead != numBytes)
   {
      numReadErrMsg(numRead, numBytes, errorMessage);
      return false;
   }

   return success;
}


Nitf::TreState Nitf::SensraParser::isTreValid(const DynamicObject& tre, ostream& reporter) const
{
   TreState status(VALID);
   set<string>             testSet;
   unsigned int numFields = 0;

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, SENSRA::REF_ROW, 0U, 99999999U));

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, SENSRA::REF_COL, 0U, 99999999U));

   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, SENSRA::SENSOR_MODEL, testSet, true, true, false));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, SENSRA::SENSOR_MOUNT, -45, 45));

   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, SENSRA::SENSOR_LOC, testSet, true, true, false));

   testSet.clear();
   testSet.insert("B");
   testSet.insert("G");
   testSet.insert("M");
   testSet.insert("R");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, SENSRA::SENSOR_ALT_SOURCE, testSet, ALL_BLANK_FALSE, NOT_IN_SET_FALSE, EMIT_MSG_NOT_IN_SET_TRUE));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, SENSRA::SENSOR_ALT, -1000, 99000));

   testSet.clear();
   testSet.insert("f");
   testSet.insert("m");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, SENSRA::SENSOR_ALT_UNIT, testSet, ALL_BLANK_FALSE, NOT_IN_SET_FALSE, EMIT_MSG_NOT_IN_SET_TRUE));

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, SENSRA::SENSOR_AGL, 10U, 99000U));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, SENSRA::SENSOR_PITCH, -90.1, 90.1));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, SENSRA::SENSOR_ROLL, -180.1, 180.1));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, SENSRA::SENSOR_YAW, -180.1, 180.1));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, SENSRA::PLATFORM_PITCH, -90.1, 90.1));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, SENSRA::PLATFORM_ROLL, -180.1, 180.1));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, SENSRA::PLATFORM_HDG, 0.0, 360.0));

   testSet.clear();
   testSet.insert("R");
   testSet.insert("N");
   testSet.insert("G");
   testSet.insert("M");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, SENSRA::GROUND_SPD_SOURCE, testSet, ALL_BLANK_TRUE, NOT_IN_SET_FALSE, EMIT_MSG_NOT_IN_SET_TRUE));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, SENSRA::GROUND_SPD, 0.0, 10000.0));

   testSet.clear();
   testSet.insert("k");
   testSet.insert("f");
   testSet.insert("m");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, SENSRA::GRND_SPD_UNIT, testSet, ALL_BLANK_FALSE, NOT_IN_SET_FALSE, EMIT_MSG_NOT_IN_SET_TRUE));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, SENSRA::GROUND_TRACK, 0.0, 360.0));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, SENSRA::VERT_VEL, -9999, 9999));

   testSet.clear();
   testSet.insert("f");
   testSet.insert("m");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, SENSRA::VERT_VEL_UNIT, testSet, ALL_BLANK_FALSE, NOT_IN_SET_FALSE, EMIT_MSG_NOT_IN_SET_TRUE));

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, SENSRA::SWATH_FRAMES, 1U, 9999U));

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, SENSRA::NUM_SWATHS, 1U, 9999U));

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, SENSRA::SPOT_NUM, 1U, 999U));

   unsigned int totalFields = tre.getNumAttributes();
   if (status != INVALID && totalFields != numFields)
   {
      reporter << "Total fields in the Dynamic Object(" <<
         totalFields <<") did not match the number found(" << numFields << ") ";
      status = INVALID;
   }

   if (status == INVALID)
   {
      reporter << " INVALID fields found in the SENSRA TAG/SDE\n" ;
   }
   else if (status == SUSPECT)
   {
      reporter << " SUSPECT fields found in the SENSRA TAG/SDE\n" ;
   }

   return status;
}


bool Nitf::SensraParser::fromDynamicObject(const DynamicObject& input, ostream& output, size_t& numBytesWritten,
   string &errorMessage) const
{
   size_t sizeIn = max(static_cast<ostream::pos_type>(0), output.tellp());
   size_t sizeOut(sizeIn);

   try
   {
      output <<   toString( dv_cast<unsigned int>(input.getAttribute(SENSRA::REF_ROW)), 8, -1);
      output <<   toString( dv_cast<unsigned int>(input.getAttribute(SENSRA::REF_COL)), 8, -1);
      output << sizeString( dv_cast<string>(input.getAttribute (SENSRA::SENSOR_MODEL)), 6);
      output <<   toString( dv_cast<int>(input.getAttribute(SENSRA::SENSOR_MOUNT)), 3, -1);
      output << sizeString( dv_cast<string>(input.getAttribute (SENSRA::SENSOR_LOC)), 21);
      output << sizeString( dv_cast<string>(input.getAttribute (SENSRA::SENSOR_ALT_SOURCE)), 1);
      output <<   toString( dv_cast<int>(input.getAttribute(SENSRA::SENSOR_ALT)), 6, -1);
      output << sizeString( dv_cast<string>(input.getAttribute (SENSRA::SENSOR_ALT_UNIT)), 1);
      output <<   toString( dv_cast<unsigned int>(input.getAttribute(SENSRA::SENSOR_AGL)), 5, -1);
      output <<   toString( dv_cast<double>(input.getAttribute(SENSRA::SENSOR_PITCH)), 7, 3, ZERO_FILL, POS_SIGN_TRUE);
      output <<   toString( dv_cast<double>(input.getAttribute(SENSRA::SENSOR_ROLL)), 8, 3, ZERO_FILL, POS_SIGN_TRUE);
      output <<   toString( dv_cast<double>(input.getAttribute(SENSRA::SENSOR_YAW)), 8, 3, ZERO_FILL, POS_SIGN_TRUE);
      output <<   toString( dv_cast<double>(input.getAttribute(SENSRA::PLATFORM_PITCH)), 7, 3, ZERO_FILL, POS_SIGN_TRUE);
      output <<   toString( dv_cast<double>(input.getAttribute(SENSRA::PLATFORM_ROLL)), 8, 3, ZERO_FILL, POS_SIGN_TRUE);
      output <<   toString( dv_cast<double>(input.getAttribute(SENSRA::PLATFORM_HDG)), 5, 1);
      output << sizeString( dv_cast<string>(input.getAttribute (SENSRA::GROUND_SPD_SOURCE)), 1);
      output <<   toString( dv_cast<double>(input.getAttribute(SENSRA::GROUND_SPD)), 6, 1);
      output << sizeString( dv_cast<string>(input.getAttribute (SENSRA::GRND_SPD_UNIT)), 1);
      output <<   toString( dv_cast<double>(input.getAttribute(SENSRA::GROUND_TRACK)), 5, 1);
      output <<   toString( dv_cast<int>(input.getAttribute(SENSRA::VERT_VEL)), 5, -1);
      output << sizeString( dv_cast<string>(input.getAttribute (SENSRA::VERT_VEL_UNIT)), 1);
      output <<   toString( dv_cast<unsigned int>(input.getAttribute(SENSRA::SWATH_FRAMES)), 4, -1);
      output <<   toString( dv_cast<unsigned int>(input.getAttribute(SENSRA::NUM_SWATHS)), 4, -1);
      output <<   toString( dv_cast<unsigned int>(input.getAttribute(SENSRA::SPOT_NUM)), 3, -1);
   }
   catch (const bad_cast&)
   {
      return false;
   }

   sizeOut = output.tellp();
   numBytesWritten = sizeOut - sizeIn;
   return true;
}
