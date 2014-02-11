/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef POINTCLOUDDATADESCRIPTORIMP_H
#define POINTCLOUDDATADESCRIPTORIMP_H

#include "DataDescriptorImp.h"
#include "DimensionDescriptor.h"
#include "PointCloudDataDescriptor.h"
#include "TypesFile.h"
#include "UnitsImp.h"

#include <string>
#include <vector>

using XERCES_CPP_NAMESPACE_QUALIFIER DOMNode;

class PointCloudDataDescriptorImp : public DataDescriptorImp
{
public:
   PointCloudDataDescriptorImp(const std::string& name, const std::string& type, DataElement* pParent);
   PointCloudDataDescriptorImp(const std::string& name, const std::string& type, const std::vector<std::string>& parent);
   PointCloudDataDescriptorImp(DOMNode *pDocument, unsigned int version, DataElement *pParent);
   ~PointCloudDataDescriptorImp();

   virtual uint32_t getPointCount() const;
   virtual void setPointCount(uint32_t pointTotal);
   virtual PointCloudArrangement getArrangement() const;
   virtual void setArrangement(PointCloudArrangement arrangement);
   virtual double getXScale() const;
   virtual void setXScale(double scale);
   virtual double getYScale() const;
   virtual void setYScale(double scale);
   virtual double getZScale() const;
   virtual void setZScale(double scale);
   virtual double getXOffset() const;
   virtual void setXOffset(double offset);
   virtual double getYOffset() const;
   virtual void setYOffset(double offset);
   virtual double getZOffset() const;
   virtual void setZOffset(double offset);
   virtual double getXMin() const;
   virtual void setXMin(double min);
   virtual double getYMin() const;
   virtual void setYMin(double min);
   virtual double getZMin() const;
   virtual void setZMin(double min);
   virtual double getXMax() const;
   virtual void setXMax(double max);
   virtual double getYMax() const;
   virtual void setYMax(double max);
   virtual double getZMax() const;
   virtual void setZMax(double max);
   virtual EncodingType getSpatialDataType() const;
   virtual void setSpatialDataType(EncodingType type);
   virtual bool hasIntensityData() const;
   virtual void setHasIntensityData(bool intensityPresent);
   virtual EncodingType getIntensityDataType() const;
   virtual void setIntensityDataType(EncodingType type);
   virtual bool hasClassificationData() const;
   virtual void setHasClassificationData(bool classificationPresent);
   virtual EncodingType getClassificationDataType() const;
   virtual void setClassificationDataType(EncodingType type);
   virtual size_t getPointSizeInBytes() const;

   using DataDescriptorImp::copy;
   DataDescriptor* copy(const std::string& name, DataElement* pParent) const;

   void addToMessageLog(Message* pMessage) const;

   virtual bool toXml(XMLWriter* pXml) const;
   virtual bool fromXml(DOMNode* pDocument, unsigned int version);
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   static void getDataDescriptorTypes(std::vector<std::string>& classList);
   static bool isKindOfDataDescriptor(const std::string& className);

private:
   uint32_t mPointCount;
   PointCloudArrangement mArrangement;
   double mXScale;
   double mYScale;
   double mZScale;
   double mXOffset;
   double mYOffset;
   double mZOffset;
   double mXMin;
   double mYMin;
   double mZMin;
   double mXMax;
   double mYMax;
   double mZMax;
   EncodingType mSpatialDataType;
   bool mHasIntensityData;
   EncodingType mIntensityDataType;
   bool mHasClassificationData;
   EncodingType mClassificationDataType;
};

#define POINTCLOUDDATADESCRIPTORADAPTEREXTENSION_CLASSES \
   DATADESCRIPTORADAPTEREXTENSION_CLASSES

#define POINTCLOUDDATADESCRIPTORADAPTER_METHODS(impClass) \
   DATADESCRIPTORADAPTER_METHODS(impClass) \
   uint32_t getPointCount() const \
   { \
      return impClass::getPointCount(); \
   } \
   void setPointCount(uint32_t pointTotal) \
   { \
      impClass::setPointCount(pointTotal); \
   } \
   PointCloudArrangement getArrangement() const \
   { \
      return impClass::getArrangement(); \
   } \
   void setArrangement(PointCloudArrangement arrangement) \
   { \
      impClass::setArrangement(arrangement); \
   } \
   double getXScale() const \
   { \
      return impClass::getXScale(); \
   } \
   void setXScale(double scale) \
   { \
      impClass::setXScale(scale); \
   } \
   double getYScale() const \
   { \
      return impClass::getYScale(); \
   } \
   void setYScale(double scale) \
   { \
      impClass::setYScale(scale); \
   } \
   double getZScale() const \
   { \
      return impClass::getZScale(); \
   } \
   void setZScale(double scale) \
   { \
      impClass::setZScale(scale); \
   } \
   double getXOffset() const \
   { \
      return impClass::getXOffset(); \
   } \
   void setXOffset(double offset) \
   { \
      impClass::setXOffset(offset); \
   } \
   double getYOffset() const \
   { \
      return impClass::getYOffset(); \
   } \
   void setYOffset(double offset) \
   { \
      impClass::setYOffset(offset); \
   } \
   double getZOffset() const \
   { \
      return impClass::getZOffset(); \
   } \
   void setZOffset(double offset) \
   { \
      impClass::setZOffset(offset); \
   } \
   double getXMin() const \
   { \
      return impClass::getXMin(); \
   } \
   void setXMin(double min) \
   { \
      impClass::setXMin(min); \
   } \
   double getYMin() const \
   { \
      return impClass::getYMin(); \
   } \
   void setYMin(double min) \
   { \
      impClass::setYMin(min); \
   } \
   double getZMin() const \
   { \
      return impClass::getZMin(); \
   } \
   void setZMin(double min) \
   { \
      impClass::setZMin(min); \
   } \
   double getXMax() const \
   { \
      return impClass::getXMax(); \
   } \
   void setXMax(double max) \
   { \
      impClass::setXMax(max); \
   } \
   double getYMax() const \
   { \
      return impClass::getYMax(); \
   } \
   void setYMax(double max) \
   { \
      impClass::setYMax(max); \
   } \
   double getZMax() const \
   { \
      return impClass::getZMax(); \
   } \
   void setZMax(double max) \
   { \
      impClass::setZMax(max); \
   } \
   EncodingType getSpatialDataType() const \
   { \
      return impClass::getSpatialDataType(); \
   } \
   void setSpatialDataType(EncodingType type) \
   { \
      impClass::setSpatialDataType(type); \
   } \
   bool hasIntensityData() const \
   { \
      return impClass::hasIntensityData(); \
   } \
   void setHasIntensityData(bool intensityPresent) \
   { \
      impClass::setHasIntensityData(intensityPresent); \
   } \
   EncodingType getIntensityDataType() const \
   { \
      return impClass::getIntensityDataType(); \
   } \
   void setIntensityDataType(EncodingType type) \
   { \
      impClass::setIntensityDataType(type); \
   } \
   bool hasClassificationData() const \
   { \
      return impClass::hasClassificationData(); \
   } \
   void setHasClassificationData(bool classificationPresent) \
   { \
      impClass::setHasClassificationData(classificationPresent); \
   } \
   EncodingType getClassificationDataType() const \
   { \
      return impClass::getClassificationDataType(); \
   } \
   void setClassificationDataType(EncodingType type) \
   { \
      impClass::setClassificationDataType(type); \
   } \
   size_t getPointSizeInBytes() const \
   { \
      return impClass::getPointSizeInBytes(); \
   } \

#endif
