/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "AppVersion.h"
#include "DataRequest.h"
#include "DimensionDescriptor.h"
#include "Hdf4Utilities.h"
#include "ModisPager.h"
#include "PlugInArgList.h"
#include "PlugInRegistration.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterFileDescriptor.h"
#include "StringUtilities.h"

#include <mfhdf.h>
#include <vector>

REGISTER_PLUGIN_BASIC(OpticksModis, ModisPager);

ModisPager::ModisPager() :
   CachedPager(10 * 1024 * 1024 * 4),  // Specify a cache size large enough (40MB) to contain data for all
                                       // bands with the default number of concurrent rows, which provides
                                       // cache hits when performing interleave conversions on import
   mFileHandle(FAIL),
   mDatasetHandle(FAIL),
   mpMetadata(NULL),
   mRasterConversion()     // Default to an invalid conversion instead of the configuration setting value
                           // so that the values for the child raster elements will not be converted
{
   setName("MODIS Pager");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setDescription("Provides access to MODIS data in a file.");
   setShortDescription("On-disk MODIS data access");
   setDescriptorId("{698840EC-A3AA-45f6-BA25-6A75BCC07F22}");
   setVersion(APP_VERSION_NUMBER);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

ModisPager::~ModisPager()
{
   if (mDatasetHandle != FAIL)
   {
      SDendaccess(mDatasetHandle);
   }

   if (mFileHandle != FAIL)
   {
      SDend(mFileHandle);
   }
}

std::string ModisPager::MetadataArg()
{
   static std::string sArgName = "Metadata";
   return sArgName;
}

std::string ModisPager::DatasetNameArg()
{
   static std::string sArgName = "Dataset Name";
   return sArgName;
}

bool ModisPager::getInputSpecification(PlugInArgList*& pArgList)
{
   if (CachedPager::getInputSpecification(pArgList) == false)
   {
      return false;
   }

   VERIFY(pArgList != NULL);
   VERIFY(pArgList->addArg<DynamicObject>(ModisPager::MetadataArg(), NULL, "The metadata used to determine "
      "the HDF dataset containing the requested band data.  If no value is provided, the metadata in " +
      CachedPager::PagedElementArg() + " is used."));
   VERIFY(pArgList->addArg<std::string>(ModisPager::DatasetNameArg(), NULL, "The name of the HDF dataset "
      "containing the child element data.  This arg value should only be set when paging the child element data."));
   VERIFY(pArgList->addArg<ModisUtilities::RasterConversionType>(ModisUtilities::rasterConversionArg(),
      mRasterConversion, "Specifies whether the raster values should be converted to radiance or reflectance.  "
      "If no value is provided, the raster values will not be converted."));

   return true;
}

bool ModisPager::parseInputArgs(PlugInArgList* pArgList)
{
   if (CachedPager::parseInputArgs(pArgList) == false)
   {
      return false;
   }

   VERIFY(pArgList != NULL);

   // Metadata
   const DynamicObject* pMetadata = pArgList->getPlugInArgValue<DynamicObject>(ModisPager::MetadataArg());
   if (pMetadata != NULL)
   {
      // Copy the metadata since the given metadata may be deleted (e.g. metadata chipping on import)
      // before the raster data is obtained
      mpMetadata = FactoryResource<DynamicObject>();
      mpMetadata->merge(pMetadata);
   }

   // Dataset name
   std::string datasetName;
   if (pArgList->getPlugInArgValue<std::string>(ModisPager::DatasetNameArg(), datasetName) == true)
   {
      mDatasetName = datasetName;
   }

   // Raster pixel conversion
   VERIFY(pArgList->getPlugInArgValue<ModisUtilities::RasterConversionType>(ModisUtilities::rasterConversionArg(),
      mRasterConversion));

   return true;
}

bool ModisPager::openFile(const std::string& filename)
{
   if ((mFileHandle != FAIL) || (filename.empty() == true))
   {
      return false;
   }

   // Open the file
   mFileHandle = SDstart(filename.c_str(), DFACC_READ);

   // Open a specific HDF dataset only if a dataset has been requested
   if ((mFileHandle != FAIL) && (mDatasetName.empty() == false))
   {
      mDatasetHandle = SDselect(mFileHandle, SDnametoindex(mFileHandle, mDatasetName.c_str()));
      if (mDatasetHandle == FAIL)
      {
         SDend(mFileHandle);
         mFileHandle = FAIL;
         return false;
      }
   }

   return (mFileHandle != FAIL);
}

CachedPage::UnitPtr ModisPager::fetchUnit(DataRequest* pOriginalRequest)
{
   if (pOriginalRequest == NULL)
   {
      return CachedPage::UnitPtr();
   }

   // Check for interleave conversions, which are not supported by this pager
   const RasterElement* pRaster = getRasterElement();
   VERIFYRV(pRaster != NULL, CachedPage::UnitPtr());

   const RasterDataDescriptor* pDescriptor = dynamic_cast<const RasterDataDescriptor*>(pRaster->getDataDescriptor());
   VERIFYRV(pDescriptor != NULL, CachedPage::UnitPtr());

   const RasterFileDescriptor* pFileDescriptor =
      dynamic_cast<const RasterFileDescriptor*>(pDescriptor->getFileDescriptor());
   VERIFYRV(pFileDescriptor != NULL, CachedPage::UnitPtr());

   InterleaveFormatType requestedInterleave = pOriginalRequest->getInterleaveFormat();
   InterleaveFormatType fileInterleave = pFileDescriptor->getInterleaveFormat();
   if (requestedInterleave != fileInterleave)
   {
      return CachedPage::UnitPtr();
   }

   VERIFYRV(requestedInterleave == BSQ, CachedPage::UnitPtr());         // The MODIS datasets are BSQ

   // Get and validate the extents of the data to be loaded
   DimensionDescriptor startRow = pOriginalRequest->getStartRow();
   DimensionDescriptor stopRow = pOriginalRequest->getStopRow();
   unsigned int concurrentRows = pOriginalRequest->getConcurrentRows();

   DimensionDescriptor startColumn = pOriginalRequest->getStartColumn();
   DimensionDescriptor stopColumn = pOriginalRequest->getStopColumn();
   unsigned int concurrentColumns = pOriginalRequest->getConcurrentColumns();

   DimensionDescriptor startBand = pOriginalRequest->getStartBand();
   DimensionDescriptor stopBand = pOriginalRequest->getStopBand();
   unsigned int concurrentBands = pOriginalRequest->getConcurrentBands();

   if ((startRow.isOnDiskNumberValid() == false) || (stopRow.isOnDiskNumberValid() == false) ||
      (startColumn.isOnDiskNumberValid() == false) || (stopColumn.isOnDiskNumberValid() == false) ||
      (startBand.isOnDiskNumberValid() == false) || (stopBand.isOnDiskNumberValid() == false))
   {
      return CachedPage::UnitPtr();
   }

   if ((startRow.getOnDiskNumber() > stopRow.getOnDiskNumber()) ||
      (startColumn.getOnDiskNumber() > stopColumn.getOnDiskNumber()) ||
      (startBand.getOnDiskNumber() != stopBand.getOnDiskNumber()))      // The data accessor only allows one
                                                                        // concurrent band for BSQ data
   {
      return CachedPage::UnitPtr();
   }

   VERIFYRV(concurrentBands == 1, CachedPage::UnitPtr());

   if ((startRow.getActiveNumber() + concurrentRows - 1) > stopRow.getActiveNumber())
   {
      concurrentRows = stopRow.getActiveNumber() - startRow.getActiveNumber() + 1;
   }

   if ((startColumn.getActiveNumber() + concurrentColumns - 1) > stopColumn.getActiveNumber())
   {
      concurrentColumns = stopColumn.getActiveNumber() - startColumn.getActiveNumber() + 1;
   }

   // Get the HDF dataset name for the band and its index within the HDF dataset
   const DynamicObject* pMetadata = mpMetadata.get();
   if (pMetadata == NULL)
   {
      pMetadata = pRaster->getMetadata();
   }

   VERIFYRV(pMetadata != NULL, CachedPage::UnitPtr());

   std::string datasetName;
   int bandIndex = -1;
   unsigned int bandCount = 0;

   std::vector<ModisUtilities::HdfDatasetInfo> bandDatasets = ModisUtilities::getBandDatasets(pMetadata);
   if (bandDatasets.empty() == false)
   {
      for (std::vector<ModisUtilities::HdfDatasetInfo>::iterator iter = bandDatasets.begin();
         iter != bandDatasets.end();
         ++iter)
      {
         VERIFYRV(iter->second > 0, CachedPage::UnitPtr());

         if (startBand.getOnDiskNumber() < (bandCount + iter->second))
         {
            datasetName = iter->first;
            bandIndex = (startBand.getOnDiskNumber() - bandCount) % iter->second;
            break;
         }

         bandCount += iter->second;
      }
   }
   else
   {
      // Paging a child raster element, so the HDF dataset should already be open
      datasetName = mDatasetName;
      bandIndex = startBand.getOnDiskNumber();
   }

   if ((datasetName.empty() == true) || (bandIndex < 0))
   {
      return CachedPage::UnitPtr();
   }

   // Open the HDF dataset containing the band data
   if (mDatasetName != datasetName)
   {
      mDatasetName = datasetName;
      if (mDatasetHandle != FAIL)
      {
         SDendaccess(mDatasetHandle);
      }

      mDatasetHandle = SDselect(mFileHandle, SDnametoindex(mFileHandle, mDatasetName.c_str()));
      if (mDatasetHandle == FAIL)
      {
         return CachedPage::UnitPtr();
      }
   }

   VERIFYRV(mDatasetHandle != FAIL, CachedPage::UnitPtr());

   // Load the band data
   EncodingType inputDataType;

   int32 hdfDataType = 0;
   if (SDgetinfo(mDatasetHandle, NULL, NULL, NULL, &hdfDataType, NULL) == SUCCEED)
   {
      inputDataType = HdfUtilities::hdf4TypeToEncodingType(hdfDataType);
   }

   EncodingType outputDataType = pDescriptor->getDataType();
   switch (outputDataType)
   {
   case INT1UBYTE:
      return loadBand<unsigned char>(bandIndex, inputDataType, startRow, startColumn, startBand, concurrentRows,
         concurrentColumns);

   case INT1SBYTE:
      return loadBand<signed char>(bandIndex, inputDataType, startRow, startColumn, startBand, concurrentRows,
         concurrentColumns);

   case INT2UBYTES:
      return loadBand<unsigned short>(bandIndex, inputDataType, startRow, startColumn, startBand, concurrentRows,
         concurrentColumns);

   case INT2SBYTES:
      return loadBand<signed short>(bandIndex, inputDataType, startRow, startColumn, startBand, concurrentRows,
         concurrentColumns);

   case INT4UBYTES:
      return loadBand<unsigned int>(bandIndex, inputDataType, startRow, startColumn, startBand, concurrentRows,
         concurrentColumns);

   case INT4SBYTES:
      return loadBand<signed int>(bandIndex, inputDataType, startRow, startColumn, startBand, concurrentRows,
         concurrentColumns);

   case FLT4BYTES:
      return loadBand<float>(bandIndex, inputDataType, startRow, startColumn, startBand, concurrentRows,
         concurrentColumns);

   case FLT8BYTES:
      return loadBand<double>(bandIndex, inputDataType, startRow, startColumn, startBand, concurrentRows,
         concurrentColumns);

   default:
      break;
   }

   return CachedPage::UnitPtr();
}

template <typename Out>
CachedPage::UnitPtr ModisPager::loadBand(int bandIndex, EncodingType inputDataType,
                                         const DimensionDescriptor& startRow, const DimensionDescriptor& startColumn,
                                         const DimensionDescriptor& startBand, unsigned int concurrentRows,
                                         unsigned int concurrentColumns) const
{
   if ((mDatasetName.empty() == true) || (bandIndex < 0))
   {
      return CachedPage::UnitPtr();
   }

   // Set up the arrays for the call to SDreaddata()
   int32 numDims = 0;
   if (SDgetinfo(mDatasetHandle, NULL, &numDims, NULL, NULL, NULL) == FAIL)
   {
      return CachedPage::UnitPtr();
   }

   // Starting location in each dimension
   std::vector<int32> startValues;
   if (numDims >= 3)
   {
      startValues.push_back(bandIndex);
   }

   startValues.push_back(startRow.getOnDiskNumber());
   startValues.push_back(startColumn.getOnDiskNumber());

   // Skip factor in each dimension
   std::vector<int32> strideValues;
   if (numDims >= 3)
   {
      // Since only one band is being read at a time, there is no band skip factor
      strideValues.push_back(1);
   }

   const RasterElement* pRaster = getRasterElement();
   VERIFYRV(pRaster != NULL, CachedPage::UnitPtr());

   const RasterDataDescriptor* pDescriptor = dynamic_cast<const RasterDataDescriptor*>(pRaster->getDataDescriptor());
   VERIFYRV(pDescriptor != NULL, CachedPage::UnitPtr());

   strideValues.push_back(pDescriptor->getRowSkipFactor() + 1);
   strideValues.push_back(pDescriptor->getColumnSkipFactor() + 1);

   // Number of values to read in each dimension
   std::vector<int32> numValues;
   if (numDims >= 3)
   {
      numValues.push_back(1);
   }

   numValues.push_back(concurrentRows);
   numValues.push_back(concurrentColumns);

   // Allocate memory for the raw data from the file (source) and for the final results
   // which include the scales and offsets (destination)
   int numPixels = 1;
   for (std::vector<int32>::iterator iter = numValues.begin(); iter != numValues.end(); ++iter)
   {
      numPixels *= *iter;
   }

   int numBytes = numPixels * getBytesPerBand();

   ArrayResource<char> pSource(numBytes, true);
   void* pSrc = pSource.get();
   if (pSrc == NULL)
   {
      return CachedPage::UnitPtr();
   }

   ArrayResource<Out> pDestination(numPixels, true);
   Out* pDest = pDestination.get();
   if (pDest == NULL)
   {
      return CachedPage::UnitPtr();
   }

   // Read the raw band data from the file
   if (SDreaddata(mDatasetHandle, &startValues[0], &strideValues[0], &numValues[0], pSrc) == FAIL)
   {
      return CachedPage::UnitPtr();
   }

   // Copy the band data into the memory which will back the CachedPage
   bool success = false;
   switch (inputDataType)
   {
   case INT1UBYTE:
      success = populateBandData(reinterpret_cast<unsigned char*>(pSrc), pDest, numPixels, bandIndex);
      break;

   case INT1SBYTE:
      success = populateBandData(reinterpret_cast<signed char*>(pSrc), pDest, numPixels, bandIndex);
      break;

   case INT2UBYTES:
      success = populateBandData(reinterpret_cast<unsigned short*>(pSrc), pDest, numPixels, bandIndex);
      break;

   case INT2SBYTES:
      success = populateBandData(reinterpret_cast<signed short*>(pSrc), pDest, numPixels, bandIndex);
      break;

   case INT4UBYTES:
      success = populateBandData(reinterpret_cast<unsigned int*>(pSrc), pDest, numPixels, bandIndex);
      break;

   case INT4SBYTES:
      success = populateBandData(reinterpret_cast<signed int*>(pSrc), pDest, numPixels, bandIndex);
      break;

   case FLT4BYTES:
      success = populateBandData(reinterpret_cast<float*>(pSrc), pDest, numPixels, bandIndex);
      break;

   case FLT8BYTES:
      success = populateBandData(reinterpret_cast<double*>(pSrc), pDest, numPixels, bandIndex);
      break;

   default:
      break;
   }

   if (success == false)
   {
      return CachedPage::UnitPtr();
   }

   // Transfer ownership of the resulting data into a new page which will be owned by the caller of this method
   return CachedPage::UnitPtr(new CachedPage::CacheUnit(reinterpret_cast<char*>(pDestination.release()), startRow,
      concurrentRows, numBytes, startBand));
}

template <typename In, typename Out>
bool ModisPager::populateBandData(In* pInData, Out* pOutData, unsigned int numPixels, int bandIndex) const
{
   if ((pInData == NULL) || (pOutData == NULL))
   {
      return false;
   }

   // Get the valid data range and fill value
   DataVariant rangeVariant = getMetadataValue(VALID_RANGE);
   DataVariant fillValueVariant = getMetadataValue(FILL_VALUE);

   std::string rangeText = rangeVariant.toDisplayString();
   std::vector<In> range = StringUtilities::fromDisplayString<std::vector<In> >(rangeText);
   In fillValue = static_cast<In>(0.0);

   if ((range.size() != 2) || (fillValueVariant.getValue(fillValue) == false))
   {
      return false;
   }

   In rangeMin = range[0];
   In rangeMax = range[1];

   // Get the radiance scales and offsets
   Out radianceScale = static_cast<Out>(1.0);
   Out radianceOffset = static_cast<Out>(0.0);

   if (mRasterConversion == ModisUtilities::CONVERT_TO_RADIANCE)
   {
      if ((mDatasetName.empty() == true) || (bandIndex < 0))
      {
         return false;
      }

      std::vector<Out> scales;
      DataVariant scalesVariant = getMetadataValue(RADIANCE_SCALES);
      scalesVariant.getValue(scales);

      std::vector<Out> offsets;
      DataVariant offsetsVariant = getMetadataValue(RADIANCE_OFFSETS);
      offsetsVariant.getValue(offsets);

      if ((bandIndex >= static_cast<int>(scales.size())) || (bandIndex >= static_cast<int>(offsets.size())))
      {
         return false;
      }

      radianceScale = scales[bandIndex];
      radianceOffset = offsets[bandIndex];
   }

   // Get the reflectance scales and offsets
   Out reflectanceScale = static_cast<Out>(1.0);
   Out reflectanceOffset = static_cast<Out>(0.0);

   if (mRasterConversion == ModisUtilities::CONVERT_TO_REFLECTANCE)
   {
      if ((mDatasetName.empty() == true) || (bandIndex < 0))
      {
         return false;
      }

      std::vector<Out> scales;
      DataVariant scalesVariant = getMetadataValue(REFLECTANCE_SCALES);
      scalesVariant.getValue(scales);

      std::vector<Out> offsets;
      DataVariant offsetsVariant = getMetadataValue(REFLECTANCE_OFFSETS);
      offsetsVariant.getValue(offsets);

      if ((scales.empty() == false) && (offsets.empty() == false))   // The emissive datasets will not have
                                                                     // reflectance conversion factors
      {
         if ((bandIndex >= static_cast<int>(scales.size())) || (bandIndex >= static_cast<int>(offsets.size())))
         {
            return false;
         }

         reflectanceScale = scales[bandIndex];
         reflectanceOffset = offsets[bandIndex];
      }
   }

   // Get the scale factor
   double scaleFactor = 1.0;     // The scale factor attribute type is documented to be float64 in the MODIS Level
                                 // 1B Products Data Dictionary, Table 2.4.3, Page 106 (dated February 27, 2009)
   DataVariant scaleFactorVariant = getMetadataValue(SCALE_FACTOR);
   scaleFactorVariant.getValue(scaleFactor);

   // Iterate over each pixel populating the output data with the input pixel value, adjusted appropriately
   for (unsigned int i = 0; i < numPixels; ++i, ++pInData, ++pOutData)
   {
      In inValue = *pInData;
      Out outValue = static_cast<Out>(inValue);

      // Check if the pixel value is outside the valid range
      if ((inValue < rangeMin) || (inValue > rangeMax))
      {
         // Convert the value to the fill value
         outValue = static_cast<Out>(fillValue);
      }
      else
      {
         // Convert to radiance or reflectance by applying the scale and offset - The conversion equations are obtained
         // from the MODIS Level 1B Products Data Dictionary, Section 2.2.1, Pages 90 and 92 (dated February 27, 2009)
         if (mRasterConversion == ModisUtilities::CONVERT_TO_RADIANCE)
         {
            outValue = radianceScale * (outValue - radianceOffset);
         }
         else if (mRasterConversion == ModisUtilities::CONVERT_TO_REFLECTANCE)
         {
            outValue = reflectanceScale * (outValue - reflectanceOffset);
         }

         // Apply the scale factor
         outValue *= static_cast<Out>(scaleFactor);
      }

      *pOutData = outValue;
   }

   return true;
}

DataVariant ModisPager::getMetadataValue(const std::string& attributeName) const
{
   // Get the metadata
   const DynamicObject* pMetadata = mpMetadata.get();
   if (pMetadata == NULL)
   {
      const RasterElement* pRaster = getRasterElement();
      VERIFYRV(pRaster != NULL, DataVariant());

      const RasterDataDescriptor* pDescriptor = dynamic_cast<const RasterDataDescriptor*>(pRaster->getDataDescriptor());
      VERIFYRV(pDescriptor != NULL, DataVariant());

      pMetadata = pDescriptor->getMetadata();
   }

   VERIFYRV(pMetadata != NULL, DataVariant());

   // Get the attribute value from the metadata
   DataVariant attributeValue = pMetadata->getAttribute(attributeName);
   if ((attributeValue.isValid() == false) && (mDatasetName.empty() == false))
   {
      // The attribute was not present, so look for the attribute in the dataset group
      std::string datasetAttributeName = mDatasetName + "/" + attributeName;
      attributeValue = pMetadata->getAttributeByPath(datasetAttributeName);
   }

   return attributeValue;
}
