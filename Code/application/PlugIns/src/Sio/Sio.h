/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

// Sio.h
//
// This file contains common definitions and declarations used
// by both the Sio exporter and importer plug-ins.
//
#ifndef _SIO_H
#define _SIO_H

#include "Classification.h"
#include "DynamicObject.h"
#include "EnumWrapper.h"
#include "ObjectFactory.h"
#include "ObjectResource.h"
#include "TypesFile.h"

#include <stdio.h>
#include <string>
#include <vector>

enum eData_TypeEnum
{
   DATA_NONE,
   DATA_CHAR,
   DATA_UCHAR,
   DATA_SHORT,
   DATA_USHORT,
   DATA_LONG,
   DATA_ULONG,
   DATA_FLOAT,
   DATA_DOUBLE,
   DATA_ICOMPLEX,
   DATA_FCOMPLEX
};

/**
 * @EnumWrapper ::eData_TypeEnum.
 */
typedef EnumWrapper<eData_TypeEnum> eData_Type;

#define ATTRIBUTE_SERIALIZATION_SENTINEL "**** END OF FORMATTED ATTRIBUTES ****"

         // If the following enumeration is modified, then the
         // initialization of the class static data structure
         // in the cpp must be upgraded in corresponding fashion.
enum eHeader_ItemEnum 
{
   ORIGINAL_SENSOR,
   IMAGE_WIDTH,
   IMAGE_HEIGHT,
   IMAGE_DATA_FORMAT,
   IMAGE_BYTES_PER_PIXEL,
   IMAGE_BANDS,
   IMAGE_INTERLEAVING,
   IMAGE_COORDINATE_SYSTEM,
   IMAGE_HEADER_BYTES,
   IMAGE_BYTE_ORDER,
   UPPER_LEFT_CORNER_LAT,
   UPPER_LEFT_CORNER_LONG,
   UPPER_RIGHT_CORNER_LAT,
   UPPER_RIGHT_CORNER_LONG,
   LOWER_RIGHT_CORNER_LAT,
   LOWER_RIGHT_CORNER_LONG,
   LOWER_LEFT_CORNER_LAT,
   LOWER_LEFT_CORNER_LONG,
   CENTER_LAT,
   CENTER_LONG,
   CENTER_FREQUENCY_VECTOR,
   LOW_POWER_FREQUENCY_VECTOR,
   HIGH_POWER_FREQUENCY_VECTOR,
   SUN_AZIMUTH,
   SUN_ELEVATION,
   BAD_BANDS_COUNT,
   BAD_BANDS_VECTOR,
   DISPLAY_WIDTH,
   DISPLAY_HEIGHT,
   DISPLAY_DEPTH,
   DISPLAY_BANDS_VECTOR,
   DISPLAY_COLUMNS_VECTOR,
   DISPLAY_ROWS_VECTOR,
   LATITUDE_VECTOR,
   LONGITUDE_VECTOR,
   STATISTICS_BOOLS,
   INVALID_LAST_ENUM_ITEM_FLAG         // MUST ALWAYS BE LAST ENUM VALUE!!!
};

/**
 * @EnumWrapper ::eHeader_ItemEnum.
 */
typedef EnumWrapper<eHeader_ItemEnum> eHeader_Item;

typedef eHeader_Item Header_Key_Type;

            // The following structure describes a parameter type.
            // There will be one of these structures for each of the
            // parameter types in the enumeration for header data.
struct Parameter_List_Entry
{
   bool         eParameter_Initialized;   // Has the parm been set?
   int         lVector_Length;         // If a vector, how many?
   union                           // The values are kept here...
   {
      std::string*   pstrData;
      std::string**   pstrzData;
      int      lData;
      int*      plzData;
      double   dData;
      double*   pdzData;
      float      fData;
      float*   pfzData;
   } uParameter_Value;
};

            // The following vector contains the metadata for
            // each parameter type. This structure allows the methods
            // to properly, yet generically, handle each image
            // parameter value. It is initialized in the CPP, and must
            // be maintained if the enumeration for header data changes.
typedef enum 
{
   LONG_PARM,
   FLOAT_PARM,
   DOUBLE_PARM,
   CSTRING_PARM,
   LONG_VECTOR_PARM,
   FLOAT_VECTOR_PARM,
   DOUBLE_VECTOR_PARM,
   CSTRING_VECTOR_PARM
} eParm_Data_Type;

typedef struct
{
   eParm_Data_Type   eAttribute_Type;
   char*         pczAttribute_Name;
   bool         eSave_To_SIO;
} Attribute_Metadata;

union muValue
{
   unsigned char ucharInput;
   char          charInput;
   int           int2sInput;
   unsigned int  int2uInput;
   int           int4sInput;
   unsigned int  int4uInput;
   float         flt4Input;
   double        doubleInput;
};

typedef union muValue UNION_VALUE;

struct SioFile
{
   SioFile();
   ~SioFile();

   bool deserialize(FILE* pFile);

   int mVersion;
   int mOriginalVersion;
   EndianType mEndian;
   EncodingType mDataType;
   int mBitsPerElement;
   int mColumns;
   int mRows;
   int mBands;
   int mBadBands;
   int mOptimizations;
   std::vector<int> mBandToFileMap;
   std::vector< std::vector<int> > mBadValues;
   std::vector<double> mStatMin;
   std::vector<double> mStatMax;
   std::vector<double> mStatAvg;
   std::vector<double> mStatStdDev;
   std::vector<double*> mStatPercentile;
   std::vector<double*> mStatBinCenter;
   std::vector<unsigned int*> mStatHistogram;
   std::vector<float> mCenterWavelengths;
   std::vector<float> mStartWavelengths;
   std::vector<float> mEndWavelengths;
   Parameter_List_Entry mParameters[INVALID_LAST_ENUM_ITEM_FLAG];
   FactoryResource<Classification> mpClassification;
   FactoryResource<DynamicObject> mpMetadata;
   UnitType mUnitType;
   std::string mUnitName;
   double mRangeMin;
   double mRangeMax;
   double mScale;
   std::vector<unsigned int> mOrigRowNumbers;
   std::vector<unsigned int> mOrigColumnNumbers;
   std::vector<unsigned int> mOrigBandNumbers;
};

#endif
