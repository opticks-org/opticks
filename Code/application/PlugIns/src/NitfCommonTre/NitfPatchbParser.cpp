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
#include "NitfPatchbParser.h"
#include "NitfUtilities.h"
#include "ObjectResource.h"
#include "PlugInRegistration.h"

#include <sstream>

using namespace std;
using namespace Nitf;
using namespace Nitf::TRE;

REGISTER_PLUGIN(OpticksNitfCommonTre, PatchbParser, Nitf::PatchbParser());

Nitf::PatchbParser::PatchbParser()
{
   setName("PATCHB");
   setDescriptorId("{ADC5C380-EADB-4c39-A84E-B975183816DD}");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

bool Nitf::PatchbParser::runAllTests(Progress* pProgress, ostream& failure)
{
   static const string data(
      "0999"                 // PAT_NO
      "0"                    // LAST_PAT_FLAG
      "1234567"              // LNSTRT
      "1234567"              // LNSTOP
      "12345"                // AZL
      "12345"                // NVL
      "681"                  // FVL
      "43000"                // NPIXEL
      "43000"                // FVPIX
      "512"                  // FRAME
      "86399.99"             // UTC
      "359.999"              // SHEAD
      "31.0000"              // GRAVITY
      "09999"                // INS_V_NC
      "+9999"                // INS_V_EC
      "-9999"                // INS_V_DC
      "+80.0000"             // OFFLAT
      "-80.0000"             // OFFLONG
      "359"                  // TRACK
      "120.00"               // GSWEEP
      "1.000000"             // SHEAR
      "000001"               // BATCH_NO
      );

   static const string data_error4(
      "0999"                 // PAT_NO
      "0"                    // LAST_PAT_FLAG
      "0000000"              // LNSTRT
      "1234567"              // LNSTOP
      "12345"                // AZL
      "12345"                // NVL
      "681"                  // FVL
      "43000"                // NPIXEL
      "43000"                // FVPIX
      "512"                  // FRAME
      "86399.99"             // UTC
      "359.999"              // SHEAD
      "31.0000"              // GRAVITY
      "09999"                // INS_V_NC
      "+9999"                // INS_V_EC
      "-9999"                // INS_V_DC
      "+80.0000"             // OFFLAT
      "-80.0000"             // OFFLONG
      "359"                  // TRACK
      "120.00"               // GSWEEP
      "1.000000"             // SHEAR
      "000001"               // BATCH_NO
      );

   static const string data5(
      "0999"                 // PAT_NO
      "0"                    // LAST_PAT_FLAG - set to spaces
      "1234567"              // LNSTRT
      "1234567"              // LNSTOP
      "12345"                // AZL
      "     "                // NVL - set to spaces
      "   "                  // FVL - set to spaces
      "43000"                // NPIXEL
      "43000"                // FVPIX
      "   "                  // FRAME - set to spaces
      "86399.99"             // UTC
      "359.999"              // SHEAD
      "       "              // GRAVITY - set to spaces
      "+9999"                // INS_V_NC
      "+9999"                // INS_V_EC
      "-9999"                // INS_V_DC
      "        "             // OFFLAT - set to spaces
      "        "             // OFFLONG - set to spaces
      "359"                  // TRACK
      "120.00"               // GSWEEP
      "        "             // SHEAR - set to spaces
      "      "               // BATCH_NO - set to spaces
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

bool Nitf::PatchbParser::toDynamicObject(istream& input, size_t numBytes, DynamicObject& output,
   string &errorMessage) const
{
   vector<char> buf;
   bool success(true);

   readField<int>(input, output, success, PATCHB::PAT_NO, 4, errorMessage, buf);
   readField<int>(input, output, success, PATCHB::LAST_PAT_FLAG, 1, errorMessage, buf, true);
   readField<int>(input, output, success, PATCHB::LNSTRT, 7, errorMessage, buf);
   readField<int>(input, output, success, PATCHB::LNSTOP, 7, errorMessage, buf);
   readField<int>(input, output, success, PATCHB::AZL, 5, errorMessage, buf);
   readField<int>(input, output, success, PATCHB::NVL, 5, errorMessage, buf, true);
   readField<int>(input, output, success, PATCHB::FVL, 3, errorMessage, buf, true);
   readField<int>(input, output, success, PATCHB::NPIXEL, 5, errorMessage, buf);
   readField<int>(input, output, success, PATCHB::FVPIX, 5, errorMessage, buf);
   readField<int>(input, output, success, PATCHB::FRAME, 3, errorMessage, buf, true);
   readField<double>(input, output, success, PATCHB::UTC, 8, errorMessage, buf);
   readField<double>(input, output, success, PATCHB::SHEAD, 7, errorMessage, buf);
   readField<double>(input, output, success, PATCHB::GRAVITY, 7, errorMessage, buf, true);
   readField<int>(input, output, success, PATCHB::INS_V_NC, 5, errorMessage, buf);
   readField<int>(input, output, success, PATCHB::INS_V_EC, 5, errorMessage, buf);
   readField<int>(input, output, success, PATCHB::INS_V_DC, 5, errorMessage, buf);
   readField<double>(input, output, success, PATCHB::OFFLAT, 8, errorMessage, buf, true);
   readField<double>(input, output, success, PATCHB::OFFLONG, 8, errorMessage, buf, true);
   readField<int>(input, output, success, PATCHB::TRACK, 3, errorMessage, buf);
   readField<double>(input, output, success, PATCHB::GSWEEP, 6, errorMessage, buf);
   readField<double>(input, output, success, PATCHB::SHEAR, 8, errorMessage, buf, true);
   readField<int>(input, output, success, PATCHB::BATCH_NO, 6, errorMessage, buf, true);

   int64_t numRead = input.tellg();
   if (numRead < 0 || numRead > static_cast<int64_t>(std::numeric_limits<size_t>::max()) ||
      numRead != static_cast<int64_t>(numBytes))
   {
      numReadErrMsg(numRead, numBytes, errorMessage);
      return false;
   }

   return success;
}


Nitf::TreState Nitf::PatchbParser::isTreValid(const DynamicObject& tre, ostream& reporter) const
{
   TreState status(VALID);
   set<string> testSet;
   unsigned int numFields = 0;

   status = MaxState(status, testTagValueRange<int>(tre, reporter, &numFields, PATCHB::PAT_NO, 1, 999));
   status = MaxState(status, testTagValueRange<int>(tre, reporter, &numFields, PATCHB::LAST_PAT_FLAG, 0, 1, true));
   status = MaxState(status, testTagValueRange<int>(tre, reporter, &numFields, PATCHB::LNSTRT, 1, 9999999));
   status = MaxState(status, testTagValueRange<int>(tre, reporter, &numFields, PATCHB::LNSTOP, 20, 9999999));
   status = MaxState(status, testTagValueRange<int>(tre, reporter, &numFields, PATCHB::AZL, 20, 99999));
   status = MaxState(status, testTagValueRange<int>(tre, reporter, &numFields, PATCHB::NVL, 20, 99999, true));
   status = MaxState(status, testTagValueRange<int>(tre, reporter, &numFields, PATCHB::FVL, 1, 681, true));
   status = MaxState(status, testTagValueRange<int>(tre, reporter, &numFields, PATCHB::NPIXEL, 170, 43000));
   status = MaxState(status, testTagValueRange<int>(tre, reporter, &numFields, PATCHB::FVPIX, 1, 43000));
   status = MaxState(status, testTagValueRange<int>(tre, reporter, &numFields, PATCHB::FRAME, 1, 512, true));
   status = MaxState(status, testTagValueRange<double>(tre, reporter, &numFields, PATCHB::UTC, 0.0, 86399.99));
   status = MaxState(status, testTagValueRange<double>(tre, reporter, &numFields, PATCHB::SHEAD, 0.0, 359.999));
   status = MaxState(status, testTagValueRange<double>(tre, reporter, &numFields,
      PATCHB::GRAVITY, 31.0, 33.9999, true));
   status = MaxState(status, testTagValueRange<int>(tre, reporter, &numFields, PATCHB::INS_V_NC, -9999, 9999));
   status = MaxState(status, testTagValueRange<int>(tre, reporter, &numFields, PATCHB::INS_V_EC, -9999, 9999));
   status = MaxState(status, testTagValueRange<int>(tre, reporter, &numFields, PATCHB::INS_V_DC, -9999, 9999));
   status = MaxState(status, testTagValueRange<double>(tre, reporter, &numFields, PATCHB::OFFLAT, -80.0, 80.0, true));
   status = MaxState(status, testTagValueRange<double>(tre, reporter, &numFields, PATCHB::OFFLONG, -80.0, 80.0, true));
   status = MaxState(status, testTagValueRange<int>(tre, reporter, &numFields, PATCHB::TRACK, 0, 359));
   status = MaxState(status, testTagValueRange<double>(tre, reporter, &numFields, PATCHB::GSWEEP, 0.0, 120.00));
   status = MaxState(status, testTagValueRange<double>(tre, reporter, &numFields, PATCHB::SHEAR, 0.85, 1.0, true));
   status = MaxState(status, testTagValueRange<int>(tre, reporter, &numFields, PATCHB::BATCH_NO, 1, 999999, true));

   unsigned int totalFields = tre.getNumAttributes();
   if (status != INVALID && totalFields != numFields)
   {
      reporter << "Total fields in the Dynamic Object(" <<
         totalFields << ") did not match the number found(" << numFields << ") ";
      status = INVALID;
   }

   if (status == INVALID)
   {
      reporter << " INVALID fields found in the PATCHB TAG/SDE\n" ;
   }
   else if (status == SUSPECT)
   {
      reporter << " SUSPECT fields found in the PATCHB TAG/SDE\n" ;
   }

   return status;
}


bool Nitf::PatchbParser::fromDynamicObject(const DynamicObject& input, ostream& output, size_t& numBytesWritten,
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
      output << toString(dv_cast<int>(input.getAttribute(PATCHB::PAT_NO)), 4);
      output << toString(dv_cast<int>(input.getAttribute(PATCHB::LAST_PAT_FLAG)), 1,
         -1, ZERO_FILL, false, false, 3, true);
      output << toString(dv_cast<int>(input.getAttribute(PATCHB::LNSTRT)), 7);
      output << toString(dv_cast<int>(input.getAttribute(PATCHB::LNSTOP)), 7);
      output << toString(dv_cast<int>(input.getAttribute(PATCHB::AZL)), 5);
      output << toString(dv_cast<int>(input.getAttribute(PATCHB::NVL)), 5,
         -1, ZERO_FILL, false, false, 3, true);
      output << toString(dv_cast<int>(input.getAttribute(PATCHB::FVL)), 3,
         -1, ZERO_FILL, false, false, 3, true);
      output << toString(dv_cast<int>(input.getAttribute(PATCHB::NPIXEL)), 5);
      output << toString(dv_cast<int>(input.getAttribute(PATCHB::FVPIX)), 5);
      output << toString(dv_cast<int>(input.getAttribute(PATCHB::FRAME)), 3,
         -1, ZERO_FILL, false, false, 3, true);
      output << toString(dv_cast<double>(input.getAttribute(PATCHB::UTC)), 8, 2);
      output << toString(dv_cast<double>(input.getAttribute(PATCHB::SHEAD)), 7, 3);
      output << toString(dv_cast<double>(input.getAttribute(PATCHB::GRAVITY)), 7, 4,
         ZERO_FILL, false, false, 3, true);
      output << toString(dv_cast<int>(input.getAttribute(PATCHB::INS_V_NC)), 5, -1, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<int>(input.getAttribute(PATCHB::INS_V_EC)), 5, -1, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<int>(input.getAttribute(PATCHB::INS_V_DC)), 5, -1, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<double>(input.getAttribute(PATCHB::OFFLAT)), 8, 4, ZERO_FILL, POS_SIGN_TRUE,
         false, 3, true);
      output << toString(dv_cast<double>(input.getAttribute(PATCHB::OFFLONG)), 8, 4, ZERO_FILL, POS_SIGN_TRUE,
         false, 3, true);
      output << toString(dv_cast<int>(input.getAttribute(PATCHB::TRACK)), 3);
      output << toString(dv_cast<double>(input.getAttribute(PATCHB::GSWEEP)), 6, 2);
      output << toString(dv_cast<double>(input.getAttribute(PATCHB::SHEAR)), 8,
         -1, ZERO_FILL, false, false, 3, true);
      output << toString(dv_cast<int>(input.getAttribute(PATCHB::BATCH_NO)), 6,
         -1, ZERO_FILL, false, false, 3, true);
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
