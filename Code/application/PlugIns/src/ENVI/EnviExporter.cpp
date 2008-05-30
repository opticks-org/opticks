/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "AppConfig.h"

#if defined(UNIX_API)
#include <unistd.h>
#endif

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "AppVerify.h"
#include "Classification.h"
#include "DimensionDescriptor.h"
#include "EnviExporter.h"
#include "Filename.h"
#include "MessageLogResource.h"
#include "ModelServices.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "Progress.h"
#include "RasterElement.h"
#include "RasterDataDescriptor.h"
#include "RasterFileDescriptor.h"
#include "RasterUtilities.h"
#include "SpecialMetadata.h"
#include "TypesFile.h"
#include "Units.h"

#include <algorithm>
#include <vector>
using namespace std;

EnviExporter::EnviExporter() :
   mAbortFlag(false),
   mpProgress(NULL),
   mpRaster(NULL),
   mpFileDescriptor(NULL),
   mpStep(NULL)
{
   setName("ENVI Exporter");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setExtensions("Envi Header Files (*.hdr)");
   setSubtype(TypeConverter::toString<RasterElement>());
   setDescriptorId("{08D313EC-2AB2-4e66-B840-6102A2C8E30A}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

EnviExporter::~EnviExporter()
{
}

bool EnviExporter::getInputSpecification(PlugInArgList*& pArgList)
{
   pArgList = mpPlugInManager->getPlugInArgList();
   VERIFY(pArgList != NULL);

   PlugInArg* pArg = mpPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName(ProgressArg());
   pArg->setType("Progress");
   pArg->setDefaultValue(NULL);
   pArgList->addArg(*pArg);

   pArg = mpPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName(ExportItemArg());
   pArg->setType("RasterElement");
   pArg->setDefaultValue(NULL);
   pArgList->addArg(*pArg);

   pArg = mpPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName(ExportDescriptorArg());
   pArg->setType("RasterFileDescriptor");
   pArg->setDefaultValue(NULL);
   pArgList->addArg(*pArg);

   return true;
}

bool EnviExporter::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

bool EnviExporter::hasAbort()
{
   return true;
}

bool EnviExporter::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Execute ENVI Exporter", "app", "AFDB9430-4D60-447b-8406-6B1C58F8A027");
   mpStep = pStep.get();

   if (!extractInputArgs(pInArgList))
   {
      return false;
   }

   char *pInterleaves[3] = {"bsq", "bip", "bil"};
   string message = "";

   // Test for incompatible sensor data
   const RasterDataDescriptor* pDescriptor = dynamic_cast<const RasterDataDescriptor*>(mpRaster->getDataDescriptor());
   if (pDescriptor == NULL)
   {
      message = "Could not get the data descriptor!";
      if (mpProgress != NULL) mpProgress->updateProgress(message, 0, ERRORS);
      pStep->finalize(Message::Failure, message);
      return false;
   }

   const RasterFileDescriptor* pFileDescriptor =
      dynamic_cast<const RasterFileDescriptor*>(pDescriptor->getFileDescriptor());
   if (pFileDescriptor == NULL)
   {
      message = "Could not get the file descriptor! "
         "Possible reasons include an attempt to export data not loaded from a file "
         "or modified after the file has been loaded.";
      if (mpProgress != NULL) mpProgress->updateProgress(message, 0, ERRORS);
      pStep->finalize(Message::Failure, message);
      return false;
   }

   // Preline bytes
   if (pFileDescriptor->getPrelineBytes() > 0)
   {
      message = "An ENVI header cannot represent a data set with preline bytes.";
      if (mpProgress != NULL) mpProgress->updateProgress(message, 0, ERRORS);
      pStep->finalize(Message::Failure, message);
      return false;
   }

   // Postline bytes
   if (pFileDescriptor->getPostlineBytes() > 0)
   {
      message = "An ENVI header cannot represent a data set with postline bytes.";
      if (mpProgress != NULL) mpProgress->updateProgress(message, 0, ERRORS);
      pStep->finalize(Message::Failure, message);
      return false;
   }

   // Preband bytes
   if (pFileDescriptor->getPrebandBytes() > 0)
   {
      message = "An ENVI header cannot represent a data set with preband bytes.";
      if (mpProgress != NULL) mpProgress->updateProgress(message, 0, ERRORS);
      pStep->finalize(Message::Failure, message);
      return false;
   }

   // Postband bytes
   if (pFileDescriptor->getPostbandBytes() > 0)
   {
      message = "An ENVI header cannot represent a data set with postband bytes.";
      if (mpProgress != NULL) mpProgress->updateProgress(message, 0, ERRORS);
      pStep->finalize(Message::Failure, message);
      return false;
   }

   // Multiple files
   InterleaveFormatType interleave = pFileDescriptor->getInterleaveFormat();
   const vector<const Filename*>& bandFiles = pFileDescriptor->getBandFiles();

   if ((interleave == BSQ) && (bandFiles.empty() == false))
   {
      message = "An ENVI header cannot represent BSQ multi-file data.";
      if (mpProgress != NULL) mpProgress->updateProgress(message, 0, ERRORS);
      pStep->finalize(Message::Failure, message);
      return false;
   }

   //Determine if original file is formatted in raw format, so that ENVI import
   //will succeed.  If not, then reject the ENVI export
   const DynamicObject* pMetadata = pDescriptor->getMetadata();
   bool isOriginalDataRawFormatted = false;
   string originalFileKey = "Is_Original_File_Raw_Data";
   if (pMetadata != NULL)
   {
      const bool* pValue = pMetadata->getAttribute(originalFileKey).getPointerToValue<bool>();
      if (pValue != NULL)
      {
         isOriginalDataRawFormatted = *pValue;
      }
   }

   if (!isOriginalDataRawFormatted)
   {
      message = "An ENVI header cannot be written out because the original data file is stored in an incompatible manner.";
      if (mpProgress != NULL) mpProgress->updateProgress(message, 0, ERRORS);
      pStep->finalize(Message::Failure, message);
      return false;
   }

   const string& filename = mpFileDescriptor->getFilename();

   FILE* pStream = fopen(filename.c_str(), "wt");
   if (pStream == NULL)
   {
      message = "Unable to write to file:\n" + filename;
      if (mpProgress != NULL) mpProgress->updateProgress(message, 0, ERRORS);
      pStep->finalize(Message::Failure, message);
      return false;
   }

   // Get the data type
   int dataType = 0;

   EncodingType eDataType = pDescriptor->getDataType();
   switch(eDataType)
   {
      case INT1UBYTE:
      case INT1SBYTE:
         dataType = 1;
         break;
      case INT2UBYTES:
         dataType = 12;
         break;
      case INT2SBYTES:
         dataType = 2;
         break;
      case INT4SCOMPLEX:
         dataType = 99;    // Recognized only by this application
         break;
      case INT4UBYTES:
         dataType = 13;
         break;
      case INT4SBYTES:
         dataType = 3;
         break;
      case FLT4BYTES:
         dataType = 4;
         break;
      case FLT8COMPLEX:
         dataType = 6;
         break;
      case FLT8BYTES:
         dataType = 5;
         break;
      default:
         break;
   }

   message = "Start ENVI Export";
   if (mpProgress != NULL) mpProgress->updateProgress(message, 0, NORMAL);

   int i = fprintf(pStream, "ENVI\n");
   if (i > 0)
   {
      i = fprintf(pStream, "description = {\n");
   }
   if (i > 0)
   {
      string filename = mpRaster->getFilename();
      if (filename.empty() == false)
      {
         i = fprintf(pStream, "    FILENAME = %s\n", filename.c_str());
      }
   }
   if (i > 0)
   {
      const Classification *pClass = mpRaster->getClassification();
      if (pClass != NULL)
      {
         string classLevel = pClass->getLevel();
         if (classLevel != "U")
         {
            string level = "U";
            if (classLevel == "U")
            {
               level = "UNCLASSIFIED";
            }
            else if (classLevel == "C")
            {
               level = "CONFIDENTIAL";
            }
            else if (classLevel == "R")
            {
               level = "RESTRICTED";
            }
            else if (classLevel == "S")
            {
               level = "SECRET";
            }
            else if (classLevel == "T")
            {
               level = "TOP SECRET";
            }

            i = fprintf(pStream, "    CLASSIFICATION = %s\n", level.c_str());
         }
      }
   }

   if (pMetadata != NULL)
   {
      vector<string> metadataKeys;
      pMetadata->getAttributeNames(metadataKeys);

      string fieldType;
      for (vector<string>::iterator it = metadataKeys.begin(); it != metadataKeys.end(); ++it)
      {
         if (mAbortFlag)
         {
            message = "ENVI export aborted!";
            if (mpProgress != NULL) mpProgress->updateProgress(message, 0, ABORT);
            pStep->finalize(Message::Abort);

            fclose(pStream);
            remove(filename.c_str());
            return false;
         }

         if (i > 0)
         {
            string key = *it;
            if (!key.empty())
            {
               string value = pMetadata->getAttribute(key).toXmlString();
               if (!value.empty())
               {
                  if (i > 0)
                  {
                     i = fprintf(pStream, "    %s = %s\n", key.c_str(), value.c_str());
                  }
               }
            }
         }
      }
   }

   if (i > 0)
   {
      i = fprintf(pStream, "}\n");
   }

   if (mAbortFlag)
   {
      message = "ENVI export aborted!";
      if (mpProgress != NULL) mpProgress->updateProgress(message, 0, ABORT);
      pStep->finalize(Message::Abort);

      fclose(pStream);
      remove(filename.c_str());
      return false;
   }

   if (i > 0)
   {
      i = fprintf(pStream, "samples = %d\n", pFileDescriptor->getColumnCount());
   }
   if (i > 0)
   {
      i = fprintf(pStream, "lines = %d\n", pFileDescriptor->getRowCount());
   }
   if (i > 0)
   {
      i = fprintf(pStream, "bands = %d\n", pFileDescriptor->getBandCount());
   }
   if (i > 0)
   {
      i = fprintf(pStream, "header offset = %d\n", pFileDescriptor->getHeaderBytes());
   }
   if (i > 0)
   {
      i = fprintf(pStream, "file type = ENVI Standard\n");
   }
   if (i > 0)
   {
      i = fprintf(pStream, "data type = %d\n", dataType);
   }
   if (i > 0)
   {
      i = fprintf(pStream, "interleave = %s\n", pInterleaves[pFileDescriptor->getInterleaveFormat()]);
   }

   bool bMsb = false;
   if (pFileDescriptor->getEndian() == BIG_ENDIAN)
   {
      bMsb = true;
   }

   if (i > 0)
   {
      i = fprintf(pStream, "byte order = %d\n", bMsb);
   }

   if (i > 0)
   {
      DimensionDescriptor dimDesc = pFileDescriptor->getActiveColumn(0);
      if (dimDesc.isValid())
      {
         if (dimDesc.isOriginalNumberValid())
         {
            if (dimDesc.getOriginalNumber() > 0)
            {
               i = fprintf(pStream, "x start = %d\n", dimDesc.getOriginalNumber() + 1); // ENVI uses 1 based numbers
            }
         }
      }
   }
   if (i > 0)
   {
      DimensionDescriptor dimDesc = pFileDescriptor->getActiveRow(0);
      if (dimDesc.isValid())
      {
         if (dimDesc.isOriginalNumberValid())
         {
            if (dimDesc.getOriginalNumber() > 0)
            {
               i = fprintf(pStream, "y start = %d\n", dimDesc.getOriginalNumber() + 1);
            }
         }
      }
   }

   // geo points
   if (i > 0)
   {
      if (mpRaster->isGeoreferenced())
      {
         const vector<DimensionDescriptor>& rows = pFileDescriptor->getRows();
         const vector<DimensionDescriptor>& cols = pFileDescriptor->getColumns();
         if (!rows.empty() && !cols.empty())
         {
#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : This functionality should be moved into a method in a new RasterElementExporterShell class (dsulgrov)")
            list<GcpPoint> gcps;
            unsigned int startRow, startCol, endRow, endCol;
            startRow = rows.front().getActiveNumber();
            endRow = rows.back().getActiveNumber();
            startCol = cols.front().getActiveNumber();
            endCol = cols.back().getActiveNumber();
            GcpPoint urPoint, ulPoint, lrPoint, llPoint, centerPoint;
            ulPoint.mPixel = LocationType(startCol, startRow);
            urPoint.mPixel = LocationType(endCol, startRow);
            llPoint.mPixel = LocationType(startCol, endRow);
            lrPoint.mPixel = LocationType(endCol, endRow);

            ulPoint.mCoordinate = mpRaster->convertPixelToGeocoord(ulPoint.mPixel);
            urPoint.mCoordinate = mpRaster->convertPixelToGeocoord(urPoint.mPixel);
            llPoint.mCoordinate = mpRaster->convertPixelToGeocoord(llPoint.mPixel);
            lrPoint.mCoordinate = mpRaster->convertPixelToGeocoord(lrPoint.mPixel);

            //reset the coordinates, because on import they are required to be in
            //on-disk numbers not active numbers
            unsigned int diskStartRow, diskStartCol, diskEndRow, diskEndCol;
            diskStartRow = rows.front().getOnDiskNumber();
            diskEndRow = rows.back().getOnDiskNumber();
            diskStartCol = cols.front().getOnDiskNumber();
            diskEndCol = cols.back().getOnDiskNumber();
            ulPoint.mPixel = LocationType(diskStartCol, diskStartRow);
            urPoint.mPixel = LocationType(diskEndCol, diskStartRow);
            llPoint.mPixel = LocationType(diskStartCol, diskEndRow);
            lrPoint.mPixel = LocationType(diskEndCol, diskEndRow);

            gcps.push_back(ulPoint);
            gcps.push_back(urPoint);
            gcps.push_back(llPoint);
            gcps.push_back(lrPoint);

            list<GcpPoint>::const_iterator it;
            fprintf(pStream, "geo points = {");
            for (it=gcps.begin(); it!=gcps.end(); ++it)
            {
               GcpPoint gcp = *it;
               // add 1.5 to adjust from Opticks to ENVI pixel coordinate systems
               i = fprintf(pStream, "\n %.4f, %.4f, %.8f, %.8f", gcp.mPixel.mX + 1.5, 
                  gcp.mPixel.mY + 1.5, gcp.mCoordinate.mX, gcp.mCoordinate.mY);
            }
            i = fprintf(pStream, "}\n");
         }
      }
   }

   // reflectance scale factor
   if (i > 0)
   {
      const Units* pUnits = pDescriptor->getUnits();
      if (pUnits != NULL)
      {
         if (pUnits->getUnitType() == REFLECTANCE && 
            abs(pUnits->getScaleFromStandard() - 1.0) > 0.0000001)
         {
            float fltVal = static_cast<float>(1.0/pUnits->getScaleFromStandard());
            i = fprintf(pStream, "reflectance scale factor = %f\n", fltVal);
         }
      }
   }
   // Bad bands
   const vector<DimensionDescriptor>& allBands = pFileDescriptor->getBands();
   vector<DimensionDescriptor>::const_iterator allBandsIter;
   if (i > 0)
   {
      const vector<DimensionDescriptor>& activeBands = pDescriptor->getBands();
      if (allBands.size() != activeBands.size())
      {
         if (i > 0)
         {
            i = fprintf(pStream, "bbl = {\n");
         }

         unsigned int count = 0;
         for (allBandsIter = allBands.begin(); allBandsIter != allBands.end(); ++allBandsIter, ++count)
         {
            if (mAbortFlag)
            {
               message = "ENVI export aborted!";
               if (mpProgress != NULL) mpProgress->updateProgress(message, 0, ABORT);
               pStep->finalize(Message::Abort);

               fclose(pStream);
               remove(filename.c_str());
               return false;
            }

            if (allBandsIter->isActiveNumberValid())
            {
               if (i > 0)
               {
                  i = fprintf(pStream, "1");
               }
            }
            else
            {
               if (i > 0)
               {
                  i = fprintf(pStream, "0");
               }
            }

            if (allBandsIter == (allBands.end() - 1))
            {
               if (i > 0)
               {
                  i = fprintf(pStream, "}\n");
               }
            }
            else
            {
               if (i > 0)
               {
                  i = fprintf(pStream, ",");
               }
               if (i > 0)
               {
                  if ((count + 1) % 40 == 0)
                  {
                     i = fprintf(pStream, "\n");
                  }
               }
            }
         }
      }
   }

   // Wavelengths
   if (i > 0)
   {
      vector<double> centerWavelengths;
      vector<double> startWavelengths;
      vector<double> endWavelengths;
      const DynamicObject* pMetadata = pDescriptor->getMetadata();
      if (pMetadata != NULL)
      {
         string pCenterPath[] = { SPECIAL_METADATA_NAME, 
            BAND_METADATA_NAME, CENTER_WAVELENGTHS_METADATA_NAME, END_METADATA_NAME };
         const vector<double> *pWavelengthData = dv_cast<vector<double> >(
            &pMetadata->getAttributeByPath(pCenterPath));
         if (pWavelengthData != NULL)
         {
            centerWavelengths = *pWavelengthData;
         }
         string pStartPath[] = { SPECIAL_METADATA_NAME, 
            BAND_METADATA_NAME, START_WAVELENGTHS_METADATA_NAME, END_METADATA_NAME };
         pWavelengthData = dv_cast<vector<double> >(
            &pMetadata->getAttributeByPath(pStartPath));
         if (pWavelengthData != NULL)
         {
            startWavelengths = *pWavelengthData;
         }
         string pEndPath[] = { SPECIAL_METADATA_NAME, 
            BAND_METADATA_NAME, END_WAVELENGTHS_METADATA_NAME, END_METADATA_NAME };
         pWavelengthData = dv_cast<vector<double> >(
            &pMetadata->getAttributeByPath(pEndPath));
         if (pWavelengthData != NULL)
         {
            endWavelengths = *pWavelengthData;
         }
      }

      if (!centerWavelengths.empty())
      {
         i = fprintf(pStream, "wavelength units = Micrometers\n");

         if (i > 0)
         {
            i = fprintf (pStream, "wavelength = {\n");
         }

         for (allBandsIter = allBands.begin(); allBandsIter != allBands.end(); ++allBandsIter)
         {
            if (mAbortFlag)
            {
               message = "ENVI export aborted!";
               if (mpProgress != NULL) mpProgress->updateProgress(message, 0, ABORT);
               pStep->finalize(Message::Abort);

               fclose(pStream);
               remove(filename.c_str());
               return false;
            }
            if ((i > 0) && (allBandsIter != allBands.begin()))
            {
               i = fprintf(pStream, ",\n");      
            }
            double wavelength = 0.0;
            if (i > 0)
            {
               if (allBandsIter->isActiveNumberValid())
               {
                  unsigned int bandNumber = allBandsIter->getActiveNumber();
                  if (bandNumber < centerWavelengths.size())
                  {
                     wavelength = centerWavelengths[bandNumber];
                  }
               }
               i = fprintf(pStream, "%1.12g", wavelength);
            }
         }
         if (i > 0)
         {
            i = fprintf(pStream, "}");
         }
      }

      if (!centerWavelengths.empty())
      {
         if (i > 0)
         {
            i = fprintf(pStream, "\n");
         }
         if (i > 0)
         {
            i = fprintf(pStream, "fwhm = {\n");
         }

         if (!startWavelengths.empty() && !endWavelengths.empty())
         {
            for (allBandsIter = allBands.begin(); allBandsIter != allBands.end(); ++allBandsIter)
            {
               if (mAbortFlag)
               {
                  message = "ENVI export aborted!";
                  if (mpProgress != NULL) mpProgress->updateProgress(message, 0, ABORT);
                  pStep->finalize(Message::Abort);

                  fclose(pStream);
                  remove(filename.c_str());
                  return false;
               }

               if ((i > 0) && (allBandsIter != allBands.begin()))
               {
                  i = fprintf(pStream, ",\n");
               }
               if (i > 0)
               {
                  double fwhm = 0.0;
                  if (allBandsIter->isActiveNumberValid())
                  {
                     unsigned int bandNumber = allBandsIter->getActiveNumber();
                     if ((bandNumber < startWavelengths.size()) && (bandNumber < endWavelengths.size()))
                     {
                        fwhm = endWavelengths[bandNumber] - startWavelengths[bandNumber];
                     }
                  }
                  i = fprintf(pStream, "%1.12g", fwhm);
               }
            }
            if (i > 0)
            {
               i = fprintf(pStream, "}");
            }
         }
         else
         {
            vector<double> calculatedFwhm;
            calculatedFwhm.reserve(centerWavelengths.size());
            vector<double>::iterator wave;
            double value = 0.0;
            for (wave = centerWavelengths.begin(); wave != centerWavelengths.end(); ++wave)
            {
               if ((wave + 1) != centerWavelengths.end())
               {
                  double centerWavelength = *wave;
                  double nextCenterWavelength = *(wave + 1);
                  value = 1.3 * (nextCenterWavelength - centerWavelength);
                  calculatedFwhm.push_back(value);
               }
            }
            calculatedFwhm.push_back(value);

            for (allBandsIter = allBands.begin(); allBandsIter != allBands.end(); ++allBandsIter)
            {
               if (mAbortFlag)
               {
                  message = "ENVI export aborted!";
                  if (mpProgress != NULL) mpProgress->updateProgress(message, 0, ABORT);
                  pStep->finalize(Message::Abort);

                  fclose(pStream);
                  remove(filename.c_str());
                  return false;
               }

               if ((i > 0) && (allBandsIter != allBands.begin()))
               {
                  i = fprintf(pStream, ",\n");
               }

               if (i > 0)
               {
                  double fwhm = 0.0;
                  if (allBandsIter->isActiveNumberValid())
                  {
                     unsigned int bandNumber = allBandsIter->getActiveNumber();
                     if (bandNumber < calculatedFwhm.size())
                     {
                        fwhm = calculatedFwhm[bandNumber];
                     }
                  }
                  i = fprintf(pStream, "%1.12g", fwhm);
               }
            }
            if (i > 0)
            {
               i = fprintf(pStream, "}");
            }
         }
      }
   }

   //band names
   if (i > 0)
   {      
      vector<string> bandNames = RasterUtilities::getBandNames(pDescriptor);
      if (!bandNames.empty())
      {
         if (i > 0)
         {
            i = fprintf (pStream, "\nband names = {\n");
         }

         for (allBandsIter = allBands.begin(); allBandsIter != allBands.end(); ++allBandsIter)
         {
            if (mAbortFlag)
            {
               message = "ENVI export aborted!";
               if (mpProgress != NULL) mpProgress->updateProgress(message, 0, ABORT);
               pStep->finalize(Message::Abort);

               fclose(pStream);
               remove(filename.c_str());
               return false;
            }
            if ((i > 0) && (allBandsIter != allBands.begin()))
            {
               i = fprintf(pStream, ",\n");      
            }
            if (i > 0)
            {
               string bandName = "Unknown";
               if (allBandsIter->isActiveNumberValid())
               {
                  unsigned int bandNumber = allBandsIter->getActiveNumber();
                  if (bandNumber < bandNames.size())
                  {
                     bandName = bandNames[bandNumber];
                  }                  
               }
               i = fprintf(pStream, "%s", bandName.c_str());
            }
         }
         if (i > 0)
         {
            i = fprintf(pStream, "}");
         }
      }
   }

   // Add a new line so that ENVI will successfully read the file
   if (i > 0)
   {
      i = fprintf(pStream, "\n");
   }

   fclose(pStream);

   if (i <= 0)
   {
      message = "Error writing to file:\n" + filename;
      if (mpProgress != NULL) mpProgress->updateProgress(message, 0, ERRORS);
      pStep->finalize(Message::Failure, message);
      return false;
   }
   else
   {
      message = "Completed ENVI Export";
      if (mpProgress != NULL) mpProgress->updateProgress(message, 100, NORMAL);
      pStep->finalize(Message::Success);
   }

   return true;
}

bool EnviExporter::abort()
{
   mAbortFlag = true;
   return true;
}

bool EnviExporter::extractInputArgs(PlugInArgList* pArgList)
{
   if (pArgList == NULL)
   {
      return false;
   }

   PlugInArg* pArg = NULL;

   // Progress
   if (pArgList->getArg(ProgressArg(), pArg) && (pArg != NULL))
   {
      mpProgress = pArg->getPlugInArgValue<Progress>();
   }

   // Sensor data
   if (!pArgList->getArg(ExportItemArg(), pArg) || (pArg == NULL))
   {
      string message = "Could not read the raster element input value!";
      if (mpProgress != NULL) mpProgress->updateProgress(message, 0, ERRORS);
      mpStep->finalize(Message::Failure, message);
      return false;
   }

   mpRaster = pArg->getPlugInArgValue<RasterElement>();
   if (mpRaster == NULL)
   {
      string message = "The raster element input value is invalid!";
      if (mpProgress != NULL) mpProgress->updateProgress(message, 0, ERRORS);  
      mpStep->finalize(Message::Failure, message);
      return false;
   }

    // File descriptor
   if (!pArgList->getArg(ExportDescriptorArg(), pArg) || (pArg == NULL))
   {
      string message = "Could not read the file descriptor input value!";
      if (mpProgress != NULL) mpProgress->updateProgress(message, 0, ERRORS);
      mpStep->finalize(Message::Failure, message);
      return false;
   }

   mpFileDescriptor = pArg->getPlugInArgValue<RasterFileDescriptor>();
   if (mpFileDescriptor == NULL)
   {
      string message = "The file descriptor input value is invalid!";
      if (mpProgress != NULL) mpProgress->updateProgress(message, 0, ERRORS);
      mpStep->finalize(Message::Failure, message);
      return false;
   }

   return true;
}
