/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"
#include "ArgumentList.h"
#include "assert.h"
#include "ConfigurationSettingsImp.h"
#include "DesktopServices.h"
#include "LayerList.h"
#include "ModelServicesImp.h"
#include "RasterElement.h"
#include "RasterDataDescriptor.h"
#include "RasterFileDescriptor.h"
#include "SpatialDataWindow.h"
#include "SpatialDataView.h"
#include "TestBedTestUtilities.h"
#include "TestSuiteNewSession.h"
#include "TestUtilities.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QString>

#include <stdlib.h>
#include <string>
using namespace std;

bool runWizard(const std::string &wizardName)
{
   std::string runString = "BatchProcessing" + SLASH + "runwiz.py";
   ArgumentList *pArgList = ArgumentList::instance();
   if (pArgList != NULL)
   {
      std::string deploymentFile = pArgList->getOption("deployment");
      if (deploymentFile.empty())
      {
         // Use the deployment file for the test bed application instead of the OpticksBatch
         // application, which ensures the test bed default configuration settings are used
         QDir binaryDir(QCoreApplication::applicationDirPath());
         binaryDir.makeAbsolute();
         QString binaryDirPath = binaryDir.canonicalPath();
         deploymentFile = binaryDirPath.toStdString() + SLASH + "opticks.dep";
      }

      if (!deploymentFile.empty())
      {
         runString += " -c \"" + deploymentFile + "\"";
      }
   }
   runString += " -i \"" + wizardName + "\"";
   runString += " -d \"" + QCoreApplication::applicationDirPath().toStdString() + "\"";
   printf("runstring: %s\n", runString.c_str());
   int result = system(runString.c_str());

   return result == EXIT_SUCCESS;
}

class BandMathBatchProcessingTest : public TestCase
{
public:
   BandMathBatchProcessingTest() : TestCase("BandMathBatch") {}
   bool run()
   {
      bool success = true;
      SpatialDataWindow *pWindow = NULL;
      RasterElement *pRasterElement = NULL;
      string sName = "";

      // writes a cube named AddBands.sio
      issea(runWizard("AddBands"));
      // open the new cube and compare pixel values to expected values
      pWindow = TestUtilities::loadDataSet( "BatchProcessing/data/AddBands.ice.h5", "Ice Importer" );

      issea( pWindow != NULL );
      sName = pWindow->getName();
      issea( sName != "" );
      pRasterElement = dynamic_cast<RasterElement*>( ModelServicesImp::instance()->getElement( sName, "RasterElement", NULL ) );
      issea( pRasterElement != NULL );

      RasterDataDescriptor *pDataDescriptor = NULL;
      pDataDescriptor = dynamic_cast<RasterDataDescriptor*>( pRasterElement->getDataDescriptor() );
      issea( pDataDescriptor != NULL );
      issea( pDataDescriptor->getBandCount() == 1 );

      // verify pixel 412,105
      issea( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 412 ), pDataDescriptor->getActiveRow( 105 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) == 506 );

      // verify pixel 37,185
      issea( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 37 ), pDataDescriptor->getActiveRow( 185 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) == 466 );

      // verify pixel 173,247
      issea( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 173 ), pDataDescriptor->getActiveRow( 247 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) == 464 );

      // verify pixel 425,392
      issea( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 425 ), pDataDescriptor->getActiveRow( 392 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) == 447 );

      // verify pixel 72,460
      issea( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 72 ), pDataDescriptor->getActiveRow( 460 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) == 443 );
      issea(TestUtilities::destroyWorkspaceWindow(pWindow));

      // writes a cube named MultiplyAndSubtractBands.sio
      issea(runWizard("MultiplyAndSubtractBands"));

      // open the new cube and compare pixel values to expected values
      pWindow = NULL;
      pRasterElement = NULL;
      sName = "";
      pWindow = TestUtilities::loadDataSet( "BatchProcessing/data/MultiplyAndSubtractBands.ice.h5", "Ice Importer" );
      issea( pWindow != NULL );
      sName = pWindow->getName();
      issea( sName != "" );
      pRasterElement = dynamic_cast<RasterElement*>( ModelServicesImp::instance()->getElement( sName, "RasterElement", NULL ) );
      issea( pRasterElement != NULL );

      pDataDescriptor = NULL;
      pDataDescriptor = dynamic_cast<RasterDataDescriptor*>( pRasterElement->getDataDescriptor() );
      issea( pDataDescriptor != NULL );
      issea( pDataDescriptor->getBandCount() == 1 );

      // verify pixel 412,105
      issea( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 412 ), pDataDescriptor->getActiveRow( 105 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) == 845 );

      // verify pixel 37,185
      issea( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 37 ), pDataDescriptor->getActiveRow( 185 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) == 9630 );

      // verify pixel 173,247
      issea( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 173 ), pDataDescriptor->getActiveRow( 247 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) == 2928 );

      // verify pixel 425,392
      issea( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 425 ), pDataDescriptor->getActiveRow( 392 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) == 9237 );

      // verify pixel 72,460
      issea( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 72 ), pDataDescriptor->getActiveRow( 460 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) == 33334 );
      issea(TestUtilities::destroyWorkspaceWindow(pWindow));

      // writes a cube named AddCubes.sio
      issea(runWizard("AddCubes"));

      // open the new cube and compare pixel values to expected values
      pWindow = NULL;
      pRasterElement = NULL;
      sName = "";
      pWindow = TestUtilities::loadDataSet( "BatchProcessing/data/AddCubes.ice.h5", "Ice Importer" );
      issea( pWindow != NULL );
      sName = pWindow->getName();
      issea( sName != "" );
      pRasterElement = dynamic_cast<RasterElement*>( ModelServicesImp::instance()->getElement( sName, "RasterElement", NULL ) );
      issea( pRasterElement != NULL );

      pDataDescriptor = NULL;
      pDataDescriptor = dynamic_cast<RasterDataDescriptor*>( pRasterElement->getDataDescriptor() );
      issea( pDataDescriptor != NULL );
      issea( pDataDescriptor->getBandCount() == 7 );

      // verify pixel 412,105
      issea( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 412 ), pDataDescriptor->getActiveRow( 105 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) == 166 );
      issea( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 412 ), pDataDescriptor->getActiveRow( 105 ), pDataDescriptor->getActiveBand( 1 ), COMPLEX_MAGNITUDE ) == 61 );
      issea( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 412 ), pDataDescriptor->getActiveRow( 105 ), pDataDescriptor->getActiveBand( 2 ), COMPLEX_MAGNITUDE ) == 51 );
      issea( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 412 ), pDataDescriptor->getActiveRow( 105 ), pDataDescriptor->getActiveBand( 3 ), COMPLEX_MAGNITUDE ) == 27 );
      issea( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 412 ), pDataDescriptor->getActiveRow( 105 ), pDataDescriptor->getActiveBand( 4 ), COMPLEX_MAGNITUDE ) == 24 );
      issea( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 412 ), pDataDescriptor->getActiveRow( 105 ), pDataDescriptor->getActiveBand( 5 ), COMPLEX_MAGNITUDE ) == 295 );
      issea( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 412 ), pDataDescriptor->getActiveRow( 105 ), pDataDescriptor->getActiveBand( 6 ), COMPLEX_MAGNITUDE ) == 10 );

      // verify pixel 37,185
      issea( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 37 ), pDataDescriptor->getActiveRow( 185 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) == 176 );
      issea( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 37 ), pDataDescriptor->getActiveRow( 185 ), pDataDescriptor->getActiveBand( 1 ), COMPLEX_MAGNITUDE ) == 72 );
      issea( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 37 ), pDataDescriptor->getActiveRow( 185 ), pDataDescriptor->getActiveBand( 2 ), COMPLEX_MAGNITUDE ) == 71 );
      issea( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 37 ), pDataDescriptor->getActiveRow( 185 ), pDataDescriptor->getActiveBand( 3 ), COMPLEX_MAGNITUDE ) == 232 );
      issea( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 37 ), pDataDescriptor->getActiveRow( 185 ), pDataDescriptor->getActiveBand( 4 ), COMPLEX_MAGNITUDE ) == 220 );
      issea( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 37 ), pDataDescriptor->getActiveRow( 185 ), pDataDescriptor->getActiveBand( 5 ), COMPLEX_MAGNITUDE ) == 292 );
      issea( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 37 ), pDataDescriptor->getActiveRow( 185 ), pDataDescriptor->getActiveBand( 6 ), COMPLEX_MAGNITUDE ) == 77 );

      // verify pixel 173,247
      issea( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 173 ), pDataDescriptor->getActiveRow( 247 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) == 227 );
      issea( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 173 ), pDataDescriptor->getActiveRow( 247 ), pDataDescriptor->getActiveBand( 1 ), COMPLEX_MAGNITUDE ) == 98 );
      issea( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 173 ), pDataDescriptor->getActiveRow( 247 ), pDataDescriptor->getActiveBand( 2 ), COMPLEX_MAGNITUDE ) == 126 );
      issea( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 173 ), pDataDescriptor->getActiveRow( 247 ), pDataDescriptor->getActiveBand( 3 ), COMPLEX_MAGNITUDE ) == 112 );
      issea( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 173 ), pDataDescriptor->getActiveRow( 247 ), pDataDescriptor->getActiveBand( 4 ), COMPLEX_MAGNITUDE ) == 166 );
      issea( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 173 ), pDataDescriptor->getActiveRow( 247 ), pDataDescriptor->getActiveBand( 5 ), COMPLEX_MAGNITUDE ) == 303 );
      issea( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 173 ), pDataDescriptor->getActiveRow( 247 ), pDataDescriptor->getActiveBand( 6 ), COMPLEX_MAGNITUDE ) == 105 );

      // verify pixel 425,392
      issea( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 425 ), pDataDescriptor->getActiveRow( 392 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) == 244 );
      issea( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 425 ), pDataDescriptor->getActiveRow( 392 ), pDataDescriptor->getActiveBand( 1 ), COMPLEX_MAGNITUDE ) == 106 );
      issea( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 425 ), pDataDescriptor->getActiveRow( 392 ), pDataDescriptor->getActiveBand( 2 ), COMPLEX_MAGNITUDE ) == 140 );
      issea( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 425 ), pDataDescriptor->getActiveRow( 392 ), pDataDescriptor->getActiveBand( 3 ), COMPLEX_MAGNITUDE ) == 157 );
      issea( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 425 ), pDataDescriptor->getActiveRow( 392 ), pDataDescriptor->getActiveBand( 4 ), COMPLEX_MAGNITUDE ) == 239 );
      issea( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 425 ), pDataDescriptor->getActiveRow( 392 ), pDataDescriptor->getActiveBand( 5 ), COMPLEX_MAGNITUDE ) == 332 );
      issea( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 425 ), pDataDescriptor->getActiveRow( 392 ), pDataDescriptor->getActiveBand( 6 ), COMPLEX_MAGNITUDE ) == 127 );

      // verify pixel 72,460
      issea( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 72 ), pDataDescriptor->getActiveRow( 460 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) == 320 );
      issea( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 72 ), pDataDescriptor->getActiveRow( 460 ), pDataDescriptor->getActiveBand( 1 ), COMPLEX_MAGNITUDE ) == 138 );
      issea( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 72 ), pDataDescriptor->getActiveRow( 460 ), pDataDescriptor->getActiveBand( 2 ), COMPLEX_MAGNITUDE ) == 169 );
      issea( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 72 ), pDataDescriptor->getActiveRow( 460 ), pDataDescriptor->getActiveBand( 3 ), COMPLEX_MAGNITUDE ) == 252 );
      issea( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 72 ), pDataDescriptor->getActiveRow( 460 ), pDataDescriptor->getActiveBand( 4 ), COMPLEX_MAGNITUDE ) == 284 );
      issea( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 72 ), pDataDescriptor->getActiveRow( 460 ), pDataDescriptor->getActiveBand( 5 ), COMPLEX_MAGNITUDE ) == 267 );
      issea( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 72 ), pDataDescriptor->getActiveRow( 460 ), pDataDescriptor->getActiveBand( 6 ), COMPLEX_MAGNITUDE ) == 124 );
      issea(TestUtilities::destroyWorkspaceWindow(pWindow));

      // writes a cube named ComplexExpression.sio
      issea(runWizard("ComplexExpression"));

      // open the new cube and compare pixel values to expected values
      pWindow = NULL;
      pRasterElement = NULL;
      sName = "";
      pWindow = TestUtilities::loadDataSet( "BatchProcessing/data/ComplexExpression.ice.h5", "Ice Importer" );
      issea( pWindow != NULL );
      sName = pWindow->getName();
      issea( sName != "" );
      pRasterElement = dynamic_cast<RasterElement*>( ModelServicesImp::instance()->getElement( sName, "RasterElement", NULL ) );
      issea( pRasterElement != NULL );

      pDataDescriptor = NULL;
      pDataDescriptor = dynamic_cast<RasterDataDescriptor*>( pRasterElement->getDataDescriptor() );
      issea( pDataDescriptor != NULL );

      issea( pDataDescriptor->getBandCount() == 7 );

      // verify pixel 412,105      
      issea( fabs( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 412 ), pDataDescriptor->getActiveRow( 105 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) - 17533.7460577308 ) < 1e-3 );
      issea( fabs( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 412 ), pDataDescriptor->getActiveRow( 105 ), pDataDescriptor->getActiveBand( 1 ), COMPLEX_MAGNITUDE ) - 3466.3043117297 ) < 1e-3 );
      issea( fabs( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 412 ), pDataDescriptor->getActiveRow( 105 ), pDataDescriptor->getActiveBand( 2 ), COMPLEX_MAGNITUDE ) - 4709.8083097297 ) < 1e-3 );
      issea( fabs( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 412 ), pDataDescriptor->getActiveRow( 105 ), pDataDescriptor->getActiveBand( 3 ), COMPLEX_MAGNITUDE ) - 1952.1513065532 ) < 1e-3 );
      issea( fabs( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 412 ), pDataDescriptor->getActiveRow( 105 ), pDataDescriptor->getActiveBand( 4 ), COMPLEX_MAGNITUDE ) - 3707.6095164149 ) < 1e-3 );
      issea( fabs( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 412 ), pDataDescriptor->getActiveRow( 105 ), pDataDescriptor->getActiveBand( 5 ), COMPLEX_MAGNITUDE ) - 27689.2535996449 ) < 1e-3 );
      issea( fabs( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 412 ), pDataDescriptor->getActiveRow( 105 ), pDataDescriptor->getActiveBand( 6 ), COMPLEX_MAGNITUDE ) - 1047.8954676812 ) < 1e-3 );

      // verify pixel 37,185
      issea( fabs( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 37 ), pDataDescriptor->getActiveRow( 185 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) - 14253.3692173012 ) < 1e-3 );
      issea( fabs( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 37 ), pDataDescriptor->getActiveRow( 185 ), pDataDescriptor->getActiveBand( 1 ), COMPLEX_MAGNITUDE ) - 3044.2939032654 ) < 1e-3 );
      issea( fabs( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 37 ), pDataDescriptor->getActiveRow( 185 ), pDataDescriptor->getActiveBand( 2 ), COMPLEX_MAGNITUDE ) - 4122.5290042618 ) < 1e-3 );
      issea( fabs( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 37 ), pDataDescriptor->getActiveRow( 185 ), pDataDescriptor->getActiveBand( 3 ), COMPLEX_MAGNITUDE ) - 12689.2389022649 ) < 1e-3 );
      issea( fabs( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 37 ), pDataDescriptor->getActiveRow( 185 ), pDataDescriptor->getActiveBand( 4 ), COMPLEX_MAGNITUDE ) - 16468.5152566560 ) < 1e-3 );
      issea( fabs( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 37 ), pDataDescriptor->getActiveRow( 185 ), pDataDescriptor->getActiveBand( 5 ), COMPLEX_MAGNITUDE ) - 27798.5734450281 ) < 1e-3 );
      issea( fabs( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 37 ), pDataDescriptor->getActiveRow( 185 ), pDataDescriptor->getActiveBand( 6 ), COMPLEX_MAGNITUDE ) - 3159.3186992462 ) < 1e-3 );

      // verify pixel 173,247
      issea( fabs( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 173 ), pDataDescriptor->getActiveRow( 247 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) - 21912.9083010237 ) < 1e-3 );
      issea( fabs( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 173 ), pDataDescriptor->getActiveRow( 247 ), pDataDescriptor->getActiveBand( 1 ), COMPLEX_MAGNITUDE ) - 5479.7290258777 ) < 1e-3 );
      issea( fabs( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 173 ), pDataDescriptor->getActiveRow( 247 ), pDataDescriptor->getActiveBand( 2 ), COMPLEX_MAGNITUDE ) - 11701.8849007175 ) < 1e-3 );
      issea( fabs( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 173 ), pDataDescriptor->getActiveRow( 247 ), pDataDescriptor->getActiveBand( 3 ), COMPLEX_MAGNITUDE ) - 8344.0053396985 ) < 1e-3 );
      issea( fabs( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 173 ), pDataDescriptor->getActiveRow( 247 ), pDataDescriptor->getActiveBand( 4 ), COMPLEX_MAGNITUDE ) - 24975.8265772064 ) < 1e-3 );
      issea( fabs( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 173 ), pDataDescriptor->getActiveRow( 247 ), pDataDescriptor->getActiveBand( 5 ), COMPLEX_MAGNITUDE ) - 28809.8100045040 ) < 1e-3 );
      issea( fabs( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 173 ), pDataDescriptor->getActiveRow( 247 ), pDataDescriptor->getActiveBand( 6 ), COMPLEX_MAGNITUDE ) - 9501.8301092178 ) < 1e-3 );

      // verify pixel 425,392
      issea( fabs( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 425 ), pDataDescriptor->getActiveRow( 392 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) - 20279.1107544608 ) < 1e-3 );
      issea( fabs( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 425 ), pDataDescriptor->getActiveRow( 392 ), pDataDescriptor->getActiveBand( 1 ), COMPLEX_MAGNITUDE ) - 4575.4087203445 ) < 1e-3 );
      issea( fabs( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 425 ), pDataDescriptor->getActiveRow( 392 ), pDataDescriptor->getActiveBand( 2 ), COMPLEX_MAGNITUDE ) - 8772.5372834734 ) < 1e-3 );
      issea( fabs( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 425 ), pDataDescriptor->getActiveRow( 392 ), pDataDescriptor->getActiveBand( 3 ), COMPLEX_MAGNITUDE ) - 8356.0862725819 ) < 1e-3 );
      issea( fabs( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 425 ), pDataDescriptor->getActiveRow( 392 ), pDataDescriptor->getActiveBand( 4 ), COMPLEX_MAGNITUDE ) - 20279.1107544608 ) < 1e-3 );
      issea( fabs( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 425 ), pDataDescriptor->getActiveRow( 392 ), pDataDescriptor->getActiveBand( 5 ), COMPLEX_MAGNITUDE ) - 29437.6967557510 ) < 1e-3 );
      issea( fabs( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 425 ), pDataDescriptor->getActiveRow( 392 ), pDataDescriptor->getActiveBand( 6 ), COMPLEX_MAGNITUDE ) - 6521.8234322591 ) < 1e-3 );

      // verify pixel 72,460
      issea( fabs( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 72 ), pDataDescriptor->getActiveRow( 460 ), pDataDescriptor->getActiveBand( 0 ), COMPLEX_MAGNITUDE ) - 13468.6533981664 ) < 1e-3 );
      issea( fabs( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 72 ), pDataDescriptor->getActiveRow( 460 ), pDataDescriptor->getActiveBand( 1 ), COMPLEX_MAGNITUDE ) - 3077.3959775079 ) < 1e-3 );
      issea( fabs( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 72 ), pDataDescriptor->getActiveRow( 460 ), pDataDescriptor->getActiveBand( 2 ), COMPLEX_MAGNITUDE ) - 4514.3498964043 ) < 1e-3 );
      issea( fabs( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 72 ), pDataDescriptor->getActiveRow( 460 ), pDataDescriptor->getActiveBand( 3 ), COMPLEX_MAGNITUDE ) - 10283.1811991730 ) < 1e-3 );
      issea( fabs( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 72 ), pDataDescriptor->getActiveRow( 460 ), pDataDescriptor->getActiveBand( 4 ), COMPLEX_MAGNITUDE ) - 13041.7068325844 ) < 1e-3 );
      issea( fabs( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 72 ), pDataDescriptor->getActiveRow( 460 ), pDataDescriptor->getActiveBand( 5 ), COMPLEX_MAGNITUDE ) - 26933.3320528525 ) < 1e-3 );
      issea( fabs( pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( 72 ), pDataDescriptor->getActiveRow( 460 ), pDataDescriptor->getActiveBand( 6 ), COMPLEX_MAGNITUDE ) - 2520.9485903488 ) < 1e-3 );
      issea(TestUtilities::destroyWorkspaceWindow(pWindow));

      return success;
   }
};

class ImportExportBatchProcessingTest : public TestCase
{
public:
   ImportExportBatchProcessingTest() : TestCase("ImportExportBatch") {}
   bool run()
   {
      bool success = true;
      SpatialDataWindow *pWindowSio = NULL;
      SpatialDataWindow *pWindowNew = NULL;
      RasterElement *pRasterElementSio = NULL;
      RasterElement *pRasterElementNew = NULL;
      RasterDataDescriptor *pDataDescriptorSio = NULL;
      RasterDataDescriptor *pDataDescriptorNew = NULL;

      string sNameSio = "";
      string sNameNew = "";

      // writes a cube called SioInNitfOut.ntf
      issea(runWizard("SioInNitfOut"));

      pWindowSio = TestUtilities::loadDataSet(TestUtilities::getTestDataPath() + "daytonchip.sio", "SIO Importer");
      issea( pWindowSio != NULL );
      if( pWindowSio != NULL )
      {
         sNameSio = pWindowSio->getName();
         issea( sNameSio != "" );
         pRasterElementSio = dynamic_cast<RasterElement*>( ModelServicesImp::instance()->getElement( sNameSio, "RasterElement", NULL ) );
         issea( pRasterElementSio != NULL );
      }

      pDataDescriptorSio = NULL;
      pDataDescriptorSio = dynamic_cast<RasterDataDescriptor*>( pRasterElementSio->getDataDescriptor() );
      issea( pDataDescriptorSio != NULL );

      pWindowNew = TestUtilities::loadDataSet( "BatchProcessing/data/SioInNitfOut.ntf", "NITF Importer" );
      issea( pWindowNew != NULL );
      if( pWindowNew != NULL )
      {      
         sNameNew = pWindowNew->getName();
         issea( sNameNew != "" );
         pRasterElementNew = dynamic_cast<RasterElement*>( ModelServicesImp::instance()->getElement( sNameNew, "RasterElement", NULL ) );
         issea( pRasterElementNew != NULL );
      }

      pDataDescriptorNew = NULL;
      pDataDescriptorNew = dynamic_cast<RasterDataDescriptor*>( pRasterElementNew->getDataDescriptor() );
      issea( pDataDescriptorNew != NULL );

      // now compare the cubes pixel by pixel
      issea( pDataDescriptorSio->getRowCount() == pDataDescriptorNew->getRowCount() );
      issea( pDataDescriptorSio->getColumnCount() == pDataDescriptorNew->getColumnCount() );
      issea( pDataDescriptorSio->getBandCount() == pDataDescriptorNew->getBandCount() );
      for( unsigned int row = 0; row < pDataDescriptorSio->getRowCount(); row++ )
      {
         for( unsigned int col = 0; col < pDataDescriptorSio->getColumnCount(); col++ )
         {
            for( unsigned int band = 0; band < pDataDescriptorSio->getBandCount(); band++ )
            {
               issea( pRasterElementSio->getPixelValue( pDataDescriptorSio->getActiveColumn( col ), pDataDescriptorSio->getActiveRow( row ), pDataDescriptorSio->getActiveBand( band ), COMPLEX_MAGNITUDE ) == 
                  pRasterElementNew->getPixelValue( pDataDescriptorNew->getActiveColumn( col ), pDataDescriptorNew->getActiveRow( row ), pDataDescriptorNew->getActiveBand( band ), COMPLEX_MAGNITUDE ) );
            }
         }
      }
      issea(TestUtilities::destroyWorkspaceWindow(pWindowSio));
      issea(TestUtilities::destroyWorkspaceWindow(pWindowNew));
      pDataDescriptorSio = NULL;
      pDataDescriptorNew = NULL;

      // writes a cube called SioInTifOut.tif
      issea(runWizard("SioInTifOut"));

      pWindowSio = NULL;
      pWindowNew = NULL;
      pRasterElementSio = NULL;
      pRasterElementNew = NULL;
      sNameSio = "";
      sNameNew = "";
      pWindowSio = TestUtilities::loadDataSet(TestUtilities::getTestDataPath() + "daytonchip.sio", "SIO Importer");
      issea( pWindowSio != NULL );
      if( pWindowSio != NULL )
      {
         sNameSio = pWindowSio->getName();
         issea( sNameSio != "" );
         pRasterElementSio = dynamic_cast<RasterElement*>( ModelServicesImp::instance()->getElement( sNameSio, "RasterElement", NULL ) );
         issea( pRasterElementSio != NULL );
      }

      pDataDescriptorSio = NULL;
      pDataDescriptorSio = dynamic_cast<RasterDataDescriptor*>( pRasterElementSio->getDataDescriptor() );
      issea( pDataDescriptorSio != NULL );

      pWindowNew = TestUtilities::loadDataSet( "BatchProcessing/data/SioInTifOut.tif", "GeoTIFF Importer" );
      issea( pWindowNew != NULL );
      if( pWindowNew != NULL )
      {      
         sNameNew = pWindowNew->getName();
         issea( sNameNew != "" );
         pRasterElementNew = dynamic_cast<RasterElement*>( ModelServicesImp::instance()->getElement( sNameNew, "RasterElement", NULL ) );
         issea( pRasterElementNew != NULL );
      }

      pDataDescriptorNew = NULL;
      pDataDescriptorNew = dynamic_cast<RasterDataDescriptor*>( pRasterElementNew->getDataDescriptor() );
      issea( pDataDescriptorNew != NULL );

      // now compare the cubes pixel by pixel
      issea( pDataDescriptorSio->getRowCount() == pDataDescriptorNew->getRowCount() );
      issea( pDataDescriptorSio->getColumnCount() == pDataDescriptorNew->getColumnCount() );
      issea( pDataDescriptorSio->getBandCount() == pDataDescriptorNew->getBandCount() );
      for( unsigned int row = 0; row < pDataDescriptorSio->getRowCount(); row++ )
      {
         for( unsigned int col = 0; col < pDataDescriptorSio->getColumnCount(); col++ )
         {
            for( unsigned int band = 0; band < pDataDescriptorSio->getBandCount(); band++ )
            {
               issea( pRasterElementSio->getPixelValue( pDataDescriptorSio->getActiveColumn( col ), pDataDescriptorSio->getActiveRow( row ), pDataDescriptorSio->getActiveBand( band ), COMPLEX_MAGNITUDE ) == 
                  pRasterElementNew->getPixelValue( pDataDescriptorNew->getActiveColumn( col ), pDataDescriptorNew->getActiveRow( row ), pDataDescriptorNew->getActiveBand( band ), COMPLEX_MAGNITUDE ) );
            }
         }
      }
      issea(TestUtilities::destroyWorkspaceWindow(pWindowSio));
      issea(TestUtilities::destroyWorkspaceWindow(pWindowNew));
      pDataDescriptorSio = NULL;
      pDataDescriptorNew = NULL;

      // writes a cube called SioInIceOut.ice.h5
      issea(runWizard("SioInIceOut")); 

      pWindowSio = NULL;
      pWindowNew = NULL;
      pRasterElementSio = NULL;
      pRasterElementNew = NULL;
      sNameSio = "";
      sNameNew = "";
      pWindowSio = TestUtilities::loadDataSet(TestUtilities::getTestDataPath() + "daytonchip.sio", "SIO Importer");
      issea( pWindowSio != NULL );
      if( pWindowSio != NULL )
      {
         sNameSio = pWindowSio->getName();
         issea( sNameSio != "" );
         pRasterElementSio = dynamic_cast<RasterElement*>( ModelServicesImp::instance()->getElement( sNameSio, "RasterElement", NULL ) );
         issea( pRasterElementSio != NULL );
      }

      pDataDescriptorSio = NULL;
      pDataDescriptorSio = dynamic_cast<RasterDataDescriptor*>( pRasterElementSio->getDataDescriptor() );
      issea( pDataDescriptorSio != NULL );

      pWindowNew = TestUtilities::loadDataSet( "BatchProcessing/data/SioInIceOut.ice.h5", "Ice Importer" );
      issea( pWindowNew != NULL );
      if( pWindowNew != NULL )
      {      
         sNameNew = pWindowNew->getName();
         issea( sNameNew != "" );
         pRasterElementNew = dynamic_cast<RasterElement*>( ModelServicesImp::instance()->getElement( sNameNew, "RasterElement", NULL ) );
         issea( pRasterElementNew != NULL );
      }

      pDataDescriptorNew = NULL;
      pDataDescriptorNew = dynamic_cast<RasterDataDescriptor*>( pRasterElementNew->getDataDescriptor() );
      issea( pDataDescriptorNew != NULL );

      // now compare the cubes pixel by pixel
      issea( pDataDescriptorSio->getRowCount() == pDataDescriptorNew->getRowCount() );
      issea( pDataDescriptorSio->getColumnCount() == pDataDescriptorNew->getColumnCount() );
      issea( pDataDescriptorSio->getBandCount() == pDataDescriptorNew->getBandCount() );
      for( unsigned int row = 0; row < pDataDescriptorSio->getRowCount(); row++ )
      {
         for( unsigned int col = 0; col < pDataDescriptorSio->getColumnCount(); col++ )
         {
            for( unsigned int band = 0; band < pDataDescriptorSio->getBandCount(); band++ )
            {
               issea( pRasterElementSio->getPixelValue( pDataDescriptorSio->getActiveColumn( col ), pDataDescriptorSio->getActiveRow( row ), pDataDescriptorSio->getActiveBand( band ), COMPLEX_MAGNITUDE ) == 
                  pRasterElementNew->getPixelValue( pDataDescriptorNew->getActiveColumn( col ), pDataDescriptorNew->getActiveRow( row ), pDataDescriptorNew->getActiveBand( band ), COMPLEX_MAGNITUDE ) );
            }
         }
      }

      issea(TestUtilities::destroyWorkspaceWindow(pWindowSio));
      issea(TestUtilities::destroyWorkspaceWindow(pWindowNew));
      pWindowSio = NULL;
      pDataDescriptorSio = NULL;
      pDataDescriptorNew = NULL;

      return success;
   }
};

class NitfExportCornerCoordinatesTest : public TestCase
{
public:
   NitfExportCornerCoordinatesTest() : TestCase("NitfExportCornerCoordinatesTest") {}

   bool run()
   {
      bool success = true;

      // opens nine_large-i2.ntf, RPC georefs, exports a NITF chip of (100,100)->(400,400)

      issea(runWizard("NitfExportCornerCoordinates"));

      SpatialDataWindow *pNewWindow = TestUtilities::loadDataSet("BatchProcessing/data/"
         "NitfExportCornerCoordinates.ntf", "Auto Importer");
      issea(pNewWindow != NULL);
      RasterElement *pNewRasterElement = pNewWindow->getSpatialDataView()->getLayerList()->getPrimaryRasterElement();
      issearf(pNewRasterElement != NULL);
      DataDescriptor *pNewDd = pNewRasterElement->getDataDescriptor();
      issearf(pNewDd != NULL);
      RasterFileDescriptor *pNewFd = dynamic_cast<RasterFileDescriptor*>(
         pNewDd->getFileDescriptor());
      issearf(pNewFd != NULL);
      std::list<GcpPoint> newGcps = pNewFd->getGcps();
      issea(TestUtilities::destroyWorkspaceWindow(pNewWindow));

      // confirm the center/corner points match what's in nine_large-i2.ntf
      const double TOLERANCE = 0.0005; // precision in NITF corner points
      SpatialDataWindow *pSrcWindow = TestUtilities::loadDataSet(TestUtilities::getTestDataPath() +
         "GeoReference/nine_large-i2.ntf", "Auto Importer");
      issearf(pSrcWindow != NULL);
      RasterElement *pSrcRasterElement = pSrcWindow->getSpatialDataView()->getLayerList()->getPrimaryRasterElement();
      issea(pSrcRasterElement != NULL);
      issea( TestUtilities::runGeoRef(pSrcRasterElement, "RPC Georeference") );
      issearf( pSrcRasterElement->isGeoreferenced() );

      issea(gcpPresent(newGcps, pSrcRasterElement, LocationType(100, 100)));
      issea(gcpPresent(newGcps, pSrcRasterElement, LocationType(100, 400)));
      issea(gcpPresent(newGcps, pSrcRasterElement, LocationType(400, 400)));
      issea(gcpPresent(newGcps, pSrcRasterElement, LocationType(400, 100)));
      issea(gcpPresent(newGcps, pSrcRasterElement, LocationType(250, 250)));

      issea(TestUtilities::destroyWorkspaceWindow(pSrcWindow));
      return success;

   }

   bool gcpPresent(const std::list<GcpPoint> &gcps, const RasterElement *pGeoreference,
      LocationType pixel)
   {
      if (pGeoreference == NULL)
      {
         return false;
      }

      const double PIXEL_TOLERANCE = 1e-8;
      const double GEO_TOLERANCE = 0.0005; // precision in NITF corner points
      LocationType geo = pGeoreference->convertPixelToGeocoord(pixel);

      for (std::list<GcpPoint>::const_iterator iter = gcps.begin();
         iter != gcps.end(); ++iter)
      {
         const GcpPoint &gcp = *iter;
         if ((fabs(geo.mX - gcp.mCoordinate.mX) < GEO_TOLERANCE) &&
            (fabs(geo.mY - gcp.mCoordinate.mY) < GEO_TOLERANCE) &&
            (fabs(pixel.mX - gcp.mPixel.mX) < PIXEL_TOLERANCE) &&
            (fabs(pixel.mY - gcp.mPixel.mY) < PIXEL_TOLERANCE))
         {
            return true;
         }
      }

      return false;
   }
};

class BatchProcessingCleanup : public TestCase
{
public:
   BatchProcessingCleanup() : TestCase("Cleanup") {}
   bool run()
   {
      bool success = true;

#ifdef UNIX_API
      issea( system( "rm -r BatchProcessing/data" ) == EXIT_SUCCESS );
#else
      issea( system( "rmdir /S /Q BatchProcessing\\data" ) == EXIT_SUCCESS );
#endif

      if( !success )
      {
         printf( "    -Cleanup failed, please manually remove the data directory\n" );
      }
      return success;
   }
};

class BatchProcessingTestSuite : public TestSuiteNewSession
{
public:
   BatchProcessingTestSuite() : TestSuiteNewSession( "Batch" )
   {
      addTestCase( new BandMathBatchProcessingTest );
      addTestCase( new ImportExportBatchProcessingTest );
      addTestCase( new NitfExportCornerCoordinatesTest );
      addTestCase( new BatchProcessingCleanup );
   }
};

REGISTER_SUITE( BatchProcessingTestSuite )
