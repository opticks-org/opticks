/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QString>

#include "AppVersion.h"
#include "DataVariant.h"
#include "DimensionDescriptor.h"
#include "DynamicObject.h"
#include "NitfBandsaParser.h"
#include "NitfConstants.h"
#include "NitfUtilities.h"
#include "ObjectResource.h"
#include "PlugInRegistration.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterFileDescriptor.h"

#include <sstream>
#include <string>

using namespace std;
using namespace Nitf;
using namespace Nitf::TRE;

REGISTER_PLUGIN(OpticksNitfCommonTre, BandsaParser, Nitf::BandsaParser());

Nitf::BandsaParser::BandsaParser()
{
   setName("BANDSA");
   setDescriptorId("{24419D18-47FB-482f-AE47-832B86F4D07A}");
   setSubtype(CreateOnExportSubtype());      // This call is needed for exportMetadata()
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

bool Nitf::BandsaParser::runAllTests(Progress* pProgress, ostream& failure)
{
   static const string data(
      "99.9999"                 // ROW_SPACING
      "m"                       // ROW_SPACING_UNITS
      "9999.99"                 // COL_SPACING
      "r"                       // COL_SPACING_UNITS
      "899.99"                  // FOCAL_LENGTH
      "0003"                    // BANDCOUNT

      "00.01"                   // BANDPEAK
      "19.99"                   // BANDLBOUND
      "00.01"                   // BANDUBOUND
      "19.99"                   // BANDWIDTH
      "0000.1"                  // BANDCALDRK
      "00.01"                   // BANDCALINC
      "000.1"                   // BANDRESP
      "999.9"                   // BANDASD
      "99.99"                   // BANDGSD

      "00.01"                   // BANDPEAK
      "19.99"                   // BANDLBOUND
      "00.01"                   // BANDUBOUND
      "19.99"                   // BANDWIDTH
      "0000.1"                  // BANDCALDRK
      "00.01"                   // BANDCALINC
      "000.1"                   // BANDRESP
      "999.9"                   // BANDASD
      "99.99"                   // BANDGSD

      "00.01"                   // BANDPEAK
      "19.99"                   // BANDLBOUND
      "00.01"                   // BANDUBOUND
      "19.99"                   // BANDWIDTH
      "0000.1"                  // BANDCALDRK
      "00.01"                   // BANDCALINC
      "000.1"                   // BANDRESP
      "999.9"                   // BANDASD
      "99.99"                   // BANDGSD
      );

   static const string data_error4(
      "99.9999"                 // ROW_SPACING
      "m"                       // ROW_SPACING_UNITS
      "9999.99"                 // COL_SPACING
      "r"                       // COL_SPACING_UNITS
      "899.99"                  // FOCAL_LENGTH
      "0003"                    // BANDCOUNT

      "00.00"                   // BANDPEAK             Error min value = 00.01
      "20.00"                   // BANDLBOUND           Error max value = 19.99
      "00.01"                   // BANDUBOUND
      "19.99"                   // BANDWIDTH
      "0000.1"                  // BANDCALDRK
      "00.01"                   // BANDCALINC
      "000.1"                   // BANDRESP
      "999.9"                   // BANDASD
      "99.99"                   // BANDGSD

      "00.01"                   // BANDPEAK
      "19.99"                   // BANDLBOUND
      "00.01"                   // BANDUBOUND
      "19.99"                   // BANDWIDTH
      "0000.1"                  // BANDCALDRK
      "00.01"                   // BANDCALINC
      "000.1"                   // BANDRESP
      "999.9"                   // BANDASD
      "99.99"                   // BANDGSD

      "00.01"                   // BANDPEAK
      "19.99"                   // BANDLBOUND
      "00.01"                   // BANDUBOUND
      "19.99"                   // BANDWIDTH
      "0000.1"                  // BANDCALDRK
      "00.01"                   // BANDCALINC
      "000.1"                   // BANDRESP
      "999.9"                   // BANDASD
      "99.99"                   // BANDGSD
      );

   static const string data5(
      "99.9999"                 // ROW_SPACING
      "m"                       // ROW_SPACING_UNITS
      "9999.99"                 // COL_SPACING
      "r"                       // COL_SPACING_UNITS
      "899.99"                  // FOCAL_LENGTH
      "0003"                    // BANDCOUNT

      "     "                   // BANDPEAK - set to spaces
      "     "                   // BANDLBOUND - set to spaces
      "     "                   // BANDUBOUND - set to spaces
      "     "                   // BANDWIDTH - set to spaces
      "      "                  // BANDCALDRK - set to spaces
      "     "                   // BANDCALINC - set to spaces
      "     "                   // BANDRESP - set to spaces
      "     "                   // BANDASD - set to spaces
      "     "                   // BANDGSD - set to spaces

      "00.01"                   // BANDPEAK
      "19.99"                   // BANDLBOUND
      "00.01"                   // BANDUBOUND
      "19.99"                   // BANDWIDTH
      "0000.1"                  // BANDCALDRK
      "00.01"                   // BANDCALINC
      "000.1"                   // BANDRESP
      "999.9"                   // BANDASD
      "99.99"                   // BANDGSD

      "00.01"                   // BANDPEAK
      "19.99"                   // BANDLBOUND
      "00.01"                   // BANDUBOUND
      "19.99"                   // BANDWIDTH
      "0000.1"                  // BANDCALDRK
      "00.01"                   // BANDCALINC
      "000.1"                   // BANDRESP
      "999.9"                   // BANDASD
      "99.99"                   // BANDGSD
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
      std::stringstream tmpStream;
      status = this->isTreValid(*treDO.get(), tmpStream);
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

bool Nitf::BandsaParser::toDynamicObject(istream& input, size_t numBytes, DynamicObject& output,
   string &errorMessage) const
{
   vector<char> buf;
   bool ok(true);
   bool success(true);

   readField<double>(input, output, success, BANDSA::ROW_SPACING, 7, errorMessage, buf);
   readField<string>(input, output, success, BANDSA::ROW_SPACING_UNITS, 1, errorMessage, buf);
   readField<double>(input, output, success, BANDSA::COL_SPACING, 7, errorMessage, buf);
   readField<string>(input, output, success, BANDSA::COL_SPACING_UNITS, 1, errorMessage, buf);
   readField<double>(input, output, success, BANDSA::FOCAL_LENGTH, 6, errorMessage, buf);

   readField<unsigned int>(input, output, success, BANDSA::BANDCOUNT, 4, errorMessage, buf);
   unsigned int bandcount = QString(&buf.front()).toUInt();      // BANDCOUNT == the number of bands in the cube

   for (unsigned int i = 0; i < bandcount; ++i)
   {
      stringstream bandStreamStr;
      bandStreamStr << "#" << i;
      string bandStr(bandStreamStr.str());

      string fieldName;

      fieldName = BANDSA::BANDPEAK + bandStr;
      readField<double>(input, output, success, fieldName, 5, errorMessage, buf, true);

      fieldName = BANDSA::BANDLBOUND + bandStr;
      readField<double>(input, output, success, fieldName, 5, errorMessage, buf, true);

      fieldName = BANDSA::BANDUBOUND + bandStr;
      readField<double>(input, output, success, fieldName, 5, errorMessage, buf, true);

      fieldName = BANDSA::BANDWIDTH + bandStr;
      readField<double>(input, output, success, fieldName, 5, errorMessage, buf, true);

      fieldName = BANDSA::BANDCALDRK + bandStr;
      readField<double>(input, output, success, fieldName, 6, errorMessage, buf, true);

      fieldName = BANDSA::BANDCALINC + bandStr;
      readField<double>(input, output, success, fieldName, 5, errorMessage, buf, true);

      fieldName = BANDSA::BANDRESP + bandStr;
      readField<double>(input, output, success, fieldName, 5, errorMessage, buf, true);

      fieldName = BANDSA::BANDASD + bandStr;
      readField<double>(input, output, success, fieldName, 5, errorMessage, buf, true);

      fieldName = BANDSA::BANDGSD + bandStr;
      readField<double>(input, output, success, fieldName, 5, errorMessage, buf, true);
   }


   int numRead = input.tellg();
   if (numRead != numBytes)
   {
      numReadErrMsg(numRead, numBytes, errorMessage);
      return false;
   }

   return success;
}


Nitf::TreState Nitf::BandsaParser::isTreValid(const DynamicObject& tre, ostream& reporter) const
{
   TreState status(VALID);
   set<string>             testSet;
   unsigned int numFields = 0;

   unsigned int bandcount(0);
   try
   {
      bandcount = dv_cast<unsigned int>(tre.getAttribute(BANDSA::BANDCOUNT));
      numFields++;
   }
   catch (const bad_cast&)
   {
      reporter << "Field \"" << "BANDCOUNT" << "\" missing from the Dynamic Object";
      status = INVALID;
   }
   status = MaxState(status, testTagValueRange(reporter, bandcount, 1U, 999U));
   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, BANDSA::ROW_SPACING, 0.0F, 9999.99F));

   testSet.clear();
   testSet.insert("f");
   testSet.insert("m");
   testSet.insert("r");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, BANDSA::ROW_SPACING_UNITS, testSet, true, true, true));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, BANDSA::COL_SPACING, 0.0F, 9999.99F));

   testSet.clear();
   testSet.insert("f");
   testSet.insert("m");
   testSet.insert("r");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, BANDSA::COL_SPACING_UNITS, testSet, true, true, true));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, BANDSA::FOCAL_LENGTH, 0.005F, 999.99F));

   for (unsigned int i = 0; i < bandcount; ++i)
   {
      stringstream bandStreamStr;
      bandStreamStr << "#" << i;
      string bandStr(bandStreamStr.str());

      string fieldName;

      // In the following cases add 1/2 of the next decimal place (eg +0.005 to 0.01)
      // to the test range to allow for floating point round off

      fieldName = BANDSA::BANDPEAK + bandStr;
      status = MaxState(status, testTagValueRange<double>(tre, reporter,
         &numFields, fieldName, 0.005F, 19.995F, true));

      fieldName = BANDSA::BANDLBOUND + bandStr;
      status = MaxState(status, testTagValueRange<double>(tre, reporter,
         &numFields, fieldName, 0.005F, 19.995F, true));

      fieldName = BANDSA::BANDUBOUND + bandStr;
      status = MaxState(status, testTagValueRange<double>(tre, reporter,
         &numFields, fieldName, 0.005F, 19.995F, true));

      fieldName = BANDSA::BANDWIDTH + bandStr;
      status = MaxState(status, testTagValueRange<double>(tre, reporter,
         &numFields, fieldName, 0.005F, 19.995F, true));

      fieldName = BANDSA::BANDCALDRK + bandStr;
      status = MaxState(status, testTagValueRange<double>(tre, reporter,
         &numFields, fieldName, 0.05F, 9999.95F, true));

      fieldName = BANDSA::BANDCALINC + bandStr;
      status = MaxState(status, testTagValueRange<double>(tre, reporter,
         &numFields, fieldName, 0.005F, 19.995F, true));

      fieldName = BANDSA::BANDRESP + bandStr;
      status = MaxState(status, testTagValueRange<double>(tre, reporter,
         &numFields, fieldName, 0.05F, 999.995F, true));

      fieldName = BANDSA::BANDASD + bandStr;
      status = MaxState(status, testTagValueRange<double>(tre, reporter,
         &numFields, fieldName, 0.05F, 999.995F, true));

      fieldName = BANDSA::BANDGSD + bandStr;
      status = MaxState(status, testTagValueRange<double>(tre, reporter,
         &numFields, fieldName, 0.005F, 99.995F, true));
   }

   unsigned int totalFields = tre.getNumAttributes();
   if (status != INVALID && totalFields != numFields)
   {
      reporter << "Total fields in the Dynamic Object(" <<
         totalFields << ") did not match the number found(" << numFields << ") ";
      status = INVALID;
   }

   if (status == INVALID)
   {
      reporter << " INVALID fields found in the BANDSA TAG/SDE\n" ;
   }
   else if (status == SUSPECT)
   {
      reporter << " SUSPECT fields found in the BANDSA TAG/SDE\n" ;
   }

   return status;
}


bool Nitf::BandsaParser::fromDynamicObject(const DynamicObject& input, ostream& output, size_t& numBytesWritten,
   string &errorMessage) const
{
   size_t sizeIn = max(static_cast<ostream::pos_type>(0), output.tellp());
   size_t sizeOut(sizeIn);

   try
   {
      output << toString(dv_cast<double>(input.getAttribute(BANDSA::ROW_SPACING)), 7, 4);
      output << sizeString(dv_cast<string>(input.getAttribute(BANDSA::ROW_SPACING_UNITS)), 1);
      output << toString(dv_cast<double>(input.getAttribute(BANDSA::COL_SPACING)), 7, 4);
      output << sizeString(dv_cast<string>(input.getAttribute(BANDSA::COL_SPACING_UNITS)), 1);
      output << toString(dv_cast<double>(input.getAttribute(BANDSA::FOCAL_LENGTH)), 6, 2);

      unsigned int bandcount = dv_cast<unsigned int>(input.getAttribute(BANDSA::BANDCOUNT));
      output << toString(bandcount, 4);

      for (unsigned int i = 0; i < bandcount; ++i)
      {
         stringstream bandStreamStr;
         bandStreamStr << "#" << i;
         string bandStr(bandStreamStr.str());

         string fieldName;

         fieldName = BANDSA::BANDPEAK + bandStr;
         output << toString(dv_cast<double>(input.getAttribute(fieldName)), 5, 2, ZERO_FILL, false, false, 3, true);

         fieldName = BANDSA::BANDLBOUND + bandStr;
         output << toString(dv_cast<double>(input.getAttribute(fieldName)), 5, 2, ZERO_FILL, false, false, 3, true);

         fieldName = BANDSA::BANDUBOUND + bandStr;
         output << toString(dv_cast<double>(input.getAttribute(fieldName)), 5, 2, ZERO_FILL, false, false, 3, true);

         fieldName = BANDSA::BANDWIDTH + bandStr;
         output << toString(dv_cast<double>(input.getAttribute(fieldName)), 5, 2, ZERO_FILL, false, false, 3, true);

         fieldName = BANDSA::BANDCALDRK + bandStr;
         output << toString(dv_cast<double>(input.getAttribute(fieldName)), 6, 1, ZERO_FILL, false, false, 3, true);

         fieldName = BANDSA::BANDCALINC + bandStr;
         output << toString(dv_cast<double>(input.getAttribute(fieldName)), 5, 2, ZERO_FILL, false, false, 3, true);

         fieldName = BANDSA::BANDRESP + bandStr;
         output << toString(dv_cast<double>(input.getAttribute(fieldName)), 5, 1, ZERO_FILL, false, false, 3, true);

         fieldName = BANDSA::BANDASD + bandStr;
         output << toString(dv_cast<double>(input.getAttribute(fieldName)), 5, 2, ZERO_FILL, false, false, 3, true);

         fieldName = BANDSA::BANDGSD + bandStr;
         output << toString(dv_cast<double>(input.getAttribute(fieldName)), 5, 2, ZERO_FILL, false, false, 3, true);
      }

   }
   catch (const bad_cast&)
   {
      return false;
   }

   sizeOut = output.tellp();
   numBytesWritten = sizeOut - sizeIn;
   return true;
}

TreExportStatus Nitf::BandsaParser::exportMetadata(const RasterDataDescriptor &descriptor, 
   const RasterFileDescriptor &exportDescriptor, DynamicObject &tre, 
   unsigned int & ownerIndex, string & tagType, string &errorMessage) const
{
   // Find out if we are exporting a subset of the original bands. If so then delete the
   // band info for the excluded bands.

   const DynamicObject* pMetadata = descriptor.getMetadata();
   VERIFYRV(pMetadata != NULL, REMOVE);
   try
   {
      const DataVariant& nitfMetadata = pMetadata->getAttribute(Nitf::NITF_METADATA);
      const DynamicObject* pExistingBandsa = getTagHandle(dv_cast<DynamicObject>(nitfMetadata), "BANDSA", FindFirst());
      if (!pExistingBandsa)
      {
         return UNCHANGED;
      }

      const vector<DimensionDescriptor>& exportBands = exportDescriptor.getBands();

      VERIFYRV(!exportBands.empty(), REMOVE);

      tre.setAttribute(Nitf::TRE::BANDSA::ROW_SPACING,
         pExistingBandsa->getAttribute(Nitf::TRE::BANDSA::ROW_SPACING));
      tre.setAttribute(Nitf::TRE::BANDSA::ROW_SPACING_UNITS,
         pExistingBandsa->getAttribute(Nitf::TRE::BANDSA::ROW_SPACING_UNITS));
      tre.setAttribute(Nitf::TRE::BANDSA::COL_SPACING,
         pExistingBandsa->getAttribute(Nitf::TRE::BANDSA::COL_SPACING));
      tre.setAttribute(Nitf::TRE::BANDSA::COL_SPACING_UNITS,
         pExistingBandsa->getAttribute(Nitf::TRE::BANDSA::COL_SPACING_UNITS));
      tre.setAttribute(Nitf::TRE::BANDSA::FOCAL_LENGTH,
         pExistingBandsa->getAttribute(Nitf::TRE::BANDSA::FOCAL_LENGTH));

      unsigned int bandcount(0);

      for (vector<DimensionDescriptor>::const_iterator iter = exportBands.begin(); iter != exportBands.end(); ++iter)
      {
         // Use the original band number to find the associated band data in the original TRE
         LOG_IF(!iter->isOriginalNumberValid(), continue);
         unsigned int origBandNum = iter->getOriginalNumber();

         stringstream bandStreamStr;
         bandStreamStr << "#" << bandcount;
         string bandStr(bandStreamStr.str());

         stringstream origBandStreamStr;
         origBandStreamStr << "#" << origBandNum;
         string origBandStr(origBandStreamStr.str());

         ++bandcount;

         string fieldName;
         string origFieldName;

         fieldName = BANDSA::BANDPEAK + bandStr;
         origFieldName = BANDSA::BANDPEAK + origBandStr;
         tre.setAttribute(fieldName, pExistingBandsa->getAttribute(origFieldName));

         fieldName = BANDSA::BANDLBOUND + bandStr;
         origFieldName = BANDSA::BANDLBOUND + origBandStr;
         tre.setAttribute(fieldName, pExistingBandsa->getAttribute(origFieldName));

         fieldName = BANDSA::BANDUBOUND + bandStr;
         origFieldName = BANDSA::BANDUBOUND + origBandStr;
         tre.setAttribute(fieldName, pExistingBandsa->getAttribute(origFieldName));

         fieldName = BANDSA::BANDWIDTH + bandStr;
         origFieldName = BANDSA::BANDWIDTH + origBandStr;
         tre.setAttribute(fieldName, pExistingBandsa->getAttribute(origFieldName));

         fieldName = BANDSA::BANDCALDRK + bandStr;
         origFieldName = BANDSA::BANDCALDRK + origBandStr;
         tre.setAttribute(fieldName, pExistingBandsa->getAttribute(origFieldName));

         fieldName = BANDSA::BANDCALINC + bandStr;
         origFieldName = BANDSA::BANDCALINC + origBandStr;
         tre.setAttribute(fieldName, pExistingBandsa->getAttribute(origFieldName));

         fieldName = BANDSA::BANDRESP + bandStr;
         origFieldName = BANDSA::BANDRESP + origBandStr;
         tre.setAttribute(fieldName, pExistingBandsa->getAttribute(origFieldName));

         fieldName = BANDSA::BANDASD + bandStr;
         origFieldName = BANDSA::BANDASD + origBandStr;
         tre.setAttribute(fieldName, pExistingBandsa->getAttribute(origFieldName));

         fieldName = BANDSA::BANDGSD + bandStr;
         origFieldName = BANDSA::BANDGSD + origBandStr;
         tre.setAttribute(fieldName, pExistingBandsa->getAttribute(origFieldName));
      }

      tre.setAttribute(Nitf::TRE::BANDSA::BANDCOUNT, bandcount);
   }
   catch (const bad_cast&)
   {
      return REMOVE;
   }
   catch (const string& message)
   {
      errorMessage = message;
      return REMOVE;
   }

   return REPLACE;
}
