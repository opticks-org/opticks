/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <sys/stat.h>

#include "AppVersion.h"
#include "SioImporter.h"
#include "DimensionDescriptor.h"
#include "Endian.h"
#include "FileResource.h"
#include "GcpList.h"
#include "ImportDescriptor.h"
#include "MessageLogResource.h"
#include "ModelServices.h"
#include "ObjectResource.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "RasterElement.h"
#include "RasterDataDescriptor.h"
#include "RasterFileDescriptor.h"
#include "RasterUtilities.h"
#include "Sio.h"
#include "SpecialMetadata.h"
#include "Statistics.h"
#include "TestDataPath.h"
#include "Units.h"

using namespace std;

SioImporter::SioImporter()
: mVersion9Sio(false)
{
   setName("SIO Importer");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setExtensions("Standard Format Files (*.sio)");
   setDescriptorId("{CB23148B-76E7-490d-95E2-3F62C3F2051D}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

SioImporter::~SioImporter()
{
}

vector<ImportDescriptor*> SioImporter::getImportDescriptors(const string& filename)
{
   vector<ImportDescriptor*> descriptors;
   if (filename.empty() == false)
   {
      // Read the header values
      FileResource pFile(filename.c_str(), "rb");

      SioFile sioFile;
      bool bSuccess = sioFile.deserialize(pFile.get());
      if (bSuccess == false)
      {
         return descriptors;
      }

      if (sioFile.mOriginalVersion == 9)
      {
         mVersion9Sio = true;
      }


      // Create the import descriptor
      ImportDescriptor* pImportDescriptor = mpModel->createImportDescriptor(filename, "RasterElement", NULL);
      if (pImportDescriptor != NULL)
      {
         RasterDataDescriptor* pDescriptor =
            dynamic_cast<RasterDataDescriptor*>(pImportDescriptor->getDataDescriptor());
         if (pDescriptor != NULL)
         {
            FactoryResource<RasterFileDescriptor> pFileDescriptor;
            if (pFileDescriptor.get() != NULL)
            {
               // Filename
               pFileDescriptor->setFilename(filename);

               // Endian
               pFileDescriptor->setEndian(sioFile.mEndian);

               // Rows
               vector<DimensionDescriptor> rows;
               for (int i = 0; i < sioFile.mRows; ++i)
               {
                  DimensionDescriptor rowDim;

                  // Do not set an active number since the user has not selected the rows to load
                  if (static_cast<unsigned int>(i) < sioFile.mOrigRowNumbers.size())
                  {
                     rowDim.setOriginalNumber(sioFile.mOrigRowNumbers[i]);
                  }
                  else
                  {
                     rowDim.setOriginalNumber(i);
                  }

                  rowDim.setOnDiskNumber(i);
                  rows.push_back(rowDim);
               }

               pDescriptor->setRows(rows);
               pFileDescriptor->setRows(rows);

               // Columns
               vector<DimensionDescriptor> columns;
               for (int i = 0; i < sioFile.mColumns; ++i)
               {
                  DimensionDescriptor columnDim;

                  // Do not set an active number since the user has not selected the rows to load
                  if (static_cast<unsigned int>(i) < sioFile.mOrigColumnNumbers.size())
                  {
                     columnDim.setOriginalNumber(sioFile.mOrigColumnNumbers[i]);
                  }
                  else
                  {
                     columnDim.setOriginalNumber(i);
                  }

                  columnDim.setOnDiskNumber(i);
                  columns.push_back(columnDim);
               }

               pDescriptor->setColumns(columns);
               pFileDescriptor->setColumns(columns);

               // Bands
               vector<DimensionDescriptor> bands;
               for (int i = 0; i < (sioFile.mBands - sioFile.mBadBands); ++i)
               {
                  DimensionDescriptor bandDim;
                  // Do not set an active number since the user has not selected the rows to load
                  if (static_cast<unsigned int>(i) < sioFile.mOrigBandNumbers.size())
                  {
                     bandDim.setOriginalNumber(sioFile.mOrigBandNumbers[i]);
                  }
                  else
                  {
                     bandDim.setOriginalNumber(i);
                  }

                  bandDim.setOnDiskNumber(i);
                  bands.push_back(bandDim);
               }

               pDescriptor->setBands(bands);
               pFileDescriptor->setBands(bands);

               // Bits per pixel
               pFileDescriptor->setBitsPerElement(sioFile.mBitsPerElement);

               // Data type
               pDescriptor->setDataType(sioFile.mDataType);

               // Interleave format
               pDescriptor->setInterleaveFormat(BIP);
               pFileDescriptor->setInterleaveFormat(BIP);

               // Bad values
               if (sioFile.mBadValues.empty() == true)
               {
                  if ((sioFile.mDataType != FLT4BYTES) && (sioFile.mDataType != FLT8COMPLEX) &&
                     (sioFile.mDataType != FLT8BYTES))
                  {
                     vector<int> badValues;
                     badValues.push_back(0);

                     pDescriptor->setBadValues(badValues);
                  }
               }

               // Header bytes
               pFileDescriptor->setHeaderBytes(28);

               // Trailer bytes
               struct stat statBuffer;
               if (stat(filename.c_str(), &statBuffer) == 0)
               {
                  double dataBytes = 28 + (sioFile.mRows * sioFile.mColumns * (sioFile.mBands - sioFile.mBadBands) *
                     (sioFile.mBitsPerElement / 8));
                  pFileDescriptor->setTrailerBytes(static_cast<unsigned int>(statBuffer.st_size - dataBytes));
               }

               // Units
               FactoryResource<Units> pUnits;
               pUnits->setUnitType(sioFile.mUnitType);
               pUnits->setUnitName(sioFile.mUnitName);
               pUnits->setRangeMin(sioFile.mRangeMin);
               pUnits->setRangeMax(sioFile.mRangeMax);
               pUnits->setScaleFromStandard(sioFile.mScale);

               pDescriptor->setUnits(pUnits.get());
               pFileDescriptor->setUnits(pUnits.get());

               // GCPs
               GcpPoint gcpLowerLeft;
               gcpLowerLeft.mPixel.mX = 0.0;
               gcpLowerLeft.mPixel.mY = 0.0;

               GcpPoint gcpLowerRight;
               gcpLowerRight.mPixel.mX = sioFile.mColumns - 1.0;
               gcpLowerRight.mPixel.mY = 0.0;

               GcpPoint gcpUpperLeft;
               gcpUpperLeft.mPixel.mX = 0.0;
               gcpUpperLeft.mPixel.mY = sioFile.mRows - 1.0;

               GcpPoint gcpUpperRight;
               gcpUpperRight.mPixel.mX = sioFile.mColumns - 1.0;
               gcpUpperRight.mPixel.mY = sioFile.mRows - 1.0;

               GcpPoint gcpCenter;
               gcpCenter.mPixel.mX = sioFile.mColumns / 2.0 - 0.5;
               gcpCenter.mPixel.mY = sioFile.mRows / 2.0 - 0.5;

               bool bValidGcps = false;
               for (int i = ORIGINAL_SENSOR; i < INVALID_LAST_ENUM_ITEM_FLAG; ++i)
               {
                  if (sioFile.mParameters[i].eParameter_Initialized == true)
                  {
                     switch (i)
                     {
                        case UPPER_LEFT_CORNER_LAT:
                           if ((sioFile.mVersion == 5) || (sioFile.mVersion == 6))
                           {
                              gcpUpperLeft.mCoordinate.mY = sioFile.mParameters[i].uParameter_Value.dData;
                           }
                           else if ((sioFile.mVersion == 7) || (sioFile.mVersion == 8))
                           {
                              gcpUpperLeft.mCoordinate.mX = sioFile.mParameters[i].uParameter_Value.dData;
                           }
                           bValidGcps = true;
                           break;

                        case UPPER_LEFT_CORNER_LONG:
                           if ((sioFile.mVersion == 5) || (sioFile.mVersion == 6))
                           {
                              gcpUpperLeft.mCoordinate.mX = sioFile.mParameters[i].uParameter_Value.dData;
                           }
                           else if ((sioFile.mVersion == 7) || (sioFile.mVersion == 8))
                           {
                              gcpUpperLeft.mCoordinate.mY = sioFile.mParameters[i].uParameter_Value.dData;
                           }
                           bValidGcps = true;
                           break;

                        case LOWER_LEFT_CORNER_LAT:
                           if ((sioFile.mVersion == 5) || (sioFile.mVersion == 6))
                           {
                              gcpLowerLeft.mCoordinate.mY = sioFile.mParameters[i].uParameter_Value.dData;
                           }
                           else if ((sioFile.mVersion == 7) || (sioFile.mVersion == 8))
                           {
                              gcpLowerLeft.mCoordinate.mX = sioFile.mParameters[i].uParameter_Value.dData;
                           }
                           bValidGcps = true;
                           break;

                        case LOWER_LEFT_CORNER_LONG:
                           if ((sioFile.mVersion == 5) || (sioFile.mVersion == 6))
                           {
                              gcpLowerLeft.mCoordinate.mX = sioFile.mParameters[i].uParameter_Value.dData;
                           }
                           else if ((sioFile.mVersion == 7) || (sioFile.mVersion == 8))
                           {
                              gcpLowerLeft.mCoordinate.mY = sioFile.mParameters[i].uParameter_Value.dData;
                           }
                           bValidGcps = true;
                           break;

                        case UPPER_RIGHT_CORNER_LAT:
                           if ((sioFile.mVersion == 5) || (sioFile.mVersion == 6))
                           {
                              gcpUpperRight.mCoordinate.mY = sioFile.mParameters[i].uParameter_Value.dData;
                           }
                           else if ((sioFile.mVersion == 7) || (sioFile.mVersion == 8))
                           {
                              gcpUpperRight.mCoordinate.mX = sioFile.mParameters[i].uParameter_Value.dData;
                           }
                           bValidGcps = true;
                           break;

                        case UPPER_RIGHT_CORNER_LONG:
                           if ((sioFile.mVersion == 5) || (sioFile.mVersion == 6))
                           {
                              gcpUpperRight.mCoordinate.mX = sioFile.mParameters[i].uParameter_Value.dData;
                           }
                           else if ((sioFile.mVersion == 7) || (sioFile.mVersion == 8))
                           {
                              gcpUpperRight.mCoordinate.mY = sioFile.mParameters[i].uParameter_Value.dData;
                           }
                           bValidGcps = true;
                           break;

                        case LOWER_RIGHT_CORNER_LAT:
                           if ((sioFile.mVersion == 5) || (sioFile.mVersion == 6))
                           {
                              gcpLowerRight.mCoordinate.mY = sioFile.mParameters[i].uParameter_Value.dData;
                           }
                           else if ((sioFile.mVersion == 7) || (sioFile.mVersion == 8))
                           {
                              gcpLowerRight.mCoordinate.mX = sioFile.mParameters[i].uParameter_Value.dData;
                           }
                           bValidGcps = true;
                           break;

                        case LOWER_RIGHT_CORNER_LONG:
                           if ((sioFile.mVersion == 5) || (sioFile.mVersion == 6))
                           {
                              gcpLowerRight.mCoordinate.mX = sioFile.mParameters[i].uParameter_Value.dData;
                           }
                           else if ((sioFile.mVersion == 7) || (sioFile.mVersion == 8))
                           {
                              gcpLowerRight.mCoordinate.mY = sioFile.mParameters[i].uParameter_Value.dData;
                           }
                           bValidGcps = true;
                           break;

                        case CENTER_LAT:
                           if ((sioFile.mVersion == 5) || (sioFile.mVersion == 6))
                           {
                              gcpCenter.mCoordinate.mY = sioFile.mParameters[i].uParameter_Value.dData;
                           }
                           else if ((sioFile.mVersion == 7) || (sioFile.mVersion == 8))
                           {
                              gcpCenter.mCoordinate.mX = sioFile.mParameters[i].uParameter_Value.dData;
                           }
                           bValidGcps = true;
                           break;

                        case CENTER_LONG:
                           if ((sioFile.mVersion == 5) || (sioFile.mVersion == 6))
                           {
                              gcpCenter.mCoordinate.mX = sioFile.mParameters[i].uParameter_Value.dData;
                           }
                           else if ((sioFile.mVersion == 7) || (sioFile.mVersion == 8))
                           {
                              gcpCenter.mCoordinate.mY = sioFile.mParameters[i].uParameter_Value.dData;
                           }
                           bValidGcps = true;
                           break;

                        default:
                           break;
                     }
                  }
               }

               if (bValidGcps == true)
               {
                  list<GcpPoint> gcps;
                  gcps.push_back(gcpLowerLeft);
                  gcps.push_back(gcpLowerRight);
                  gcps.push_back(gcpUpperLeft);
                  gcps.push_back(gcpUpperRight);
                  gcps.push_back(gcpCenter);

                  pFileDescriptor->setGcps(gcps);
               }

               // Classification
               pDescriptor->setClassification(sioFile.mpClassification.get());

               // Metadata
               pDescriptor->setMetadata(sioFile.mpMetadata.get());

               DynamicObject* pMetadata = pDescriptor->getMetadata();
               if (pMetadata != NULL)
               {
                  vector<double> startWavelengths(sioFile.mStartWavelengths.size());
                  copy(sioFile.mStartWavelengths.begin(), sioFile.mStartWavelengths.end(), startWavelengths.begin());
                  vector<double> endWavelengths(sioFile.mEndWavelengths.size());
                  copy(sioFile.mEndWavelengths.begin(), sioFile.mEndWavelengths.end(), endWavelengths.begin());
                  vector<double> centerWavelengths(sioFile.mCenterWavelengths.size());
                  copy(sioFile.mCenterWavelengths.begin(), sioFile.mCenterWavelengths.end(), centerWavelengths.begin());

                  string pStartPath[] = { SPECIAL_METADATA_NAME, BAND_METADATA_NAME,
                     START_WAVELENGTHS_METADATA_NAME, END_METADATA_NAME };
                  pMetadata->setAttributeByPath(pStartPath, startWavelengths);
                  string pEndPath[] = { SPECIAL_METADATA_NAME, BAND_METADATA_NAME,
                     END_WAVELENGTHS_METADATA_NAME, END_METADATA_NAME };
                  pMetadata->setAttributeByPath(pEndPath, endWavelengths);
                  string pCenterPath[] = { SPECIAL_METADATA_NAME, BAND_METADATA_NAME,
                     CENTER_WAVELENGTHS_METADATA_NAME, END_METADATA_NAME };
                  pMetadata->setAttributeByPath(pCenterPath, centerWavelengths);
               }

               // File descriptor
               pDescriptor->setFileDescriptor(pFileDescriptor.get());
            }
         }

         descriptors.push_back(pImportDescriptor);
      }
   }

   return descriptors;
}

unsigned char SioImporter::getFileAffinity(const string& filename)
{
   if (getImportDescriptors(filename).empty())
   {
      return Importer::CAN_NOT_LOAD;
   }
   else
   {
      return Importer::CAN_LOAD;
   }
}

bool SioImporter::validate(const DataDescriptor* pDescriptor, string& errorMessage) const
{
   if (mVersion9Sio)
   {
      //issue a warning about version 9 sio's
      errorMessage = "This SIO file was created with a development version of the software.  "
         "This file is not officially supported and should not be used in a production environment.";
   }
   else
   {
      errorMessage = "SIO files are deprecated.  This file will still be loaded, but the importer "
         "may be removed in the future.  Please export the data using the Ice Exporter.";
   }

   string baseErrorMessage;

   bool bValid = RasterElementImporterShell::validate(pDescriptor, baseErrorMessage);
   if (baseErrorMessage.empty() == false)
   {
      errorMessage += string("\n") + baseErrorMessage;
   }

   if (bValid == false)
   {
      return false;
   }

   const RasterDataDescriptor* pRasterDescriptor = dynamic_cast<const RasterDataDescriptor*>(pDescriptor);
   if (pRasterDescriptor == NULL)
   {
      errorMessage += "\nThe data descriptor is invalid!";
      return false;
   }

   const RasterFileDescriptor* pFileDescriptor =
      dynamic_cast<const RasterFileDescriptor*>(pRasterDescriptor->getFileDescriptor());
   if (pFileDescriptor == NULL)
   {
      errorMessage += "\nThe file descriptor is invalid!";
      return false;
   }

   // Processing location restrictions
   ProcessingLocation processingLocation = pRasterDescriptor->getProcessingLocation();
   if (processingLocation != IN_MEMORY)
   {
      // Multiple band files
      const vector<const Filename*>& bandFiles = pFileDescriptor->getBandFiles();
      if (bandFiles.empty() == false)
      {
         errorMessage += "\nOn-disk processing is not supported with multiple files!";
         return false;
      }
   }

   return true;
}

bool SioImporter::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   // Read the statistics from the file and set into the raster element
   RasterElement* pRaster = NULL;
   if (pInArgList != NULL)
   {
      pRaster = pInArgList->getPlugInArgValue<RasterElement>(ImportElementArg());
   }

   if (pRaster != NULL)
   {
      // Get the filename
      string filename = pRaster->getFilename();

      // Read the file
      FileResource pFile(filename.c_str(), "rb");

      SioFile sioFile;
      if (sioFile.deserialize(pFile.get()) == true)
      {
         if (sioFile.mOriginalVersion == 9 && isBatch())
         {
            //Since version 9 sio's are not officially supported
            //don't load them in batch to force users to load them
            //interactively which will show them the reason why.
            return false;
         }

         const RasterDataDescriptor* pDescriptor =
            dynamic_cast<const RasterDataDescriptor*>(pRaster->getDataDescriptor());

         if (pDescriptor != NULL && 
            RasterUtilities::isSubcube(pDescriptor, false) == false &&
            Service<SessionManager>()->isSessionLoading() == false)
         {
            const vector<DimensionDescriptor>& bands = pDescriptor->getBands();
            for (unsigned int i = 0; i < bands.size(); ++i)
            {
               Statistics* pStatistics = pRaster->getStatistics(bands[i]);
               if (pStatistics != NULL)
               {
                  // Bad values
                  if (i < sioFile.mBadValues.size())
                  {
                     vector<int> badValues = sioFile.mBadValues[i];
                     pStatistics->setBadValues(badValues);
                  }

                  // Min
                  if (i < sioFile.mStatMin.size())
                  {
                     double dMin = sioFile.mStatMin[i];
                     pStatistics->setMin(dMin);
                  }

                  // Max
                  if (i < sioFile.mStatMax.size())
                  {
                     double dMax = sioFile.mStatMax[i];
                     pStatistics->setMax(dMax);
                  }

                  // Average
                  if (i < sioFile.mStatAvg.size())
                  {
                     double dAverage = sioFile.mStatAvg[i];
                     pStatistics->setAverage(dAverage);
                  }

                  // Standard deviation
                  if (i < sioFile.mStatStdDev.size())
                  {
                     double dStdDev = sioFile.mStatStdDev[i];
                     pStatistics->setStandardDeviation(dStdDev);
                  }

                  // Percentiles
                  if (i < sioFile.mStatPercentile.size())
                  {
                     double* pPercentiles = sioFile.mStatPercentile[i];
                     pStatistics->setPercentiles(pPercentiles);
                  }

                  // Histogram
                  if ((i < sioFile.mStatBinCenter.size()) && (i < sioFile.mStatHistogram.size()))
                  {
                     double* pBinCenters = sioFile.mStatBinCenter[i];
                     unsigned int* pCounts = sioFile.mStatHistogram[i];
                     pStatistics->setHistogram(pBinCenters, pCounts);
                  }
               }
            }
         }
      }
   }

   return RasterElementImporterShell::execute(pInArgList, pOutArgList);
}

bool assertionLogger(std::ostream &outputer, std::string code, std::string file, int line)
{
   outputer << "Assertion failed: '" << code << "', File: " << file << ", Line: " << line << std::endl;
   return false;
}

#define testableAssert(outputer,code) \
((code) ? true : assertionLogger(outputer, #code, __FILE__, __LINE__))

#define issea(outputer,x) \
   if (success) \
   { \
      success &= testableAssert(outputer, x); \
   }

bool SioImporter::runOperationalTests(Progress *pProgress, std::ostream& failure)
{
   return true;
}

bool SioImporter::ensureStatisticsReadProperly(Progress *pProgress, std::ostream& failure)
{
   bool success = true;
   success &= setBatch();
   PlugInArgList* pInList = NULL;
   PlugInArgList* pOutList = NULL;
   issea(failure, getInputSpecification(pInList) != false);
   issea(failure, getOutputSpecification(pOutList) != false);
   PlugInArg* pRasterElementArg = NULL;
   issea(failure, pInList->getArg(ImportElementArg(), pRasterElementArg) != false);

   string testFilePath = getTestDataPath() + "tipjul5bands.sio";

   RasterElement* pRasterElement = NULL;
   if (success)
   {
      vector<ImportDescriptor*> descriptors = getImportDescriptors(testFilePath);
      if (descriptors.empty() == false)
      {
         ImportDescriptor* pImportDescriptor = descriptors.front();
         if (pImportDescriptor != NULL)
         {
            DataDescriptor* pDescriptor = pImportDescriptor->getDataDescriptor();
            if (pDescriptor != NULL)
            {
               Service<ModelServices> pModel;
               pRasterElement = dynamic_cast<RasterElement*>(pModel->createElement(pDescriptor));
               if (pRasterElement != NULL)
               {
                  pRasterElementArg->setActualValue(pRasterElement);
               }
            }
         }
      }
   }

   issea(failure, execute(pInList, pOutList) != false);
   issea(failure, pRasterElement != NULL);
   if (success)
   {
      RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(pRasterElement->getDataDescriptor());
      issea(failure, pDescriptor != NULL);

      const vector<DimensionDescriptor>& loadedBands = pDescriptor->getBands();
      issea(failure, loadedBands.size() == 5);
      int iNumBandsWithStats = 0;
      for (int i = 0; i < 5; ++i)
      {
         // we don't want to do an assert yet... only when we know 4 bands have computed statistics
         Statistics* pStatistics = pRasterElement->getStatistics(loadedBands[i]);
         if (pStatistics != NULL)
         {
            if (pStatistics->areStatisticsCalculated() == true)
            {
               if (success)
               {
                  iNumBandsWithStats++;
               }
            }
         }
      }

      // success of the band computation is dependent on 4 bands with statistics
      issea(failure, iNumBandsWithStats == 3);
   }
   if (pRasterElement != NULL)
   {
      Service<ModelServices> pModel;
      pModel->destroyElement(pRasterElement);
      pRasterElement = NULL;
   }
   Service<PlugInManagerServices> pPim;
   if (pInList)
   {
      pPim->destroyPlugInArgList(pInList);
   }

   if (pOutList)
   {
      pPim->destroyPlugInArgList(pOutList);
   }

   return success;
}

bool SioImporter::runAllTests(Progress *pProgress, std::ostream& failure)
{
   bool success = runOperationalTests(pProgress, failure);
   if (success)
   {
      success = ensureStatisticsReadProperly(pProgress, failure);
   }
   return success;
}
