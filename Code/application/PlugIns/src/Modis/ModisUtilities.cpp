/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "DataVariant.h"
#include "DynamicObject.h"
#include "ModisL1bImporter.h"
#include "ModisUtilities.h"
#include "StringUtilities.h"
#include "StringUtilitiesMacros.h"
#include "TypeConverter.h"

namespace StringUtilities
{
   BEGIN_ENUM_MAPPING_ALIAS(ModisUtilities::RasterConversionType, RasterConversionType)
      ADD_ENUM_MAPPING(ModisUtilities::NO_CONVERSION, "No Conversion", "NoConversion")
      ADD_ENUM_MAPPING(ModisUtilities::CONVERT_TO_RADIANCE, "Convert to Radiance", "ConvertToRadiance")
      ADD_ENUM_MAPPING(ModisUtilities::CONVERT_TO_REFLECTANCE, "Convert to Reflectance", "ConvertToReflectance")
   END_ENUM_MAPPING()
}

namespace TypeConverter
{
   template <>
   const char* toString<ModisUtilities::RasterConversionType>()
   {
      return "ModisUtilities::RasterConversionType";
   }

   template <>
   const char* toString<std::vector<ModisUtilities::RasterConversionType*> >()
   {
      return "vector<ModisUtilities::RasterConversionType>";
   }

   template <>
   const char* toString<std::vector<ModisUtilities::RasterConversionType> >()
   {
      return toString<std::vector<ModisUtilities::RasterConversionType*> >();
   }
}

std::string ModisUtilities::getModisProductName(const DynamicObject* pMetadata)
{
   if (pMetadata == NULL)
   {
      return std::string();
   }

   const std::string shortNamePath = CORE_METADATA +
      std::string("/INVENTORYMETADATA/COLLECTIONDESCRIPTIONCLASS/SHORTNAME");

   const DataVariant& shortNameValue = pMetadata->getAttributeByPath(shortNamePath);
   if (shortNameValue.isValid() == true)
   {
      std::string productName;
      if (shortNameValue.getValue(productName) == true)
      {
         return productName;
      }
   }

   return std::string();
}

ModisUtilities::ResolutionType ModisUtilities::getResolution(const DynamicObject* pMetadata)
{
   // Determine the resolution (250m, 500m, or 1km) by checking the MODIS product name value in the metadata
   std::string productName = getModisProductName(pMetadata);
   if ((productName.empty() == true) || (productName.length() < 6))
   {
      return ResolutionType();
   }

   // The resolution is determined by the character at index 5, which will be 'Q', 'H', or '1' for
   // quarter kilometer (250m), half kilometer (500m) and one kilometer (1km) resolution respectively
   switch (toupper(productName[5]))
   {
   case 'Q':
      return MODIS_QKM;

   case 'H':
      return MODIS_HKM;

   case '1':
      return MODIS_1KM;

   default:
      break;
   }

   return ResolutionType();
}

ModisUtilities::RasterConversionType ModisUtilities::getRasterConversion(const DynamicObject* pMetadata)
{
   // Get the raster conversion type from the metadata
   std::string rasterConversionText;
   if (pMetadata != NULL)
   {
      DataVariant rasterConversion = pMetadata->getAttribute(RASTER_CONVERSION);
      if (rasterConversion.isValid() == true)
      {
         rasterConversion.getValue(rasterConversionText);
      }
   }

   // Get the raster conversion from the configuration setting if the value does not exist in the metadata
   if (rasterConversionText.empty() == true)
   {
      rasterConversionText = ModisL1bImporter::getSettingRasterConversion();
   }

   return StringUtilities::fromXmlString<ModisUtilities::RasterConversionType>(rasterConversionText);
}

std::string ModisUtilities::rasterConversionArg()
{
   return "Raster Conversion";
}

std::vector<ModisUtilities::HdfDatasetInfo> ModisUtilities::getBandDatasets(const DynamicObject* pMetadata)
{
   ResolutionType resolution = getResolution(pMetadata);
   if (resolution.isValid() == false)
   {
      return std::vector<HdfDatasetInfo>();
   }

   // Specify the names of the HDF datasets containing the band data and the number of bands in the dataset
   std::vector<HdfDatasetInfo> bandDatasets;
   switch (resolution)
   {
   case MODIS_QKM:
      bandDatasets.push_back(std::make_pair("EV_250_RefSB", 2));
      break;

   case MODIS_HKM:
      bandDatasets.push_back(std::make_pair("EV_250_Aggr500_RefSB", 2));
      bandDatasets.push_back(std::make_pair("EV_500_RefSB", 5));
      break;

   case MODIS_1KM:
      bandDatasets.push_back(std::make_pair("EV_250_Aggr1km_RefSB", 2));
      bandDatasets.push_back(std::make_pair("EV_500_Aggr1km_RefSB", 5));
      bandDatasets.push_back(std::make_pair("EV_1KM_RefSB", 15));
      bandDatasets.push_back(std::make_pair("EV_1KM_Emissive", 16));
      break;

   default:
      break;
   }

   return bandDatasets;
}

std::pair<double, double> ModisUtilities::getBandWavelengths(unsigned int bandNumber)
{
   static std::vector<std::pair<double, double> > sBandWavelengths;
   if (sBandWavelengths.empty() == true)
   {
      // Wavelengths are obtained from http://mcst.gsfc.nasa.gov/calibration/information
      // and are ordered as (start wavelength, end wavelength)
      sBandWavelengths.push_back(std::make_pair(0.620, 0.670));      // Band 1
      sBandWavelengths.push_back(std::make_pair(0.841, 0.876));      // Band 2
      sBandWavelengths.push_back(std::make_pair(0.459, 0.479));      // Band 3
      sBandWavelengths.push_back(std::make_pair(0.545, 0.565));      // Band 4
      sBandWavelengths.push_back(std::make_pair(1.230, 1.250));      // Band 5
      sBandWavelengths.push_back(std::make_pair(1.628, 1.652));      // Band 6
      sBandWavelengths.push_back(std::make_pair(2.105, 2.155));      // Band 7
      sBandWavelengths.push_back(std::make_pair(0.405, 0.420));      // Band 8
      sBandWavelengths.push_back(std::make_pair(0.438, 0.448));      // Band 9
      sBandWavelengths.push_back(std::make_pair(0.483, 0.493));      // Band 10
      sBandWavelengths.push_back(std::make_pair(0.526, 0.536));      // Band 11
      sBandWavelengths.push_back(std::make_pair(0.546, 0.556));      // Band 12
      sBandWavelengths.push_back(std::make_pair(0.662, 0.672));      // Band 13 (hi and lo)
      sBandWavelengths.push_back(std::make_pair(0.673, 0.683));      // Band 14 (hi and lo)
      sBandWavelengths.push_back(std::make_pair(0.743, 0.753));      // Band 15
      sBandWavelengths.push_back(std::make_pair(0.862, 0.877));      // Band 16
      sBandWavelengths.push_back(std::make_pair(0.890, 0.920));      // Band 17
      sBandWavelengths.push_back(std::make_pair(0.931, 0.941));      // Band 18
      sBandWavelengths.push_back(std::make_pair(0.915, 0.965));      // Band 19
      sBandWavelengths.push_back(std::make_pair(3.660, 3.840));      // Band 20
      sBandWavelengths.push_back(std::make_pair(3.929, 3.989));      // Band 21
      sBandWavelengths.push_back(std::make_pair(3.929, 3.989));      // Band 22
      sBandWavelengths.push_back(std::make_pair(4.020, 4.080));      // Band 23
      sBandWavelengths.push_back(std::make_pair(4.433, 4.498));      // Band 24
      sBandWavelengths.push_back(std::make_pair(4.482, 4.549));      // Band 25
      sBandWavelengths.push_back(std::make_pair(1.360, 1.390));      // Band 26
      sBandWavelengths.push_back(std::make_pair(6.535, 6.895));      // Band 27
      sBandWavelengths.push_back(std::make_pair(7.175, 7.475));      // Band 28
      sBandWavelengths.push_back(std::make_pair(8.400, 8.700));      // Band 29
      sBandWavelengths.push_back(std::make_pair(9.580, 9.880));      // Band 30
      sBandWavelengths.push_back(std::make_pair(10.780, 11.280));    // Band 31
      sBandWavelengths.push_back(std::make_pair(11.770, 12.270));    // Band 32
      sBandWavelengths.push_back(std::make_pair(13.185, 13.485));    // Band 33
      sBandWavelengths.push_back(std::make_pair(13.485, 13.785));    // Band 34
      sBandWavelengths.push_back(std::make_pair(13.785, 14.085));    // Band 35
      sBandWavelengths.push_back(std::make_pair(14.085, 14.385));    // Band 36
   }

   if (bandNumber < sBandWavelengths.size())
   {
      return sBandWavelengths[bandNumber];
   }

   return std::make_pair(0.0, 0.0);
}

std::map<std::string, std::string> ModisUtilities::getGeolocationDatasets(const DynamicObject* pMetadata)
{
   // Specify the names of the HDF datasets containing the geolocation data and their location within the HDF file
   static std::string sGeolocationFieldLocation = "/MODIS_SWATH_Type_L1B/Geolocation Fields/";
   static std::string sDataFieldLocation = "/MODIS_SWATH_Type_L1B/Data Fields/";

   std::map<std::string, std::string> geolocationDatasets;
   geolocationDatasets["Latitude"] = sGeolocationFieldLocation;
   geolocationDatasets["Longitude"] = sGeolocationFieldLocation;

   ResolutionType resolution = getResolution(pMetadata);
   if (resolution == MODIS_1KM)
   {
      geolocationDatasets["Height"] = sDataFieldLocation;
      geolocationDatasets["SensorZenith"] = sDataFieldLocation;
      geolocationDatasets["SensorAzimuth"] = sDataFieldLocation;
      geolocationDatasets["Range"] = sDataFieldLocation;
      geolocationDatasets["SolarZenith"] = sDataFieldLocation;
      geolocationDatasets["SolarAzimuth"] = sDataFieldLocation;
   }

   return geolocationDatasets;
}
