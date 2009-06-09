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
#include "DimensionDescriptor.h"
#include "DynamicObject.h"
#include "NitfBlockaParser.h"
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

REGISTER_PLUGIN(OpticksNitfCommonTre, BlockaParser, Nitf::BlockaParser());

Nitf::BlockaParser::BlockaParser()
{
   setName("BLOCKA");
   setDescriptorId("{7B20E698-099C-4385-B027-B8989FD0C97E}");
   setSubtype(CreateOnExportSubtype());      // This call is needed for exportMetadata()
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

bool Nitf::BlockaParser::runAllTests(Progress* pProgress, ostream& failure)
{
   static const string data(
      "01"                      // BLOCK_INSTANCE
      "99999"                   // N_GRAY
      "99999"                   // L_LINES
      "359"                     // LAYOVER_ANGLE
      "359"                     // SHADOW_ANGLE
      "                "        // RESERVED1
      "+12.123456-123.123456"   // FRLC_LOC
      "+00.000000+000.000000"   // LRLC_LOC
      "-90.000000+359.999999"   // LRFC_LOC
      "-90.000000-359.999999"   // FRFC_LOC
      "010.0"                   // RESERVED2
      );

   static const string data_error4(
      "01"                      // BLOCK_INSTANCE
      "99999"                   // N_GRAY
      "99999"                   // L_LINES
      "360"                     // LAYOVER_ANGLE  // Error max == 359
      "359"                     // SHADOW_ANGLE
      "                "        // RESERVED1
      "+12.123456-123.123456"   // FRLC_LOC
      "+00.000000+000.000000"   // LRLC_LOC
      "-90.000000+359.999999"   // LRFC_LOC
      "-90.000000-359.999999"   // FRFC_LOC
      "010.0"                   // RESERVED2
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
         failure << "Error: Negative test with LNSTRT = data out of range failed: did not return SUSPECT\n";
         failure << tmpStream.str();
         treDO->clear();
         return false;
      }
      status = VALID;
   }

   treDO->clear();

   return (status != INVALID);
}

bool Nitf::BlockaParser::toDynamicObject(istream& input, size_t numBytes, DynamicObject& output,
   string &errorMessage) const
{
   vector<char> buf;
   bool ok(true);
   bool success(true);

   readField<unsigned int>(input, output, success, BLOCKA::BLOCK_INSTANCE, 2, errorMessage, buf);
   readField<unsigned int>(input, output, success, BLOCKA::N_GRAY, 5, errorMessage, buf);
   readField<unsigned int>(input, output, success, BLOCKA::L_LINES, 5, errorMessage, buf);
   readField<unsigned int>(input, output, success, BLOCKA::LAYOVER_ANGLE, 3, errorMessage, buf, true);
   readField<unsigned int>(input, output, success, BLOCKA::SHADOW_ANGLE, 3, errorMessage, buf, true);
   readField<string>(input, output, success, BLOCKA::RESERVED1, 16, errorMessage, buf, true);
   readField<string>(input, output, success, BLOCKA::FRLC_LOC, 21, errorMessage, buf, true);
   readField<string>(input, output, success, BLOCKA::LRLC_LOC, 21, errorMessage, buf, true);
   readField<string>(input, output, success, BLOCKA::LRFC_LOC, 21, errorMessage, buf, true);
   readField<string>(input, output, success, BLOCKA::FRFC_LOC, 21, errorMessage, buf, true);
   readField<double>(input, output, success, BLOCKA::RESERVED2, 5, errorMessage, buf);

   size_t numRead = input.tellg();
   if (numRead != numBytes)
   {
      numReadErrMsg(numRead, numBytes, errorMessage);
      return false;
   }

   return success;
}


Nitf::TreState Nitf::BlockaParser::isTreValid(const DynamicObject& tre, ostream& reporter) const
{
   TreState status(VALID);
   set<string> testSet;
   unsigned int numFields = 0;

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, BLOCKA::BLOCK_INSTANCE, 1U, 99U));

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, BLOCKA::N_GRAY, 0U, 99999U));

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, BLOCKA::L_LINES, 1U, 99999U));

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, BLOCKA::LAYOVER_ANGLE, 0U, 359U));

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, BLOCKA::SHADOW_ANGLE, 0U, 359U));

   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, BLOCKA::RESERVED1, testSet, true, true, false));

   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, BLOCKA::FRLC_LOC, testSet, true, true, false));

   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, BLOCKA::LRLC_LOC, testSet, true, true, false));

   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, BLOCKA::LRFC_LOC, testSet, true, true, false));

   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, BLOCKA::FRFC_LOC, testSet, true, true, false));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, BLOCKA::RESERVED2, 10.0F, 10.0F));

   unsigned int totalFields = tre.getNumAttributes();
   if (status != INVALID && totalFields != numFields)
   {
      reporter << "Total fields in the Dynamic Object(" <<
         totalFields << ") did not match the number found(" << numFields << ") ";
      status = INVALID;
   }

   if (status == INVALID)
   {
      reporter << " INVALID fields found in the BLOCKA TAG/SDE\n" ;
   }
   else if (status == SUSPECT)
   {
      reporter << " SUSPECT fields found in the BLOCKA TAG/SDE\n" ;
   }

   return status;
}

bool Nitf::BlockaParser::fromDynamicObject(const DynamicObject& input, ostream& output, size_t& numBytesWritten,
   string &errorMessage) const
{
   size_t sizeIn = max(static_cast<ostream::pos_type>(0), output.tellp());
   size_t sizeOut(sizeIn);

   try
   {
      output << toString(dv_cast<unsigned int>(input.getAttribute(BLOCKA::BLOCK_INSTANCE)), 2, -1);
      output << toString(dv_cast<unsigned int>(input.getAttribute(BLOCKA::N_GRAY)), 5, -1);
      output << toString(dv_cast<unsigned int>(input.getAttribute(BLOCKA::L_LINES)), 5, -1);
      output << toString(dv_cast<unsigned int>(input.getAttribute(BLOCKA::LAYOVER_ANGLE)), 3, -1);
      output << toString(dv_cast<unsigned int>(input.getAttribute(BLOCKA::SHADOW_ANGLE)), 3, -1);
      output << sizeString(dv_cast<string>(input.getAttribute(BLOCKA::RESERVED1)), 16);
      output << sizeString(dv_cast<string>(input.getAttribute(BLOCKA::FRLC_LOC)), 21);
      output << sizeString(dv_cast<string>(input.getAttribute(BLOCKA::LRLC_LOC)), 21);
      output << sizeString(dv_cast<string>(input.getAttribute(BLOCKA::LRFC_LOC)), 21);
      output << sizeString(dv_cast<string>(input.getAttribute(BLOCKA::FRFC_LOC)), 21);
      output << toString(dv_cast<double>(input.getAttribute(BLOCKA::RESERVED2)), 5, 1);
   }
   catch (const bad_cast&)
   {
      return false;
   }

   sizeOut = output.tellp();
   numBytesWritten = sizeOut - sizeIn;
   return true;
}

TreExportStatus Nitf::BlockaParser::exportMetadata(const RasterDataDescriptor &descriptor, 
   const RasterFileDescriptor &exportDescriptor, DynamicObject &tre, 
   unsigned int & ownerIndex, string & tagType, string &errorMessage) const

{
   // Rules for BLOCKA:
   // 1) BLOCKA can only be used if the original image referenced by it is less than 100,000 lines long.
   // 2) If there is an existing BLOCKA then don't change it. Add or update the ICHIPB TAG when chipping.
   // 3) If BLOCKA does not exist and there exists an RPC (A or B) then do not add a BLOCKA.
   // 4) If BLOCKA and RPC A or B does not exist then adding a BLOCKA is optional.
   //     a) When chipping we have the choice to add the ICHIPB with the BLOCKA in reference to the original image or
   //          just the BLOCKA in reference to the chip.
   //     b) When not chipping then adding a BLOCKA is optional to get  more accurate corner coordinates.


   const DynamicObject* pMetadata = descriptor.getMetadata();
   VERIFYRV(pMetadata != NULL, REMOVE);
   try
   {

      try
      {
         const DataVariant& nitfMetadata = pMetadata->getAttribute(Nitf::NITF_METADATA);
         const DynamicObject* pExistingBlocka =
            getTagHandle(dv_cast<DynamicObject>(nitfMetadata), "BLOCKA", FindFirst());

         const DynamicObject* pExistingRpc00a =
            getTagHandle(dv_cast<DynamicObject>(nitfMetadata), "RPC00A", FindFirst());

         const DynamicObject* pExistingRpc00b =
            getTagHandle(dv_cast<DynamicObject>(nitfMetadata), "RPC00B", FindFirst());

         if (pExistingBlocka || pExistingRpc00a || pExistingRpc00b)
         {
            return UNCHANGED;
         }
      }
      catch (const bad_cast&)
      {
         return REMOVE;
      }

      const vector<DimensionDescriptor>& exportRows = exportDescriptor.getRows();
      VERIFYRV(!exportRows.empty(), REMOVE);

      DimensionDescriptor lastRow = exportRows.back();
      unsigned int numRows = lastRow.getOnDiskNumber();
      if (numRows >= 100000)
      {
         errorMessage = "Max rows >= 100,000. Could not build BLOCKA TRE\n";
         return REMOVE;
      }


      // Get the RasterElement out of ModelServices

      Service<ModelServices> pModel;
      RasterElement* pRaster = dynamic_cast<RasterElement*>
         (pModel->getElement(descriptor.getName(), descriptor.getType(), descriptor.getParent()));
      VERIFYRV(pRaster != NULL, REMOVE);

      if (!pRaster->isGeoreferenced())
      {
         return REMOVE; // not georeferenced, can’t find corner coords
      }

      // Find the active numbers of the exported corners.  Get the DimensionDescriptors
      const vector<DimensionDescriptor>& exportCols = exportDescriptor.getColumns();
      VERIFYRV(!exportCols.empty(), REMOVE);

      // getOnDiskNumber() returns pixel index of file being created
      DimensionDescriptor firstRow = exportRows.front();
      DimensionDescriptor firstCol = exportCols.front();
      DimensionDescriptor lastCol = exportCols.back();

      LocationType ulGeo = pRaster->convertPixelToGeocoord
         (LocationType(firstCol.getActiveNumber(), firstRow.getActiveNumber()));

      LocationType llGeo = pRaster->convertPixelToGeocoord
         (LocationType(firstCol.getActiveNumber(), lastRow.getActiveNumber()));

      LocationType urGeo = pRaster->convertPixelToGeocoord
         (LocationType(lastCol.getActiveNumber(), firstRow.getActiveNumber()));

      LocationType lrGeo = pRaster->convertPixelToGeocoord(
         LocationType(lastCol.getActiveNumber(), lastRow.getActiveNumber()));

      string blank;

      // The BLOCKA corners are not in the same order as the IGEOLO. They shoud map in the order:
      // IGEOLO 1 ul mapped to BLOCKA 4 FRFC
      // IGEOLO 2 ur mapped to BLOCKA 1 FRLC
      // IGEOLO 3 lr mapped to BLOCKA 2 LRLC
      // IGEOLO 4 ll mapped to BLOCKA 3 LRFC

      // llGeo.mX is lat, llGeo.mY is lon

      string ll_lat = toString(llGeo.mX, 10, 6, ZERO_FILL, POS_SIGN_TRUE);
      string ll_long = toString(llGeo.mY, 11, 6, ZERO_FILL, POS_SIGN_TRUE);
      string ll_latlong = ll_lat + ll_long;

      string lr_lat = toString(lrGeo.mX, 10, 6, ZERO_FILL, POS_SIGN_TRUE);
      string lr_long = toString(lrGeo.mY, 11, 6, ZERO_FILL, POS_SIGN_TRUE);
      string lr_latlong = lr_lat + lr_long;

      string ul_lat = toString(ulGeo.mX, 10, 6, ZERO_FILL, POS_SIGN_TRUE);
      string ul_long = toString(ulGeo.mY, 11, 6, ZERO_FILL, POS_SIGN_TRUE);
      string ul_latlong = ul_lat + ul_long;

      string ur_lat = toString(urGeo.mX, 10, 6, ZERO_FILL, POS_SIGN_TRUE);
      string ur_long = toString(urGeo.mY, 11, 6, ZERO_FILL, POS_SIGN_TRUE);
      string ur_latlong = ur_lat + ur_long;

      tre.setAttribute(Nitf::TRE::BLOCKA::BLOCK_INSTANCE, 1U);
      tre.setAttribute(Nitf::TRE::BLOCKA::N_GRAY, 0U);
      tre.setAttribute(Nitf::TRE::BLOCKA::L_LINES, numRows);            // Num image rows < 100,000
      tre.setAttribute(Nitf::TRE::BLOCKA::LAYOVER_ANGLE, 0U);
      tre.setAttribute(Nitf::TRE::BLOCKA::SHADOW_ANGLE, 0U);
      tre.setAttribute(Nitf::TRE::BLOCKA::RESERVED1, blank);
      tre.setAttribute(Nitf::TRE::BLOCKA::FRLC_LOC, ur_latlong);
      tre.setAttribute(Nitf::TRE::BLOCKA::LRLC_LOC, lr_latlong);
      tre.setAttribute(Nitf::TRE::BLOCKA::LRFC_LOC, ll_latlong);
      tre.setAttribute(Nitf::TRE::BLOCKA::FRFC_LOC, ul_latlong);

      double tempD(10.0F);
      tre.setAttribute(Nitf::TRE::BLOCKA::RESERVED2, tempD);

   }
   catch (const string& message)
   {
      errorMessage = message;
      return REMOVE;
   }

   return REPLACE;
}
