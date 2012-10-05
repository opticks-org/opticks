/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "BadValues.h"
#include "Classification.h"
#include "DimensionDescriptor.h"
#include "DynamicObject.h"
#include "Hdf5Attribute.h"
#include "Hdf5Dataset.h"
#include "Hdf5File.h"
#include "Hdf5Group.h"
#include "Hdf5IncrementalReader.h"
#include "Hdf5Utilities.h"
#include "IceReader.h"
#include "ImportDescriptor.h"
#include "ObjectResource.h"
#include "PseudocolorLayer.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterFileDescriptor.h"
#include "RasterUtilities.h"
#include "SpatialDataView.h"
#include "SpecialMetadata.h"
#include "Statistics.h"
#include "StatisticsReaderWriter.h"
#include "StringUtilities.h"
#include "ThresholdLayer.h"
#include "TypesFile.h"
#include "Units.h"
#include "XercesIncludes.h"
#include "xmlreader.h"

#include <QtCore/QString>

using namespace std;

#define ICEWARNING(expr, msg) \
   if (expr) \
   { \
      mWarnings.push_back(msg); \
   }

#define ICEWARNING_RET(expr, msg, rv) \
   if (expr) \
   { \
      mWarnings.push_back(msg); \
      return rv; \
   }

#define ICEERROR_RET(expr, msg, rv) \
   if (expr) \
   { \
      mErrors.push_back(msg); \
      return rv; \
   }

#define ICEERROR(expr, msg) \
   if (expr) \
   { \
      mErrors.push_back(msg); \
   }

#define PARSE_ATTRIBUTE(hdf5element_var, attr_name, var_name, var_type) \
   if (!parseError) \
   { \
      Hdf5Attribute* pTempAttr = hdf5element_var->getAttribute(attr_name); \
      if (pTempAttr != NULL) \
      { \
         if (!pTempAttr->getValueAs<var_type>(var_name)) \
         { \
            parseError = true; \
         } \
      } \
      else \
      { \
         parseError = true; \
      } \
   }

#define PARSE_ATTRIBUTE_DEFAULT(hdf5element_var, attr_name, var_name, var_type, var_default) \
   if (!parseError) \
   { \
      PARSE_ATTRIBUTE(hdf5element_var, attr_name, var_name, var_type); \
      if (parseError) \
      { \
         warningMessage += "The value \"" + string(attr_name) + "\" could not be read and will be set to " + \
            StringUtilities::toDisplayString<var_type>(var_default) + ".\n"; \
         var_name = var_default; \
         parseError = false; \
      } \
   }

IceReader::IceReader(Hdf5File& iceFile) : mIceFile(iceFile)
{
   readFormatDescriptor();
}

void IceReader::readFormatDescriptor()
{
   Hdf5Group* pRoot = mIceFile.getRootGroup();
   MessageResource msg("File Header", "app", "2E806E3C-7739-4183-8529-2E7164921CAB", "Ice Reader");
   if (pRoot == NULL)
   {
      mIceFileValid = NOT_ICE;
      return;
   }

   mIceDescriptor = IceUtilities::createIceFormatDescriptor(pRoot);
   if (!mIceDescriptor.isValidIceFile())
   {
      mIceFileValid = NOT_ICE;
      return;
   }

   mIceDescriptor.addToMessage(msg.get());

   //Check for versions that we support importing
   if (!mIceDescriptor.getSupportedFeature(IceFormatDescriptor::FILE_FORMAT_VERSION))
   {
      mErrors.push_back(QString("Ice files that are version %1.%2 are no "
          "longer supported by this importer.  Please use an earlier "
          "version of the application to load and then re-export this "
          "file in Ice format.").arg(mIceDescriptor.getVersion().mMajorVersion)
          .arg(mIceDescriptor.getVersion().mMinorVersion).toStdString());
      mIceFileValid = NOT_SUPPORTED;
      return;
   }

   if (mIceDescriptor.getSupportedFeature(IceFormatDescriptor::FILE_FORMAT_VERSION_DEPRECATED))
   {
      mWarnings.push_back(QString("Ice files that are version %1.%2 are "
          "deprecated by this importer.  This file will still be loaded. "
          "In the future this importer may stop loading version %1.%2 Ice files. "
          "Please re-export this file using the Ice Exporter in order to "
          "create a newer Ice file.").arg(mIceDescriptor.getVersion().mMajorVersion)
          .arg(mIceDescriptor.getVersion().mMinorVersion).toStdString());
   }

   mIceFileValid = FULLY_SUPPORTED;
}

IceReader::ValidityType IceReader::getIceFileValidity()
{
   return mIceFileValid;
}

IceUtilities::FileType IceReader::getFileType()
{
   if (mIceFileValid != NOT_ICE && mIceDescriptor.isValidIceFile())
   {
      return mIceDescriptor.getFileType();
   }
   return IceUtilities::FileType();
}

vector<ImportDescriptor*> IceReader::getImportDescriptors()
{
   Hdf5Group* pRoot = mIceFile.getRootGroup();
   vector<ImportDescriptor*> descriptors;
   ImportDescriptor* pDesc = NULL;
   bool foundData = false;
   if (pRoot != NULL)
   {
      const Hdf5Group* pDataset = dynamic_cast<const Hdf5Group*>(pRoot->getElement("Datasets"));
      if (pDataset != NULL)
      {
         const Hdf5Group* pCube = dynamic_cast<const Hdf5Group*>(pDataset->getElement("Cube1"));
         if (pCube != NULL)
         {
            pDesc = getImportDescriptor(pCube);
            foundData = true;
         }
      }
   }
   ICEERROR(!foundData, "Ice file is improperly formatted, cannot locate cube data.");
   if (pDesc == NULL)
   {
      //return an empty descriptor so that validate() can provide more details
      //on the parse problem to the user.
      ImportDescriptorResource pImportDescriptor(mIceFile.getFilename(), "RasterElement");
      pDesc = pImportDescriptor.release();
   }

   descriptors.push_back(pDesc);
   return descriptors;
}

const vector<string>& IceReader::getWarnings()
{
   return mWarnings;
}

const vector<string>& IceReader::getErrors()
{
   return mErrors;
}

ImportDescriptor* IceReader::getImportDescriptor(const Hdf5Group* pCube)
{
   ImportDescriptorResource pImportDescriptor(mIceFile.getFilename(), "RasterElement");
   VERIFYRV(pImportDescriptor.get() != NULL, NULL);
   RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(pImportDescriptor->getDataDescriptor());
   VERIFYRV(pDescriptor != NULL, NULL);
   { //scope the pTempFileDescriptor resource      
      FactoryResource<RasterFileDescriptor> pTempFileDescriptor;
      VERIFYRV(pTempFileDescriptor.get() != NULL, NULL);
      pDescriptor->setFileDescriptor(pTempFileDescriptor.get());
   }

   RasterFileDescriptor* pFileDescriptor = dynamic_cast<RasterFileDescriptor*>(pDescriptor->getFileDescriptor());
   pFileDescriptor->setFilename(mIceFile.getFilename());

   const Hdf5Dataset* pData = dynamic_cast<const Hdf5Dataset*>(pCube->getElement("RawData"));
   ICEERROR_RET(pData == NULL, "Ice file is improperly formatted, cannot locate cube data.", NULL);

   if (!parseDimensionDescriptors(pCube, pDescriptor))
   {
      return NULL;
   }
   if (!parseClassification(pCube, pDescriptor))
   {
      return NULL;
   }
   if (!parseMetadata(pCube, pDescriptor))
   {
      return NULL;
   }
   if (!parseGcps(pCube, pDescriptor))
   {
      return NULL;
   }
   if (!parseUnits(pCube, pDescriptor))
   {
      return NULL;
   }
   if (!parseDisplayInformation(pCube, pDescriptor))
   {
      return NULL;
   }

   EncodingType encoding;
   pData->getDataEncoding(encoding);
   pFileDescriptor->setBitsPerElement(RasterUtilities::bytesInEncoding(encoding)*8);
   pDescriptor->setDataType(encoding);
   pDescriptor->setValidDataTypes(vector<EncodingType>(1, encoding));

   string fullPathAndName = pData->getFullPathAndName();
   pFileDescriptor->setDatasetLocation(fullPathAndName);

   return pImportDescriptor.release();
}

DynamicObject* IceReader::parseDynamicObject(const Hdf5Dataset* pDynObjDs, const string& elementTag)
{
   DO_IF(pDynObjDs == NULL, return NULL);
   auto_ptr<string> pMetadataString(pDynObjDs->readData<string>());
   DO_IF(pMetadataString.get() == NULL, return NULL);
   XmlReader reader(Service<MessageLogMgr>()->getLog(), false);
   XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* pDocument = reader.parseString(*pMetadataString);
   DO_IF(pDocument == NULL, return NULL);
   XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* pRootElement = pDocument->getDocumentElement();
   DO_IF(pRootElement == NULL, return NULL);
   XERCES_CPP_NAMESPACE_QUALIFIER DOMNodeList* pDynObjectList =
      pRootElement->getElementsByTagName(X(elementTag.c_str()));
   DO_IF( (pDynObjectList == NULL || pDynObjectList->getLength() != 1), return NULL);
   XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* pDynObject = pDynObjectList->item(0);
   DO_IF(pDynObject == NULL, return NULL);
   FactoryResource<DynamicObject> pObject;
   bool success = pObject->fromXml(pDynObject, XmlBase::VERSION);
   DO_IF(!success, return NULL);
   return pObject.release();
}

bool IceReader::parseMetadata(const Hdf5Group* pCube, RasterDataDescriptor* pDescriptor)
{
   const Hdf5Dataset* pMetadataDs = dynamic_cast<const Hdf5Dataset*>(pCube->getElement("Metadata"));
   if (pMetadataDs != NULL)
   {
      FactoryResource<DynamicObject> pObject(parseDynamicObject(pMetadataDs, "DynamicObject"));
      if (pObject.get() != NULL)
      {
         pDescriptor->setMetadata(pObject.get());
      }
      else
      {
         ICEWARNING(true, "Metadata detected in file, but could not be parsed properly.");
      }
   }   
   DynamicObject* pMetadata = pDescriptor->getMetadata();

   const Hdf5Dataset* pBandNames = dynamic_cast<const Hdf5Dataset*>(pCube->getElement("BandNames"));
   if (pBandNames != NULL)
   {
      auto_ptr<vector<string> > pBandNameData(pBandNames->readData<vector<string> >());
      bool bandNamesParsed = false;
      if (pBandNameData.get() != NULL)
      {
         if (pBandNameData->size() == pDescriptor->getBandCount())
         {
            string pBandNamesPath[] = { SPECIAL_METADATA_NAME,
               BAND_METADATA_NAME, NAMES_METADATA_NAME, END_METADATA_NAME };
            bandNamesParsed = pMetadata->setAttributeByPath(pBandNamesPath, *pBandNameData);
         }
      }
      ICEWARNING(!bandNamesParsed, "Band names detected in file, but are improperly formatted.");
   }

   const Hdf5Group* pWavelengthGroup = dynamic_cast<const Hdf5Group*>(pCube->getElement("Wavelengths"));
   if (pWavelengthGroup != NULL)
   {
      const Hdf5Dataset* pCenterWavelengthDs =
         dynamic_cast<const Hdf5Dataset*>(pWavelengthGroup->getElement("Center"));
      if (pCenterWavelengthDs != NULL)
      {
         auto_ptr<vector<double> > pCenterWavelengthData(pCenterWavelengthDs->readData<vector<double> >());
         bool centerWavelengthsParsed = false;
         if (pCenterWavelengthData.get() != NULL)
         {
            if (pCenterWavelengthData->size() == pDescriptor->getBandCount())
            {
               string pCenterWavelengthPath[] = { SPECIAL_METADATA_NAME, BAND_METADATA_NAME, 
                  CENTER_WAVELENGTHS_METADATA_NAME, END_METADATA_NAME };
               centerWavelengthsParsed = pMetadata->setAttributeByPath(pCenterWavelengthPath, *pCenterWavelengthData);
            }
         }
         ICEWARNING(!centerWavelengthsParsed, "Center wavelengths detected in file, but are improperly formatted.");
      }
      const Hdf5Dataset* pStartWavelengthDs = dynamic_cast<const Hdf5Dataset*>(pWavelengthGroup->getElement("Start"));
      if (pStartWavelengthDs != NULL)
      {
         auto_ptr<vector<double> > pStartWavelengthData(pStartWavelengthDs->readData<vector<double> >());
         bool startWavelengthsParsed = false;
         if (pStartWavelengthData.get() != NULL)
         {
            if (pStartWavelengthData->size() == pDescriptor->getBandCount())
            {
               string pStartWavelengthPath[] = { SPECIAL_METADATA_NAME, BAND_METADATA_NAME,
                  START_WAVELENGTHS_METADATA_NAME, END_METADATA_NAME };
               startWavelengthsParsed = pMetadata->setAttributeByPath(pStartWavelengthPath, *pStartWavelengthData);
            }
         }
         ICEWARNING(!startWavelengthsParsed, "Start wavelengths detected in file, but are improperly formatted.");
      }
      const Hdf5Dataset* pEndWavelengthDs = dynamic_cast<const Hdf5Dataset*>(pWavelengthGroup->getElement("End"));
      if (pEndWavelengthDs != NULL)
      {
         auto_ptr<vector<double> > pEndWavelengthData(pEndWavelengthDs->readData<vector<double> >());
         bool endWavelengthsParsed = false;
         if (pEndWavelengthData.get() != NULL)
         {
            if (pEndWavelengthData->size() == pDescriptor->getBandCount())
            {
               string pEndWavelengthPath[] = { SPECIAL_METADATA_NAME, BAND_METADATA_NAME,
                  END_WAVELENGTHS_METADATA_NAME, END_METADATA_NAME };
               endWavelengthsParsed = pMetadata->setAttributeByPath(pEndWavelengthPath, *pEndWavelengthData);
            }
         }
         ICEWARNING(!endWavelengthsParsed, "End wavelengths detected in file, but are improperly formatted.");
      }
   }

   return true;
}

bool IceReader::parseDimensionDescriptors(const Hdf5Group* pCube, RasterDataDescriptor* pDescriptor)
{
   const Hdf5Dataset* pData = dynamic_cast<const Hdf5Dataset*>(pCube->getElement("RawData"));
   ICEERROR_RET(pData == NULL, "Ice file is improperly formatted, cannot locate cube data.", false);
   RasterFileDescriptor* pFileDescriptor = dynamic_cast<RasterFileDescriptor*>(pDescriptor->getFileDescriptor());

   vector<hsize_t> dimensions = pData->getDimensionSizes();
   ICEERROR_RET(dimensions.size() != 3,
      "Ice file is improperly formatted: data has incorrect number of dimensions.", false);

   Hdf5Attribute* pAttr = NULL;
   string interleaveStr = "Unknown Interleave";
   pAttr = pData->getAttribute("InterleaveFormat");
   if (pAttr != NULL)
   {
      pAttr->getValueAs<string>(interleaveStr);
   }
   unsigned int numRows = 0;
   unsigned int numColumns = 0;
   unsigned int numBands = 0;

   bool parseError;
   InterleaveFormatType interleave = StringUtilities::fromXmlString<InterleaveFormatType>(interleaveStr, &parseError);
   ICEWARNING_RET(parseError, "The interleave format, number of rows, "
      "columns and bands cannot automatically be determined, will use "
      "user provided values.", true);

   pFileDescriptor->setInterleaveFormat(interleave);
   pDescriptor->setInterleaveFormat(interleave);
   switch (interleave)
   {
   case BIP: // row, column, band
      numRows = static_cast<unsigned int>(dimensions[0]);
      numColumns = static_cast<unsigned int>(dimensions[1]);
      numBands = static_cast<unsigned int>(dimensions[2]);
      break;
   case BSQ: // band, row, column
      numBands = static_cast<unsigned int>(dimensions[0]);
      numRows = static_cast<unsigned int>(dimensions[1]);
      numColumns = static_cast<unsigned int>(dimensions[2]);
      break;
   case BIL: // row, band, column
      numRows = static_cast<unsigned int>(dimensions[0]);
      numBands = static_cast<unsigned int>(dimensions[1]);
      numColumns = static_cast<unsigned int>(dimensions[2]);
      break;
   default:
      ICEWARNING_RET(true, "The number of rows, columns and bands cannot automatically be determined; "
         "will use user-provided values.", true);
   }

   vector<unsigned int> originalRows;
   vector<unsigned int> originalCols;
   vector<unsigned int> originalBands;
   if (mIceDescriptor.getSupportedFeature(IceFormatDescriptor::ORIGINAL_NUMBERS_IN_DATASET))
   {
      const Hdf5Group* pOriginalGroup = dynamic_cast<const Hdf5Group*>(pCube->getElement("OriginalNumbers"));
      if (pOriginalGroup != NULL)
      {
         const Hdf5Dataset* pBandOriginalDs = dynamic_cast<const Hdf5Dataset*>(pOriginalGroup->getElement("Band"));
         if (pBandOriginalDs != NULL)
         {
            auto_ptr<vector<unsigned int> > bandOriginalNumbers(pBandOriginalDs->readData<vector<unsigned int> >());
            if (bandOriginalNumbers.get() != NULL)
            {
               originalBands = *bandOriginalNumbers;
            }
         }

         const Hdf5Dataset* pRowOriginalDs = dynamic_cast<const Hdf5Dataset*>(pOriginalGroup->getElement("Row"));
         if (pRowOriginalDs != NULL)
         {
            auto_ptr<vector<unsigned int> > rowOriginalNumbers(pRowOriginalDs->readData<vector<unsigned int> >());
            if (rowOriginalNumbers.get() != NULL)
            {
               originalRows = *rowOriginalNumbers;
            }
         }

         const Hdf5Dataset* pColumnOriginalDs = dynamic_cast<const Hdf5Dataset*>(pOriginalGroup->getElement("Column"));
         if (pColumnOriginalDs != NULL)
         {
            auto_ptr<vector<unsigned int> > columnOriginalNumbers(
               pColumnOriginalDs->readData<vector<unsigned int> >());
            if (columnOriginalNumbers.get() != NULL)
            {
               originalCols = *columnOriginalNumbers;
            }
         }
      }
   }
   else
   {
      pAttr = pData->getAttribute("Original Cube Row Numbers");
      if (pAttr != NULL)
      {
         pAttr->getValueAs<vector<unsigned int> >(originalRows);
      }
      pAttr = pData->getAttribute("Original Cube Column Numbers");
      if (pAttr != NULL)
      {
         pAttr->getValueAs<vector<unsigned int> >(originalCols);
      }
      pAttr = pData->getAttribute("Original Cube Band Numbers");
      if (pAttr != NULL)
      {
         pAttr->getValueAs<vector<unsigned int> >(originalBands);
      }
   }

   bool orgBandsCorrect = originalBands.size() == numBands;
   bool orgRowsCorrect = originalRows.size() == numRows;
   bool orgColumnsCorrect = originalCols.size() == numColumns;
   ICEWARNING(!orgBandsCorrect,
      "Original band numbers could not be detected in the file or were improperly formatted.");
   ICEWARNING(!orgRowsCorrect,
      "Original row numbers could not be detected in the file or were improperly formatted.");
   ICEWARNING(!orgColumnsCorrect,
      "Original column numbers could not be detected in the file or were improperly formatted.");

   vector<DimensionDescriptor> rows;
   vector<DimensionDescriptor> columns;
   vector<DimensionDescriptor> bands;
   unsigned int t;
   for (t = 0; t < numRows; ++t)
   {
      DimensionDescriptor desc;
      if (orgRowsCorrect)
      {
         desc.setOriginalNumber(originalRows[t]);
      }
      else
      {
         desc.setOriginalNumber(t);
      }
      desc.setOnDiskNumber(t);
      rows.push_back(desc);
   }

   for (t = 0; t < numColumns; ++t)
   {
      DimensionDescriptor desc;
      if (orgColumnsCorrect)
      {
         desc.setOriginalNumber(originalCols[t]);
      }
      else
      {
         desc.setOriginalNumber(t);
      }
      desc.setOnDiskNumber(t);
      columns.push_back(desc);
   }

   for (t = 0; t < numBands; ++t)
   {
      DimensionDescriptor desc;
      if (orgBandsCorrect)
      {
         desc.setOriginalNumber(originalBands[t]);
      }
      else
      {
         desc.setOriginalNumber(t);
      }
      desc.setOnDiskNumber(t);
      bands.push_back(desc);
   }

   pFileDescriptor->setRows(rows);
   pDescriptor->setRows(rows);

   pFileDescriptor->setColumns(columns);
   pDescriptor->setColumns(columns);

   pFileDescriptor->setBands(bands);
   pDescriptor->setBands(bands);

   return true;
}

bool IceReader::parseClassification(const Hdf5Group* pCube, RasterDataDescriptor* pDescriptor)
{
   if (mIceDescriptor.getSupportedFeature(IceFormatDescriptor::CUBE_CLASSIFICATION))
   {
      const Hdf5Group* pClassificationGroup = dynamic_cast<const Hdf5Group*>(pCube->getElement("Classification"));
      if (pClassificationGroup != NULL)
      {
         bool parseError = false;
         FactoryResource<Classification> pClassification;
         string level;
         string system;
         string codewords;
         string control;
         string releasing;
         string reason;
         string declassType;
         string declassEx;
         string downgradeLevel;
         string countryCode;
         string desc;
         string authority;
         string authorityType;
         string secControlNum;
         string copyNumber;
         string numberOfCopies;
         FactoryResource<DateTime> pDeclassDate(NULL);
         FactoryResource<DateTime> pDowngradeDate(NULL);
         FactoryResource<DateTime> pSourceDate(NULL);
         PARSE_ATTRIBUTE(pClassificationGroup, "Level", level, string);
         bool parsedLevel = !parseError;

         //set classification level here so even if the rest can't be parsed we preserve it.
         pClassification->setLevel(level);
         PARSE_ATTRIBUTE(pClassificationGroup, "System", system, string);
         PARSE_ATTRIBUTE(pClassificationGroup, "Codewords", codewords, string);
         PARSE_ATTRIBUTE(pClassificationGroup, "Control", control, string);
         PARSE_ATTRIBUTE(pClassificationGroup, "Releasing", releasing, string);
         PARSE_ATTRIBUTE(pClassificationGroup, "Reason", reason, string);
         PARSE_ATTRIBUTE(pClassificationGroup, "DeclassificationType", declassType, string);
         PARSE_ATTRIBUTE(pClassificationGroup, "DeclassificationExemption", declassEx, string);
         PARSE_ATTRIBUTE(pClassificationGroup, "DowngradeLevel", downgradeLevel, string);
         PARSE_ATTRIBUTE(pClassificationGroup, "CountryCode", countryCode, string);
         PARSE_ATTRIBUTE(pClassificationGroup, "Description", desc, string);
         PARSE_ATTRIBUTE(pClassificationGroup, "Authority", authority, string);
         PARSE_ATTRIBUTE(pClassificationGroup, "AuthorityType", authorityType, string);
         PARSE_ATTRIBUTE(pClassificationGroup, "SecurityControlNumber", secControlNum, string);
         PARSE_ATTRIBUTE(pClassificationGroup, "CopyNumber", copyNumber, string);
         PARSE_ATTRIBUTE(pClassificationGroup, "NumberOfCopies", numberOfCopies, string);
         Hdf5Attribute* pDeclassDateAttr = pClassificationGroup->getAttribute("DeclassificationDate");
         if (!parseError && pDeclassDateAttr != NULL)
         {
            pDeclassDate = FactoryResource<DateTime>(pDeclassDateAttr->readData<DateTime>());
            parseError = (pDeclassDate.get() == NULL);
         }
         else
         {
            parseError = true;
         }
         Hdf5Attribute* pDowngradeDateAttr = pClassificationGroup->getAttribute("DowngradeDate");
         if (!parseError && pDowngradeDateAttr != NULL)
         {
            pDowngradeDate = FactoryResource<DateTime>(pDowngradeDateAttr->readData<DateTime>());
            parseError = (pDowngradeDate.get() == NULL);
         }
         else
         {
            parseError = true;
         }
         Hdf5Attribute* pSourceDateAttr = pClassificationGroup->getAttribute("SourceDate");
         if (!parseError && pSourceDateAttr != NULL)
         {
            pSourceDate = FactoryResource<DateTime>(pSourceDateAttr->readData<DateTime>());
            parseError = (pSourceDate.get() == NULL);
         }
         else
         {
            parseError = true;
         }
         FactoryResource<DynamicObject> pClassMetadata(NULL);
         if (!parseError)
         {
            const Hdf5Dataset* pClassMetadataDs =
               dynamic_cast<const Hdf5Dataset*>(pClassificationGroup->getElement("Metadata"));

            //metadata will only be present if original classification had 1 or more attributes
            if (pClassMetadataDs != NULL)
            {
               pClassMetadata = FactoryResource<DynamicObject>(parseDynamicObject(pClassMetadataDs, "classification"));
               parseError = (pClassMetadata.get() == NULL);
            }
         }
         if (!parseError)
         {
            //only set attributes if we parsed all successfully
            pClassification->setSystem(system);
            pClassification->setCodewords(codewords);
            pClassification->setFileControl(control);
            pClassification->setFileReleasing(releasing);
            pClassification->setClassificationReason(reason);
            pClassification->setDeclassificationDate(pDeclassDate.get());
            pClassification->setDeclassificationType(declassType);
            pClassification->setDeclassificationExemption(declassEx);
            pClassification->setFileDowngrade(downgradeLevel);
            pClassification->setDowngradeDate(pDowngradeDate.get());
            pClassification->setCountryCode(countryCode);
            pClassification->setDescription(desc);
            pClassification->setAuthority(authority);
            pClassification->setAuthorityType(authorityType);
            pClassification->setSecurityControlNumber(secControlNum);
            pClassification->setSecuritySourceDate(pSourceDate.get());
            pClassification->setFileCopyNumber(copyNumber);
            pClassification->setFileNumberOfCopies(numberOfCopies);
            pClassification->adoptiveMerge(pClassMetadata.get());
            string errorMessage;
            if (!pClassification->isValid(errorMessage))
            {
               ICEWARNING(true, "Using invalid classification found in file.");
               ICEWARNING(true, "Classification invalid for following reason: " + errorMessage);
            }
         }
         else
         {
            if (parsedLevel)
            {
               ICEWARNING(true, "Only able to parse classification level found in file. "
                  "All other classification information was not parsed.");
            }
            else
            {
               ICEWARNING(true, "Classification of image detected in file, but improperly formatted. "
                  "Defaulting to classification of the system.");
            }
         }
         pDescriptor->setClassification(pClassification.get());
      }
      else
      {
         ICEWARNING(true, "Classification could not be determined from file. "
            "Defaulting to classification of the system.");
      }
   }
   return true;
}

bool IceReader::parseGcps(const Hdf5Group* pCube, RasterDataDescriptor* pDescriptor)
{
   const Hdf5Dataset* pGcpDs = dynamic_cast<const Hdf5Dataset*>(pCube->getElement("GroundControlPoints"));
   if (pGcpDs != NULL)
   {
      auto_ptr<list<GcpPoint> > pGcps(pGcpDs->readData<list<GcpPoint> >());
      if (pGcps.get() != NULL)
      {
         if (pDescriptor != NULL)
         {
            RasterFileDescriptor* pFileDescriptor =
               dynamic_cast<RasterFileDescriptor*>(pDescriptor->getFileDescriptor());
            if (pFileDescriptor != NULL)
            {
               pFileDescriptor->setGcps(*pGcps);
            }
         }
      }
      else
      {
         ICEWARNING(true, "Ground Control Points detected in file, but could not be parsed properly.");
      }
   }
   return true;
}

bool IceReader::parseUnits(const Hdf5Group* pCube, RasterDataDescriptor* pDescriptor)
{
   if (mIceDescriptor.getSupportedFeature(IceFormatDescriptor::CUBE_UNITS))
   {
      const Hdf5Group* pUnitsGroup = dynamic_cast<const Hdf5Group*>(pCube->getElement("Units"));
      ICEERROR_RET(pUnitsGroup == NULL, "Ice file is improperly formatted, cannot locate unit information.", false);
      FactoryResource<Units> pNewUnits;
      RasterFileDescriptor* pFileDesc = dynamic_cast<RasterFileDescriptor*>(pDescriptor->getFileDescriptor());
      Hdf5Attribute* pAttr = pUnitsGroup->getAttribute("RangeMax");
      bool parsed = false;
      if (pAttr != NULL)
      {
         double rangeMax;
         if (pAttr->getValueAs<double>(rangeMax))
         {
            pNewUnits->setRangeMax(rangeMax);
            parsed = true;
         }
      }
      ICEWARNING(!parsed, "Range Max of the Units could not parsed properly.");

      pAttr = pUnitsGroup->getAttribute("RangeMin");
      parsed = false;
      if (pAttr != NULL)
      {
         double rangeMin;
         if (pAttr->getValueAs<double>(rangeMin))
         {
            pNewUnits->setRangeMin(rangeMin);
            parsed = true;
         }
      }
      ICEWARNING(!parsed, "Range Min of the Units could not parsed properly.");

      pAttr = pUnitsGroup->getAttribute("ScaleFromStandard");
      parsed = false;
      if (pAttr != NULL)
      {
         double scaleFromStandard;
         if (pAttr->getValueAs<double>(scaleFromStandard))
         {
            pNewUnits->setScaleFromStandard(scaleFromStandard);
            parsed = true;
         }
      }
      ICEWARNING(!parsed, "The Scale From Standard of the Units could not parsed properly.");

      pAttr = pUnitsGroup->getAttribute("Name");
      parsed = false;
      if (pAttr != NULL)
      {
         string name;
         if (pAttr->getValueAs<string>(name))
         {
            pNewUnits->setUnitName(name);
            parsed = true;
         }
      }
      ICEWARNING(!parsed, "Name of the Units could not parsed properly.");

      pAttr = pUnitsGroup->getAttribute("Type");
      parsed = false;
      if (pAttr != NULL)
      {
         string typeStr;
         if (pAttr->getValueAs<string>(typeStr))
         {
            bool strParseError = false;
            UnitType type = StringUtilities::fromXmlString<UnitType>(typeStr, &strParseError);
            if (!strParseError)
            {
               pNewUnits->setUnitType(type);
               parsed = true;
            }
         }
      }
      ICEWARNING(!parsed, "Type of the Units could not parsed properly.");

      if (pDescriptor != NULL)
      {
         pDescriptor->setUnits(pNewUnits.get());
      }
      if (pFileDesc != NULL)
      {
         pFileDesc->setUnits(pNewUnits.get());
      }
   }
   return true;
}

bool IceReader::parseDisplayInformation(const Hdf5Group* pCube, RasterDataDescriptor* pDescriptor)
{
   DO_IF(pDescriptor == NULL, return false);
   if (mIceDescriptor.getSupportedFeature(IceFormatDescriptor::CUBE_DISPLAY_INFO))
   {
      const Hdf5Group* pDisplayGroup = dynamic_cast<const Hdf5Group*>(pCube->getElement("DisplayInformation"));
      ICEERROR_RET(pDisplayGroup == NULL,
         "Ice file is improperly formatted; cannot locate display information.", false);
      RasterFileDescriptor* pFileDesc = dynamic_cast<RasterFileDescriptor*>(pDescriptor->getFileDescriptor());
      DO_IF(pFileDesc == NULL, return false);

      Hdf5Attribute* pAttr = pDisplayGroup->getAttribute("GrayDisplayedBand");
      bool parsed = false;
      if (pAttr != NULL)
      {
         unsigned int grayBand;
         if (pAttr->getValueAs<unsigned int>(grayBand))
         {
            DimensionDescriptor bandDim = pFileDesc->getOnDiskBand(grayBand);
            if (bandDim.isValid())
            {
               pDescriptor->setDisplayBand(GRAY, bandDim);
               parsed = true;
            }
         }
      }
      ICEWARNING(!parsed, "Gray displayed band could not be parsed properly.");

      pAttr = pDisplayGroup->getAttribute("RedDisplayedBand");
      parsed = false;
      if (pAttr != NULL)
      {
         unsigned int redBand;
         if (pAttr->getValueAs<unsigned int>(redBand))
         {
            DimensionDescriptor bandDim = pFileDesc->getOnDiskBand(redBand);
            if (bandDim.isValid())
            {
               pDescriptor->setDisplayBand(RED, bandDim);
               parsed = true;
            }
         }
      }
      ICEWARNING(!parsed, "Red displayed band could not be parsed properly.");

      pAttr = pDisplayGroup->getAttribute("GreenDisplayedBand");
      parsed = false;
      if (pAttr != NULL)
      {
         unsigned int greenBand;
         if (pAttr->getValueAs<unsigned int>(greenBand))
         {
            DimensionDescriptor bandDim = pFileDesc->getOnDiskBand(greenBand);
            if (bandDim.isValid())
            {
               pDescriptor->setDisplayBand(GREEN, bandDim);
               parsed = true;
            }
         }
      }
      ICEWARNING(!parsed, "Green displayed band could not be parsed properly.");

      pAttr = pDisplayGroup->getAttribute("BlueDisplayedBand");
      parsed = false;
      if (pAttr != NULL)
      {
         unsigned int blueBand;
         if (pAttr->getValueAs<unsigned int>(blueBand))
         {
            DimensionDescriptor bandDim = pFileDesc->getOnDiskBand(blueBand);
            if (bandDim.isValid())
            {
               pDescriptor->setDisplayBand(BLUE, bandDim);
               parsed = true;
            }
         }
      }
      ICEWARNING(!parsed, "Blue displayed band could not be parsed properly.");

      pAttr = pDisplayGroup->getAttribute("DisplayMode");
      parsed = false;
      if (pAttr != NULL)
      {
         string displayModeStr;
         if (pAttr->getValueAs<string>(displayModeStr))
         {
            bool strParseError = false;
            DisplayMode displayMode = StringUtilities::fromXmlString<DisplayMode>(displayModeStr, &strParseError);
            if (!strParseError)
            {
               if (pDescriptor != NULL)
               {
                  pDescriptor->setDisplayMode(displayMode);
               }
               parsed = true;
            }
         }
      }
      ICEWARNING(!parsed, "Display mode could not be parsed properly.");

      pAttr = pDisplayGroup->getAttribute("XPixelSize");
      parsed = false;
      if (pAttr != NULL)
      {
         double xPixelSize;
         if (pAttr->getValueAs<double>(xPixelSize))
         {
            pDescriptor->setXPixelSize(xPixelSize);
            pFileDesc->setXPixelSize(xPixelSize);
            parsed = true;
         }
      }
      ICEWARNING(!parsed, "The X pixel size could not be parsed properly.");

      pAttr = pDisplayGroup->getAttribute("YPixelSize");
      parsed = false;
      if (pAttr != NULL)
      {
         double yPixelSize;
         if (pAttr->getValueAs<double>(yPixelSize))
         {
            pDescriptor->setYPixelSize(yPixelSize);
            pFileDesc->setYPixelSize(yPixelSize);
            parsed = true;
         }
      }
      ICEWARNING(!parsed, "The Y pixel size could not be parsed properly.");

   }
   return true;
}

bool IceReader::loadCubeStatistics(RasterElement* pElement)
{
   if (mIceDescriptor.getSupportedFeature(IceFormatDescriptor::BAND_STATISTICS))
   {
      hid_t fileHandle = mIceFile.getFileHandle();
      DO_IF(fileHandle < 0, return false);

      DO_IF(pElement == NULL, return false);
      RasterDataDescriptor* pDataDesc = dynamic_cast<RasterDataDescriptor*>(pElement->getDataDescriptor());
      DO_IF(pDataDesc == NULL, return false);
      RasterFileDescriptor* pFileDesc = dynamic_cast<RasterFileDescriptor*>(pDataDesc->getFileDescriptor());
      DO_IF(pFileDesc == NULL, return false);

      // Only load statistics if the user hasn't overridden the badValues in the import options dialog.
      if (pDataDesc->getBadValues() != NULL && pDataDesc->getBadValues()->empty())
      {
         // the statistics metadata format changed in ICE version 1.30.
         bool preVersion130 = 
            (mIceDescriptor.getSupportedFeature(IceFormatDescriptor::BAND_STATISTICS_FLOATING_PT_BAD_VALUES) == false);

         // Look for Metadata. If it's present, parse it.
         { // Resource scoping
            Hdf5DataSetResource statMetadataDs;
            {  //Turn off error handling while we check for stats, since it may not exist
               Hdf5ErrorHandlerResource errHandler(NULL, NULL);
               statMetadataDs = Hdf5DataSetResource(fileHandle, "/Datasets/Cube1/BandStatistics/Metadata");
            }

            DO_IF(*statMetadataDs < 0, return false);
            Hdf5DataSpaceResource dataSpace(H5Dget_space(*statMetadataDs));
            hsize_t sizeArray[H5S_MAX_RANK];
            int numdimensions = H5Sget_simple_extent_dims(*dataSpace, sizeArray, NULL);
            DO_IF(numdimensions != 1, return false);
            Hdf5IncrementalReader statReader(*statMetadataDs);
            const vector<DimensionDescriptor>& bands = pFileDesc->getBands();
            DO_IF(sizeArray[0] != bands.size(), return false);
            hsize_t oneValue = 1;
            hsize_t currentRow;
            vector<DimensionDescriptor>::const_iterator iter;
            for (currentRow = 0, iter = bands.begin();
                  iter != bands.end(); ++currentRow, ++iter)
            {
               if (!iter->isActiveNumberValid())
               {
                  // This dimension descriptor doesn't have any active number, so it was subcubed out during import.
                  continue;
               }
               statReader.selectHyperslab(H5S_SELECT_SET, &currentRow, &oneValue, &oneValue, NULL);
               if (preVersion130)
               {
                  auto_ptr<StatisticsMetadata> pValues(
                     statReader.readSelectedData<StatisticsMetadata>());
                  Statistics* pCubeStat = pElement->getStatistics(*iter);

                  //set resolution and bad values first because they clear any other
                  //set statistics.
                  if (pCubeStat != NULL)
                  {
                     pCubeStat->setStatisticsResolution(pValues->mStatResolution);
                     StatisticsMetadata::BadValueType* pBadValues =
                        reinterpret_cast<StatisticsMetadata::BadValueType *>(pValues->mBadValues.p);
                     vector<StatisticsMetadata::BadValueType> badValues;
                     badValues.reserve(pValues->mBadValues.len);
                     for (size_t i = 0; i < pValues->mBadValues.len; ++i)
                     {
                        badValues.push_back(pBadValues[i]);
                     }

                     pCubeStat->setBadValues(badValues);
                  }
               }
               else
               {
                  auto_ptr<StatisticsMetadataFloat> pValues(statReader.readSelectedData<StatisticsMetadataFloat>());
                  Statistics* pCubeStat = pElement->getStatistics(*iter);

                  //set resolution and bad values first because they clear any other
                  //set statistics.
                  if (pCubeStat != NULL)
                  {
                     pCubeStat->setStatisticsResolution(pValues->mStatResolution);
                     StatisticsMetadataFloat::BadValueType* pBadValues =
                        reinterpret_cast<StatisticsMetadataFloat::BadValueType *>(pValues->mBadValues.p);
                     std::string badValuesStr;
                     badValuesStr.reserve(pValues->mBadValues.len);
                     for (size_t i = 0; i < pValues->mBadValues.len; ++i)
                     {
                        badValuesStr += pBadValues[i];
                     }
                     FactoryResource<BadValues> pImportedBadValues;
                     if (pImportedBadValues->setBadValues(badValuesStr) == false)
                     {
                        std::string warningMsg = "Unable to import bad values criteria for band " +
                           StringUtilities::toDisplayString<unsigned int>(iter->getOriginalNumber() + 1) +
                           ":\n   " + pImportedBadValues->getLastErrorMsg();
                        mWarnings.push_back(warningMsg);
                     }
                     pCubeStat->setBadValues(pImportedBadValues.get());
                  }
               }
            }
         }

         // Look for StoredStatistics. If it's present, parse it.
         { // Resource scoping
            Hdf5DataSetResource statisticsDs;
            {  //Turn off error handling while we check for stats, since it may not exist
               Hdf5ErrorHandlerResource errHandler(NULL, NULL); 
               statisticsDs = Hdf5DataSetResource(fileHandle, "/Datasets/Cube1/BandStatistics/StoredStatistics");
            }

            // If the optional statistics field isn't present, return true.
            DO_IF(*statisticsDs < 0, return true);
            Hdf5DataSpaceResource dataSpace(H5Dget_space(*statisticsDs));
            hsize_t sizeArray[H5S_MAX_RANK];
            int numdimensions = H5Sget_simple_extent_dims(*dataSpace, sizeArray, NULL);
            DO_IF(numdimensions != 1, return false);
            //read the statistics
            Hdf5IncrementalReader statReader(*statisticsDs);
            hsize_t oneValue = 1;
            for (hsize_t currentRow = 0; currentRow < sizeArray[0]; ++currentRow)
            {
               statReader.selectHyperslab(H5S_SELECT_SET, &currentRow, &oneValue, &oneValue, NULL);
               auto_ptr<StatisticsValues> pValues(statReader.readSelectedData<StatisticsValues>());
               DO_IF(pValues.get() == NULL, return false);
               DimensionDescriptor loadedBand = pDataDesc->getOnDiskBand(pValues->mOnDiskBandNumber);
               DO_IF(!loadedBand.isValid(), return false);
               Statistics* pCubeStat = pElement->getStatistics(loadedBand);
               DO_IF(pCubeStat == NULL, return false);

               StatisticsValues::BinCenterType* pBinCenters =
                  reinterpret_cast<StatisticsValues::BinCenterType *>(pValues->mpBinCenters.p);
               StatisticsValues::HistogramType* pHistogramCounts =
                  reinterpret_cast<StatisticsValues::HistogramType *>(pValues->mpHistogramCounts.p);
               StatisticsValues::PercentileType* pPercentiles =
                  reinterpret_cast<StatisticsValues::PercentileType*>(pValues->mpPercentiles.p);
               if (pValues->mpBinCenters.len != 256 || pValues->mpHistogramCounts.len != 256 ||
                  pValues->mpPercentiles.len != 1001)
               {
                  //the data read from the file isn't correct, so don't continue reading
                  //statistics out.
                  return false;
               }
               //This code is validating the stored statistics because of a bug that affects version 1 and version 1.1 
               //Ice files. When these versions are no longer supported by the importer this check can come out.
               if (currentRow == 0)
               {
                  const unsigned int* pCubeHistogramCounts = NULL;
                  const double* pCubeBinCenters = NULL;

                  pCubeStat->getHistogram(pCubeBinCenters, pCubeHistogramCounts);
                  if (pHistogramCounts != NULL && pCubeHistogramCounts != NULL)
                  {
                     for (unsigned int i = 0; i < 256; ++i)
                     {
                        if (pCubeHistogramCounts[i] != pHistogramCounts[i])
                        {
                           //the data read from the file isn't correct, so don't continue reading
                           //statistics out.
                           return false;
                        }
                     }
                  }
               }
               bool areStatsCalculated = pCubeStat->areStatisticsCalculated();
               pCubeStat->setAverage(pValues->mAverage);
               pCubeStat->setMin(pValues->mMin);
               pCubeStat->setMax(pValues->mMax);
               pCubeStat->setStandardDeviation(pValues->mStandardDeviation);
               pCubeStat->setHistogram(pBinCenters, pHistogramCounts);
               pCubeStat->setPercentiles(pPercentiles);
               areStatsCalculated = pCubeStat->areStatisticsCalculated();
            }
         }
      }
   }

   return true;
}

bool IceReader::createLayer(const string& hdfPath, SpatialDataView* pView,
   DataElement* pParent, string& warningMessage)
{
   Hdf5Group* pRoot = mIceFile.getRootGroup();
   DO_IF(pRoot == NULL, return NULL);

   // Read common Layer attributes
   const string layerPath = hdfPath + "/Layer";
   const Hdf5Group* pLayerGroup = dynamic_cast<const Hdf5Group*>(pRoot->getElementByPath(layerPath));
   Layer* pLayer = parseAndCreateLayer(pLayerGroup, pView, pParent, warningMessage);
   DO_IF(pLayer == NULL, return false);

   // Read type-specific attributes
   const LayerType layerType = pLayer->getLayerType();
   switch (layerType)
   {
      case PSEUDOCOLOR:
      {
         const string pseudocolorLayerPath = hdfPath + "/PseudocolorLayer";
         const Hdf5Group* pPseudocolorLayerGroup = dynamic_cast<const Hdf5Group*>
            (pRoot->getElementByPath(pseudocolorLayerPath));
         PseudocolorLayer* pPseudocolorLayer = static_cast<PseudocolorLayer*>(pLayer);
         DO_IF(parsePseudocolorLayer(pPseudocolorLayerGroup,
            pPseudocolorLayer, warningMessage) == false, return false);
         break;
      }

      case THRESHOLD:
      {
         const string thresholdLayerPath = hdfPath + "/ThresholdLayerProperties";
         const Hdf5Group* pThresholdLayerGroup = dynamic_cast<const Hdf5Group*>
            (pRoot->getElementByPath(thresholdLayerPath));
         ThresholdLayer* pThresholdLayer = static_cast<ThresholdLayer*>(pLayer);
         DO_IF(parseThresholdLayer(pThresholdLayerGroup, pThresholdLayer, warningMessage) == false, return false);
         break;
      }

      default:
      {
         return false;
      }
   }

   return true;
}

Layer* IceReader::parseAndCreateLayer(const Hdf5Group* pLayerGroup,
   SpatialDataView* pView, DataElement* pParent, string& warningMessage)
{
   DO_IF(pView == NULL, return NULL);
   DO_IF(pLayerGroup == NULL, return NULL);

   bool parseError = false;
   string layerName;
   PARSE_ATTRIBUTE(pLayerGroup, "Name", layerName, string);
   DO_IF(parseError == true, return NULL);

   string layerTypeString;
   PARSE_ATTRIBUTE(pLayerGroup, "LayerType", layerTypeString, string);
   DO_IF(parseError == true, return NULL);

   LayerType layerType = StringUtilities::fromXmlString<LayerType>(layerTypeString);
   DO_IF(layerType.isValid() == false, return NULL);

   double xScaleFactor = 1.0;
   PARSE_ATTRIBUTE_DEFAULT(pLayerGroup, "XScaleFactor", xScaleFactor, double, 1.0);

   double yScaleFactor = 1.0;
   PARSE_ATTRIBUTE_DEFAULT(pLayerGroup, "YScaleFactor", yScaleFactor, double, 1.0);

   double xOffset = 0.0;
   PARSE_ATTRIBUTE_DEFAULT(pLayerGroup, "XOffset", xOffset, double, 0.0);

   double yOffset = 0.0;
   PARSE_ATTRIBUTE_DEFAULT(pLayerGroup, "YOffset", yOffset, double, 0.0);

   Layer* pLayer = pView->createLayer(layerType, pParent, layerName);
   DO_IF(pLayer == NULL, return NULL);

   pLayer->setXScaleFactor(xScaleFactor);
   pLayer->setYScaleFactor(yScaleFactor);
   pLayer->setXOffset(xOffset);
   pLayer->setYOffset(yOffset);

   return pLayer;
}

bool IceReader::parsePseudocolorLayer(const Hdf5Group* pPseudocolorLayerGroup,
   PseudocolorLayer* pPseudocolorLayer, string& warningMessage)
{
   DO_IF(pPseudocolorLayerGroup == NULL, return false);
   DO_IF(pPseudocolorLayer == NULL, return false);

   // Read Symbol
   bool parseError = false;
   string symbolTypeString;
   PARSE_ATTRIBUTE_DEFAULT(pPseudocolorLayerGroup, "Symbol", symbolTypeString, string, "Solid");

   SymbolType symbolType = StringUtilities::fromXmlString<SymbolType>(symbolTypeString);
   DO_IF(symbolType.isValid() == false, symbolType = SOLID);
   pPseudocolorLayer->setSymbol(symbolType);

   // Read classes
   const Hdf5Dataset* pClassNamesDataset = dynamic_cast<const Hdf5Dataset*>
      (pPseudocolorLayerGroup->getElement("ClassNames"));

   const Hdf5Dataset* pClassValuesDataset = dynamic_cast<const Hdf5Dataset*>
      (pPseudocolorLayerGroup->getElement("ClassValues"));

   const Hdf5Dataset* pClassIsDisplayedDataset = dynamic_cast<const Hdf5Dataset*>
      (pPseudocolorLayerGroup->getElement("ClassIsDisplayed"));

   const Hdf5Dataset* pClassColorsDataset = dynamic_cast<const Hdf5Dataset*>
      (pPseudocolorLayerGroup->getElement("ClassColors"));

   // Verify that all classes exist.
   if (pClassNamesDataset == NULL || pClassValuesDataset == NULL ||
      pClassIsDisplayedDataset == NULL || pClassColorsDataset == NULL)
   {
      warningMessage += "Pseudocolor Layer class datasets are not present in the file. No classes will be loaded.";
      return true;
   }

   // Read ClassNames
   auto_ptr<vector<string> > pClassNames(pClassNamesDataset->readData<vector<string> >());

   // Read ClassValues
   auto_ptr<vector<int> > pClassValues(pClassValuesDataset->readData<vector<int> >());

   // Read ClassIsDisplayed
   auto_ptr<vector<unsigned char> > pClassIsDisplayed(pClassIsDisplayedDataset->readData<vector<unsigned char> >());

   // Read ClassColors
   auto_ptr<vector<string> > pClassColors(pClassColorsDataset->readData<vector<string> >());

   // Verify that all classes exist and are the same size
   if (pClassNames.get() == NULL || pClassValues.get() == NULL ||
      pClassIsDisplayed.get() == NULL || pClassColors.get() == NULL)
   {
      warningMessage += "Pseudocolor Layer classes are not present in the file. No classes will be loaded.";
      return true;
   }

   const unsigned int numClasses = pClassNames->size();
   if (numClasses != pClassValues->size() || numClasses != pClassIsDisplayed->size() ||
      numClasses != pClassColors->size())
   {
      warningMessage += "Pseudocolor Layer classes detected in file, but are improperly formatted.";
      return true;
   }

   // Add all classes to pPseudocolorLayer
   for (unsigned int i = 0; i < numClasses; ++i)
   {
      const string& className = (*pClassNames)[i];
      const int& classValue = (*pClassValues)[i];
      const bool& classIsDisplayed = (*pClassIsDisplayed)[i];
      bool classColorError;
      const ColorType& classColor = StringUtilities::fromXmlString<ColorType>((*pClassColors)[i], &classColorError);
      DO_IF(classColorError == true,
         warningMessage += ("Unable to load ClassColor for class " + className + ". An invalid color will be set.\n"));

      DO_IF(pPseudocolorLayer->addInitializedClass(className, classValue, classColor, classIsDisplayed) < 0,
         warningMessage += ("Unable to add class " + className + ". Other classes will still be added.\n"));
   }

   return true;
}

bool IceReader::parseThresholdLayer(const Hdf5Group* pThresholdLayerGroup, ThresholdLayer* pThresholdLayer,
                                    string& warningMessage)
{
   DO_IF(pThresholdLayerGroup == NULL, return false);
   DO_IF(pThresholdLayer == NULL, return false);

   bool parseError = false;

   // Symbol
   string symbolTypeString;
   PARSE_ATTRIBUTE_DEFAULT(pThresholdLayerGroup, "Symbol", symbolTypeString, string, string());

   SymbolType symbol = StringUtilities::fromXmlString<SymbolType>(symbolTypeString);
   DO_IF(symbol.isValid() == false, symbol = ThresholdLayer::getSettingMarkerSymbol());
   pThresholdLayer->setSymbol(symbol);

   // Color
   string colorTypeString;
   PARSE_ATTRIBUTE_DEFAULT(pThresholdLayerGroup, "Color", colorTypeString, string, string());

   ColorType color = StringUtilities::fromXmlString<ColorType>(colorTypeString);
   DO_IF(color.isValid() == false, color = ThresholdLayer::getSettingMarkerColor());
   pThresholdLayer->setColor(color);

   // Pass area
   string passAreaString;
   PARSE_ATTRIBUTE_DEFAULT(pThresholdLayerGroup, "PassArea", passAreaString, string, string());

   PassArea passArea = StringUtilities::fromXmlString<PassArea>(passAreaString);
   DO_IF(passArea.isValid() == false, passArea = ThresholdLayer::getSettingPassArea());
   pThresholdLayer->setPassArea(passArea);

   // Region units
   string unitsString;
   PARSE_ATTRIBUTE_DEFAULT(pThresholdLayerGroup, "RegionUnits", unitsString, string, string());

   RegionUnits units = StringUtilities::fromXmlString<RegionUnits>(unitsString);
   DO_IF(units.isValid() == false, units = ThresholdLayer::getSettingRegionUnits());
   pThresholdLayer->setRegionUnits(units);

   // First threshold
   double firstThreshold = 0.0;
   PARSE_ATTRIBUTE_DEFAULT(pThresholdLayerGroup, "FirstThreshold", firstThreshold, double, 0.0);
   pThresholdLayer->setFirstThreshold(firstThreshold);

   // Second threshold
   double secondThreshold = 0.0;
   PARSE_ATTRIBUTE_DEFAULT(pThresholdLayerGroup, "SecondThreshold", secondThreshold, double, 0.0);
   pThresholdLayer->setSecondThreshold(secondThreshold);

   // displayed band
   unsigned int bandNumber = 0;
   PARSE_ATTRIBUTE_DEFAULT(pThresholdLayerGroup, "DisplayBandNumber", bandNumber, unsigned int, 0);
   const RasterElement* pElement = dynamic_cast<const RasterElement*>(pThresholdLayer->getDataElement());
   if (pElement != NULL)
   {
      const RasterDataDescriptor* pDesc = dynamic_cast<const RasterDataDescriptor*>(pElement->getDataDescriptor());
      if (pDesc != NULL)
      {
         pThresholdLayer->setDisplayedBand(pDesc->getActiveBand(bandNumber));
      }
   }

   return true;
}
