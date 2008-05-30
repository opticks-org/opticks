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
#include "NitfMensrbParser.h"
#include "NitfUtilities.h"
#include "ObjectResource.h"

#include <sstream>

using namespace std;
using namespace Nitf;
using namespace Nitf::TRE;

Nitf::MensrbParser::MensrbParser()
{
   setName("MENSRB");
   setDescriptorId("{D9C19A09-C990-4e9d-82A1-5DEB7345ADBD}");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

bool Nitf::MensrbParser::runAllTests(Progress* pProgress, ostream& failure)
{
   static const string data(
      "333030.1234X1023030.1234Y" // ACFT_LOC
      "123.45"                    // ACFT_LOC_ACCY
      "123456"                    // ACFT_ALT
      "333030.1234X1023030.1234Y" // RP_LOC
      "999.99"                    // RP_LOC_ACCY
      "-01000"                    // RP_ELEV
      "-9999.9"                   // OF_PC_R
      "+9999.9"                   // OF_PC_A
      "1.00000"                   // COSGRZ
      "3000000"                   // RGCRP
      "L"                         // RLMAP
      "99999"                     // RP_ROW
      "00001"                     // RP_COL
      "0.50000000"                // C_R_NC
      "-0.5000000"                // C_R_EC
      "0.50000000"                // C_R_DC
      "-0.500000"                 // C_AZ_NC
      "0.5000000"                 // C_AZ_EC
      "-0.500000"                 // C_AZ_DC
      "0.5000000"                 // C_AL_NC
      "-0.500000"                 // C_AL_EC
      "0.5000000"                 // C_AL_DC
      "001"                       // TOTAL_TILES_COLS
      "00001"                     // TOTAL_TILES_ROWS
      );

   static const string data_error4(
      "333030.1234X1023030.1234Y" // ACFT_LOC
      "123.45"                    // ACFT_LOC_ACCY
      "123456"                    // ACFT_ALT
      "333030.1234X1023030.1234Y" // RP_LOC
      "999.99"                    // RP_LOC_ACCY
      "-02000"                    // RP_ELEV          // ERROR: data out of range
      "-9999.9"                   // OF_PC_R
      "+9999.9"                   // OF_PC_A
      "1.00000"                   // COSGRZ
      "3000000"                   // RGCRP
      "L"                         // RLMAP
      "99999"                     // RP_ROW
      "00001"                     // RP_COL
      "0.50000000"                // C_R_NC
      "-0.5000000"                // C_R_EC
      "0.50000000"                // C_R_DC
      "-0.500000"                 // C_AZ_NC
      "0.5000000"                 // C_AZ_EC
      "-0.500000"                 // C_AZ_DC
      "0.5000000"                 // C_AL_NC
      "-0.500000"                 // C_AL_EC
      "0.5000000"                 // C_AL_DC
      "001"                       // TOTAL_TILES_COLS
      "00001"                     // TOTAL_TILES_ROWS
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
         failure << "Error: Negative test with RP_ELEV = data out of range failed: did not return SUSPECT\n";
         failure << tmpStream.str();
         treDO->clear();
         return false;
      }
      status = VALID;
   }

   treDO->clear();

   return (status != INVALID);
}


bool Nitf::MensrbParser::toDynamicObject(istream& input, size_t numBytes, DynamicObject& output,
   string &errorMessage) const
{
   vector<char> buf;
   bool ok(true);
   bool success(true);

   readField<string>(input, output, success, MENSRB::ACFT_LOC, 25, errorMessage, buf);
   readField<double>(input, output, success, MENSRB::ACFT_LOC_ACCY, 6, errorMessage, buf);
   readField<int>(input, output, success, MENSRB::ACFT_ALT, 6, errorMessage, buf);
   readField<string>(input, output, success, MENSRB::RP_LOC, 25, errorMessage, buf);
   readField<double>(input, output, success, MENSRB::RP_LOC_ACCY, 6, errorMessage, buf);
   readField<int>(input, output, success, MENSRB::RP_ELEV, 6, errorMessage, buf);
   readField<double>(input, output, success, MENSRB::OF_PC_R, 7, errorMessage, buf, true);
   readField<double>(input, output, success, MENSRB::OF_PC_A, 7, errorMessage, buf, true);
   readField<double>(input, output, success, MENSRB::COSGRZ, 7, errorMessage, buf);
   readField<int>(input, output, success, MENSRB::RGCRP, 7, errorMessage, buf);
   readField<string>(input, output, success, MENSRB::RLMAP, 1, errorMessage, buf);
   readField<int>(input, output, success, MENSRB::RP_ROW, 5, errorMessage, buf, true);
   readField<int>(input, output, success, MENSRB::RP_COL, 5, errorMessage, buf, true);
   readField<double>(input, output, success, MENSRB::C_R_NC, 10, errorMessage, buf);
   readField<double>(input, output, success, MENSRB::C_R_EC, 10, errorMessage, buf);
   readField<double>(input, output, success, MENSRB::C_R_DC, 10, errorMessage, buf);
   readField<double>(input, output, success, MENSRB::C_AZ_NC, 9, errorMessage, buf);
   readField<double>(input, output, success, MENSRB::C_AZ_EC, 9, errorMessage, buf);
   readField<double>(input, output, success, MENSRB::C_AZ_DC, 9, errorMessage, buf);
   readField<double>(input, output, success, MENSRB::C_AL_NC, 9, errorMessage, buf);
   readField<double>(input, output, success, MENSRB::C_AL_EC, 9, errorMessage, buf);
   readField<double>(input, output, success, MENSRB::C_AL_DC, 9, errorMessage, buf);
   readField<int>(input, output, success, MENSRB::TOTAL_TILES_COLS, 3, errorMessage, buf, true);
   readField<int>(input, output, success, MENSRB::TOTAL_TILES_ROWS, 5, errorMessage, buf, true);

   size_t numRead = input.tellg();
   if (numRead != numBytes)
   {
      numReadErrMsg(numRead, numBytes, errorMessage);
      return false;
   }

   return success;
}



Nitf::TreState Nitf::MensrbParser::isTreValid(const DynamicObject& tre, ostream& reporter) const
{
   TreState status(VALID);
   set<string> testSet;
   unsigned int numFields = 0;

   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, MENSRB::ACFT_LOC, testSet, false, true, false));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, MENSRB::ACFT_LOC_ACCY, 0.0, 999.99));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, MENSRB::ACFT_ALT, 0, 999999));

   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, MENSRB::RP_LOC, testSet, false, true, false));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, MENSRB::RP_LOC_ACCY, 0.0, 999.99));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, MENSRB::RP_ELEV, -1000, 30000));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, MENSRB::OF_PC_R, -9999.9, 9999.9));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, MENSRB::OF_PC_A, -9999.9, 9999.9));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, MENSRB::COSGRZ, 0.0, 1.0));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, MENSRB::RGCRP, 0, 3000000));

   testSet.clear();
   testSet.insert("L");
   testSet.insert("R");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, MENSRB::RLMAP, testSet, false, false, true));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, MENSRB::RP_ROW, 1, 99999));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, MENSRB::RP_COL, 1, 99999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, MENSRB::C_R_NC, -1.0, 1.0));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, MENSRB::C_R_EC, -1.0, 1.0));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, MENSRB::C_R_DC, -1.0, 1.0));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, MENSRB::C_AZ_NC, -1.0, 1.0));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, MENSRB::C_AZ_EC, -1.0, 1.0));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, MENSRB::C_AZ_DC, -1.0, 1.0));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, MENSRB::C_AL_NC, -1.0, 1.0));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, MENSRB::C_AL_EC, -1.0, 1.0));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, MENSRB::C_AL_DC, -1.0, 1.0));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, MENSRB::TOTAL_TILES_COLS, 1, 999));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, MENSRB::TOTAL_TILES_ROWS, 1, 999));

   unsigned int totalFields = tre.getNumAttributes();
   if (status != INVALID && totalFields != numFields)
   {
      reporter << "Total fields in the Dynamic Object(" <<
         totalFields <<") did not match the number found(" << numFields << ") ";
      status = INVALID;
   }

   if (status == INVALID)
   {
      reporter << " INVALID fields found in the MENSRB TAG/SDE\n" ;
   }
   else if (status == SUSPECT)
   {
      reporter << " SUSPECT fields found in the MENSRB TAG/SDE\n" ;
   }

   return status;
}

bool Nitf::MensrbParser::fromDynamicObject(const DynamicObject& input, ostream& output, size_t& numBytesWritten,
   string &errorMessage) const
{
   size_t sizeIn = max(static_cast<ostream::pos_type>(0), output.tellp());
   size_t sizeOut(sizeIn);

   try
   {
      output << sizeString( dv_cast<string>(input.getAttribute(MENSRB::ACFT_LOC)), 25);
      output <<   toString( dv_cast<double>(input.getAttribute(MENSRB::ACFT_LOC_ACCY)), 6, 2);
      output <<   toString( dv_cast<int>(input.getAttribute(MENSRB::ACFT_ALT)), 6);
      output << sizeString( dv_cast<string>(input.getAttribute(MENSRB::RP_LOC)), 25);
      output <<   toString( dv_cast<double>(input.getAttribute(MENSRB::RP_LOC_ACCY)), 6, 2);
      output <<   toString( dv_cast<int>(input.getAttribute(MENSRB::RP_ELEV)), 6, -1, ZERO_FILL, POS_SIGN_TRUE);
      output <<   toString( dv_cast<double>(input.getAttribute(MENSRB::OF_PC_R)), 7, 1, ZERO_FILL, POS_SIGN_TRUE);
      output <<   toString( dv_cast<double>(input.getAttribute(MENSRB::OF_PC_A)), 7, 1, ZERO_FILL, POS_SIGN_TRUE);
      output <<   toString( dv_cast<double>(input.getAttribute(MENSRB::COSGRZ)), 7);
      output <<   toString( dv_cast<int>(input.getAttribute(MENSRB::RGCRP)), 7);
      output << sizeString( dv_cast<string>(input.getAttribute(MENSRB::RLMAP)), 1);
      output <<   toString( dv_cast<int>(input.getAttribute(MENSRB::RP_ROW)), 5);
      output <<   toString( dv_cast<int>(input.getAttribute(MENSRB::RP_COL)), 5);
      output <<   toString( dv_cast<double>(input.getAttribute(MENSRB::C_R_NC)), 10, -1, ZERO_FILL, POS_SIGN_TRUE);
      output <<   toString( dv_cast<double>(input.getAttribute(MENSRB::C_R_EC)), 10, -1, ZERO_FILL, POS_SIGN_TRUE);
      output <<   toString( dv_cast<double>(input.getAttribute(MENSRB::C_R_DC)), 10, -1, ZERO_FILL, POS_SIGN_TRUE);
      output <<   toString( dv_cast<double>(input.getAttribute(MENSRB::C_AZ_NC)), 9, -1, ZERO_FILL, POS_SIGN_TRUE);
      output <<   toString( dv_cast<double>(input.getAttribute(MENSRB::C_AZ_EC)), 9, -1, ZERO_FILL, POS_SIGN_TRUE);
      output <<   toString( dv_cast<double>(input.getAttribute(MENSRB::C_AZ_DC)), 9, -1, ZERO_FILL, POS_SIGN_TRUE);
      output <<   toString( dv_cast<double>(input.getAttribute(MENSRB::C_AL_NC)), 9, -1, ZERO_FILL, POS_SIGN_TRUE);
      output <<   toString( dv_cast<double>(input.getAttribute(MENSRB::C_AL_EC)), 9, -1, ZERO_FILL, POS_SIGN_TRUE);
      output <<   toString( dv_cast<double>(input.getAttribute(MENSRB::C_AL_DC)), 9, -1, ZERO_FILL, POS_SIGN_TRUE);
      output <<   toString( dv_cast<int>(input.getAttribute(MENSRB::TOTAL_TILES_COLS)), 3);
      output <<   toString( dv_cast<int>(input.getAttribute(MENSRB::TOTAL_TILES_ROWS)), 5);
   }
   catch (const bad_cast&)
   {
      return false;
   }

   sizeOut = output.tellp();
   numBytesWritten = sizeOut - sizeIn;
   return true;
}
