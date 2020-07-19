/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ConfigurationSettings.h"
#include "DataAccessorImpl.h"
#include "DesktopServices.h"
#include "Executable.h"
#include "GcpList.h"
#include "Georeference.h"
#include "ModelServices.h"
#include "PlugInArgList.h"
#include "PlugInResource.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterUtilities.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "TestBedTestUtilities.h"
#include "TestUtilities.h"
#include "ThresholdLayer.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtCore/QString>

#include <limits>

using namespace std;

bool TestUtilities::runGeoRef(RasterElement* pRasterElement, const std::string& pluginName)
{
   ExecutableResource pExecutable(pluginName);
   if (pExecutable.get() == NULL)
   {
      return false;
   }

   pExecutable->getInArgList().setPlugInArgValue(Executable::DataElementArg(), pRasterElement);

   if (pluginName == "GCP Georeference")
   {
      Service<ModelServices> pModel;

      GcpList* pList = dynamic_cast<GcpList*>(pModel->getElement("Corner Coordinates",
         TypeConverter::toString<GcpList>(), pRasterElement));
      if (pList != NULL)
      {
         // leave only the first 3 points in the gcp list
         std::list<GcpPoint> pointList = pList->getSelectedPoints();
         if (pointList.size() > 3)
         {
            for (unsigned int x = 0; x < 3; ++x)
            {
               pointList.pop_front();
            }
         }

         pList->removePoints(pointList);
         pExecutable->getInArgList().setPlugInArgValue<GcpList>(Georeference::GcpListArg(), pList);
      }

      int order = 1;
      pExecutable->getInArgList().setPlugInArgValue("Order", &order);
   }

   return pExecutable->execute();
}

ThresholdLayer* TestUtilities::createThresholdLayer( RasterElement* pRasterElement, unsigned char threshold, int valuesOver )
{
   if( pRasterElement == NULL )
   {
      return NULL;
   }

   if( threshold == numeric_limits<unsigned char>::max() )
   {
      return NULL;
   }

   Service<ApplicationServices> pApplication;
   if( pApplication.get() == NULL )
   {
      return NULL;
   }

   ObjectFactory *pFact = NULL;
   pFact = pApplication->getObjectFactory();
   if( pFact == NULL )
   {
      return NULL;
   }

   RasterDataDescriptor *pDescriptor = NULL;
   pDescriptor = dynamic_cast<RasterDataDescriptor*>( pRasterElement->getDataDescriptor() );
   if( pDescriptor == NULL )
   {
      return NULL;
   }

   SpatialDataWindow *pWindow = NULL;
   pWindow = dynamic_cast<SpatialDataWindow*>( Service<DesktopServices>()->getWindow( pRasterElement->getName(), SPATIAL_DATA_WINDOW ) );
   if( pWindow == NULL )
   {
      return NULL;
   }

   SpatialDataView *pView = NULL;
   pView = dynamic_cast<SpatialDataView*>( pWindow->getView() );
   if( pView == NULL )
   {
      return NULL;
   }

   unsigned int numRows = pDescriptor->getRowCount();
   unsigned int numCols = pDescriptor->getColumnCount();

   RasterDataDescriptor *pRMDescriptor = RasterUtilities::generateRasterDataDescriptor(
      "testMatrix", pRasterElement, numRows, numCols, 1, BSQ, INT1UBYTE, IN_MEMORY);
   if (pRMDescriptor == NULL)
   {
      return NULL;
   }
   ModelResource<RasterElement> pRMData(pRMDescriptor);
   if (pRMData.get() == NULL)
   {
      return NULL;
   }
   pRMDescriptor = NULL;

   if( valuesOver < 0 )
   {
      // fill the raster element with random data from 0 - 255
      DataAccessor da = pRMData->getDataAccessor();
      for( unsigned int row = 0; row < numRows; row++ )
      {
         if( !da.isValid() )
         {
            return NULL;
         }
         for( unsigned int col = 0; col < numCols; col++ )
         {
            unsigned char* pData = reinterpret_cast<unsigned char*>( da->getColumn() );
            *pData = rand() % numeric_limits<unsigned char>::max();
            da->nextColumn();
         }
         da->nextRow();
      }
   }
   else
   {
      // fill the raster element with random data from 0 - 255
      unsigned int value = 0;
      unsigned int max = 256;
      DataAccessor da = pRMData->getDataAccessor();
      for( unsigned int row = 0; row < numRows; row++ )
      {
         if( !da.isValid() )
         {
            return NULL;
         }
         for( unsigned int col = 0; col < numCols; col++ )
         {
            unsigned char* pData = reinterpret_cast<unsigned char*>( da->getColumn() );
            value = rand() % max;
            *pData = value;
            if( value > threshold )
            {
               //if we generated a value over the threshold,
               //decrease the number of these that we require.
               valuesOver--;

               if( valuesOver == 0 )
               {
                  max = threshold;
               }
            }
            da->nextColumn();
         }
         da->nextRow();
      }

      //if we still need values over the threshold
      //generate them and put into random positions in the data.
      if( valuesOver > 0 )
      {
         unsigned int randRow = 0;
         unsigned int randCol = 0;
         unsigned char value = 0;
         DataAccessor da2 = pRMData->getDataAccessor();
         unsigned int tries = 0;
         for( unsigned int count = 0; count < static_cast<unsigned int>( valuesOver ) &&
            ( tries < static_cast<unsigned int>( valuesOver ) * 3 ); tries++ )
         {
            randRow = rand() % numRows;
            randCol = rand() % numCols;
            da2->toPixel( randRow, randCol );
            if( !da2.isValid() )
            {
               return NULL;
            }
            unsigned char* pData = reinterpret_cast<unsigned char*>( da2->getColumn() );
            value = *pData;
            //make sure random value is below threshold, before
            //we count it as creating a value over the threshold
            if( value < threshold )
            {
               value = ( rand() % ( numeric_limits<unsigned char>::max() - threshold) ) + threshold;
               *pData = value;
               count++;
            }
         }
      }
   }

   // now add the raster element to the viewer as a threshold layer
   ThresholdLayer *pLayer = NULL;
   pLayer = dynamic_cast<ThresholdLayer*>( pView->createLayer( THRESHOLD, pRMData.release(), "resultsMatrix" ) );
   if( pLayer == NULL )
   {
      return NULL;
   }
   pLayer->setFirstThreshold( threshold );
   pLayer->setPassArea( UPPER );
   pLayer->setRegionUnits(  RAW_VALUE );

   return pLayer;
}

vector<DataElement*> TestUtilities::loadDataSet(const string& cubeName, const string& importerName,
                                                const unsigned int& minNumCubes)
{
   ImporterResource importCube(importerName, cubeName, NULL, false);
   vector<DataElement*> cubes;

   if (importCube->execute() == true)
   {
      QCoreApplication::processEvents();

      cubes = importCube->getImportedElements();
      if (cubes.size() < minNumCubes)
      {
         cubes.clear();
      }
      else
      {
         for (vector<DataElement*>::const_iterator iter = cubes.begin(); iter != cubes.end(); iter++)
         {
            if (dynamic_cast<RasterElement*>(*iter) == NULL)
            {
               cubes.clear();
               break;
            }
         }
      }
   }

   return cubes;
}

SpatialDataWindow* TestUtilities::loadDataSet(const string& cubeName, const string& importerName,
                                              SpatialDataWindow* pWindow)
{
   ImporterResource importCube(importerName, cubeName, NULL, false);
   if (!importCube->execute())
   {
      return NULL;
   }

   QCoreApplication::processEvents();

   vector<DataElement*> cubes = importCube->getImportedElements();
   if (cubes.size() != 1)
   {
      return NULL;
   }

   RasterElement* pRasterElement = dynamic_cast<RasterElement*>(cubes.front());
   if (pRasterElement == NULL)
   {
      return NULL;
   }

   if (pWindow == NULL)
   {
      Service<DesktopServices> pDesktop;
      pWindow = dynamic_cast<SpatialDataWindow*>(pDesktop->getWindow(cubeName, SPATIAL_DATA_WINDOW));
      if (pWindow == NULL)
      {
         pWindow = dynamic_cast<SpatialDataWindow*>
            (pDesktop->getWindow(pRasterElement->getName(), SPATIAL_DATA_WINDOW));
      }
   }

   return pWindow;
}

RasterElement* TestUtilities::getStandardRasterElement(bool cleanLoad, bool loadFromTempDir)
{
   string dataFileName = "StandardR03.bip";
   string dataFileToLoad = "StandardR03.hdr";
   string path = getTestDataPath();
   if (loadFromTempDir)
   {
      const Filename* pTempPath = ConfigurationSettings::getSettingTempPath();
      if (pTempPath == NULL)
      {
         return NULL;
      }

      // Copy the files to the temp dir. QFile::copy fails if the file already exists, so do not check its return value.
      // Make sure that each file is writable so that removing the temp dir after the run is not a problem.
      path = pTempPath->getFullPathAndName() + "/";
      QString tempDataFileName = QString::fromStdString(path + dataFileName);
      QFile::copy(QString::fromStdString(getTestDataPath() + dataFileName), tempDataFileName);
      QFile::setPermissions(tempDataFileName, QFile::ReadOwner | QFile::WriteOwner);

      QString tempDataFileToLoad = QString::fromStdString(path + dataFileToLoad);
      QFile::copy(QString::fromStdString(getTestDataPath() + dataFileToLoad), tempDataFileToLoad);
      QFile::setPermissions(tempDataFileToLoad, QFile::ReadOwner | QFile::WriteOwner);
   }

   dataFileName = path + dataFileName;
   dataFileToLoad = path + dataFileToLoad;
   RasterElement* pRasterElement = dynamic_cast<RasterElement*>( Service<ModelServices>()->getElement( dataFileName, "RasterElement", NULL ) );
   if (pRasterElement != NULL && !cleanLoad)
   {
      return pRasterElement;
   }
   if (cleanLoad)
   {
      Service<DesktopServices> pDesktop;
      WorkspaceWindow* pWindow = dynamic_cast<WorkspaceWindow*>(pDesktop->getWindow(dataFileName, SPATIAL_DATA_WINDOW));
      if (pWindow != NULL)
      {
         destroyWorkspaceWindow(pWindow);
      }

      pRasterElement = NULL;
   }

   ImporterResource importCube( "ENVI Importer", dataFileToLoad, NULL, false );
   if (!importCube->execute())
   {
      return NULL;
   }
   vector<DataElement*> cubes = importCube->getImportedElements();
   if (cubes.size() != 1)
   {
      return NULL;
   }
   return dynamic_cast<RasterElement*>(cubes[0]);
}

bool TestUtilities::destroyWorkspaceWindow(WorkspaceWindow* pWindow)
{
   if (pWindow == NULL)
   {
      return false;
   }

   if (Service<DesktopServices>()->deleteWindow(pWindow) == false)
   {
      return false;
   }

   // Deleting a workspace window only schedules the window for destruction, so force the window
   // to be destroyed to prevent another test from failing if trying to load the same data
   QCoreApplication::sendPostedEvents(NULL, QEvent::DeferredDelete);

   return true;
}

std::vector<TestSuiteFactory*>& TestUtilities::getFactoryVector()
{
   static std::vector<TestSuiteFactory*> sFactories;
   return sFactories;
}
