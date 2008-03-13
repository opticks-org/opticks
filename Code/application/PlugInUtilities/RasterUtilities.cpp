/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "DataVariant.h"
#include "DimensionDescriptor.h"
#include "DynamicObject.h"
#include "Endian.h"
#include "ObjectResource.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterFileDescriptor.h"
#include "RasterUtilities.h"
#include "SpecialMetadata.h"
#include "switchOnEncoding.h"

#include <algorithm>
#include <boost/bind.hpp>
#include <sstream>

using namespace std;

namespace
{
   const double BlueCenter = 0.45;
   const double BlueTolerance = 0.04;
   const double GreenCenter = 0.55;
   const double GreenTolerance = 0.04;
   const double RedCenter = 0.65;
   const double RedTolerance = 0.04;
}

vector<DimensionDescriptor> RasterUtilities::generateDimensionVector(unsigned int count,
   bool setOriginalNumbers, bool setActiveNumbers, bool setOnDiskNumbers)
{
   vector<DimensionDescriptor> retval(count);
   for (size_t i = 0; i < count; ++i)
   {
      if (setOriginalNumbers)
      {
         retval[i].setOriginalNumber(i);
      }
      if (setActiveNumbers)
      {
         retval[i].setActiveNumber(i);
      }
      if (setOnDiskNumbers)
      {
         retval[i].setOnDiskNumber(i);
      }
   }
   return retval;
}

bool RasterUtilities::determineSkipFactor(const std::vector<DimensionDescriptor>& values, unsigned int& skipFactor)
{
   unsigned int calcSkipFactor = 1;
   for (vector<DimensionDescriptor>::size_type count = 0; count != values.size(); ++count)
   {
      if (!values[count].isOnDiskNumberValid())
      {
         return false;
      }
      if (count > 0)
      {
         //determine skip factor on second iteration
         int curSkipFactor = values[count].getOnDiskNumber() - values[count-1].getOnDiskNumber();
         if (curSkipFactor < 1)
         {
            return false;
         }
         if (count > 1)
         {
            //on any iteration after second, verify skip factor remains the same
            if (curSkipFactor != calcSkipFactor)
            {
               return false;
            }
         }
         calcSkipFactor = curSkipFactor;
      }
   }
   skipFactor = calcSkipFactor - 1;
   return true;
}

bool RasterUtilities::determineExportSkipFactor(const std::vector<DimensionDescriptor>&values, unsigned int& skipFactor)
{
   unsigned int calcSkipFactor = 1;
   for (vector<DimensionDescriptor>::size_type count = 0; count != values.size(); ++count)
   {
      if (!values[count].isActiveNumberValid())
      {
         return false;
      }
      if (count > 0)
      {
         //determine skip factor on second iteration
         int curSkipFactor = values[count].getActiveNumber() - values[count-1].getActiveNumber();
         if (curSkipFactor < 1)
         {
            return false;
         }
         if (count > 1)
         {
            //on any iteration after second, verify skip factor remains the same
            if (curSkipFactor != calcSkipFactor)
            {
               return false;
            }
         }
         calcSkipFactor = curSkipFactor;
      }
   }
   skipFactor = calcSkipFactor - 1;
   return true;
}

std::vector<DimensionDescriptor> RasterUtilities::subsetDimensionVector(
   const vector<DimensionDescriptor>& origValues,
   const DimensionDescriptor& start,
   const DimensionDescriptor& stop,
   unsigned int skipFactor)
{
   vector<DimensionDescriptor> newValues = origValues;
   unsigned int theStartValue = 0;
   if (start.isOriginalNumberValid())
   {
      theStartValue = start.getOriginalNumber();
   }
   else if (origValues.empty() == false)
   {
      theStartValue = origValues.front().getOriginalNumber();
   }

   unsigned int theEndValue = 0;
   if (stop.isOriginalNumberValid())
   {
      theEndValue = stop.getOriginalNumber();
   }
   else if (origValues.empty() == false)
   {
      theEndValue = origValues.back().getOriginalNumber();
   }

   newValues.clear();
   for (unsigned int i = 0; i < origValues.size(); ++i)
   {
      DimensionDescriptor dim = origValues[i];
      unsigned int originalNumber = dim.getOriginalNumber();
      if ((originalNumber >= theStartValue) && (originalNumber <= theEndValue))
      {
         newValues.push_back(dim);
         i += skipFactor;
      }
   }

   return newValues;
}


namespace
{
   class SetOnDiskNumber
   {
   public:
      SetOnDiskNumber() : mCurrent(0)
      {
      }

      DimensionDescriptor operator()(const DimensionDescriptor& dim)
      {
         DimensionDescriptor temp = dim;
         temp.setOnDiskNumber(mCurrent++);
         return temp;
      }

   private:
      unsigned int mCurrent;
   };
}

namespace
{
   void setFileDescriptor(DataDescriptor *pDd, FileDescriptor* pFd, 
      const std::string &filename, const std::string &datasetLocation, EndianType endian)
   {
      // Populate the file descriptor
      pFd->setFilename(filename);
      pFd->setDatasetLocation(datasetLocation);
      pFd->setEndian(endian);

      RasterDataDescriptor *pRasterDd = dynamic_cast<RasterDataDescriptor*>(pDd);

      if (pRasterDd != NULL)
      {
         RasterFileDescriptor *pRasterFd = 
            dynamic_cast<RasterFileDescriptor*>(pFd);
         VERIFYNRV(pRasterFd != NULL);

         unsigned int bitsPerElement = RasterUtilities::bytesInEncoding(pRasterDd->getDataType()) * 8;
         pRasterFd->setBitsPerElement(bitsPerElement);

         vector<DimensionDescriptor> cols = pRasterDd->getColumns();
         std::transform(cols.begin(), cols.end(), cols.begin(), SetOnDiskNumber());
         pRasterFd->setColumns(cols);
         pRasterDd->setColumns(cols);
         
         vector<DimensionDescriptor> rows = pRasterDd->getRows();
         std::transform(rows.begin(), rows.end(), rows.begin(), SetOnDiskNumber());
         pRasterFd->setRows(rows);
         pRasterDd->setRows(rows);

         vector<DimensionDescriptor> bands = pRasterDd->getBands();
         std::transform(bands.begin(), bands.end(), bands.begin(), SetOnDiskNumber());
         pRasterFd->setBands(bands);
         pRasterDd->setBands(bands);

         pRasterFd->setUnits(pRasterDd->getUnits());
         pRasterFd->setXPixelSize(pRasterDd->getXPixelSize());
         pRasterFd->setYPixelSize(pRasterDd->getYPixelSize());

         pRasterFd->setInterleaveFormat(pRasterDd->getInterleaveFormat());         
      }
   }

   template<typename T>
   inline size_t sizeofType(const T *pType)
   {
      return sizeof(T);
   }

   string getBandName(const vector<string>* pBandNames, const string* pBandPrefix, const DimensionDescriptor& band)
   {
      if (!band.isActiveNumberValid() || !band.isOriginalNumberValid())
      {
         return "";
      }
      unsigned int activeNumber = band.getActiveNumber();
      if (pBandNames != NULL)
      {
         if ((activeNumber >= 0) && (activeNumber < pBandNames->size()))
         {
            return pBandNames->at(activeNumber);
         }
         else
         {
            return "";
         }
      }
      ostringstream formatter;
      string bandPrefix = "Band";
      if (pBandPrefix != NULL)
      {
         bandPrefix = *pBandPrefix;
      }
      unsigned int originalNumber = band.getOriginalNumber() + 1;
      formatter << bandPrefix << " " << originalNumber;
      return formatter.str();
   }
};

FileDescriptor *RasterUtilities::generateFileDescriptor(DataDescriptor *pDd, 
   const std::string &filename, const std::string &datasetLocation, EndianType endian)
{
   const RasterDataDescriptor *pRasterDd = dynamic_cast<const RasterDataDescriptor*>(pDd);

   // Create the file descriptor
   FileDescriptor *pFd = NULL;
   if (pRasterDd != NULL)
   {
      FactoryResource<RasterFileDescriptor> pRasterFd;
      pFd = pRasterFd.release();
   }
   else
   {
      FactoryResource<FileDescriptor> pStdFd;
      pFd = pStdFd.release();
   }

   VERIFYRV(pFd != NULL, NULL);

   setFileDescriptor(pDd, pFd, filename, datasetLocation, endian);

   return pFd;
}

FileDescriptor *RasterUtilities::generateAndSetFileDescriptor(DataDescriptor *pDd, 
   const std::string &filename, const std::string &datasetLocation, EndianType endian)
{
   VERIFYRV(pDd != NULL, NULL);

   const RasterDataDescriptor *pRasterDd = dynamic_cast<const RasterDataDescriptor*>(pDd);

   // Create the file descriptor
   if (pRasterDd != NULL)
   {
      FactoryResource<RasterFileDescriptor> pFd;
      pDd->setFileDescriptor(pFd.get());
   }
   else
   {
      FactoryResource<FileDescriptor> pFd;
      pDd->setFileDescriptor(pFd.get());
   }

   FileDescriptor* pCopy = pDd->getFileDescriptor();
   VERIFYRV(pCopy != NULL, NULL);
   setFileDescriptor(pDd, pCopy, filename, datasetLocation, endian);

   return pCopy;
}

FileDescriptor* RasterUtilities::generateFileDescriptorForExport(const DataDescriptor* pDd,
   const std::string &filename)
{
   DimensionDescriptor emptyDesc;
   vector<DimensionDescriptor> emptyVector;
   return generateFileDescriptorForExport(pDd, filename, emptyDesc, emptyDesc, 0,
      emptyDesc, emptyDesc, 0, emptyVector);
}

FileDescriptor* RasterUtilities::generateFileDescriptorForExport(const DataDescriptor* pDd,
   const std::string &filename, const DimensionDescriptor& startRow,
   const DimensionDescriptor& stopRow,
   unsigned int rowSkipFactor,
   const DimensionDescriptor& startCol,
   const DimensionDescriptor& stopCol,
   unsigned int colSkipFactor,
   const vector<DimensionDescriptor>& subsetBands)
{
   const RasterDataDescriptor *pRasterDd = dynamic_cast<const RasterDataDescriptor*>(pDd);

   // Create the file descriptor
   FileDescriptor *pFd = NULL;
   if (pRasterDd != NULL)
   {
      FactoryResource<RasterFileDescriptor> pRasterFd;
      pFd = pRasterFd.release();
   }
   else
   {
      FactoryResource<FileDescriptor> pStdFd;
      pFd = pStdFd.release();
   }

   VERIFYRV(pFd != NULL, NULL);

   pFd->setFilename(filename);
   EndianType endian = Endian::getSystemEndian();
   pFd->setEndian(endian);

   /*
#convert CreateExportFileDescriptor wizard to use this code
   */

   if (pRasterDd != NULL)
   {
      RasterFileDescriptor *pRasterFd = 
         dynamic_cast<RasterFileDescriptor*>(pFd);
      VERIFY(pRasterFd != NULL);

      unsigned int bitsPerElement = RasterUtilities::bytesInEncoding(pRasterDd->getDataType()) * 8;
      pRasterFd->setBitsPerElement(bitsPerElement);

      // Rows
      vector<DimensionDescriptor> rows = subsetDimensionVector(pRasterDd->getRows(), startRow, stopRow, rowSkipFactor);
      std::transform(rows.begin(), rows.end(), rows.begin(), SetOnDiskNumber());
      pRasterFd->setRows(rows);

      // Columns
      vector<DimensionDescriptor> columns = subsetDimensionVector(pRasterDd->getColumns(), startCol, stopCol, colSkipFactor);
      std::transform(columns.begin(), columns.end(), columns.begin(), SetOnDiskNumber());
      pRasterFd->setColumns(columns);

      pRasterFd->setUnits(pRasterDd->getUnits());
      pRasterFd->setXPixelSize(pRasterDd->getXPixelSize());
      pRasterFd->setYPixelSize(pRasterDd->getYPixelSize());

      // Bands
      vector<DimensionDescriptor> bands = pRasterDd->getBands();
      const vector<DimensionDescriptor>& origBands = pRasterDd->getBands();
      vector<DimensionDescriptor>::const_iterator foundBand;
      if (!subsetBands.empty())
      {
         bands.clear();
         for (unsigned int count = 0; count < subsetBands.size(); count++)
         {
            foundBand = find(origBands.begin(), origBands.end(), subsetBands[count]);
            if (foundBand != origBands.end())
            {
               DimensionDescriptor bandDim = subsetBands[count];
               bandDim.setOnDiskNumber(count);
               bands.push_back(bandDim);
            }
         }
      }
      else
      {
         std::transform(bands.begin(), bands.end(), bands.begin(), SetOnDiskNumber());
      }
      pRasterFd->setBands(bands);
      pRasterFd->setInterleaveFormat(pRasterDd->getInterleaveFormat());         
   }

   return pFd;
}

FileDescriptor* RasterUtilities::generateFileDescriptorForExport(const DataDescriptor* pDd,
   const std::string &filename, const DimensionDescriptor& startRow,
   const DimensionDescriptor& stopRow,
   unsigned int rowSkipFactor,
   const DimensionDescriptor& startCol,
   const DimensionDescriptor& stopCol,
   unsigned int colSkipFactor,
   const DimensionDescriptor& startBand,
   const DimensionDescriptor& stopBand,
   unsigned int bandSkipFactor)
{
   const RasterDataDescriptor* pRasterDd = dynamic_cast<const RasterDataDescriptor*>(pDd);
   if (pRasterDd != NULL)
   {
      // Bands
      vector<DimensionDescriptor> bands = subsetDimensionVector(pRasterDd->getBands(), startBand, stopBand, bandSkipFactor);
      return generateFileDescriptorForExport(pDd, filename, startRow, stopRow,
         rowSkipFactor, startCol, stopCol, colSkipFactor, bands);
   }
   return NULL;
}


size_t RasterUtilities::bytesInEncoding(EncodingType encoding)
{
   switchOnComplexEncoding(encoding, return ::sizeofType, NULL);

   return 0;
}

RasterDataDescriptor *RasterUtilities::generateRasterDataDescriptor(
   const std::string &name, DataElement *pParent, unsigned int rows, 
   unsigned int cols, EncodingType encoding, ProcessingLocation location)
{
   return generateRasterDataDescriptor(name, pParent, rows, cols, 1, BIP, encoding, location);
}

RasterDataDescriptor *RasterUtilities::generateRasterDataDescriptor(
   const std::string &name, DataElement *pParent, unsigned int rows, 
   unsigned int cols, unsigned int bands, InterleaveFormatType interleave,
   EncodingType encoding, ProcessingLocation location)
{
   Service<ModelServices> pModel;
   RasterDataDescriptor *pDd = dynamic_cast<RasterDataDescriptor*>(
      pModel->createDataDescriptor(name, "RasterElement", pParent));
   if (pDd == NULL)
   {
      return NULL;
   }

   vector<DimensionDescriptor> rowDims = generateDimensionVector(rows);
   pDd->setRows(rowDims);
   vector<DimensionDescriptor> colDims = generateDimensionVector(cols);
   pDd->setColumns(colDims);
   vector<DimensionDescriptor> bandDims = generateDimensionVector(bands);
   pDd->setBands(bandDims);

   pDd->setInterleaveFormat(interleave);
   pDd->setDataType(encoding);
   pDd->setProcessingLocation(location);

   return pDd;
}

void RasterUtilities::subsetDataDescriptor(DataDescriptor* pDd,
   const DimensionDescriptor& startRow,
   const DimensionDescriptor& stopRow,
   unsigned int rowSkipFactor,
   const DimensionDescriptor& startCol,
   const DimensionDescriptor& stopCol,
   unsigned int colSkipFactor,
   const std::vector<DimensionDescriptor>& subsetBands)
{
   RasterDataDescriptor* pRasterDd = dynamic_cast<RasterDataDescriptor*>(pDd);
   if (pRasterDd != NULL)
   {
      // Rows
      vector<DimensionDescriptor> rows = subsetDimensionVector(pRasterDd->getRows(), startRow, stopRow, rowSkipFactor);
      pRasterDd->setRows(rows);

      // Columns
      vector<DimensionDescriptor> columns = subsetDimensionVector(pRasterDd->getColumns(), startCol, stopCol, colSkipFactor);
      pRasterDd->setColumns(columns);

      // Bands
      vector<DimensionDescriptor> bands = pRasterDd->getBands();
      const vector<DimensionDescriptor>& origBands = pRasterDd->getBands();
      vector<DimensionDescriptor>::const_iterator foundBand;
      if (!subsetBands.empty())
      {
         bands.clear();
         for (unsigned int count = 0; count < subsetBands.size(); count++)
         {
            foundBand = find(origBands.begin(), origBands.end(), subsetBands[count]);
            if (foundBand != origBands.end())
            {
               DimensionDescriptor bandDim = subsetBands[count];
               bands.push_back(bandDim);
            }
         }
      }
      pRasterDd->setBands(bands);
   }
}

void RasterUtilities::subsetDataDescriptor(DataDescriptor* pDd,
   const DimensionDescriptor& startRow,
   const DimensionDescriptor& stopRow,
   unsigned int rowSkipFactor,
   const DimensionDescriptor& startCol,
   const DimensionDescriptor& stopCol,
   unsigned int colSkipFactor,
   const DimensionDescriptor& startBand,
   const DimensionDescriptor& stopBand,
   unsigned int bandSkipFactor)
{
   RasterDataDescriptor* pRasterDd = dynamic_cast<RasterDataDescriptor*>(pDd);
   if (pRasterDd != NULL)
   {
      // Bands
      vector<DimensionDescriptor> bands = subsetDimensionVector(pRasterDd->getBands(), startBand, stopBand, bandSkipFactor);
      subsetDataDescriptor(pDd, startRow, stopRow,
         rowSkipFactor, startCol, stopCol, colSkipFactor, bands);
   }
}

RasterElement* RasterUtilities::createRasterElement(const std::string& name, unsigned int rows, unsigned int columns,
   unsigned int bands, EncodingType encoding, InterleaveFormatType interleave, bool inMemory,
   DataElement* pParent)
{
   RasterDataDescriptor* pDd = generateRasterDataDescriptor(name, pParent, rows, columns,
      bands, interleave, encoding, (inMemory ? IN_MEMORY : ON_DISK) ); 

   ModelResource<RasterElement> pRasterElement(pDd);
   if (pRasterElement.get() == NULL)
   {
      return NULL;
   }

   if (pRasterElement->createDefaultPager())
   {
      return pRasterElement.release();
   }

   return NULL;
}

RasterElement* RasterUtilities::createRasterElement(const std::string& name, unsigned int rows, unsigned int columns,
   EncodingType encoding, bool inMemory, DataElement* pParent)
{
   return createRasterElement(name, rows, columns, 1, encoding, BIP, inMemory, pParent);
}

RasterDataDescriptor *RasterUtilities::generateUnchippedRasterDataDescriptor(
   const RasterElement *pOrigElement)
{
   if (pOrigElement == NULL)
   {
      return NULL;
   }

   const RasterDataDescriptor *pOrigDescriptor = dynamic_cast<const RasterDataDescriptor*>(
      pOrigElement->getDataDescriptor());
   if (pOrigDescriptor == NULL)
   {
      return NULL;
   }

   const RasterFileDescriptor *pOrigFileDescriptor = dynamic_cast<const RasterFileDescriptor*>(
      pOrigDescriptor->getFileDescriptor());
   if (pOrigFileDescriptor == NULL)
   {
      return NULL;
   }

   vector<DimensionDescriptor> rows = pOrigFileDescriptor->getRows();
   vector<DimensionDescriptor> columns = pOrigFileDescriptor->getColumns();
   vector<DimensionDescriptor> bands = pOrigFileDescriptor->getBands();

   
   for_each(rows.begin(), rows.end(),
      boost::bind(&DimensionDescriptor::setActiveNumberValid, _1, false)); 
   for_each(columns.begin(), columns.end(),
      boost::bind(&DimensionDescriptor::setActiveNumberValid, _1, false)); 
   for_each(bands.begin(), bands.end(),
      boost::bind(&DimensionDescriptor::setActiveNumberValid, _1, false)); 

   DataDescriptorResource<RasterDataDescriptor> pNewDescriptor("TempImport", "RasterElement", 
      const_cast<RasterElement*>(pOrigElement));
   VERIFY(pNewDescriptor.get() != NULL);
   pNewDescriptor->setRows(rows);
   pNewDescriptor->setColumns(columns);
   pNewDescriptor->setBands(bands);
   pNewDescriptor->setInterleaveFormat(pOrigFileDescriptor->getInterleaveFormat());
   pNewDescriptor->setDataType(pOrigDescriptor->getDataType());
   pNewDescriptor->setProcessingLocation(ON_DISK_READ_ONLY);

   FactoryResource<RasterFileDescriptor> pNewFileDescriptor(
      dynamic_cast<RasterFileDescriptor*>(pOrigFileDescriptor->copy()));
   pNewFileDescriptor->setRows(rows);
   pNewFileDescriptor->setColumns(columns);
   pNewFileDescriptor->setBands(bands);

   pNewDescriptor->setFileDescriptor(pNewFileDescriptor.get());

   return pNewDescriptor.release();
}

vector<string> RasterUtilities::getBandNames(const RasterDataDescriptor* pDescriptor)
{
   vector<string> foundBandNames;
   if (pDescriptor == NULL)
   {
      return foundBandNames;
   }
   const DynamicObject* pMetadata = pDescriptor->getMetadata();
   if (pMetadata == NULL)
   {
      return foundBandNames;
   }
   string pNamesPath[] = { SPECIAL_METADATA_NAME, BAND_METADATA_NAME, NAMES_METADATA_NAME, END_METADATA_NAME };
   const vector<string>* pBandNames = dv_cast<vector<string> >(&pMetadata->getAttributeByPath(pNamesPath));
   string pPrefixPath[] = { SPECIAL_METADATA_NAME, BAND_NAME_PREFIX_METADATA_NAME, END_METADATA_NAME };
   const string* pBandPrefix = dv_cast<string>(&pMetadata->getAttributeByPath(pPrefixPath));
   const vector<DimensionDescriptor>& activeBands = pDescriptor->getBands();
   for (unsigned int i = 0; i < activeBands.size(); ++i)
   {
      const DimensionDescriptor &bandDim = activeBands[i];
      string bandName = ::getBandName(pBandNames, pBandPrefix, bandDim);
      if (bandName.empty())
      {
         return vector<string>();
      }
      foundBandNames.push_back(bandName);
   }

   return foundBandNames;
}

string RasterUtilities::getBandName(const RasterDataDescriptor* pDescriptor, DimensionDescriptor band)
{
   if (pDescriptor == NULL)
   {
      return "";
   }
   const DynamicObject* pMetadata = pDescriptor->getMetadata();
   if (pMetadata == NULL)
   {
      return "";
   }
   string pNamesPath[] = { SPECIAL_METADATA_NAME, BAND_METADATA_NAME, NAMES_METADATA_NAME, END_METADATA_NAME };
   const vector<string>* pBandNames = dv_cast<vector<string> >(&pMetadata->getAttributeByPath(pNamesPath));
   string pPrefixPath[] = { SPECIAL_METADATA_NAME, BAND_NAME_PREFIX_METADATA_NAME, END_METADATA_NAME };
   const string* pBandPrefix = dv_cast<string>(&pMetadata->getAttributeByPath(pPrefixPath));
   return ::getBandName(pBandNames, pBandPrefix, band);
}

namespace
{
   bool findTrueColorDimensionDescriptors(const RasterDataDescriptor *pDescriptor,
      DimensionDescriptor &red, DimensionDescriptor &green, DimensionDescriptor &blue)
   {
      if (pDescriptor == NULL)
      {
         return false;
      }

      const DynamicObject* pMeta = pDescriptor->getMetadata();
      if (pMeta == NULL)
      {
         return false;
      }

      string pWavelengthPath[] = { SPECIAL_METADATA_NAME, BAND_METADATA_NAME, 
         CENTER_WAVELENGTHS_METADATA_NAME, END_METADATA_NAME };
      const vector<double> *pWaveLengths = dv_cast<vector<double> >(
         &pMeta->getAttributeByPath(pWavelengthPath));
      if (pWaveLengths == NULL)
      {
         return false;
      }

      if(pWaveLengths->size() < 3)   // need at least 3 bands for true color
      {
         return false;
      }

      int redIndex(-1), greenIndex(-1), blueIndex(-1);

      // get best blue band match
      blueIndex = RasterUtilities::findBestMatch(*pWaveLengths, BlueCenter, BlueTolerance);
      if (blueIndex == -1)
      {
         return false;
      }

      // get best green band match
      greenIndex = RasterUtilities::findBestMatch(*pWaveLengths, GreenCenter, GreenTolerance, blueIndex+1);
      if (greenIndex == -1)
      {
         return false;
      }

      // get best red band match
      redIndex = RasterUtilities::findBestMatch(*pWaveLengths, RedCenter, RedTolerance, greenIndex+1);
      if (redIndex == -1)
      {
         return false;
      }

      red = pDescriptor->getActiveBand(redIndex);
      green = pDescriptor->getActiveBand(greenIndex);
      blue = pDescriptor->getActiveBand(blueIndex);

      return true;
   }
}
bool RasterUtilities::canBeDisplayedInTrueColor(const RasterDataDescriptor* pDescriptor)
{
   DimensionDescriptor dummy;
   return findTrueColorDimensionDescriptors(pDescriptor, dummy, dummy, dummy);
}

bool RasterUtilities::setDisplayBandsToTrueColor(RasterDataDescriptor* pDescriptor)
{
   DimensionDescriptor red;
   DimensionDescriptor green;
   DimensionDescriptor blue;

   if (findTrueColorDimensionDescriptors(pDescriptor, red, green, blue))
   {
      pDescriptor->setDisplayBand(BLUE, blue);
      pDescriptor->setDisplayBand(GREEN, green);
      pDescriptor->setDisplayBand(RED, red);
      pDescriptor->setDisplayMode(RGB_MODE);
      return true;
   }
   return false;
}

int RasterUtilities::findBestMatch(const std::vector<double> &values, double value, 
                                   double tolerance, int startAt)
{
   int numValues = values.size();
   if (numValues < 1)
   {
      return -1;
   }

   int bestMatch(-1);
   double leastDiff(tolerance);
   double diff;
   for (int i=startAt; i<numValues; ++i)
   {
      diff = fabs(value - values[i]);
      if (diff < leastDiff)
      {
         bestMatch = i;
         leastDiff = diff;
      }
   }

   return bestMatch;
}

vector<RasterChannelType> RasterUtilities::getVisibleRasterChannels()
{
   vector<RasterChannelType> channels;
   channels.push_back(GRAY);
   channels.push_back(RED);
   channels.push_back(GREEN);
   channels.push_back(BLUE);

   return channels;
}

namespace
{
   template<typename T>
   bool chipMetadataDimTemplate(DataVariant &dimMetadataVariant, const vector<DimensionDescriptor> &selectedDims)
   {
      vector<T> *pVec = dimMetadataVariant.getPointerToValue<vector<T> >();
      if (pVec != NULL)
      {
         vector<T> &dimMetadataVector = *pVec;
         vector<T> newDimMetadata;
         newDimMetadata.reserve(selectedDims.size());
         vector<DimensionDescriptor>::const_iterator selectedDimIter = selectedDims.begin();
         for (size_t i = 0; i < dimMetadataVector.size() && selectedDimIter != selectedDims.end(); ++i)
         {
            VERIFY(selectedDimIter->isActiveNumberValid());
            if (i == selectedDimIter->getActiveNumber())
            {
               newDimMetadata.push_back(dimMetadataVector[i]);
               ++selectedDimIter;
            }
         }

         dimMetadataVector.swap(newDimMetadata);
         return true;
      }
      return false;
   }

   void chipMetadataDim(DynamicObject *pMetadata, const vector<DimensionDescriptor> &selectedDims)
   {
      VERIFYNRV(pMetadata != NULL);
      vector<string> attributeNames;
      pMetadata->getAttributeNames(attributeNames);
      for (vector<string>::const_iterator iter = attributeNames.begin();
         iter != attributeNames.end(); ++iter)
      {
         DataVariant &dimMetadataVariant = pMetadata->getAttribute(*iter);

         if (dimMetadataVariant.getTypeName() == "DynamicObject")
         {
            DynamicObject* pChildMetadata = dv_cast<DynamicObject>(&dimMetadataVariant);
            if (pChildMetadata != NULL)
            {
               chipMetadataDim(pChildMetadata, selectedDims);
            }
         }

         if (chipMetadataDimTemplate<char>(dimMetadataVariant, selectedDims)) continue;
         if (chipMetadataDimTemplate<unsigned char>(dimMetadataVariant, selectedDims)) continue;
         if (chipMetadataDimTemplate<short>(dimMetadataVariant, selectedDims)) continue;
         if (chipMetadataDimTemplate<unsigned short>(dimMetadataVariant, selectedDims)) continue;
         if (chipMetadataDimTemplate<int>(dimMetadataVariant, selectedDims)) continue;
         if (chipMetadataDimTemplate<unsigned int>(dimMetadataVariant, selectedDims)) continue;
         if (chipMetadataDimTemplate<long>(dimMetadataVariant, selectedDims)) continue;
         if (chipMetadataDimTemplate<unsigned long>(dimMetadataVariant, selectedDims)) continue;
         if (chipMetadataDimTemplate<int64_t>(dimMetadataVariant, selectedDims)) continue;
         if (chipMetadataDimTemplate<uint64_t>(dimMetadataVariant, selectedDims)) continue;
         if (chipMetadataDimTemplate<float>(dimMetadataVariant, selectedDims)) continue;
         if (chipMetadataDimTemplate<double>(dimMetadataVariant, selectedDims)) continue;
         if (chipMetadataDimTemplate<bool>(dimMetadataVariant, selectedDims)) continue;
         if (chipMetadataDimTemplate<string>(dimMetadataVariant, selectedDims)) continue;
      }

      return;
   }
}

bool RasterUtilities::chipMetadata(
   DynamicObject* pMetadata,                                     
   const vector<DimensionDescriptor> &selectedRows,
   const vector<DimensionDescriptor> &selectedColumns,
   const vector<DimensionDescriptor> &selectedBands)
{
   if (pMetadata != NULL)
   {
      DynamicObject *pAppMetadata = pMetadata->getAttribute(SPECIAL_METADATA_NAME).getPointerToValue<DynamicObject>();
      if (pAppMetadata != NULL)
      {
         DynamicObject *pRowMetadata = pAppMetadata->getAttribute(ROW_METADATA_NAME).getPointerToValue<DynamicObject>();
         if (pRowMetadata != NULL)
         {
            chipMetadataDim(pRowMetadata, selectedRows);
         }

         DynamicObject *pColumnMetadata = pAppMetadata->getAttribute(COLUMN_METADATA_NAME).getPointerToValue<DynamicObject>();
         if (pColumnMetadata != NULL)
         {
            chipMetadataDim(pColumnMetadata, selectedColumns);
         }

         DynamicObject *pBandMetadata = pAppMetadata->getAttribute(BAND_METADATA_NAME).getPointerToValue<DynamicObject>();
         if (pBandMetadata != NULL)
         {
            chipMetadataDim(pBandMetadata, selectedBands);
         }
      }
   }

   return true;

}
