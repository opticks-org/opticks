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
#include "DateTime.h"
#include "NitfAcftbParser.h"
#include "NitfConstants.h"
#include "NitfUtilities.h"
#include "ObjectResource.h"
#include "PlugInRegistration.h"
#include "StringUtilities.h"

#include <set>
#include <sstream>
#include <string>

#include <boost/algorithm/string/case_conv.hpp>

using namespace std;
using namespace Nitf;
using namespace Nitf::TRE;

REGISTER_PLUGIN(OpticksNitfCommonTre, AcftbParser, Nitf::AcftbParser());

Nitf::AcftbParser::AcftbParser()
{
   setName("ACFTB");
   setDescriptorId("{E033DB4E-1D6B-4a9e-8D8D-4F213D1BDAD7}");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}


bool Nitf::AcftbParser::runAllTests(Progress* pProgress, ostream& failure)
{
   static const string data(
      "AC_MSN_IDX1234567890"        // AC_MSN_ID]
      "AC_TAIL_NO"                  // AC_TAIL_NO
      "200607301305"                // AC_TO
      "IHFR"                        // SENSOR_ID_TYPE
      "AIP   "                      // SENSOR_ID
      "1"                           // SCENE_SOURCE
      "123456"                      // SCNUM
      "20060730"                    // PDATE
      "123456"                      // IMHOSTNO
      "12345"                       // IMREQID
      "123"                         // MPLAN
      "-88.00000000-100.00000000"   // ENTLOC
      "000.01"                      // LOC_ACCY
      "+01000"                      // ENTELV
      "m"                           // ELV_UNIT
      "-88.00000000-100.00000000"   // EXITTLOC
      "-01000"                      // EXITELV
      "090.000"                     // TMAP
      "01.0000"                     // ROW_SPACING
      "m"                           // ROW_SPACING_UNITS
      "01.0000"                     // COL_SPACING
      "m"                           // COL_SPACING_UNITS
      "000.01"                      //FOCAL_LENGTH
      "123456"                      // SENSERIAL
      "1234.12"                     // ABSWVER
      "20010911"                    // CAL_DATE
      "1234"                        // PATCH_TOT
      "123"                         // MTI_TOT
      );


   static const string data_error4(
      "AC_MSN_IDX1234567890"        // AC_MSN_ID]
      "AC_TAIL_NO"                  // AC_TAIL_NO
      "200607301305"                // AC_TO
      "IHFR"                        // SENSOR_ID_TYPE
      "AIP   "                      // SENSOR_ID
      "1"                           // SCENE_SOURCE
      "123456"                      // SCNUM
      "20060730"                    // PDATE
      "123456"                      // IMHOSTNO
      "12345"                       // IMREQID
      "123"                         // MPLAN
      "-88.00000000-100.00000000"   // ENTLOC
      "000.01"                      // LOC_ACCY
      "+01000"                      // ENTELV
      "m"                           // ELV_UNIT
      "-88.00000000-100.00000000"   // EXITTLOC
      "-01000"                      // EXITELV
      "090.000"                     // TMAP
      "01.0000"                     // ROW_SPACING
      "m"                           // ROW_SPACING_UNITS
      "01.0000"                     // COL_SPACING
      "m"                           // COL_SPACING_UNITS
      "000.01"                      //FOCAL_LENGTH
      "123456"                      // SENSERIAL
      "1234.12"                     // ABSWVER
      "20010911"                    // CAL_DATE
      "1234"                        // PATCH_TOT
      "12g"                         // MTI_TOT
      );

   static const string data_error5(
      "AC_MSN_IDX1234567890"        // AC_MSN_ID]
      "AC_TAIL_NO"                  // AC_TAIL_NO
      "200607301305"                // AC_TO
      "IHFR"                        // SENSOR_ID_TYPE
      "AIP   "                      // SENSOR_ID
      "1"                           // SCENE_SOURCE
      "123456"                      // SCNUM
      "20060730"                    // PDATE
      "123456"                      // IMHOSTNO
      "12345"                       // IMREQID
      "123"                         // MPLAN
      "-88.00000000-100.00000000"   // ENTLOC
      "000.01"                      // LOC_ACCY
      "+30001"                      // ENTELV      // ERROR == -1000 <= ENTELV <= 30000
      "m"                           // ELV_UNIT
      "-88.00000000-100.00000000"   // EXITTLOC
      "-01000"                      // EXITELV
      "090.000"                     // TMAP
      "01.0000"                     // ROW_SPACING
      "m"                           // ROW_SPACING_UNITS
      "01.0000"                     // COL_SPACING
      "m"                           // COL_SPACING_UNITS
      "000.01"                      //FOCAL_LENGTH
      "123456"                      // SENSERIAL
      "1234.12"                     // ABSWVER
      "20010911"                    // CAL_DATE
      "1234"                        // PATCH_TOT
      "123"                         // MTI_TOT
      );

   static const string data6(
      "AC_MSN_IDX1234567890"        // AC_MSN_ID]
      "AC_TAIL_NO"                  // AC_TAIL_NO
      "200607301305"                // AC_TO
      "IHFR"                        // SENSOR_ID_TYPE
      "AIP   "                      // SENSOR_ID
      " "                           // SCENE_SOURCE - set to spaces
      "123456"                      // SCNUM
      "20060730"                    // PDATE
      "123456"                      // IMHOSTNO
      "12345"                       // IMREQID
      "123"                         // MPLAN
      "-88.00000000-100.00000000"   // ENTLOC
      "000.01"                      // LOC_ACCY
      "      "                      // ENTELV - set to spaces
      "m"                           // ELV_UNIT
      "-88.00000000-100.00000000"   // EXITTLOC
      "      "                      // EXITELV - set to spaces
      "       "                     // TMAP - set to spaces
      "01.0000"                     // ROW_SPACING
      "m"                           // ROW_SPACING_UNITS
      "01.0000"                     // COL_SPACING
      "m"                           // COL_SPACING_UNITS
      "000.01"                      //FOCAL_LENGTH
      "      "                      // SENSERIAL - set to spaces
      "1234.12"                     // ABSWVER
      "20010911"                    // CAL_DATE
      "1234"                        // PATCH_TOT
      "123"                         // MTI_TOT
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

   // Start of test 4 - false parser test
   stringstream input4(data_error4);
   numBytes = input4.str().size();

   success = toDynamicObject(input4, numBytes, *treDO.get(), errorMessage);

   if (success)
   {
      failure << "Error: Negative test: alpha in numeric MTI_TOT field failed: did not return false\n";
      return false;
   }

   treDO->clear();

   // Start of test 5 - SUSPECT test
   stringstream input5(data_error5);
   numBytes = input5.str().size();
   success = toDynamicObject(input5, numBytes, *treDO.get(), errorMessage);
   if (success)
   {
      std::stringstream tmpStream;
      status = this->isTreValid(*treDO.get(), tmpStream);  // This test should return SUSPECT
      if (status != SUSPECT)
      {
         failure << "---Negative test failed: ENTELV should have returned SUSPECT\n";
         failure << tmpStream.str();
         treDO->clear();
         return false;
      }
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


Nitf::TreState Nitf::AcftbParser::isTreValid(const DynamicObject& tre, ostream& reporter) const
{
   TreState status(VALID);
   set<string> testSet;
   unsigned int numFields = 0;

   testSet.clear();
   testSet.insert("NOT AVAILABLE");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, ACFTB::AC_MSN_ID, testSet, false, true, false));

   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, ACFTB::AC_TAIL_NO, testSet, true, true, false));

   // This field does not have to exist but if it does it needs to be correct
   // Assume it is good DateTime if it was stored as an actual DataTime.
   const DataVariant& ac_toValue = tre.getAttribute(ACFTB::AC_TO);
   string value = ac_toValue.toXmlString();
   string actualType = ac_toValue.getTypeName();
   if (value != "")                     // If not a null string then it exists
   {
      if (actualType != "DateTime")
      {
         reporter << "Error: DateTime field \"AC_TO\" found stored as " << actualType;
         status = INVALID;
      }
      ++numFields;
   }

   try
   {
      value = dv_cast<string>(tre.getAttribute(ACFTB::SENSOR_ID_TYPE));

      // ccff where cc indicates the sensor category, And ff indicates the sensor format:
      string cc(value.substr(0, 2));
      string ff(value.substr(2, 2));
      if (value.size() == 4 &&
         (cc == "IH" || cc == "IM" || cc == "IL" || cc == "MH" || cc == "MM" ||
             cc == "ML" || cc == "VH" || cc == "VM" || cc == "VL" || cc == "VF") &&
         (ff == "FR" || ff == "LS" || ff == "PB" || ff == "PS"))
      {
         // special case test: 1 of cc && 1 of ff == success
      }
      else
      {
         testSet.clear();
         testSet.insert("SAR");
         status = MaxState(status, testTagValidBcsASet(tre, reporter,
            &numFields, ACFTB::SENSOR_ID_TYPE, testSet, false, false, true));
      }

      ++numFields;
   }
   catch (const bad_cast&)
   {
      reporter << "Field \"" << ACFTB::SENSOR_ID_TYPE << "\" missing from the Dynamic Object";
      status = INVALID;
   }

   testSet.clear();
   testSet.insert("APG-73");
   testSet.insert("AIP");
   testSet.insert("ASARS1");
   testSet.insert("ASARS2");
   testSet.insert("CA236");
   testSet.insert("CA260");
   testSet.insert("CA261");
   testSet.insert("CA265");
   testSet.insert("CA270");
   testSet.insert("CA295");
   testSet.insert("D500");
   testSet.insert("DB110");
   testSet.insert("DS-SAR");
   testSet.insert("GHR");
   testSet.insert("HYDICE");
   testSet.insert("HSAR");
   testSet.insert("IRLS");
   testSet.insert("LAEO");
   testSet.insert("MAEO");
   testSet.insert("SIR-C");
   testSet.insert("SYERS");
   testSet.insert("TSAR");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, ACFTB::SENSOR_ID, testSet, false, true, true));

   status = MaxState(status, testTagValueRange<int>(tre, reporter, &numFields, ACFTB::SCENE_SOURCE, 0, 9, true));
   status = MaxState(status, testTagValueRange<int>(tre, reporter, &numFields, ACFTB::SCNUM, 0, 999999));

   // Just check that the field exists. Assume the DateTime class will contain a vaild date/time
   if (tre.getAttribute(ACFTB::PDATE).getPointerToValue<DateTime>() == NULL)
   {
      reporter << "Field \"" << ACFTB::PDATE << "\" missing from the Dynamic Object";
      status = INVALID;
   }
   else
   {
      ++numFields;
   }

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, ACFTB::IMHOSTNO, 0, 999999));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, ACFTB::IMREQID, 0, 99999));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, ACFTB::MPLAN, 1, 999));

   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, ACFTB::ENTLOC, testSet, true, true, false));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, ACFTB::LOC_ACCY, 0.0, 999.99));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, ACFTB::ENTELV, -1000, 30000, true));

   testSet.clear();
   testSet.insert("f");
   testSet.insert("m");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, ACFTB::ELV_UNIT, testSet, true, false, true));

   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, ACFTB::EXITLOC, testSet, true, true, false));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, ACFTB::EXITELV, -1000, 30000, true));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, ACFTB::TMAP, 0.0, 180.00, true));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "ROW_SPACING", 0.0, 9999.99));

   testSet.clear();
   testSet.insert("f");
   testSet.insert("m");
   testSet.insert("r");
   testSet.insert("u");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, ACFTB::ROW_SPACING_UNITS, testSet, false, false, true));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, ACFTB::COL_SPACING, 0.0, 9999.99));

   testSet.clear();
   testSet.insert("f");
   testSet.insert("m");
   testSet.insert("r");
   testSet.insert("u");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, ACFTB::COL_SPACING_UNITS, testSet, true, false, true));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, ACFTB::FOCAL_LENGTH, 0.01, 999.99));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, ACFTB::SENSERIAL, 0, 999999, true));

   // The ABSWVER is the Version/Revision number in the form "VVVV.RR" so make it a string not a double
   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, ACFTB::ABSWVER, testSet, true, true, false));

   // This field does not have to exist but if it does it needs to be correct
   // Assume it is good DateTime if it was stored as an actual DataTime.
   const DataVariant& cal_DateValue = tre.getAttribute(ACFTB::CAL_DATE);
   value = cal_DateValue.toXmlString();
   actualType = cal_DateValue.getTypeName();
   if (value != "")                     // If not a null string then it exists
   {
      if (actualType != "DateTime")
      {
         reporter << "Error: DateTime field \"CAL_DATE\" found stored as " << actualType;
         status = INVALID;
      }
      ++numFields;
   }

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, ACFTB::PATCH_TOT, 0, 9999));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, ACFTB::MTI_TOT, 0, 999));

   unsigned int totalFields = tre.getNumAttributes();
   if (status != INVALID && totalFields != numFields)
   {
      reporter << "Total fields in the Dynamic Object(" <<
         totalFields << ") did not match the number found(" << numFields << ") ";
      status = INVALID;
   }

   if (status == INVALID)
   {
      reporter << " INVALID fields found in the ACFTB TAG/SDE\n" ;
   }
   else if (status == SUSPECT)
   {
      reporter << " SUSPECT fields found in the ACFTB TAG/SDE\n" ;
   }

   return status;
}

bool Nitf::AcftbParser::toDynamicObject(istream& input, size_t numBytes, DynamicObject& output,
   string &errorMessage) const
{
   vector<char> buf;
   bool ok(true);
   bool success(true);

   readField<string>(input, output, success, ACFTB::AC_MSN_ID, 20, errorMessage, buf);
   readField<string>(input, output, success, ACFTB::AC_TAIL_NO, 10, errorMessage, buf, true);

   if (success)
   {
      unsigned short yy(0);
      unsigned short mm(0);
      unsigned short dd(0);
      unsigned short hh(0);
      unsigned short min(0);
      unsigned short ss(0);
      success = readFromStream(input, buf, 12) && DtgParseCCYYMMDDhhmm(string(&buf.front()), yy, mm, dd, hh, min);
      if (success)
      {
         FactoryResource<DateTime> appDTG;
         success = appDTG->set(yy, mm, dd, hh, min, ss);
         if (success)
         {
            success = success && output.setAttribute(ACFTB::AC_TO, *appDTG.get());
            if (!success)
            {
               errorMessage += "setAttribute " + ACFTB::AC_TO + " failed\n";
            }
         }
      }
      else if (StringUtilities::isAllBlank(string(&buf.front())))
      {
         success = true;
      }
      else
      {
         errorMessage += "Parsing " + ACFTB::AC_TO + " failed\n";
      }
   }

   readField<string>(input, output, success, ACFTB::SENSOR_ID_TYPE, 4, errorMessage, buf);
   readField<string>(input, output, success, ACFTB::SENSOR_ID, 6, errorMessage, buf);
   readField<int>(input, output, success, ACFTB::SCENE_SOURCE, 1, errorMessage, buf, true);
   readField<int>(input, output, success, ACFTB::SCNUM, 6, errorMessage, buf);

   if (success)
   {
      unsigned short yy(0);
      unsigned short mm(0);
      unsigned short dd(0);
      success = readFromStream(input, buf, 8) && DtgParseCCYYMMDD(string(&buf.front()), yy, mm, dd);
      if (success)
      {
         FactoryResource<DateTime> appDTG;
         success = appDTG->set(yy, mm, dd);
         if (success)
         {
            success = success && output.setAttribute(ACFTB::PDATE, *appDTG.get());
            if (!success)
            {
               errorMessage += "setAttribute " + ACFTB::PDATE + " failed\n";
            }
         }
         else
         {
            errorMessage += "DateTime set " + ACFTB::PDATE + " failed\n";
         }
      }
      else
      {
         errorMessage += "Parsing " + ACFTB::PDATE + " failed\n";
      }

   }

   readField<int>(input, output, success, ACFTB::IMHOSTNO, 6, errorMessage, buf);
   readField<int>(input, output, success, ACFTB::IMREQID, 5, errorMessage, buf);
   readField<int>(input, output, success, ACFTB::MPLAN, 3, errorMessage, buf);
   readField<string>(input, output, success, ACFTB::ENTLOC, 25, errorMessage, buf, true);
   readField<double>(input, output, success, ACFTB::LOC_ACCY, 6, errorMessage, buf, true);
   readField<int>(input, output, success, ACFTB::ENTELV, 6, errorMessage, buf, true);
   readField<string>(input, output, success, ACFTB::ELV_UNIT, 1, errorMessage, buf, true);
   readField<string>(input, output, success, ACFTB::EXITLOC, 25, errorMessage, buf, true);
   readField<int>(input, output, success, ACFTB::EXITELV, 6, errorMessage, buf, true);
   readField<double>(input, output, success, ACFTB::TMAP, 7, errorMessage, buf, true);
   readField<double>(input, output, success, ACFTB::ROW_SPACING, 7, errorMessage, buf);
   readField<string>(input, output, success, ACFTB::ROW_SPACING_UNITS, 1, errorMessage, buf);
   readField<double>(input, output, success, ACFTB::COL_SPACING, 7, errorMessage, buf);
   readField<string>(input, output, success, ACFTB::COL_SPACING_UNITS, 1, errorMessage, buf);
   readField<double>(input, output, success, ACFTB::FOCAL_LENGTH, 6, errorMessage, buf);
   readField<int>(input, output, success, ACFTB::SENSERIAL, 6, errorMessage, buf, true);
   readField<string>(input, output, success, ACFTB::ABSWVER, 7, errorMessage, buf, true);

   if (success)
   {
      unsigned short yy(0);
      unsigned short mm(0);
      unsigned short dd(0);
      success = readFromStream(input, buf, 8) && DtgParseCCYYMMDD(string(&buf.front()), yy, mm, dd);
      if (success)
      {
         FactoryResource<DateTime> appDTG;
         success = appDTG->set(yy, mm, dd);
         if (success)
         {
            success = success && output.setAttribute(ACFTB::CAL_DATE, *appDTG.get());
            if (!success)
            {
               errorMessage += "setAttribute " + ACFTB::CAL_DATE + " failed\n";
            }
         }
         else
         {
            errorMessage += "DateTime Set " + ACFTB::CAL_DATE + " failed\n";
         }
      }
      else if (StringUtilities::isAllBlank(string(&buf.front())))
      {
         success = true;
      }
      else
      {
         errorMessage += "Parsing " + ACFTB::CAL_DATE + " failed\n";
      }
   }

   readField<int>(input, output, success, ACFTB::PATCH_TOT, 4, errorMessage, buf);
   readField<int>(input, output, success, ACFTB::MTI_TOT, 3, errorMessage, buf);

   size_t numRead = input.tellg();
   if (numRead != numBytes)
   {
      numReadErrMsg(numRead, numBytes, errorMessage);
      return false;
   }

   return success;
}



bool Nitf::AcftbParser::fromDynamicObject(const DynamicObject& input, ostream& output, size_t& numBytesWritten,
   string &errorMessage) const
{
   size_t sizeIn = max(static_cast<ostream::pos_type>(0), output.tellp());
   size_t sizeOut(sizeIn);

   try
   {
      output << sizeString( dv_cast<string>(input.getAttribute(ACFTB::AC_MSN_ID)), 20);
      output << sizeString( dv_cast<string>(input.getAttribute(ACFTB::AC_TAIL_NO)), 10);

      // put date in form CCYYMMDDhhmm for this TAG    see: strftime() for format info
      const DateTime* pAppDtgAcTo = dv_cast<DateTime>(&input.getAttribute(ACFTB::AC_TO));
      if (pAppDtgAcTo == NULL)
      {
         return false;
      }

      string CCYYMMDDhhmm = pAppDtgAcTo->getFormattedUtc("%Y%m%d%H%M");
      output << sizeString(CCYYMMDDhhmm, 12);

      output << sizeString(dv_cast<string>(input.getAttribute(ACFTB::SENSOR_ID_TYPE)), 4);
      output << sizeString(dv_cast<string>(input.getAttribute(ACFTB::SENSOR_ID)), 6);
      output << toString(dv_cast<int>(input.getAttribute(ACFTB::SCENE_SOURCE)),
         1, -1, ZERO_FILL, false, false, 3, true);
      output << toString(dv_cast<int>(input.getAttribute(ACFTB::SCNUM)), 6);

      // put date in form CCYYMMDD for this TAG
      const DateTime* pAppDtgPDate = dv_cast<DateTime>(&input.getAttribute(ACFTB::PDATE));
      if (pAppDtgPDate == NULL)
      {
         return false;
      }
      string CCYYMMDD = pAppDtgPDate->getFormattedUtc("%Y%m%d");
      output << sizeString(CCYYMMDD, 8);

      output << toString(dv_cast<int>(input.getAttribute(ACFTB::IMHOSTNO)), 6);
      output << toString(dv_cast<int>(input.getAttribute(ACFTB::IMREQID)), 5);
      output << toString(dv_cast<int>(input.getAttribute(ACFTB::MPLAN)), 3);
      output << sizeString(dv_cast<string>(input.getAttribute(ACFTB::ENTLOC)), 25);
      output << toString(dv_cast<double>(input.getAttribute(ACFTB::LOC_ACCY)), 6, 2);
      output << toString(dv_cast<int>(input.getAttribute(ACFTB::ENTELV)),
         6, -1, ZERO_FILL, POS_SIGN_TRUE, false, 3, true);
      output << sizeString(dv_cast<string>(input.getAttribute(ACFTB::ELV_UNIT)), 1);
      output << sizeString(dv_cast<string>(input.getAttribute(ACFTB::EXITLOC)), 25);
      output << toString(dv_cast<int>(input.getAttribute(ACFTB::EXITELV)),
         6, -1, ZERO_FILL, POS_SIGN_TRUE, false, 3, true);
      output << toString(dv_cast<double>(input.getAttribute(ACFTB::TMAP)), 7, 3, ZERO_FILL, false, false, 3, true);
      output << toString(dv_cast<double>(input.getAttribute(ACFTB::ROW_SPACING)), 7, 4);
      output << sizeString(dv_cast<string>(input.getAttribute(ACFTB::ROW_SPACING_UNITS)), 1);
      output << toString(dv_cast<double>(input.getAttribute(ACFTB::COL_SPACING)), 7, 4);
      output << sizeString(dv_cast<string>(input.getAttribute(ACFTB::COL_SPACING_UNITS)), 1);
      output << toString(dv_cast<double>(input.getAttribute(ACFTB::FOCAL_LENGTH)), 6, 2);
      output << toString(dv_cast<int>(input.getAttribute(ACFTB::SENSERIAL)), 6, -1, ZERO_FILL, false, false, 3, true);
      output << sizeString(dv_cast<string>(input.getAttribute(ACFTB::ABSWVER)), 7);

      // put date in form CCYYMMDD for this TAG
      const DateTime* pAppDtgCalDate = dv_cast<DateTime>(&input.getAttribute(ACFTB::CAL_DATE));
      if (pAppDtgCalDate == NULL)
      {
         CCYYMMDD = "        ";
      }
      else
      {
         CCYYMMDD = pAppDtgCalDate->getFormattedUtc("%Y%m%d");
      }
      output << sizeString(CCYYMMDD, 8);

      output << toString(dv_cast<int>(input.getAttribute(ACFTB::PATCH_TOT)), 4);
      output << toString(dv_cast<int>(input.getAttribute(ACFTB::MTI_TOT)), 3);
   }
   catch (const bad_cast&)
   {
      return false;
   }

   sizeOut = output.tellp();
   numBytesWritten = sizeOut - sizeIn;
   return true;
}
