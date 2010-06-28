/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "DataAccessor.h"
#include "DataAccessorImpl.h"
#include "DataRequest.h"
#include "DataVariant.h"
#include "DimensionDescriptor.h"
#include "DynamicObject.h"
#include "Endian.h"
#include "Int64.h"
#include "ObjectResource.h"
#include "Progress.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterFileDescriptor.h"
#include "RasterLayer.h"
#include "RasterUtilities.h"
#include "SpecialMetadata.h"
#include "switchOnEncoding.h"
#include "UInt64.h"

#include <algorithm>
#include <boost/bind.hpp>
#include <sstream>

namespace
{
   void calculateNewPoints(Opticks::PixelLocation point0,
                           Opticks::PixelLocation point1,
                           std::vector<Opticks::PixelLocation>& endPoints)
   {
      endPoints.clear();
      bool steep = abs(point1.mY - point0.mY) > abs(point1.mX - point0.mX);
      if (steep)
      {
         std::swap(point0.mX, point0.mY);
         std::swap(point1.mX, point1.mY);
      }
      bool swapped = (point0.mX > point1.mX);
      if (swapped)
      {
         std::swap(point0, point1);
      }
      int deltax = point1.mX - point0.mX;
      int deltay = abs(point1.mY - point0.mY);
      int error = deltax / 2;
      int ystep = (point0.mY < point1.mY) ? 1 : -1;
      int y = point0.mY;
      for (int x = point0.mX; x <= point1.mX; ++x)
      {
         endPoints.push_back(steep ? Opticks::PixelLocation(y, x) : Opticks::PixelLocation(x, y));
         error -= deltay;
         if (error < 0)
         {
            y += ystep;
            error += deltax;
         }
      }
      if (swapped)
      {
         std::reverse(endPoints.begin(), endPoints.end());
      }
   }

   template<typename T>
   void setPixel(T* pPixel, int defaultValue, unsigned int numValues)
   {
      memset(pPixel, defaultValue, sizeof(T) * numValues);
   }

   template<>
   void setPixel<IntegerComplex>(IntegerComplex* pPixel, int defaultValue, unsigned int numValues)
   {
      memset(pPixel, defaultValue, sizeof(IntegerComplex) * numValues);
   }

   template<>
   void setPixel<FloatComplex>(FloatComplex* pPixel, int defaultValue, unsigned int numValues)
   {
      memset(pPixel, defaultValue, sizeof(FloatComplex) * numValues);
   }
}

std::vector<DimensionDescriptor> RasterUtilities::generateDimensionVector(unsigned int count,
   bool setOriginalNumbers, bool setActiveNumbers, bool setOnDiskNumbers)
{
   std::vector<DimensionDescriptor> retval(count);
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
   for (std::vector<DimensionDescriptor>::size_type count = 0; count != values.size(); ++count)
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
   for (std::vector<DimensionDescriptor>::size_type count = 0; count != values.size(); ++count)
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
   const std::vector<DimensionDescriptor>& origValues,
   const DimensionDescriptor& start,
   const DimensionDescriptor& stop,
   unsigned int skipFactor)
{
   std::vector<DimensionDescriptor> newValues = origValues;
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

      RasterDataDescriptor* pRasterDd = dynamic_cast<RasterDataDescriptor*>(pDd);
      if (pRasterDd != NULL)
      {
         RasterFileDescriptor* pRasterFd = dynamic_cast<RasterFileDescriptor*>(pFd);
         VERIFYNRV(pRasterFd != NULL);

         unsigned int bitsPerElement = RasterUtilities::bytesInEncoding(pRasterDd->getDataType()) * 8;
         pRasterFd->setBitsPerElement(bitsPerElement);

         std::vector<DimensionDescriptor> cols = pRasterDd->getColumns();
         std::transform(cols.begin(), cols.end(), cols.begin(), SetOnDiskNumber());
         pRasterFd->setColumns(cols);
         pRasterDd->setColumns(cols);
         
         std::vector<DimensionDescriptor> rows = pRasterDd->getRows();
         std::transform(rows.begin(), rows.end(), rows.begin(), SetOnDiskNumber());
         pRasterFd->setRows(rows);
         pRasterDd->setRows(rows);

         std::vector<DimensionDescriptor> bands = pRasterDd->getBands();
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

   std::string getBandName(const std::vector<std::string>* pBandNames,
                           const std::string* pBandPrefix,
                           const DimensionDescriptor& band)
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
      }
      std::ostringstream formatter;
      std::string bandPrefix = "Band";
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
   const RasterDataDescriptor* pRasterDd = dynamic_cast<const RasterDataDescriptor*>(pDd);

   // Create the file descriptor
   FileDescriptor* pFd = NULL;
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

   const RasterDataDescriptor* pRasterDd = dynamic_cast<const RasterDataDescriptor*>(pDd);

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
   std::vector<DimensionDescriptor> emptyVector;
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
   const std::vector<DimensionDescriptor>& subsetBands)
{
   const RasterDataDescriptor* pRasterDd = dynamic_cast<const RasterDataDescriptor*>(pDd);

   // Create the file descriptor
   FileDescriptor* pFd = NULL;
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
      RasterFileDescriptor* pRasterFd = dynamic_cast<RasterFileDescriptor*>(pFd);
      VERIFY(pRasterFd != NULL);

      unsigned int bitsPerElement = RasterUtilities::bytesInEncoding(pRasterDd->getDataType()) * 8;
      pRasterFd->setBitsPerElement(bitsPerElement);

      // Rows
      std::vector<DimensionDescriptor> rows =
         subsetDimensionVector(pRasterDd->getRows(), startRow, stopRow, rowSkipFactor);
      std::transform(rows.begin(), rows.end(), rows.begin(), SetOnDiskNumber());
      pRasterFd->setRows(rows);

      // Columns
      std::vector<DimensionDescriptor> columns =
         subsetDimensionVector(pRasterDd->getColumns(), startCol, stopCol, colSkipFactor);
      std::transform(columns.begin(), columns.end(), columns.begin(), SetOnDiskNumber());
      pRasterFd->setColumns(columns);

      pRasterFd->setUnits(pRasterDd->getUnits());
      pRasterFd->setXPixelSize(pRasterDd->getXPixelSize());
      pRasterFd->setYPixelSize(pRasterDd->getYPixelSize());

      // Bands
      std::vector<DimensionDescriptor> bands = pRasterDd->getBands();
      const std::vector<DimensionDescriptor>& origBands = pRasterDd->getBands();
      std::vector<DimensionDescriptor>::const_iterator foundBand;
      if (!subsetBands.empty())
      {
         bands.clear();
         for (unsigned int count = 0; count < subsetBands.size(); count++)
         {
            foundBand = std::find(origBands.begin(), origBands.end(), subsetBands[count]);
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
      std::vector<DimensionDescriptor> bands =
         subsetDimensionVector(pRasterDd->getBands(), startBand, stopBand, bandSkipFactor);
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

RasterDataDescriptor* RasterUtilities::generateRasterDataDescriptor(const std::string& name, DataElement* pParent,
                                                                    unsigned int rows, unsigned int columns,
                                                                    EncodingType encoding, ProcessingLocation location)
{
   return generateRasterDataDescriptor(name, pParent, rows, columns, 1, BIP, encoding, location);
}

RasterDataDescriptor* RasterUtilities::generateRasterDataDescriptor(const std::string& name, DataElement* pParent,
                                                                    unsigned int rows, unsigned int columns,
                                                                    unsigned int bands, InterleaveFormatType interleave,
                                                                    EncodingType encoding, ProcessingLocation location)
{
   Service<ModelServices> pModel;
   RasterDataDescriptor* pDd = dynamic_cast<RasterDataDescriptor*>(
      pModel->createDataDescriptor(name, "RasterElement", pParent));
   if (pDd == NULL)
   {
      return NULL;
   }

   std::vector<DimensionDescriptor> rowDims = generateDimensionVector(rows);
   pDd->setRows(rowDims);
   std::vector<DimensionDescriptor> colDims = generateDimensionVector(columns);
   pDd->setColumns(colDims);
   std::vector<DimensionDescriptor> bandDims = generateDimensionVector(bands);
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
      std::vector<DimensionDescriptor> rows =
         subsetDimensionVector(pRasterDd->getRows(), startRow, stopRow, rowSkipFactor);
      pRasterDd->setRows(rows);

      // Columns
      std::vector<DimensionDescriptor> columns =
         subsetDimensionVector(pRasterDd->getColumns(), startCol, stopCol, colSkipFactor);
      pRasterDd->setColumns(columns);

      // Bands
      std::vector<DimensionDescriptor> bands = pRasterDd->getBands();
      const std::vector<DimensionDescriptor>& origBands = pRasterDd->getBands();
      std::vector<DimensionDescriptor>::const_iterator foundBand;
      if (!subsetBands.empty())
      {
         bands.clear();
         for (unsigned int count = 0; count < subsetBands.size(); count++)
         {
            foundBand = std::find(origBands.begin(), origBands.end(), subsetBands[count]);
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
      std::vector<DimensionDescriptor> bands =
         subsetDimensionVector(pRasterDd->getBands(), startBand, stopBand, bandSkipFactor);
      subsetDataDescriptor(pDd, startRow, stopRow,
         rowSkipFactor, startCol, stopCol, colSkipFactor, bands);
   }
}

RasterElement* RasterUtilities::createRasterElement(const std::string& name, unsigned int rows, unsigned int columns,
   unsigned int bands, EncodingType encoding, InterleaveFormatType interleave, bool inMemory,
   DataElement* pParent)
{
   RasterDataDescriptor* pDd = generateRasterDataDescriptor(name, pParent, rows, columns,
      bands, interleave, encoding, (inMemory ? IN_MEMORY : ON_DISK));

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

   const RasterDataDescriptor* pOrigDescriptor = dynamic_cast<const RasterDataDescriptor*>(
      pOrigElement->getDataDescriptor());
   if (pOrigDescriptor == NULL)
   {
      return NULL;
   }

   const RasterFileDescriptor* pOrigFileDescriptor = dynamic_cast<const RasterFileDescriptor*>(
      pOrigDescriptor->getFileDescriptor());
   if (pOrigFileDescriptor == NULL)
   {
      return NULL;
   }

   std::vector<DimensionDescriptor> rows = pOrigFileDescriptor->getRows();
   std::vector<DimensionDescriptor> columns = pOrigFileDescriptor->getColumns();
   std::vector<DimensionDescriptor> bands = pOrigFileDescriptor->getBands();

   
   std::for_each(rows.begin(), rows.end(),
      boost::bind(&DimensionDescriptor::setActiveNumberValid, _1, false)); 
   std::for_each(columns.begin(), columns.end(),
      boost::bind(&DimensionDescriptor::setActiveNumberValid, _1, false)); 
   std::for_each(bands.begin(), bands.end(),
      boost::bind(&DimensionDescriptor::setActiveNumberValid, _1, false)); 

   DataDescriptorResource<RasterDataDescriptor> pNewDescriptor("TempImport", "RasterElement", 
      const_cast<RasterElement*>(pOrigElement));
   VERIFY(pNewDescriptor.get() != NULL);
   pNewDescriptor->setRows(rows);
   pNewDescriptor->setColumns(columns);
   pNewDescriptor->setBands(bands);
   pNewDescriptor->setInterleaveFormat(pOrigFileDescriptor->getInterleaveFormat());
   pNewDescriptor->setDataType(pOrigDescriptor->getDataType());
   pNewDescriptor->setValidDataTypes(pOrigDescriptor->getValidDataTypes());
   pNewDescriptor->setProcessingLocation(ON_DISK_READ_ONLY);

   FactoryResource<RasterFileDescriptor> pNewFileDescriptor(
      dynamic_cast<RasterFileDescriptor*>(pOrigFileDescriptor->copy()));
   pNewFileDescriptor->setRows(rows);
   pNewFileDescriptor->setColumns(columns);
   pNewFileDescriptor->setBands(bands);

   pNewDescriptor->setFileDescriptor(pNewFileDescriptor.get());

   return pNewDescriptor.release();
}

std::vector<std::string> RasterUtilities::getBandNames(const RasterDataDescriptor* pDescriptor)
{
   std::vector<std::string> foundBandNames;
   if (pDescriptor == NULL)
   {
      return foundBandNames;
   }
   const DynamicObject* pMetadata = pDescriptor->getMetadata();
   if (pMetadata == NULL)
   {
      return foundBandNames;
   }
   std::string pNamesPath[] = { SPECIAL_METADATA_NAME, BAND_METADATA_NAME, NAMES_METADATA_NAME, END_METADATA_NAME };
   const std::vector<std::string>* pBandNames =
      dv_cast<std::vector<std::string> >(&pMetadata->getAttributeByPath(pNamesPath));
   std::string pPrefixPath[] = { SPECIAL_METADATA_NAME, BAND_NAME_PREFIX_METADATA_NAME, END_METADATA_NAME };
   const std::string* pBandPrefix = dv_cast<std::string>(&pMetadata->getAttributeByPath(pPrefixPath));
   const std::vector<DimensionDescriptor>& activeBands = pDescriptor->getBands();
   for (unsigned int i = 0; i < activeBands.size(); ++i)
   {
      const DimensionDescriptor& bandDim = activeBands[i];
      std::string bandName = ::getBandName(pBandNames, pBandPrefix, bandDim);
      if (bandName.empty())
      {
         return std::vector<std::string>();
      }
      foundBandNames.push_back(bandName);
   }

   return foundBandNames;
}

std::string RasterUtilities::getBandName(const RasterDataDescriptor* pDescriptor, DimensionDescriptor band)
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
   std::string pNamesPath[] = { SPECIAL_METADATA_NAME, BAND_METADATA_NAME, NAMES_METADATA_NAME, END_METADATA_NAME };
   const std::vector<std::string>* pBandNames =
      dv_cast<std::vector<std::string> >(&pMetadata->getAttributeByPath(pNamesPath));
   std::string pPrefixPath[] = { SPECIAL_METADATA_NAME, BAND_NAME_PREFIX_METADATA_NAME, END_METADATA_NAME };
   const std::string* pBandPrefix = dv_cast<std::string>(&pMetadata->getAttributeByPath(pPrefixPath));
   return ::getBandName(pBandNames, pBandPrefix, band);
}

bool RasterUtilities::findColorCompositeDimensionDescriptors(const RasterDataDescriptor* pDescriptor,
   const std::string& name, DimensionDescriptor& redBand, DimensionDescriptor& greenBand, DimensionDescriptor& blueBand)
{
   redBand = DimensionDescriptor();
   greenBand = DimensionDescriptor();
   blueBand = DimensionDescriptor();

   const DynamicObject* pColorComposites = RasterLayer::getSettingColorComposites();
   if (pDescriptor == NULL || pColorComposites == NULL)
   {
      return false;
   }

   try
   {
      // Get the ColorComposite from ConfigurationSettings based on the name.
      const DynamicObject* const pObject = dv_cast<DynamicObject>(&pColorComposites->getAttribute(name));
      if (pObject == NULL)
      {
         return false;
      }

      // Red
      const double redLower = dv_cast<double>(pObject->getAttribute("redLower"));
      const double redUpper = dv_cast<double>(pObject->getAttribute("redUpper"));
      redBand = findBandWavelengthMatch(redLower, redUpper, pDescriptor);
      if (redBand.isValid() == false)
      {
         unsigned int redBandOnesBasedIndex = dv_cast<unsigned int>(pObject->getAttribute("redBand"));
         if (redBandOnesBasedIndex != 0)
         {
            redBand = pDescriptor->getOriginalBand(redBandOnesBasedIndex - 1);
         }
      }

      // Green
      const double greenLower = dv_cast<double>(pObject->getAttribute("greenLower"));
      const double greenUpper = dv_cast<double>(pObject->getAttribute("greenUpper"));
      greenBand = findBandWavelengthMatch(greenLower, greenUpper, pDescriptor);
      if (greenBand.isValid() == false)
      {
         unsigned int greenBandOnesBasedIndex = dv_cast<unsigned int>(pObject->getAttribute("greenBand"));
         if (greenBandOnesBasedIndex != 0)
         {
            greenBand = pDescriptor->getOriginalBand(greenBandOnesBasedIndex - 1);
         }
      }

      // Blue
      const double blueLower = dv_cast<double>(pObject->getAttribute("blueLower"));
      const double blueUpper = dv_cast<double>(pObject->getAttribute("blueUpper"));
      blueBand = findBandWavelengthMatch(blueLower, blueUpper, pDescriptor);
      if (blueBand.isValid() == false)
      {
         unsigned int blueBandOnesBasedIndex = dv_cast<unsigned int>(pObject->getAttribute("blueBand"));
         if (blueBandOnesBasedIndex != 0)
         {
            blueBand = pDescriptor->getOriginalBand(blueBandOnesBasedIndex - 1);
         }
      }
   }
   catch (const std::bad_cast&)
   {}

   return redBand.isValid() && greenBand.isValid() && blueBand.isValid();
}

int RasterUtilities::findBandWavelengthMatch(double lowTarget, double highTarget,
   const std::vector<double>& lowWavelengths, const std::vector<double>& highWavelengths, bool allowPartialMatch)
{
   std::vector<unsigned int> matches =
      findBandWavelengthMatches(lowTarget, highTarget, lowWavelengths, highWavelengths, allowPartialMatch);
   if (matches.empty() || matches[matches.size() / 2] < 0)
   {
      return -1;
   }

   return static_cast<int>(matches[matches.size() / 2]);
}

std::vector<unsigned int> RasterUtilities::findBandWavelengthMatches(double lowTarget, double highTarget,
   const std::vector<double>& lowWavelengths, const std::vector<double>& highWavelengths, bool allowPartialMatch)
{
   if (!highWavelengths.empty() && highWavelengths.size() != lowWavelengths.size())
   {
      return std::vector<unsigned int>();
   }

   // Filter wavelength data and select appropriate band
   typedef std::pair<std::pair<double, double>, unsigned int> wavelength_item;
   std::vector<wavelength_item> candidateBands;
   for (unsigned int bandNumber = 0; bandNumber < lowWavelengths.size(); ++bandNumber)
   {
      if (highWavelengths.empty())
      {
         // Check center wavelength and if it's in range, add it as a candidate.
         if (lowWavelengths[bandNumber] >= lowTarget && lowWavelengths[bandNumber] <= highTarget)
         {
            candidateBands.push_back(std::make_pair(
               std::make_pair(lowWavelengths[bandNumber], lowWavelengths[bandNumber]), bandNumber));
         }
      }
      else if (allowPartialMatch)
      {
         // Check for any overlap and if in range, add it as a candidate.
         if ((lowWavelengths[bandNumber] >= lowTarget && lowWavelengths[bandNumber] <= highTarget) ||
             (highWavelengths[bandNumber] >= lowTarget && highWavelengths[bandNumber] <= highTarget))
         {
            candidateBands.push_back(std::make_pair(
               std::make_pair(lowWavelengths[bandNumber], highWavelengths[bandNumber]), bandNumber));
         }
      }
      else if (lowWavelengths[bandNumber] >= lowTarget && highWavelengths[bandNumber] <= highTarget)
      {
         // If the entire band is in range, add it as a candidate
         candidateBands.push_back(std::make_pair(
            std::make_pair(lowWavelengths[bandNumber], highWavelengths[bandNumber]), bandNumber));
      }
   }
   if (!candidateBands.empty())
   {
      // Sort by low band range
      std::stable_sort(candidateBands.begin(), candidateBands.end());

      // Build the vector for return
      std::vector<unsigned int> bands(candidateBands.size());
      for (unsigned int band = 0; band < candidateBands.size(); ++band)
      {
         bands[band] = candidateBands[band].second;
      }

      return bands;
   }
   // No bands fall in range.
   return std::vector<unsigned int>();
}

DimensionDescriptor RasterUtilities::findBandWavelengthMatch(double lowTarget, double highTarget,
   const RasterDataDescriptor* pDescriptor, bool allowPartialMatch)
{
   if (pDescriptor == NULL)
   {
      return DimensionDescriptor();
   }

   const RasterFileDescriptor* pFileDescriptor =
      dynamic_cast<const RasterFileDescriptor*>(pDescriptor->getFileDescriptor());
   const DynamicObject* pMetadata = pDescriptor->getMetadata();
   if (pMetadata == NULL)
   {
      return DimensionDescriptor();
   }

   const std::string centerWavelengthsPath[] = {SPECIAL_METADATA_NAME, BAND_METADATA_NAME,
      CENTER_WAVELENGTHS_METADATA_NAME, END_METADATA_NAME};
   const std::vector<double>* pCenterWavelengths =
      dv_cast<std::vector<double> >(&pMetadata->getAttributeByPath(centerWavelengthsPath));

   const std::string startWavelengthsPath[] = {SPECIAL_METADATA_NAME, BAND_METADATA_NAME,
      START_WAVELENGTHS_METADATA_NAME, END_METADATA_NAME};
   const std::vector<double>* pStartWavelengths = 
      dv_cast<std::vector<double> >(&pMetadata->getAttributeByPath(startWavelengthsPath));

   const std::string endWavelengthsPath[] = {SPECIAL_METADATA_NAME, BAND_METADATA_NAME,
      END_WAVELENGTHS_METADATA_NAME, END_METADATA_NAME};
   const std::vector<double>* pEndWavelengths = 
      dv_cast<std::vector<double> >(&pMetadata->getAttributeByPath(endWavelengthsPath));

   bool useOriginalNumbers;
   std::vector<unsigned int> bands;
   if ((pStartWavelengths != NULL && pStartWavelengths->empty() == false) &&
      (pEndWavelengths != NULL && pEndWavelengths->empty() == false))
   {
      // Start and end wavelengths available. Use those as the targets.
      bands = findBandWavelengthMatches(lowTarget, highTarget,
         *pStartWavelengths, *pEndWavelengths, allowPartialMatch);
      useOriginalNumbers = pFileDescriptor != NULL && pStartWavelengths->size() == pFileDescriptor->getBandCount();
   }
   else if (pCenterWavelengths != NULL && pCenterWavelengths->empty() == false)
   {
      // Centers are available so we use those as the targets.
      bands = findBandWavelengthMatches(lowTarget, highTarget,
         *pCenterWavelengths, *pCenterWavelengths, allowPartialMatch);
      useOriginalNumbers = pFileDescriptor != NULL && pCenterWavelengths->size() == pFileDescriptor->getBandCount();
   }
   else if (pStartWavelengths != NULL && pStartWavelengths->empty() == false)
   {
      // Only start wavelengths are available, so use those as the targets.
      bands = findBandWavelengthMatches(lowTarget, highTarget,
         *pStartWavelengths, *pStartWavelengths, allowPartialMatch);
      useOriginalNumbers = pFileDescriptor != NULL && pStartWavelengths->size() == pFileDescriptor->getBandCount();
   }
   else if (pEndWavelengths != NULL && pEndWavelengths->empty() == false)
   {
      // Only end wavelengths are available, so use those as the targets.
      bands = findBandWavelengthMatches(lowTarget, highTarget,
         *pEndWavelengths, *pEndWavelengths, allowPartialMatch);
      useOriginalNumbers = pFileDescriptor != NULL && pEndWavelengths->size() == pFileDescriptor->getBandCount();
   }

   if (useOriginalNumbers)
   {
      class OriginalBandChecker
      {
      public:
         OriginalBandChecker(const RasterDataDescriptor* pDescriptor) :
            mpDescriptor(pDescriptor)
         {}

         bool operator()(unsigned int band)
         {
            return (mpDescriptor->getOriginalBand(band).isValid() == false);
         }

      private:
         const RasterDataDescriptor* mpDescriptor;
      };

      std::vector<unsigned int>::iterator iter =
         std::remove_if(bands.begin(), bands.end(), OriginalBandChecker(pDescriptor));
      if (iter != bands.end())
      {
         bands.erase(iter);
      }
   }

   if (bands.empty())
   {
      return DimensionDescriptor();
   }

   if (useOriginalNumbers)
   {
      return pDescriptor->getOriginalBand(bands[bands.size() / 2]);
   }

   return pDescriptor->getActiveBand(bands[bands.size() / 2]);
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
   for (int i = startAt; i < numValues; ++i)
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

std::vector<RasterChannelType> RasterUtilities::getVisibleRasterChannels()
{
   std::vector<RasterChannelType> channels;
   channels.push_back(GRAY);
   channels.push_back(RED);
   channels.push_back(GREEN);
   channels.push_back(BLUE);

   return channels;
}

namespace
{
   template<typename T>
   bool chipMetadataDimTemplate(DataVariant &dimMetadataVariant, const std::vector<DimensionDescriptor> &selectedDims)
   {
      std::vector<T> *pVec = dimMetadataVariant.getPointerToValue<std::vector<T> >();
      if (pVec != NULL)
      {
         std::vector<T> &dimMetadataVector = *pVec;
         std::vector<T> newDimMetadata;
         newDimMetadata.reserve(selectedDims.size());
         std::vector<DimensionDescriptor>::const_iterator selectedDimIter = selectedDims.begin();
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

   void chipMetadataDim(DynamicObject *pMetadata, const std::vector<DimensionDescriptor> &selectedDims)
   {
      VERIFYNRV(pMetadata != NULL);
      std::vector<std::string> attributeNames;
      pMetadata->getAttributeNames(attributeNames);
      for (std::vector<std::string>::const_iterator iter = attributeNames.begin();
         iter != attributeNames.end(); ++iter)
      {
         DataVariant& dimMetadataVariant = pMetadata->getAttribute(*iter);

         if (dimMetadataVariant.getTypeName() == "DynamicObject")
         {
            DynamicObject* pChildMetadata = dv_cast<DynamicObject>(&dimMetadataVariant);
            if (pChildMetadata != NULL)
            {
               chipMetadataDim(pChildMetadata, selectedDims);
            }
         }

         if (chipMetadataDimTemplate<char>(dimMetadataVariant, selectedDims))
         {
            continue;
         }

         if (chipMetadataDimTemplate<unsigned char>(dimMetadataVariant, selectedDims))
         {
            continue;
         }

         if (chipMetadataDimTemplate<short>(dimMetadataVariant, selectedDims))
         {
            continue;
         }

         if (chipMetadataDimTemplate<unsigned short>(dimMetadataVariant, selectedDims))
         {
            continue;
         }

         if (chipMetadataDimTemplate<int>(dimMetadataVariant, selectedDims))
         {
            continue;
         }

         if (chipMetadataDimTemplate<unsigned int>(dimMetadataVariant, selectedDims))
         {
            continue;
         }

         if (chipMetadataDimTemplate<long>(dimMetadataVariant, selectedDims))
         {
            continue;
         }

         if (chipMetadataDimTemplate<unsigned long>(dimMetadataVariant, selectedDims))
         {
            continue;
         }

         if (chipMetadataDimTemplate<int64_t>(dimMetadataVariant, selectedDims))
         {
            continue;
         }

         if (chipMetadataDimTemplate<uint64_t>(dimMetadataVariant, selectedDims))
         {
            continue;
         }

         if (chipMetadataDimTemplate<Int64>(dimMetadataVariant, selectedDims))
         {
            continue;
         }

         if (chipMetadataDimTemplate<UInt64>(dimMetadataVariant, selectedDims))
         {
            continue;
         }

         if (chipMetadataDimTemplate<float>(dimMetadataVariant, selectedDims))
         {
            continue;
         }

         if (chipMetadataDimTemplate<double>(dimMetadataVariant, selectedDims))
         {
            continue;
         }

         if (chipMetadataDimTemplate<bool>(dimMetadataVariant, selectedDims))
         {
            continue;
         }

         if (chipMetadataDimTemplate<std::string>(dimMetadataVariant, selectedDims))
         {
            continue;
         }
      }

      return;
   }
}

bool RasterUtilities::chipMetadata(DynamicObject* pMetadata, const std::vector<DimensionDescriptor>& selectedRows,
                                   const std::vector<DimensionDescriptor>& selectedColumns,
                                   const std::vector<DimensionDescriptor>& selectedBands)
{
   if (pMetadata != NULL)
   {
      DynamicObject* pAppMetadata = pMetadata->getAttribute(SPECIAL_METADATA_NAME).getPointerToValue<DynamicObject>();
      if (pAppMetadata != NULL)
      {
         DynamicObject* pRowMetadata = pAppMetadata->getAttribute(ROW_METADATA_NAME).getPointerToValue<DynamicObject>();
         if (pRowMetadata != NULL)
         {
            chipMetadataDim(pRowMetadata, selectedRows);
         }

         DynamicObject* pColumnMetadata =
            pAppMetadata->getAttribute(COLUMN_METADATA_NAME).getPointerToValue<DynamicObject>();
         if (pColumnMetadata != NULL)
         {
            chipMetadataDim(pColumnMetadata, selectedColumns);
         }

         DynamicObject* pBandMetadata =
            pAppMetadata->getAttribute(BAND_METADATA_NAME).getPointerToValue<DynamicObject>();
         if (pBandMetadata != NULL)
         {
            chipMetadataDim(pBandMetadata, selectedBands);
         }
      }
   }

   return true;

}

bool RasterUtilities::isSubcube(const RasterDataDescriptor* pDescriptor, bool checkBands)
{
   if (pDescriptor == NULL)
   {
      return false;
   }
   const RasterFileDescriptor* pFileDescriptor = 
      dynamic_cast<const RasterFileDescriptor*>(pDescriptor->getFileDescriptor());
   if (pFileDescriptor == NULL)
   {
      return false;
   }
   if (checkBands == true)
   {
      if (pDescriptor->getRows() == pFileDescriptor->getRows() &&
         pDescriptor->getColumns() == pFileDescriptor->getColumns() &&
         pDescriptor->getBands() == pFileDescriptor->getBands())
      {
         return false;
      }
   }
   else
   {
      if (pDescriptor->getRows() == pFileDescriptor->getRows() &&
         pDescriptor->getColumns() == pFileDescriptor->getColumns())
      {
         return false;
      }
   }
   return true;
}

int64_t RasterUtilities::calculateFileSize(const RasterFileDescriptor* pDescriptor)
{
   int64_t fileSize(-1);
   if (pDescriptor == NULL)
   {
      return fileSize;
   }

   int64_t numRows = pDescriptor->getRowCount();
   int64_t numColumns = pDescriptor->getColumnCount();
   int64_t numBands = pDescriptor->getBandCount();
   int64_t bitsPerElement = pDescriptor->getBitsPerElement();
   int64_t headerBytes = pDescriptor->getHeaderBytes();
   int64_t prelineBytes = pDescriptor->getPrelineBytes();
   int64_t postlineBytes = pDescriptor->getPostlineBytes();
   int64_t prebandBytes = pDescriptor->getPrebandBytes();
   int64_t postbandBytes = pDescriptor->getPostbandBytes();
   int64_t trailerBytes = pDescriptor->getTrailerBytes();
   InterleaveFormatType interleave = pDescriptor->getInterleaveFormat();

   // File size calculation
   /*
      The total number of pre-line, post-line, pre-band and post-band bytes depends
      on the interleave format. Symbols: B1 = Band 1, R1 = Row 1, C1 = Column 1

      BSQ               BIL               BIP
      -------           -------           -------
      Header            Header            Header
      Preband           Preline           Preline
      Preline           R1 B1 C1          R1 C1 B1
      B1 R1 C1          ...               ...   
      ...               R1 B1 Cn          R1 C1 Bn
      B1 R1 Cn          ...               ...   
      Postline          R1 Bn C1          R1 Cn Bn
      ...               ...               Postline
      Preline           R1 Bn Cn           ...
      B1 Rn C1          Postline          Preline
      ...               ...               Rn C1 B1
      B1 Rn Cn          Preline           ...
      Postline          Rn B1 C1          Rn Cn Bn
      Postband          ...               Postline
      ...               Rn Bn Cn          Trailer
      Preband           Postline
      Preline           Trailer  
      Bn Rn C1
      ...
      Bn Rn Cn
      Postline
      Postband
      Trailer

      NOTE: 
      Pre/Post Band bytes are not used with BIL & BIP data - it would be a tremendous waste of space.
   */

   int64_t columnSize = bitsPerElement / 8;
   int64_t rowSize(0);
   int64_t bandSize(0);

   switch (interleave)
   {
   case BSQ:
      {
         const std::vector<const Filename*>& bandFiles = pDescriptor->getBandFiles();
         if (bandFiles.empty() == false)  // then data in multiple files
         {
            numBands = 1;                 // one band per file
         }
      }
      rowSize = prelineBytes + (numColumns * columnSize) + postlineBytes;
      bandSize = prebandBytes + (numRows * rowSize) + postbandBytes;
      fileSize = headerBytes + (numBands * bandSize) + trailerBytes;
      break;

   case BIL:
      bandSize = numColumns * columnSize;
      rowSize = prelineBytes + (numBands * bandSize) + postlineBytes;
      fileSize = headerBytes + (numRows * rowSize) + trailerBytes;
      break;

   case BIP:
      columnSize *= numBands;
      rowSize = prelineBytes + (numColumns * columnSize) + postlineBytes;
      fileSize = headerBytes + (numRows * rowSize) + trailerBytes;
      break;

   default:
      break;
   }

   return fileSize;
}

bool RasterUtilities::rotate(RasterElement* pDst, const RasterElement* pSrc, double angle, int defaultValue,
                             RasterUtilities::InterpolationType interp, Progress* pProgress, bool* pAbort)
{
   if (pDst == NULL || pSrc == NULL)
   {
      if (pProgress != NULL)
      {
         pProgress->updateProgress("Invalid cube", 0, ERRORS);
      }
      return false;
   }
   const RasterDataDescriptor* pSrcDesc = static_cast<const RasterDataDescriptor*>(pSrc->getDataDescriptor());
   RasterDataDescriptor* pDstDesc = static_cast<RasterDataDescriptor*>(pDst->getDataDescriptor());
   std::vector<int> bvalues = pDstDesc->getBadValues();
   if (std::find(bvalues.begin(), bvalues.end(), defaultValue) == bvalues.end())
   {
      bvalues.push_back(defaultValue);
      pDstDesc->setBadValues(bvalues);
   }
   unsigned int numRows = pSrcDesc->getRowCount();
   unsigned int numCols = pSrcDesc->getColumnCount();
   unsigned int numBands = pSrcDesc->getBandCount();
   bool isBip = (pSrcDesc->getInterleaveFormat() == BIP);

   if (numRows != pDstDesc->getRowCount() ||
       numCols != pDstDesc->getColumnCount() ||
       numBands != pDstDesc->getBandCount() ||
       isBip != (pDstDesc->getInterleaveFormat() == BIP))
   {
      if (pProgress != NULL)
      {
         pProgress->updateProgress("Desitnation cube is not compatible with source cube.", 0, ERRORS);
      }
      return false;
   }

   // calculate the rotation of the four corners
   int x1 = numCols / 2;
   int y1 = numRows / 2;
   int x0 = -(static_cast<int>(numCols) - x1 - 1);
   int y0 = -(static_cast<int>(numRows) - y1 - 1);
   Opticks::PixelLocation ul(x0, y0);
   Opticks::PixelLocation ur(x1, y0);
   Opticks::PixelLocation ll(x0, y1);
   Opticks::PixelLocation lr(x1, y1);
   double cosA = cos(angle);
   double sinA = sin(angle);
   Opticks::PixelLocation ulPrime(static_cast<int>(ul.mX * cosA - ul.mY * sinA + 0.5),
      static_cast<int>(ul.mX * sinA + ul.mY * cosA + 0.5));
   Opticks::PixelLocation urPrime(static_cast<int>(ur.mX * cosA - ur.mY * sinA + 0.5),
      static_cast<int>(ur.mX * sinA + ur.mY * cosA + 0.5));
   Opticks::PixelLocation llPrime(static_cast<int>(ll.mX * cosA - ll.mY * sinA + 0.5),
      static_cast<int>(ll.mX * sinA + ll.mY * cosA + 0.5));
   Opticks::PixelLocation lrPrime(static_cast<int>(lr.mX * cosA - lr.mY * sinA + 0.5),
      static_cast<int>(lr.mX * sinA + lr.mY * cosA + 0.5));

   // use Bresenham's to calculate the start and end coordinates of each row
   std::vector<Opticks::PixelLocation> newRowStartPre;
   calculateNewPoints(ulPrime, llPrime, newRowStartPre);
   std::vector<Opticks::PixelLocation> newRowEndPre;
   calculateNewPoints(urPrime, lrPrime, newRowEndPre);

   // interpolate so we have the proper number of points
   std::vector<Opticks::PixelLocation> newRowStart;
   std::vector<Opticks::PixelLocation> newRowEnd;
   newRowStart.reserve(numRows);
   newRowEnd.reserve(numRows);
   double startMult = newRowStartPre.size() / static_cast<double>(numRows);
   double endMult = newRowEndPre.size() / static_cast<double>(numRows);
   for (unsigned int row = 0; row < numRows; ++row)
   {
      if (pProgress != NULL)
      {
         pProgress->updateProgress("Calculating row shifts.", row * 100 / numRows, NORMAL);
      }
      switch(interp)
      {
      case NEAREST_NEIGHBOR:
         {
            unsigned int preStartRow = static_cast<int>(startMult * row + 0.5);
            unsigned int preEndRow = static_cast<int>(endMult * row + 0.5);
            newRowStart.push_back(newRowStartPre[preStartRow]);
            newRowEnd.push_back(newRowEndPre[preEndRow]);
            break;
         }
      case BILINEAR:
      case BICUBIC:
      default:
         if (pProgress != NULL)
         {
            pProgress->updateProgress("Invalid or unsupported interpolation method.", 0, ERRORS);
         }
         return false;
      }
   }
   if (newRowStart.size() != numRows || newRowEnd.size() != numRows)
   {
      if (pProgress != NULL)
      {
         pProgress->updateProgress("Error calculating new row positions.", 0, ERRORS);
      }
      return false;
   }

   unsigned int outBandLoop = isBip ? 1 : numBands;
   unsigned int inBandLoop = isBip ? numBands : 1;
   for (unsigned int outBand = 0; outBand < outBandLoop; ++outBand)
   {
      FactoryResource<DataRequest> pSrcRequest;
      FactoryResource<DataRequest> pDstRequest;
      VERIFY(pSrcRequest.get() && pDstRequest.get());
      pSrcRequest->setRows(DimensionDescriptor(), DimensionDescriptor(), 1);
      pDstRequest->setRows(DimensionDescriptor(), DimensionDescriptor(), 1);
      pSrcRequest->setColumns(DimensionDescriptor(), DimensionDescriptor(), numCols);
      pDstRequest->setColumns(DimensionDescriptor(), DimensionDescriptor(), numCols);
      if (!isBip)
      {
         pSrcRequest->setBands(pSrcDesc->getActiveBand(outBand), pSrcDesc->getActiveBand(outBand), 1);
         pDstRequest->setBands(pDstDesc->getActiveBand(outBand), pDstDesc->getActiveBand(outBand), 1);
      }
      pDstRequest->setWritable(true);

      DataAccessor pSrcAcc = pSrc->getDataAccessor(pSrcRequest.release());
      DataAccessor pDstAcc = pDst->getDataAccessor(pDstRequest.release());
      for (unsigned int row = 0; row < numRows; ++row)
      {
         if (pProgress != NULL)
         {
            pProgress->updateProgress("Warping rows.", row * 100 / numRows, NORMAL);
         }
         if (pAbort != NULL && *pAbort)
         {
            if (pProgress != NULL)
            {
               pProgress->updateProgress("Aborted by user.", 0, ABORT);
            }
            return false;
         }
         if (!pDstAcc.isValid())
         {
            if (pProgress != NULL)
            {
               pProgress->updateProgress("Error copying data.", 0, ERRORS);
            }
            return false;
         }
         // initialize the row...this is faster than checking validity at each
         // pixel and setting the default value at that pixel
         switchOnComplexEncoding(pDstDesc->getDataType(), setPixel,
            pDstAcc->getRow(), defaultValue, inBandLoop*numCols);

         // use Bresenham's to calculate the column coordinates
         std::vector<Opticks::PixelLocation> newColPre;
         calculateNewPoints(newRowStart[row], newRowEnd[row], newColPre);
         // interpolate so we have the proper number of points
         double mult = newColPre.size() / static_cast<double>(numCols);
         for (unsigned int col = 0; col < numCols; ++col)
         {
            if (!pDstAcc.isValid())
            {
               if (pProgress != NULL)
               {
                  pProgress->updateProgress("Error copying data.", 0, ERRORS);
               }
               return false;
            }
            Opticks::PixelLocation sourcePixel;
            switch(interp)
            {
            case NEAREST_NEIGHBOR:
               {
                  unsigned int preCol = static_cast<int>(mult * col + 0.5);
                  sourcePixel = newColPre[preCol];
                  break;
               }
            case BILINEAR:
            case BICUBIC:
            default:
               if (pProgress != NULL)
               {
                  pProgress->updateProgress("Invalid or unsupported interpolation method.", 0, ERRORS);
               }
               return false;
            }
            sourcePixel -= Opticks::PixelLocation(x0, y0);
            if (sourcePixel.mX >= 0 && sourcePixel.mX < static_cast<int>(numCols) &&
                sourcePixel.mY >= 0 && sourcePixel.mY < static_cast<int>(numRows))
            {
               pSrcAcc->toPixel(sourcePixel.mY, sourcePixel.mX);
               if (!pSrcAcc.isValid())
               {
                  if (pProgress != NULL)
                  {
                     pProgress->updateProgress("Error reading source cube.", 0, ERRORS);
                  }
                  return false;
               }
               memcpy(pDstAcc->getColumn(), pSrcAcc->getColumn(), pDstDesc->getBytesPerElement() * inBandLoop);
            }
            pDstAcc->nextColumn();
         }
         pDstAcc->nextRow();
      }
   }
   pDst->updateData();

   return true;
}
