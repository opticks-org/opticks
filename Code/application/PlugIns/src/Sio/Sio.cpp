/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ApplicationServices.h"
#include "AppVerify.h"
#include "DataVariant.h"
#include "DateTime.h"
#include "Endian.h"
#include "Sio.h"

#include <sstream>
using namespace std;

Attribute_Metadata gpzAttribute_Descriptor[INVALID_LAST_ENUM_ITEM_FLAG] =
{  // Type, Enumeration ID , Serialize?
   {CSTRING_PARM, "ORIGINAL_SENSOR", true},
   {LONG_PARM, "IMAGE_WIDTH", false},
   {LONG_PARM, "IMAGE_HEIGHT", false},
   {LONG_PARM, "IMAGE_DATA_FORMAT", false},
   {LONG_PARM, "IMAGE_BYTES_PER_PIXEL", true},
   {LONG_PARM, "IMAGE_BANDS", false},
   {CSTRING_PARM, "IMAGE_INTERLEAVING", true},
   {CSTRING_PARM, "IMAGE_COORDINATE_SYSTEM", true},
   {LONG_PARM, "IMAGE_HEADER_BYTES", true},
   {LONG_PARM, "IMAGE_BYTE_ORDER", true},
   {DOUBLE_PARM, "UPPER_LEFT_CORNER_LAT", true},
   {DOUBLE_PARM, "UPPER_LEFT_CORNER_LONG", true},
   {DOUBLE_PARM, "UPPER_RIGHT_CORNER_LAT", true},
   {DOUBLE_PARM, "UPPER_RIGHT_CORNER_LONG", true},
   {DOUBLE_PARM, "LOWER_RIGHT_CORNER_LAT", true},
   {DOUBLE_PARM, "LOWER_RIGHT_CORNER_LONG", true},
   {DOUBLE_PARM, "LOWER_LEFT_CORNER_LAT", true},
   {DOUBLE_PARM, "LOWER_LEFT_CORNER_LONG", true},
   {DOUBLE_PARM, "CENTER_LAT", true},
   {DOUBLE_PARM, "CENTER_LONG", true},
   {FLOAT_VECTOR_PARM, "CENTER_FREQUENCY_VECTOR", true},
   {FLOAT_VECTOR_PARM, "LOW_POWER_FREQUENCY_VECTOR", true},
   {FLOAT_VECTOR_PARM, "HIGH_POWER_FREQUENCY_VECTOR", true},
   {FLOAT_PARM, "SUN_AZIMUTH", true},
   {FLOAT_PARM, "SUN_ELEVATION", true},
   {LONG_PARM, "BAD_BANDS_COUNT", false},
   {LONG_VECTOR_PARM, "BAD_BANDS_VECTOR", false},
   {LONG_PARM, "DISPLAY_WIDTH", false},
   {LONG_PARM, "DISPLAY_HEIGHT", false},
   {LONG_PARM, "DISPLAY_DEPTH", false},
   {LONG_VECTOR_PARM, "DISPLAY_BANDS_VECTOR", false},
   {LONG_VECTOR_PARM, "DISPLAY_COLUMNS_VECTOR", false},
   {LONG_VECTOR_PARM, "DISPLAY_ROWS_VECTOR", false},
   {DOUBLE_VECTOR_PARM, "LATITUDE_VECTOR", true},
   {DOUBLE_VECTOR_PARM, "LONGITUDE_VECTOR", true},
   {CSTRING_PARM, "STATISTICS_BOOLS", true}
};

static double convertData(UNION_VALUE& value, int datatype)
{
   double convertedValue = 0;

   switch (datatype)
   {
      case INT1SBYTE:
         convertedValue = value.charInput;
         break;
      case INT1UBYTE:
         convertedValue = value.ucharInput;
         break;
      case INT2SBYTES:
         convertedValue = value.int2sInput;
         break;
      case INT2UBYTES:
         convertedValue = value.int2uInput;
         break;
      case INT4SBYTES:
         convertedValue = value.int4sInput;
         break;
      case INT4UBYTES:
         convertedValue = value.int4uInput;
         break;
      case FLT4BYTES:
         convertedValue = value.flt4Input;
         break;
      case FLT8BYTES:
         convertedValue = value.doubleInput;
         break;
      default:
         break;
   }

   return convertedValue;
}

static void formatDate(const string& value, DateTime* pDateTime)
{
   VERIFYNRV((value.empty() == false) && (pDateTime != NULL));

   unsigned short month = 0;
   unsigned short day = 0;
   unsigned short year = 0;
   unsigned short hour = 0;
   unsigned short minute = 0;
   unsigned short second = 0;

   int iValues = sscanf(value.c_str(), "%hu %hu %hu %hu %hu %hu", &month, &day, &year, &hour, &minute, &second);
   if (iValues == 6)
   {
      pDateTime->set(year, month, day, hour, minute, second);
   }
   else if (iValues == 3)
   {
      pDateTime->set(year, month, day);
   }
}

static void formatBandBadValuesVector(vector< vector<int> >& vec, string val)
{
   int len;
   int i;
   int k;
   int numBadValues;
   int badValue;
   vector<int> badValues;
   bool noBadValues = true;

   vec.clear();

   istringstream input;
   input.str(val);
   input >> len;
   VERIFYNRV( len >= 1 );

   for (i = 0; i < len; i++)
   {
      input >> numBadValues;

      badValues.clear();
      for (k = 0; k < numBadValues; k++)
      {
         input >> badValue;

         badValues.push_back(badValue);
         noBadValues = false;
      }

      vec.push_back(badValues);
   }

   if (noBadValues)
   {
      vec.clear();
   }
}

static bool readDynamicObjectTags(DynamicObject* pDynObjRoot, string key, string type, string val)
{
   if (pDynObjRoot == NULL || key.empty() || type.empty() || val.empty() )
   {
      return false;
   }

   DynamicObject* pDynObj = pDynObjRoot;
   string treeType = "DynamicObject";
   string::size_type firstPos = 0;
   string::size_type lastPos = key.find(':');
   while (lastPos != string::npos && treeType == "DynamicObject")
   {
      string keyTok = key.substr(firstPos, lastPos);
      DataVariant& attrValue = pDynObj->getAttribute(keyTok);
      treeType = attrValue.getTypeName();
      pDynObj = attrValue.getPointerToValue<DynamicObject>();
      firstPos = lastPos + 1;
      lastPos = key.find(':', firstPos);
   }

   if (lastPos == string::npos)
   {
      if (treeType != "DynamicObject")
      {
         return false;
      }

      string keyTok = key.substr(firstPos, lastPos);

      DataVariant var;
      DataVariant::Status status = var.fromXmlString(type, val);

      if (status != DataVariant::SUCCESS)
      {
         if (type == "DynamicObject")
         {
            FactoryResource<DynamicObject> res;
            var = *res.get();
         }
         else
         {
            return false;
         }
      }

      if (var.isValid())
      {
         pDynObj->setAttribute(keyTok, var);
      }
      else
      {
         return false;
      }
   }

   return true;
}

SioFile::SioFile() :
   mVersion(8),
   mEndian(Endian::getSystemEndian()),
   mDataType(),
   mBitsPerElement(0),
   mColumns(0),
   mRows(0),
   mBands(0),
   mBadBands(0),
   mOptimizations(-1),
   mUnitType(DIGITAL_NO),
   mUnitName(string()),
   mRangeMin(0.0),
   mRangeMax(0.0),
   mScale(1.0)
{
}

SioFile::~SioFile()
{
   std::vector<double*>::iterator mPercentileIter;
   for (mPercentileIter = mStatPercentile.begin(); mPercentileIter != mStatPercentile.end(); ++mPercentileIter)
   {
      double* pPercentile = *mPercentileIter;
      if (pPercentile != NULL)
      {
         delete [] pPercentile;
      }
   }

   std::vector<double*>::iterator mCenterIter;
   for (mCenterIter = mStatBinCenter.begin(); mCenterIter != mStatBinCenter.end(); ++mCenterIter)
   {
      double* pCenter = *mCenterIter;
      if (pCenter != NULL)
      {
         delete [] pCenter;
      }
   }

   std::vector<unsigned int*>::iterator mHistogramIter;
   for (mHistogramIter = mStatHistogram.begin(); mHistogramIter != mStatHistogram.end(); ++mHistogramIter)
   {
      unsigned int* pHistogram = *mHistogramIter;
      if (pHistogram != NULL)
      {
         delete [] pHistogram;
      }
   }
}

bool SioFile::deserialize(FILE* pFile)
{
   if (pFile == NULL)
   {
      return false;
   }

   // Version
   fread(&mVersion, sizeof(mVersion), 1, pFile);

   mOriginalVersion = mVersion;

//Version 9 sio's are only written by the temporary Sio Exporter provided in Opticks
//The exporter has bugs and will be removed in Opticks, therefore version 9 sio's
//will never be supported by the Opticks Team.
//#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : When Sio Exporter is removed from Opticks, " \
//   "remove the ability to load version 9 sio's (kstreith)")
   // Endian
   if ((mVersion != 5) && (mVersion != 6) && (mVersion != 7) && (mVersion != 8) && (mVersion != 9))
   {
      if (mEndian == LITTLE_ENDIAN_ORDER)
      {
         Endian endian(BIG_ENDIAN_ORDER);
         endian.swapValue(mVersion);

         if ((mVersion == 5) || (mVersion == 6) || (mVersion == 7) || (mVersion == 8) || (mVersion == 9))
         {
            mEndian = BIG_ENDIAN_ORDER;
         }
      }
      else if (mEndian == BIG_ENDIAN_ORDER)
      {
         Endian endian(LITTLE_ENDIAN_ORDER);
         endian.swapValue(mVersion);

         if ((mVersion == 5) || (mVersion == 6) || (mVersion == 7) || (mVersion == 8) || (mVersion == 9))
         {
            mEndian = LITTLE_ENDIAN_ORDER;
         }
      }

      if ((mVersion != 5) && (mVersion != 6) && (mVersion != 7) && (mVersion != 8) && (mVersion != 9))
      {
         return false;
      }
   }

   // Version 9 is identical to version 8, except will be blacklisted later
   if (mVersion == 9)
   {
      mVersion = 8;
   }

   Endian endian(mEndian);

   // Data type
   int dataType = 0;
   if (fread(&dataType, sizeof(dataType), 1, pFile) != 1)
   {
      return false;
   }

   endian.swapValue(dataType);

   switch (dataType)
   {
      case DATA_CHAR:
         mDataType = INT1SBYTE;
         mBitsPerElement = 8;
         break;

      case DATA_UCHAR:
         mDataType = INT1UBYTE;
         mBitsPerElement = 8;
         break;

      case DATA_SHORT:
         mDataType = INT2SBYTES;
         mBitsPerElement = 16;
         break;

      case DATA_USHORT:
         mDataType = INT2UBYTES;
         mBitsPerElement = 16;
         break;

      case DATA_LONG:
         mDataType = INT4SBYTES;
         mBitsPerElement = 32;
         break;

      case DATA_ULONG:
         mDataType = INT4UBYTES;
         mBitsPerElement = 32;
         break;

      case DATA_FLOAT:
         mDataType = FLT4BYTES;
         mBitsPerElement = 32;
         break;

      case DATA_DOUBLE:
         mDataType = FLT8BYTES;
         mBitsPerElement = 64;
         break;

      case DATA_ICOMPLEX:
         mDataType = INT4SCOMPLEX;
         mBitsPerElement = 32;
         break;

      case DATA_FCOMPLEX:
         mDataType = FLT8COMPLEX;
         mBitsPerElement = 64;
         break;

      default:
         break;
   }

   // Num columns
   if (fread(&mColumns, sizeof(mColumns), 1, pFile) != 1)
   {
      return false;
   }

   endian.swapValue(mColumns);

   // Num rows
   if (fread(&mRows, sizeof(mRows), 1, pFile) != 1)
   {
      return false;
   }

   endian.swapValue(mRows);

   // Num bands
   if (fread(&mBands, sizeof(mBands), 1, pFile) != 1)
   {
      return false;
   }

   endian.swapValue(mBands);

   // Num bad bands
   if (fread(&mBadBands, sizeof(mBadBands), 1, pFile) != 1)
   {
      return false;
   }

   endian.swapValue(mBadBands);

   // Statistics optimizations
   if (fread(&mOptimizations, sizeof(mOptimizations), 1, pFile) != 1)
   {
      return false;
   }

   endian.swapValue(mOptimizations);

   // Skip the raw data
   long dataBytes = mRows * mColumns* (mBands - mBadBands) * (mBitsPerElement / 8);
   if (fseek(pFile, dataBytes, SEEK_CUR) != 0)
   {
      return false;
   }

   // Bad bands
   if (mBadBands > 0)
   {
      // Skip list of bad bands
      fseek(pFile, sizeof(int) * mBadBands, SEEK_CUR);

      // Band->file map
      for (int i = 0; i < mBands; ++i)
      {
         int bandNumber = -1;
         fread(&bandNumber, sizeof(bandNumber), 1, pFile);
         endian.swapValue(bandNumber);

         mBandToFileMap.push_back(bandNumber);
      }
   }
   else
   {
      // Skip the band->file mappings
      fseek(pFile, sizeof(int) * mBands, SEEK_CUR);
   }

   // Skip the file->band mappings
   fseek(pFile, sizeof(int) * (mBands - mBadBands), SEEK_CUR);

   // Statistics
   if (mOptimizations)
   {
      if (mVersion == 5)
      {
         // This bypass is because version 1.3 isn't writing out
         // anything other than unsigned char correctly
         if (mDataType != INT1UBYTE)
         {
            long skipCount = mBands * ((2 * mBitsPerElement / 8) + (2 * sizeof(double)));
            fseek(pFile, skipCount, SEEK_CUR);
         }
         else
         {
            UNION_VALUE uValue;
            for (int i = 0; i < mBands; i++)
            {
               // Maximum
               fread(&uValue, mBitsPerElement / 8, 1, pFile);
               double max = convertData(uValue, mDataType);
               mStatMax.push_back(max);

               // Minimum
               fread(&uValue, mBitsPerElement / 8, 1, pFile);
               double min = convertData(uValue, dataType);
               mStatMin.push_back(min);

               // Average
               double avg = 0.0;
               fread(&avg, sizeof(double), 1, pFile);
               endian.swapValue(avg);
               mStatAvg.push_back(avg);

               // Standard deviation
               double stddev = 0.0;
               fread(&stddev, sizeof(double), 1, pFile);
               endian.swapValue(stddev);
               mStatStdDev.push_back(stddev);
            }
         }
      }
      else if ((mVersion == 6) || (mVersion == 7) || (mVersion == 8))
      {
         unsigned int ulBytes = 0;
         fread(&ulBytes, sizeof(unsigned int), 1, pFile);
         endian.swapValue(ulBytes);

         int iCount = 0;
         if (mVersion == 8)
         {
            iCount = ulBytes / ((sizeof(double) * 4) + (sizeof(double) * 1001) +
               (sizeof(double) * 256) + (sizeof(unsigned int) * 256));
         }
         else
         {
            iCount = ulBytes / ((sizeof(double) * 4) + (sizeof(double) * 1001) +
               (sizeof(float) * 256) + (sizeof(unsigned int) * 256));
         }

         for (int i = 0; i < iCount; ++i)
         {
            double dValue = 0.0;

            // Maximum
            fread(&dValue, sizeof(double), 1, pFile);
            endian.swapValue(dValue);
            mStatMax.push_back(dValue);

            // Minimum
            fread(&dValue, sizeof(double), 1, pFile);
            endian.swapValue(dValue);
            mStatMin.push_back(dValue);

            // Average
            fread(&dValue, sizeof(double), 1, pFile);
            endian.swapValue(dValue);
            mStatAvg.push_back(dValue);

            // Standard deviation
            fread(&dValue, sizeof(double), 1, pFile);
            endian.swapValue(dValue);
            mStatStdDev.push_back(dValue);

            // Percentiles
            double* pPercentiles = new double[1001];
            fread(pPercentiles, sizeof(double), 1001, pFile);
            for (int j = 0; j < 1001; ++j)
            {
               endian.swapValue(pPercentiles[j]);
            }

            mStatPercentile.push_back(pPercentiles);

            // Bin centers
            double* pBinCenters = new double[256];
            if (mVersion == 8)
            {
               fread(pBinCenters, sizeof(double), 256, pFile);
               for (int j = 0; j < 256; ++j)
               {
                  endian.swapValue(pBinCenters[j]);
               }
            }
            else
            {
               vector<float> vecBinCenters(256);
               float* pFloatBinCenters = &vecBinCenters.front();
               fread(pFloatBinCenters, sizeof(float), 256, pFile);
               for (int j = 0; j < 256; ++j)
               {
                  endian.swapValue(pFloatBinCenters[j]);
                  pBinCenters[j] = static_cast<double>(pFloatBinCenters[j]);
               }
            }

            mStatBinCenter.push_back(pBinCenters);

            // Histogram
            unsigned int* pHistogram = new unsigned int[256];
            fread(pHistogram, sizeof(unsigned int), 256, pFile);
            for (int j = 0; j < 256; ++j)
            {
               endian.swapValue(pHistogram[j]);
            }

            mStatHistogram.push_back(pHistogram);
         }
      }
   }

   // Wavelengths
   if ((mVersion == 5) || (mVersion == 6))
   {
      for (int i = 0; i < mBands; ++i)
      {
         float freq[3];
         for (int j = 0; j < 3; ++j)
         {
            fread(&freq[j], sizeof(float), 1, pFile);
            endian.swapValue(freq[j]);
         }

         if (i < (mBands - mBadBands))
         {
            mCenterWavelengths.push_back(freq[0]);
            mStartWavelengths.push_back(freq[1]);
            mEndWavelengths.push_back(freq[2]);
         }
      }
   }
   else if ((mVersion == 7) || (mVersion == 8))
   {
      for (int i = 0; i < (mBands - mBadBands); ++i)
      {
         float freq[3];
         for (int j = 0; j < 3; ++j)
         {
            fread(&freq[j], sizeof(float), 1, pFile);
            endian.swapValue(freq[j]);
         }

         mCenterWavelengths.push_back(freq[0]);
         mStartWavelengths.push_back(freq[1]);
         mEndWavelengths.push_back(freq[2]);
      }
   }

   // Latitude/longitude GCPs
   memset(mParameters, 0, sizeof(mParameters));

   try
   {
      bool sentinelFound = false;
      while (sentinelFound == false)
      {
         int length = 0;
         int elements_read = fread(&length, sizeof(int), 1, pFile);
         if (elements_read != 1)
         {
            return false;
         }

         endian.swapValue(length);

         char pczBuffer[4096];
         if ((length > 0) && (length < sizeof(pczBuffer)))
         {
            fread(pczBuffer, length, 1, pFile);
            pczBuffer[length] = 0;

            if (strcmp(pczBuffer, ATTRIBUTE_SERIALIZATION_SENTINEL) == 0)
            {
               sentinelFound = true;
               break;
            }

            // Find current attribute matching saved name
            int i = 0;
            for (i = 0; i < INVALID_LAST_ENUM_ITEM_FLAG; i++)
            {
               if (strncmp(gpzAttribute_Descriptor[i].pczAttribute_Name, pczBuffer, length) == 0)
               {
                  break;
               }
            }

            if (i < INVALID_LAST_ENUM_ITEM_FLAG)
            {
               switch (gpzAttribute_Descriptor[i].eAttribute_Type)
               {
                  case LONG_VECTOR_PARM:
                     fread(&length, sizeof(int), 1, pFile);
                     endian.swapValue(length);

                     mParameters[i].lVector_Length = length;
                     mParameters[i].uParameter_Value.plzData = new int[length];
                     fread(mParameters[i].uParameter_Value.plzData, sizeof(int), length, pFile);
                     endian.swapBuffer(mParameters[i].uParameter_Value.plzData, sizeof(int), length);
                     mParameters[i].eParameter_Initialized = true;
                     break;

                  case FLOAT_VECTOR_PARM:
                     fread(&length, sizeof(int), 1, pFile);
                     endian.swapValue(length);

                     mParameters[i].lVector_Length = length;
                     mParameters[i].uParameter_Value.pfzData = new float[length];
                     fread(mParameters[i].uParameter_Value.plzData, sizeof(float), length, pFile);
                     endian.swapBuffer(mParameters[i].uParameter_Value.plzData, sizeof(float), length);
                     mParameters[i].eParameter_Initialized = true;
                     break;

                  case DOUBLE_VECTOR_PARM:
                     fread(&length, sizeof(int), 1, pFile);
                     endian.swapValue(length);

                     mParameters[i].lVector_Length = length;
                     mParameters[i].uParameter_Value.pdzData = new double[length];
                     fread(mParameters[i].uParameter_Value.plzData, sizeof(double), length, pFile);
                     endian.swapBuffer(mParameters[i].uParameter_Value.plzData, sizeof(double), length);
                     mParameters[i].eParameter_Initialized = true;
                     break;

                  case CSTRING_VECTOR_PARM:
                     fread(&length, sizeof(int), 1, pFile);
                     endian.swapValue(length);

                     mParameters[i].lVector_Length = length;
                     mParameters[i].uParameter_Value.pstrzData = new string*[length];
                     for (int k = 0; k < mParameters[i].lVector_Length; ++k)
                     {
                        fread(&length, sizeof(int), 1, pFile);
                        endian.swapValue(length);

                        mParameters[i].uParameter_Value.pstrzData[k] = new string;
                        fread(pczBuffer, length, 1, pFile);
                        pczBuffer[length] = 0;
                        mParameters[i].uParameter_Value.pstrzData[k]->append(pczBuffer);
                        mParameters[i].eParameter_Initialized = true;
                     }
                     break;

                  case CSTRING_PARM:
                     fread(&length, sizeof(int), 1, pFile);
                     endian.swapValue(length);

                     mParameters[i].uParameter_Value.pstrData = new string;
                     fread(pczBuffer, length, 1, pFile);
                     pczBuffer[length] = 0;
                     mParameters[i].uParameter_Value.pstrData->append(pczBuffer);
                     mParameters[i].eParameter_Initialized = true;
                     break;

                  case LONG_PARM:
                     fread(&mParameters[i].uParameter_Value.lData, sizeof(int), 1, pFile);
                     endian.swapValue(mParameters[i].uParameter_Value.lData);
                     if (mParameters[i].uParameter_Value.lData == -1)
                     {
                        mParameters[i].eParameter_Initialized = false;
                     }
                     else
                     {
                        mParameters[i].eParameter_Initialized = true;
                     }
                     break;

                  case FLOAT_PARM:
                     fread(&mParameters[i].uParameter_Value.fData, sizeof(float), 1, pFile);
                     endian.swapValue(mParameters[i].uParameter_Value.fData);
                     if (mParameters[i].uParameter_Value.fData == -1)
                     {
                        mParameters[i].eParameter_Initialized = false;
                     }
                     else
                     {
                        mParameters[i].eParameter_Initialized = true;
                     }
                     break;

                  case DOUBLE_PARM:
                     fread(&mParameters[i].uParameter_Value.dData, sizeof(double), 1, pFile);
                     endian.swapValue(mParameters[i].uParameter_Value.dData);
                     if (mParameters[i].uParameter_Value.dData == -1)
                     {
                        mParameters[i].eParameter_Initialized = false;
                     }
                     else
                     {
                        mParameters[i].eParameter_Initialized = true;
                     }
                     break;

                  default:
                     mParameters[i].eParameter_Initialized = false;
                     break;
               }
            }
         }
      }
   }
   catch (...)
   {
      return false;
   }

   // Classification and metadata
   int numEntries = 0;
   fread(&numEntries, sizeof(int), 1, pFile);
   endian.swapValue(numEntries);

   for (int i = 0; i < numEntries; ++i)
   {
      // Key
      string key;

      int length = 0;
      fread(&length, sizeof(int), 1, pFile);
      endian.swapValue(length);

      char buffer[4096];
      if (length <= sizeof(buffer))
      {
         fread(buffer, length, 1, pFile);
         buffer[length] = '\0';
         key = buffer;
      }

      // Value
      string value;

      fread(&length, sizeof(int), 1, pFile);
      endian.swapValue(length);

      if (length <= sizeof(buffer))
      {
         fread(buffer, length, 1, pFile);
         buffer[length] = '\0';
         value = buffer;
      }

      // Parse the string
      if (key.empty() == false)
      {
         string::size_type colonPos = key.find_first_of(':');
         if (colonPos == string::npos)
         {
            // Add to metadata
            if (key != "Metadata") // don't add if the root "Metadata" tag
            {
               string::size_type delimPos = value.find('\n');
               string type = value.substr(0, delimPos);
               string actualValue = value.substr(delimPos + 1);
               readDynamicObjectTags(mpMetadata.get(), key, type, actualValue);
            }
         }
         else
         {
            string destination = key.substr(0, colonPos);
            string shortKey = key.substr(colonPos + 1);
            if (destination == "BadValues")
            {
               if (shortKey == "allValues")
               {
                  formatBandBadValuesVector(mBadValues, value);
               }
            }
            else if (destination == "Classification")
            {
               if (shortKey == "level")
               {
                  mpClassification->setLevel(value);
               }
               else if (shortKey == "system")
               {
                  mpClassification->setSystem(value);
               }
               else if (shortKey == "codewords")
               {
                  mpClassification->setCodewords(value);
               }
               else if (shortKey == "fileControl")
               {
                  mpClassification->setFileControl(value);
               }
               else if (shortKey == "fileReleasing")
               {
                  mpClassification->setFileReleasing(value);
               }
               else if (shortKey == "declassificationDate")
               {
                  FactoryResource<DateTime> pDateTime;
                  formatDate(value, pDateTime.get());
                  if (pDateTime->getStructured() != 0)
                  {
                     mpClassification->setDeclassificationDate(pDateTime.get());
                  }
               }
               else if (shortKey == "declassificationExemption")
               {
                  mpClassification->setDeclassificationExemption(value);
               }
               else if (shortKey == "fileDowngrade")
               {
                  mpClassification->setFileDowngrade(value);
               }
               else if (shortKey == "countryCode")
               {
                  mpClassification->setCountryCode(value);
               }
               else if (shortKey == "downgradeDate")
               {
                  FactoryResource<DateTime> pDateTime;
                  formatDate(value, pDateTime.get());
                  if (pDateTime->getStructured() != 0)
                  {
                     mpClassification->setDowngradeDate(pDateTime.get());
                  }
               }
               else if (shortKey == "description")
               {
                  mpClassification->setDescription(value);
               }
               else if (shortKey == "authority")
               {
                  mpClassification->setAuthority(value);
               }
               else if (shortKey == "authorityType")
               {
                  mpClassification->setAuthorityType(value);
               }
               else if (shortKey == "securitySourceDate")
               {
                  FactoryResource<DateTime> pDateTime;
                  formatDate(value, pDateTime.get());
                  if (pDateTime->getStructured() != 0)
                  {
                     mpClassification->setSecuritySourceDate(pDateTime.get());
                  }
               }
               else if (shortKey == "securityControlNumber")
               {
                  mpClassification->setSecurityControlNumber(value);
               }
               else if (shortKey == "fileCopyNumber")
               {
                  mpClassification->setFileCopyNumber(value);
               }
               else if (shortKey == "fileNumberOfCopies")
               {
                  mpClassification->setFileNumberOfCopies(value);
               }
            }
            else if (destination == "Metadata")
            {
               string::size_type delimPos = value.find('\n');
               string type = value.substr(0, delimPos);
               string actualValue = value.substr(delimPos + 1);
               readDynamicObjectTags(mpMetadata.get(), shortKey, type, actualValue);
            }
         }
      }
   }

   // Units
   UnitType::EnumType eUnit = mUnitType;
   if (fread(&eUnit, sizeof(eUnit), 1, pFile) == 1)
   {
      endian.swapValue(eUnit);
      mUnitType = eUnit;
      int nameLength = 0;
      if (fread(&nameLength, sizeof(nameLength), 1, pFile) == 1)
      {
         endian.swapValue(nameLength);

         vector<char> name(nameLength + 1);
         if (fread(&name[0], nameLength, 1, pFile) == 1)
         {
            name[nameLength] = '\0';
            mUnitName = &name[0];
         }

         if (fread(&mRangeMin, sizeof(mRangeMin), 1, pFile) == 1)
         {
            endian.swapValue(mRangeMin);

            if (fread(&mRangeMax, sizeof(mRangeMax), 1, pFile) == 1)
            {
               endian.swapValue(mRangeMax);

               if (fread(&mScale, sizeof(mScale), 1, pFile) == 1)
               {
                  endian.swapValue(mScale);
               }
            }
         }
      }
   }

   // Original row, column, and band numbers
   // Rows
   int iCount = 0;
   fread(&iCount, sizeof(iCount), 1, pFile);
   endian.swapValue(iCount);

   for (int i = 0; i < iCount; i++)
   {
      unsigned int origRow = 0;
      fread(&origRow, sizeof(origRow), 1, pFile);
      endian.swapValue(origRow);

      mOrigRowNumbers.push_back(origRow);
   }

   // Columns
   fread(&iCount, sizeof(iCount), 1, pFile);
   endian.swapValue(iCount);

   for (int i = 0; i < iCount; i++)
   {
      unsigned int origColumn = 0;
      fread(&origColumn, sizeof(origColumn), 1, pFile);
      endian.swapValue(origColumn);

      mOrigColumnNumbers.push_back(origColumn);
   }

   // Bands
   fread(&iCount, sizeof(iCount), 1, pFile);
   endian.swapValue(iCount);

   for (int i = 0; i < iCount; i++)
   {
      unsigned int origBand = 0;
      fread(&origBand, sizeof(origBand), 1, pFile);
      endian.swapValue(origBand);

      mOrigBandNumbers.push_back(origBand);
   }

   return true;
}
