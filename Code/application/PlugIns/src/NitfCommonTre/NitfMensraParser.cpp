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
#include "NitfMensraParser.h"
#include "NitfUtilities.h"
#include "ObjectResource.h"
#include "PlugInRegistration.h"

#include <sstream>

using namespace std;
using namespace Nitf;
using namespace Nitf::TRE;

REGISTER_PLUGIN(OpticksNitfCommonTre, MensraParser, Nitf::MensraParser());

Nitf::MensraParser::MensraParser()
{
   setName("MENSRA");
   setDescriptorId("{88FB9901-FE7D-4cdc-9CD9-97FDA1A44E43}");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

bool Nitf::MensraParser::runAllTests(Progress* pProgress, ostream& failure)
{
   static const string data(
      "333030.12X1023030.12Y"   // CCRP_LOC
      "023456"                  // CCRP_ALT
      "+2000.0"                 // OF_PC_R
      "-2000.0"                 // OF_PC_A
      "0.50000"                 // COSGRZ
      "1234567"                 // RGCCRP
      "R"                       // RLMAP
      "19999"                   // CCRP_ROW
      "19999"                   // CCRP_COL
      "333030.12X1023030.12Y"   // ACFT_LOC
      "12345"                   // ACFT_ALT
      "0.50000"                 // C_R_NC
      "-0.5000"                 // C_R_EC
      "0.50000"                 // C_R_DC
      "-0.5000"                 // C_AZ_NC
      "0.50000"                 // C_AZ_EC
      "-0.5000"                 // C_AZ_DC
      "0.50000"                 // C_AL_NC
      "-0.5000"                 // C_AL_EC
      "0.50000"                 // C_AL_DC
      );

   static const string data_error4(
      "333030.12X1023030.12Y"   // CCRP_LOC
      "030001"                  // CCRP_ALT   // ERROR: -1000 <= CCRP_ALT <= 30000
      "+2000.0"                 // OF_PC_R
      "-2000.0"                 // OF_PC_A
      "0.50000"                 // COSGRZ
      "1234567"                 // RGCCRP
      "R"                       // RLMAP
      "19999"                   // CCRP_ROW
      "19999"                   // CCRP_COL
      "333030.12X1023030.12Y"   // ACFT_LOC
      "12345"                   // ACFT_ALT
      "0.50000"                 // C_R_NC
      "-0.5000"                 // C_R_EC
      "0.50000"                 // C_R_DC
      "-0.5000"                 // C_AZ_NC
      "0.50000"                 // C_AZ_EC
      "-0.5000"                 // C_AZ_DC
      "0.50000"                 // C_AL_NC
      "-0.5000"                 // C_AL_EC
      "0.50000"                 // C_AL_DC
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

   // start of test 4 - Negative test
   stringstream input4(data_error4);
   numBytes = input4.str().size();

   success = toDynamicObject(input4, numBytes, *treDO.get(), errorMessage);
   if (success != true)  // negative test so success must == false.
   {
      failure << "Error: Parse failed\n";
      treDO->clear();
      return false;
   }

   stringstream tmpStream;
   status = isTreValid(*treDO.get(), tmpStream);
   if (status != SUSPECT)
   {
      failure << "Error: Negative test -1000 <= CCRP_ALT <= 30000 failed did not return SUSPECT\n";
      failure << tmpStream.str();
      treDO->clear();
      return false;
   }
   status = VALID;

   treDO->clear();

   return (status != INVALID);
}

bool Nitf::MensraParser::toDynamicObject(istream& input, size_t numBytes, DynamicObject& output,
   string &errorMessage) const
{
   vector<char> buf;
   bool success(true);

   readField<string>(input, output, success, MENSRA::CCRP_LOC, 21, errorMessage, buf);
   readField<int>(input, output, success, MENSRA::CCRP_ALT, 6, errorMessage, buf);
   readField<double>(input, output, success, MENSRA::OF_PC_R, 7, errorMessage, buf);
   readField<double>(input, output, success, MENSRA::OF_PC_A, 7, errorMessage, buf);
   readField<double>(input, output, success, MENSRA::COSGRZ, 7, errorMessage, buf);
   readField<int>(input, output, success, MENSRA::RGCCRP, 7, errorMessage, buf);
   readField<string>(input, output, success, MENSRA::RLMAP, 1, errorMessage, buf);
   readField<int>(input, output, success, MENSRA::CCRP_ROW, 5, errorMessage, buf);
   readField<int>(input, output, success, MENSRA::CCRP_COL, 5, errorMessage, buf);
   readField<string>(input, output, success, MENSRA::ACFT_LOC, 21, errorMessage, buf);
   readField<int>(input, output, success, MENSRA::ACFT_ALT, 5, errorMessage, buf);
   readField<double>(input, output, success, MENSRA::C_R_NC, 7, errorMessage, buf);
   readField<double>(input, output, success, MENSRA::C_R_EC, 7, errorMessage, buf);
   readField<double>(input, output, success, MENSRA::C_R_DC, 7, errorMessage, buf);
   readField<double>(input, output, success, MENSRA::C_AZ_NC, 7, errorMessage, buf);
   readField<double>(input, output, success, MENSRA::C_AZ_EC, 7, errorMessage, buf);
   readField<double>(input, output, success, MENSRA::C_AZ_DC, 7, errorMessage, buf);
   readField<double>(input, output, success, MENSRA::C_AL_NC, 7, errorMessage, buf);
   readField<double>(input, output, success, MENSRA::C_AL_EC, 7, errorMessage, buf);
   readField<double>(input, output, success, MENSRA::C_AL_DC, 7, errorMessage, buf);

   int64_t numRead = input.tellg();
   if (numRead < 0 || static_cast<uint64_t>(numRead) > std::numeric_limits<size_t>::max() ||
      numRead != static_cast<int64_t>(numBytes))
   {
      numReadErrMsg(numRead, numBytes, errorMessage);
      return false;
   }

   return success;
}



Nitf::TreState Nitf::MensraParser::isTreValid(const DynamicObject& tre, ostream& reporter) const
{
   TreState status(VALID);
   set<string> testSet;
   unsigned int numFields = 0;

   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, MENSRA::CCRP_LOC, testSet, false, true, false));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, MENSRA::CCRP_ALT, -1000, 30000));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, MENSRA::OF_PC_R, -2000.0, 2000.0));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, MENSRA::OF_PC_A, -2000.0, 2000.0));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, MENSRA::COSGRZ, 0.0, 1.0));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, MENSRA::RGCCRP, 0, 3000000));

   testSet.clear();
   testSet.insert("L");
   testSet.insert("R");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, MENSRA::RLMAP, testSet, false, false, true));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, MENSRA::CCRP_ROW, 0, 19999));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, MENSRA::CCRP_COL, 0, 19999));

   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, MENSRA::ACFT_LOC, testSet, false, true, false));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, MENSRA::ACFT_ALT, 0, 99999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, MENSRA::C_R_NC, -1.0, 1.0));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, MENSRA::C_R_EC, -1.0, 1.0));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, MENSRA::C_R_DC, -1.0, 1.0));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, MENSRA::C_AZ_NC, -1.0, 1.0));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, MENSRA::C_AZ_EC, -1.0, 1.0));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, MENSRA::C_AZ_DC, -1.0, 1.0));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, MENSRA::C_AL_NC, -1.0, 1.0));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, MENSRA::C_AL_EC, -1.0, 1.0));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, MENSRA::C_AL_DC, -1.0, 1.0));

   unsigned int totalFields = tre.getNumAttributes();
   if (status != INVALID && totalFields != numFields)
   {
      reporter << "Total fields in the Dynamic Object(" <<
         totalFields << ") did not match the number found(" << numFields << ") ";
      status = INVALID;
   }

   if (status == INVALID)
   {
      reporter << " INVALID fields found in the MENSRA TAG/SDE\n" ;
   }
   else if (status == SUSPECT)
   {
      reporter << " SUSPECT fields found in the MENSRA TAG/SDE\n" ;
   }

   return status;
}



bool Nitf::MensraParser::fromDynamicObject(const DynamicObject& input, ostream& output, size_t& numBytesWritten,
   string &errorMessage) const
{
   if (output.tellp() < 0 || static_cast<uint64_t>(output.tellp()) > std::numeric_limits<size_t>::max())
   {
      return false;
   }
   size_t sizeIn = max<size_t>(0, static_cast<size_t>(output.tellp()));
   size_t sizeOut(sizeIn);

   try
   {
      output << sizeString(dv_cast<string>(input.getAttribute(MENSRA::CCRP_LOC)), 21);
      output << toString(dv_cast<int>(input.getAttribute(MENSRA::CCRP_ALT)), 6, -1, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<double>(input.getAttribute(MENSRA::OF_PC_R)), 7, 1, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<double>(input.getAttribute(MENSRA::OF_PC_A)), 7, 1, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<double>(input.getAttribute(MENSRA::COSGRZ)), 7, 5);
      output << toString(dv_cast<int>(input.getAttribute(MENSRA::RGCCRP)), 7);
      output << sizeString(dv_cast<string>(input.getAttribute(MENSRA::RLMAP)), 1);
      output << toString(dv_cast<int>(input.getAttribute(MENSRA::CCRP_ROW)), 5);
      output << toString(dv_cast<int>(input.getAttribute(MENSRA::CCRP_COL)), 5);
      output << sizeString(dv_cast<string>(input.getAttribute(MENSRA::ACFT_LOC)), 21);
      output << toString(dv_cast<int>(input.getAttribute(MENSRA::ACFT_ALT)), 5);
      output << toString(dv_cast<double>(input.getAttribute(MENSRA::C_R_NC)), 7, -1, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<double>(input.getAttribute(MENSRA::C_R_EC)), 7, -1, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<double>(input.getAttribute(MENSRA::C_R_DC)), 7, -1, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<double>(input.getAttribute(MENSRA::C_AZ_NC)), 7, -1, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<double>(input.getAttribute(MENSRA::C_AZ_EC)), 7, -1, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<double>(input.getAttribute(MENSRA::C_AZ_DC)), 7, -1, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<double>(input.getAttribute(MENSRA::C_AL_NC)), 7, -1, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<double>(input.getAttribute(MENSRA::C_AL_EC)), 7, -1, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<double>(input.getAttribute(MENSRA::C_AL_DC)), 7, -1, ZERO_FILL, POS_SIGN_TRUE);
   }
   catch (const bad_cast&)
   {
      return false;
   }

   if (output.tellp() < 0 || static_cast<uint64_t>(output.tellp()) > std::numeric_limits<size_t>::max())
   {
      return false;
   }
   sizeOut = static_cast<size_t>(output.tellp());
   numBytesWritten = sizeOut - sizeIn;
   return true;
}
