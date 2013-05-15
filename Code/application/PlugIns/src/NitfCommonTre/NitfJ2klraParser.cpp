/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "DataVariant.h"
#include "DynamicObject.h"
#include "NitfConstants.h"
#include "NitfJ2klraParser.h"
#include "NitfUtilities.h"
#include "PlugInRegistration.h"
#include "StringUtilities.h"

#include <QtCore/QString>

#include <set>
#include <sstream>
#include <string>

REGISTER_PLUGIN(OpticksNitfCommonTre, J2klraParser, Nitf::J2klraParser());

Nitf::J2klraParser::J2klraParser()
{
   setName("J2KLRA");
   setDescriptorId("{16C03338-966F-4997-ADAE-EC9BFEA1A706}");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

Nitf::J2klraParser::~J2klraParser()
{}

Nitf::TreState Nitf::J2klraParser::isTreValid(const DynamicObject& tre, std::ostream& reporter) const
{
   TreState status = VALID;
   std::set<std::string> testSet;
   unsigned int numFields = 0;

   status = MaxState(status, testTagValueRange<unsigned short>(tre, reporter, &numFields, TRE::J2KLRA::ORIG, 0U, 9U));
   status = MaxState(status, testTagValueRange<unsigned short>(tre, reporter, &numFields, TRE::J2KLRA::NLEVELS_O,
      0U, 32U));
   status = MaxState(status, testTagValueRange<unsigned short>(tre, reporter, &numFields, TRE::J2KLRA::NBANDS_O,
      1U, 16384U));

   unsigned short numLayers = 0;
   try
   {
      numLayers = dv_cast<unsigned short>(tre.getAttribute(TRE::J2KLRA::NLAYERS_O));
      ++numFields;
   }
   catch (const std::bad_cast&)
   {
      reporter << "Field \"" << TRE::J2KLRA::NLAYERS_O << "\" missing from the Dynamic Object";
      status = INVALID;
   }
   status = MaxState(status, testTagValueRange<unsigned short>(reporter, numLayers, 1U, 999U));

   for (unsigned short i = 0; i < numLayers; ++i)
   {
      std::string layerId = TRE::J2KLRA::LAYER_ID + "[" + StringUtilities::toDisplayString(i) + "]";
      status = MaxState(status, testTagValueRange<unsigned short>(tre, reporter, &numFields, layerId, 0U, 998U));

      std::string bitrate = TRE::J2KLRA::BITRATE + "[" + StringUtilities::toDisplayString(i) + "]";
      status = MaxState(status, testTagValueRange<double>(tre, reporter, &numFields, bitrate, 0.0, 37.0));
   }

   status = MaxState(status, testTagValueRange<unsigned short>(tre, reporter, &numFields, TRE::J2KLRA::NLEVELS_I,
      0U, 32U, true));
   status = MaxState(status, testTagValueRange<unsigned short>(tre, reporter, &numFields, TRE::J2KLRA::NBANDS_I,
      1U, 16384U, true));
   status = MaxState(status, testTagValueRange<unsigned short>(tre, reporter, &numFields, TRE::J2KLRA::NLAYERS_I,
      1U, 999U, true));

   unsigned int totalFields = tre.getNumAttributes();
   if (status != INVALID && totalFields != numFields)
   {
      reporter << "Total fields in the Dynamic Object (" << totalFields << ") did not match the number found (" <<
         numFields << ") ";
      status = INVALID;
   }

   if (status == INVALID)
   {
      reporter << " INVALID fields found in the J2KLRA TAG/SDE\n" ;
   }
   else if (status == SUSPECT)
   {
      reporter << " SUSPECT fields found in the J2KLRA TAG/SDE\n" ;
   }

   return status;
}

bool Nitf::J2klraParser::toDynamicObject(std::istream& input, size_t numBytes, DynamicObject& output,
                                         std::string& errorMessage) const
{
   bool success = true;
   std::vector<char> buf;

   readField<unsigned short>(input, output, success, TRE::J2KLRA::ORIG, 1, errorMessage, buf);
   readField<unsigned short>(input, output, success, TRE::J2KLRA::NLEVELS_O, 2, errorMessage, buf);
   readField<unsigned short>(input, output, success, TRE::J2KLRA::NBANDS_O, 5, errorMessage, buf);
   readField<unsigned short>(input, output, success, TRE::J2KLRA::NLAYERS_O, 3, errorMessage, buf);

   if (success == true)
   {
      unsigned short numLayers = QString(&buf.front()).toUShort();
      for (unsigned short i = 0; i < numLayers; ++i)
      {
         std::string layerId = TRE::J2KLRA::LAYER_ID + "[" + StringUtilities::toDisplayString(i) + "]";
         readField<unsigned short>(input, output, success, layerId, 3, errorMessage, buf);

         std::string bitrate = TRE::J2KLRA::BITRATE + "[" + StringUtilities::toDisplayString(i) + "]";
         readField<double>(input, output, success, bitrate, 9, errorMessage, buf);
      }
   }

   int64_t numRead = input.tellg();
   if (numRead > 0 && static_cast<uint64_t>(numRead) < std::numeric_limits<size_t>::max() &&
      numRead != static_cast<int64_t>(numBytes))
   {
      readField<unsigned short>(input, output, success, TRE::J2KLRA::NLEVELS_I, 2, errorMessage, buf, true);
      readField<unsigned short>(input, output, success, TRE::J2KLRA::NBANDS_I, 5, errorMessage, buf, true);
      readField<unsigned short>(input, output, success, TRE::J2KLRA::NLAYERS_I, 3, errorMessage, buf, true);
      numRead = input.tellg();
   }

   if (numRead < 0 || static_cast<uint64_t>(numRead) > std::numeric_limits<size_t>::max() ||
      numRead != static_cast<int64_t>(numBytes))
   {
      numReadErrMsg(numRead, numBytes, errorMessage);
      return false;
   }

   return success;
}

bool Nitf::J2klraParser::fromDynamicObject(const DynamicObject& input, std::ostream& output, size_t& numBytesWritten,
                                           std::string &errorMessage) const
{
   if (output.tellp() < 0 || static_cast<uint64_t>(output.tellp()) > std::numeric_limits<size_t>::max())
   {
      return false;
   }

   size_t sizeIn = std::max<size_t>(0, static_cast<size_t>(output.tellp()));
   size_t sizeOut(sizeIn);

   try
   {
      output << toString(dv_cast<unsigned short>(input.getAttribute(TRE::J2KLRA::ORIG)), 1);
      output << toString(dv_cast<unsigned short>(input.getAttribute(TRE::J2KLRA::NLEVELS_O)), 2);
      output << toString(dv_cast<unsigned short>(input.getAttribute(TRE::J2KLRA::NBANDS_O)), 5);

      unsigned short numLayers = dv_cast<unsigned short>(input.getAttribute(TRE::J2KLRA::NLAYERS_O));
      output << toString(numLayers, 3);

      for (unsigned short i = 0; i < numLayers; ++i)
      {
         std::string layerId = TRE::J2KLRA::LAYER_ID + "[" + StringUtilities::toDisplayString(i) + "]";
         output << toString(dv_cast<unsigned short>(input.getAttribute(layerId)), 3);

         std::string bitrate = TRE::J2KLRA::BITRATE + "[" + StringUtilities::toDisplayString(i) + "]";
         output << toString(dv_cast<double>(input.getAttribute(bitrate)), 9, 6);
      }

      const DataVariant& levels = input.getAttribute(TRE::J2KLRA::NLEVELS_I);
      if (levels.isValid() == true)
      {
         output << toString(dv_cast<unsigned short>(levels), 2, -1, ZERO_FILL, false, false, 3, true);
      }

      const DataVariant& bands = input.getAttribute(TRE::J2KLRA::NBANDS_I);
      if (bands.isValid() == true)
      {
         output << toString(dv_cast<unsigned short>(bands), 5, -1, ZERO_FILL, false, false, 3, true);
      }

      const DataVariant& layers = input.getAttribute(TRE::J2KLRA::NLAYERS_I);
      if (layers.isValid() == true)
      {
         output << toString(dv_cast<unsigned short>(layers), 3, -1, ZERO_FILL, false, false, 3, true);
      }
   }
   catch (const std::bad_cast&)
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

bool Nitf::J2klraParser::runAllTests(Progress* pProgress, std::ostream& failure)
{
   static const std::string validData(
      "0"            // ORIG
      "01"           // NLEVELS_O
      "00001"        // NBANDS_O
      "001"          // NLAYERS_O
      "001"          // LAYER_ID
      "01.234567"    // BITRATE
      "01"           // NLEVELS_I
      "00001"        // NBANDS_I
      "001"          // NLAYERS_I
      );

   static const std::string alphanumericError(
      "0"            // ORIG
      "01"           // NLEVELS_O
      "00001"        // NBANDS_O
      "001"          // NLAYERS_O
      "001"          // LAYER_ID
      "01.234567"    // BITRATE
      "0a"           // NLEVELS_I      // Allowable characters are numeric values between 00 - 32
      "00001"        // NBANDS_I
      "001"          // NLAYERS_I
      );

   static const std::string outOfRangeError(
      "0"            // ORIG
      "01"           // NLEVELS_O
      "16385"        // NBANDS_O       // Maximum value = 16384
      "001"          // NLAYERS_O
      "001"          // LAYER_ID
      "01.234567"    // BITRATE
      "01"           // NLEVELS_I
      "00001"        // NBANDS_I
      "001"          // NLAYERS_I
      );

   FactoryResource<DynamicObject> pTreData;

   // Positive test: Valid TRE data
   std::istringstream input(validData);
   size_t numBytes = validData.size();
   std::string errorMessage;

   bool success = toDynamicObject(input, numBytes, *pTreData.get(), errorMessage);
   if (errorMessage.empty() == false)
   {
      failure << errorMessage << std::endl;
      errorMessage.clear();
   }

   TreState status = INVALID;
   if (success == true)
   {
      status = isTreValid(*pTreData.get(), failure);
   }

   if (status != VALID)
   {
      return false;
   }

   pTreData->clear();

   // Negative test: Invalid stream size error
   std::stringstream input2(validData);
   input2 << "1";
   numBytes = validData.size() + 1;

   success = toDynamicObject(input2, numBytes, *pTreData.get(), errorMessage);
   if (success == true)
   {
      failure << "Invalid stream size error test failed.\n";
      return false;
   }

   errorMessage.clear();
   pTreData->clear();

   // Negative test: Insufficient stream size error
   std::string dataTemp = validData;
   dataTemp.resize(validData.size() - 1);
   std::stringstream input3(dataTemp);
   numBytes = dataTemp.size();

   success = toDynamicObject(input3, numBytes, *pTreData.get(), errorMessage);
   if (success == true)
   {
      failure << "Insufficient stream size error test failed.\n";
      return false;
   }

   errorMessage.clear();
   pTreData->clear();

   // Negative test: Alphanumeric value error
   std::stringstream input4(alphanumericError);
   numBytes = input4.str().size();

   success = toDynamicObject(input4, numBytes, *pTreData.get(), errorMessage);
   if (success == true)
   {
      failure << "Alphanumeric value error test failed.\n";
      return false;
   }

   errorMessage.clear();
   pTreData->clear();

   // Negative test: Out-of-range error
   std::stringstream input5(outOfRangeError);
   numBytes = input5.str().size();

   success = toDynamicObject(input5, numBytes, *pTreData.get(), errorMessage);
   if (success == false)
   {
      failure << "Out-of-range error test failed to parse the TRE.\n";
      return false;
   }

   std::stringstream tmpStream;
   status = isTreValid(*pTreData.get(), tmpStream);
   if (status != SUSPECT)
   {
      failure << "Out-of-range error test failed to read the out-of-range value.\n";
      return false;
   }

   errorMessage.clear();
   pTreData->clear();
   return true;
}
