/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef RASTERFILEDESCRIPTORIMP_H
#define RASTERFILEDESCRIPTORIMP_H

#include "DimensionDescriptor.h"
#include "FileDescriptorImp.h"
#include "GcpList.h"
#include "UnitsImp.h"
#include "xmlwriter.h"

#include <list>
#include <string>
#include <vector>

#include "XercesIncludes.h"

class RasterFileDescriptorImp : public FileDescriptorImp
{
public:
   RasterFileDescriptorImp();
   ~RasterFileDescriptorImp();

   RasterFileDescriptorImp& operator =(const RasterFileDescriptorImp& descriptor);

   void setHeaderBytes(unsigned int bytes);
   unsigned int getHeaderBytes() const;
   void setTrailerBytes(unsigned int bytes);
   unsigned int getTrailerBytes() const;
   void setPrelineBytes(unsigned int bytes);
   unsigned int getPrelineBytes() const;
   void setPostlineBytes(unsigned int bytes);
   unsigned int getPostlineBytes() const;
   void setPrebandBytes(unsigned int bytes);
   unsigned int getPrebandBytes() const;
   void setPostbandBytes(unsigned int bytes);
   unsigned int getPostbandBytes() const;

   void setBitsPerElement(unsigned int numBits);
   unsigned int getBitsPerElement() const;

   void setInterleaveFormat(InterleaveFormatType format);
   InterleaveFormatType getInterleaveFormat() const;

   void setRows(const std::vector<DimensionDescriptor>& rows);
   const std::vector<DimensionDescriptor>& getRows() const;
   DimensionDescriptor getOriginalRow(unsigned int originalNumber) const;
   DimensionDescriptor getOnDiskRow(unsigned int onDiskNumber) const;
   DimensionDescriptor getActiveRow(unsigned int activeNumber) const;
   unsigned int getRowCount() const;

   void setColumns(const std::vector<DimensionDescriptor>& columns);
   const std::vector<DimensionDescriptor>& getColumns() const;
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

   void setUnits(const UnitsImp* pUnits);
   UnitsImp* getUnits();
   const UnitsImp* getUnits() const;

   void setGcps(const std::list<GcpPoint>& gcps);
   const std::list<GcpPoint>& getGcps() const;

   void setBandFiles(const std::vector<std::string>& bandFiles);
   void setBandFiles(const std::vector<const Filename*>& bandFiles);
   const std::vector<const Filename*>& getBandFiles() const;


   void addToMessageLog(Message* pMessage) const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   static void getFileDescriptorTypes(std::vector<std::string>& classList);
   static bool isKindOfFileDescriptor(const std::string& className);

private:
   void clearBandFiles();

   unsigned int mHeaderBytes;
   unsigned int mTrailerBytes;
   unsigned int mPrelineBytes;
   unsigned int mPostlineBytes;
   unsigned int mPrebandBytes;
   unsigned int mPostbandBytes;
   unsigned int mBitsPerElement;
   InterleaveFormatType mInterleave;

   std::vector<DimensionDescriptor> mRows;
   std::vector<DimensionDescriptor> mColumns;
   std::vector<DimensionDescriptor> mBands;

   double mXPixelSize;
   double mYPixelSize;

   UnitsImp mUnits;
   std::list<GcpPoint> mGcps;

   std::vector<const Filename*> mBandFiles;
};

namespace XmlUtilities
{
   void serializeDimensionDescriptor(const DimensionDescriptor& desc, XMLWriter* pXml);
   void deserializeDimensionDescriptor(DimensionDescriptor& desc, XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* pNode);

   void serializeDimensionDescriptors(const std::string& elementName,
      const std::vector<DimensionDescriptor>& desc, XMLWriter* pXml);
   void deserializeDimensionDescriptors(const std::string& elementName,
      std::vector<DimensionDescriptor>& desc,
      XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* pNode);
};

#define RASTERFILEDESCRIPTORADAPTEREXTENSION_CLASSES \
   FILEDESCRIPTORADAPTEREXTENSION_CLASSES

#define RASTERFILEDESCRIPTORADAPTER_METHODS(impClass) \
   FILEDESCRIPTORADAPTER_METHODS(impClass) \
   void setHeaderBytes(unsigned int bytes) \
   { \
      impClass::setHeaderBytes(bytes); \
   } \
   unsigned int getHeaderBytes() const \
   { \
      return impClass::getHeaderBytes(); \
   } \
   void setTrailerBytes(unsigned int bytes) \
   { \
      impClass::setTrailerBytes(bytes); \
   } \
   unsigned int getTrailerBytes() const \
   { \
      return impClass::getTrailerBytes(); \
   } \
   void setPrelineBytes(unsigned int bytes) \
   { \
      impClass::setPrelineBytes(bytes); \
   } \
   unsigned int getPrelineBytes() const \
   { \
      return impClass::getPrelineBytes(); \
   } \
   void setPostlineBytes(unsigned int bytes) \
   { \
      impClass::setPostlineBytes(bytes); \
   } \
   unsigned int getPostlineBytes() const \
   { \
      return impClass::getPostlineBytes(); \
   } \
   void setBitsPerElement(unsigned int numBits) \
   { \
      return impClass::setBitsPerElement(numBits); \
   } \
   unsigned int getBitsPerElement() const \
   { \
      return impClass::getBitsPerElement(); \
   } \
   void setRows(const std::vector<DimensionDescriptor>& rows) \
   { \
      impClass::setRows(rows); \
   } \
   const std::vector<DimensionDescriptor>& getRows() const \
   { \
      return impClass::getRows(); \
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
      impClass::setUnits(dynamic_cast<const UnitsImp*>(pUnits)); \
   } \
   Units* getUnits() \
   { \
      return dynamic_cast<Units*>(impClass::getUnits()); \
   } \
   const Units* getUnits() const \
   { \
      return dynamic_cast<const Units*>(impClass::getUnits()); \
   } \
   void setGcps(const std::list<GcpPoint>& gcps) \
   { \
      impClass::setGcps(gcps); \
   } \
   const std::list<GcpPoint>& getGcps() const \
   { \
      return impClass::getGcps(); \
   } \
     void setPrebandBytes(unsigned int bytes) \
   { \
      impClass::setPrebandBytes(bytes); \
   } \
   unsigned int getPrebandBytes() const \
   { \
      return impClass::getPrebandBytes(); \
   } \
   void setPostbandBytes(unsigned int bytes) \
   { \
      impClass::setPostbandBytes(bytes); \
   } \
   unsigned int getPostbandBytes() const \
   { \
      return impClass::getPostbandBytes(); \
   } \
   void setInterleaveFormat(InterleaveFormatType format) \
   { \
      impClass::setInterleaveFormat(format); \
   } \
   InterleaveFormatType getInterleaveFormat() const \
   { \
      return impClass::getInterleaveFormat(); \
   } \
   void setBandFiles(const std::vector<std::string>& bandFiles) \
   { \
      impClass::setBandFiles(bandFiles); \
   } \
   void setBandFiles(const std::vector<const Filename*>& bandFiles) \
   { \
      impClass::setBandFiles(bandFiles); \
   } \
   const std::vector<const Filename*>& getBandFiles() const \
   { \
      return impClass::getBandFiles(); \
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
   }


#endif
