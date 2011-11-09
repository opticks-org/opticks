/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"
#include "AppVersion.h"
#include "DataVariant.h"
#include "DateTime.h"
#include "NitfAcftaParser.h"
#include "NitfConstants.h"
#include "NitfUtilities.h"
#include "ObjectResource.h"
#include "PlugInRegistration.h"

#include <set>
#include <sstream>
#include <string>

#include <boost/algorithm/string/case_conv.hpp>

using namespace std;
using namespace Nitf;
using namespace Nitf::TRE;

REGISTER_PLUGIN(OpticksNitfCommonTre, AcftaParser, Nitf::AcftaParser());

Nitf::AcftaParser::AcftaParser()
{
   setName("ACFTA");
   setDescriptorId("{18C38BF9-7C76-4f35-9354-821DBDDD65D5}");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

bool Nitf::AcftaParser::runAllTests(Progress* pProgress, ostream& failure)
{
   static const string data(
      "AC_MSN_IDX"               // AC_MSN_ID
      "C"                        // SCTYPE
      "1234"                     // SCNUM
      "ASR"                      // SENSOR_ID
      "0234"                     // PATCH_TOT
      "123"                      // MTI_TOT
      "16JUN06"                  // PDATE
      "123"                      // IMHOSTNO
      "12345"                    // IMREQID
      "1"                        // SCENE_SOURCE
      "12"                       // MPLAN
      "-88.000000-100.000000"    // ENTLOC
      "+01000"                   // ENTELV
      "-88.000000-100.000000"    // EXITLOC
      "+01000"                   // EXITELV
      "090.000"                  // TMAP
      "060"                      // RCS
      "01.0000"                  // ROW_SPACING
      "01.0000"                  // COL_SPACING
      "1234"                     // SENSERIAL
      "1234.12"                  // ABSWVER
      );

   static const string negdata4(             // test case 4 with SUSPECT data
      "AC_MSN_IDX"               // AC_MSN_ID
      "C"                        // SCTYPE
      "1234"                     // SCNUM
      "ASR"                      // SENSOR_ID
      "0234"                     // PATCH_TOT
      "123"                      // MTI_TOT
      "16JUN06"                  // PDATE
      "123"                      // IMHOSTNO
      "12345"                    // IMREQID
      "1"                        // SCENE_SOURCE
      "12"                       // MPLAN
      "-88.000000-100.000000"    // ENTLOC
      "-02000"                   // ENTELV         // SUSPECT: min value == -1000
      "-88.000000-100.000000"    // EXITLOC
      "+01000"                   // EXITELV
      "090.000"                  // TMAP
      "060"                      // RCS
      "01.0000"                  // ROW_SPACING
      "01.0000"                  // COL_SPACING
      "1234"                     // SENSERIAL
      "1234.12"                  // ABSWVER
      );

   static const string data_error5(
      "AC_MSN_IDX"               // AC_MSN_ID
      "C"                        // SCTYPE
      "1234"                     // SCNUM
      "ASR"                      // SENSOR_ID
      "0234"                     // PATCH_TOT
      "123"                      // MTI_TOT
      "16JUN06"                  // PDATE
      "123"                      // IMHOSTNO
      "12345"                    // IMREQID
      "1"                        // SCENE_SOURCE
      "12"                       // MPLAN
      "-88.000000-100.000000"    // ENTLOC
      "+01000"                   // ENTELV
      "-88.000000-100.000000"    // EXITLOC
      "+01000"                   // EXITELV
      "090.000"                  // TMAP
      "060"                      // RCS
      "01.0000"                  // ROW_SPACING
      "01.0000"                  // COL_SPACING
      "123G"                     // SENSERIAL        // ERROR: alpha in numeric
      "1234.12"                  // ABSWVER
      );

   static const string data6(
      "AC_MSN_IDX"               // AC_MSN_ID
      "C"                        // SCTYPE
      "1234"                     // SCNUM
      "ASR"                      // SENSOR_ID
      "0234"                     // PATCH_TOT
      "123"                      // MTI_TOT
      "16JUN06"                  // PDATE
      "123"                      // IMHOSTNO
      "12345"                    // IMREQID
      "1"                        // SCENE_SOURCE
      "12"                       // MPLAN
      "-88.000000-100.000000"    // ENTLOC
      "+01000"                   // ENTELV
      "-88.000000-100.000000"    // EXITLOC
      "      "                   // EXITELV - set to spaces
      "090.000"                  // TMAP
      "   "                      // RCS - set to spaces
      "01.0000"                  // ROW_SPACING
      "01.0000"                  // COL_SPACING
      "    "                     // SENSERIAL - set to spaces
      "1234.12"                  // ABSWVER
      );

   FactoryResource<DynamicObject> treDO;
   size_t numBytes(0);


   // Start of test 1
   stringstream input(data);
   numBytes = data.size();

   string errorMessage;
   bool success = toDynamicObject(input, numBytes, *treDO.get(), errorMessage);

   if (!errorMessage.empty())
   {
      failure << errorMessage << endl;
      errorMessage.clear();
   }

   TreState status(INVALID);
   if (success == true)
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

   // Start of test 4 - SUSPECT test
   stringstream input4(negdata4);
   numBytes = negdata4.size();
   success = toDynamicObject(input4, numBytes, *treDO.get(), errorMessage);
   if (success)
   {
      std::stringstream tmpStream;
      status = this->isTreValid(*treDO.get(), tmpStream);  // This test should return SUSPECT
      if (status != SUSPECT)
      {
         failure << "---Negative test failed: should have returned SUSPECT\n";
         failure << tmpStream.str();
         treDO->clear();
         return false;
      }
   }

   // Start of test 5 - false parser test
   stringstream input5(data_error5);
   numBytes = input5.str().size();
   success = toDynamicObject(input5, numBytes, *treDO.get(), errorMessage);
   if (success == true)
   {
      failure << "Error: Negative test: alpha in numeric SENSERIAL field failed: did not return false\n";
      return false;
   }

   treDO->clear();

   // Start of test 6 - blanks in optional fields
   errorMessage.clear();
   stringstream input6(data6);
   success = toDynamicObject(input6, input6.str().size(), *treDO.get(), errorMessage);
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

      if (input6.str() != tmpStream.str())
      {
         failure << "Error: Test with blank data failed: fromDynamicObject returned an unexpected value\n";
         return false;
      }
   }

   treDO->clear();
   return true;
}

Nitf::TreState Nitf::AcftaParser::isTreValid(const DynamicObject& tre, ostream& reporter) const
{
   TreState status(VALID);
   set<string> testSet;
   unsigned int numFields = 0;

   status = testTagValidBcsASet(tre, reporter, &numFields, ACFTA::AC_MSN_ID, testSet, false, true, false);

   testSet.clear();
   testSet.insert("C");
   testSet.insert("R");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, ACFTA::SCTYPE, testSet, true, false, true));
   status = MaxState(status, testTagValueRange<int>(tre, reporter, &numFields, ACFTA::SCNUM, 0, 9999));

   testSet.clear();
   testSet.insert("ASR");
   testSet.insert("APG");
   testSet.insert("DST");
   status = MaxState(status, testTagValidBcsASet(tre, reporter, &numFields,
      ACFTA::SENSOR_ID, testSet, false, true, true));
   status = MaxState(status, testTagValueRange<int>(tre, reporter, &numFields, ACFTA::PATCH_TOT, 0, 999));
   status = MaxState(status, testTagValueRange<int>(tre, reporter, &numFields, ACFTA::MTI_TOT, 0, 999));

   try
   {
      const DateTime* pDate = dv_cast<DateTime>(&tre.getAttribute(ACFTA::PDATE));
      if (pDate == NULL)
      {
         status = INVALID;
      }

      ++numFields;
      if (pDate->isValid() == false)
      {
         reporter << "Field \"" << ACFTA::PDATE << "\" is invalid.";
         status = INVALID;
      }
   }
   catch (const bad_cast&)
   {
      reporter << "Field \"" << ACFTA::PDATE << "\" missing from the Dynamic Object.";
      status = INVALID;
   }

   status = MaxState(status, testTagValueRange<int>(tre, reporter, &numFields, ACFTA::IMHOSTNO, 0, 999));
   status = MaxState(status, testTagValueRange<int>(tre, reporter, &numFields, ACFTA::IMREQID, 0, 99999));
   status = MaxState(status, testTagValueRange<int>(tre, reporter, &numFields, ACFTA::SCENE_SOURCE, 0, 9));
   status = MaxState(status, testTagValueRange<int>(tre, reporter, &numFields, ACFTA::MPLAN, 0, 99));

   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter, &numFields, ACFTA::ENTLOC, testSet));
   status = MaxState(status, testTagValueRange<int>(tre, reporter, &numFields, ACFTA::ENTELV, -1000, 30000));

   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter, &numFields, ACFTA::EXITLOC, testSet, true));
   status = MaxState(status, testTagValueRange<int>(tre, reporter, &numFields, ACFTA::EXITELV, -1000, 30000, true));

   // Valid values are 0.0 to 180.0 and 999.999 so need two tests.
   // Need to use tmpstatus because if status == SUSPECT then 2nd test will be called even if 1st test == VALID
   TreState tmpstatus = testTagValueRange<double>(tre, reporter, NULL, ACFTA::TMAP, 0.0, 180.0);
   if (tmpstatus != VALID)
   {
      tmpstatus = MaxState(tmpstatus, testTagValueRange<double>(tre, reporter, NULL, ACFTA::TMAP, 999.999, 999.999));
   }

   if (tmpstatus != INVALID)
   {
      ++numFields;
   }
   status = MaxState(status, tmpstatus);

   status = MaxState(status, testTagValueRange<int>(tre, reporter, &numFields, ACFTA::RCS, 40, 80, true));
   status = MaxState(status, testTagValueRange<double>(tre, reporter, &numFields, ACFTA::ROW_SPACING, 0.0, 9999.99));
   status = MaxState(status, testTagValueRange<double>(tre, reporter, &numFields, ACFTA::COL_SPACING, 0.0, 9999.99));
   status = MaxState(status, testTagValueRange<int>(tre, reporter, &numFields, ACFTA::SENSERIAL, 1, 9999, true));

   // The ABSWVER is the Version/Revision number in the form "VVVV.RR" so test it as a string not a double
   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter, &numFields, ACFTA::ABSWVER, testSet, true));

   unsigned int totalFields = tre.getNumAttributes();
   if (status != INVALID && totalFields != numFields)
   {
      reporter << "Total fields in the Dynamic Object(" << totalFields <<
         ") did not match the number found(" << numFields << ") ";
      status = INVALID;
   }

   if (status == INVALID)
   {
      reporter << " INVALID fields found in the ACFTA TAG/SDE\n";
   }
   else if (status == SUSPECT)
   {
      reporter << " SUSPECT fields found in the ACFTA TAG/SDE\n";
   }

   return status;
}


bool Nitf::AcftaParser::toDynamicObject(istream& input, size_t numBytes, DynamicObject& output,
   string &errorMessage) const
{
   vector<char> buf;
   bool success(true);

   readField<string>(input, output, success, ACFTA::AC_MSN_ID, 10, errorMessage, buf);
   readField<string>(input, output, success, ACFTA::SCTYPE, 1, errorMessage, buf, true);
   readField<int>(input, output, success, ACFTA::SCNUM, 4, errorMessage, buf);
   readField<string>(input, output, success, ACFTA::SENSOR_ID, 3, errorMessage, buf);
   readField<int>(input, output, success, ACFTA::PATCH_TOT, 4, errorMessage, buf);
   readField<int>(input, output, success, ACFTA::MTI_TOT, 3, errorMessage, buf);

   if (success)
   {
      unsigned short yy(0);
      unsigned short mm(0);
      unsigned short dd(0);
      success = readFromStream(input, buf, 7) && DtgParseDDMMMYY(string(&buf.front()), yy, mm, dd);
      if (success)
      {
         FactoryResource<DateTime> appDTG;
         success = appDTG->set(yy, mm, dd);
         if (success)
         {
            success = success && output.setAttribute(ACFTA::PDATE, *appDTG.get());
            if (!success)
            {
               errorMessage += "setAttribute " + ACFTA::PDATE + " failed\n";
            }
         }
         else
         {
            errorMessage += "DateTime Set " + ACFTA::PDATE + " failed\n";
         }
      }
      else
      {
         errorMessage += "Parsing " + ACFTA::PDATE + " failed\n";
      }
   }

   readField<int>(input, output, success, ACFTA::IMHOSTNO, 3, errorMessage, buf);
   readField<int>(input, output, success, ACFTA::IMREQID, 5, errorMessage, buf);
   readField<int>(input, output, success, ACFTA::SCENE_SOURCE, 1, errorMessage, buf);
   readField<int>(input, output, success, ACFTA::MPLAN, 2, errorMessage, buf);
   readField<string>(input, output, success, ACFTA::ENTLOC, 21, errorMessage, buf);
   readField<int>(input, output, success, ACFTA::ENTELV, 6, errorMessage, buf);
   readField<string>(input, output, success, ACFTA::EXITLOC, 21, errorMessage, buf, true);
   readField<int>(input, output, success, ACFTA::EXITELV, 6, errorMessage, buf, true);
   readField<double>(input, output, success, ACFTA::TMAP, 7, errorMessage, buf);
   readField<int>(input, output, success, ACFTA::RCS, 3, errorMessage, buf, true);
   readField<double>(input, output, success, ACFTA::ROW_SPACING, 7, errorMessage, buf);
   readField<double>(input, output, success, ACFTA::COL_SPACING, 7, errorMessage, buf);
   readField<int>(input, output, success, ACFTA::SENSERIAL, 4, errorMessage, buf, true);

   // The ABSWVER is the Version/Revision number in the form "VVVV.RR" so make it a string not a double
   readField<string>(input, output, success, ACFTA::ABSWVER, 7, errorMessage, buf, true);

   int64_t numRead = input.tellg();
   if (numRead < 0 || static_cast<uint64_t>(numRead) > std::numeric_limits<size_t>::max() ||
      numRead != static_cast<int64_t>(numBytes))
   {
      numReadErrMsg(numRead, numBytes, errorMessage);
      return false;
   }

   return success;
}

bool Nitf::AcftaParser::fromDynamicObject(const DynamicObject& input, ostream& output, size_t& numBytesWritten,
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
      output << sizeString(dv_cast<string>(input.getAttribute(ACFTA::AC_MSN_ID)), 10);
      output << sizeString(dv_cast<string>(input.getAttribute(ACFTA::SCTYPE)), 1);
      output << toString(dv_cast<int>(input.getAttribute(ACFTA::SCNUM)), 4);
      output << sizeString(dv_cast<string>(input.getAttribute(ACFTA::SENSOR_ID)), 3);
      output << toString(dv_cast<int>(input.getAttribute(ACFTA::PATCH_TOT)), 4);
      output << toString(dv_cast<int>(input.getAttribute(ACFTA::MTI_TOT)), 3);

      const DateTime* pAppDtg = dv_cast<DateTime>(&input.getAttribute(ACFTA::PDATE));
      if (pAppDtg == NULL)
      {
         return false;
      }

      // put date in form DDMMMYY for this TAG    see: strftime() for format info
      string ddmmmyy = pAppDtg->getFormattedUtc("%d%b%y");
      boost::to_upper(ddmmmyy);
      output << sizeString(ddmmmyy, 7);

      output << toString(dv_cast<int>(input.getAttribute(ACFTA::IMHOSTNO)), 3);
      output << toString(dv_cast<int>(input.getAttribute(ACFTA::IMREQID)), 5);
      output << toString(dv_cast<int>(input.getAttribute(ACFTA::SCENE_SOURCE)), 1);
      output << toString(dv_cast<int>(input.getAttribute(ACFTA::MPLAN)), 2);
      output << sizeString(dv_cast<string>(input.getAttribute(ACFTA::ENTLOC)), 21);
      output << toString(dv_cast<int>(input.getAttribute(ACFTA::ENTELV)), 6, -1, ZERO_FILL, POS_SIGN_TRUE);
      output << sizeString(dv_cast<string>(input.getAttribute(ACFTA::EXITLOC)), 21);
      output << toString(dv_cast<int>(input.getAttribute(ACFTA::EXITELV)),
         6, -1, ZERO_FILL, POS_SIGN_TRUE, false, 3, true);
      output << toString(dv_cast<double>(input.getAttribute(ACFTA::TMAP)), 7, 3);
      output << toString(dv_cast<int>(input.getAttribute(ACFTA::RCS)), 3, -1, ZERO_FILL, false, false, 3, true);
      output << toString(dv_cast<double>(input.getAttribute(ACFTA::ROW_SPACING)), 7, 4);
      output << toString(dv_cast<double>(input.getAttribute(ACFTA::COL_SPACING)), 7, 4);
      output << toString(dv_cast<int>(input.getAttribute(ACFTA::SENSERIAL)), 4, -1, ZERO_FILL, false, false, 3, true);
      output << sizeString(dv_cast<string>(input.getAttribute(ACFTA::ABSWVER)), 7);
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
