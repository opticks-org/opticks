/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "assert.h"
#include "DataAccessor.h"
#include "DataAccessorImpl.h"
#include "Executable.h"
#include "PlugInArgList.h"
#include "PlugInResource.h"
#include "RasterDataDescriptor.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "TestBedTestUtilities.h"
#include "TestCase.h"
#include "TestSuiteNewSession.h"
#include "TypesFile.h"

#include <limits>
#include <math.h>
#include <string.h>

using namespace std;

class PrincipalComponentAnalysisTestCase : public TestCase
{
public:
   PrincipalComponentAnalysisTestCase() : TestCase( "PrincipalComponentAnalysis" ) {}
   bool run()
   {
      bool success = true;
      RasterElement* pElement = TestUtilities::getStandardRasterElement(false, true);
      issearf(pElement != NULL);

      RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(pElement->getDataDescriptor());
      issearf(pDescriptor != NULL);

      ExecutableResource pPlugIn("Principal Component Analysis");
      issearf(pPlugIn.get() != NULL);

      PlugInArgList& argsIn = pPlugIn->getInArgList();
      string transformType = "Second Moment";
      issearf(argsIn.setPlugInArgValue<string>("Transform Type", &transformType));

      int components = static_cast<int>(pDescriptor->getBandCount());
      issearf(argsIn.setPlugInArgValue<int>("Components", &components));

      EncodingType outputEncodingType = INT1UBYTE;
      issearf(argsIn.setPlugInArgValue<EncodingType>("Output Encoding Type", &outputEncodingType));

      int maxScaleValue = static_cast<int>(numeric_limits<unsigned char>::max());
      issearf(argsIn.setPlugInArgValue<int>("Max Scale Value", &maxScaleValue));
      issearf(pPlugIn->execute());

      PlugInArgList& argsOut = pPlugIn->getOutArgList();
      RasterElement* pElementOut = argsOut.getPlugInArgValue<RasterElement>("Corrected Data Cube");
      issearf(pElementOut != NULL);

      RasterDataDescriptor* pDescriptorOut = dynamic_cast<RasterDataDescriptor*>(pElementOut->getDataDescriptor());
      issearf(pDescriptorOut != NULL);
      issearf(pDescriptorOut->getRowCount() == 181);
      issearf(pDescriptorOut->getColumnCount() == 97);

      // A sample of expected values which should have been computed. This sample includes only the first 5 rows.
      const unsigned char pExpectedData[] = {
        168, 125, 64, 103, 30, 205, 133, 166, 179, 109, 82, 168, 101, 13, 171, 86, 38, 87, 54, 126, 151, 125, 149, 128,
        95, 139, 138, 108, 113, 138, 92, 127, 133, 151, 85, 141, 157, 138, 72, 122, 116, 103, 98, 130, 165, 112, 135,
        176, 151, 92, 134, 129, 163, 146, 161, 141, 120, 174, 110, 150, 99, 137, 108, 181, 142, 116, 65, 182, 163, 123,
        153, 142, 140, 154, 162, 132, 119, 58, 159, 95, 107, 75, 137, 150, 105, 120, 128, 104, 188, 168, 97, 105, 105,
        147, 107, 122, 124, 168, 124, 66, 93, 31, 204, 139, 158, 201, 84, 137, 167, 151, 149, 150, 138, 89, 101, 66,
        94, 186, 114, 137, 98, 81, 122, 91, 82, 93, 121, 123, 109, 143, 153, 130, 139, 157, 152, 73, 137, 120, 106, 86,
        133, 150, 135, 114, 169, 146, 53, 113, 130, 142, 137, 120, 99, 115, 160, 96, 126, 111, 132, 86, 146, 147, 134,
        84, 128, 118, 120, 100, 145, 143, 152, 195, 133, 163, 74, 144, 85, 108, 65, 181, 161, 115, 129, 125, 137, 186,
        133, 83, 97, 117, 115, 133, 91, 131, 168, 125, 65, 97, 33, 207, 125, 151, 194, 75, 51, 175, 147, 153, 166, 126,
        90, 112, 70, 119, 143, 156, 124, 153, 92, 130, 124, 120, 125, 139, 140, 102, 165, 170, 179, 153, 177, 143, 65,
        122, 112, 93, 83, 139, 153, 113, 147, 174, 136, 74, 136, 157, 170, 137, 138, 135, 110, 150, 74, 122, 158, 164,
        110, 93, 140, 111, 133, 160, 153, 107, 102, 135, 140, 170, 140, 107, 135, 64, 160, 58, 101, 75, 178, 127, 116,
        123, 119, 97, 176, 154, 94, 127, 117, 116, 129, 170, 135, 170, 125, 66, 96, 32, 205, 135, 153, 197, 76, 121,
        162, 163, 157, 181, 105, 85, 105, 80, 131, 182, 166, 153, 145, 103, 107, 107, 116, 116, 165, 147, 113, 174,
        175, 143, 160, 197, 140, 90, 123, 125, 140, 70, 133, 160, 137, 149, 190, 136, 73, 128, 162, 165, 146, 138,
        127, 104, 150, 95, 123, 117, 117, 105, 131, 139, 137, 115, 128, 145, 109, 117, 136, 137, 156, 178, 131, 124,
        62, 155, 88, 89, 56, 145, 129, 90, 158, 124, 138, 198, 163, 99, 91, 106, 139, 123, 130, 131, 170, 126, 65, 97,
        31, 208, 127, 151, 194, 87, 72, 179, 160, 148, 188, 125, 97, 102, 61, 103, 145, 141, 100, 120, 141, 113, 97,
        148, 150, 162, 93, 98, 177, 159, 89, 108, 174, 143, 94, 145, 120, 114, 91, 128, 143, 121, 122, 149, 105, 59,
        123, 125, 177, 144, 158, 148, 109, 130, 59, 149, 113, 135, 108, 97, 145, 164, 128, 134, 141, 125, 118, 141,
        137, 154, 176, 110, 130, 51, 169, 80, 113, 68, 157, 137, 76, 136, 102, 157, 175, 167, 103, 108, 107, 119, 135,
        129, 151 };

      DataAccessor dataAccessor = pElementOut->getDataAccessor();
      const unsigned int numElementsToTest = sizeof(pExpectedData) / sizeof(pExpectedData[0]);
      const unsigned int numColsToTest = pDescriptorOut->getColumnCount();
      const unsigned int numRowsToTest = numElementsToTest / numColsToTest;
      for (unsigned int row = 0; row < numRowsToTest; ++row)
      {
         const unsigned int rowOffset = row * numColsToTest;
         issearf(dataAccessor.isValid());
         unsigned char* pData = reinterpret_cast<unsigned char*>(dataAccessor->getRow());
         issearf(pData != NULL);

         issearf(memcmp(pData, &pExpectedData[rowOffset], numColsToTest * sizeof(pExpectedData[0])) == 0);
         dataAccessor->nextRow();
      }

      return success;
   }
};

class PrincipalComponentAnalysisTestSuite : public TestSuiteNewSession
{
public:
   PrincipalComponentAnalysisTestSuite() : TestSuiteNewSession( "PrincipalComponentAnalysis" )
   {
      addTestCase( new PrincipalComponentAnalysisTestCase );
   }
};

REGISTER_SUITE( PrincipalComponentAnalysisTestSuite )
