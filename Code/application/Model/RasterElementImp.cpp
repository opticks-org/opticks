/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"
#include "AppVerify.h"
#include "ConfigurationSettings.h"
#include "ConvertToBipPager.h"
#include "ConvertToBsqPager.h"
#include "DataAccessorImpl.h"
#include "DataRequest.h"
#include "DimensionDescriptor.h"
#include "Executable.h"
#include "FileResource.h"
#include "Georeference.h"
#include "Importer.h"
#include "ModelServices.h"
#include "ObjectResource.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInResource.h"
#include "RasterDataDescriptor.h"
#include "RasterDataDescriptorImp.h"
#include "RasterElement.h"
#include "RasterElementImp.h"
#include "RasterFileDescriptor.h"
#include "RasterFileDescriptorImp.h"
#include "RasterPage.h"
#include "RasterPager.h"
#include "RasterUtilities.h"
#include "SessionItemDeserializer.h"
#include "SessionItemSerializer.h"
#include "SessionManager.h"
#include "StatisticsImp.h"
#include "xmlwriter.h"

#include <fstream>
#include <limits>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
using namespace std;
XERCES_CPP_NAMESPACE_USE

RasterElementImp::RasterElementImp(const DataDescriptorImp& descriptor, const string& id) :
   DataElementImp(descriptor, id), mpTerrain(NULL), mpPager(NULL),
   mpConverterPager(NULL), mCubePointerAccessor(NULL, NULL), mModified(false),
   mpGeoPlugin(NULL)
{
   RasterDataDescriptorImp* pDescriptor = dynamic_cast<RasterDataDescriptorImp*>(getDataDescriptor());
   if (pDescriptor != NULL)
   {
      RasterFileDescriptorImp* pFileDescriptor = dynamic_cast<RasterFileDescriptorImp*>(
         pDescriptor->getFileDescriptor());
      vector<DimensionDescriptor> bands = pDescriptor->getBands();
      vector<DimensionDescriptor> fileBands;
      vector<DimensionDescriptor>::iterator fileBandIter;
      vector<int> badValues = pDescriptor->getBadValues();

      if (pFileDescriptor != NULL)
      {
         fileBands = pFileDescriptor->getBands();
         fileBandIter = fileBands.begin();
      }
      for (vector<DimensionDescriptor>::size_type i = 0; i < bands.size(); ++i)
      {
         bands[i].setActiveNumber(i);
         StatisticsImp* pStatistics = new StatisticsImp(this, bands[i]);
         if (pStatistics != NULL)
         {
            pStatistics->setBadValues(badValues);
            mStatistics[bands[i]] = pStatistics;
         }
         if (!fileBands.empty())
         {
            unsigned int onDiskNumber = bands[i].getOnDiskNumber();
            if (onDiskNumber < fileBands.size())
            {
               fileBands[onDiskNumber].setActiveNumber(i);
            }
         }
      }
      pDescriptor->setBands(bands);
      if (pFileDescriptor != NULL)
      {
         pFileDescriptor->setBands(fileBands);
      }

      vector<DimensionDescriptor> rows = pDescriptor->getRows();
      vector<DimensionDescriptor> fileRows;
      vector<DimensionDescriptor>::iterator fileRowIter;
      if (pFileDescriptor != NULL)
      {
         fileRows = pFileDescriptor->getRows();
         fileRowIter = fileRows.begin();
      }
      for (vector<DimensionDescriptor>::size_type i = 0; i < rows.size(); ++i)
      {
         rows[i].setActiveNumber(i);
         if (!fileRows.empty())
         {
            unsigned int onDiskNumber = rows[i].getOnDiskNumber();
            if (onDiskNumber < fileRows.size())
            {
               fileRows[onDiskNumber].setActiveNumber(i);
            }
         }
      }
      pDescriptor->setRows(rows);
      if (pFileDescriptor != NULL)
      {
         pFileDescriptor->setRows(fileRows);
      }

      vector<DimensionDescriptor> columns = pDescriptor->getColumns();
      vector<DimensionDescriptor> fileColumns;
      vector<DimensionDescriptor>::iterator fileColIter;
      if (pFileDescriptor != NULL)
      {
         fileColumns = pFileDescriptor->getColumns();
         fileColIter = fileColumns.begin();
      }
      for (vector<DimensionDescriptor>::size_type i = 0; i < columns.size(); ++i)
      {
         columns[i].setActiveNumber(i);
         if (!fileColumns.empty())
         {
            unsigned int onDiskNumber = columns[i].getOnDiskNumber();
            if (onDiskNumber < fileColumns.size())
            {
               fileColumns[onDiskNumber].setActiveNumber(i);
            }
         }
      }
      pDescriptor->setColumns(columns);
      if (pFileDescriptor != NULL)
      {
         pFileDescriptor->setColumns(fileColumns);
      }
   }
}

RasterElementImp::~RasterElementImp()
{
   if (mpTerrain.get() != NULL)
   {
      RasterElement* pTerrain = mpTerrain.get();
      mpTerrain.reset(NULL);
      Service<ModelServices>()->destroyElement(pTerrain);
   }

   mCubePointerAccessor = DataAccessor(NULL, NULL);
   if (mpConverterPager != NULL)
   {
      delete mpConverterPager;
   }

   Service<PlugInManagerServices> pPluginManager;
   if (mpPager != NULL)
   {
      pPluginManager->destroyPlugIn(dynamic_cast<PlugIn*>(mpPager));
   }

   if (mTempFilename.empty() == false)
   {
      remove(mTempFilename.c_str());
   }

   if (mpGeoPlugin != NULL)
   {
      pPluginManager->destroyPlugIn(dynamic_cast<PlugIn*>(mpGeoPlugin));
   }

}

double RasterElementImp::getPixelValue(DimensionDescriptor columnDim, DimensionDescriptor rowDim,
                                     DimensionDescriptor bandDim, ComplexComponent component) const
{
   if ((columnDim.isValid() == false) || (rowDim.isValid() == false))
   {
      return 0.0;
   }

   if ((columnDim.isActiveNumberValid() == false) || (rowDim.isActiveNumberValid() == false))
   {
      return 0.0;
   }

   const RasterDataDescriptorImp* pDescriptor = dynamic_cast<const RasterDataDescriptorImp*>(getDataDescriptor());
   if (pDescriptor == NULL)
   {
      return 0.0;
   }

   FactoryResource<DataRequest> pRequest;
   pRequest->setRows(rowDim, rowDim, 1);
   pRequest->setColumns(columnDim, columnDim, 1);
   pRequest->setBands(bandDim, bandDim, 1);

   DataAccessor da = getDataAccessor(pRequest.release());
   if (da.isValid() == false)
   {
      return 0.0;
   }

   void* pData = da->getColumn();
   EncodingType dataType = pDescriptor->getDataType();

   return Service<ModelServices>()->getDataValue(dataType, pData, component, 0);
}

void RasterElementImp::updateData()
{
   map<DimensionDescriptor, StatisticsImp*>::iterator iter;
   for (iter = mStatistics.begin(); iter != mStatistics.end(); ++iter)
   {
      StatisticsImp* pStatistics = iter->second;
      if (pStatistics != NULL)
      {
         pStatistics->resetAll();
      }
   }

   mModified = true;
   notify(SIGNAL_NAME(RasterElement, DataModified));
}

uint64_t RasterElementImp::sanitizeData(double value)
{
   uint64_t badValueCount = 0;

   const RasterDataDescriptor* const pDescriptor = dynamic_cast<RasterDataDescriptor*>(getDataDescriptor());
   if (pDescriptor != NULL)
   {
      EncodingType dataType = pDescriptor->getDataType();
      if (dataType == FLT4BYTES || dataType == FLT8COMPLEX || dataType == FLT8BYTES)
      {
         const uint64_t numRows = pDescriptor->getRowCount();
         const uint64_t numColumns = pDescriptor->getColumnCount();
         const uint64_t numBands = pDescriptor->getBandCount();
         const InterleaveFormatType interleave = pDescriptor->getInterleaveFormat();

         if (interleave == BIP || interleave == BIL)
         {
            uint64_t elementCount = numColumns * numBands;

            FactoryResource<DataRequest> pRequest;
            pRequest->setWritable(true);
            pRequest->setBands(pDescriptor->getActiveBand(0),
               pDescriptor->getActiveBand(static_cast<unsigned int>(numBands - 1)));

            DataAccessor da = getDataAccessor(pRequest.release());
            for (uint64_t row = 0; row < numRows; ++row)
            {
               VERIFYRV(da.isValid(), badValueCount);
               void* pRow = da->getRow();
               VERIFYRV(pRow != NULL, badValueCount);
               badValueCount += RasterUtilities::sanitizeData(pRow, elementCount, dataType, value);
               da->nextRow();
            }
         }
         else if (interleave == BSQ)
         {
            const vector<DimensionDescriptor>& bands = pDescriptor->getBands();
            uint64_t elementCount = numColumns;
            for (uint64_t band = 0; band < numBands; ++band)
            {
               FactoryResource<DataRequest> pRequest;
               pRequest->setWritable(true);
               pRequest->setBands(bands[static_cast<unsigned int>(band)], bands[static_cast<unsigned int>(band)]);
               
               DataAccessor da = getDataAccessor(pRequest.release());
               for (uint64_t row = 0; row < numRows; ++row)
               {
                  VERIFYRV(da.isValid(), badValueCount);
                  void* pRow = da->getRow();
                  VERIFYRV(pRow != NULL, badValueCount);
                  badValueCount += RasterUtilities::sanitizeData(pRow, elementCount, dataType, value);
                  da->nextRow();
               }
            }
         }
      }
   }

   if (badValueCount != 0)
   {
      updateData();
   }

   return badValueCount;
}

void RasterElementImp::setTerrain(RasterElement* pTerrain)
{
   if (pTerrain != mpTerrain.get())
   {
      mpTerrain.reset(pTerrain);
      notify(SIGNAL_NAME(RasterElement, TerrainSet), boost::any(mpTerrain.get()));
   }
}

const RasterElement* RasterElementImp::getTerrain() const
{
   return mpTerrain.get();
}

Statistics* RasterElementImp::getStatistics(DimensionDescriptor band) const
{
   if (band.isValid() == false)
   {
      const RasterDataDescriptor* pDescriptor = dynamic_cast<const RasterDataDescriptor*>(getDataDescriptor());
      VERIFYRV(pDescriptor != NULL, NULL);
      band = pDescriptor->getActiveBand(0);
   }

   map<DimensionDescriptor, StatisticsImp*>::const_iterator iter = mStatistics.find(band);
   if (iter != mStatistics.end())
   {
      Statistics* pStatistics = iter->second;
      return pStatistics;
   }

   return NULL;
}


bool RasterElementImp::toXml(XMLWriter* pXml) const
{
   // Cannot be represented in XML format
   return false;
}

bool RasterElementImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   // Cannot be represented in XML format
   return false;
}

const string& RasterElementImp::getObjectType() const
{
   static string sType("RasterElementImp");
   return sType;
}

bool RasterElementImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "RasterElement"))
   {
      return true;
   }

   return DataElementImp::isKindOf(className);
}

bool RasterElementImp::isKindOfElement(const string& className)
{
   if ((className == "RasterElementImp") || (className == "RasterElement"))
   {
      return true;
   }

   return DataElementImp::isKindOfElement(className);
}

void RasterElementImp::getElementTypes(vector<string>& classList)
{
   classList.push_back("RasterElement");
   DataElementImp::getElementTypes(classList);
}

void RasterElementImp::Deleter::operator()(DataAccessorImpl* pDataAccessor)
{
   delete pDataAccessor;
   delete this;
}

bool RasterElementImp::updateDims(const vector<DimensionDescriptor>& srcDims,
                                  const vector<DimensionDescriptor>& selectedDims,
                                  vector<DimensionDescriptor>& chipActiveDims,
                                  vector<DimensionDescriptor>& chipOnDiskDims)
{
   chipActiveDims.clear();
   chipOnDiskDims.clear();
   unsigned int i = 0;
   vector<DimensionDescriptor>::const_iterator selectedDim = selectedDims.begin();
   for (vector<DimensionDescriptor>::const_iterator srcDim = srcDims.begin();
      srcDim != srcDims.end();
      ++srcDim)
   {
      DimensionDescriptor newDimObj;
      DimensionDescriptor srcDimObj = *srcDim;
      newDimObj = srcDimObj;

      newDimObj.setActiveNumberValid(false);
      if (selectedDim != selectedDims.end())
      {
         DimensionDescriptor selectedDimObj = *selectedDim;
         if (srcDimObj.isActiveNumberValid() &&
            selectedDimObj.getActiveNumber() == srcDimObj.getActiveNumber()) 
         {
            newDimObj.setActiveNumber(i);
            chipActiveDims.push_back(newDimObj);
            ++i;
            ++selectedDim;
         }
      }
      if (newDimObj.isOnDiskNumberValid())
      {
         chipOnDiskDims.push_back(newDimObj);
      }
   }

   return true;
}

RasterElement *RasterElementImp::createChip(DataElement *pParent, const string &appendName, 
   const vector<DimensionDescriptor> &selectedRows,
   const vector<DimensionDescriptor> &selectedColumns,
   const vector<DimensionDescriptor> &selectedBands) const
{
   // Create a data descriptor based on the sensor data with the chip rows and columns
   string name = appendToBasename(getName(), appendName);
   return createChipInternal(pParent, name, selectedRows, selectedColumns, selectedBands);
}

RasterElement* RasterElementImp::createChipInternal(DataElement* pParent, const string& name,
                                                    const vector<DimensionDescriptor>& selectedRows,
                                                    const vector<DimensionDescriptor>& selectedColumns,
                                                    const vector<DimensionDescriptor>& selectedBands,
                                                    bool copyRasterData) const
{
   const RasterDataDescriptorImp* pDescriptor = dynamic_cast<const RasterDataDescriptorImp*>(getDataDescriptor());
   VERIFYRV(pDescriptor != NULL, NULL);

   DataDescriptor* pNewDescriptor1 = pDescriptor->copy(name, pParent);

   auto_ptr<RasterDataDescriptorImp> pNewDescriptor(dynamic_cast<RasterDataDescriptorImp*>(pNewDescriptor1));
   VERIFYRV(pNewDescriptor.get() != NULL, NULL);

   if (pNewDescriptor->getProcessingLocation() == ON_DISK_READ_ONLY)
   {
      pNewDescriptor->setProcessingLocation(ON_DISK);
   }
   const RasterFileDescriptorImp* pFileDescriptorImp = dynamic_cast<const RasterFileDescriptorImp*>(
      pDescriptor->getFileDescriptor());
   RasterFileDescriptorImp* pNewFileDescriptorImp = NULL;
   if (pFileDescriptorImp != NULL)
   {
      pNewFileDescriptorImp = dynamic_cast<RasterFileDescriptorImp*>(pNewDescriptor->getFileDescriptor());
   }
   pNewDescriptor->setName(name);
   
   // Create new DimensionDescriptors
   const vector<DimensionDescriptor>* pSelectedRows = &selectedRows;
   if (selectedRows.empty())
   {
      pSelectedRows = &pDescriptor->getRows();
   }
   const vector<DimensionDescriptor>* pSelectedCols = &selectedColumns;
   if (selectedColumns.empty())
   {
      pSelectedCols = &pDescriptor->getColumns();
   }
   const vector<DimensionDescriptor>* pSelectedBands = &selectedBands;
   if (selectedBands.empty())
   {
      pSelectedBands = &pDescriptor->getBands();
   }
      
   const vector<DimensionDescriptor>* pSrcRows = &pDescriptor->getRows();
   const vector<DimensionDescriptor>* pSrcCols = &pDescriptor->getColumns();
   const vector<DimensionDescriptor>* pSrcBands = &pDescriptor->getBands();
   if (pFileDescriptorImp != NULL)
   {
      pSrcRows = &pFileDescriptorImp->getRows();
      pSrcCols = &pFileDescriptorImp->getColumns();
      pSrcBands = &pFileDescriptorImp->getBands();
   }

   vector<DimensionDescriptor> chipActiveRows;
   vector<DimensionDescriptor> chipOnDiskRows;
   updateDims(*pSrcRows, *pSelectedRows, chipActiveRows, chipOnDiskRows);
   VERIFYRV(!chipActiveRows.empty(), NULL);

   vector<DimensionDescriptor> chipActiveCols;
   vector<DimensionDescriptor> chipOnDiskCols;
   updateDims(*pSrcCols, *pSelectedCols, chipActiveCols, chipOnDiskCols);
   VERIFYRV(!chipActiveCols.empty(), NULL);


   vector<DimensionDescriptor> chipActiveBands;
   vector<DimensionDescriptor> chipOnDiskBands;
   updateDims(*pSrcBands, *pSelectedBands, chipActiveBands, chipOnDiskBands);
   VERIFYRV(!chipActiveBands.empty(), NULL);

   pNewDescriptor->setRows(chipActiveRows);
   pNewDescriptor->setColumns(chipActiveCols);
   pNewDescriptor->setBands(chipActiveBands);

   vector<RasterChannelType> colors;
   colors.push_back(GRAY);
   colors.push_back(RED);
   colors.push_back(GREEN);
   colors.push_back(BLUE);
   for (vector<RasterChannelType>::iterator itr = colors.begin(); itr != colors.end(); ++itr)
   {
      DimensionDescriptor display = pDescriptor->getDisplayBand(*itr);
      if (display.isValid())
      {
         display = pNewDescriptor->getOriginalBand(display.getOriginalNumber());
      }
      pNewDescriptor->setDisplayBand(*itr, display);
   }

   if (pNewFileDescriptorImp != NULL)
   {
      VERIFYRV(!chipOnDiskRows.empty(), NULL);
      VERIFYRV(!chipOnDiskCols.empty(), NULL);
      VERIFYRV(!chipOnDiskBands.empty(), NULL);
      
      pNewFileDescriptorImp->setRows(chipOnDiskRows);
      pNewFileDescriptorImp->setColumns(chipOnDiskCols);
      pNewFileDescriptorImp->setBands(chipOnDiskBands);
   }

   ModelResource<RasterElement> pRasterChip(dynamic_cast<RasterDataDescriptor*>(pNewDescriptor.release()));
   VERIFYRV(pRasterChip.get() != NULL, NULL);
   const RasterDataDescriptor* pChipDescriptor = dynamic_cast<RasterDataDescriptor*>(
      pRasterChip->getDataDescriptor());
   VERIFYRV(pChipDescriptor != NULL, NULL);

   pRasterChip->createDefaultPager();

   bool abort = false;
   VERIFYRV(RasterUtilities::chipMetadata(pRasterChip->getMetadata(), *pSelectedRows, *pSelectedCols,
      *pSelectedBands), NULL);

   if (copyRasterData)
   {
      VERIFYRV(copyDataToChip(pRasterChip.get(), *pSelectedRows, *pSelectedCols, *pSelectedBands, abort), NULL);
   }

   return pRasterChip.release();
}

string RasterElementImp::appendToBasename(const string &name,
                                               const string &append)
{
   string appendedName = name;
   string::size_type loc = name.rfind('.');
   if (loc == string::npos)
   {
      appendedName.append(append);
   }
   else
   {
      appendedName.insert(loc, append);
   }
   return appendedName;
}

bool RasterElementImp::copyDataToChip(RasterElement *pRasterChip, 
   const vector<DimensionDescriptor> &selectedRows,
   const vector<DimensionDescriptor> &selectedColumns,
   const vector<DimensionDescriptor> &selectedBands,
   bool &abort, Progress *pProgress) const
{
   StatusBarProgress statusBarProgress;
   if (pProgress == NULL)
   {
      pProgress = &statusBarProgress;
   }

   VERIFY(pRasterChip != NULL);
   RasterDataDescriptor* pDescriptorChip = dynamic_cast<RasterDataDescriptor*>(pRasterChip->getDataDescriptor());
   VERIFY(pDescriptorChip != NULL);

   bool success = false;

   switch (pDescriptorChip->getInterleaveFormat())
   {
   case BIP:
      success = copyDataBip(pRasterChip, selectedRows, selectedColumns, selectedBands, abort, pProgress);
      break;
   case BSQ: // fall through
   case BIL:
      success = copyDataBsq(pRasterChip, selectedRows, selectedColumns, selectedBands, abort, pProgress);
      break;
   default:
      break;
   }

   return success;
}


bool RasterElementImp::copyDataBip(RasterElement* pChipElement, const vector<DimensionDescriptor>& selectedRows,
                                   const vector<DimensionDescriptor>& selectedColumns,
                                   const vector<DimensionDescriptor>& selectedBands, bool& abort,
                                   Progress* pProgress) const
{
   VERIFY(pProgress != NULL);
   string progressText = "Copying data";

   const RasterDataDescriptor* pSrcDd = dynamic_cast<const RasterDataDescriptor*>(getDataDescriptor());
   VERIFY(pSrcDd != NULL);

   const vector<DimensionDescriptor>& srcActiveRows = pSrcDd->getRows();
   const vector<DimensionDescriptor>& srcActiveCols = pSrcDd->getColumns();
   const vector<DimensionDescriptor>& srcActiveBands = pSrcDd->getBands();

   int bytesPerElement = pSrcDd->getBytesPerElement();

   bool copyCompleted = false;

   int rowIndex = 0;
   if (selectedBands.size() == srcActiveBands.size())
   {
      // all bands
      if (selectedColumns.size() == 
         selectedColumns.back().getActiveNumber() - selectedColumns.front().getActiveNumber() + 1)
      {
         unsigned int startRow = selectedRows.front().getActiveNumber();
         unsigned int stopRow = selectedRows.back().getActiveNumber();
         unsigned int startCol = selectedColumns.front().getActiveNumber();
         unsigned int stopCol = selectedColumns.back().getActiveNumber();

         // full contiguous row at a time
         FactoryResource<DataRequest> pSrcRequest;
         pSrcRequest->setInterleaveFormat(BIP);
         pSrcRequest->setRows(selectedRows.front(), selectedRows.back());
         pSrcRequest->setColumns(selectedColumns.front(), selectedColumns.back());
         DataAccessor srcDa = getDataAccessor(pSrcRequest.release());

         FactoryResource<DataRequest> pChipRequest;
         pChipRequest->setWritable(true);
         pChipRequest->setInterleaveFormat(BIP);
         DataAccessor chipDa = pChipElement->getDataAccessor(pChipRequest.release());

         VERIFY(chipDa.isValid() && srcDa.isValid());
         for (vector<DimensionDescriptor>::const_iterator rowIter = selectedRows.begin();
            rowIter != selectedRows.end();
            ++rowIter)
         {
            DimensionDescriptor rowDim = *rowIter;
            srcDa->toPixel(rowDim.getActiveNumber(), startCol);
            VERIFY(srcDa.isValid() && chipDa.isValid());
            char* pChip = reinterpret_cast<char*>(chipDa->getRow());
            char* pSrc = reinterpret_cast<char*>(srcDa->getRow());
            memcpy(pChip, pSrc, bytesPerElement * selectedBands.size() * selectedColumns.size());
            chipDa->nextRow();

            pProgress->updateProgress(progressText, (rowIndex++ * 100) / selectedRows.size(), NORMAL);
            if (abort)
            {
               return false;
            }
         }
         copyCompleted = true;
      }
      else
      {
         // one full pixel at a time
         FactoryResource<DataRequest> pSrcRequest;
         pSrcRequest->setInterleaveFormat(BIP);
         DataAccessor srcDa = getDataAccessor(pSrcRequest.release());
         
         FactoryResource<DataRequest> pChipRequest;
         pChipRequest->setWritable(true);
         pChipRequest->setInterleaveFormat(BIP);
         DataAccessor chipDa = pChipElement->getDataAccessor(pChipRequest.release());

         VERIFY(chipDa.isValid() && srcDa.isValid());
         for (vector<DimensionDescriptor>::const_iterator rowIter = selectedRows.begin();
            rowIter != selectedRows.end();
            ++rowIter)
         {
            VERIFY(chipDa.isValid());
            char* pChip = reinterpret_cast<char*>(chipDa->getRow());
            unsigned int rowOffsetChip = 0;
            for (vector<DimensionDescriptor>::const_iterator colIter = selectedColumns.begin();
               colIter != selectedColumns.end();
               ++colIter)
            {
               DimensionDescriptor rowDim = *rowIter;
               DimensionDescriptor colDim = *colIter;

               srcDa->toPixel(rowDim.getActiveNumber(), colDim.getActiveNumber());
               VERIFY(srcDa.isValid());
               char* pSrc = reinterpret_cast<char*>(srcDa->getColumn());
               memcpy(pChip + rowOffsetChip, pSrc, bytesPerElement * selectedBands.size());
               rowOffsetChip += bytesPerElement * selectedBands.size();
            }
            chipDa->nextRow();
            pProgress->updateProgress(progressText, (rowIndex++ * 100)/selectedRows.size(), NORMAL);
            if (abort)
            {
               return false;
            }
         }
         copyCompleted = true;
      }
   }

   if (!copyCompleted)
   {
      // slowest possible copy, per pixel, but it works for any BIP data
      FactoryResource<DataRequest> pSrcRequest;
      pSrcRequest->setInterleaveFormat(BIP);
      DataAccessor srcDa = getDataAccessor(pSrcRequest.release());

      FactoryResource<DataRequest> pChipRequest;
      pChipRequest->setWritable(true);
      pChipRequest->setInterleaveFormat(BIP);
      DataAccessor chipDa = pChipElement->getDataAccessor(pChipRequest.release());

      for (vector<DimensionDescriptor>::const_iterator rowIter = selectedRows.begin();
         rowIter != selectedRows.end();
         ++rowIter)
      {
         VERIFY(chipDa.isValid());
         char* pChip = reinterpret_cast<char*>(chipDa->getRow());
         unsigned int chipOffset = 0;
         for (vector<DimensionDescriptor>::const_iterator colIter = selectedColumns.begin();
            colIter != selectedColumns.end();
            ++colIter)
         {
            DimensionDescriptor rowDim = *rowIter;
            DimensionDescriptor colDim = *colIter;

            srcDa->toPixel(rowDim.getActiveNumber(), colDim.getActiveNumber());
            VERIFY(srcDa.isValid());
            char* pSrc = reinterpret_cast<char*>(srcDa->getColumn());
            for (vector<DimensionDescriptor>::const_iterator bandIter = selectedBands.begin();
               bandIter != selectedBands.end();
               ++bandIter)
            {
               DimensionDescriptor bandDim = *bandIter;

               memcpy(pChip+chipOffset, pSrc + bytesPerElement * (bandDim.getActiveNumber()), 
                  bytesPerElement);
               chipOffset += bytesPerElement;
            }

         }
         chipDa->nextRow();
         pProgress->updateProgress(progressText, (rowIndex++ * 100)/selectedRows.size(), NORMAL);
         if (abort)
         {
            return false;
         }
      }
      copyCompleted = true;

   }
   VERIFY(copyCompleted);

   return true;
}

bool RasterElementImp::copyDataBsq(RasterElement* pChipElement, const vector<DimensionDescriptor>& selectedRows,
                                   const vector<DimensionDescriptor>& selectedColumns,
                                   const vector<DimensionDescriptor>& selectedBands, bool& abort,
                                   Progress* pProgress) const
{
   VERIFY(pChipElement != NULL);
   VERIFY(pProgress != NULL);
   string progressText = "Copying data";

   const RasterDataDescriptor* pSrcDd = dynamic_cast<const RasterDataDescriptor*>(getDataDescriptor());
   VERIFY(pSrcDd != NULL);
   const RasterDataDescriptor* pChipDd = dynamic_cast<const RasterDataDescriptor*>(pChipElement->getDataDescriptor());
   VERIFY(pChipDd != NULL);

   const vector<DimensionDescriptor>& srcActiveRows = pSrcDd->getRows();
   const vector<DimensionDescriptor>& srcActiveCols = pSrcDd->getColumns();
   const vector<DimensionDescriptor>& srcActiveBands = pSrcDd->getBands();

   int bytesPerElement = pSrcDd->getBytesPerElement();

   bool copyCompleted = false;

   if (selectedColumns.size() == 
      selectedColumns.back().getActiveNumber() - selectedColumns.front().getActiveNumber() + 1)
   {
      unsigned int chipBand = 0;
      unsigned int step = 0;
      unsigned int steps = selectedRows.size() * selectedBands.size();
      for (vector<DimensionDescriptor>::const_iterator bandItr = selectedBands.begin();
         bandItr != selectedBands.end(); ++bandItr)
      {
         DimensionDescriptor bandDim = *bandItr;
         DimensionDescriptor startRowDim = selectedRows.front();
         DimensionDescriptor stopRowDim = selectedRows.back();
         DimensionDescriptor startColDim = selectedColumns.front();
         DimensionDescriptor stopColDim = selectedColumns.back();

         // copy a full row at a time
         FactoryResource<DataRequest> pSrcRequest;
         pSrcRequest->setInterleaveFormat(BSQ);
         pSrcRequest->setRows(startRowDim, stopRowDim);
         pSrcRequest->setColumns(startColDim, stopColDim);
         pSrcRequest->setBands(bandDim, bandDim);
         DataAccessor srcDa = getDataAccessor(pSrcRequest.release());

         FactoryResource<DataRequest> pChipRequest;
         pChipRequest->setInterleaveFormat(BSQ);
         pChipRequest->setWritable(true);
         pChipRequest->setBands(pChipDd->getActiveBand(chipBand), pChipDd->getActiveBand(chipBand));
         DataAccessor chipDa = pChipElement->getDataAccessor(pChipRequest.release());

         VERIFY(chipDa.isValid() && srcDa.isValid());
         for (vector<DimensionDescriptor>::const_iterator rowItr = selectedRows.begin();
            rowItr != selectedRows.end(); ++rowItr)
         {
            srcDa->toPixel(rowItr->getActiveNumber(), 0);
            VERIFY(chipDa.isValid());
            VERIFY(srcDa.isValid());
            char* pChip = reinterpret_cast<char*>(chipDa->getRow());
            char* pSrc = reinterpret_cast<char*>(srcDa->getRow());
            memcpy(pChip, pSrc, selectedColumns.size() * bytesPerElement);
            chipDa->nextRow();
            pProgress->updateProgress(progressText, (step++ * 100)/steps, NORMAL);
            if (abort)
            {
               return false;
            }
         }
         ++chipBand;
      }
      copyCompleted = true;
   }
   else
   {
      unsigned int chipBand = 0;
      unsigned int step = 0;
      unsigned int steps = selectedBands.size() * selectedRows.size();
      for (vector<DimensionDescriptor>::const_iterator bandItr = selectedBands.begin();
         bandItr != selectedBands.end(); ++bandItr)
      {
         DimensionDescriptor bandDim = *bandItr;

         // slowest copy, one pixel at a time
         FactoryResource<DataRequest> pSrcRequest;
         pSrcRequest->setInterleaveFormat(BSQ);
         pSrcRequest->setBands(bandDim, bandDim);
         DataAccessor srcDa = getDataAccessor(pSrcRequest.release());

         FactoryResource<DataRequest> pChipRequest;
         pChipRequest->setInterleaveFormat(BSQ);
         pChipRequest->setWritable(true);
         pChipRequest->setBands(pChipDd->getActiveBand(chipBand), pChipDd->getActiveBand(chipBand));
         DataAccessor chipDa = pChipElement->getDataAccessor(pChipRequest.release());

         VERIFY(chipDa.isValid() && srcDa.isValid());
         for (vector<DimensionDescriptor>::const_iterator rowItr = selectedRows.begin();
            rowItr != selectedRows.end(); ++rowItr)
         {
            DimensionDescriptor rowDim = *rowItr;

            VERIFY(chipDa.isValid());
            char* pChip = reinterpret_cast<char*>(chipDa->getRow());
            unsigned int chipOffset = 0;
            for (vector<DimensionDescriptor>::const_iterator colItr = selectedColumns.begin();
               colItr != selectedColumns.end(); ++colItr)
            {
               DimensionDescriptor colDim = *colItr;

               srcDa->toPixel(rowDim.getActiveNumber(), colDim.getActiveNumber());
               VERIFY(srcDa.isValid());
               char* pOrig = reinterpret_cast<char*>(srcDa->getColumn());
               memcpy(pChip + chipOffset, pOrig, bytesPerElement);
               chipOffset += bytesPerElement;
            }
            chipDa->nextRow();
            pProgress->updateProgress(progressText, (step++ * 100)/steps, NORMAL);
            if (abort)
            {
               return false;
            }
         }
         ++chipBand;
      }
      copyCompleted = true;


   }
   return copyCompleted;

   return false;
}

DataElement* RasterElementImp::copy(const string& name, DataElement* pParent) const
{
   vector<DimensionDescriptor> dims;
   return createChipInternal(pParent, name, dims, dims, dims);
}

RasterElement* RasterElementImp::copyShallow(const string& name, DataElement* pParent) const
{
   vector<DimensionDescriptor> dims;
   return createChipInternal(pParent, name, dims, dims, dims, false);
}

RasterElementImp::StatusBarProgress::StatusBarProgress() : mText(), mPercent(0), mGranularity(NORMAL)
{
}

void RasterElementImp::StatusBarProgress::getProgress(string &text, int &percent, ReportingLevel &gran) const
{
   text = mText;
   percent = mPercent;
   gran = mGranularity;
}

void RasterElementImp::StatusBarProgress::updateProgress(const char *pText, int percent, ReportingLevel gran)
{
   mText = pText;
   mPercent = percent;
   mGranularity = gran;

   string message = mText;
   if (gran == NORMAL)
   {
      message += ": " + boost::lexical_cast<string>(percent) + "%";
   }

   Service<DesktopServices>()->setStatusBarMessage(message);
}

bool RasterElementImp::createTemporaryFile()
{
   if (mTempFilename.empty() == false)
   {
      remove(mTempFilename.c_str());
      mTempFilename.erase();
   }

   const RasterDataDescriptor* pDescriptor = dynamic_cast<const RasterDataDescriptor*>(getDataDescriptor());
   if (pDescriptor == NULL)
   {
      return false;
   }

   unsigned int rows = pDescriptor->getRowCount();
   unsigned int columns = pDescriptor->getColumnCount();
   unsigned int bands = pDescriptor->getBandCount();
   unsigned int size = pDescriptor->getBytesPerElement();

   const Filename* pTempPath = ConfigurationSettings::getSettingTempPath();
   string tempPath;
   if (pTempPath != NULL)
   {
      tempPath = pTempPath->getFullPathAndName();
   }

   char* pTempFilename = tempnam(tempPath.c_str(), "RE");
   if (pTempFilename == NULL)
   {
      return false;
   }
   mTempFilename = pTempFilename;
   free(pTempFilename);

   LargeFileResource tempFile(false);
   if (!tempFile.open(mTempFilename, O_RDWR | O_CREAT | O_BINARY, S_IREAD | S_IWRITE | S_IEXEC))
   {
      return false;
   }

   uint64_t totalSize = static_cast<uint64_t>(rows) * columns * bands * size;

#if defined(UNIX_API)
   struct statvfs sbuf;
   if (statvfs(mTempFilename.c_str(), &sbuf) ==0)
   {
      // Be sure there is enough space in the file system
      if ((totalSize + sbuf.f_bsize - 1) / sbuf.f_bsize < sbuf.f_bfree)
      {
#endif
         // seek here
         if (tempFile.seek(totalSize - 1, SEEK_SET) != (totalSize - 1) || // seek failed, OR
            tempFile.write(mTempFilename.c_str(), 1) != 1)  // write failed
         {
            return false;
         }
#if defined(UNIX_API)
      }
   }
#endif

   tempFile.close();

   return createMemoryMappedPager(true);
}

bool RasterElementImp::createMemoryMappedPager()
{
   return createMemoryMappedPager(false);
}

bool RasterElementImp::setPager(RasterPager* pPager)
{
   //if you pass in the same RasterPage pointer, twice in a row
   //it does the assign only once, and return true for every time.
   //if you pass in a null value, it will destroy the existing
   //plugins that it is holding onto, then it will assign
   //the null values into it's member variables.
   //if you pass in a new RasterPager* pointer, it will destroy
   //any existing plugins first, before holding onto
   //the new plugin pointers.
   if (pPager == mpPager)
   {
      return true;
   }

   if (pPager == NULL)
   {
      return false;
   }

   if (mpPager != NULL)
   {
      //destroy the old plugins first
      Service<PlugInManagerServices> pServices;
      pServices->destroyPlugIn(dynamic_cast<PlugIn*>(mpPager));
   }

   //re-assign the pointers to hold onto the new plug-ins.
   mpPager = pPager;

   return true;
}

RasterPager* RasterElementImp::getPager() const
{
   return mpPager;
}

const string& RasterElementImp::getTemporaryFilename() const
{
   return mTempFilename;
}

bool RasterElementImp::serialize(SessionItemSerializer& serializer) const
{
   const RasterDataDescriptor* pDescriptor = dynamic_cast<const RasterDataDescriptor*>(getDataDescriptor());
   VERIFY(pDescriptor);

   XMLWriter xml(getObjectType().c_str());
   xml.addAttr("displayName", getDisplayName());
   DataElement* pParent = getParent();
   if (pParent)
   {
      xml.addAttr("parentId", pParent->getId());
   }
   xml.addText(getDisplayText(), xml.addElement("DisplayText"));
   xml.pushAddPoint(xml.addElement("DataDescriptor"));
   if (!pDescriptor->toXml(&xml))
   {
      return false;
   }
   xml.popAddPoint();
   SessionItem* pSessionGeoPlugin = dynamic_cast<SessionItem*>(mpGeoPlugin);
   if (pSessionGeoPlugin)
   {
      xml.addAttr("geoPlugin", pSessionGeoPlugin->getId());
   }

   // statistics
   for (map<DimensionDescriptor, StatisticsImp*>::const_iterator stat = mStatistics.begin();
               stat != mStatistics.end(); ++stat)
   {
      xml.pushAddPoint(xml.addElement("statistics"));
      xml.addAttr("band", stat->first.getActiveNumber());
      if (!stat->second->toXml(&xml))
      {
         return false;
      }
      xml.popAddPoint();
   }
   if (!serializer.serialize(xml))
   {
      return false;
   }

   if (mModified || pDescriptor->getFileDescriptor() == NULL)
   {
#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : Modify this to only save when necessary (tclarke)")
      //mModified = false;

      // serialize the cube
      serializer.endBlock();
      int64_t datasetSize = static_cast<int64_t>(pDescriptor->getRowCount()) *
         pDescriptor->getColumnCount() *
         pDescriptor->getBandCount() *
         pDescriptor->getBytesPerElement();
      serializer.reserve(datasetSize);

      // if the entire thing is contiguous so use a single serialize
      const void* pRawData = getRawData();
      if (pRawData != NULL)
      {
         return serializer.serialize(pRawData, datasetSize);
      }

      // write out all the data a row at a time
      unsigned int totalOuterBands = (pDescriptor->getInterleaveFormat() == BSQ) ? pDescriptor->getBandCount() : 1;
      for (unsigned int outerBand = 0; outerBand < totalOuterBands; ++outerBand)
      {
         // Get a data accessor with an entire concurrent row
         FactoryResource<DataRequest> pRequest;
#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : fix this when there's a getNextBand() (tclarke)")
         // since there's not getNextBand() we need to request only 1 band for BSQ
         if (pDescriptor->getInterleaveFormat() == BSQ)
         {
            pRequest->setBands(pDescriptor->getActiveBand(outerBand), pDescriptor->getActiveBand(outerBand), 1);
         }
         DataAccessor acc = getDataAccessor(pRequest.release());
         for (unsigned int row = 0; row < pDescriptor->getRowCount(); ++row)
         {
            if (!acc.isValid() || !serializer.serialize(acc->getRow(), acc->getRowSize()))
            {
               return false;
            }
            acc->nextRow();
         }
      }
   }
   return true;
}

bool RasterElementImp::deserialize(SessionItemDeserializer& deserializer)
{
   RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(getDataDescriptor());
   VERIFY(pDescriptor);

   XmlReader reader(NULL, false);
   DOMElement* pRoot = deserializer.deserialize(reader, getObjectType().c_str());
   if (pRoot == NULL)
   {
      return false;
   }
   try
   {
      string parentId = A(pRoot->getAttribute(X("parentId")));
      if (parentId.empty() == false)
      {
         DataElement* pParent = dynamic_cast<DataElement*>(Service<SessionManager>()->getSessionItem(parentId));
         if (pParent)
         {
            Service<ModelServices>()->setElementParent(dynamic_cast<DataElement*>(this), pParent);
         }
      }
      setDisplayName(A(pRoot->getAttribute(X("displayName"))));
      mStatistics.clear();
      const RasterDataDescriptorImp* pDataDesc = static_cast<RasterDataDescriptorImp*>(getDataDescriptor());
      for (DOMNode *pNode = pRoot->getFirstChild(); pNode != NULL; pNode = pNode->getNextSibling())
      {
         if (XMLString::equals(pNode->getNodeName(), X("DataDescriptor")))
         {
            Service<ModelServices>()->setElementName(dynamic_cast<DataElement*>(this),
               A(static_cast<DOMElement*>(pNode)->getAttribute(X("name"))));
            if (!pDescriptor->fromXml(pNode, XmlBase::VERSION))
            {
               return false;
            }
         }
         else if (XMLString::equals(pNode->getNodeName(), X("DisplayText")))
         {
            setDisplayText(A(pNode->getTextContent()));
         }
         else if (XMLString::equals(pNode->getNodeName(), X("statistics")))
         {
            DOMElement* pStat = static_cast<DOMElement*>(pNode);
            unsigned int band = StringUtilities::fromXmlString<unsigned int>(
               A(pStat->getAttribute(X("band"))));
            DimensionDescriptor bandDesc = pDataDesc->getActiveBand(band);
            StatisticsImp* pStatistics = new StatisticsImp(this, bandDesc);
            if (pStatistics == NULL || !pStatistics->fromXml(pStat, XmlBase::VERSION))
            {
               return false;
            }
            mStatistics[bandDesc] = pStatistics;
         }
      }

      if (deserializer.getBlockSizes().size() > 1)
      {
         int64_t cubeSize = deserializer.getBlockSizes()[1];
         deserializer.nextBlock();

         if (!createDefaultPager())
         {
            // should never have on-disk read-only data saved to the session
            return false;
         }
         unsigned int totalOuterBands = (pDescriptor->getInterleaveFormat() == BSQ) ? pDescriptor->getBandCount() : 1;
         for (unsigned int outerBand = 0; outerBand < totalOuterBands; ++outerBand)
         {
            // Get a data accessor with an entire concurrent row
            FactoryResource<DataRequest> pRequest;
#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : fix this when there's a getNextBand() (tclarke)")
            // since there's not getNextBand() we need to request only 1 band for BSQ
            if (pDescriptor->getInterleaveFormat() == BSQ)
            {
               pRequest->setBands(pDescriptor->getActiveBand(outerBand), pDescriptor->getActiveBand(outerBand), 1);
            }
            pRequest->setWritable(true);
            DataAccessor acc = getDataAccessor(pRequest.release());
            for (unsigned int row = 0; row < pDescriptor->getRowCount(); ++row)
            {
               if (!acc.isValid() || !deserializer.deserialize(acc->getRow(), acc->getRowSize()))
               {
                  return false;
               }
               acc->nextRow();
            }
         }
      }
      else
      {
         // use the original importer (if available) to import the cube
         string importerName;
         DataDescriptorImp* pDdi = dynamic_cast<DataDescriptorImp*>(pDescriptor);
         if (pDdi != NULL)
         {
            importerName = pDdi->getImporterName();
         }
         if (importerName.empty())
         {
            importerName = "Auto Importer";
         }
         ExecutableResource importer(importerName);
         bool dummy = true;
         importer->getInArgList().setPlugInArgValue(Importer::SessionLoadArg(), &dummy);
         importer->getInArgList().setPlugInArgValueLoose(Importer::ImportElementArg(),
            dynamic_cast<DataElement*>(this));
         if (!importer->execute())
         {
            return false;
         }
      }

      // Restore the georeference plug-in last so that the georeference plug-in will not be destroyed
      // before it is restored if loading the data fails
      if (pRoot->hasAttribute(X("geoPlugin")))
      {
         Service<SessionManager> pManager;
         mpGeoPlugin = dynamic_cast<Georeference*>(pManager->getSessionItem(A(pRoot->getAttribute(X("geoPlugin")))));
      }
      else
      {
         mpGeoPlugin = NULL;
      }
   }
   catch (const XmlReader::DomParseException&)
   {
      return false;
   }
   return true;
}

void RasterElementImp::incrementDataAccessor(DataAccessorImpl& da)
{
   VERIFYNRV (da.mpRasterPager != NULL);
   VERIFYNRV (da.mpRasterPager != NULL);

   const RasterDataDescriptor* pDescriptor = dynamic_cast<const RasterDataDescriptor*>(getDataDescriptor());
   VERIFYNRV (pDescriptor != NULL);

   //release the previous page
   if (da.mpRasterPage != NULL)
   {
      da.mpRasterPager->releasePage(da.mpRasterPage);
   }

   //update the DataAccessor properties
   da.mAccessorRow += da.mCurrentRow;
   da.mCurrentRow = 0;
   da.mAccessorColumn = da.mpRequest->getStartColumn().getActiveNumber();
   da.mAccessorBand = da.mpRequest->getStartBand().getActiveNumber();

   //get a new raster page loaded into memory,
   //the only thing different from the previous page that we requested
   //should be the startRow.

   //request the same number of concurrentRows, cols, and bands
   //that we originally requested in the getDataAccessor()
   //call
   RasterPage* pPage = NULL;
   if (da.mAccessorRow < pDescriptor->getRowCount() &&
      da.mAccessorColumn < pDescriptor->getColumnCount() &&
      da.mAccessorBand < pDescriptor->getBandCount())
   {
      pPage = da.mpRasterPager->getPage(da.mpRequest.get(),
         pDescriptor->getActiveRow(da.mAccessorRow), 
         pDescriptor->getActiveColumn(da.mAccessorColumn), 
         pDescriptor->getActiveBand(da.mAccessorBand));
   }
   //set the validatily of the data accessor to be dependent on
   //the getPage returning a non-null value
   da.mbValid = pPage != NULL;

   if (da.isValid())
   {
      da.mpPage = reinterpret_cast<char*>(pPage->getRawData());
      //set the validity of the data accessor to be dependent on the seekTo
      //returning a non-null value
      da.mbValid = da.mpPage != NULL;
      if (da.isValid())
      {
         //update the number of concurrent rows
         //based on the amount rows available in
         //the block that was returned to us.
         unsigned int numBlockColumns = pPage->getNumColumns();
         unsigned int numBlockBands = pPage->getNumBands();
         unsigned int numBlockInterlineBytes = pPage->getInterlineBytes();
         if (numBlockColumns == 0)
         {
            numBlockColumns = pDescriptor->getColumnCount();
         }

         if (numBlockBands == 0)
         {
            numBlockBands = pDescriptor->getBandCount();
         }

         da.mConcurrentRows = pPage->getNumRows();
         da.mConcurrentColumns = numBlockColumns;
         da.mConcurrentBands = numBlockBands;
         da.updateDataSizes(pDescriptor->getBytesPerElement(), numBlockInterlineBytes);
      }
   }
   else
   {
      da.mpPage = NULL;
   }

   da.mpRasterPage = pPage;
}

DataAccessor RasterElementImp::getDataAccessor(DataRequest *pRequestIn) const
{
   if (pRequestIn != NULL)
   {
      if (pRequestIn->getWritable())
      {
         // can't get a writable accessor to a const RasterElement

         FactoryResource<DataRequest> pRequest(pRequestIn); // destroy the request
         return DataAccessor(NULL, NULL);
      }
   }

   return const_cast<RasterElementImp*>(this)->getDataAccessor(pRequestIn);
}

DataAccessor RasterElementImp::getDataAccessor(DataRequest* pRequestIn)
{
   if (pRequestIn == NULL)
   {
      FactoryResource<DataRequest> pRequestDefault;
      pRequestIn = pRequestDefault.release();
   }
   FactoryResource<DataRequest> pRequest(pRequestIn);

   DataAccessorImpl* pImpl = NULL;

   const RasterDataDescriptor* pDescriptor = dynamic_cast<const RasterDataDescriptor*>(getDataDescriptor());
   if (pDescriptor == NULL)
   {
      return DataAccessor(NULL, NULL);
   }

   pRequest->polish(pDescriptor);
   if (!pRequest->validate(pDescriptor))
   {
      return DataAccessor(NULL, NULL);
   }

   if (createDefaultPager() == false)
   {
      return DataAccessor(NULL, NULL);
   }

   unsigned int numRows = pDescriptor->getRowCount();
   unsigned int numColumns = pDescriptor->getColumnCount();
   unsigned int numBands = pDescriptor->getBandCount();
   unsigned int bytesPerElement = pDescriptor->getBytesPerElement();

   InterleaveFormatType sourceInterleave = pDescriptor->getInterleaveFormat();
   InterleaveFormatType interleave = pRequest->getInterleaveFormat();

   RasterPager* pPager = mpPager;
   if (interleave == BIP && (sourceInterleave == BSQ || sourceInterleave == BIL))
   {
      if (mpConverterPager == NULL)
      {
         mpConverterPager = new ConvertToBipPager(dynamic_cast<RasterElement*>(this));
      }
      pPager = mpConverterPager;
   }
   else if (interleave == BSQ && (sourceInterleave == BIP || sourceInterleave == BIL))
   {
      if (mpConverterPager == NULL)
      {
         mpConverterPager = new ConvertToBsqPager(dynamic_cast<RasterElement*>(this));
      }
      pPager = mpConverterPager;
   }
   else if ( interleave != sourceInterleave )
   {
      // Current conversions supported are BIP or BIL to BSQ and BSQ or BIL to BIP.
      return DataAccessor(NULL, NULL);
   }

   if (pPager == NULL)
   {
      return DataAccessor(NULL, NULL);
   }

   if (pPager->getSupportedRequestVersion() < pRequest->getRequestVersion(pDescriptor))
   {
      return DataAccessor(NULL, NULL);
   }

   //request that the data be mapped from the file on disk into memory.
   RasterPage* pPage = pPager->getPage(pRequest.get(), pRequest->getStartRow(), pRequest->getStartColumn(),
      pRequest->getStartBand());
   if (pPage != NULL)
   {
      //if we were successful, create a DataAccessorImpl
      char* pRawData = reinterpret_cast<char*>(pPage->getRawData());
      if (pRawData != NULL)
      {
         unsigned int numPageRows = pPage->getNumRows();
         unsigned int numPageColumns = pPage->getNumColumns();
         unsigned int numPageBands = pPage->getNumBands();
         unsigned int numPageInterlineBytes = pPage->getInterlineBytes();

         if (pPager == mpPager)
         {
            if (numPageColumns == 0)
            {
               //set the number of columns in the
               //page to be equal to the number
               //of columns in the cube
               //returned by the DataDescriptor
               //see RasterPage::getNumColumns() method.
               numPageColumns = numColumns;
            }

            if (numPageBands == 0)
            {
               //set the number of bands in the
               //page equal to the number of
               //bands in the cube returned by
               //the DataDescriptor.
               //see RasterPage::getNumBands() method.
               numPageBands = numBands;
            }
         }
         pImpl = new DataAccessorImpl(pRawData, pRequest.release(), 
            numPageRows, numPageInterlineBytes,
            numPageColumns, 
            numPageBands,
            bytesPerElement, dynamic_cast<RasterElement*>(this));

         pImpl->mpRasterPage = pPage;
         pImpl->mpRasterPager = pPager;
      }
   }

   DataAccessorDeleter* pDeleter = NULL;
   if (pImpl != NULL)
   {
      pDeleter = new RasterElementImp::Deleter;
   }

   //return the DataAccessor
   return DataAccessor(pDeleter, pImpl);
}

bool RasterElementImp::createMemoryMappedPager(bool bUseDataDescriptor)
{
   Service<PlugInManagerServices> pManager;

   PlugIn* pPlugIn = pManager->createPlugIn("MemoryMappedPager");
   if (pPlugIn == NULL)
   {
      return false;
   }

   bool success = false;

   Executable* pExecutable = dynamic_cast<Executable*>(pPlugIn);
   if (pExecutable != NULL)
   {
      FilenameImp* pFilename = NULL;
      bool isWritable = !mTempFilename.empty();

      // Input args
      PlugInArgList* pInArgs = NULL;
      success = pExecutable->getInputSpecification(pInArgs);
      if ((success == true) && (pInArgs != NULL))
      {
         PlugInArg* pArg = NULL;
         success = pInArgs->getArg("Raster Element", pArg);
         if ((success == true) && (pArg != NULL))
         {
            pArg->setActualValue(dynamic_cast<RasterElement*>(this));
         }

         success = pInArgs->getArg("Filename", pArg);
         if ((success == true) && (pArg != NULL))
         {
            string filename = mTempFilename;
            if (filename.empty() == true)
            {
               filename = getFilename();
            }

            pFilename = new FilenameImp(filename);
            pArg->setActualValue(pFilename);
         }

         success = pInArgs->getArg("isWritable", pArg);
         if ((success == true) && (pArg != NULL))
         {
            pArg->setActualValue(&isWritable);
         }

         success = pInArgs->getArg("Use Data Descriptor", pArg);
         if ((success == true) && (pArg != NULL))
         {
            pArg->setActualValue(&bUseDataDescriptor);
         }
      }

      // Output args
      PlugInArgList* pOutArgs = NULL;
      pExecutable->getOutputSpecification(pOutArgs);

      // Execute the plug-in
      success = pExecutable->execute(pInArgs, pOutArgs);

      // Clean-up
      delete pFilename;
      if (pInArgs != NULL)
      {
         pManager->destroyPlugInArgList(pInArgs);
      }

      if (pOutArgs != NULL)
      {
         pManager->destroyPlugInArgList(pOutArgs);
      }
   }

   RasterPager* pPager = dynamic_cast<RasterPager*>(pPlugIn);
   success = success && (pPager != NULL);

   if (success == true)
   {
      setPager(pPager);
   }
   else
   {
      pManager->destroyPlugIn(pPlugIn);
   }

   return success;
}

bool RasterElementImp::createInMemoryPager()
{
   ExecutableResource pPlugin("In Memory Pager");
   VERIFY(pPlugin->getPlugIn() != NULL);

   RasterPager* pPager = dynamic_cast<RasterPager*>(pPlugin->getPlugIn());
   VERIFY(pPager != NULL);

   void* pData = NULL;
   Service<ModelServices> pModel;
   const RasterDataDescriptorImp* pRasterDescriptor = dynamic_cast<const RasterDataDescriptorImp*>(getDataDescriptor());
   if (pRasterDescriptor != NULL)
   {
      uint64_t numRows = pRasterDescriptor->getRowCount();
      uint64_t numColumns = pRasterDescriptor->getColumnCount();
      uint64_t numBands = pRasterDescriptor->getBandCount();
      unsigned int bytesPerElement = pRasterDescriptor->getBytesPerElement();

      uint64_t dataSize = numRows * numColumns * numBands * bytesPerElement;
      if (dataSize <= numeric_limits<size_t>::max())
      {
         pData = pModel->getMemoryBlock(static_cast<size_t>(dataSize));
      }
   }

   if (pData == NULL)
   {
      return false;
   }

   VERIFY(pPlugin->getInArgList().setPlugInArgValue("Raster Element", dynamic_cast<RasterElement*>(this)));
   VERIFY(pPlugin->getInArgList().setPlugInArgValue("Memory", pData));

   VERIFY(pPlugin->execute());

   VERIFY(setPager(pPager));

   pPlugin->releasePlugIn();

   return true;
}

bool RasterElementImp::createDefaultPager()
{
   if (mpPager != NULL)
   {
      return true;
   }

   DataDescriptorImp* pDescriptor = getDataDescriptor();
   VERIFY(pDescriptor != NULL);

   switch (pDescriptor->getProcessingLocation())
   {
   case IN_MEMORY:
      return createInMemoryPager();
   case ON_DISK:
      return createTemporaryFile();
   case ON_DISK_READ_ONLY: // fall through
   default:
      return false;
   }

   return false;
}

const void* RasterElementImp::getRawData() const
{
   return const_cast<RasterElementImp*>(this)->getRawData();
}

void *RasterElementImp::getRawData()
{
   if (!mCubePointerAccessor.isValid())
   {
      const RasterDataDescriptor* pDescriptor = dynamic_cast<const RasterDataDescriptor*>(getDataDescriptor());
      VERIFYRV(pDescriptor != NULL, NULL);

      if (pDescriptor->getProcessingLocation() == IN_MEMORY)
      {
         unsigned int numRows = pDescriptor->getRowCount();
         mCubePointerAccessor = getDataAccessor();

         RasterPage* pPage = mCubePointerAccessor->mpRasterPage;
         if (pPage == NULL || numRows != pPage->getNumRows() || pPage->getInterlineBytes() != 0)
         {
            // this does not contain the full scene
            mCubePointerAccessor = DataAccessor(NULL, NULL);
         }
      }
   }

   if (mCubePointerAccessor.isValid())
   {
      return mCubePointerAccessor->getRow();
   }

   return NULL;
}

bool RasterElementImp::writeRawData(void* pData, InterleaveFormatType interleaveType,
   unsigned int startRow, unsigned int numRows, unsigned int startColumn, unsigned int numColumns,
   unsigned int startBand, unsigned int numBands)
{
   char* pSrc = reinterpret_cast<char*>(pData);
   RasterDataDescriptor* pDesc = dynamic_cast<RasterDataDescriptor*>(getDataDescriptor());
   if (pSrc == NULL || pDesc == NULL || interleaveType.isValid() == false)
   {
      return false;
   }

   unsigned int stopRow = startRow + numRows - 1;
   unsigned int stopColumn = startColumn + numColumns - 1;
   unsigned int stopBand = startBand + numBands - 1;
   if (stopRow < startRow || stopRow > pDesc->getRowCount() ||
      stopColumn < startColumn || stopColumn > pDesc->getColumnCount() ||
      stopBand < startBand || stopBand > pDesc->getBandCount())
   {
      return false;
   }

   unsigned int bytesPerElement = pDesc->getBytesPerElement();
   if (interleaveType == BSQ)
   {
      for (unsigned int band = 0; band < numBands; ++band)
      {
         FactoryResource<DataRequest> pRequest;
         pRequest->setInterleaveFormat(BSQ);
         pRequest->setRows(pDesc->getActiveRow(startRow), pDesc->getActiveRow(stopRow), 1);
         pRequest->setColumns(pDesc->getActiveColumn(startColumn), pDesc->getActiveColumn(stopColumn), numColumns);
         pRequest->setBands(pDesc->getActiveBand(startBand + band), pDesc->getActiveBand(startBand + band), 1);
         pRequest->setWritable(true);
         DataAccessor daImage = getDataAccessor(pRequest.release());
         VERIFY(daImage.isValid());

         for (unsigned int row = 0; row < numRows; ++row)
         {
            for (unsigned int column = 0; column < numColumns; ++column)
            {
               daImage->toPixel(startRow + row, startColumn + column);
               VERIFY(daImage.isValid());

               unsigned int offset = (numColumns * numRows * band * bytesPerElement) +
                  (numColumns * row * bytesPerElement) + (column * bytesPerElement);
               memcpy(daImage->getColumn(), pSrc + offset, bytesPerElement);
            }
         }
      }
   }
   else if (interleaveType == BIP)
   {
      FactoryResource<DataRequest> pRequest;
      pRequest->setInterleaveFormat(BIP);
      pRequest->setRows(pDesc->getActiveRow(startRow), pDesc->getActiveRow(stopRow), 1);
      pRequest->setColumns(pDesc->getActiveColumn(startColumn), pDesc->getActiveColumn(stopColumn), numColumns);
      pRequest->setWritable(true);
      DataAccessor daImage = getDataAccessor(pRequest.release());
      VERIFY(daImage.isValid());

      for (unsigned int row = 0; row < numRows; ++row)
      {
         for (unsigned int column = 0; column < numColumns; ++column)
         {
            daImage->toPixel(startRow + row, startColumn + column);
            VERIFY(daImage.isValid());

            for (unsigned int band = 0; band < numBands; ++band)
            {
               memcpy(static_cast<unsigned char*>(daImage->getColumn()) + ((band + startBand) * bytesPerElement),
                  pSrc + (row * numColumns * numBands * bytesPerElement) +
                  (column * numBands * bytesPerElement) + (band * bytesPerElement),
                  bytesPerElement);
            }
         }
      }
   }
   else if (interleaveType == BIL)
   {
      for (unsigned int row = 0; row < numRows; ++row)
      {
         for (unsigned int band = 0; band < numBands; ++band)
         {
            FactoryResource<DataRequest> pRequest;
            pRequest->setInterleaveFormat(BIL);
            pRequest->setRows(pDesc->getActiveRow(startRow + row), pDesc->getActiveRow(startRow + row), 1);
            pRequest->setColumns(pDesc->getActiveColumn(startColumn), pDesc->getActiveColumn(stopColumn), numColumns);
            pRequest->setBands(pDesc->getActiveBand(startBand + band), pDesc->getActiveBand(startBand + band), 1);
            pRequest->setWritable(true);
            DataAccessor daImage = getDataAccessor(pRequest.release());
            VERIFY(daImage.isValid());

            for (unsigned int column = 0; column < numColumns; ++column)
            {
               daImage->toPixel(startRow + row, startColumn + column);
               VERIFY(daImage.isValid());

               unsigned int offset = (row * numColumns * numBands * bytesPerElement) +
                  (band * numColumns * bytesPerElement) + (column * bytesPerElement);
               memcpy(daImage->getColumn(), pSrc + offset, bytesPerElement);
            }
         }
      }
   }

   return true;
}

LocationType RasterElementImp::convertPixelToGeocoord(LocationType pixel, bool quick, bool* pAccurate) const
{
   LocationType geocoord;

   if (pAccurate != NULL)
   {
      *pAccurate = false;
   }

   if (mpGeoPlugin != NULL)
   {
      if (!quick)
      {
         geocoord = mpGeoPlugin->pixelToGeo(pixel, pAccurate);
      }
      else
      {
         geocoord = mpGeoPlugin->pixelToGeoQuick(pixel, pAccurate);
      }
   }
   return geocoord;
}

vector<LocationType> RasterElementImp::convertPixelsToGeocoords(
   const vector<LocationType>& pixels, bool quick, bool* pAccurate) const
{
   vector<LocationType> geocoords;
   
   if (pAccurate != NULL)
   {
      bool bAccurate(false);
      *pAccurate = true;
      for (vector<LocationType>::const_iterator it = pixels.begin(); it != pixels.end(); ++it)
      {
         geocoords.push_back(convertPixelToGeocoord(*it, quick, &bAccurate));
         *pAccurate = *pAccurate && bAccurate;
      }
   }
   else
   {
      transform(pixels.begin(), pixels.end(), back_inserter(geocoords), 
         boost::bind(&RasterElementImp::convertPixelToGeocoord, this, _1, quick, pAccurate));
   }

   return geocoords;
}

LocationType RasterElementImp::convertGeocoordToPixel(LocationType geocoord, bool quick, bool* pAccurate) const
{
   LocationType pixel;

   if (pAccurate != NULL)
   {
      *pAccurate = false;
   }

   if (mpGeoPlugin != NULL)
   {
      if (!quick)
      {
         pixel = mpGeoPlugin->geoToPixel(geocoord, pAccurate);
      }
      else
      {
         pixel = mpGeoPlugin->geoToPixelQuick(geocoord, pAccurate);
      }
   }
   return pixel;
}

vector<LocationType> RasterElementImp::convertGeocoordsToPixels(
   const vector<LocationType>& geocoords, bool quick, bool* pAccurate) const
{
   vector<LocationType> pixels;

   if (pAccurate != NULL)
   {
      bool bAccurate(false);
      *pAccurate = true;
      for (vector<LocationType>::const_iterator it = geocoords.begin(); it != geocoords.end(); ++it)
      {
         pixels.push_back(convertGeocoordToPixel(*it, quick, &bAccurate));
         *pAccurate = *pAccurate && bAccurate;
      }
   }
   else
   {
      transform(geocoords.begin(), geocoords.end(), std::back_inserter(pixels), 
         boost::bind(&RasterElementImp::convertGeocoordToPixel, this, _1, quick, pAccurate));
   }

   return pixels;
}

bool RasterElementImp::isGeoreferenced() const
{
   return (getGeoreferencePlugin() != NULL);
}

void RasterElementImp::setGeoreferencePlugin(Georeference *pGeo)
{
   if (pGeo == NULL)
   {
      return;
   }

   if (pGeo == mpGeoPlugin)
   {
      return;
   }

   Service<PlugInManagerServices> pPluginManager;
   if (mpGeoPlugin != NULL)
   {
      pPluginManager->destroyPlugIn(dynamic_cast<PlugIn*>(mpGeoPlugin));
   }
   mpGeoPlugin = pGeo;
   notify(SIGNAL_NAME(RasterElement, GeoreferenceModified));
}

Georeference *RasterElementImp::getGeoreferencePlugin() const
{
   return mpGeoPlugin;
}

void RasterElementImp::updateGeoreferenceData()
{
   if (isGeoreferenced())
   {
      notify(SIGNAL_NAME(RasterElement, GeoreferenceModified));
   }
}