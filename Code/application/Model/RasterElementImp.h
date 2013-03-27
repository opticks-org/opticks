/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef RASTERELEMENTIMP_H
#define RASTERELEMENTIMP_H

#include "ComplexData.h"
#include "DataAccessor.h"
#include "DataElementImp.h"
#include "DimensionDescriptor.h"
#include "SafePtr.h"
#include "StatisticsImp.h"
#include "TypesFile.h"
#include "ProgressAdapter.h"

#include <boost/any.hpp>
#include <vector>

class RasterElementImp : public DataElementImp
{
public:
   RasterElementImp(const DataDescriptorImp& descriptor, const std::string& id);
   ~RasterElementImp();

   virtual double getPixelValue(DimensionDescriptor columnDim, DimensionDescriptor rowDim,
      DimensionDescriptor bandDim = DimensionDescriptor(), ComplexComponent component = COMPLEX_MAGNITUDE) const;

   virtual DataAccessor getDataAccessor(DataRequest* pRequestIn = NULL);
   virtual DataAccessor getDataAccessor(DataRequest* pRequestIn = NULL) const;

   virtual void incrementDataAccessor(DataAccessorImpl &da);
   virtual void updateData();
   virtual uint64_t sanitizeData(double value = 0.0);


   void setTerrain(RasterElement* pTerrain);
   const RasterElement* getTerrain() const;

   Statistics* getStatistics(DimensionDescriptor band) const;

   RasterElement *createChip(DataElement *pParent, const std::string &appendName,
      const std::vector<DimensionDescriptor>& selectedRows,
      const std::vector<DimensionDescriptor>& selectedColumns,
      const std::vector<DimensionDescriptor>& selectedBands = std::vector<DimensionDescriptor>()) const;
   DataElement *copy(const std::string &name, DataElement *pParent) const;
   virtual RasterElement* copyShallow(const std::string& name, DataElement* pParent) const;

   bool createTemporaryFile();
   bool createDefaultPager();
   bool createMemoryMappedPager();
   bool createInMemoryPager(void* pData, bool bOwner = true);
   bool setPager(RasterPager* pPager);
   RasterPager* getPager() const;

   const std::string& getTemporaryFilename() const;
   bool serialize(SessionItemSerializer& serializer) const;
   bool deserialize(SessionItemDeserializer &deserializer);

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   static bool isKindOfElement(const std::string& className);
   static void getElementTypes(std::vector<std::string>& classList);

   class Deleter : public DataAccessorDeleter
   {
      void operator()(DataAccessorImpl* pDataAccessor);
   };

   const void *getRawData() const;
   void *getRawData();
   bool setRawData(void* pData, bool bOwner = true);
   bool writeRawData(void* pData, InterleaveFormatType interleaveType,
      unsigned int startRow, unsigned int numRows, unsigned int startColumn, unsigned int numColumns,
      unsigned int startBand, unsigned int numBands);

   // Georeferencing
   LocationType convertPixelToGeocoord(LocationType pixel, bool quick = false, bool* pAccurate = NULL) const;
   std::vector<LocationType> convertPixelsToGeocoords(
      const std::vector<LocationType>& pixels, bool quick = false, bool* pAccurate = NULL) const;
   LocationType convertGeocoordToPixel(LocationType geocoord, bool quick = false, bool* pAccurate = NULL) const;
   std::vector<LocationType> convertGeocoordsToPixels(
      const std::vector<LocationType>& geocoords, bool quick = false, bool* pAccurate = NULL) const;
   bool isGeoreferenced() const;
   void setGeoreferencePlugin(Georeference* pGeo);
   Georeference *getGeoreferencePlugin() const;
   void updateGeoreferenceData();

protected:
   void updateStatisticsBadValues(Subject& subject, const std::string& signal, const boost::any& value);
   void updateDescriptorBadValues(Subject& subject, const std::string& signal, const boost::any& value);

   RasterElement* createChipInternal(DataElement* pParent, const std::string& name,
      const std::vector<DimensionDescriptor>& selectedRows,
      const std::vector<DimensionDescriptor>& selectedColumns,
      const std::vector<DimensionDescriptor>& selectedBands = std::vector<DimensionDescriptor>(),
      bool copyRasterData = true) const;

   bool createMemoryMappedPager(bool bUseDataDescriptor);

   bool copyDataToChip(RasterElement *pRasterChip, 
      const std::vector<DimensionDescriptor> &selectedRows,
      const std::vector<DimensionDescriptor> &selectedColumns,
      const std::vector<DimensionDescriptor> &selectedBands,
      bool &abort, Progress *pProgress = NULL) const;

   bool copyDataBip(RasterElement* pChipElement, const std::vector<DimensionDescriptor>& selectedRows,
      const std::vector<DimensionDescriptor>& selectedColumns, const std::vector<DimensionDescriptor>& selectedBands,
      bool& abort, Progress* pProgress) const;

   bool copyDataBil(RasterElement* pChipElement, const std::vector<DimensionDescriptor>& selectedRows,
      const std::vector<DimensionDescriptor>& selectedColumns, const std::vector<DimensionDescriptor>& selectedBands,
      bool& abort, Progress* pProgress) const;

   bool copyDataBsq(RasterElement* pChipElement, const std::vector<DimensionDescriptor>& selectedRows,
      const std::vector<DimensionDescriptor>& selectedColumns, const std::vector<DimensionDescriptor>& selectedBands,
      bool& abort, Progress* pProgress) const;

   /**
    * Appends to the basename of name.
    *
    * @param append
    *        The string to append to the basename.
    * @param name
    *        The string whose basename is appended to.
    * @return A string with append added to the basename of name.
    */
   static std::string appendToBasename(const std::string &name, const std::string &append);

   /**
    * Create new DimensionDescriptor vectors for the selectedDims.
    *
    * @param srcDims
    *        The objects to copy
    * @param selectedDims
    *        The subset of srcDims which forms the new active numbers.
    * @param chipActiveDims
    *        Output arg. Any existing values in the vector will be clear without deletion.
    *        The values for the new active numbers.
    * @param chipOnDiskDims
    *        Output arg. Any existing values in the vector will be clear without deletion.
    *        All elements from srcDims, with updated active numbers.  This vector will be
    *        empty if the on-disk numbers of srcDim are invalid.
    *
    * @return True if the operation was succesful, false otherwise.
    */
   static bool updateDims(
      const std::vector<DimensionDescriptor> &srcDims,
      const std::vector<DimensionDescriptor> &selectedDims, 
      std::vector<DimensionDescriptor> &chipActiveDims,
      std::vector<DimensionDescriptor> &chipOnDiskDims);

   class StatusBarProgress : public ProgressAdapter
   {
   public:
      StatusBarProgress();
      void getProgress(std::string &text, int &percent, ReportingLevel &gran) const;
      using ProgressAdapter::updateProgress;
      void updateProgress(const char *pText, int percent, ReportingLevel gran);

   private:
      std::string mText;
      int mPercent;
      ReportingLevel mGranularity;
   };

private:
   RasterElementImp(const RasterElementImp& rhs);
   RasterElementImp& operator=(const RasterElementImp& rhs);
   SafePtr<RasterElement> mpTerrain;
   std::map<DimensionDescriptor, StatisticsImp*> mStatistics;

   std::string mTempFilename;

   RasterPager* mpPager;
   RasterPager* mpBipConverterPager;
   RasterPager* mpBilConverterPager;
   RasterPager* mpBsqConverterPager;

   DataAccessor mCubePointerAccessor;

   mutable bool mModified;

   Georeference* mpGeoPlugin;
};

#define RASTERELEMENTADAPTEREXTENSION_CLASSES \
   DATAELEMENTADAPTEREXTENSION_CLASSES

#define RASTERELEMENTADAPTER_METHODS(impClass) \
   DATAELEMENTADAPTER_METHODS(impClass) \
   double getPixelValue(DimensionDescriptor pColumn, DimensionDescriptor pRow, \
      DimensionDescriptor pBand = DimensionDescriptor(), ComplexComponent component = COMPLEX_MAGNITUDE) const \
   { \
      return impClass::getPixelValue(pColumn, pRow, pBand, component); \
   } \
   virtual DataAccessor getDataAccessor(DataRequest *pRequest) \
   { \
      return impClass::getDataAccessor(pRequest); \
   } \
   virtual DataAccessor getDataAccessor(DataRequest *pRequest) const\
   { \
      return impClass::getDataAccessor(pRequest); \
   } \
   void incrementDataAccessor(DataAccessorImpl& accessor) \
   { \
      return impClass::incrementDataAccessor(accessor); \
   } \
   void updateData() \
   { \
      return impClass::updateData(); \
   } \
   virtual uint64_t sanitizeData(double value = 0.0) \
   { \
      return impClass::sanitizeData(value); \
   } \
   virtual RasterElement* copyShallow(const std::string& name, DataElement* pParent) const \
   { \
      return impClass::copyShallow(name, pParent); \
   } \
   void setTerrain(RasterElement* pTerrain) \
   { \
      return impClass::setTerrain(pTerrain); \
   } \
   const RasterElement* getTerrain() const \
   { \
      return impClass::getTerrain(); \
   } \
   Statistics* getStatistics(DimensionDescriptor pBand) const \
   { \
      return impClass::getStatistics(pBand); \
   } \
   RasterElement *createChip(DataElement *pParent, \
      const std::string &appendName, \
      const std::vector<DimensionDescriptor> &selectedRows, \
      const std::vector<DimensionDescriptor> &selectedColumns, \
      const std::vector<DimensionDescriptor> &selectedBands = std::vector<DimensionDescriptor>()) const \
   { \
      return impClass::createChip(pParent, appendName, selectedRows, selectedColumns, selectedBands); \
   } \
   bool createTemporaryFile() \
   { \
      return impClass::createTemporaryFile(); \
   } \
   bool createInMemoryPager(void* pData, bool bOwner = true) \
   { \
      return impClass::createInMemoryPager(pData, bOwner); \
   } \
   bool createMemoryMappedPager() \
   { \
      return impClass::createMemoryMappedPager(); \
   } \
   bool createDefaultPager() \
   { \
      return impClass::createDefaultPager(); \
   } \
   bool setPager(RasterPager* pPager) \
   { \
      return impClass::setPager(pPager); \
   } \
   RasterPager* getPager() const \
   { \
      return impClass::getPager(); \
   } \
   const std::string& getTemporaryFilename() const \
   { \
      return impClass::getTemporaryFilename(); \
   } \
   const void *getRawData() const \
   { \
      return impClass::getRawData(); \
   } \
   void *getRawData() \
   { \
      return impClass::getRawData(); \
   } \
   bool setRawData(void* pData, bool bOwner = true) \
   { \
      return impClass::setRawData(pData, bOwner); \
   } \
   bool copyDataToChip(RasterElement *pRasterChip, \
      const std::vector<DimensionDescriptor> &selectedRows, \
      const std::vector<DimensionDescriptor> &selectedColumns, \
      const std::vector<DimensionDescriptor> &selectedBands, \
      bool &abort, Progress *pProgress = NULL) const \
   { \
      return impClass::copyDataToChip(pRasterChip, selectedRows, selectedColumns, \
         selectedBands, abort, pProgress); \
   } \
   LocationType convertPixelToGeocoord(LocationType pixel, bool quick = false, bool* pAccurate = NULL) const \
   { \
      return impClass::convertPixelToGeocoord(pixel, quick, pAccurate); \
   } \
   std::vector<LocationType> convertPixelsToGeocoords(const std::vector<LocationType>& pixels, \
                                                      bool quick = false, bool* pAccurate = NULL) const \
   { \
      return impClass::convertPixelsToGeocoords(pixels, quick, pAccurate); \
   } \
   LocationType convertGeocoordToPixel(LocationType geocoord, bool quick = false, bool* pAccurate = NULL) const \
   { \
      return impClass::convertGeocoordToPixel(geocoord, quick, pAccurate); \
   } \
   std::vector<LocationType> convertGeocoordsToPixels(const std::vector<LocationType>& geocoords, \
                                                      bool quick = false, bool* pAccurate = NULL) const \
   { \
      return impClass::convertGeocoordsToPixels(geocoords, quick, pAccurate); \
   } \
   bool isGeoreferenced() const \
   { \
      return impClass::isGeoreferenced(); \
   } \
   void setGeoreferencePlugin(Georeference *pGeoPlugin) \
   { \
      return impClass::setGeoreferencePlugin(pGeoPlugin); \
   } \
   Georeference *getGeoreferencePlugin() const \
   { \
      return impClass::getGeoreferencePlugin(); \
   } \
   void updateGeoreferenceData() \
   { \
      return impClass::updateGeoreferenceData(); \
   } \
   bool writeRawData(void* pData, InterleaveFormatType interleaveType, \
      unsigned int startRow, unsigned int numRows, unsigned int startColumn, unsigned int numColumns, \
      unsigned int startBand, unsigned int numBands) \
   { \
      return impClass::writeRawData(pData, interleaveType, startRow, numRows, \
         startColumn, numColumns, startBand, numBands); \
   }

#endif