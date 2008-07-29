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
#include "NitfChipConverter.h"
#include "NitfConstants.h"
#include "NitfIchipbParser.h"
#include "NitfUtilities.h"
#include "ObjectResource.h"
#include "RasterDataDescriptor.h"
#include "RasterFileDescriptor.h"

#include <ossim/support_data/ossimNitfIchipbTag.h>


#include <algorithm>
#include <ostream>
#include <sstream>
#include <boost/bind.hpp>
using namespace std;
using namespace Nitf;
using namespace Nitf::TRE;

Nitf::IchipbParser::IchipbParser()
{
   setName("ICHIPB");
   setDescriptorId("{763AE984-BD71-4bae-AFE1-D95007E29312}");
   setSubtype(CreateOnExportSubtype());      // This call is needed for exportMetadata()
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

bool Nitf::IchipbParser::runAllTests(Progress* pProgress, ostream& failure)
{
   static const string data(
      "00"                      // XFRM_FLAG
      "0001.00000"              // SCALE_FACTOR
      "00"                      // ANAMRPH_CORR
      "00"                      // SCANBLK_NUM
      "00000000.500"            // OP_ROW_11
      "00000000.500"            // OP_COL_11
      "00000000.500"            // OP_ROW_12
      "5048.5000000"            // OP_COL_12
      "5048.5000000"            // OP_ROW_21
      "00000000.500"            // OP_COL_21
      "5048.5000000"            // OP_ROW_22
      "5048.5000000"            // OP_COL_22
      "00004096.500"            // FI_ROW_11
      "00001024.500"            // FI_COL_11
      "00004096.500"            // FI_ROW_12
      "6072.5000000"            // FI_COL_12
      "9104.5000000"            // FI_ROW_21
      "00001024.500"            // FI_COL_21
      "9104.5000000"            // FI_ROW_22
      "6072.5000000"            // FI_COL_22
      "00010009"                // FI_ROW
      "00012549"                // FI_COL
      );

   static const string data_error4(
      "00"                      // XFRM_FLAG
      "0001.00000"              // SCALE_FACTOR
      "00"                      // ANAMRPH_CORR
      "0a"                      // SCANBLK_NUM     // ERROR alpha in numeric field
      "00000000.500"            // OP_ROW_11
      "00000000.500"            // OP_COL_11
      "00000000.500"            // OP_ROW_12
      "5048.5000000"            // OP_COL_12
      "5048.5000000"            // OP_ROW_21
      "00000000.500"            // OP_COL_21
      "5048.5000000"            // OP_ROW_22
      "5048.5000000"            // OP_COL_22
      "00004096.500"            // FI_ROW_11
      "00001024.500"            // FI_COL_11
      "00004096.500"            // FI_ROW_12
      "6072.5000000"            // FI_COL_12
      "9104.5000000"            // FI_ROW_21
      "00001024.500"            // FI_COL_21
      "9104.5000000"            // FI_ROW_22
      "6072.5000000"            // FI_COL_22
      "00010009"                // FI_ROW
      "00012549"                // FI_COL
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
      failure << "Error: Negative test: alpha in numeric SCANBLK_NUM field failed: did not return false\n";
      return false;
   }

   treDO->clear();

   return (status != INVALID);
}

bool Nitf::IchipbParser::toDynamicObject(const ossimNitfRegisteredTag& input, DynamicObject& output,
   string &errorMessage) const
{
   const ossimNitfRegisteredTag* pInput = &input;
   const ossimNitfIchipbTag* pTag = PTR_CAST(ossimNitfIchipbTag, pInput);

   if (pTag != NULL)
   {
      return output.setAttribute(ICHIPB::XFRM_FLAG, pTag->getXfrmFlag()) &&
             output.setAttribute(ICHIPB::SCALE_FACTOR, pTag->getScaleFactor()) &&
             output.setAttribute(ICHIPB::ANAMRPH_CORR, pTag->getAnamrphCorrFlag()) &&
             output.setAttribute(ICHIPB::SCANBLK_NUM, pTag->getScanBlock()) &&
             output.setAttribute(ICHIPB::FI_COL_11, pTag->getFiCol11()) &&
             output.setAttribute(ICHIPB::FI_COL_12, pTag->getFiCol12()) &&
             output.setAttribute(ICHIPB::FI_COL_21, pTag->getFiCol21()) &&
             output.setAttribute(ICHIPB::FI_COL_22, pTag->getFiCol22()) &&
             output.setAttribute(ICHIPB::FI_ROW_11, pTag->getFiRow11()) &&
             output.setAttribute(ICHIPB::FI_ROW_12, pTag->getFiRow12()) &&
             output.setAttribute(ICHIPB::FI_ROW_21, pTag->getFiRow21()) &&
             output.setAttribute(ICHIPB::FI_ROW_22, pTag->getFiRow22()) &&
             output.setAttribute(ICHIPB::OP_COL_11, pTag->getOpCol11()) &&
             output.setAttribute(ICHIPB::OP_COL_12, pTag->getOpCol12()) &&
             output.setAttribute(ICHIPB::OP_COL_21, pTag->getOpCol21()) &&
             output.setAttribute(ICHIPB::OP_COL_22, pTag->getOpCol22()) &&
             output.setAttribute(ICHIPB::OP_ROW_11, pTag->getOpRow11()) &&
             output.setAttribute(ICHIPB::OP_ROW_12, pTag->getOpRow12()) &&
             output.setAttribute(ICHIPB::OP_ROW_21, pTag->getOpRow21()) &&
             output.setAttribute(ICHIPB::OP_ROW_22, pTag->getOpRow22()) &&
             output.setAttribute(ICHIPB::FI_ROW, pTag->getFullImageRows()) &&
             output.setAttribute(ICHIPB::FI_COL, pTag->getFullImageCols());
   }

   return false;
}

bool Nitf::IchipbParser::toDynamicObject(istream& input, size_t numBytes, DynamicObject& output,
   string &errorMessage) const
{
#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : This method isn't ever being called.  Should it be removed? (leckels)")
   vector<char> buf;
   bool ok(true);

   bool success(true);

   readField<unsigned int>(input, output, success, ICHIPB::XFRM_FLAG, 2, errorMessage, buf);
   readField<double>(input, output, success, ICHIPB::SCALE_FACTOR, 10, errorMessage, buf);
   readField<unsigned int>(input, output, success, ICHIPB::ANAMRPH_CORR, 2, errorMessage, buf);
   readField<unsigned int>(input, output, success, ICHIPB::SCANBLK_NUM, 2, errorMessage, buf);
   readField<double>(input, output, success, ICHIPB::OP_ROW_11, 12, errorMessage, buf);
   readField<double>(input, output, success, ICHIPB::OP_COL_11, 12, errorMessage, buf);
   readField<double>(input, output, success, ICHIPB::OP_ROW_12, 12, errorMessage, buf);
   readField<double>(input, output, success, ICHIPB::OP_COL_12, 12, errorMessage, buf);
   readField<double>(input, output, success, ICHIPB::OP_ROW_21, 12, errorMessage, buf);
   readField<double>(input, output, success, ICHIPB::OP_COL_21, 12, errorMessage, buf);
   readField<double>(input, output, success, ICHIPB::OP_ROW_22, 12, errorMessage, buf);
   readField<double>(input, output, success, ICHIPB::OP_COL_22, 12, errorMessage, buf);
   readField<double>(input, output, success, ICHIPB::FI_ROW_11, 12, errorMessage, buf);
   readField<double>(input, output, success, ICHIPB::FI_COL_11, 12, errorMessage, buf);
   readField<double>(input, output, success, ICHIPB::FI_ROW_12, 12, errorMessage, buf);
   readField<double>(input, output, success, ICHIPB::FI_COL_12, 12, errorMessage, buf);
   readField<double>(input, output, success, ICHIPB::FI_ROW_21, 12, errorMessage, buf);
   readField<double>(input, output, success, ICHIPB::FI_COL_21, 12, errorMessage, buf);
   readField<double>(input, output, success, ICHIPB::FI_ROW_22, 12, errorMessage, buf);
   readField<double>(input, output, success, ICHIPB::FI_COL_22, 12, errorMessage, buf);
   readField<unsigned int>(input, output, success, ICHIPB::FI_ROW, 8, errorMessage, buf);
   readField<unsigned int>(input, output, success, ICHIPB::FI_COL, 8, errorMessage, buf);

   int numRead = input.tellg();
   if (numRead != numBytes)
   {
      numReadErrMsg(numRead, numBytes, errorMessage);
      return false;
   }

   return success;
}


Nitf::TreState Nitf::IchipbParser::isTreValid(const DynamicObject& tre, ostream& reporter) const
{
   TreState status(VALID);
   set<string>             testSet;
   unsigned int numFields = 0;

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, ICHIPB::XFRM_FLAG, 0U, 99U));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, ICHIPB::SCALE_FACTOR, 0.0, 99999999.999));

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, ICHIPB::ANAMRPH_CORR, 0U, 99U));

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, ICHIPB::SCANBLK_NUM, 0U, 99U));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, ICHIPB::OP_ROW_11, 0.0, 99999999.999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, ICHIPB::OP_COL_11, 0.0, 99999999.999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, ICHIPB::OP_ROW_12, 0.0, 99999999.999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, ICHIPB::OP_COL_12, 0.0, 99999999.999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, ICHIPB::OP_ROW_21, 0.0, 99999999.999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, ICHIPB::OP_COL_21, 0.0, 99999999.999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, ICHIPB::OP_ROW_22, 0.0, 99999999.999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, ICHIPB::OP_COL_22, 0.0, 99999999.999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, ICHIPB::FI_ROW_11, 0.0, 99999999.999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, ICHIPB::FI_COL_11, 0.0, 99999999.999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, ICHIPB::FI_ROW_12, 0.0, 99999999.999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, ICHIPB::FI_COL_12, 0.0, 99999999.999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, ICHIPB::FI_ROW_21, 0.0, 99999999.999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, ICHIPB::FI_COL_21, 0.0, 99999999.999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, ICHIPB::FI_ROW_22, 0.0, 99999999.999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, ICHIPB::FI_COL_22, 0.0, 99999999.999));

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, ICHIPB::FI_ROW, 1U, 99999999U));

   status = MaxState(status, testTagValueRange<unsigned int>(tre, reporter,
      &numFields, ICHIPB::FI_COL, 1U, 99999999U));

   unsigned int totalFields = tre.getNumAttributes();
   if (status != INVALID && totalFields != numFields)
   {
      reporter << "Total fields in the Dynamic Object(" <<
         totalFields <<") did not match the number found(" << numFields << ") ";
      status = INVALID;
   }

   if (status == INVALID)
   {
      reporter << " INVALID fields found in the ICHIPB TAG/SDE\n" ;
   }
   else if (status == SUSPECT)
   {
      reporter << " SUSPECT fields found in the ICHIPB TAG/SDE\n" ;
   }

   return status;
}



bool Nitf::IchipbParser::fromDynamicObject(const DynamicObject& input, ostream& output, size_t& numBytesWritten,
   string &errorMessage) const
{
   size_t sizeIn = max(static_cast<ostream::pos_type>(0), output.tellp());
   size_t sizeOut(sizeIn);

   try
   {
      output <<   toString( dv_cast<bool>(input.getAttribute(ICHIPB::XFRM_FLAG)), 2);
      output <<   toString( dv_cast<double>(input.getAttribute(ICHIPB::SCALE_FACTOR)), 10, 5);
      output <<   toString( dv_cast<bool>(input.getAttribute(ICHIPB::ANAMRPH_CORR)), 2);
      output <<   toString( dv_cast<unsigned int>(input.getAttribute(ICHIPB::SCANBLK_NUM)), 2);
      output <<   toString( dv_cast<double>(input.getAttribute(ICHIPB::OP_ROW_11)), 12, 3);
      output <<   toString( dv_cast<double>(input.getAttribute(ICHIPB::OP_COL_11)), 12, 3);
      output <<   toString( dv_cast<double>(input.getAttribute(ICHIPB::OP_ROW_12)), 12, 3);
      output <<   toString( dv_cast<double>(input.getAttribute(ICHIPB::OP_COL_12)), 12, 3);
      output <<   toString( dv_cast<double>(input.getAttribute(ICHIPB::OP_ROW_21)), 12, 3);
      output <<   toString( dv_cast<double>(input.getAttribute(ICHIPB::OP_COL_21)), 12, 3);
      output <<   toString( dv_cast<double>(input.getAttribute(ICHIPB::OP_ROW_22)), 12, 3);
      output <<   toString( dv_cast<double>(input.getAttribute(ICHIPB::OP_COL_22)), 12, 3);
      output <<   toString( dv_cast<double>(input.getAttribute(ICHIPB::FI_ROW_11)), 12, 3);
      output <<   toString( dv_cast<double>(input.getAttribute(ICHIPB::FI_COL_11)), 12, 3);
      output <<   toString( dv_cast<double>(input.getAttribute(ICHIPB::FI_ROW_12)), 12, 3);
      output <<   toString( dv_cast<double>(input.getAttribute(ICHIPB::FI_COL_12)), 12, 3);
      output <<   toString( dv_cast<double>(input.getAttribute(ICHIPB::FI_ROW_21)), 12, 3);
      output <<   toString( dv_cast<double>(input.getAttribute(ICHIPB::FI_COL_21)), 12, 3);
      output <<   toString( dv_cast<double>(input.getAttribute(ICHIPB::FI_ROW_22)), 12, 3);
      output <<   toString( dv_cast<double>(input.getAttribute(ICHIPB::FI_COL_22)), 12, 3);
      output <<   toString( dv_cast<unsigned int>(input.getAttribute(ICHIPB::FI_ROW)), 8);
      output <<   toString( dv_cast<unsigned int>(input.getAttribute(ICHIPB::FI_COL)), 8);
   }
   catch (const bad_cast&)
   {
      return false;
   }

   sizeOut = output.tellp();
   numBytesWritten = sizeOut - sizeIn;
   return true;
}

TreExportStatus Nitf::IchipbParser::exportMetadata(const RasterDataDescriptor &descriptor, 
   const RasterFileDescriptor &exportDescriptor, DynamicObject &tre, 
   unsigned int & ownerIndex, string & tagType, string &errorMessage) const
{
   const DynamicObject *pMetadata = descriptor.getMetadata();
   VERIFYRV(pMetadata != NULL, REMOVE);
   try
   {

      try
      {
         const DataVariant &nitfMetadata = pMetadata->getAttribute(Nitf::NITF_METADATA);
         const DynamicObject *pExistingIchipb =
            getTagHandle(dv_cast<DynamicObject>(nitfMetadata), "ICHIPB", FindFirst());
         if (pExistingIchipb != NULL)
         {
            tre.merge(pExistingIchipb);
         }

         // Make sure we have at least one of BLOCKA, RPC00A or RPC00B otherwise we don't write an ICHIPB
         const DynamicObject *pExistingBlocka =
            getTagHandle(dv_cast<DynamicObject>(nitfMetadata), "BLOCKA", FindFirst());

         const DynamicObject *pExistingRpc00a =
            getTagHandle(dv_cast<DynamicObject>(nitfMetadata), "RPC00A", FindFirst());

         const DynamicObject *pExistingRpc00b =
            getTagHandle(dv_cast<DynamicObject>(nitfMetadata), "RPC00B", FindFirst());

         if (!pExistingBlocka && !pExistingRpc00a && !pExistingRpc00b)
         {
            return REMOVE;
         }

      }
      catch (const bad_cast&)
      {
         return REMOVE;
      }

      const vector<DimensionDescriptor> &exportRows = exportDescriptor.getRows();
      const vector<DimensionDescriptor> &exportCols = exportDescriptor.getColumns();

      VERIFYRV(!exportRows.empty(), REMOVE);
      VERIFYRV(!exportCols.empty(), REMOVE);

      // getOnDiskNumber() returns pixel index of file being created
      DimensionDescriptor firstRow = exportRows.front();
      DimensionDescriptor firstCol = exportCols.front();
      DimensionDescriptor lastRow = exportRows.back();
      DimensionDescriptor lastCol = exportCols.back();

      VERIFYRV(firstRow.isValid(), REMOVE);
      VERIFYRV(firstCol.isValid(), REMOVE);
      VERIFYRV(lastRow.isValid(), REMOVE);
      VERIFYRV(lastCol.isValid(), REMOVE);

       // will always be 0 + .5 zero based
      tre.setAttribute(Nitf::TRE::ICHIPB::OP_ROW_11, firstRow.getOnDiskNumber() + 0.5);
      tre.setAttribute(Nitf::TRE::ICHIPB::OP_COL_11, firstCol.getOnDiskNumber() + 0.5);

      tre.setAttribute(Nitf::TRE::ICHIPB::OP_ROW_12, firstRow.getOnDiskNumber() + 0.5);
      tre.setAttribute(Nitf::TRE::ICHIPB::OP_COL_12, lastCol.getOnDiskNumber() + 0.5);

      // will be number of rows +.5 zero based
      tre.setAttribute(Nitf::TRE::ICHIPB::OP_ROW_21, lastRow.getOnDiskNumber() + 0.5);
      tre.setAttribute(Nitf::TRE::ICHIPB::OP_COL_21, firstCol.getOnDiskNumber() + 0.5);

      tre.setAttribute(Nitf::TRE::ICHIPB::OP_ROW_22, lastRow.getOnDiskNumber() + 0.5);
      tre.setAttribute(Nitf::TRE::ICHIPB::OP_COL_22, lastCol.getOnDiskNumber() + 0.5);

      Nitf::ChipConverter chipConverter(descriptor);

      vector<LocationType> pixels;
      // getActiveNumber index of cube of as it was loaded.  Original row/col number of subset as loaded from disk.
      // eg. if file on disk is row 0 to 99 but we loaded 9 to 89 (80 rows) (then 0 = 9 on disk, and 79 = 89 on disk)
      pixels.push_back(LocationType(firstCol.getActiveNumber() + 0.5,
         firstRow.getActiveNumber() + 0.5));
      pixels.push_back(LocationType(lastCol.getActiveNumber() + 0.5,
         firstRow.getActiveNumber() + 0.5));
      pixels.push_back(LocationType(firstCol.getActiveNumber() + 0.5,
         lastRow.getActiveNumber() + 0.5));
      pixels.push_back(LocationType(lastCol.getActiveNumber() + 0.5,
         lastRow.getActiveNumber() + 0.5));

      // merge the old ICHIPB into the info to create a new ICHIPB based on the original (pre NITF chippped) image

      // Use old ICHIPB merged with new chip info. transform 4 pixels from getActiveNumber() to original image numbers.
      vector<LocationType> originalPixels;
      transform(pixels.begin(), pixels.end(), back_inserter(originalPixels),
         boost::bind(&ChipConverter::activeToOriginal, &chipConverter,
         _1));

      // originalPixels() row/col number of file on disk based on Original (pre NITF chippped) image.
      // eg if file on disk is row 0 to 99. Original file was 1000 rows and 0 on disk was 100. Then 0 - 99 == 100 - 199
      tre.setAttribute(Nitf::TRE::ICHIPB::FI_ROW_11, originalPixels[0].mY);
      tre.setAttribute(Nitf::TRE::ICHIPB::FI_COL_11, originalPixels[0].mX);

      tre.setAttribute(Nitf::TRE::ICHIPB::FI_ROW_12, originalPixels[1].mY);
      tre.setAttribute(Nitf::TRE::ICHIPB::FI_COL_12, originalPixels[1].mX);

      tre.setAttribute(Nitf::TRE::ICHIPB::FI_ROW_21, originalPixels[2].mY);
      tre.setAttribute(Nitf::TRE::ICHIPB::FI_COL_21, originalPixels[2].mX);

      tre.setAttribute(Nitf::TRE::ICHIPB::FI_ROW_22, originalPixels[3].mY);
      tre.setAttribute(Nitf::TRE::ICHIPB::FI_COL_22, originalPixels[3].mX);

   }
   catch (const string& message)
   {
      errorMessage = message;
      return REMOVE;
   }

   return REPLACE;
}
