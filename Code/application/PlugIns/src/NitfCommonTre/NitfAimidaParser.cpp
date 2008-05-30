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
#include "NitfAimidaParser.h"
#include "NitfConstants.h"
#include "NitfUtilities.h"
#include "ObjectResource.h"

#include <set>
#include <sstream>

#include <boost/algorithm/string/case_conv.hpp>

using namespace std;
using namespace Nitf;
using namespace Nitf::TRE;

Nitf::AimidaParser::AimidaParser()
{
   setName("AIMIDA");
   setDescriptorId("{54D7D8C8-64F2-4c37-903B-EA2D7062A3AD}");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

bool Nitf::AimidaParser::runAllTests(Progress* pProgress, ostream& failure)
{
   static const string data(
      "11SEP01"               // MISSION_DATE
      "A001"                  // MISSION_NO
      "A1"                    // FLIGHT_NO
      "123"                   // OP_NUM
      "AZ"                    // START_SEGMENT
      "01"                    // REPRO_NUM
      "G01"                   // REPLAY
      " "                     // RESERVED1
      "12"                    // START_COLUMN
      "12345"                 // START_ROW
      "AZ"                    // END_SEGMENT
      "12"                    // END_COLUMN
      "12345"                 // END_ROW
      "US"                    // COUNTRY
      "    "                  // RESERVED2
      "8800X10000Y"           // LOCATION
      "1234Z"                 // TIME
      "31AUG06"               // CREATION_DATE
      );

   static const string data_error4(
      "11SEP01"               // MISSION_DATE
      "A001"                  // MISSION_NO
      "A1"                    // FLIGHT_NO
      "123"                   // OP_NUM
      "AZ"                    // START_SEGMENT
      "01"                    // REPRO_NUM
      "G01"                   // REPLAY
      " "                     // RESERVED1
      "00"                    // START_COLUMN      // ERROR: data out of range
      "12345"                 // START_ROW
      "AZ"                    // END_SEGMENT
      "12"                    // END_COLUMN
      "12345"                 // END_ROW
      "US"                    // COUNTRY
      "    "                  // RESERVED2
      "8800X10000Y"           // LOCATION
      "1234Z"                 // TIME
      "31AUG06"               // CREATION_DATE
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
   stringstream input2;
   input2.write(data.c_str(), data.size());
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
   if(success)
   {
      std::stringstream tmpStream;
      status = this->isTreValid(*treDO.get(), tmpStream);  // This test should return SUSPECT
      if(status != SUSPECT)
      {
         failure << "---Negative test failed: START_COLUMN should have returned SUSPECT\n";
         failure << tmpStream.str();
         treDO->clear();
         return false;
      }
   }

   treDO->clear();



   return (status != INVALID);
}

Nitf::TreState Nitf::AimidaParser::isTreValid(const DynamicObject& tre, ostream& reporter) const
{
   TreState status(VALID);
   set<string>  testSet;
   unsigned int numFields = 0;

   if (tre.getAttribute(AIMIDA::MISSION_DATE).getPointerToValue<DateTime>() == NULL)
   {
      reporter << "Field \"" << AIMIDA::MISSION_DATE << "\" missing from the Dynamic Object";
      status = INVALID;
   }
   else
   {
      ++numFields;
   }

   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, AIMIDA::MISSION_NO, testSet, false, true, false));

   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, AIMIDA::FLIGHT_NO, testSet, false, true, false));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, AIMIDA::OP_NUM, 0, 999));

   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, AIMIDA::START_SEGMENT, testSet, true, true, false));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, AIMIDA::REPRO_NUM, 0, 99));

   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, AIMIDA::REPLAY, testSet, false, true, false));

#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : This is a one char blank field in STDI-0002. For Classified versions this may be something else. (lbeck)")
   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, AIMIDA::RESERVED1, testSet, true, true, true));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, AIMIDA::START_COLUMN, 1, 99));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, AIMIDA::START_ROW, 1, 99999));

   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, AIMIDA::END_SEGMENT, testSet, true, true, false));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, AIMIDA::END_COLUMN, 1, 99));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, AIMIDA::END_ROW, 1, 99999));

   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, AIMIDA::COUNTRY, testSet, true, true, false));

#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : This is a four char blank field in STDI-0002. For Classified versions this may be something else. (lbeck)")
   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, AIMIDA::RESERVED2, testSet, true, true, true));

   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, AIMIDA::LOCATION, testSet, false, true, false));

   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, AIMIDA::TIME, testSet, false, true, false));

   FactoryResource<DateTime> appDTG;
   if (tre.getAttribute(AIMIDA::CREATION_DATE).getPointerToValue<DateTime>() == NULL)
   {
      reporter << "Field \"" << AIMIDA::CREATION_DATE << "\" missing from the Dynamic Object";
      status = INVALID;
   }
   else
   {
      ++numFields;
   }

   unsigned int totalFields = tre.getNumAttributes();
   if (status != INVALID && totalFields != numFields)
   {
      reporter << "Total fields in the Dynamic Object(" <<
         totalFields <<") did not match the number found(" << numFields << ") ";
      status = INVALID;
   }

   if (status == INVALID)
   {
      reporter << " INVALID fields found in the AIMIDA TAG/SDE\n" ;
   }
   else if (status == SUSPECT)
   {
      reporter << " SUSPECT fields found in the AIMIDA TAG/SDE\n" ;
   }

   return status;
}


bool Nitf::AimidaParser::toDynamicObject(istream& input, size_t numBytes, DynamicObject& output,
   string &errorMessage) const
{
   vector<char> buf;
   bool ok(true);

   unsigned short yy(0), mm(0), dd(0);
   bool success(true);

   success = readFromStream(input, buf, 7) && DtgParseDDMMMYY(string(&buf.front()), yy, mm, dd);
   if (success)
   {
      FactoryResource<DateTime> appDTG;
      success = appDTG->set(yy, mm, dd);
      if (success)
      {
         success = success && output.setAttribute(AIMIDA::MISSION_DATE, *appDTG.get());
         if (!success)
         {
            errorMessage += "setAttribute " + AIMIDA::MISSION_DATE + " failed\n";
         }
      }
      else
      {
         errorMessage += "DateTime Set " + AIMIDA::MISSION_DATE + " failed\n";
      }
   }
   else
   {
      errorMessage += "Parsing " + AIMIDA::MISSION_DATE + " failed\n";
   }

   readField<string>(input, output, success, AIMIDA::MISSION_NO, 4, errorMessage, buf);
   readField<string>(input, output, success, AIMIDA::FLIGHT_NO, 2, errorMessage, buf);
   readField<int>(input, output, success, AIMIDA::OP_NUM, 3, errorMessage, buf);
   readField<string>(input, output, success, AIMIDA::START_SEGMENT, 2, errorMessage, buf, true);
   readField<int>(input, output, success, AIMIDA::REPRO_NUM, 2, errorMessage, buf);
   readField<string>(input, output, success, AIMIDA::REPLAY, 3, errorMessage, buf);
   readField<string>(input, output, success, AIMIDA::RESERVED1, 1, errorMessage, buf, true);
   readField<int>(input, output, success, AIMIDA::START_COLUMN, 2, errorMessage, buf);
   readField<int>(input, output, success, AIMIDA::START_ROW, 5, errorMessage, buf);
   readField<string>(input, output, success, AIMIDA::END_SEGMENT, 2, errorMessage, buf, true);
   readField<int>(input, output, success, AIMIDA::END_COLUMN, 2, errorMessage, buf);
   readField<int>(input, output, success, AIMIDA::END_ROW, 5, errorMessage, buf);
   readField<string>(input, output, success, AIMIDA::COUNTRY, 2, errorMessage, buf, true);
   readField<string>(input, output, success, AIMIDA::RESERVED2, 4, errorMessage, buf, true);
   readField<string>(input, output, success, AIMIDA::LOCATION, 11, errorMessage, buf);
   readField<string>(input, output, success, AIMIDA::TIME, 5, errorMessage, buf);

   if (success)
   {
      yy = mm = dd = 0;
      success = readFromStream(input, buf, 7) && DtgParseDDMMMYY(string(&buf.front()), yy, mm, dd);
      if (!success)
      {
         errorMessage += "Parsing " + AIMIDA::CREATION_DATE + " failed\n";
      }
      FactoryResource<DateTime> appDTG;
      success = appDTG->set(yy, mm, dd);
      if (success)
      {
         success = success && output.setAttribute(AIMIDA::CREATION_DATE, *appDTG.get());
         if (!success)
         {
            errorMessage += "setAttribute " + AIMIDA::CREATION_DATE + " failed\n";
         }
      }
      else
      {
         errorMessage += "DateTime Set " + AIMIDA::CREATION_DATE + " failed\n";
      }
   }

   size_t numRead = input.tellg();
   if (numRead != numBytes)
   {
      numReadErrMsg(numRead, numBytes, errorMessage);
      return false;
   }

   return success;
}



bool Nitf::AimidaParser::fromDynamicObject(const DynamicObject& input, ostream& output, size_t& numBytesWritten,
   string &errorMessage) const
{
   size_t sizeIn = max(static_cast<ostream::pos_type>(0), output.tellp());
   size_t sizeOut(sizeIn);

   try
   {
      // put date in form DDMMMYY for this TAG                  see: strftime() for format info
      const DateTime *pAppDtgMissionDate = dv_cast<DateTime>(&input.getAttribute(AIMIDA::MISSION_DATE));
      if (pAppDtgMissionDate == NULL)
      {
         return false;
      }

      string ddmmmyy = pAppDtgMissionDate->getFormattedUtc("%d%b%y");
      boost::to_upper(ddmmmyy);
      output << sizeString(ddmmmyy, 7);

      output << sizeString( dv_cast<string>(input.getAttribute(AIMIDA::MISSION_NO)), 4);
      output << sizeString( dv_cast<string>(input.getAttribute(AIMIDA::FLIGHT_NO)), 2);
      output <<   toString( dv_cast<int>(input.getAttribute   (AIMIDA::OP_NUM)), 3);
      output << sizeString( dv_cast<string>(input.getAttribute(AIMIDA::START_SEGMENT)), 2);
      output <<   toString( dv_cast<int>(input.getAttribute   (AIMIDA::REPRO_NUM)), 2);
      output << sizeString( dv_cast<string>(input.getAttribute(AIMIDA::REPLAY)), 3);
      output << sizeString( dv_cast<string>(input.getAttribute(AIMIDA::RESERVED1)), 1);
      output <<   toString( dv_cast<int>(input.getAttribute   (AIMIDA::START_COLUMN)), 2);
      output <<   toString( dv_cast<int>(input.getAttribute   (AIMIDA::START_ROW)), 5);
      output << sizeString( dv_cast<string>(input.getAttribute(AIMIDA::END_SEGMENT)), 2);
      output <<   toString( dv_cast<int>(input.getAttribute   (AIMIDA::END_COLUMN)), 2);
      output <<   toString( dv_cast<int>(input.getAttribute   (AIMIDA::END_ROW)), 5);
      output << sizeString( dv_cast<string>(input.getAttribute(AIMIDA::COUNTRY)), 2);
      output << sizeString( dv_cast<string>(input.getAttribute(AIMIDA::RESERVED2)), 4);
      output << sizeString( dv_cast<string>(input.getAttribute(AIMIDA::LOCATION)), 11);
      output << sizeString( dv_cast<string>(input.getAttribute(AIMIDA::TIME)), 5);

      // put date in form DDMMMYY for this TAG
      const DateTime *pAppDtg = dv_cast<DateTime>(&input.getAttribute(AIMIDA::CREATION_DATE));
      if (pAppDtg == NULL)
      {
         return false;
      }

      ddmmmyy = pAppDtg->getFormattedUtc("%d%b%y");
      boost::to_upper(ddmmmyy);
      output << sizeString(ddmmmyy, 7);
   }
   catch (const bad_cast&)
   {
      return false;
   }

   sizeOut = output.tellp();
   numBytesWritten = sizeOut - sizeIn;
   return true;
}
