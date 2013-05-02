/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MODISUTILITIES_H
#define MODISUTILITIES_H

#include "DataVariantValidator.h"
#include "EnumWrapper.h"

#include <map>
#include <string>
#include <utility>
#include <vector>

// Metadata attribute names defined in the MODIS spec
#define ARCHIVE_METADATA "ArchiveMetadata.0"
#define CORE_METADATA "CoreMetadata.0"
#define STRUCT_METADATA "StructMetadata.0"
#define BAND_NAMES "band_names"
#define FILL_VALUE "_FillValue"
#define RADIANCE_OFFSETS "radiance_offsets"
#define RADIANCE_SCALES "radiance_scales"
#define RASTER_CONVERSION "Raster Conversion Type"
#define REFLECTANCE_OFFSETS "reflectance_offsets"
#define REFLECTANCE_SCALES "reflectance_scales"
#define SCALE_FACTOR "scale_factor"
#define UNITS "units"
#define VALID_RANGE "valid_range"

class DynamicObject;

namespace ModisUtilities
{
   // Product name
   std::string getModisProductName(const DynamicObject* pMetadata);

   // Resolution
   enum ResolutionTypeEnum
   {
      MODIS_QKM,
      MODIS_HKM,
      MODIS_1KM
   };

   typedef EnumWrapper<ResolutionTypeEnum> ResolutionType;

   ResolutionType getResolution(const DynamicObject* pMetadata);

   // Raster pixel conversion
   enum RasterConversionTypeEnum
   {
      NO_CONVERSION,
      CONVERT_TO_RADIANCE,
      CONVERT_TO_REFLECTANCE
   };

   typedef EnumWrapper<RasterConversionTypeEnum> RasterConversionType;

   RasterConversionType getRasterConversion(const DynamicObject* pMetadata);

   std::string rasterConversionArg();

   // HDF Datasets
   typedef std::pair<std::string, unsigned int> HdfDatasetInfo;
   std::vector<HdfDatasetInfo> getBandDatasets(const DynamicObject* pMetadata);

   std::pair<double, double> getBandWavelengths(unsigned int bandNumber);
   std::map<std::string, std::string> getGeolocationDatasets(const DynamicObject* pMetadata);
};

template <> class VariantTypeValidator<ModisUtilities::RasterConversionType> {};

#endif
