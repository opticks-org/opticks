/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "Classification.h"
#include "ColorType.h"
#include "DataAccessor.h"
#include "DataAccessorImpl.h"
#include "DataRequest.h"
#include "DimensionDescriptor.h"
#include "DynamicObject.h"
#include "Hdf5IncrementalWriter.h"
#include "Hdf5Utilities.h"
#include "IceWriter.h"
#include "Layer.h"
#include "ObjectResource.h"
#include "Progress.h"
#include "PseudocolorLayer.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterFileDescriptor.h"
#include "RasterUtilities.h"
#include "SpecialMetadata.h"
#include "Statistics.h"
#include "StatisticsReaderWriter.h"
#include "StringUtilities.h"
#include "StringUtilitiesMacros.h"
#include "TypesFile.h"
#include "Units.h"
#include "xmlwriter.h"

#include <iomanip>
#include <sstream>

using namespace std;

namespace
{
   unsigned int getDisplayedBandToStoreInFile(RasterChannelType channel,
      const RasterDataDescriptor* pDataDesc, const RasterFileDescriptor* pOutputFileDescriptor)
   {
      DimensionDescriptor displayBand = pDataDesc->getDisplayBand(channel);
      DimensionDescriptor loadedBand;
      if (displayBand.isOriginalNumberValid())
      {
         loadedBand = pDataDesc->getOriginalBand(displayBand.getOriginalNumber());
      }
      unsigned int bandNum = 0;
      if (loadedBand.isValid() && loadedBand.isActiveNumberValid())
      {
         unsigned int activeNumber = loadedBand.getActiveNumber();
         DimensionDescriptor fileBand = pOutputFileDescriptor->getActiveBand(activeNumber);
         //look up the band in the file descriptor to make sure that the band hasn't been chipped out.
         //if the band was chipped out then reset that displayed band to band 0.
         if (fileBand.isValid() && fileBand.isOnDiskNumberValid())
         {
            bandNum = fileBand.getOnDiskNumber();
         }
      }
      return bandNum;
   }
};

BEGIN_ENUM_MAPPING(IceCompressionType)
ADD_ENUM_MAPPING(NONE, "None", "none")
ADD_ENUM_MAPPING(GZIP, "GZIP", "gzip")
ADD_ENUM_MAPPING(SHUFFLE_AND_GZIP, "Shuffle+GZIP", "shuffle_gzip")
END_ENUM_MAPPING()

IceWriter::IceWriter(hid_t fileHandle, IceUtilities::FileType fileType) :
   mFileHandle(fileHandle),
   mAborted(false),
   mFileType(fileType),
   mChunkSize(std::max(IceWriter::getSettingChunkSize(), 1) * 1024 * 1024), // convert from MB to bytes
   mCompressionType(StringUtilities::fromXmlString<IceCompressionType>(IceWriter::getSettingCompressionType())),
   mGzipCompressionLevel(std::max(std::min(IceWriter::getSettingGzipCompressionLevel(), 9), 0))
{
}

void IceWriter::writeFileHeader()
{
   Hdf5GroupResource formatDescriptor(H5Gcreate(mFileHandle, "IceFormatDescriptor", 0));
   ICEVERIFY(*formatDescriptor >= 0)
   ICEVERIFY(HdfUtilities::writeAttribute<string>(*formatDescriptor, "Creator", string(APP_NAME)));
   ICEVERIFY(HdfUtilities::writeAttribute<string>(*formatDescriptor, "CreatorVersion", string(APP_VERSION_NUMBER)));
   Service<ConfigurationSettings> pSettings;  
   string os = pSettings->getOperatingSystemName();
   string arch = pSettings->getArchitectureName();
   ICEVERIFY(HdfUtilities::writeAttribute<string>(*formatDescriptor, "CreatorOS", os));
   ICEVERIFY(HdfUtilities::writeAttribute<string>(*formatDescriptor, "CreatorArch", arch));
   unsigned int majorVersion = 1;
   unsigned int minorVersion = 10; //support 0-99 minor versions
   unsigned int formatVersion = (majorVersion * 100) + minorVersion;
   ICEVERIFY(HdfUtilities::writeAttribute<unsigned int>(*formatDescriptor, "FormatVersion", formatVersion));
   ICEVERIFY(HdfUtilities::writeAttribute<string>(*formatDescriptor,
      "FileType", StringUtilities::toXmlString(mFileType)));
}

void IceWriter::writeCube(const string& hdfPath,
                          RasterElement* pCube,
                          const RasterFileDescriptor* pOutputFileDescriptor,
                          Progress* pProgress)
{
   string cubePath = hdfPath + "/RawData";
   HdfUtilities::createGroups(cubePath, mFileHandle); // create groups for this new dataset
   const RasterDataDescriptor* pDataDesc = dynamic_cast<const RasterDataDescriptor*>(pCube->getDataDescriptor());
   ICEVERIFY(pDataDesc != NULL);

   InterleaveFormatType interleave = pDataDesc->getInterleaveFormat();
   const vector<DimensionDescriptor>& bands = pOutputFileDescriptor->getBands();
   const vector<DimensionDescriptor>& rows = pOutputFileDescriptor->getRows();
   const vector<DimensionDescriptor>& cols = pOutputFileDescriptor->getColumns();
   unsigned int bpe = pDataDesc->getBytesPerElement();

   Hdf5DataSetResource dataId;
   if (interleave == BIL)
   {
      writeBilCubeData(cubePath, pCube, pOutputFileDescriptor, dataId, pProgress);
   }
   else if (interleave == BIP)
   {
      writeBipCubeData(cubePath, pCube, pOutputFileDescriptor, dataId, pProgress);
   }
   else if (interleave == BSQ)
   {
      writeBsqCubeData(cubePath, pCube, pOutputFileDescriptor, dataId, pProgress);
   }

   abortIfNecessary();

   //Write Out Attributes of Cube
   string interleaveString = StringUtilities::toXmlString(interleave);
   ICEVERIFY(HdfUtilities::writeAttribute(*dataId, "InterleaveFormat", interleaveString));

   //Write out original numbers
   vector<unsigned int> originalRowNums;
   vector<unsigned int> originalColNums;
   vector<unsigned int> originalBandNums;
   originalRowNums.reserve(rows.size());
   originalColNums.reserve(cols.size());
   originalBandNums.reserve(bands.size());

   for (vector<DimensionDescriptor>::const_iterator rowIter = rows.begin(); rowIter != rows.end(); ++rowIter)
   {
      originalRowNums.push_back((*rowIter).getOriginalNumber());
   }

   for (vector<DimensionDescriptor>::const_iterator col = cols.begin(); col != cols.end(); ++col)
   {
      originalColNums.push_back((*col).getOriginalNumber());
   }

   for (vector<DimensionDescriptor>::const_iterator band = bands.begin(); band != bands.end(); ++band)
   {
      originalBandNums.push_back((*band).getOriginalNumber());
   }

   abortIfNecessary();

   string originalNumbersPath = hdfPath + "/OriginalNumbers";
   HdfUtilities::createGroups(originalNumbersPath, mFileHandle, true);
   ICEVERIFY(HdfUtilities::writeDataset<vector<unsigned int> >(mFileHandle,
                                      originalNumbersPath + "/Row",
                                      originalRowNums));
   ICEVERIFY(HdfUtilities::writeDataset<vector<unsigned int> >(mFileHandle,
                                      originalNumbersPath + "/Column",
                                      originalColNums));
   ICEVERIFY(HdfUtilities::writeDataset<vector<unsigned int> >(mFileHandle,
                                      originalNumbersPath + "/Band",
                                      originalBandNums));

   abortIfNecessary();

   //Write the Metadata
   const DynamicObject* pOriginalMetadata = pDataDesc->getMetadata();
   ICEVERIFY(pOriginalMetadata != NULL);
   if (pOriginalMetadata->getNumAttributes() != 0)
   {
      FactoryResource<DynamicObject> pMetadata;
      ICEVERIFY(pMetadata.get() != NULL);
      pMetadata->merge(pOriginalMetadata);
      RasterUtilities::chipMetadata(pMetadata.get(), rows, cols, bands); 
      string pCenterWavelengthPath[] = { SPECIAL_METADATA_NAME, BAND_METADATA_NAME, 
         CENTER_WAVELENGTHS_METADATA_NAME, END_METADATA_NAME };
      string pStartWavelengthPath[] = { SPECIAL_METADATA_NAME, BAND_METADATA_NAME,
         START_WAVELENGTHS_METADATA_NAME, END_METADATA_NAME };
      string pEndWavelengthPath[] = { SPECIAL_METADATA_NAME, BAND_METADATA_NAME,
         END_WAVELENGTHS_METADATA_NAME, END_METADATA_NAME };
      string pBandNamesPath[] = { SPECIAL_METADATA_NAME,
         BAND_METADATA_NAME, NAMES_METADATA_NAME, END_METADATA_NAME };

      string wavelengthsPath = hdfPath + "/Wavelengths";
      HdfUtilities::createGroups(wavelengthsPath, mFileHandle, true);
      vector<double>* pCenterWavelengthVar = dv_cast<vector<double> >(&pMetadata->getAttributeByPath(
         pCenterWavelengthPath));
      if (pCenterWavelengthVar != NULL)
      {
         ICEVERIFY(HdfUtilities::writeDataset<vector<double> >(mFileHandle,
            wavelengthsPath + "/Center", *pCenterWavelengthVar));
      }
      pMetadata->removeAttributeByPath(pCenterWavelengthPath);
      vector<double>* pStartWavelengthVar = dv_cast<vector<double> >(&pMetadata->getAttributeByPath(
         pStartWavelengthPath));
      if (pStartWavelengthVar != NULL)
      {
         ICEVERIFY(HdfUtilities::writeDataset<vector<double> >(mFileHandle,
            wavelengthsPath + "/Start", *pStartWavelengthVar));
      }
      pMetadata->removeAttributeByPath(pStartWavelengthPath);
      vector<double>* pEndWavelengthVar = dv_cast<vector<double> >(&pMetadata->getAttributeByPath(pEndWavelengthPath));
      if (pEndWavelengthVar != NULL)
      {
         ICEVERIFY(HdfUtilities::writeDataset<vector<double> >(mFileHandle,
            wavelengthsPath + "/End", *pEndWavelengthVar));
      }
      pMetadata->removeAttributeByPath(pEndWavelengthPath);
      vector<string>* pBandNamesVar = dv_cast<vector<string> >(&pMetadata->getAttributeByPath(pBandNamesPath));
      if (pBandNamesVar != NULL)
      {
         ICEVERIFY(HdfUtilities::writeDataset<vector<string> >(mFileHandle,
            hdfPath + "/BandNames", *pBandNamesVar));
      }
      pMetadata->removeAttributeByPath(pBandNamesPath);

      writeDynamicObject(pMetadata.get(), hdfPath + "/Metadata", "CubeMetadata");
   }

   abortIfNecessary();

   //Write out the classification information
   const Classification* pClassification = pCube->getClassification();
   writeClassification(pClassification, hdfPath + "/Classification", pProgress);

   abortIfNecessary();

   //Write the Geo information
   if (pCube->isGeoreferenced())
   {
      if (!rows.empty() && !cols.empty())
      {
#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : This functionality should be moved into a method " \
   "in a new RasterElementExporterShell class (dsulgrov)")
         list<GcpPoint> gcps;
         unsigned int startRow = rows.front().getActiveNumber();
         unsigned int endRow = rows.back().getActiveNumber();
         unsigned int startCol = cols.front().getActiveNumber();
         unsigned int endCol = cols.back().getActiveNumber();
         GcpPoint urPoint;
         GcpPoint ulPoint;
         GcpPoint lrPoint;
         GcpPoint llPoint;
         GcpPoint centerPoint;
         ulPoint.mPixel = LocationType(startCol, startRow);
         urPoint.mPixel = LocationType(endCol, startRow);
         llPoint.mPixel = LocationType(startCol, endRow);
         lrPoint.mPixel = LocationType(endCol, endRow);
         centerPoint.mPixel = LocationType( (startCol + endCol) / 2, (startRow + endRow) / 2);

         ulPoint.mCoordinate = pCube->convertPixelToGeocoord(ulPoint.mPixel);
         urPoint.mCoordinate = pCube->convertPixelToGeocoord(urPoint.mPixel);
         llPoint.mCoordinate = pCube->convertPixelToGeocoord(llPoint.mPixel);
         lrPoint.mCoordinate = pCube->convertPixelToGeocoord(lrPoint.mPixel);
         centerPoint.mCoordinate = pCube->convertPixelToGeocoord(centerPoint.mPixel);

         //reset the coordinates, because on import they are required to be in
         //on-disk numbers not active numbers
         unsigned int diskStartRow = rows.front().getOnDiskNumber();
         unsigned int diskEndRow = rows.back().getOnDiskNumber();
         unsigned int diskStartCol = cols.front().getOnDiskNumber();
         unsigned int diskEndCol = cols.back().getOnDiskNumber();
         ulPoint.mPixel = LocationType(diskStartCol, diskStartRow);
         urPoint.mPixel = LocationType(diskEndCol, diskStartRow);
         llPoint.mPixel = LocationType(diskStartCol, diskEndRow);
         lrPoint.mPixel = LocationType(diskEndCol, diskEndRow);
         centerPoint.mPixel = LocationType( (diskStartCol + diskEndCol) / 2, (diskStartRow + diskEndRow) / 2);

         gcps.push_back(ulPoint);
         gcps.push_back(urPoint);
         gcps.push_back(llPoint);
         gcps.push_back(lrPoint);
         gcps.push_back(centerPoint);
         ICEVERIFY(HdfUtilities::writeDataset<list<GcpPoint> >(mFileHandle,
            hdfPath + "/GroundControlPoints", gcps));
      }
   }
   else
   {
      if (pProgress != NULL)
      {
         pProgress->updateProgress("The data set was not georeferenced "
            "so the exported data set will not contain geographic information."
            , 0, WARNING);
      }
   }

   string unitPath = hdfPath + "/Units";
   HdfUtilities::createGroups(unitPath, mFileHandle, true);
   Hdf5GroupResource unitGroup(H5Gopen(mFileHandle, unitPath.c_str()));

   const Units* pUnits = pDataDesc->getUnits();
   HdfUtilities::writeAttribute<double>(*unitGroup, "RangeMax", pUnits->getRangeMax());
   HdfUtilities::writeAttribute<double>(*unitGroup, "RangeMin", pUnits->getRangeMin());
   HdfUtilities::writeAttribute<double>(*unitGroup, "ScaleFromStandard", pUnits->getScaleFromStandard());
   HdfUtilities::writeAttribute<string>(*unitGroup, "Name", pUnits->getUnitName());
   HdfUtilities::writeAttribute<string>(*unitGroup, "Type", StringUtilities::toXmlString(pUnits->getUnitType()));

   string displayPath = hdfPath + "/DisplayInformation";
   HdfUtilities::createGroups(displayPath, mFileHandle, true);
   Hdf5GroupResource displayGroup(H5Gopen(mFileHandle, displayPath.c_str()));
   HdfUtilities::writeAttribute<unsigned int>(*displayGroup, "GrayDisplayedBand",
      getDisplayedBandToStoreInFile(GRAY, pDataDesc, pOutputFileDescriptor));
   HdfUtilities::writeAttribute<unsigned int>(*displayGroup, "RedDisplayedBand",
      getDisplayedBandToStoreInFile(RED, pDataDesc, pOutputFileDescriptor));
   HdfUtilities::writeAttribute<unsigned int>(*displayGroup, "GreenDisplayedBand",
      getDisplayedBandToStoreInFile(GREEN, pDataDesc, pOutputFileDescriptor));
   HdfUtilities::writeAttribute<unsigned int>(*displayGroup, "BlueDisplayedBand",
      getDisplayedBandToStoreInFile(BLUE, pDataDesc, pOutputFileDescriptor));
   HdfUtilities::writeAttribute<string>(*displayGroup, "DisplayMode",
      StringUtilities::toXmlString(pDataDesc->getDisplayMode()));
   HdfUtilities::writeAttribute<double>(*displayGroup, "XPixelSize", pDataDesc->getXPixelSize());
   HdfUtilities::writeAttribute<double>(*displayGroup, "YPixelSize", pDataDesc->getYPixelSize());

   string statPath = hdfPath + "/BandStatistics";
   HdfUtilities::createGroups(statPath, mFileHandle, true);
   vector<pair<Statistics*, unsigned int> > calculatedStatistics;
   { //scoped so that statMetadataWriter can close any resources it has open
      vector<hsize_t> statDimensions;
      statDimensions.push_back(bands.size());
      Hdf5IncrementalWriter<StatisticsMetadata> statMetadataWriter(mFileHandle,
         string(statPath + "/Metadata"), statDimensions);

      StatisticsMetadata statMetadata;
      hsize_t row = 0;
      for (vector<DimensionDescriptor>::const_iterator bandIter = bands.begin();
           bandIter != bands.end();
           ++bandIter, ++row)
      {
         //capture statistics for exported bands
         DimensionDescriptor curBand = *bandIter;
         if (!curBand.isValid() || !curBand.isOnDiskNumberValid() || !curBand.isActiveNumberValid())
         {
            continue;
         }
         int activeNumber = curBand.getActiveNumber();
         DimensionDescriptor loadedCurBand = pDataDesc->getActiveBand(activeNumber);
         Statistics* pStatistics = pCube->getStatistics(loadedCurBand);
         if (pStatistics != NULL)
         {
            if (pStatistics->areStatisticsCalculated())
            {
               calculatedStatistics.push_back(make_pair(pStatistics, curBand.getOnDiskNumber()));
            }
            statMetadata.mStatResolution = pStatistics->getStatisticsResolution();
            const vector<int>& badValues = pStatistics->getBadValues();
            statMetadata.mBadValues.len = badValues.size();
            if (statMetadata.mBadValues.len != 0)
            {
               statMetadata.mBadValues.p = const_cast<int*>(&(badValues.front()));
            }
            else
            {
               statMetadata.mBadValues.p = NULL;
            }
         }
         else
         {
            statMetadata.mStatResolution = 0;
            statMetadata.mBadValues.len = 0;
            statMetadata.mBadValues.p = NULL;
         }
         ICEVERIFY(statMetadataWriter.writeBlock(row, statMetadata));
      }
   }

   if (!calculatedStatistics.empty() && 
      pOutputFileDescriptor->getRows() == pDataDesc->getRows() &&
      pOutputFileDescriptor->getColumns() == pDataDesc->getColumns())
   {
      vector<hsize_t> dimensions;
      dimensions.push_back(calculatedStatistics.size());
      Hdf5IncrementalWriter<StatisticsValues> statWriter(mFileHandle,
         string(statPath + "/StoredStatistics"), dimensions);

      vector<pair<Statistics*, unsigned int> >::const_iterator bandIter;
      hsize_t row;
      StatisticsValues statVal;
      for (bandIter = calculatedStatistics.begin(), row = 0;
           bandIter != calculatedStatistics.end();
           ++bandIter, ++row)
      {
         Statistics* pStatistics = bandIter->first;
         statVal.mOnDiskBandNumber = bandIter->second;
         statVal.mAverage = pStatistics->getAverage();
         statVal.mMax = pStatistics->getMax();
         statVal.mMin = pStatistics->getMin();
         statVal.mStandardDeviation = pStatistics->getStandardDeviation();
         statVal.mpPercentiles.p = const_cast<double*>(pStatistics->getPercentiles());
         statVal.mpPercentiles.len = 1001;
         const double* pBinCenters = NULL;
         const unsigned int* pHistogramCounts = NULL;
         pStatistics->getHistogram(pBinCenters, pHistogramCounts);
         statVal.mpBinCenters.p = const_cast<double*>(pBinCenters);
         statVal.mpBinCenters.len = 256;
         statVal.mpHistogramCounts.p = const_cast<unsigned int*>(pHistogramCounts);
         statVal.mpHistogramCounts.len = 256;

         ICEVERIFY(statWriter.writeBlock(row, statVal));
      }
   }
   //NOTE: Currently the RasterElement::getTerrain() isn't being serialized and that is
   //intentional until some decisions are made about how terrains should be handled.

   abortIfNecessary();
}

void IceWriter::writeLayer(const string& hdfPath, const string& datasetPath, const Layer* pLayer, Progress* pProgress)
{
   // Write out the Dataset if one was provided.
   HdfUtilities::createGroups(hdfPath, mFileHandle, true);
   if (datasetPath.empty() == false)
   {
      Hdf5GroupResource layerGroup(H5Gopen(mFileHandle, hdfPath.c_str()));
      ICEVERIFY_MSG(HdfUtilities::writeAttribute(*layerGroup,
         "Dataset", datasetPath), "Unable to set Dataset path.");
   }

   // Write type-specific attributes
   const LayerType layerType = pLayer->getLayerType();
   switch (layerType)
   {
      case PSEUDOCOLOR:
      {
         writePseudocolorLayerProperties(hdfPath, reinterpret_cast<const PseudocolorLayer*>(pLayer), pProgress);
         break;
      }

      default:
      {
         ICEVERIFY_MSG(false, "\"" + StringUtilities::toDisplayString(layerType) + "\" is an unsupported layer type.");
      }
   }

   // Write common layer attributes
   writeLayerProperties(hdfPath, pLayer, pProgress);
   abortIfNecessary();
}

void IceWriter::writeClassification(const Classification* pClassification,
                                    const string& groupName,
                                    Progress* pProgress)
{
   if (pClassification != NULL)
   {
      HdfUtilities::createGroups(groupName, mFileHandle, true);
      Hdf5GroupResource classificationGroup(H5Gopen(mFileHandle, groupName.c_str()));
      ICEVERIFY(HdfUtilities::writeAttribute<string>(*classificationGroup, "Level", pClassification->getLevel()));
      ICEVERIFY(HdfUtilities::writeAttribute<string>(*classificationGroup, "System", pClassification->getSystem()));
      ICEVERIFY(HdfUtilities::writeAttribute<string>(*classificationGroup, "Codewords",
         pClassification->getCodewords()));
      ICEVERIFY(HdfUtilities::writeAttribute<string>(*classificationGroup, "Control",
         pClassification->getFileControl()));
      ICEVERIFY(HdfUtilities::writeAttribute<string>(*classificationGroup, "Releasing",
         pClassification->getFileReleasing()));
      ICEVERIFY(HdfUtilities::writeAttribute<string>(*classificationGroup, "Reason",
         pClassification->getClassificationReason()));
      ICEVERIFY(HdfUtilities::writeAttribute<string>(*classificationGroup, "DeclassificationType",
         pClassification->getDeclassificationType()));
      const DateTime* pDeclassDateTime = pClassification->getDeclassificationDate();
      FactoryResource<DateTime> pDeclassDateTimeRes;
      if (pDeclassDateTime == NULL)
      {
         pDeclassDateTime = pDeclassDateTimeRes.get();
      }
      ICEVERIFY(HdfUtilities::writeAttribute<DateTime>(*classificationGroup, "DeclassificationDate",
         *pDeclassDateTime));
      ICEVERIFY(HdfUtilities::writeAttribute<string>(*classificationGroup, "DeclassificationExemption",
         pClassification->getDeclassificationExemption()));
      ICEVERIFY(HdfUtilities::writeAttribute<string>(*classificationGroup, "DowngradeLevel",
         pClassification->getFileDowngrade()));
      const DateTime* pDowngradeDateTime = pClassification->getDowngradeDate();
      FactoryResource<DateTime> pDowngradeDateTimeRes;
      if (pDowngradeDateTime == NULL)
      {
         pDowngradeDateTime = pDowngradeDateTimeRes.get();
      }
      ICEVERIFY(HdfUtilities::writeAttribute<DateTime>(*classificationGroup, "DowngradeDate",
         *pDowngradeDateTime));
      ICEVERIFY(HdfUtilities::writeAttribute<string>(*classificationGroup, "CountryCode",
         pClassification->getCountryCode()));
      ICEVERIFY(HdfUtilities::writeAttribute<string>(*classificationGroup, "Description",
         pClassification->getDescription()));
      ICEVERIFY(HdfUtilities::writeAttribute<string>(*classificationGroup, "Authority",
         pClassification->getAuthority()));
      ICEVERIFY(HdfUtilities::writeAttribute<string>(*classificationGroup, "AuthorityType",
         pClassification->getAuthorityType()));
      const DateTime* pSourceDateTime = pClassification->getSecuritySourceDate();
      FactoryResource<DateTime> pSourceDateTimeRes;
      if (pSourceDateTime == NULL)
      {
         pSourceDateTime = pSourceDateTimeRes.get();
      }
      ICEVERIFY(HdfUtilities::writeAttribute<DateTime>(*classificationGroup, "SourceDate", *pSourceDateTime));
      ICEVERIFY(HdfUtilities::writeAttribute<string>(*classificationGroup, "SecurityControlNumber",
         pClassification->getSecurityControlNumber()));
      ICEVERIFY(HdfUtilities::writeAttribute<string>(*classificationGroup, "CopyNumber",
         pClassification->getFileCopyNumber()));
      ICEVERIFY(HdfUtilities::writeAttribute<string>(*classificationGroup, "NumberOfCopies",
         pClassification->getFileNumberOfCopies()));
      string classificationText;
      pClassification->getClassificationText(classificationText);
      ICEVERIFY(HdfUtilities::writeAttribute<string>(*classificationGroup, "ClassificationText", classificationText));

      writeDynamicObject(pClassification, groupName + "/Metadata", "ClassificationMetadata");
   }
   else
   {
      if (pProgress != NULL)
      {
         pProgress->updateProgress("Classification information could not exported."
            , 0, WARNING);
      }
   }
}

void IceWriter::writeDynamicObject(const DynamicObject* pDynObj,
                                   const string& datasetName,
                                   const string& rootElementName)
{
   if (pDynObj->getNumAttributes() != 0)
   {
      XMLWriter dynObjectWriter(rootElementName.c_str(), Service<MessageLogMgr>()->getLog());
      ICEVERIFY_MSG(pDynObj->toXml(&dynObjectWriter), "DynamicObject could not be serialized to xml.");
      string dynObjectContents = dynObjectWriter.writeToString();
      ICEVERIFY_MSG(HdfUtilities::writeDataset(mFileHandle,
         datasetName, dynObjectContents),
         "Dynamic Object could not be written to file.");
   }
}

void IceWriter::writeBilCubeData(const std::string& hdfPath,
                                 RasterElement* pCube,
                                 const RasterFileDescriptor* pOutputFileDescriptor,
                                 Hdf5DataSetResource& dataId,
                                 Progress* pProgress)
{
   RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(pCube->getDataDescriptor());
   ICEVERIFY(pDescriptor != NULL);
   const vector<DimensionDescriptor>& bands = pOutputFileDescriptor->getBands();
   const vector<DimensionDescriptor>& rows = pOutputFileDescriptor->getRows();
   const vector<DimensionDescriptor>& cols = pOutputFileDescriptor->getColumns();

   const vector<DimensionDescriptor>& cubeBands = pDescriptor->getBands();
   const vector<DimensionDescriptor>& cubeRows = pDescriptor->getRows();
   const vector<DimensionDescriptor>& cubeCols = pDescriptor->getColumns();

   unsigned int bpe = pDescriptor->getBytesPerElement();

   ICEVERIFY_MSG(!rows.empty() && !cols.empty() && !bands.empty(), "No data selected for export.")

   unsigned int startRow = rows.front().getActiveNumber();
   unsigned int endRow = rows.back().getActiveNumber();

   // set up the dataspace for the amount of data to read in
   hsize_t offset[3] = {0}; // the start offset to read in the file
   hsize_t counts[3] = {1, 1, 1}; // how much data to read at a time
   hsize_t dimSpace[3];
   hsize_t compSpace[3];

   dimSpace[0] = rows.size();
   compSpace[1] = dimSpace[1] = bands.size();
   compSpace[2] = dimSpace[2] = cols.size();

   unsigned int rowSize = cols.size() * bands.size() * bpe;
   unsigned int rowsInChunk = mChunkSize/rowSize; // determine number of rows that fit into a 1MB chunk
   if (rowsInChunk > rows.size())
   {
      rowsInChunk = rows.size();
   }
   if (rowsInChunk == 0)
   {
      rowsInChunk = 1;
   }
   compSpace[0] = rowsInChunk;

   createDatasetForCube(dimSpace, compSpace, pDescriptor->getDataType(), mFileHandle, hdfPath, dataId);
   Hdf5DataSpaceResource dspaceId(H5Dget_space(*dataId));
   ICEVERIFY(*dspaceId >= 0);
   Hdf5TypeResource hdfEncoding(H5Dget_type(*dataId));
   ICEVERIFY(*hdfEncoding >= 0);

   FactoryResource<DataRequest> pRequest;
   pRequest->setInterleaveFormat(BIL);
   pRequest->setRows(rows.front(), rows.back());
   DataAccessor da = pCube->getDataAccessor(pRequest.release());
   ICEVERIFY(da.isValid());

   bool bEntireRow = (cubeCols.size() == cols.size()) && (cubeBands.size() == bands.size());

   vector<char> pWriteBufferRes(rowsInChunk*rowSize, 0);
   char* pWriteBuffer = &pWriteBufferRes.front();

   counts[0] = rowsInChunk;
   counts[1] = bands.size();
   counts[2] = cols.size();

   Hdf5DataSpaceResource mspaceId(H5Screate_simple(3, counts, NULL));
   ICEVERIFY(*mspaceId >= 0);
   offset[1] = offset[2] = 0; // reset to beginning of rows and bands

   unsigned int numChunks = rows.size() / rowsInChunk;
   if (rows.size() % rowsInChunk != 0)
   {
      numChunks++;
   }

   abortIfNecessary();

   int rowIndex = 0;
   if (bEntireRow)
   {
      for (unsigned int chunkNumber = 0; chunkNumber < numChunks; ++chunkNumber)
      {
         unsigned int startChunkRow = chunkNumber * rowsInChunk;
         unsigned int endChunkRow = (chunkNumber + 1) * rowsInChunk;
         if (endChunkRow > rows.size())
         {
            endChunkRow = rows.size();
         }

         char* pBuffer = pWriteBuffer;
         for (unsigned int rowCount = startChunkRow; rowCount < endChunkRow; ++rowCount)
         {
            if (pProgress != NULL)
            {
               pProgress->updateProgress("Exporting cube...",
                  (rowIndex++ * 100) / rows.size(), NORMAL);
            }

            unsigned int rowActiveNum = rows[rowCount].getActiveNumber();
            da->toPixel(rowActiveNum, 0);
            ICEVERIFY(da.isValid());
            memcpy(pBuffer, da->getRow(), rowSize);
            pBuffer += rowSize;

            abortIfNecessary();
         }

         offset[0] = startChunkRow;

         if (chunkNumber + 1 == numChunks)
         {
            counts[0] = endChunkRow - startChunkRow;
            mspaceId = Hdf5DataSpaceResource(H5Screate_simple(3, counts, NULL));
            ICEVERIFY(*mspaceId >= 0);
         }

         herr_t status = H5Sselect_hyperslab(*dspaceId, H5S_SELECT_SET, offset, NULL, counts, NULL);
         ICEVERIFY(status >= 0);

         status = H5Dwrite(*dataId, *hdfEncoding, *mspaceId, *dspaceId, H5P_DEFAULT, pWriteBuffer);
         ICEVERIFY(status >= 0);
      }
   }
   else
   {
      unsigned int totalColumns = cubeCols.size();
      for (unsigned int chunkNumber = 0; chunkNumber < numChunks; ++chunkNumber)
      {
         unsigned int startChunkRow = chunkNumber * rowsInChunk;
         unsigned int endChunkRow = (chunkNumber + 1) * rowsInChunk;
         if (endChunkRow > rows.size())
         {
            endChunkRow = rows.size();
         }

         char* pBuffer = pWriteBuffer;
         for (unsigned int rowCount = startChunkRow; rowCount < endChunkRow; ++rowCount)
         {
            if (pProgress != NULL)
            {
               pProgress->updateProgress("Exporting cube...",
                  (rowIndex++ * 100) / rows.size(), NORMAL);
            }

            unsigned int rowActiveNum = rows[rowCount].getActiveNumber();
            da->toPixel(rowActiveNum, 0);
            ICEVERIFY(da.isValid());
            
            for (unsigned int bandCount = 0; bandCount < bands.size(); ++bandCount)
            {
               unsigned int bandActiveNum = bands[bandCount].getActiveNumber();
               for (unsigned int colCount = 0; colCount < cols.size(); ++colCount)
               {
                  unsigned int colActiveNum = cols[colCount].getActiveNumber();
                  memcpy(pBuffer, static_cast<char*>(da->getColumn()) +
                     (bandActiveNum * totalColumns * bpe) + (colActiveNum * bpe), bpe);
                  pBuffer += bpe;
               }
            }

            abortIfNecessary();
         }

         offset[0] = startChunkRow;

         if (chunkNumber + 1 == numChunks)
         {
            counts[0] = endChunkRow - startChunkRow;
            mspaceId = Hdf5DataSpaceResource(H5Screate_simple(3, counts, NULL));
            ICEVERIFY(*mspaceId >= 0);
         }

         herr_t status = H5Sselect_hyperslab(*dspaceId, H5S_SELECT_SET, offset, NULL, counts, NULL);
         ICEVERIFY(status >= 0);

         status = H5Dwrite(*dataId, *hdfEncoding, *mspaceId, *dspaceId, H5P_DEFAULT, pWriteBuffer);
         ICEVERIFY(status >= 0);
      }
   }
}

void IceWriter::writeBipCubeData(const string& hdfPath,
                                 RasterElement* pCube,
                                 const RasterFileDescriptor* pOutputFileDescriptor,
                                 Hdf5DataSetResource& dataId,
                                 Progress* pProgress)
{
   RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(pCube->getDataDescriptor());
   ICEVERIFY(pDescriptor != NULL)
   const vector<DimensionDescriptor>& bands = pOutputFileDescriptor->getBands();
   const vector<DimensionDescriptor>& rows = pOutputFileDescriptor->getRows();
   const vector<DimensionDescriptor>& cols = pOutputFileDescriptor->getColumns();

   const vector<DimensionDescriptor>& cubeBands = pDescriptor->getBands();
   const vector<DimensionDescriptor>& cubeRows = pDescriptor->getRows();
   const vector<DimensionDescriptor>& cubeCols = pDescriptor->getColumns();

   unsigned int bpe = pDescriptor->getBytesPerElement();

   ICEVERIFY_MSG(!rows.empty() && !cols.empty() && !bands.empty(), "No data selected for export.")

   unsigned int startRow = rows.front().getActiveNumber();
   unsigned int endRow = rows.back().getActiveNumber();

   // set up the dataspace for the amount of data to read in
   hsize_t offset[3] = {0}; // the start offset to read in the file
   hsize_t counts[3] = {1, 1, 1}; // how much data to read at a time
   hsize_t dimSpace[3];
   hsize_t compSpace[3];

   dimSpace[0] = rows.size();
   dimSpace[1] = cols.size();
   dimSpace[2] = bands.size();
   compSpace[1] = cols.size();
   compSpace[2] = bands.size();

   unsigned int rowSize = cols.size() * bands.size() * bpe;
   unsigned int rowsInChunk = mChunkSize/rowSize; // determine number of rows that fit into a chunk
   if (rowsInChunk > rows.size())
   {
      rowsInChunk = rows.size();
   }
   if (rowsInChunk == 0)
   {
      rowsInChunk = 1;
   }
   compSpace[0] = rowsInChunk;

   createDatasetForCube(dimSpace, compSpace, pDescriptor->getDataType(), mFileHandle, hdfPath, dataId );
   Hdf5DataSpaceResource dspaceId(H5Dget_space(*dataId));
   ICEVERIFY(*dspaceId >= 0);
   Hdf5TypeResource hdfEncoding(H5Dget_type(*dataId));
   ICEVERIFY(*hdfEncoding >= 0);

   FactoryResource<DataRequest> pRequest;
   pRequest->setInterleaveFormat(BIP);
   pRequest->setRows(rows.front(), rows.back());
   DataAccessor da = pCube->getDataAccessor(pRequest.release());
   ICEVERIFY(da.isValid());

   bool bEntireRow = (cubeCols.size() == cols.size()) && (cubeBands.size() == bands.size());

   vector<char> pWriteBufferRes(rowsInChunk*rowSize, 0);
   char* pWriteBuffer = &pWriteBufferRes.front();

   counts[0] = rowsInChunk;
   counts[1] = cols.size();
   counts[2] = bands.size();

   Hdf5DataSpaceResource mspaceId(H5Screate_simple(3, counts, NULL));
   ICEVERIFY(*mspaceId >= 0);
   offset[1] = 0;
   offset[2] = 0; // reset to beginning of rows and bands

   unsigned int numChunks = rows.size() / rowsInChunk;
   if (rows.size() % rowsInChunk != 0)
   {
      numChunks++;
   }

   abortIfNecessary();

   int rowIndex = 0;
   if (bEntireRow)
   {
      for (unsigned int chunkNumber = 0; chunkNumber < numChunks; ++chunkNumber)
      {
         unsigned int startChunkRow = chunkNumber * rowsInChunk;
         unsigned int endChunkRow = (chunkNumber + 1) * rowsInChunk;
         if (endChunkRow > rows.size())
         {
            endChunkRow = rows.size();
         }

         char* pBuffer = pWriteBuffer;
         for (unsigned int rowCount = startChunkRow; rowCount < endChunkRow; ++rowCount)
         {
            if (pProgress != NULL)
            {
               pProgress->updateProgress("Exporting cube...",
                  (rowIndex++ * 100) / rows.size(), NORMAL);
            }

            unsigned int rowActiveNum = rows[rowCount].getActiveNumber();
            da->toPixel( rowActiveNum, 0 );
            ICEVERIFY(da.isValid());
            memcpy(pBuffer, da->getRow(), rowSize);
            pBuffer += rowSize;
            
            abortIfNecessary();
         }

         offset[0] = startChunkRow;

         if (chunkNumber + 1 == numChunks)
         {
            counts[0] = endChunkRow - startChunkRow;
            mspaceId = Hdf5DataSpaceResource(H5Screate_simple(3, counts, NULL));
            ICEVERIFY(*mspaceId >= 0);
         }

         herr_t status = H5Sselect_hyperslab(*dspaceId, H5S_SELECT_SET, offset, NULL, counts, NULL);
         ICEVERIFY(status >= 0);

         status = H5Dwrite(*dataId, *hdfEncoding, *mspaceId, *dspaceId, H5P_DEFAULT, pWriteBuffer);
         ICEVERIFY(status >= 0);


      }
   }
   else
   {
      for (unsigned int chunkNumber = 0; chunkNumber < numChunks; ++chunkNumber)
      {
         unsigned int startChunkRow = chunkNumber * rowsInChunk;
         unsigned int endChunkRow = (chunkNumber + 1) * rowsInChunk;
         if (endChunkRow > rows.size())
         {
            endChunkRow = rows.size();
         }

         char* pBuffer = pWriteBuffer;
         for (unsigned int rowCount = startChunkRow; rowCount < endChunkRow; ++rowCount)
         {
            if (pProgress != NULL)
            {
               pProgress->updateProgress("Exporting cube...",
                  (rowIndex++ * 100) / rows.size(), NORMAL);
            }

            for (unsigned int colCount = 0; colCount < cols.size(); ++colCount)
            {
               unsigned int rowActiveNum = rows[rowCount].getActiveNumber();
               unsigned int colActiveNum = cols[colCount].getActiveNumber();
               da->toPixel( rowActiveNum, colActiveNum );
               ICEVERIFY(da.isValid());

               for (unsigned int bandCount = 0; bandCount < bands.size(); ++bandCount)
               {
                  unsigned int bandActiveNum = bands[bandCount].getActiveNumber();
                  memcpy(pBuffer,
                     static_cast<char*>(da->getColumn())+(bandActiveNum*bpe), bpe);
                  pBuffer += bpe;
               }
            }

            abortIfNecessary();
         }

         offset[0] = startChunkRow;

         if (chunkNumber + 1 == numChunks)
         {
            counts[0] = endChunkRow - startChunkRow;
            mspaceId = Hdf5DataSpaceResource(H5Screate_simple(3, counts, NULL));
            ICEVERIFY(*mspaceId >= 0);
         }

         herr_t status = H5Sselect_hyperslab(*dspaceId, H5S_SELECT_SET, offset, NULL, counts, NULL);
         ICEVERIFY(status >= 0);

         status = H5Dwrite(*dataId, *hdfEncoding, *mspaceId, *dspaceId, H5P_DEFAULT, pWriteBuffer);
         ICEVERIFY(status >= 0);
      }
   }
}

void IceWriter::writeBsqCubeData(const string& hdfPath,
                                 RasterElement* pCube,
                                 const RasterFileDescriptor* pOutputFileDescriptor,
                                 Hdf5DataSetResource& dataId,
                                 Progress* pProgress)
{
   RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(pCube->getDataDescriptor());
   ICEVERIFY(pDescriptor != NULL);
   const vector<DimensionDescriptor>& bands = pOutputFileDescriptor->getBands();
   const vector<DimensionDescriptor>& rows = pOutputFileDescriptor->getRows();
   const vector<DimensionDescriptor>& cols = pOutputFileDescriptor->getColumns();

   const vector<DimensionDescriptor>& cubeBands = pDescriptor->getBands();
   const vector<DimensionDescriptor>& cubeRows = pDescriptor->getRows();
   const vector<DimensionDescriptor>& cubeCols = pDescriptor->getColumns();

   unsigned int bpe = pDescriptor->getBytesPerElement();

   ICEVERIFY_MSG(!rows.empty() && !cols.empty() && !bands.empty(), "No data selected for export.")

   // set up the dataspace for the amount of data to read in
   hsize_t offset[3] = {0}; // the start offset to read in the file
   hsize_t counts[3] = {1, 1, 1}; // how much data to read at a time
   hsize_t dimSpace[3] = {1, 1, 1}; // define a memory dataspace that expresses how much of the file will be read

   hsize_t compSpace[3] = {1, 1, 1};

   dimSpace[0] = bands.size();
   dimSpace[1] = rows.size();
   dimSpace[2] = cols.size();

   // compress in chunks
   compSpace[0] = 1;
   counts[0] = 1; //only try to fit 1 band into a chunk
   compSpace[2] = cols.size();
   counts[2] = cols.size();

   unsigned int rowSize = cols.size() * bpe;
   unsigned int rowsInChunk = mChunkSize/rowSize; // determine number of rows that fit into a chunk
   if (rowsInChunk > rows.size())
   {
      rowsInChunk = rows.size();
   }
   if (rowsInChunk == 0)
   {
      rowsInChunk = 1;
   }
   compSpace[1] = rowsInChunk;
   counts[1] = rowsInChunk;

   createDatasetForCube(dimSpace, compSpace, pDescriptor->getDataType(), mFileHandle, hdfPath, dataId);
   Hdf5DataSpaceResource dSpaceId(H5Dget_space(*dataId));
   ICEVERIFY(*dSpaceId >= 0);
   Hdf5TypeResource hdfEncoding(H5Dget_type(*dataId));
   ICEVERIFY(*hdfEncoding >= 0);

   bool bEntireRow = (cubeCols.size() == cols.size());

   vector<char> pWriteBufferRes(rowsInChunk*rowSize, 0);
   char* pWriteBuffer = &pWriteBufferRes.front();

   Hdf5DataSpaceResource mspaceId(H5Screate_simple(3, counts, NULL));
   ICEVERIFY(*mspaceId >= 0);

   unsigned int numChunks = rows.size() / rowsInChunk;
   if (rows.size() % rowsInChunk != 0)
   {
      numChunks++;
      counts[1] = (rows.size() % rowsInChunk);
   }
   Hdf5DataSpaceResource lastChunkWriteId(H5Screate_simple(3, counts, NULL));

   abortIfNecessary();

   offset[2] = 0; //always write out a whole row
   int rowIndex = 0;
   for (unsigned int bandCount = 0; bandCount < bands.size(); ++bandCount)
   {
      offset[0] = bandCount;

      // BSQ the slow way
      FactoryResource<DataRequest> pRequest;
      pRequest->setInterleaveFormat(BSQ);
      pRequest->setBands(bands[bandCount], bands[bandCount]);
      DataAccessor da = pCube->getDataAccessor(pRequest.release());
      ICEVERIFY(da.isValid());

      for (unsigned int chunkNumber = 0; chunkNumber < numChunks; ++chunkNumber)
      {
         unsigned int startChunkRow = chunkNumber * rowsInChunk;
         unsigned int endChunkRow = (chunkNumber + 1) * rowsInChunk;
         if (endChunkRow > rows.size())
         {
            endChunkRow = rows.size();
         }

         char* pBuffer = pWriteBuffer;
         for (unsigned int rowCount = startChunkRow; rowCount < endChunkRow; ++rowCount)
         {
            if (pProgress != NULL)
            {
               pProgress->updateProgress("Exporting cube...",
                  (rowIndex++ * 100) / (bands.size() * rows.size()), NORMAL);
            }

            unsigned int rowActiveNum = rows[rowCount].getActiveNumber();
            for (unsigned int colCount = 0; colCount < cols.size(); ++colCount)
            {
               unsigned int colActiveNum = cols[colCount].getActiveNumber();
               da->toPixel(rowActiveNum, colActiveNum);
               ICEVERIFY(da.isValid());

               memcpy(pBuffer, da->getColumn(), bpe);
               pBuffer += bpe;
            }

            abortIfNecessary();
         }

         offset[1] = startChunkRow;
         counts[1] = endChunkRow - startChunkRow;

         hid_t memorySpace = *mspaceId;
         if (chunkNumber + 1 == numChunks)
         {
            memorySpace = *lastChunkWriteId;
         }

         herr_t status = H5Sselect_hyperslab(*dSpaceId, H5S_SELECT_SET, offset, NULL, counts, NULL);
         ICEVERIFY(status >= 0);

         status = H5Dwrite(*dataId, *hdfEncoding, memorySpace, *dSpaceId, H5P_DEFAULT, pWriteBuffer);
         ICEVERIFY(status >= 0);
      }
   }
}

void IceWriter::createDatasetForCube(hsize_t dimSpace[3],
                                       hsize_t chunkSpace[],
                                       EncodingType encoding,
                                       hid_t fd,
                                       const string& hdfPath,
                                       Hdf5DataSetResource& dataset )
{
   //get hdf encoding
   bool bCompressible = true;
   hid_t hdfType = 0;
   Hdf5TypeResource hdfTypeRes;
   switch (encoding)
   {
   case INT1SBYTE:
      hdfType = H5T_NATIVE_CHAR;
      break;
   case INT1UBYTE:
      hdfType = H5T_NATIVE_UCHAR;
      break;
   case INT2SBYTES:
      hdfType = H5T_NATIVE_SHORT;
      break;
   case INT2UBYTES:
      hdfType = H5T_NATIVE_USHORT;
      break;
   case INT4SBYTES:
      hdfType = H5T_NATIVE_INT;
      break;
   case INT4UBYTES:
      hdfType = H5T_NATIVE_UINT;
      break;
   case FLT4BYTES:
      hdfType = H5T_NATIVE_FLOAT;
      break;
   case FLT8BYTES:
      hdfType = H5T_NATIVE_DOUBLE;
      break;
   
   case INT4SCOMPLEX:
      hdfTypeRes = Hdf5TypeResource(H5Tcreate(H5T_COMPOUND, sizeof(IntegerComplex)));
      hdfType = *hdfTypeRes;
      H5Tinsert(hdfType, "Real", HOFFSET(IntegerComplex, mReal), H5T_NATIVE_SHORT);
      H5Tinsert(hdfType, "Imaginary", offsetof(IntegerComplex, mImaginary), H5T_NATIVE_SHORT);
      bCompressible = false;
      break;
   case FLT8COMPLEX:
      hdfTypeRes = Hdf5TypeResource(H5Tcreate(H5T_COMPOUND, sizeof(FloatComplex)));
      hdfType = *hdfTypeRes;
      H5Tinsert(hdfType, "Real", HOFFSET(FloatComplex, mReal), H5T_NATIVE_FLOAT);
      H5Tinsert(hdfType, "Imaginary", offsetof(FloatComplex, mImaginary), H5T_NATIVE_FLOAT);
      bCompressible = false;
      break;

   default:
      ICEVERIFY_MSG(false, "Unsupported encoding type");
   }

   hid_t plist = H5P_DEFAULT;
   if (bCompressible)
   {
      plist = H5Pcreate(H5P_DATASET_CREATE);
      herr_t status = H5Pset_chunk(plist, 3, chunkSpace);
      ICEVERIFY(status >= 0);
      switch(mCompressionType)
      {
      case SHUFFLE_AND_GZIP:
         status = H5Pset_shuffle(plist);
         ICEVERIFY(status >= 0);
         // fall through
      case GZIP:
         status = H5Pset_deflate(plist, mGzipCompressionLevel);
         ICEVERIFY(status >= 0);
         break;
      case NONE:
      default:
         break;
      }
   }

   Hdf5DataSpaceResource dspaceId(H5Screate_simple(3, dimSpace, NULL));
   ICEVERIFY(*dspaceId >= 0);

   dataset = Hdf5DataSetResource(H5Dcreate(fd, hdfPath.c_str(), hdfType, *dspaceId, plist));
   ICEVERIFY(*dataset >= 0);

   H5Pclose(plist);
}

void IceWriter::writePseudocolorLayerProperties(const string& hdfPath,
   const PseudocolorLayer* pLayer, Progress*pProgress)
{
   const string layerPath = hdfPath + "/PseudocolorLayer";
   HdfUtilities::createGroups(layerPath, mFileHandle, true);

   // Scope the Hdf5GroupResource
   {
      Hdf5GroupResource layerGroup(H5Gopen(mFileHandle, layerPath.c_str()));
      ICEVERIFY_MSG(*layerGroup >= 0, "Unable to open the group \"" + layerPath + "\".");

      ICEVERIFY_MSG(HdfUtilities::writeAttribute(*layerGroup, "Symbol",
         StringUtilities::toXmlString(pLayer->getSymbol())), "Unable to write Symbol.");
   }

   vector<int> classIds;
   pLayer->getClassIDs(classIds);
   unsigned int numClasses = classIds.size();
   if (numClasses != 0)
   {
      vector<string> classNames(numClasses);
      vector<int> classValues(numClasses);
      vector<unsigned char> classIsDisplayed(numClasses);
      vector<string> classColors(numClasses);
      for (unsigned int i = 0; i < numClasses; ++i)
      {
         ICEVERIFY_MSG(pLayer->getClassName(classIds[i], classNames[i]), "Unable to obtain the Class Name");
         classValues[i] = pLayer->getClassValue(classIds[i]);
         classIsDisplayed[i] = static_cast<unsigned char>(pLayer->isClassDisplayed(classIds[i]));
         classColors[i] = StringUtilities::toXmlString(pLayer->getClassColor(classIds[i]));
         abortIfNecessary();
      }

      ICEVERIFY_MSG(HdfUtilities::writeDataset(mFileHandle,
         layerPath + "/ClassNames", classNames), "Unable to write ClassNames.");

      ICEVERIFY_MSG(HdfUtilities::writeDataset(mFileHandle,
         layerPath + "/ClassValues", classValues), "Unable to write ClassValues.");

      ICEVERIFY_MSG(HdfUtilities::writeDataset(mFileHandle,
         layerPath + "/ClassIsDisplayed", classIsDisplayed), "Unable to write ClassIsDisplayed.");

      ICEVERIFY_MSG(HdfUtilities::writeDataset(mFileHandle,
         layerPath + "/ClassColors", classColors), "Unable to write ClassColors.");
   }
}

void IceWriter::writeLayerProperties(const string& hdfPath,
   const Layer* pLayer, Progress*pProgress)
{
   const string layerPath = hdfPath + "/Layer";
   HdfUtilities::createGroups(layerPath, mFileHandle, true);
   Hdf5GroupResource layerGroup(H5Gopen(mFileHandle, layerPath.c_str()));

   ICEVERIFY_MSG(HdfUtilities::writeAttribute(*layerGroup, "Name",
      pLayer->getName()), "Unable to write Layer name.");

   ICEVERIFY_MSG(HdfUtilities::writeAttribute(*layerGroup, "LayerType",
      StringUtilities::toXmlString(pLayer->getLayerType())), "Unable to write LayerType.");

   ICEVERIFY_MSG(HdfUtilities::writeAttribute(*layerGroup, "XScaleFactor",
      pLayer->getXScaleFactor()), "Unable to write XScaleFactor.");

   ICEVERIFY_MSG(HdfUtilities::writeAttribute(*layerGroup, "YScaleFactor",
      pLayer->getYScaleFactor()), "Unable to write YScaleFactor.");

   ICEVERIFY_MSG(HdfUtilities::writeAttribute(*layerGroup, "XOffset",
      pLayer->getXOffset()), "Unable to write XOffset.");

   ICEVERIFY_MSG(HdfUtilities::writeAttribute(*layerGroup, "YOffset",
      pLayer->getYOffset()), "Unable to write YOffset.");
}

void IceWriter::abort()
{
   mAborted = true;
}

void IceWriter::setChunkSize(int chunkSize)
{
   mChunkSize = chunkSize;
}

void IceWriter::setCompressionType(IceCompressionType type)
{
   mCompressionType = type;
}

void IceWriter::setGzipCompressionLevel(int level)
{
   mGzipCompressionLevel = level;
}

int IceWriter::getChunkSize() const
{
   return mChunkSize;
}

IceCompressionType IceWriter::getCompressionType() const
{
   return mCompressionType;
}

int IceWriter::getGzipCompressionLevel() const
{
   return mGzipCompressionLevel;
}

void IceWriter::abortIfNecessary()
{
   if (mAborted)
   {
      throw IceAbortException();
   }
}
