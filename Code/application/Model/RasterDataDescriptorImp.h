/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef RASTERDATADESCRIPTORIMP_H
#define RASTERDATADESCRIPTORIMP_H

#include "DataDescriptorImp.h"
#include "DimensionDescriptor.h"
#include "TypesFile.h"
#include "UnitsAdapter.h"

#include <string>
#include <vector>

using XERCES_CPP_NAMESPACE_QUALIFIER DOMNode;

class RasterDataDescriptorImp : public DataDescriptorImp
{
public:
   RasterDataDescriptorImp(const std::string& name, const std::string& type, DataElement* pParent);
   RasterDataDescriptorImp(const std::string& name, const std::string& type, const std::vector<std::string>& parent);
   RasterDataDescriptorImp(DOMNode *pDocument, unsigned int version, DataElement *pParent);
   ~RasterDataDescriptorImp();

   using DataDescriptorImp::copy;

   void setDataType(EncodingType dataType);
   EncodingType getDataType() const;
   void setValidDataTypes(const std::vector<EncodingType>& validDataTypes);
   const std::vector<EncodingType>& getValidDataTypes() const;
   unsigned int getBytesPerElement() const;
   void setInterleaveFormat(InterleaveFormatType format);
   InterleaveFormatType getInterleaveFormat() const;

   void setBadValues(const std::vector<int>& badValues);
   const std::vector<int>& getBadValues() const;

   void setRows(const std::vector<DimensionDescriptor>& rows);
   const std::vector<DimensionDescriptor>& getRows() const;
   unsigned int getRowSkipFactor() const;
   DimensionDescriptor getOriginalRow(unsigned int originalNumber) const;
   DimensionDescriptor getOnDiskRow(unsigned int onDiskNumber) const;
   DimensionDescriptor getActiveRow(unsigned int activeNumber) const;
   unsigned int getRowCount() const;

   void setColumns(const std::vector<DimensionDescriptor>& columns);
   const std::vector<DimensionDescriptor>& getColumns() const;
   unsigned int getColumnSkipFactor() const;
   DimensionDescriptor getOriginalColumn(unsigned int originalNumber) const;
   DimensionDescriptor getOnDiskColumn(unsigned int onDiskNumber) const;
   DimensionDescriptor getActiveColumn(unsigned int activeNumber) const;
   unsigned int getColumnCount() const;

   void setBands(const std::vector<DimensionDescriptor>& bands);
   const std::vector<DimensionDescriptor>& getBands() const;
   DimensionDescriptor getOriginalBand(unsigned int originalNumber) const;
   DimensionDescriptor getOnDiskBand(unsigned int onDiskNumber) const;
   DimensionDescriptor getActiveBand(unsigned int activeNumber) const;
   unsigned int getBandCount() const;

   void setXPixelSize(double pixelSize);
   double getXPixelSize() const;
   void setYPixelSize(double pixelSize);
   double getYPixelSize() const;

   void setUnits(const Units* pUnits);
   Units* getUnits();
   const Units* getUnits() const;

   void setDisplayBand(RasterChannelType eColor, DimensionDescriptor band);
   DimensionDescriptor getDisplayBand(RasterChannelType eColor) const;
   void setDisplayMode(DisplayMode displayMode);
   DisplayMode getDisplayMode() const;

   virtual DataDescriptor* copy(const std::string& name, DataElement* pParent) const;
   virtual DataDescriptor* copy(const std::string& name, const std::vector<std::string>& parent) const;
   virtual bool clone(const DataDescriptor* pDescriptor);

   void addToMessageLog(Message* pMessage) const;

   virtual bool toXml(XMLWriter* pXml) const;
   virtual bool fromXml(DOMNode* pDocument, unsigned int version);
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   static void getDataDescriptorTypes(std::vector<std::string>& classList);
   static bool isKindOfDataDescriptor(const std::string& className);

protected:
   void notifyUnitsModified(Subject& subject, const std::string& signal, const boost::any& data);

private:
   RasterDataDescriptorImp(const RasterDataDescriptorImp& rhs);
   RasterDataDescriptorImp& operator=(const RasterDataDescriptorImp& rhs);

   EncodingType mDataType;
   std::vector<EncodingType> mValidDataTypes;
   InterleaveFormatType mInterleave;
   std::vector<int> mBadValues;

   std::vector<DimensionDescriptor> mRows;
   std::vector<DimensionDescriptor> mColumns;
   std::vector<DimensionDescriptor> mBands;

   unsigned int mRowSkipFactor;
   unsigned int mColumnSkipFactor;

   double mXPixelSize;
   double mYPixelSize;

   UnitsAdapter mUnits;

   DimensionDescriptor mGrayBand;
   DimensionDescriptor mRedBand;
   DimensionDescriptor mGreenBand;
   DimensionDescriptor mBlueBand;
   DisplayMode mDisplayMode;
};

#define RASTERDATADESCRIPTORADAPTEREXTENSION_CLASSES \
   DATADESCRIPTORADAPTEREXTENSION_CLASSES

#define RASTERDATADESCRIPTORADAPTER_METHODS(impClass) \
   DATADESCRIPTORADAPTER_METHODS(impClass) \
   void setDataType(EncodingType dataType) \
   { \
      impClass::setDataType(dataType); \
   } \
   EncodingType getDataType() const \
   { \
      return impClass::getDataType(); \
   } \
   void setValidDataTypes(const std::vector<EncodingType>& validDataTypes) \
   { \
      impClass::setValidDataTypes(validDataTypes); \
   } \
   const std::vector<EncodingType>& getValidDataTypes() const \
   { \
      return impClass::getValidDataTypes(); \
   } \
   unsigned int getBytesPerElement() const \
   { \
      return impClass::getBytesPerElement(); \
   } \
   void setBadValues(const std::vector<int>& badValues) \
   { \
      impClass::setBadValues(badValues); \
   } \
   const std::vector<int>& getBadValues() const \
   { \
      return impClass::getBadValues(); \
   } \
   void setRows(const std::vector<DimensionDescriptor>& rows) \
   { \
      impClass::setRows(rows); \
   } \
   const std::vector<DimensionDescriptor>& getRows() const \
   { \
      return impClass::getRows(); \
   } \
   unsigned int getRowSkipFactor() const \
   { \
      return impClass::getRowSkipFactor(); \
   } \
   DimensionDescriptor getOriginalRow(unsigned int originalNumber) const \
   { \
      return impClass::getOriginalRow(originalNumber); \
   } \
   DimensionDescriptor getOnDiskRow(unsigned int onDiskNumber) const \
   { \
      return impClass::getOnDiskRow(onDiskNumber); \
   } \
   DimensionDescriptor getActiveRow(unsigned int activeNumber) const \
   { \
      return impClass::getActiveRow(activeNumber); \
   } \
   unsigned int getRowCount() const \
   { \
      return impClass::getRowCount(); \
   } \
   void setColumns(const std::vector<DimensionDescriptor>& columns) \
   { \
      impClass::setColumns(columns); \
   } \
   const std::vector<DimensionDescriptor>& getColumns() const \
   { \
      return impClass::getColumns(); \
   } \
   unsigned int getColumnSkipFactor() const \
   { \
      return impClass::getColumnSkipFactor(); \
   } \
   DimensionDescriptor getOriginalColumn(unsigned int originalNumber) const \
   { \
      return impClass::getOriginalColumn(originalNumber); \
   } \
   DimensionDescriptor getOnDiskColumn(unsigned int onDiskNumber) const \
   { \
      return impClass::getOnDiskColumn(onDiskNumber); \
   } \
   DimensionDescriptor getActiveColumn(unsigned int activeNumber) const \
   { \
      return impClass::getActiveColumn(activeNumber); \
   } \
   unsigned int getColumnCount() const \
   { \
      return impClass::getColumnCount(); \
   } \
   void setXPixelSize(double pixelSize) \
   { \
      impClass::setXPixelSize(pixelSize); \
   } \
   double getXPixelSize() const \
   { \
      return impClass::getXPixelSize(); \
   } \
   void setYPixelSize(double pixelSize) \
   { \
      impClass::setYPixelSize(pixelSize); \
   } \
   double getYPixelSize() const \
   { \
      return impClass::getYPixelSize(); \
   } \
   void setUnits(const Units* pUnits) \
   { \
      impClass::setUnits(pUnits); \
   } \
   Units* getUnits() \
   { \
      return impClass::getUnits(); \
   } \
   const Units* getUnits() const \
   { \
      return impClass::getUnits(); \
   } \
   void setInterleaveFormat(InterleaveFormatType format) \
   { \
      impClass::setInterleaveFormat(format); \
   } \
   InterleaveFormatType getInterleaveFormat() const \
   { \
      return impClass::getInterleaveFormat(); \
   } \
   void setBands(const std::vector<DimensionDescriptor>& bands) \
   { \
      impClass::setBands(bands); \
   } \
   const std::vector<DimensionDescriptor>& getBands() const \
   { \
      return impClass::getBands(); \
   } \
   DimensionDescriptor getOriginalBand(unsigned int originalNumber) const \
   { \
      return impClass::getOriginalBand(originalNumber); \
   } \
   DimensionDescriptor getOnDiskBand(unsigned int onDiskNumber) const \
   { \
      return impClass::getOnDiskBand(onDiskNumber); \
   } \
   DimensionDescriptor getActiveBand(unsigned int activeNumber) const \
   { \
      return impClass::getActiveBand(activeNumber); \
   } \
   unsigned int getBandCount() const \
   { \
      return impClass::getBandCount(); \
   } \
   void setDisplayBand(RasterChannelType eColor, DimensionDescriptor pBand) \
   { \
      impClass::setDisplayBand(eColor, pBand); \
   } \
   DimensionDescriptor getDisplayBand(RasterChannelType eColor) const \
   { \
      return impClass::getDisplayBand(eColor); \
   } \
   void setDisplayMode(DisplayMode displayMode) \
   { \
      impClass::setDisplayMode(displayMode); \
   } \
   DisplayMode getDisplayMode() const \
   { \
      return impClass::getDisplayMode(); \
   }
#endif
