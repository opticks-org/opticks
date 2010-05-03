/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QString>
#include <QtCore/QStringList>

#include "AppVersion.h"
#include "Classification.h"
#include "DimensionDescriptor.h"
#include "DynamicObject.h"
#include "Endian.h"
#include "EnviImporter.h"
#include "FileFinder.h"
#include "FileResource.h"
#include "GeoPoint.h"
#include "ImportDescriptor.h"
#include "ModelServices.h"
#include "ObjectResource.h"
#include "PlugInRegistration.h"
#include "RasterDataDescriptor.h"
#include "RasterFileDescriptor.h"
#include "RasterUtilities.h"
#include "SpecialMetadata.h"
#include "StringUtilities.h"
#include "Units.h"

#include <sstream>

using namespace std;

REGISTER_PLUGIN_BASIC(OpticksENVI, EnviImporter);

static bool parseDefaultBands(EnviField* pField, vector<unsigned int>* pBandNumbers);

template <class T>
void vectorFromField(EnviField* pField, vector<T>& vec, const char* pFormat)
{
   char* pBuffer = new char[pField->mValue.size() + 1];
   strcpy(pBuffer, pField->mValue.c_str());

   char* pPtr = strtok(pBuffer, ",");
   while (pPtr != NULL)
   {
      T value;
      int count = sscanf(pPtr, pFormat, &value);
      if (count == 1)
      {
         vec.push_back (value);
      }

      pPtr = strtok (NULL, ",");
   }
   delete [] pBuffer;
}

EnviImporter::EnviImporter() :
   mWavelengthUnits(WU_UNKNOWN)
{
   setName("ENVI Importer");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setExtensions("ENVI Header Files (*.hdr);;ENVI Data Files (*.bsq *.bil *.bip *.dat *.cub *.img)");
   setDescriptorId("{811F49A2-3930-4a43-AC69-5A08DAEC93B8}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);

   mFields.mTag = "envi";
}

EnviImporter::~EnviImporter()
{
}

vector<ImportDescriptor*> EnviImporter::getImportDescriptors(const string& filename)
{
   string headerFile = filename;
   string dataFile;
   bool bSuccess = parseHeader(headerFile);
   if (bSuccess == false)
   {
      dataFile = filename;           // was passed data file name instead of header file name
      headerFile = findHeaderFile(headerFile);
      if (headerFile.empty() == false)
      {
         bSuccess = parseHeader(headerFile);
      }
   }

   EnviField* pField = NULL;
   vector<ImportDescriptor*> descriptors;
   if (bSuccess == true)
   {
      if (dataFile.empty() == true)  // was passed header file name and now need to find the data file name
      {
         dataFile = findDataFile(headerFile);
      }

      if (dataFile.empty() == false)
      {
         ImportDescriptor* pImportDescriptor = mpModel->createImportDescriptor(dataFile, "RasterElement", NULL);
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
                  pFileDescriptor->setFilename(dataFile);

                  // Coordinate offset
                  int columnOffset = 0;
                  int rowOffset = 0;

                  pField = mFields.find("x start");
                  if (pField != NULL)
                  {
                     // ENVI numbers are 1 based vs Opticks being 0 based
                     columnOffset = atoi(pField->mValue.c_str()) - 1;
                  }

                  pField = mFields.find("y start");
                  if (pField != NULL)
                  {
                     rowOffset = atoi(pField->mValue.c_str()) - 1; // ENVI numbers are 1 based vs Opticks being 0 based
                  }

                  // Rows
                  vector<DimensionDescriptor> rows;
                  pField = mFields.find("lines");
                  if (pField != NULL)
                  {
                     int numRows = atoi(pField->mValue.c_str());
                     for (int i = 0; i < numRows; ++i)
                     {
                        DimensionDescriptor rowDim;
                        rowDim.setOriginalNumber(static_cast<unsigned int>(rowOffset + i));
                        rowDim.setOnDiskNumber(static_cast<unsigned int>(i));
                        rows.push_back(rowDim);
                     }

                     pDescriptor->setRows(rows);
                     pFileDescriptor->setRows(rows);
                  }

                  string samplesStr = "samples";
                  string bandsStr = "bands";

                  // Special case: if the file type is an ENVI Spectral Library, then swap samples with bands
                  // If no file type field exists, assume this is a normal ENVI header (not a Spectral Library)
                  EnviField* pFileTypeField = mFields.find("file type");
                  if (pFileTypeField != NULL && (pFileTypeField->mValue ==
                     "ENVI Spectral Library" || pFileTypeField->mValue == "Spectral Library"))
                  {
                     samplesStr = "bands";
                     bandsStr = "samples";

                     // Since bands and samples are swapped, force the interleave to BIP
                     pField = mFields.find("interleave");
                     if (pField != NULL)
                     {
                        pField->mValue = "bip";
                     }
                  }

                  // Columns
                  vector<DimensionDescriptor> columns;
                  pField = mFields.find(samplesStr);
                  if (pField != NULL)
                  {
                     int numColumns = atoi(pField->mValue.c_str());
                     for (int i = 0; i < numColumns; ++i)
                     {
                        DimensionDescriptor columnDim;
                        columnDim.setOriginalNumber(static_cast<unsigned int>(columnOffset + i));
                        columnDim.setOnDiskNumber(static_cast<unsigned int>(i));
                        columns.push_back(columnDim);
                     }

                     pDescriptor->setColumns(columns);
                     pFileDescriptor->setColumns(columns);
                  }

                  // Bands
                  vector<DimensionDescriptor> bands;
                  pField = mFields.find(bandsStr);
                  if (pField != NULL)
                  {
                     int numBands = atoi(pField->mValue.c_str());
                     bands = RasterUtilities::generateDimensionVector(numBands, true, false, true);
                     pDescriptor->setBands(bands);
                     pFileDescriptor->setBands(bands);
                  }

                  // Description
                  list<GcpPoint> gcps;

                  pField = mFields.find("description");
                  if (pField != NULL)
                  {
                     // Metadata
                     if (pField->mChildren.empty() == false)
                     {
                        FactoryResource<DynamicObject> pMetadata;
                        for (unsigned int i = 0; i < pField->mChildren.size(); ++i)
                        {
                           EnviField* pChild = pField->mChildren[i];
                           if (pChild != NULL)
                           {
                              if (pChild->mTag == "classification")
                              {
                                 // Classification
                                 FactoryResource<Classification> pClassification;
                                 if (pClassification.get() != NULL)
                                 {
                                    string classLevel;
                                    classLevel.append(1, *(pChild->mValue.data()));
                                    pClassification->setLevel(classLevel);

                                    pDescriptor->setClassification(pClassification.get());
                                 }
                              }
                              else if ((pChild->mTag == "ll") || (pChild->mTag == "lr") || (pChild->mTag == "ul") ||
                                 (pChild->mTag == "ur") || (pChild->mTag == "center"))
                              {
                                 GcpPoint gcp;
                                 bool dmsFormat = false;
                                 char ns;
                                 char ew;

                                 sscanf(pChild->mValue.c_str(), "%lg%c %lg%c", &gcp.mCoordinate.mY, &ew,
                                    &gcp.mCoordinate.mX, &ns);
                                 if (fabs(gcp.mCoordinate.mY) > 180.0 || fabs(gcp.mCoordinate.mX) > 90.0)
                                 {
                                    dmsFormat = true;
                                 }

                                 double deg;
                                 double min;
                                 double sec;
                                 if (dmsFormat == true)
                                 {
                                    deg = static_cast<int>(gcp.mCoordinate.mY / 10000.0);
                                    min = static_cast<int>((gcp.mCoordinate.mY - 10000.0 * deg) / 100.0);
                                    sec = gcp.mCoordinate.mY - 10000.0 * deg - 100.0 * min;
                                    gcp.mCoordinate.mY = deg + (min / 60.0) + (sec / 3600.0);
                                 }

                                 if (ew == 'W' || ew == 'w')
                                 {
                                    gcp.mCoordinate.mY = -gcp.mCoordinate.mY;
                                 }

                                 if (dmsFormat)
                                 {
                                    deg = static_cast<int>(gcp.mCoordinate.mX / 10000.0);
                                    min = static_cast<int>((gcp.mCoordinate.mX - 10000.0 * deg) / 100.0);
                                    sec = gcp.mCoordinate.mX - 10000.0 * deg - 100.0 * min;
                                    gcp.mCoordinate.mX = deg + (min / 60.0) + (sec / 3600.0);
                                 }

                                 if (ns == 'S' || ns == 's')
                                 {
                                    gcp.mCoordinate.mX = -gcp.mCoordinate.mX;
                                 }

                                 if (pChild->mTag == "ll")
                                 {
                                    gcp.mPixel.mX = 0.0;
                                    gcp.mPixel.mY = 0.0;
                                 }
                                 else if (pChild->mTag == "lr")
                                 {
                                    gcp.mPixel.mX = columns.size() - 1.0;
                                    gcp.mPixel.mY = 0.0;
                                 }
                                 else if (pChild->mTag == "ul")
                                 {
                                    gcp.mPixel.mX = 0.0;
                                    gcp.mPixel.mY = rows.size() - 1.0;
                                 }
                                 else if (pChild->mTag == "ur")
                                 {
                                    gcp.mPixel.mX = columns.size() - 1.0;
                                    gcp.mPixel.mY = rows.size() - 1.0;
                                 }
                                 else if (pChild->mTag == "center")
                                 {
                                    gcp.mPixel.mX = (columns.size() / 2.0) - 0.5;
                                    gcp.mPixel.mY = (rows.size() / 2.0) - 0.5;
                                 }

                                 gcps.push_back(gcp);
                              }
                              else if (pChild->mTag.empty() == false)
                              {
                                 pMetadata->setAttribute(pChild->mTag, pChild->mValue);
                              }
                           }
                        }

                        if (pMetadata->getNumAttributes() > 0)
                        {
                           pDescriptor->setMetadata(pMetadata.get());
                        }
                     }
                  }

                  if (gcps.empty())  // not in description, check for geo points keyword
                  {
                     pField = mFields.find("geo points");
                     if (pField != NULL)
                     {
                        vector<double> geoValues;
                        const int expectedNumValues = 16;  // 4 values for each of the 4 corners
                        geoValues.reserve(expectedNumValues);
                        for (unsigned int i = 0; i < pField->mChildren.size(); i++)
                        {
                           vectorFromField(pField->mChildren.at(i), geoValues, "%lf");
                        }

                        if (geoValues.size() == expectedNumValues)
                        {
                           vector<double>::iterator iter = geoValues.begin();
                           GcpPoint gcp;
                           while (iter != geoValues.end())
                           {
                              gcp.mPixel.mX = *iter++ - 1.5;  // adjust ref point for ENVI's use of
                              gcp.mPixel.mY = *iter++ - 1.5;  // upper left corner and one-based first pixel
                              gcp.mCoordinate.mX = *iter++;   // GcpPoint has lat as mX and Lon as mY 
                              gcp.mCoordinate.mY = *iter++;   // geo point field has lat then lon value
                              gcps.push_back(gcp);
                           }
                        }
                     }
                  }

                  // GCPs
                  if (gcps.empty() == false)
                  {
                     pFileDescriptor->setGcps(gcps);
                  }

                  // Header bytes
                  pField = mFields.find("header offset");
                  if (pField != NULL)
                  {
                     int headerBytes = atoi(pField->mValue.c_str());
                     pFileDescriptor->setHeaderBytes(static_cast<unsigned int>(headerBytes));
                  }

                  // Data type
                  pField = mFields.find("data type");
                  if (pField != NULL)
                  {
                     switch (atoi(pField->mValue.c_str()))
                     {
                        case 1:     // char
                           pDescriptor->setDataType(INT1UBYTE);
                           pFileDescriptor->setBitsPerElement(8);
                           break;

                        case 2:     // short
                           pDescriptor->setDataType(INT2SBYTES);
                           pFileDescriptor->setBitsPerElement(16);
                           break;

                        case 3:     // int
                           pDescriptor->setDataType(INT4SBYTES);
                           pFileDescriptor->setBitsPerElement(32);
                           break;

                        case 4:     // float
                           pDescriptor->setDataType(FLT4BYTES);
                           pFileDescriptor->setBitsPerElement(32);
                           break;

                        case 5:     // double
                           pDescriptor->setDataType(FLT8BYTES);
                           pFileDescriptor->setBitsPerElement(64);
                           break;

                        case 6:     // float complex
                           pDescriptor->setDataType(FLT8COMPLEX);
                           pFileDescriptor->setBitsPerElement(64);
                           break;

                        case 9:     // double complex
                           // not supported
                           break;

                        case 12:    // unsigned short
                           pDescriptor->setDataType(INT2UBYTES);
                           pFileDescriptor->setBitsPerElement(16);
                           break;

                        case 13:    // unsigned int
                           pDescriptor->setDataType(INT4UBYTES);
                           pFileDescriptor->setBitsPerElement(32);
                           break;

                        case 14:    // 64-bit int
                        case 15:    // unsigned 64-bit int
                           // not supported
                           break;

                        case 99:    // integer complex (recognized only by this application)
                           pDescriptor->setDataType(INT4SCOMPLEX);
                           pFileDescriptor->setBitsPerElement(32);
                           break;

                        default:
                           break;
                     }

                     // Bad values
                     EncodingType dataType = pDescriptor->getDataType();
                     if ((dataType != FLT4BYTES) && (dataType != FLT8COMPLEX) && (dataType != FLT8BYTES))
                     {
                        vector<int> badValues;
                        badValues.push_back(0);

                        pDescriptor->setBadValues(badValues);
                     }

                     pDescriptor->setValidDataTypes(vector<EncodingType>(1, dataType));
                  }

                  // Interleave format
                  pField = mFields.find("interleave");
                  if (pField != NULL)
                  {
                     string interleave = StringUtilities::toLower(pField->mValue);
                     if (interleave == "bip")
                     {
                        pDescriptor->setInterleaveFormat(BIP);
                        pFileDescriptor->setInterleaveFormat(BIP);
                     }
                     else if (interleave == "bil")
                     {
                        pDescriptor->setInterleaveFormat(BIL);
                        pFileDescriptor->setInterleaveFormat(BIL);
                     }
                     else if (interleave == "bsq")
                     {
                        pDescriptor->setInterleaveFormat(BSQ);
                        pFileDescriptor->setInterleaveFormat(BSQ);
                     }
                  }

                  // Endian
                  pField = mFields.find("byte order");
                  if (pField != NULL)
                  {
                     int byteOrder = atoi(pField->mValue.c_str());
                     if (byteOrder == 0)
                     {
                        pFileDescriptor->setEndian(LITTLE_ENDIAN_ORDER);
                     }
                     else if (byteOrder == 1)
                     {
                        pFileDescriptor->setEndian(BIG_ENDIAN_ORDER);
                     }
                  }

                  // check for scaling factor
                  pField = mFields.find("reflectance scale factor");
                  if (pField != NULL)
                  {
                     double scalingFactor = 0.0;
                     stringstream scaleStream(pField->mValue);
                     scaleStream >> scalingFactor;
                     if (!scaleStream.fail() && scalingFactor != 0.0)
                     {
                        Units* pUnits = pDescriptor->getUnits();
                        if (pUnits != NULL)
                        {
                           pUnits->setScaleFromStandard(1.0 / scalingFactor);
                           pUnits->setUnitName("Reflectance");
                           pUnits->setUnitType(REFLECTANCE);
                        }
                     }
                  }

                  // Pixel size
                  pField = mFields.find("pixel size");
                  if (pField != NULL)
                  {
                     if (pField->mChildren.size() == 2)
                     {
                        pField = pField->mChildren[0];
                        if (pField != NULL)
                        {
                           double pixelSize = 1.0;
                           if (sscanf(pField->mValue.c_str(), "%g", &pixelSize) == 1)
                           {
                              pDescriptor->setXPixelSize(pixelSize);
                              pFileDescriptor->setXPixelSize(pixelSize);
                           }
                        }

                        pField = pField->mChildren[1];
                        if (pField != NULL)
                        {
                           double pixelSize = 1.0;
                           if (sscanf(pField->mValue.c_str(), "%g", &pixelSize) == 1)
                           {
                              pDescriptor->setYPixelSize(pixelSize);
                              pFileDescriptor->setYPixelSize(pixelSize);
                           }
                        }
                     }
                  }

                  // Default bands
                  pField = mFields.find("default bands");
                  if (pField != NULL)
                  {
                     vector<unsigned int> displayBands;
                     parseDefaultBands(pField, &displayBands);

                     if (displayBands.size() == 1)
                     {
                        DimensionDescriptor grayBand = pFileDescriptor->getOriginalBand(displayBands[0]);

                        pDescriptor->setDisplayBand(GRAY, grayBand);
                        pDescriptor->setDisplayMode(GRAYSCALE_MODE);
                     }
                     else if (displayBands.size() == 3)
                     {
                        DimensionDescriptor redBand = pFileDescriptor->getOriginalBand(displayBands[0]);
                        DimensionDescriptor greenBand = pFileDescriptor->getOriginalBand(displayBands[1]);
                        DimensionDescriptor blueBand = pFileDescriptor->getOriginalBand(displayBands[2]);

                        pDescriptor->setDisplayBand(RED, redBand);
                        pDescriptor->setDisplayBand(GREEN, greenBand);
                        pDescriptor->setDisplayBand(BLUE, blueBand);
                        pDescriptor->setDisplayMode(RGB_MODE);
                     }
                  }

                  // Bad bands
                  pField = mFields.find("bbl");
                  if (pField != NULL)
                  {
                     vector<unsigned int> validBands;
                     parseBbl(pField, validBands);

                     vector<DimensionDescriptor> bandsToLoad;
                     for (vector<unsigned int>::const_iterator iter = validBands.begin();
                        iter != validBands.end();
                        ++iter)
                     {
                        const unsigned int onDiskNumber = *iter;
                        const DimensionDescriptor dim = pFileDescriptor->getOnDiskBand(onDiskNumber);
                        if (dim.isValid())
                        {
                           bandsToLoad.push_back(dim);
                        }
                     }

                     pDescriptor->setBands(bandsToLoad);
                  }

                  DynamicObject* pMetadata = pDescriptor->getMetadata();

                  // Band names
                  pField = mFields.find("band names");
                  if (pField != NULL)
                  {
                     vector<string> bandNames;
                     bandNames.reserve(bands.size());
                     vector<string> strNames;
                     for (vector<EnviField*>::size_type i = 0; i < pField->mChildren.size(); ++i)
                     {
                        strNames = StringUtilities::split(pField->mChildren[i]->mValue, ',');
                        copy(strNames.begin(), strNames.end(), back_inserter(bandNames));
                     }
                     vector<string>::iterator it;
                     for (it = bandNames.begin(); it != bandNames.end(); ++it)
                     {
                        *it = StringUtilities::stripWhitespace(*it);
                     }

                     if (pMetadata != NULL)
                     {
                        string pNamesPath[] = { SPECIAL_METADATA_NAME, BAND_METADATA_NAME,
                           NAMES_METADATA_NAME, END_METADATA_NAME };
                        pMetadata->setAttributeByPath(pNamesPath, bandNames);
                     }
                  }

                  // wavelength units
                  pField = mFields.find("wavelength units");
                  if (pField != NULL)
                  {
                     mWavelengthUnits = strToType(pField->mValue);
                  }

                  // Wavelengths
                  vector<double> centerWavelengths;
                  pField = mFields.find("wavelength");
                  if (pField != NULL)
                  {
                     if ((parseWavelengths(pField, &centerWavelengths) == true) && (pMetadata != NULL))
                     {
                        string pCenterPath[] = { SPECIAL_METADATA_NAME, BAND_METADATA_NAME,
                           CENTER_WAVELENGTHS_METADATA_NAME, END_METADATA_NAME };
                        pMetadata->setAttributeByPath(pCenterPath, centerWavelengths);
                     }
                  }

                  // FWHM
                  pField = mFields.find("fwhm");
                  if (pField != NULL)
                  {
                     vector<double> startWavelengths;
                     vector<double> endWavelengths;

                     if ((parseFwhm(pField, &startWavelengths, &centerWavelengths, &endWavelengths) == true) &&
                        (pMetadata != NULL))
                     {
                        string pStartPath[] = { SPECIAL_METADATA_NAME, BAND_METADATA_NAME,
                           START_WAVELENGTHS_METADATA_NAME, END_METADATA_NAME };
                        pMetadata->setAttributeByPath(pStartPath, startWavelengths);
                        string pEndPath[] = { SPECIAL_METADATA_NAME, BAND_METADATA_NAME,
                           END_WAVELENGTHS_METADATA_NAME, END_METADATA_NAME };
                        pMetadata->setAttributeByPath(pEndPath, endWavelengths);
                     }
                  }

                  // File descriptor
                  pDescriptor->setFileDescriptor(pFileDescriptor.get());
               }
            }

            descriptors.push_back(pImportDescriptor);
         }
      }
   }

   return descriptors;
}

unsigned char EnviImporter::getFileAffinity(const string& filename)
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

bool EnviImporter::parseWavelengths(EnviField* pField, vector<double>* pWavelengthCenters)
{
   unsigned int i;
   double maxWavelength(0.0);

   for (i = 0; i < pField->mChildren.size(); i++)
   {
      vectorFromField(pField->mChildren.at(i), *pWavelengthCenters, "%lf");
   }
   for (i = 0; i < pWavelengthCenters->size(); ++i)
   {
      if (pWavelengthCenters->at(i) > maxWavelength)
      {
         maxWavelength = pWavelengthCenters->at(i);
      }
   }

   switch (mWavelengthUnits)
   {
   case WU_MICROMETERS:  // already in micrometers, nothing further to do
      break;

   case WU_NANOMETERS:
      for (i = 0; i < pWavelengthCenters->size(); ++i)
      {
         pWavelengthCenters->at(i) *= 0.001;  // convert to micrometers
      }
      break;

   case WU_WAVENUMBER:
      for (i = 0; i < pWavelengthCenters->size(); ++i)
      {
         pWavelengthCenters->at(i) = 10000.0/pWavelengthCenters->at(i);  // convert to micrometers
      }
      break;

   case WU_GHZ:     // fall through
   case WU_MHZ:     // fall through
   case WU_INDEX:
      pWavelengthCenters->clear();  // not supported
      return false;

   case WU_UNKNOWN: // fall through
   default:
      if (maxWavelength > 50.0)  // could be in nanometers, so convert
      {
         for (i = 0; i < pWavelengthCenters->size(); ++i)
         {
            pWavelengthCenters->at(i) *= 0.001;  // convert to micrometers
         }

         mWavelengthUnits = WU_NANOMETERS;
      }
      break;
   }

   return true;
}

static bool parseDefaultBands(EnviField* pField, vector<unsigned int>* pBandNumbers)
{
   if ((pField == NULL) || (pBandNumbers == NULL))
   {
      return false;
   }

   for (vector<EnviField*>::size_type i = 0; i < pField->mChildren.size(); ++i)
   {
      vectorFromField(pField->mChildren[i], *pBandNumbers, "%u");
   }

   for (vector<unsigned int>::size_type i = 0; i < pBandNumbers->size(); ++i)
   {
      pBandNumbers->at(i) = pBandNumbers->at(i) - 1;
   }

   return true;
}

bool EnviImporter::parseFwhm(EnviField* pField, vector<double>* pWavelengthStarts,
                             const vector<double>* pWavelengthCenters, vector<double>* pWavelengthEnds)
{
   vector<double> fwhmValues;
   for (vector<EnviField*>::size_type i = 0; i < pField->mChildren.size(); ++i)
   {
      vectorFromField(pField->mChildren[i], fwhmValues, "%lf");
   }

   if (fwhmValues.size() != pWavelengthCenters->size())
   {
      return false;
   }

   switch (mWavelengthUnits)
   {
   case WU_MICROMETERS:  // Already in micrometers, nothing further to do
      break;

   case WU_NANOMETERS:
      for (vector<double>::size_type i = 0; i < fwhmValues.size(); ++i)
      {
         fwhmValues[i] *= 0.001;  // Convert to micrometers
      }
      break;

   case WU_WAVENUMBER:
      for (vector<double>::size_type i = 0; i < fwhmValues.size(); ++i)
      {
         // Convert the center wavelength to wave number
         double centerValue = 10000.0 / pWavelengthCenters->at(i);

         // Find the start and stop wave numbers
         double startValue = centerValue - (fwhmValues[i] / 2);
         double endValue = centerValue + (fwhmValues[i] / 2);

         // Convert the start and stop wave numbers to micrometers
         double startConverted = 10000.0 / startValue;
         double endConverted = 10000.0 / endValue;

         // Update the FWHM value
         fwhmValues[i] = endConverted - startConverted;
      }
      break;

   case WU_GHZ:      // Fall through
   case WU_MHZ:      // Fall through
   case WU_INDEX:
      return false;  // Not supported

   case WU_UNKNOWN:  // Fall through
   default:
      break;
   }

   for (vector<double>::size_type i = 0; i < fwhmValues.size(); ++i)
   {
      pWavelengthStarts->push_back(pWavelengthCenters->at(i) - fwhmValues[i] / 2.0);
      pWavelengthEnds->push_back(pWavelengthCenters->at(i) + fwhmValues[i] / 2.0);
   }

   return true;
}

bool EnviImporter::parseBbl(EnviField* pField, vector<unsigned int>& goodBands)
{
   VERIFY(pField != NULL);
   int band = 0;

   for (vector<EnviField*>::size_type i = 0; i < pField->mChildren.size(); ++i)
   {
      int fields = 1;
      char* pLine = strdup(pField->mChildren.at(i)->mValue.c_str());
      char* pCurr = strtok(pLine, ",");
      float f1 = 0.0;
      while (fields == 1 && pCurr != NULL)
      {
         fields = sscanf(pCurr, "%f", &f1);
         pCurr = strtok(NULL, ",");
         if (fields == 1)
         {
            if (f1 == 1.0f)
            {
               goodBands.push_back(band);
            }
            ++band;
         }
      }

      free(pLine);
   }
   return true;
}

string EnviImporter::findHeaderFile(const string& dataPath)
{
   string temp = dataPath + ".hdr";

   FILE* pStream = fopen(temp.c_str(), "rb");
   if (pStream != NULL)
   {
      fclose(pStream);
      return temp;
   }
   else
   {
      unsigned int i = dataPath.rfind('.');
      if ((i >= 0) && (i < dataPath.size()))
      {
         temp = dataPath.substr (0, i) + ".hdr";
         pStream = fopen(temp.c_str(), "rb");
         if (pStream != NULL)
         {
            fclose(pStream);
            return temp;
         }
      }
   }
   return "";
}

string EnviImporter::findDataFile(const string& headerPath)
{
   FILE* pStream = NULL;
   string temp;

   // Read the header if necessary
   EnviField* pField = mFields.find("description");
   if (pField == NULL)
   {
      bool bSuccess = parseHeader(headerPath);
      if (bSuccess == false)
      {
         return "";
      }
   }

   // Check for the filename field
   pField = mFields.find("description");
   if (pField != NULL)
   {
      pField = pField->find("filename");
      if (pField != NULL)
      {
         temp = pField->mValue;
         if (!temp.empty())
         {
            pStream = fopen(temp.c_str(), "rb");
         }

         if (pStream != NULL)
         {
            fclose(pStream);
            return temp;
         }
      }
   }

   // Check for files with the same name
   temp = headerPath;
   unsigned int i = temp.rfind('.');
   if ((i >= 0) && (i < temp.size()))
   {
      temp = temp.substr(0, i);

      // Check common extensions
      char* pExtensions[] = { ".bip", ".bil", ".bsq", ".dat", "", ".sio", ".cub", ".img" };
      for (int j = 0; j < sizeof(pExtensions) / sizeof(pExtensions[0]); j++)
      {
         string attempt = temp + pExtensions[j];
         pStream = fopen(attempt.c_str(), "rb");
         if (pStream != NULL)
         {
            fclose(pStream);
            return attempt;
         }
      }
   }

   return "";
}

bool EnviImporter::parseHeader(const string& filename)
{
   if (filename.empty() == true)
   {
      return false;
   }

   bool bSuccess = mFields.populateFromHeader(filename);
   return bSuccess;
}

bool EnviImporter::validate(const DataDescriptor* pDescriptor, string& errorMessage) const
{
   if (RasterElementImporterShell::validate(pDescriptor, errorMessage) == false)
   {
      return false;
   }

   const RasterDataDescriptor* pRasterDesc = dynamic_cast<const RasterDataDescriptor*>(pDescriptor);
   if (pRasterDesc == NULL)
   {
      errorMessage = "The data descriptor is invalid!";
      return false;
   }

   const RasterFileDescriptor* pFileDescriptor = 
      dynamic_cast<const RasterFileDescriptor*>(pDescriptor->getFileDescriptor());
   if (pFileDescriptor == NULL)
   {
      errorMessage = "The file descriptor is invalid!";
      return false;
   }
   const Filename& filename = pFileDescriptor->getFilename();
   if (filename.getFullPathAndName().empty() == true)
   {
      errorMessage = "The data filename is invalid!";
      return false;
   }

   unsigned int numRows = pFileDescriptor->getRowCount();
   unsigned int numColumns = pFileDescriptor->getColumnCount();
   unsigned int numBands = pFileDescriptor->getBandCount();
   unsigned int bitsPerElement = pFileDescriptor->getBitsPerElement();

   if (numRows == 0)
   {
      errorMessage = "The number of rows is invalid!";
      return false;
   }
   if (numColumns == 0)
   {
      errorMessage = "The number of columns is invalid!";
      return false;
   }
   if (numBands == 0)
   {
      errorMessage = "The number of bands is invalid!";
      return false;
   }
   if (bitsPerElement == 0)
   {
      errorMessage = "The number of bits per element is invalid!";
      return false;
   }

   // check required size against file size/s
   int64_t requiredSize = RasterUtilities::calculateFileSize(pFileDescriptor);
   if (requiredSize < 0)
   {
      errorMessage = "Unable to determine data file size due to problem in RasterFileDescriptor.";
      return false;
   }

   LargeFileResource file;
   if (file.open(filename.getFullPathAndName(), O_RDONLY | O_BINARY, S_IREAD) == false)
   {
      errorMessage = "The data file: " + string(filename) + " does not exist!";
      return false;
   }
   if (file.fileLength() < requiredSize)
   {
      errorMessage = "The size of the data file does not match the parameters in the header file!";
      return false;
   }

   // check metadata for data files
   EnviField* pFileTypeField = mFields.find("file type");
   if (pFileTypeField == NULL || (pFileTypeField->mValue !=
      "ENVI Spectral Library" && pFileTypeField->mValue != "Spectral Library"))
   {
      const DynamicObject* pMetadata = pRasterDesc->getMetadata();
      if (pMetadata != NULL)
      {
         string pNamesPath[] = { SPECIAL_METADATA_NAME, BAND_METADATA_NAME, 
            NAMES_METADATA_NAME, END_METADATA_NAME };
         const vector<string>* pBandNames(NULL);
         pBandNames = dv_cast<vector<string> >(&pMetadata->getAttributeByPath(pNamesPath));

         if (pBandNames != NULL && pBandNames->size() != pRasterDesc->getBandCount())
         {
            if (errorMessage.empty() == false)
            {
               errorMessage += "\n";
            }

            errorMessage += "Possible problem in ENVI header file: The number of band "
               "names did not match the number of bands.";
         }
      }
   }

   return true;
}

EnviImporter::WavelengthUnitsType EnviImporter::strToType(string strType)
{
   string target = StringUtilities::toLower(StringUtilities::stripWhitespace(strType));
   WavelengthUnitsType eType;
   if (target == "micrometers")
   {
      eType = WU_MICROMETERS;
   }
   else if (target == "nanometers")
   {
      eType = WU_NANOMETERS;
   }
   else if (target == "wavenumber")
   {
      eType = WU_WAVENUMBER;
   }
   else if (target == "ghz")
   {
      eType = WU_GHZ;
   }
   else if (target == "mhz")
   {
      eType = WU_MHZ;
   }
   else if (target == "unknown")
   {
      eType = WU_UNKNOWN;
   }

   return eType;
}
