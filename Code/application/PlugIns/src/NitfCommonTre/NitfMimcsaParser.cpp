/*
 * The information in this file is
 * Copyright(c) 2021 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"
#include "AppVersion.h"
#include "DataVariant.h"
#include "NitfMimcsaParser.h"
#include "NitfConstants.h"
#include "NitfUtilities.h"
#include "ObjectResource.h"
#include "PlugInRegistration.h"

using namespace std;
using namespace Nitf;
using namespace Nitf::TRE;

REGISTER_PLUGIN(OpticksNitfCommonTre, MimcsaParser, Nitf::MimcsaParser());

Nitf::MimcsaParser::MimcsaParser()
{
   setName("MIMCSA");
   setDescriptorId("{2C4B6EDC-D491-4C90-8C6F-E35FD64B8816}");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

bool Nitf::MimcsaParser::runAllTests(Progress* pProgress, ostream& failure)
{
   // TRE doesn't have NTB tests yet
   return true;
}

Nitf::TreState Nitf::MimcsaParser::isTreValid(const DynamicObject& tre, ostream& reporter) const
{
   TreState status(VALID);
   set<string> testSet;
   unsigned int numFields = 0;

   status = testTagValidBcsASet(tre, reporter, &numFields, MIMCSA::LAYER_ID, testSet, false, true, false);
   status = MaxState(status, testTagValueRange<double>(tre, reporter, &numFields, MIMCSA::NOMINAL_FRAME_RATE, 0, 3.4028234e38));
   status = MaxState(status, testTagValueRange<double>(tre, reporter, &numFields, MIMCSA::MIN_FRAME_RATE, 0, 3.4028234e38));
   status = MaxState(status, testTagValueRange<double>(tre, reporter, &numFields, MIMCSA::MAX_FRAME_RATE, 0, 3.4028234e38));
   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter, &numFields, MIMCSA::T_RSET, 0, 99));

   testSet.clear();
   testSet.insert("C9");
   testSet.insert("M9");
   testSet.insert("CA");
   testSet.insert("MA");
   testSet.insert("CB");
   testSet.insert("MB");
   testSet.insert("NC");
   testSet.insert("NM");
   testSet.insert("C8");
   testSet.insert("M8");
   status = MaxState(status, testTagValidBcsASet(tre, reporter, &numFields, MIMCSA::MI_REQ_DECODER, testSet, false, false, true));

   testSet.clear();
   testSet.insert("Not Applicable");
   status = MaxState(status, testTagValidBcsASet(tre, reporter, &numFields, MIMCSA::MI_REQ_PROFILE, testSet, true, true, true));

   testSet.clear();
   testSet.insert("N/A");
   status = MaxState(status, testTagValidBcsASet(tre, reporter, &numFields, MIMCSA::MI_REQ_LEVEL, testSet, true, true, true));

   return status;
}


bool Nitf::MimcsaParser::toDynamicObject(istream& input, size_t numBytes, DynamicObject& output,
   string &errorMessage) const
{
   vector<char> buf;
   bool success(true);

   readField<string>(input, output, success, MIMCSA::LAYER_ID, 36, errorMessage, buf);
   readField<double>(input, output, success, MIMCSA::NOMINAL_FRAME_RATE, 13, errorMessage, buf);
   readField<double>(input, output, success, MIMCSA::MIN_FRAME_RATE, 13, errorMessage, buf);
   readField<double>(input, output, success, MIMCSA::MAX_FRAME_RATE, 13, errorMessage, buf);
   readField<unsigned int>(input, output, success, MIMCSA::T_RSET, 2, errorMessage, buf);
   readField<string>(input, output, success, MIMCSA::MI_REQ_DECODER, 2, errorMessage, buf);
   readField<string>(input, output, success, MIMCSA::MI_REQ_PROFILE, 36, errorMessage, buf);
   readField<string>(input, output, success, MIMCSA::MI_REQ_LEVEL, 6, errorMessage, buf);

   int64_t numRead = input.tellg();
   if (numRead < 0 || static_cast<uint64_t>(numRead) > std::numeric_limits<size_t>::max() ||
      numRead != static_cast<int64_t>(numBytes))
   {
      numReadErrMsg(numRead, numBytes, errorMessage);
      return false;
   }

   return success;
}

bool Nitf::MimcsaParser::fromDynamicObject(const DynamicObject& input, ostream& output, size_t& numBytesWritten,
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
      output << sizeString(dv_cast<string>(input.getAttribute(MIMCSA::LAYER_ID)), 36);
      output << toString(dv_cast<double>(input.getAttribute(MIMCSA::NOMINAL_FRAME_RATE)), 13);
      output << toString(dv_cast<double>(input.getAttribute(MIMCSA::MIN_FRAME_RATE)), 13);
      output << toString(dv_cast<double>(input.getAttribute(MIMCSA::MAX_FRAME_RATE)), 13);
      output << toString(dv_cast<unsigned int>(input.getAttribute(MIMCSA::T_RSET)), 2);
      output << sizeString(dv_cast<string>(input.getAttribute(MIMCSA::MI_REQ_DECODER)), 2);
      output << sizeString(dv_cast<string>(input.getAttribute(MIMCSA::MI_REQ_PROFILE)), 36);
      output << sizeString(dv_cast<string>(input.getAttribute(MIMCSA::MI_REQ_LEVEL)), 6);
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
