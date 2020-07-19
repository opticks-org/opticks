/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "assert.h"
#include "ConfigurationSettings.h"
#include "DesktopServices.h"
#include "Executable.h"
#include "ImportDescriptor.h"
#include "ModelServicesImp.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInResource.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "SpatialDataWindow.h"
#include "SpatialDataView.h"
#include "TestBedTestUtilities.h"
#include "TestSuiteNewSession.h"
#include "TestUtilities.h"

#include <fstream>

using namespace std;

RasterElement* runBandMath( std::string filename1, std::string filename2, std::string expression, bool onDisk );

class BandMathAverageBandsTest : public TestCase
{
public:
   BandMathAverageBandsTest() : TestCase("AverageBands") {}

   bool run()
   {
      bool success = true;
      RasterElement* pSourceRasterElement = TestUtilities::getStandardRasterElement();
      issearf(pSourceRasterElement != NULL);

      bool useDegrees = false;
      bool displayResults = true;
      string expression = "(b45+b46+b47+b48)/4"; // ACTIVE NUMBERS - must match Band Binning file contents below.

      ExecutableResource pBandMath("Band Math", "", NULL, true);
      issearf(pBandMath.get() != NULL);
      issearf(pBandMath->getInArgList().setPlugInArgValue(Executable::DataElementArg(), pSourceRasterElement));
      issearf(pBandMath->getInArgList().setPlugInArgValue("Degrees", &useDegrees));
      issearf(pBandMath->getInArgList().setPlugInArgValue("Display Results", &displayResults));
      issearf(pBandMath->getInArgList().setPlugInArgValue("Input Expression", &expression));
      issearf(pBandMath->execute());

      RasterElement* pBandMathResults = pBandMath->getOutArgList().getPlugInArgValue<RasterElement>("Band Math Result");
      issearf(pBandMathResults != NULL);
      RasterDataDescriptor* pDataDescriptor =
         dynamic_cast<RasterDataDescriptor*>(pBandMathResults->getDataDescriptor());
      issearf(pDataDescriptor != NULL);
      issearf(pDataDescriptor->getBandCount() == 1);

      // verify pixel 37, 87
      issearf(pBandMathResults->getPixelValue(pDataDescriptor->getActiveColumn(37),
         pDataDescriptor->getActiveRow(87), pDataDescriptor->getActiveBand(0), COMPLEX_MAGNITUDE) == 5698.5);

      // verify pixel 82, 152
      issearf(pBandMathResults->getPixelValue(pDataDescriptor->getActiveColumn(82),
         pDataDescriptor->getActiveRow(152), pDataDescriptor->getActiveBand(0), COMPLEX_MAGNITUDE) == 6549);

      // Run Band Binning and compare results.
      FactoryResource<Filename> pFilename;
      pFilename->setFullPathAndName(ConfigurationSettings::getSettingTempPath()->getFullPathAndName() +
         "/AverageBands.txt");
      ofstream file(pFilename->getFullPathAndName().c_str());
      issearf(file.good());
      file << "48 51" << endl;   // ORIGINAL NUMBERS - must match expression for BandMath above.
      file.close();

      ExecutableResource pBandBinning("Band Binning", "", NULL, true);
      issearf(pBandBinning.get() != NULL);
      issearf(pBandBinning->getInArgList().setPlugInArgValue(Executable::DataElementArg(), pSourceRasterElement));
      issearf(pBandBinning->getInArgList().setPlugInArgValue("Filename", pFilename.get()));
      issearf(pBandBinning->execute());
      remove(pFilename->getFullPathAndName().c_str());

      RasterElement* pBandBinningResults =
         pBandBinning->getOutArgList().getPlugInArgValue<RasterElement>(Executable::DataElementArg());
      issearf(pBandBinningResults != NULL);
      pDataDescriptor = dynamic_cast<RasterDataDescriptor*>(pBandBinningResults->getDataDescriptor());
      issearf(pDataDescriptor != NULL);
      issearf(pDataDescriptor->getBandCount() == 1);

      // verify pixel 37, 87: truncation is intentional because the output data type matches the input data type
      issearf(pBandBinningResults->getPixelValue(pDataDescriptor->getActiveColumn(37),
         pDataDescriptor->getActiveRow(87), pDataDescriptor->getActiveBand(0), COMPLEX_MAGNITUDE) == 5698); // truncated

      // verify pixel 82, 152
      issearf(pBandBinningResults->getPixelValue(pDataDescriptor->getActiveColumn(82),
         pDataDescriptor->getActiveRow(152), pDataDescriptor->getActiveBand(0), COMPLEX_MAGNITUDE) == 6549);

      return success;
   }
};

class BandMathAddBandsTest : public TestCase
{
public:
   BandMathAddBandsTest() : TestCase( "AddBands" ) {}

   bool run()
   {
      bool success = true;
      string filename1 = TestUtilities::getTestDataPath() + "BandMath/ir1pad.sio";
      string filename2 = "";
      string expression = "b2+b5+b6";

      RasterElement *pNewRasterElement = NULL;
      pNewRasterElement = runBandMath( filename1, filename2, expression, false );
      issea( pNewRasterElement != NULL );

      RasterDataDescriptor *pDataDescriptor = NULL;
      pDataDescriptor = dynamic_cast<RasterDataDescriptor*>( pNewRasterElement->getDataDescriptor() );
      issea( pDataDescriptor != NULL );

      issea( pDataDescriptor->getBandCount() == 1 );

      // verify pixel 412,105
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 412 ), pDataDescriptor->getActiveRow( 105 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) == 506 );

      // verify pixel 37,185
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 37 ), pDataDescriptor->getActiveRow( 185 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) == 466 );

      // verify pixel 173,247
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 173 ), pDataDescriptor->getActiveRow( 247 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) == 464 );

      // verify pixel 425,392
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 425 ), pDataDescriptor->getActiveRow( 392 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) == 447 );

      // verify pixel 72,460
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 72 ), pDataDescriptor->getActiveRow( 460 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) == 443 );

      issea(TestUtilities::destroyWorkspaceWindow(dynamic_cast<WorkspaceWindow*>(
         Service<DesktopServices>()->getWindow(pDataDescriptor->getName(), SPATIAL_DATA_WINDOW))));

      return success;
   }
};

class BandMathMultiplyAndSubtractBandsTest : public TestCase
{
public:
   BandMathMultiplyAndSubtractBandsTest() : TestCase( "MultiplyAndSubtractBands" ) {}

   bool run()
   {
      bool success = true;
      string filename1 = TestUtilities::getTestDataPath() + "BandMath/mi1clouds.sio";
      string filename2 = "";
      string expression = "b1*b4-b7";

      RasterElement *pNewRasterElement = NULL;
      pNewRasterElement = runBandMath( filename1, filename2, expression, false );
      issea( pNewRasterElement != NULL );

      RasterDataDescriptor *pDataDescriptor = NULL;
      pDataDescriptor = dynamic_cast<RasterDataDescriptor*>( pNewRasterElement->getDataDescriptor() );
      issea( pDataDescriptor != NULL );

      issea( pDataDescriptor->getBandCount() == 1 );

      // verify pixel 412,105
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 412 ), pDataDescriptor->getActiveRow( 105 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) == 845 );

      // verify pixel 37,185
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 37 ), pDataDescriptor->getActiveRow( 185 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) == 9630 );

      // verify pixel 173,247
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 173 ), pDataDescriptor->getActiveRow( 247 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) == 2928 );

      // verify pixel 425,392
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 425 ), pDataDescriptor->getActiveRow( 392 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) == 9237 );

      // verify pixel 72,460
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 72 ), pDataDescriptor->getActiveRow( 460 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) == 33334 );

      issea(TestUtilities::destroyWorkspaceWindow(dynamic_cast<WorkspaceWindow*>(
         Service<DesktopServices>()->getWindow(pDataDescriptor->getName(), SPATIAL_DATA_WINDOW))));

      return success;
   }
};

class BandMathAddCubesTest : public TestCase
{
public:
   BandMathAddCubesTest() : TestCase( "AddCubes" ) {}

   bool run()
   {
      bool success = true;
      string filename1 = TestUtilities::getTestDataPath() + "BandMath/mi1clouds.sio";
      string filename2 = TestUtilities::getTestDataPath() + "BandMath/mi2clouds.sio";
      string expression = "c1+c2";

      RasterElement *pNewRasterElement = NULL;
      pNewRasterElement = runBandMath( filename1, filename2, expression, false );
      issea( pNewRasterElement != NULL );

      RasterDataDescriptor *pDataDescriptor = NULL;
      pDataDescriptor = dynamic_cast<RasterDataDescriptor*>( pNewRasterElement->getDataDescriptor() );
      issea( pDataDescriptor != NULL );

      issea( pDataDescriptor->getBandCount() == 7 );

      // verify pixel 412,105
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 412 ), pDataDescriptor->getActiveRow( 105 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) == 166 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 412 ), pDataDescriptor->getActiveRow( 105 ), pDataDescriptor->getActiveBand( 1 ), COMPLEX_MAGNITUDE ) == 61 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 412 ), pDataDescriptor->getActiveRow( 105 ), pDataDescriptor->getActiveBand( 2 ), COMPLEX_MAGNITUDE ) == 51 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 412 ), pDataDescriptor->getActiveRow( 105 ), pDataDescriptor->getActiveBand( 3 ), COMPLEX_MAGNITUDE ) == 27 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 412 ), pDataDescriptor->getActiveRow( 105 ), pDataDescriptor->getActiveBand( 4 ), COMPLEX_MAGNITUDE ) == 24 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 412 ), pDataDescriptor->getActiveRow( 105 ), pDataDescriptor->getActiveBand( 5 ), COMPLEX_MAGNITUDE ) == 295 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 412 ), pDataDescriptor->getActiveRow( 105 ), pDataDescriptor->getActiveBand( 6 ), COMPLEX_MAGNITUDE ) == 10 );

      // verify pixel 37,185
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 37 ), pDataDescriptor->getActiveRow( 185 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) == 176 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 37 ), pDataDescriptor->getActiveRow( 185 ), pDataDescriptor->getActiveBand( 1 ), COMPLEX_MAGNITUDE ) == 72 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 37 ), pDataDescriptor->getActiveRow( 185 ), pDataDescriptor->getActiveBand( 2 ), COMPLEX_MAGNITUDE ) == 71 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 37 ), pDataDescriptor->getActiveRow( 185 ), pDataDescriptor->getActiveBand( 3 ), COMPLEX_MAGNITUDE ) == 232 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 37 ), pDataDescriptor->getActiveRow( 185 ), pDataDescriptor->getActiveBand( 4 ), COMPLEX_MAGNITUDE ) == 220 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 37 ), pDataDescriptor->getActiveRow( 185 ), pDataDescriptor->getActiveBand( 5 ), COMPLEX_MAGNITUDE ) == 292 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 37 ), pDataDescriptor->getActiveRow( 185 ), pDataDescriptor->getActiveBand( 6 ), COMPLEX_MAGNITUDE ) == 77 );

      // verify pixel 173,247
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 173 ), pDataDescriptor->getActiveRow( 247 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) == 227 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 173 ), pDataDescriptor->getActiveRow( 247 ), pDataDescriptor->getActiveBand( 1 ), COMPLEX_MAGNITUDE ) == 98 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 173 ), pDataDescriptor->getActiveRow( 247 ), pDataDescriptor->getActiveBand( 2 ), COMPLEX_MAGNITUDE ) == 126 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 173 ), pDataDescriptor->getActiveRow( 247 ), pDataDescriptor->getActiveBand( 3 ), COMPLEX_MAGNITUDE ) == 112 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 173 ), pDataDescriptor->getActiveRow( 247 ), pDataDescriptor->getActiveBand( 4 ), COMPLEX_MAGNITUDE ) == 166 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 173 ), pDataDescriptor->getActiveRow( 247 ), pDataDescriptor->getActiveBand( 5 ), COMPLEX_MAGNITUDE ) == 303 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 173 ), pDataDescriptor->getActiveRow( 247 ), pDataDescriptor->getActiveBand( 6 ), COMPLEX_MAGNITUDE ) == 105 );

      // verify pixel 425,392
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 425 ), pDataDescriptor->getActiveRow( 392 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) == 244 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 425 ), pDataDescriptor->getActiveRow( 392 ), pDataDescriptor->getActiveBand( 1 ), COMPLEX_MAGNITUDE ) == 106 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 425 ), pDataDescriptor->getActiveRow( 392 ), pDataDescriptor->getActiveBand( 2 ), COMPLEX_MAGNITUDE ) == 140 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 425 ), pDataDescriptor->getActiveRow( 392 ), pDataDescriptor->getActiveBand( 3 ), COMPLEX_MAGNITUDE ) == 157 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 425 ), pDataDescriptor->getActiveRow( 392 ), pDataDescriptor->getActiveBand( 4 ), COMPLEX_MAGNITUDE ) == 239 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 425 ), pDataDescriptor->getActiveRow( 392 ), pDataDescriptor->getActiveBand( 5 ), COMPLEX_MAGNITUDE ) == 332 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 425 ), pDataDescriptor->getActiveRow( 392 ), pDataDescriptor->getActiveBand( 6 ), COMPLEX_MAGNITUDE ) == 127 );

      // verify pixel 72,460
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 72 ), pDataDescriptor->getActiveRow( 460 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) == 320 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 72 ), pDataDescriptor->getActiveRow( 460 ), pDataDescriptor->getActiveBand( 1 ), COMPLEX_MAGNITUDE ) == 138 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 72 ), pDataDescriptor->getActiveRow( 460 ), pDataDescriptor->getActiveBand( 2 ), COMPLEX_MAGNITUDE ) == 169 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 72 ), pDataDescriptor->getActiveRow( 460 ), pDataDescriptor->getActiveBand( 3 ), COMPLEX_MAGNITUDE ) == 252 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 72 ), pDataDescriptor->getActiveRow( 460 ), pDataDescriptor->getActiveBand( 4 ), COMPLEX_MAGNITUDE ) == 284 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 72 ), pDataDescriptor->getActiveRow( 460 ), pDataDescriptor->getActiveBand( 5 ), COMPLEX_MAGNITUDE ) == 267 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 72 ), pDataDescriptor->getActiveRow( 460 ), pDataDescriptor->getActiveBand( 6 ), COMPLEX_MAGNITUDE ) == 124 );

      issea(TestUtilities::destroyWorkspaceWindow(dynamic_cast<WorkspaceWindow*>(
         Service<DesktopServices>()->getWindow(pDataDescriptor->getName(), SPATIAL_DATA_WINDOW))));

      return success;
   }
};

class BandMathComplexExpressionTest : public TestCase
{
public:
   BandMathComplexExpressionTest() : TestCase( "ComplexExpression" ) {}

   bool run()
   {
      bool success = true;
      string filename1 = TestUtilities::getTestDataPath() + "BandMath/ir1pad.sio";
      string filename2 = TestUtilities::getTestDataPath() + "BandMath/mi2clouds.sio";
      string expression = "(c1*c2)-c2/(c1*cos(c1))";

      RasterElement *pNewRasterElement = NULL;
      pNewRasterElement = runBandMath( filename1, filename2, expression, false );
      issea( pNewRasterElement != NULL );

      RasterDataDescriptor *pDataDescriptor = NULL;
      pDataDescriptor = dynamic_cast<RasterDataDescriptor*>( pNewRasterElement->getDataDescriptor() );
      issea( pDataDescriptor != NULL );

      issea( pDataDescriptor->getBandCount() == 7 );

      // verify pixel 412,105      
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 412 ), pDataDescriptor->getActiveRow( 105 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) - 17533.7460577308 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 412 ), pDataDescriptor->getActiveRow( 105 ), pDataDescriptor->getActiveBand( 1 ), COMPLEX_MAGNITUDE ) - 3466.3043117297 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 412 ), pDataDescriptor->getActiveRow( 105 ), pDataDescriptor->getActiveBand( 2 ), COMPLEX_MAGNITUDE ) - 4709.8083097297 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 412 ), pDataDescriptor->getActiveRow( 105 ), pDataDescriptor->getActiveBand( 3 ), COMPLEX_MAGNITUDE ) - 1952.1513065532 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 412 ), pDataDescriptor->getActiveRow( 105 ), pDataDescriptor->getActiveBand( 4 ), COMPLEX_MAGNITUDE ) - 3707.6095164149 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 412 ), pDataDescriptor->getActiveRow( 105 ), pDataDescriptor->getActiveBand( 5 ), COMPLEX_MAGNITUDE ) - 27689.2535996449 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 412 ), pDataDescriptor->getActiveRow( 105 ), pDataDescriptor->getActiveBand( 6 ), COMPLEX_MAGNITUDE ) - 1047.8954676812 ) < 1e-3 );

      // verify pixel 37,185
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 37 ), pDataDescriptor->getActiveRow( 185 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) - 14253.3692173012 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 37 ), pDataDescriptor->getActiveRow( 185 ), pDataDescriptor->getActiveBand( 1 ), COMPLEX_MAGNITUDE ) - 3044.2939032654 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 37 ), pDataDescriptor->getActiveRow( 185 ), pDataDescriptor->getActiveBand( 2 ), COMPLEX_MAGNITUDE ) - 4122.5290042618 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 37 ), pDataDescriptor->getActiveRow( 185 ), pDataDescriptor->getActiveBand( 3 ), COMPLEX_MAGNITUDE ) - 12689.2389022649 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 37 ), pDataDescriptor->getActiveRow( 185 ), pDataDescriptor->getActiveBand( 4 ), COMPLEX_MAGNITUDE ) - 16468.5152566560 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 37 ), pDataDescriptor->getActiveRow( 185 ), pDataDescriptor->getActiveBand( 5 ), COMPLEX_MAGNITUDE ) - 27798.5734450281 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 37 ), pDataDescriptor->getActiveRow( 185 ), pDataDescriptor->getActiveBand( 6 ), COMPLEX_MAGNITUDE ) - 3159.3186992462 ) < 1e-3 );

      // verify pixel 173,247
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 173 ), pDataDescriptor->getActiveRow( 247 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) - 21912.9083010237 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 173 ), pDataDescriptor->getActiveRow( 247 ), pDataDescriptor->getActiveBand( 1 ), COMPLEX_MAGNITUDE ) - 5479.7290258777 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 173 ), pDataDescriptor->getActiveRow( 247 ), pDataDescriptor->getActiveBand( 2 ), COMPLEX_MAGNITUDE ) - 11701.8849007175 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 173 ), pDataDescriptor->getActiveRow( 247 ), pDataDescriptor->getActiveBand( 3 ), COMPLEX_MAGNITUDE ) - 8344.0053396985 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 173 ), pDataDescriptor->getActiveRow( 247 ), pDataDescriptor->getActiveBand( 4 ), COMPLEX_MAGNITUDE ) - 24975.8265772064 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 173 ), pDataDescriptor->getActiveRow( 247 ), pDataDescriptor->getActiveBand( 5 ), COMPLEX_MAGNITUDE ) - 28809.8100045040 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 173 ), pDataDescriptor->getActiveRow( 247 ), pDataDescriptor->getActiveBand( 6 ), COMPLEX_MAGNITUDE ) - 9501.8301092178 ) < 1e-3 );

      // verify pixel 425,392
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 425 ), pDataDescriptor->getActiveRow( 392 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) - 20279.1107544608 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 425 ), pDataDescriptor->getActiveRow( 392 ), pDataDescriptor->getActiveBand( 1 ), COMPLEX_MAGNITUDE ) - 4575.4087203445 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 425 ), pDataDescriptor->getActiveRow( 392 ), pDataDescriptor->getActiveBand( 2 ), COMPLEX_MAGNITUDE ) - 8772.5372834734 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 425 ), pDataDescriptor->getActiveRow( 392 ), pDataDescriptor->getActiveBand( 3 ), COMPLEX_MAGNITUDE ) - 8356.0862725819 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 425 ), pDataDescriptor->getActiveRow( 392 ), pDataDescriptor->getActiveBand( 4 ), COMPLEX_MAGNITUDE ) - 20279.1107544608 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 425 ), pDataDescriptor->getActiveRow( 392 ), pDataDescriptor->getActiveBand( 5 ), COMPLEX_MAGNITUDE ) - 29437.6967557510 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 425 ), pDataDescriptor->getActiveRow( 392 ), pDataDescriptor->getActiveBand( 6 ), COMPLEX_MAGNITUDE ) - 6521.8234322591 ) < 1e-3 );

      // verify pixel 72,460
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 72 ), pDataDescriptor->getActiveRow( 460 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) - 13468.6533981664 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 72 ), pDataDescriptor->getActiveRow( 460 ), pDataDescriptor->getActiveBand( 1 ), COMPLEX_MAGNITUDE ) - 3077.3959775079 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 72 ), pDataDescriptor->getActiveRow( 460 ), pDataDescriptor->getActiveBand( 2 ), COMPLEX_MAGNITUDE ) - 4514.3498964043 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 72 ), pDataDescriptor->getActiveRow( 460 ), pDataDescriptor->getActiveBand( 3 ), COMPLEX_MAGNITUDE ) - 10283.1811991730 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 72 ), pDataDescriptor->getActiveRow( 460 ), pDataDescriptor->getActiveBand( 4 ), COMPLEX_MAGNITUDE ) - 13041.7068325844 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 72 ), pDataDescriptor->getActiveRow( 460 ), pDataDescriptor->getActiveBand( 5 ), COMPLEX_MAGNITUDE ) - 26933.3320528525 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 72 ), pDataDescriptor->getActiveRow( 460 ), pDataDescriptor->getActiveBand( 6 ), COMPLEX_MAGNITUDE ) - 2520.9485903488 ) < 1e-3 );

      issea(TestUtilities::destroyWorkspaceWindow(dynamic_cast<WorkspaceWindow*>(
         Service<DesktopServices>()->getWindow(pDataDescriptor->getName(), SPATIAL_DATA_WINDOW))));

      return success;
   }
};


class BandMathAddBandsOnDiskTest : public TestCase
{
public:
   BandMathAddBandsOnDiskTest() : TestCase( "AddBandsOnDisk" ) {}

   bool run()
   {
      bool success = true;
      string filename1 = TestUtilities::getTestDataPath() + "BandMath/ir1pad.sio";
      string filename2 = "";
      string expression = "b2+b5+b6";

      RasterElement *pNewRasterElement = NULL;
      pNewRasterElement = runBandMath( filename1, filename2, expression, true );
      issea( pNewRasterElement != NULL );

      RasterDataDescriptor *pDataDescriptor = NULL;
      pDataDescriptor = dynamic_cast<RasterDataDescriptor*>( pNewRasterElement->getDataDescriptor() );
      issea( pDataDescriptor != NULL );

      issea( pDataDescriptor->getBandCount() == 1 );

      // verify pixel 412,105
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 412 ), pDataDescriptor->getActiveRow( 105 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) == 506 );

      // verify pixel 37,185
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 37 ), pDataDescriptor->getActiveRow( 185 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) == 466 );

      // verify pixel 173,247
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 173 ), pDataDescriptor->getActiveRow( 247 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) == 464 );

      // verify pixel 425,392
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 425 ), pDataDescriptor->getActiveRow( 392 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) == 447 );

      // verify pixel 72,460
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 72 ), pDataDescriptor->getActiveRow( 460 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) == 443 );

      issea(TestUtilities::destroyWorkspaceWindow(dynamic_cast<WorkspaceWindow*>(
         Service<DesktopServices>()->getWindow(pDataDescriptor->getName(), SPATIAL_DATA_WINDOW))));

      return success;
   }
};

class BandMathMultiplyAndSubtractBandsOnDiskTest : public TestCase
{
public:
   BandMathMultiplyAndSubtractBandsOnDiskTest() : TestCase( "MultiplyAndSubtractBandsOnDisk" ) {}

   bool run()
   {
      bool success = true;
      string filename1 = TestUtilities::getTestDataPath() + "BandMath/mi1clouds.sio";
      string filename2 = "";
      string expression = "b1*b4-b7";

      RasterElement *pNewRasterElement = NULL;
      pNewRasterElement = runBandMath( filename1, filename2, expression, true );
      issea( pNewRasterElement != NULL );

      RasterDataDescriptor *pDataDescriptor = NULL;
      pDataDescriptor = dynamic_cast<RasterDataDescriptor*>( pNewRasterElement->getDataDescriptor() );
      issea( pDataDescriptor != NULL );

      issea( pDataDescriptor->getBandCount() == 1 );

      // verify pixel 412,105
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 412 ), pDataDescriptor->getActiveRow( 105 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) == 845 );

      // verify pixel 37,185
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 37 ), pDataDescriptor->getActiveRow( 185 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) == 9630 );

      // verify pixel 173,247
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 173 ), pDataDescriptor->getActiveRow( 247 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) == 2928 );

      // verify pixel 425,392
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 425 ), pDataDescriptor->getActiveRow( 392 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) == 9237 );

      // verify pixel 72,460
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 72 ), pDataDescriptor->getActiveRow( 460 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) == 33334 );

      issea(TestUtilities::destroyWorkspaceWindow(dynamic_cast<WorkspaceWindow*>(
         Service<DesktopServices>()->getWindow(pDataDescriptor->getName(), SPATIAL_DATA_WINDOW))));

      return success;
   }
};

class BandMathAddCubesOnDiskTest : public TestCase
{
public:
   BandMathAddCubesOnDiskTest() : TestCase( "AddCubesOnDisk" ) {}

   bool run()
   {
      bool success = true;
      string filename1 = TestUtilities::getTestDataPath() + "BandMath/mi1clouds.sio";
      string filename2 = TestUtilities::getTestDataPath() + "BandMath/mi2clouds.sio";
      string expression = "c1+c2";

      RasterElement *pNewRasterElement = NULL;
      pNewRasterElement = runBandMath( filename1, filename2, expression, true );
      issea( pNewRasterElement != NULL );

      RasterDataDescriptor *pDataDescriptor = NULL;
      pDataDescriptor = dynamic_cast<RasterDataDescriptor*>( pNewRasterElement->getDataDescriptor() );
      issea( pDataDescriptor != NULL );

      issea( pDataDescriptor->getBandCount() == 7 );

      // verify pixel 412,105
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 412 ), pDataDescriptor->getActiveRow( 105 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) == 166 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 412 ), pDataDescriptor->getActiveRow( 105 ), pDataDescriptor->getActiveBand( 1 ), COMPLEX_MAGNITUDE ) == 61 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 412 ), pDataDescriptor->getActiveRow( 105 ), pDataDescriptor->getActiveBand( 2 ), COMPLEX_MAGNITUDE ) == 51 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 412 ), pDataDescriptor->getActiveRow( 105 ), pDataDescriptor->getActiveBand( 3 ), COMPLEX_MAGNITUDE ) == 27 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 412 ), pDataDescriptor->getActiveRow( 105 ), pDataDescriptor->getActiveBand( 4 ), COMPLEX_MAGNITUDE ) == 24 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 412 ), pDataDescriptor->getActiveRow( 105 ), pDataDescriptor->getActiveBand( 5 ), COMPLEX_MAGNITUDE ) == 295 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 412 ), pDataDescriptor->getActiveRow( 105 ), pDataDescriptor->getActiveBand( 6 ), COMPLEX_MAGNITUDE ) == 10 );

      // verify pixel 37,185
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 37 ), pDataDescriptor->getActiveRow( 185 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) == 176 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 37 ), pDataDescriptor->getActiveRow( 185 ), pDataDescriptor->getActiveBand( 1 ), COMPLEX_MAGNITUDE ) == 72 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 37 ), pDataDescriptor->getActiveRow( 185 ), pDataDescriptor->getActiveBand( 2 ), COMPLEX_MAGNITUDE ) == 71 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 37 ), pDataDescriptor->getActiveRow( 185 ), pDataDescriptor->getActiveBand( 3 ), COMPLEX_MAGNITUDE ) == 232 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 37 ), pDataDescriptor->getActiveRow( 185 ), pDataDescriptor->getActiveBand( 4 ), COMPLEX_MAGNITUDE ) == 220 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 37 ), pDataDescriptor->getActiveRow( 185 ), pDataDescriptor->getActiveBand( 5 ), COMPLEX_MAGNITUDE ) == 292 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 37 ), pDataDescriptor->getActiveRow( 185 ), pDataDescriptor->getActiveBand( 6 ), COMPLEX_MAGNITUDE ) == 77 );

      // verify pixel 173,247
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 173 ), pDataDescriptor->getActiveRow( 247 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) == 227 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 173 ), pDataDescriptor->getActiveRow( 247 ), pDataDescriptor->getActiveBand( 1 ), COMPLEX_MAGNITUDE ) == 98 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 173 ), pDataDescriptor->getActiveRow( 247 ), pDataDescriptor->getActiveBand( 2 ), COMPLEX_MAGNITUDE ) == 126 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 173 ), pDataDescriptor->getActiveRow( 247 ), pDataDescriptor->getActiveBand( 3 ), COMPLEX_MAGNITUDE ) == 112 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 173 ), pDataDescriptor->getActiveRow( 247 ), pDataDescriptor->getActiveBand( 4 ), COMPLEX_MAGNITUDE ) == 166 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 173 ), pDataDescriptor->getActiveRow( 247 ), pDataDescriptor->getActiveBand( 5 ), COMPLEX_MAGNITUDE ) == 303 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 173 ), pDataDescriptor->getActiveRow( 247 ), pDataDescriptor->getActiveBand( 6 ), COMPLEX_MAGNITUDE ) == 105 );

      // verify pixel 425,392
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 425 ), pDataDescriptor->getActiveRow( 392 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) == 244 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 425 ), pDataDescriptor->getActiveRow( 392 ), pDataDescriptor->getActiveBand( 1 ), COMPLEX_MAGNITUDE ) == 106 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 425 ), pDataDescriptor->getActiveRow( 392 ), pDataDescriptor->getActiveBand( 2 ), COMPLEX_MAGNITUDE ) == 140 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 425 ), pDataDescriptor->getActiveRow( 392 ), pDataDescriptor->getActiveBand( 3 ), COMPLEX_MAGNITUDE ) == 157 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 425 ), pDataDescriptor->getActiveRow( 392 ), pDataDescriptor->getActiveBand( 4 ), COMPLEX_MAGNITUDE ) == 239 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 425 ), pDataDescriptor->getActiveRow( 392 ), pDataDescriptor->getActiveBand( 5 ), COMPLEX_MAGNITUDE ) == 332 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 425 ), pDataDescriptor->getActiveRow( 392 ), pDataDescriptor->getActiveBand( 6 ), COMPLEX_MAGNITUDE ) == 127 );

      // verify pixel 72,460
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 72 ), pDataDescriptor->getActiveRow( 460 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) == 320 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 72 ), pDataDescriptor->getActiveRow( 460 ), pDataDescriptor->getActiveBand( 1 ), COMPLEX_MAGNITUDE ) == 138 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 72 ), pDataDescriptor->getActiveRow( 460 ), pDataDescriptor->getActiveBand( 2 ), COMPLEX_MAGNITUDE ) == 169 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 72 ), pDataDescriptor->getActiveRow( 460 ), pDataDescriptor->getActiveBand( 3 ), COMPLEX_MAGNITUDE ) == 252 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 72 ), pDataDescriptor->getActiveRow( 460 ), pDataDescriptor->getActiveBand( 4 ), COMPLEX_MAGNITUDE ) == 284 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 72 ), pDataDescriptor->getActiveRow( 460 ), pDataDescriptor->getActiveBand( 5 ), COMPLEX_MAGNITUDE ) == 267 );
      issea( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 72 ), pDataDescriptor->getActiveRow( 460 ), pDataDescriptor->getActiveBand( 6 ), COMPLEX_MAGNITUDE ) == 124 );

      issea(TestUtilities::destroyWorkspaceWindow(dynamic_cast<WorkspaceWindow*>(
         Service<DesktopServices>()->getWindow(pDataDescriptor->getName(), SPATIAL_DATA_WINDOW))));

      return success;
   }
};

class BandMathComplexExpressionOnDiskTest : public TestCase
{
public:
   BandMathComplexExpressionOnDiskTest() : TestCase( "ComplexExpressionOnDisk" ) {}

   bool run()
   {
      bool success = true;
      string filename1 = TestUtilities::getTestDataPath() + "BandMath/ir1pad.sio";
      string filename2 = TestUtilities::getTestDataPath() + "BandMath/mi2clouds.sio";
      string expression = "(c1*c2)-c2/(c1*cos(c1))";

      RasterElement *pNewRasterElement = NULL;
      pNewRasterElement = runBandMath( filename1, filename2, expression, true );
      issea( pNewRasterElement != NULL );

      RasterDataDescriptor *pDataDescriptor = NULL;
      pDataDescriptor = dynamic_cast<RasterDataDescriptor*>( pNewRasterElement->getDataDescriptor() );
      issea( pDataDescriptor != NULL );

      issea( pDataDescriptor->getBandCount() == 7 );

      // verify pixel 412,105      
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 412 ), pDataDescriptor->getActiveRow( 105 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) - 17533.7460577308 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 412 ), pDataDescriptor->getActiveRow( 105 ), pDataDescriptor->getActiveBand( 1 ), COMPLEX_MAGNITUDE ) - 3466.3043117297 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 412 ), pDataDescriptor->getActiveRow( 105 ), pDataDescriptor->getActiveBand( 2 ), COMPLEX_MAGNITUDE ) - 4709.8083097297 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 412 ), pDataDescriptor->getActiveRow( 105 ), pDataDescriptor->getActiveBand( 3 ), COMPLEX_MAGNITUDE ) - 1952.1513065532 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 412 ), pDataDescriptor->getActiveRow( 105 ), pDataDescriptor->getActiveBand( 4 ), COMPLEX_MAGNITUDE ) - 3707.6095164149 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 412 ), pDataDescriptor->getActiveRow( 105 ), pDataDescriptor->getActiveBand( 5 ), COMPLEX_MAGNITUDE ) - 27689.2535996449 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 412 ), pDataDescriptor->getActiveRow( 105 ), pDataDescriptor->getActiveBand( 6 ), COMPLEX_MAGNITUDE ) - 1047.8954676812 ) < 1e-3 );

      // verify pixel 37,185
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 37 ), pDataDescriptor->getActiveRow( 185 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) - 14253.3692173012 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 37 ), pDataDescriptor->getActiveRow( 185 ), pDataDescriptor->getActiveBand( 1 ), COMPLEX_MAGNITUDE ) - 3044.2939032654 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 37 ), pDataDescriptor->getActiveRow( 185 ), pDataDescriptor->getActiveBand( 2 ), COMPLEX_MAGNITUDE ) - 4122.5290042618 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 37 ), pDataDescriptor->getActiveRow( 185 ), pDataDescriptor->getActiveBand( 3 ), COMPLEX_MAGNITUDE ) - 12689.2389022649 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 37 ), pDataDescriptor->getActiveRow( 185 ), pDataDescriptor->getActiveBand( 4 ), COMPLEX_MAGNITUDE ) - 16468.5152566560 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 37 ), pDataDescriptor->getActiveRow( 185 ), pDataDescriptor->getActiveBand( 5 ), COMPLEX_MAGNITUDE ) - 27798.5734450281 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 37 ), pDataDescriptor->getActiveRow( 185 ), pDataDescriptor->getActiveBand( 6 ), COMPLEX_MAGNITUDE ) - 3159.3186992462 ) < 1e-3 );

      // verify pixel 173,247
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 173 ), pDataDescriptor->getActiveRow( 247 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) - 21912.9083010237 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 173 ), pDataDescriptor->getActiveRow( 247 ), pDataDescriptor->getActiveBand( 1 ), COMPLEX_MAGNITUDE ) - 5479.7290258777 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 173 ), pDataDescriptor->getActiveRow( 247 ), pDataDescriptor->getActiveBand( 2 ), COMPLEX_MAGNITUDE ) - 11701.8849007175 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 173 ), pDataDescriptor->getActiveRow( 247 ), pDataDescriptor->getActiveBand( 3 ), COMPLEX_MAGNITUDE ) - 8344.0053396985 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 173 ), pDataDescriptor->getActiveRow( 247 ), pDataDescriptor->getActiveBand( 4 ), COMPLEX_MAGNITUDE ) - 24975.8265772064 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 173 ), pDataDescriptor->getActiveRow( 247 ), pDataDescriptor->getActiveBand( 5 ), COMPLEX_MAGNITUDE ) - 28809.8100045040 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 173 ), pDataDescriptor->getActiveRow( 247 ), pDataDescriptor->getActiveBand( 6 ), COMPLEX_MAGNITUDE ) - 9501.8301092178 ) < 1e-3 );

      // verify pixel 425,392
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 425 ), pDataDescriptor->getActiveRow( 392 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) - 20279.1107544608 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 425 ), pDataDescriptor->getActiveRow( 392 ), pDataDescriptor->getActiveBand( 1 ), COMPLEX_MAGNITUDE ) - 4575.4087203445 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 425 ), pDataDescriptor->getActiveRow( 392 ), pDataDescriptor->getActiveBand( 2 ), COMPLEX_MAGNITUDE ) - 8772.5372834734 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 425 ), pDataDescriptor->getActiveRow( 392 ), pDataDescriptor->getActiveBand( 3 ), COMPLEX_MAGNITUDE ) - 8356.0862725819 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 425 ), pDataDescriptor->getActiveRow( 392 ), pDataDescriptor->getActiveBand( 4 ), COMPLEX_MAGNITUDE ) - 20279.1107544608 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 425 ), pDataDescriptor->getActiveRow( 392 ), pDataDescriptor->getActiveBand( 5 ), COMPLEX_MAGNITUDE ) - 29437.6967557510 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 425 ), pDataDescriptor->getActiveRow( 392 ), pDataDescriptor->getActiveBand( 6 ), COMPLEX_MAGNITUDE ) - 6521.8234322591 ) < 1e-3 );

      // verify pixel 72,460
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 72 ), pDataDescriptor->getActiveRow( 460 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) - 13468.6533981664 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 72 ), pDataDescriptor->getActiveRow( 460 ), pDataDescriptor->getActiveBand( 1 ), COMPLEX_MAGNITUDE ) - 3077.3959775079 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 72 ), pDataDescriptor->getActiveRow( 460 ), pDataDescriptor->getActiveBand( 2 ), COMPLEX_MAGNITUDE ) - 4514.3498964043 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 72 ), pDataDescriptor->getActiveRow( 460 ), pDataDescriptor->getActiveBand( 3 ), COMPLEX_MAGNITUDE ) - 10283.1811991730 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 72 ), pDataDescriptor->getActiveRow( 460 ), pDataDescriptor->getActiveBand( 4 ), COMPLEX_MAGNITUDE ) - 13041.7068325844 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 72 ), pDataDescriptor->getActiveRow( 460 ), pDataDescriptor->getActiveBand( 5 ), COMPLEX_MAGNITUDE ) - 26933.3320528525 ) < 1e-3 );
      issea( fabs( pNewRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 72 ), pDataDescriptor->getActiveRow( 460 ), pDataDescriptor->getActiveBand( 6 ), COMPLEX_MAGNITUDE ) - 2520.9485903488 ) < 1e-3 );

      issea(TestUtilities::destroyWorkspaceWindow(dynamic_cast<WorkspaceWindow*>(
         Service<DesktopServices>()->getWindow(pDataDescriptor->getName(), SPATIAL_DATA_WINDOW))));

      return success;
   }
};

RasterElement* runBandMath( string filename1, string filename2, string expression, bool onDisk )
{
   bool success = true;
   bool useDegrees = false;
   bool displayResults = true;
   SpatialDataWindow *pWindow1 = NULL;
   SpatialDataWindow *pWindow2 = NULL;
   RasterElement* pRasterElement1 = NULL;
   RasterElement* pRasterElement2 = NULL;

   if( !onDisk )
   {
      pWindow1 = TestUtilities::loadDataSet( filename1, "SIO Importer" );
      issea( pWindow1 != NULL );      
      pRasterElement1 = dynamic_cast<RasterElement*>( ModelServicesImp::instance()->getElement( filename1, "RasterElement", NULL ) );
      issea( pRasterElement1 != NULL );

      if( filename2 != "" )
      {
         pWindow2 = TestUtilities::loadDataSet( filename2, "SIO Importer" );
         issea( pWindow2 != NULL );      
         pRasterElement2 = dynamic_cast<RasterElement*>( ModelServicesImp::instance()->getElement( filename2, "RasterElement", NULL ) );
         issea( pRasterElement2 != NULL );
      }
   }
   else
   {
      vector<ImportDescriptor*> pDescriptors;

      ImporterResource sioImporter1( "SIO Importer", filename1, NULL, false );
      pDescriptors = sioImporter1->getImportDescriptors();

      ImportDescriptor* pImportDescriptor1 = pDescriptors.at(0);
      issea(pImportDescriptor1 != NULL);

      RasterDataDescriptor* pParams1 = dynamic_cast<RasterDataDescriptor*>(pImportDescriptor1->getDataDescriptor());
      issea( pParams1 != NULL );

      if( success )
      {
         pParams1->setProcessingLocation( ON_DISK_READ_ONLY );
         sioImporter1->execute();
         pRasterElement1 = dynamic_cast<RasterElement*>( ModelServicesImp::instance()->getElement( filename1, "RasterElement", NULL ) );
         issea( pRasterElement1 != NULL );
         pWindow1 = dynamic_cast<SpatialDataWindow*>(Service<DesktopServices>()->getWindow(filename1,
            SPATIAL_DATA_WINDOW));
         issea( pWindow1 != NULL );
      }

      if( filename2 != "" )
      {
         pDescriptors.clear();

         ImporterResource sioImporter2( "SIO Importer", filename2, NULL, false );
         pDescriptors = sioImporter2->getImportDescriptors();

         ImportDescriptor* pImportDescriptor2 = pDescriptors.at(0);
         issea(pImportDescriptor2 != NULL);

         RasterDataDescriptor* pParams2 = dynamic_cast<RasterDataDescriptor*>(pImportDescriptor2->getDataDescriptor());
         issea( pParams2 != NULL );

         if( success )
         {
            pParams2->setProcessingLocation( ON_DISK_READ_ONLY );
            sioImporter2->execute();
            pRasterElement2 = dynamic_cast<RasterElement*>( ModelServicesImp::instance()->getElement( filename2, "RasterElement", NULL ) );
            issea( pRasterElement2 != NULL );
            pWindow2 = dynamic_cast<SpatialDataWindow*>(Service<DesktopServices>()->getWindow(filename2,
               SPATIAL_DATA_WINDOW));
            issea( pWindow2 != NULL );
         }
      }
   }

   // call the BandMath plugin
   ExecutableResource exPlugIn("Band Math", "", NULL, true);
   exPlugIn->getInArgList().setPlugInArgValue(Executable::DataElementArg(), pRasterElement1);
   exPlugIn->getInArgList().setPlugInArgValue("Raster Element 2", pRasterElement2);
   exPlugIn->getInArgList().setPlugInArgValue("Degrees", &useDegrees);
   exPlugIn->getInArgList().setPlugInArgValue("Display Results", &displayResults);
   exPlugIn->getInArgList().setPlugInArgValue("Input Expression", &expression);
   success = exPlugIn->execute();

   RasterElement *pResults = NULL;
   if( success )
   {
      pResults = exPlugIn->getOutArgList().getPlugInArgValue<RasterElement>("Band Math Result");
      issea( pResults != NULL );
   }

   issea(TestUtilities::destroyWorkspaceWindow(pWindow1));
   if (pWindow2 != NULL)
   {
      issea(TestUtilities::destroyWorkspaceWindow(pWindow2));
   }

   if( !success )
   {
      return NULL;
   }
   return pResults;
}

class BandMathTestSuite : public TestSuiteNewSession
{
public:
   BandMathTestSuite() : TestSuiteNewSession( "BandMath" )
   {
      addTestCase( new BandMathAverageBandsTest );
      addTestCase( new BandMathAddBandsTest );
      addTestCase( new BandMathMultiplyAndSubtractBandsTest );
      addTestCase( new BandMathAddCubesTest );
      addTestCase( new BandMathComplexExpressionTest );
      addTestCase( new BandMathAddBandsOnDiskTest );
      addTestCase( new BandMathMultiplyAndSubtractBandsOnDiskTest );
      addTestCase( new BandMathAddCubesOnDiskTest );
      addTestCase( new BandMathComplexExpressionOnDiskTest );
   }
};

REGISTER_SUITE( BandMathTestSuite )
