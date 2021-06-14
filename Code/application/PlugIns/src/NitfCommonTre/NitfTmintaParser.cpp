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
#include "DateTime.h"
#include "NitfTmintaParser.h"
#include "NitfConstants.h"
#include "NitfUtilities.h"
#include "ObjectResource.h"
#include "PlugInRegistration.h"

using namespace std;
using namespace Nitf;
using namespace Nitf::TRE;

REGISTER_PLUGIN(OpticksNitfCommonTre, TmintaParser, Nitf::TmintaParser());

Nitf::TmintaParser::TmintaParser()
{
   setName("TMINTA");
   setDescriptorId("{823E971A-33B2-43F3-BB24-F56D69209A18}");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

bool Nitf::TmintaParser::runAllTests(Progress* pProgress, ostream& failure)
{
   // TRE doesn't have NTB tests yet
   return true;
}

Nitf::TreState Nitf::TmintaParser::isTreValid(const DynamicObject& tre, ostream& reporter) const
{
   TreState status(VALID);
   set<string> testSet;
   unsigned int numFields = 0;

   // TODO

   return status;
}


bool Nitf::TmintaParser::toDynamicObject(istream& input, size_t numBytes, DynamicObject& output,
   string &errorMessage) const
{
   vector<char> buf;
   bool success(true);

   unsigned int numTimeInt = 0;
   readAndConvertFromStream(input, numTimeInt, success, TMINTA::NUM_TIME_INT, 4, errorMessage, buf);
   output.setAttribute(TMINTA::NUM_TIME_INT, numTimeInt);
   {
      FactoryResource<DynamicObject> timeInts;
      output.setAttribute(TMINTA::TIME_INT, *timeInts.get());
   }
   DynamicObject& timeInts = dv_cast<DynamicObject>(output.getAttribute(TMINTA::TIME_INT));
   for (unsigned int timeIntIdx = 0; timeIntIdx < numTimeInt; ++timeIntIdx)
   {
      {
         FactoryResource<DynamicObject> timeIntData;
         timeInts.setAttribute(StringUtilities::toDisplayString(timeIntIdx), *timeIntData.get());
      }
      DynamicObject& timeIntData = dv_cast<DynamicObject>(timeInts.getAttribute(StringUtilities::toDisplayString(timeIntIdx)));

      readField<unsigned int>(input, timeIntData, success, TMINTA::TIME_INTERVAL_INDEX, 6, errorMessage, buf);

      {
         FactoryResource<DynamicObject> startDateData;
         timeIntData.setAttribute(TMINTA::START_TIMESTAMP, *startDateData.get());
         FactoryResource<DynamicObject> endDateData;
         timeIntData.setAttribute(TMINTA::END_TIMESTAMP, *endDateData.get());
      }
      DynamicObject& startDateData = dv_cast<DynamicObject>(timeIntData.getAttribute(TMINTA::START_TIMESTAMP));
      DynamicObject& endDateData = dv_cast<DynamicObject>(timeIntData.getAttribute(TMINTA::END_TIMESTAMP));

      std::string dtgString;
      FactoryResource<DateTime> pStartDate;
      FactoryResource<DateTime> pEndDate;
      double startFrac = 0.0;
      double endFrac = 0.0;
      int startPrecision = 0;
      int endPrecision = 0;
      readAndConvertFromStream(input, dtgString, success, TMINTA::START_TIMESTAMP, 24, errorMessage, buf, true);
      DtgParseCCYYMMDDhhmmssns(dtgString, pStartDate.get(), startFrac, startPrecision);
      startDateData.setAttribute("TIMESTAMP", *pStartDate.get());
      startDateData.setAttribute("FRACTIONAL_SECONDS", startFrac);
      startDateData.setAttribute("PRECISION", startPrecision);

      readAndConvertFromStream(input, dtgString, success, TMINTA::END_TIMESTAMP, 24, errorMessage, buf, true);
      DtgParseCCYYMMDDhhmmssns(dtgString, pEndDate.get(), endFrac, endPrecision);
      endDateData.setAttribute("TIMESTAMP", *pEndDate.get());
      endDateData.setAttribute("FRACTIONAL_SECONDS", endFrac);
      endDateData.setAttribute("PRECISION", endPrecision);
   }

   int64_t numRead = input.tellg();
   if (numRead < 0 || static_cast<uint64_t>(numRead) > std::numeric_limits<size_t>::max() ||
      numRead != static_cast<int64_t>(numBytes))
   {
      numReadErrMsg(numRead, numBytes, errorMessage);
      return false;
   }

   return success;
}

bool Nitf::TmintaParser::fromDynamicObject(const DynamicObject& input, ostream& output, size_t& numBytesWritten,
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
      output << toString(dv_cast<unsigned int>(input.getAttribute(TMINTA::NUM_TIME_INT)), 4);
      const DynamicObject& timeInts = dv_cast<DynamicObject>(input.getAttribute(TMINTA::TIME_INT));
      for (auto timeIntIt = timeInts.begin(); timeIntIt != timeInts.end(); ++timeIntIt)
      {
         const DynamicObject& timeInt = dv_cast<DynamicObject>(timeIntIt->second);
         output << toString(dv_cast<unsigned int>(timeInt.getAttribute(TMINTA::TIME_INTERVAL_INDEX)), 6);
         const DynamicObject& startTimestamp = dv_cast<DynamicObject>(timeInt.getAttribute(TMINTA::START_TIMESTAMP));
         const DateTime& startDatetime = dv_cast<DateTime>(startTimestamp.getAttribute("TIMESTAMP"));
         double startFrac = dv_cast<double>(startTimestamp.getAttribute("FRACTIONAL_SECONDS"));
         int startPrecision = dv_cast<int>(startTimestamp.getAttribute("PRECISION"));
         if (startDatetime.isValid())
         {
            output << startDatetime.getFormattedUtc("%Y%m%d%H%M%S");
            output << "." << toString(startFrac, startPrecision);
            if (startPrecision < 9)
            {
               output << sizeString("-", 9 - startPrecision, '-');
            }
         }
         else
         {
            output << sizeString(" ", 25);
         }
         const DynamicObject& endTimestamp = dv_cast<DynamicObject>(timeInt.getAttribute(TMINTA::END_TIMESTAMP));
         const DateTime& endDatetime = dv_cast<DateTime>(endTimestamp.getAttribute("TIMESTAMP"));
         double endFrac = dv_cast<double>(endTimestamp.getAttribute("FRACTIONAL_SECONDS"));
         int endPrecision = dv_cast<int>(endTimestamp.getAttribute("PRECISION"));
         if (endDatetime.isValid())
         {
            output << endDatetime.getFormattedUtc("%Y%m%d%H%M%S");
            output << "." << toString(endFrac, endPrecision);
            if (endPrecision < 9)
            {
               output << sizeString("-", 9 - endPrecision, '-');
            }
         }
         else
         {
            output << sizeString(" ", 25);
         }
      }
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
