/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "DataVariant.h"
#include "DynamicObject.h"
#include "NitfConstants.h"
#include "NitfRpcParser.h"
#include "NitfUtilities.h"
#include "ObjectResource.h"

#include <sstream>
#include <ossim/support_data/ossimNitfRpcBase.h>

using namespace std;
using namespace Nitf;
using namespace Nitf::TRE;
using namespace Nitf::TRE::RPC;

Nitf::RpcParser::RpcParser()
{
   // Do nothing
}

bool Nitf::RpcParser::runAllTests(Progress* pProgress, ostream& failure)
{
   static const string data(
      "1"                       // SUCCESS
      "0128.03"                 // ERR_BIAS
      "0000.50"                 // ERR_RAND
      "002556"                  // LINE_OFFSET
      "01827"                   // SAMP_OFFSET
      "+33.5812"                // LAT_OFFSET
      "-112.0370"               // LONG_OFFSET
      "+0477"                   // HEIGHT_OFFSET
      "003716"                  // LINE_SCALE
      "06841"                   // SAMP_SCALE
      "+00.0341"                // LAT_SCALE
      "+000.0734"               // LONG_SCALE
      "+0162"                   // HEIGHT_SCALE

      "+6.267838E-4"            // LINE_NUMERATOR_COEF_PREFIX
      "+1.834668E-2"            // LINE_NUMERATOR_COEF_PREFIX
      "-1.018332E+0"            // LINE_NUMERATOR_COEF_PREFIX
      "+2.40411E-10"            // LINE_NUMERATOR_COEF_PREFIX
      "-7.031540E-4"            // LINE_NUMERATOR_COEF_PREFIX
      "+7.036708E-9"            // LINE_NUMERATOR_COEF_PREFIX
      "-3.905794E-7"            // LINE_NUMERATOR_COEF_PREFIX
      "-6.374024E-4"            // LINE_NUMERATOR_COEF_PREFIX
      "+3.272871E-3"            // LINE_NUMERATOR_COEF_PREFIX
      "-3.38456E-14"            // LINE_NUMERATOR_COEF_PREFIX
      "-7.23340E-11"            // LINE_NUMERATOR_COEF_PREFIX
      "-4.082647E-7"            // LINE_NUMERATOR_COEF_PREFIX
      "+2.818481E-7"            // LINE_NUMERATOR_COEF_PREFIX
      "-1.21567E-13"            // LINE_NUMERATOR_COEF_PREFIX
      "+2.192819E-6"            // LINE_NUMERATOR_COEF_PREFIX
      "-1.531184E-6"            // LINE_NUMERATOR_COEF_PREFIX
      "+3.76793E-12"            // LINE_NUMERATOR_COEF_PREFIX
      "-2.47717E-10"            // LINE_NUMERATOR_COEF_PREFIX
      "+2.83224E-10"            // LINE_NUMERATOR_COEF_PREFIX
      "+9.28724E-15"            // LINE_NUMERATOR_COEF_PREFIX

      "+1.000000E+0"            // LINE_DENOMINATOR_COEF_PREFIX
      "+6.416136E-4"            // LINE_DENOMINATOR_COEF_PREFIX
      "-3.216630E-3"            // LINE_DENOMINATOR_COEF_PREFIX
      "+3.835487E-7"            // LINE_DENOMINATOR_COEF_PREFIX
      "-2.931790E-7"            // LINE_DENOMINATOR_COEF_PREFIX
      "+6.94669E-11"            // LINE_DENOMINATOR_COEF_PREFIX
      "-2.79189E-10"            // LINE_DENOMINATOR_COEF_PREFIX
      "-4.228677E-7"            // LINE_DENOMINATOR_COEF_PREFIX
      "+1.511759E-6"            // LINE_DENOMINATOR_COEF_PREFIX
      "-3.68835E-12"            // LINE_DENOMINATOR_COEF_PREFIX
      "+2.38470E-14"            // LINE_DENOMINATOR_COEF_PREFIX
      "-7.25739E-11"            // LINE_DENOMINATOR_COEF_PREFIX
      "+5.28938E-11"            // LINE_DENOMINATOR_COEF_PREFIX
      "-4.96651E-15"            // LINE_DENOMINATOR_COEF_PREFIX
      "+5.10645E-10"            // LINE_DENOMINATOR_COEF_PREFIX
      "-2.11044E-12"            // LINE_DENOMINATOR_COEF_PREFIX
      "+5.46322E-14"            // LINE_DENOMINATOR_COEF_PREFIX
      "-4.30517E-14"            // LINE_DENOMINATOR_COEF_PREFIX
      "-3.21303E-14"            // LINE_DENOMINATOR_COEF_PREFIX
      "+3.33432E-14"            // LINE_DENOMINATOR_COEF_PREFIX

      "-4.111796E-4"            // SAMPLE_NUMERATOR_COEF_PREFIX
      "+9.952671E-1"            // SAMPLE_NUMERATOR_COEF_PREFIX
      "+5.539122E-3"            // SAMPLE_NUMERATOR_COEF_PREFIX
      "-1.57700E-10"            // SAMPLE_NUMERATOR_COEF_PREFIX
      "-3.589757E-3"            // SAMPLE_NUMERATOR_COEF_PREFIX
      "+3.817335E-7"            // SAMPLE_NUMERATOR_COEF_PREFIX
      "+2.124194E-9"            // SAMPLE_NUMERATOR_COEF_PREFIX
      "+6.340649E-4"            // SAMPLE_NUMERATOR_COEF_PREFIX
      "-1.531497E-5"            // SAMPLE_NUMERATOR_COEF_PREFIX
      "+1.35346E-14"            // SAMPLE_NUMERATOR_COEF_PREFIX
      "-4.27781E-10"            // SAMPLE_NUMERATOR_COEF_PREFIX
      "-3.173940E-7"            // SAMPLE_NUMERATOR_COEF_PREFIX
      "+2.588437E-6"            // SAMPLE_NUMERATOR_COEF_PREFIX
      "-3.67806E-12"            // SAMPLE_NUMERATOR_COEF_PREFIX
      "-5.165706E-7"            // SAMPLE_NUMERATOR_COEF_PREFIX
      "+1.80431E-11"            // SAMPLE_NUMERATOR_COEF_PREFIX
      "-1.27129E-14"            // SAMPLE_NUMERATOR_COEF_PREFIX
      "+6.73947E-11"            // SAMPLE_NUMERATOR_COEF_PREFIX
      "-6.15359E-13"            // SAMPLE_NUMERATOR_COEF_PREFIX
      "+4.17329E-15"            // SAMPLE_NUMERATOR_COEF_PREFIX

      "+1.000000E+0"            // SAMPLE_DENOMINATOR_COEF_PREFIX
      "+6.416136E-4"            // SAMPLE_DENOMINATOR_COEF_PREFIX
      "-3.216630E-3"            // SAMPLE_DENOMINATOR_COEF_PREFIX
      "+3.835487E-7"            // SAMPLE_DENOMINATOR_COEF_PREFIX
      "-2.931790E-7"            // SAMPLE_DENOMINATOR_COEF_PREFIX
      "+6.94669E-11"            // SAMPLE_DENOMINATOR_COEF_PREFIX
      "-2.79189E-10"            // SAMPLE_DENOMINATOR_COEF_PREFIX
      "-4.228677E-7"            // SAMPLE_DENOMINATOR_COEF_PREFIX
      "+1.511759E-6"            // SAMPLE_DENOMINATOR_COEF_PREFIX
      "-3.68835E-12"            // SAMPLE_DENOMINATOR_COEF_PREFIX
      "+2.38470E-14"            // SAMPLE_DENOMINATOR_COEF_PREFIX
      "-7.25739E-11"            // SAMPLE_DENOMINATOR_COEF_PREFIX
      "+5.28938E-11"            // SAMPLE_DENOMINATOR_COEF_PREFIX
      "-4.96651E-15"            // SAMPLE_DENOMINATOR_COEF_PREFIX
      "+5.10645E-10"            // SAMPLE_DENOMINATOR_COEF_PREFIX
      "-2.11044E-12"            // SAMPLE_DENOMINATOR_COEF_PREFIX
      "+5.46322E-14"            // SAMPLE_DENOMINATOR_COEF_PREFIX
      "-4.30517E-14"            // SAMPLE_DENOMINATOR_COEF_PREFIX
      "-3.21303E-14"            // SAMPLE_DENOMINATOR_COEF_PREFIX
      "+3.33432E-14"            // SAMPLE_DENOMINATOR_COEF_PREFIX
      );

   static const string data_error4(
      "1"                       // SUCCESS
      "0128.03"                 // ERR_BIAS
      "0000.50"                 // ERR_RAND
      "0255.6"                  // LINE_OFFSET        // ERROR floating point in int field
      "01827"                   // SAMP_OFFSET
      "+33.5812"                // LAT_OFFSET
      "-112.0370"               // LONG_OFFSET
      "+0477"                   // HEIGHT_OFFSET
      "003716"                  // LINE_SCALE
      "06841"                   // SAMP_SCALE
      "+00.0341"                // LAT_SCALE
      "+000.0734"               // LONG_SCALE
      "+0162"                   // HEIGHT_SCALE

      "+6.267838E-4"            // LINE_NUMERATOR_COEF_PREFIX
      "+1.834668E-2"            // LINE_NUMERATOR_COEF_PREFIX
      "-1.018332E+0"            // LINE_NUMERATOR_COEF_PREFIX
      "+2.40411E-10"            // LINE_NUMERATOR_COEF_PREFIX
      "-7.031540E-4"            // LINE_NUMERATOR_COEF_PREFIX
      "+7.036708E-9"            // LINE_NUMERATOR_COEF_PREFIX
      "-3.905794E-7"            // LINE_NUMERATOR_COEF_PREFIX
      "-6.374024E-4"            // LINE_NUMERATOR_COEF_PREFIX
      "+3.272871E-3"            // LINE_NUMERATOR_COEF_PREFIX
      "-3.38456E-14"            // LINE_NUMERATOR_COEF_PREFIX
      "-7.23340E-11"            // LINE_NUMERATOR_COEF_PREFIX
      "-4.082647E-7"            // LINE_NUMERATOR_COEF_PREFIX
      "+2.818481E-7"            // LINE_NUMERATOR_COEF_PREFIX
      "-1.21567E-13"            // LINE_NUMERATOR_COEF_PREFIX
      "+2.192819E-6"            // LINE_NUMERATOR_COEF_PREFIX
      "-1.531184E-6"            // LINE_NUMERATOR_COEF_PREFIX
      "+3.76793E-12"            // LINE_NUMERATOR_COEF_PREFIX
      "-2.47717E-10"            // LINE_NUMERATOR_COEF_PREFIX
      "+2.83224E-10"            // LINE_NUMERATOR_COEF_PREFIX
      "+9.28724E-15"            // LINE_NUMERATOR_COEF_PREFIX

      "+1.000000E+0"            // LINE_DENOMINATOR_COEF_PREFIX
      "+6.416136E-4"            // LINE_DENOMINATOR_COEF_PREFIX
      "-3.216630E-3"            // LINE_DENOMINATOR_COEF_PREFIX
      "+3.835487E-7"            // LINE_DENOMINATOR_COEF_PREFIX
      "-2.931790E-7"            // LINE_DENOMINATOR_COEF_PREFIX
      "+6.94669E-11"            // LINE_DENOMINATOR_COEF_PREFIX
      "-2.79189E-10"            // LINE_DENOMINATOR_COEF_PREFIX
      "-4.228677E-7"            // LINE_DENOMINATOR_COEF_PREFIX
      "+1.511759E-6"            // LINE_DENOMINATOR_COEF_PREFIX
      "-3.68835E-12"            // LINE_DENOMINATOR_COEF_PREFIX
      "+2.38470E-14"            // LINE_DENOMINATOR_COEF_PREFIX
      "-7.25739E-11"            // LINE_DENOMINATOR_COEF_PREFIX
      "+5.28938E-11"            // LINE_DENOMINATOR_COEF_PREFIX
      "-4.96651E-15"            // LINE_DENOMINATOR_COEF_PREFIX
      "+5.10645E-10"            // LINE_DENOMINATOR_COEF_PREFIX
      "-2.11044E-12"            // LINE_DENOMINATOR_COEF_PREFIX
      "+5.46322E-14"            // LINE_DENOMINATOR_COEF_PREFIX
      "-4.30517E-14"            // LINE_DENOMINATOR_COEF_PREFIX
      "-3.21303E-14"            // LINE_DENOMINATOR_COEF_PREFIX
      "+3.33432E-14"            // LINE_DENOMINATOR_COEF_PREFIX

      "-4.111796E-4"            // SAMPLE_NUMERATOR_COEF_PREFIX
      "+9.952671E-1"            // SAMPLE_NUMERATOR_COEF_PREFIX
      "+5.539122E-3"            // SAMPLE_NUMERATOR_COEF_PREFIX
      "-1.57700E-10"            // SAMPLE_NUMERATOR_COEF_PREFIX
      "-3.589757E-3"            // SAMPLE_NUMERATOR_COEF_PREFIX
      "+3.817335E-7"            // SAMPLE_NUMERATOR_COEF_PREFIX
      "+2.124194E-9"            // SAMPLE_NUMERATOR_COEF_PREFIX
      "+6.340649E-4"            // SAMPLE_NUMERATOR_COEF_PREFIX
      "-1.531497E-5"            // SAMPLE_NUMERATOR_COEF_PREFIX
      "+1.35346E-14"            // SAMPLE_NUMERATOR_COEF_PREFIX
      "-4.27781E-10"            // SAMPLE_NUMERATOR_COEF_PREFIX
      "-3.173940E-7"            // SAMPLE_NUMERATOR_COEF_PREFIX
      "+2.588437E-6"            // SAMPLE_NUMERATOR_COEF_PREFIX
      "-3.67806E-12"            // SAMPLE_NUMERATOR_COEF_PREFIX
      "-5.165706E-7"            // SAMPLE_NUMERATOR_COEF_PREFIX
      "+1.80431E-11"            // SAMPLE_NUMERATOR_COEF_PREFIX
      "-1.27129E-14"            // SAMPLE_NUMERATOR_COEF_PREFIX
      "+6.73947E-11"            // SAMPLE_NUMERATOR_COEF_PREFIX
      "-6.15359E-13"            // SAMPLE_NUMERATOR_COEF_PREFIX
      "+4.17329E-15"            // SAMPLE_NUMERATOR_COEF_PREFIX

      "+1.000000E+0"            // SAMPLE_DENOMINATOR_COEF_PREFIX
      "+6.416136E-4"            // SAMPLE_DENOMINATOR_COEF_PREFIX
      "-3.216630E-3"            // SAMPLE_DENOMINATOR_COEF_PREFIX
      "+3.835487E-7"            // SAMPLE_DENOMINATOR_COEF_PREFIX
      "-2.931790E-7"            // SAMPLE_DENOMINATOR_COEF_PREFIX
      "+6.94669E-11"            // SAMPLE_DENOMINATOR_COEF_PREFIX
      "-2.79189E-10"            // SAMPLE_DENOMINATOR_COEF_PREFIX
      "-4.228677E-7"            // SAMPLE_DENOMINATOR_COEF_PREFIX
      "+1.511759E-6"            // SAMPLE_DENOMINATOR_COEF_PREFIX
      "-3.68835E-12"            // SAMPLE_DENOMINATOR_COEF_PREFIX
      "+2.38470E-14"            // SAMPLE_DENOMINATOR_COEF_PREFIX
      "-7.25739E-11"            // SAMPLE_DENOMINATOR_COEF_PREFIX
      "+5.28938E-11"            // SAMPLE_DENOMINATOR_COEF_PREFIX
      "-4.96651E-15"            // SAMPLE_DENOMINATOR_COEF_PREFIX
      "+5.10645E-10"            // SAMPLE_DENOMINATOR_COEF_PREFIX
      "-2.11044E-12"            // SAMPLE_DENOMINATOR_COEF_PREFIX
      "+5.46322E-14"            // SAMPLE_DENOMINATOR_COEF_PREFIX
      "-4.30517E-14"            // SAMPLE_DENOMINATOR_COEF_PREFIX
      "-3.21303E-14"            // SAMPLE_DENOMINATOR_COEF_PREFIX
      "+3.33432E-14"            // SAMPLE_DENOMINATOR_COEF_PREFIX
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

   // Start of test 4 - false parser test
   stringstream input4(data_error4);
   numBytes = input4.str().size();

   success = toDynamicObject(input4, numBytes, *treDO.get(), errorMessage);

   if (success)
   {
      failure << "Error: Negative test: LINE_OFFSET floating point in int field failed: did not return false\n";
      return false;
   }

   treDO->clear();

   return (status != INVALID);
}

bool Nitf::RpcParser::ossimTagToDynamicObject(const ossimNitfRegisteredTag& input, DynamicObject& output,
   string &errorMessage) const
{
   const ossimNitfRegisteredTag* pInput = &input;
   const ossimNitfRpcBase* pBase = PTR_CAST(ossimNitfRpcBase, pInput);

   if (pBase != NULL)
   {
      bool status = output.setAttribute(SUCCESS, pBase->getSuccess());
      status = status && output.setAttribute(SAMP_SCALE, pBase->getSampleScale().toUInt32());
      status = status && output.setAttribute(SAMP_OFFSET, pBase->getSampleOffset().toUInt32());
      status = status && output.setAttribute(LONG_SCALE, pBase->getGeodeticLonScale().toDouble());
      status = status && output.setAttribute(LONG_OFFSET, pBase->getGeodeticLonOffset().toDouble());
      status = status && output.setAttribute(LINE_SCALE, pBase->getLineScale().toUInt32());
      status = status && output.setAttribute(LINE_OFFSET, pBase->getLineOffset().toUInt32());
      status = status && output.setAttribute(LAT_SCALE, pBase->getGeodeticLatScale().toDouble());
      status = status && output.setAttribute(LAT_OFFSET, pBase->getGeodeticLatOffset().toDouble());
      status = status && output.setAttribute(HEIGHT_SCALE, pBase->getGeodeticHeightScale().toInt());
      status = status && output.setAttribute(HEIGHT_OFFSET, pBase->getGeodeticHeightOffset().toInt());
      status = status && output.setAttribute(ERR_RAND, pBase->getErrorRand().toDouble());
      status = status && output.setAttribute(ERR_BIAS, pBase->getErrorBias().toDouble());

      unsigned int u;
      for (u = 1; u <= 20; ++u)
      {
         double coeff = pBase->getLineNumeratorCoeff(u-1).toDouble();
         status = status && output.setAttribute(getRpcCoefficient(LINE_NUMERATOR_COEF_PREFIX, u), coeff);
      }
      for (u = 1; u <= 20; ++u)
      {
         double coeff = pBase->getLineDenominatorCoeff(u-1).toDouble();
         status = status && output.setAttribute(getRpcCoefficient(LINE_DENOMINATOR_COEF_PREFIX, u), coeff);
      }
      for (u = 1; u <= 20; ++u)
      {
         double coeff = pBase->getSampleNumeratorCoeff(u-1).toDouble();
         status = status && output.setAttribute(getRpcCoefficient(SAMPLE_NUMERATOR_COEF_PREFIX, u), coeff);
      }
      for (u = 1; u <= 20; ++u)
      {
         double coeff = pBase->getSampleDenominatorCoeff(u-1).toDouble();
         status = status && output.setAttribute(getRpcCoefficient(SAMPLE_DENOMINATOR_COEF_PREFIX, u), coeff);
      }

      return status;
   }

   return false;
}

bool Nitf::RpcParser::toDynamicObject(istream& input, size_t numBytes, DynamicObject& output,
   string &errorMessage) const
{
   vector<char> buf;
   bool success = true;
   unsigned int u;

   readField<bool>(input, output, success, SUCCESS, 1, errorMessage, buf);
   readField<double>(input, output, success, ERR_BIAS, 7, errorMessage, buf);
   readField<double>(input, output, success, ERR_RAND, 7, errorMessage, buf);
   readField<unsigned int>(input, output, success, LINE_OFFSET, 6, errorMessage, buf);
   readField<unsigned int>(input, output, success, SAMP_OFFSET, 5, errorMessage, buf);
   readField<double>(input, output, success, LAT_OFFSET, 8, errorMessage, buf);
   readField<double>(input, output, success, LONG_OFFSET, 9, errorMessage, buf);
   readField<int>(input, output, success, HEIGHT_OFFSET, 5, errorMessage, buf);
   readField<unsigned int>(input, output, success, LINE_SCALE, 6, errorMessage, buf);
   readField<unsigned int>(input, output, success, SAMP_SCALE, 5, errorMessage, buf);
   readField<double>(input, output, success, LAT_SCALE, 8, errorMessage, buf);
   readField<double>(input, output, success, LONG_SCALE, 9, errorMessage, buf);
   readField<int>(input, output, success, HEIGHT_SCALE, 5, errorMessage, buf);

   for (u = 1; u <= 20; ++u)
   {
      readField<double>(input, output, success,
         getRpcCoefficient(LINE_NUMERATOR_COEF_PREFIX, u), 12, errorMessage, buf);
   }
   for (u = 1; u <= 20; ++u)
   {
      readField<double>(input, output, success,
         getRpcCoefficient(LINE_DENOMINATOR_COEF_PREFIX, u), 12, errorMessage, buf);
   }
   for (u = 1; u <= 20; ++u)
   {
      readField<double>(input, output, success,
         getRpcCoefficient(SAMPLE_NUMERATOR_COEF_PREFIX, u), 12, errorMessage, buf);
   }
   for (u = 1; u <= 20; ++u)
   {
      readField<double>(input, output, success,
         getRpcCoefficient(SAMPLE_DENOMINATOR_COEF_PREFIX, u), 12, errorMessage, buf);
   }

   int64_t numRead = input.tellg();
   if (numRead < 0 || numRead > static_cast<int64_t>(std::numeric_limits<size_t>::max()) ||
      numRead != static_cast<int64_t>(numBytes))
   {
      numReadErrMsg(numRead, numBytes, errorMessage);
      return false;
   }

   return success;
}

Nitf::TreState Nitf::RpcParser::isTreValid(const DynamicObject& tre, ostream& reporter) const
{
   TreState status(VALID);
   set<string>             testSet;
   unsigned int numFields = 0;
   unsigned int u;

   static const double minCoef(-9.999999E9);
   static const double maxCoef(9.999999E9);

   status = MaxState(status, testTagValueRange<bool>(tre, reporter, &numFields, SUCCESS, false, true));
   status = MaxState(status, testTagValueRange<double>(tre, reporter, &numFields, ERR_BIAS, 0.0, 9999.99));
   status = MaxState(status, testTagValueRange<double>(tre, reporter, &numFields, ERR_RAND, 0.0, 9999.99));
   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter, &numFields, LINE_OFFSET, 0U, 999999U));
   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter, &numFields, SAMP_OFFSET, 0U, 99999U));
   status = MaxState(status, testTagValueRange<double>(tre, reporter, &numFields, LAT_OFFSET, -90.0000, 90.0000));
   status = MaxState(status, testTagValueRange<double>(tre, reporter, &numFields, LONG_OFFSET, -180.0000, 180.0000));
   status = MaxState(status, testTagValueRange<int>(tre, reporter, &numFields, HEIGHT_OFFSET, -9999, 9999));
   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter, &numFields, LINE_SCALE, 1U, 999999U));
   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter, &numFields, SAMP_SCALE, 1U, 99999U));
   status = MaxState(status, testTagValueRange<double>(tre, reporter, &numFields, LAT_SCALE, -90.0000, 90.0000));
   status = MaxState(status, testTagValueRange<double>(tre, reporter, &numFields, LONG_SCALE, -180.0000, 180.0000));
   status = MaxState(status, testTagValueRange<int>(tre, reporter, &numFields, HEIGHT_SCALE, -9999, 9999));

   for (u = 1; u <= 20; ++u)
   {
      status = MaxState(status, testTagValueRange<double>(tre, reporter, &numFields,
         getRpcCoefficient(LINE_NUMERATOR_COEF_PREFIX, u), minCoef, maxCoef));
   }

   for (u = 1; u <= 20; ++u)
   {
      status = MaxState(status, testTagValueRange<double>(tre, reporter, &numFields,
         getRpcCoefficient(LINE_DENOMINATOR_COEF_PREFIX, u), minCoef, maxCoef));
   }

   for (u = 1; u <= 20; ++u)
   {
      status = MaxState(status, testTagValueRange<double>(tre, reporter, &numFields,
         getRpcCoefficient(SAMPLE_NUMERATOR_COEF_PREFIX, u), minCoef, maxCoef));
   }

   for (u = 1; u <= 20; ++u)
   {
      status = MaxState(status, testTagValueRange<double>(tre, reporter, &numFields,
         getRpcCoefficient(SAMPLE_DENOMINATOR_COEF_PREFIX, u), minCoef, maxCoef));
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
      reporter << " INVALID fields found in the RPC TAG/SDE\n" ;
   }
   else if (status == SUSPECT)
   {
      reporter << " SUSPECT fields found in the RPC TAG/SDE\n" ;
   }

   return status;
}



bool Nitf::RpcParser::fromDynamicObject(const DynamicObject& input, ostream& output, size_t& numBytesWritten,
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
      output << dv_cast<bool>(input.getAttribute(SUCCESS)) ? "1" : "0";
      output << toString(dv_cast<double>(input.getAttribute(ERR_BIAS)), 7, 2);
      output << toString(dv_cast<double>(input.getAttribute(ERR_RAND)), 7, 2);
      output << toString(dv_cast<unsigned int>(input.getAttribute(LINE_OFFSET)), 6);
      output << toString(dv_cast<unsigned int>(input.getAttribute(SAMP_OFFSET)), 5);
      output << toString(dv_cast<double>(input.getAttribute(LAT_OFFSET)), 8, 4, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<double>(input.getAttribute(LONG_OFFSET)), 9, 4, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<int>(input.getAttribute(HEIGHT_OFFSET)), 5, -1, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<unsigned int>(input.getAttribute(LINE_SCALE)), 6);
      output << toString(dv_cast<unsigned int>(input.getAttribute(SAMP_SCALE)), 5);
      output << toString(dv_cast<double>(input.getAttribute(LAT_SCALE)), 8, 4, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<double>(input.getAttribute(LONG_SCALE)), 9, 4, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<int>(input.getAttribute(HEIGHT_SCALE)), 5, -1, ZERO_FILL, POS_SIGN_TRUE);

      unsigned int u;

      for (u = 1; u <= 20; ++u)
      {
         output << toString(dv_cast<double>(input.getAttribute(getRpcCoefficient(LINE_NUMERATOR_COEF_PREFIX, u))),
            12, 6, ZERO_FILL, POS_SIGN_TRUE, USE_SCIENTIFIC_NOTATION, ONE_EXP_DIGIT);
      }
      for (u = 1; u <= 20; ++u)
      {
         output << toString(dv_cast<double>(input.getAttribute(getRpcCoefficient(LINE_DENOMINATOR_COEF_PREFIX, u))),
            12, 6, ZERO_FILL, POS_SIGN_TRUE, USE_SCIENTIFIC_NOTATION, ONE_EXP_DIGIT);
      }
      for (u = 1; u <= 20; ++u)
      {
         output << toString(dv_cast<double>(input.getAttribute(getRpcCoefficient(SAMPLE_NUMERATOR_COEF_PREFIX, u))),
            12, 6, ZERO_FILL, POS_SIGN_TRUE, USE_SCIENTIFIC_NOTATION, ONE_EXP_DIGIT);
      }
      for (u = 1; u <= 20; ++u)
      {
         output << toString(dv_cast<double>(input.getAttribute(getRpcCoefficient(SAMPLE_DENOMINATOR_COEF_PREFIX, u))),
            12, 6, ZERO_FILL, POS_SIGN_TRUE, USE_SCIENTIFIC_NOTATION, ONE_EXP_DIGIT);
      }
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
