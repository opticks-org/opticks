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
#include "NitfAimidbParser.h"
#include "ObjectResource.h"
#include "NitfUtilities.h"
#include "NitfConstants.h"
#include "StringUtilities.h"

#include <sstream>

using namespace std;
using namespace Nitf;
using namespace Nitf::TRE;

Nitf::AimidbParser::AimidbParser()
{
   setName("AIMIDB");
   setDescriptorId("{E966F26B-8AAE-4de1-820B-9B0E3966C30A}");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

bool Nitf::AimidbParser::runAllTests(Progress* pProgress, ostream& failure)
{
   static const string data(
      "20010911081530"        // ACQUISITION_DATE
      "AA01"                  // MISSION_NO
      "Mission-ID"            // MISSION_IDENTIFICATION
      "A1"                    // FLIGHT_NO
      "123"                   // OP_NUM
      "AZ"                    // START_SEGMENT
      "01"                    // REPRO_NUM
      "G01"                   // REPLAY
      " "                     // RESERVED1
      "099"                   // START_TILE_COLUMN
      "12345"                 // START_TILE_ROW
      "AZ"                    // END_SEGMENT
      "099"                   // END_TILE_COLUMN
      "12345"                 // END_TILE_ROW
      "US"                    // COUNTRY
      "    "                  // RESERVED2
      "8800X10000Y"           // LOCATION
      "             "         // RESERVED3
      );

   static const string data_error4(
      "20010911081530"        // ACQUISITION_DATE
      "AA01"                  // MISSION_NO
      "Mission-ID"            // MISSION_IDENTIFICATION
      "A1"                    // FLIGHT_NO
      "123"                   // OP_NUM
      "AZ"                    // START_SEGMENT
      "01"                    // REPRO_NUM
      "G01"                   // REPLAY
      " "                     // RESERVED1
      "000"                   // START_TILE_COLUMN    // ERROR: Data out of range
      "12345"                 // START_TILE_ROW
      "AZ"                    // END_SEGMENT
      "099"                   // END_TILE_COLUMN
      "12345"                 // END_TILE_ROW
      "US"                    // COUNTRY
      "    "                  // RESERVED2
      "8800X10000Y"           // LOCATION
      "             "         // RESERVED3
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
   if(success)
   {
      std::stringstream tmpStream;
      status = this->isTreValid(*treDO.get(), tmpStream);  // This test should return SUSPECT
      if(status != SUSPECT) {
         failure << "---Negative test failed: START_TILE_COLUMN should have returned SUSPECT\n";
         failure << tmpStream.str();
         treDO->clear();
         return false;
      }
   }

   treDO->clear();
   return (status != INVALID);
}

Nitf::TreState Nitf::AimidbParser::isTreValid(const DynamicObject& tre, ostream& reporter) const
{
   TreState status(VALID);
   set<string>  testSet;
   unsigned int numFields = 0;

   FactoryResource<DateTime> appDTG;
   if (tre.getAttribute("ACQUISITION_DATE").getPointerToValue<DateTime>() == NULL)
   {
      reporter << "Field \"" << AIMIDB::ACQUISITION_DATE << "\" missing from the Dynamic Object";
      status = INVALID;;
   }
   else
   {
      ++numFields;
   }

   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, AIMIDB::MISSION_NO, testSet, false, true, false));

   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, AIMIDB::MISSION_IDENTIFICATION, testSet, false, true, false));

   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, AIMIDB::FLIGHT_NO, testSet, false, true, false));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, AIMIDB::OP_NUM, 0, 999));

   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, AIMIDB::CURRENT_SEGMENT, testSet, true, true, false));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, AIMIDB::REPRO_NUM, 0, 99));

   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, AIMIDB::REPLAY, testSet, true, true, false));

#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : This is a one char blank field in STDI-0002. For Classified versions this may be something else. (lbeck)")
   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, AIMIDB::RESERVED1, testSet, true, true, true));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, AIMIDB::START_TILE_COLUMN, 1, 99));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, AIMIDB::START_TILE_ROW, 1, 99999));

   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, AIMIDB::END_SEGMENT, testSet, false, true, false));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, AIMIDB::END_TILE_COLUMN, 1, 99));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, AIMIDB::END_TILE_ROW, 1, 99999));

   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, AIMIDB::COUNTRY, testSet, true, true, false));

#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : This is a four char blank field in STDI-0002. For Classified versions this may be something else. (lbeck)")
   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, AIMIDB::RESERVED2, testSet, true, true, true));

   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, AIMIDB::LOCATION, testSet, true, true, false));

   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, AIMIDB::RESERVED3, testSet, true, true, true));

   unsigned int totalFields = tre.getNumAttributes();
   if (status != INVALID && totalFields != numFields)
   {
      reporter << "Total fields in the Dynamic Object(" <<
         totalFields <<") did not match the number found(" << numFields << ") ";
      status = INVALID;
   }

   if (status == INVALID)
   {
      reporter << " INVALID fields found in the AIMIDB TAG/SDE\n" ;
   }
   else if (status == SUSPECT)
   {
      reporter << " SUSPECT fields found in the AIMIDB TAG/SDE\n" ;
   }

   return status;
}


bool Nitf::AimidbParser::toDynamicObject(istream& input, size_t numBytes, DynamicObject& output,
   string &errorMessage) const
{
   vector<char> buf;
   unsigned short yy(0), mm(0), dd(0), hh(0), min(0), ss(0);
   bool ok(true);

   bool success = true;

   success = readFromStream(input, buf, 14) && DtgParseCCYYMMDDhhmmss(string(&buf.front()), yy, mm, dd, hh, min, ss);
   if (success)
   {
      FactoryResource<DateTime> appDTG;
      success = appDTG->set(yy, mm, dd, hh, min, ss);
      if (success)
      {
         success = output.setAttribute(AIMIDB::ACQUISITION_DATE, *appDTG.get());
         if (!success)
         {
            errorMessage += "setAttribute " + AIMIDB::ACQUISITION_DATE + " failed\n";
         }
      }
      else
      {
         errorMessage += "DateTime Set " + AIMIDB::ACQUISITION_DATE + " failed\n";
      }
   }
   else
   {
      errorMessage += "Parsing " + AIMIDB::ACQUISITION_DATE + " failed\n";
   }


   readField<string>(input, output, success, AIMIDB::MISSION_NO, 4, errorMessage, buf);
   readField<string>(input, output, success, AIMIDB::MISSION_IDENTIFICATION, 10, errorMessage, buf);
   readField<string>(input, output, success, AIMIDB::FLIGHT_NO, 2, errorMessage, buf);
   readField<int>(input, output, success, AIMIDB::OP_NUM, 3, errorMessage, buf);
   readField<string>(input, output, success, AIMIDB::CURRENT_SEGMENT, 2, errorMessage, buf);
   readField<int>(input, output, success, AIMIDB::REPRO_NUM, 2, errorMessage, buf);
   readField<string>(input, output, success, AIMIDB::REPLAY, 3, errorMessage, buf, true);
   readField<string>(input, output, success, AIMIDB::RESERVED1, 1, errorMessage, buf, true);
   readField<int>(input, output, success, AIMIDB::START_TILE_COLUMN, 3, errorMessage, buf);
   readField<int>(input, output, success, AIMIDB::START_TILE_ROW, 5, errorMessage, buf);
   readField<string>(input, output, success, AIMIDB::END_SEGMENT, 2, errorMessage, buf);
   readField<int>(input, output, success, AIMIDB::END_TILE_COLUMN, 3, errorMessage, buf);
   readField<int>(input, output, success, AIMIDB::END_TILE_ROW, 5, errorMessage, buf);
   readField<string>(input, output, success, AIMIDB::COUNTRY, 2, errorMessage, buf, true);
   readField<string>(input, output, success, AIMIDB::RESERVED2, 4, errorMessage, buf, true);
   readField<string>(input, output, success, AIMIDB::LOCATION, 11, errorMessage, buf, true);
   readField<string>(input, output, success, AIMIDB::RESERVED3, 13, errorMessage, buf, true);

   size_t numRead = input.tellg();
   if (numRead != numBytes)
   {
      numReadErrMsg(numRead, numBytes, errorMessage);
      return false;
   }

   return success;
}



bool Nitf::AimidbParser::fromDynamicObject(const DynamicObject& input, ostream& output, size_t& numBytesWritten,
   string &errorMessage) const
{
   size_t sizeIn = max(static_cast<ostream::pos_type>(0), output.tellp());
   size_t sizeOut(sizeIn);

   try
   {
      // put date in form CCYYMMDDhhmmss for this TAG                   see: strftime() for format info
      const DateTime *pAppDtgAcquisitionDate = dv_cast<DateTime>(&input.getAttribute(AIMIDB::ACQUISITION_DATE));
      if (pAppDtgAcquisitionDate == NULL)
      {
         return false;
      }

      string CCYYMMDDhhmmss = pAppDtgAcquisitionDate->getFormattedUtc("%Y%m%d%H%M%S");
      output << sizeString(CCYYMMDDhhmmss, 14);

      output << sizeString( dv_cast<string>(input.getAttribute (AIMIDB::MISSION_NO)), 4);
      output << sizeString( dv_cast<string>(input.getAttribute (AIMIDB::MISSION_IDENTIFICATION)), 10);
      output << sizeString( dv_cast<string>(input.getAttribute (AIMIDB::FLIGHT_NO)), 2);
      output <<   toString( dv_cast<int>(input.getAttribute    (AIMIDB::OP_NUM)), 3);
      output << sizeString( dv_cast<string>(input.getAttribute (AIMIDB::CURRENT_SEGMENT)), 2);
      output <<   toString( dv_cast<int>(input.getAttribute    (AIMIDB::REPRO_NUM)), 2);
      output << sizeString( dv_cast<string>(input.getAttribute (AIMIDB::REPLAY)), 3);
      output << sizeString( dv_cast<string>(input.getAttribute (AIMIDB::RESERVED1)), 1);
      output <<   toString( dv_cast<int>(input.getAttribute    (AIMIDB::START_TILE_COLUMN)), 3);
      output <<   toString( dv_cast<int>(input.getAttribute    (AIMIDB::START_TILE_ROW)), 5);
      output << sizeString( dv_cast<string>(input.getAttribute (AIMIDB::END_SEGMENT)), 2);
      output <<   toString( dv_cast<int>(input.getAttribute    (AIMIDB::END_TILE_COLUMN)), 3);
      output <<   toString( dv_cast<int>(input.getAttribute    (AIMIDB::END_TILE_ROW)), 5);
      output << sizeString( dv_cast<string>(input.getAttribute (AIMIDB::COUNTRY)), 2);
      output << sizeString( dv_cast<string>(input.getAttribute (AIMIDB::RESERVED2)), 4);
      output << sizeString( dv_cast<string>(input.getAttribute (AIMIDB::LOCATION)), 11);
      output << sizeString( dv_cast<string>(input.getAttribute (AIMIDB::RESERVED3)), 13);
   }
   catch (const bad_cast&)
   {
      return false;
   }

   sizeOut = output.tellp();
   numBytesWritten = sizeOut - sizeIn;
   return true;
}
