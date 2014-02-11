/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "PointCloudDataDescriptor.h"
#include "PointCloudDataDescriptorImp.h"
#include "PointCloudElement.h"
#include "PointCloudFileDescriptorImp.h"
#include "RasterUtilities.h"
#include <limits>

XERCES_CPP_NAMESPACE_USE
using namespace std;

PointCloudDataDescriptorImp::PointCloudDataDescriptorImp(const string& name, const string& type,
   DataElement* pParent) :
   DataDescriptorImp(name, type, pParent),
   mPointCount(0),
   mArrangement(POINT_ARRAY),
   mXScale(1.0),
   mYScale(1.0),
   mZScale(1.0),
   mXOffset(0.0),
   mYOffset(0.0),
   mZOffset(0.0),
   mXMin(numeric_limits<double>::max()),
   mYMin(numeric_limits<double>::max()),
   mZMin(numeric_limits<double>::max()),
   mXMax(-1.0 * numeric_limits<double>::max()),
   mYMax(-1.0 * numeric_limits<double>::max()),
   mZMax(-1.0 * numeric_limits<double>::max()),
   mSpatialDataType(INT1UBYTE),
   mHasIntensityData(false),
   mIntensityDataType(INT1UBYTE),
   mHasClassificationData(false),
   mClassificationDataType(INT1UBYTE)
{
}

PointCloudDataDescriptorImp::PointCloudDataDescriptorImp(const string& name, const string& type, const vector<string>& parent) :
   DataDescriptorImp(name, type, parent),
   mPointCount(0),
   mArrangement(POINT_ARRAY),
   mXScale(1.0),
   mYScale(1.0),
   mZScale(1.0),
   mXOffset(0.0),
   mYOffset(0.0),
   mZOffset(0.0),
   mXMin(numeric_limits<double>::max()),
   mYMin(numeric_limits<double>::max()),
   mZMin(numeric_limits<double>::max()),
   mXMax(-1.0 * numeric_limits<double>::max()),
   mYMax(-1.0 * numeric_limits<double>::max()),
   mZMax(-1.0 * numeric_limits<double>::max()),
   mSpatialDataType(INT1UBYTE),
   mHasIntensityData(false),
   mIntensityDataType(INT1UBYTE),
   mHasClassificationData(false),
   mClassificationDataType(INT1UBYTE)
{
}

PointCloudDataDescriptorImp::~PointCloudDataDescriptorImp()
{
}

uint32_t PointCloudDataDescriptorImp::getPointCount() const
{
   return mPointCount;
}

void PointCloudDataDescriptorImp::setPointCount(uint32_t pointTotal)
{
   if (pointTotal != mPointCount)
   {
      mPointCount = pointTotal;
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

PointCloudArrangement PointCloudDataDescriptorImp::getArrangement() const
{
   return mArrangement;
}

void PointCloudDataDescriptorImp::setArrangement(PointCloudArrangement arrangement)
{
   if (arrangement != mArrangement)
   {
      mArrangement = arrangement;
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

double PointCloudDataDescriptorImp::getXScale() const
{
   return mXScale;
}

void PointCloudDataDescriptorImp::setXScale(double scale)
{
   if (scale != mXScale)
   {
      mXScale = scale;
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

double PointCloudDataDescriptorImp::getYScale() const
{
   return mYScale;
}

void PointCloudDataDescriptorImp::setYScale(double scale)
{
   if (scale != mYScale)
   {
      mYScale = scale;
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

double PointCloudDataDescriptorImp::getZScale() const
{
   return mZScale;
}

void PointCloudDataDescriptorImp::setZScale(double scale)
{
   if (scale != mZScale)
   {
      mZScale = scale;
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

double PointCloudDataDescriptorImp::getXOffset() const
{
   return mXOffset;
}

void PointCloudDataDescriptorImp::setXOffset(double offset)
{
   if (offset != mXOffset)
   {
      mXOffset = offset;
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

double PointCloudDataDescriptorImp::getYOffset() const
{
   return mYOffset;
}

void PointCloudDataDescriptorImp::setYOffset(double offset)
{
   if (offset != mYOffset)
   {
      mYOffset = offset;
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

double PointCloudDataDescriptorImp::getZOffset() const
{
   return mZOffset;
}

void PointCloudDataDescriptorImp::setZOffset(double offset)
{
   if (offset != mZOffset)
   {
      mZOffset = offset;
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

double PointCloudDataDescriptorImp::getXMin() const
{
   return mXMin;
}

void PointCloudDataDescriptorImp::setXMin(double min)
{
   if (min != mXMin)
   {
      mXMin = min;
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

double PointCloudDataDescriptorImp::getYMin() const
{
   return mYMin;
}

void PointCloudDataDescriptorImp::setYMin(double min)
{
   if (min != mYMin)
   {
      mYMin = min;
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

double PointCloudDataDescriptorImp::getZMin() const
{
   return mZMin;
}

void PointCloudDataDescriptorImp::setZMin(double min)
{
   if (min != mZMin)
   {
      mZMin = min;
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

double PointCloudDataDescriptorImp::getXMax() const
{
   return mXMax;
}

void PointCloudDataDescriptorImp::setXMax(double max)
{
   if (max != mXMax)
   {
      mXMax = max;
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

double PointCloudDataDescriptorImp::getYMax() const
{
   return mYMax;
}

void PointCloudDataDescriptorImp::setYMax(double max)
{
   if (max != mYMax)
   {
      mYMax = max;
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

double PointCloudDataDescriptorImp::getZMax() const
{
   return mZMax;
}

void PointCloudDataDescriptorImp::setZMax(double max)
{
   if (max != mZMax)
   {
      mZMax = max;
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

EncodingType PointCloudDataDescriptorImp::getSpatialDataType() const
{
   return mSpatialDataType;
}

void PointCloudDataDescriptorImp::setSpatialDataType(EncodingType type)
{
   if (type != mSpatialDataType)
   {
      mSpatialDataType = type;
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

bool PointCloudDataDescriptorImp::hasIntensityData() const
{
   return mHasIntensityData;
}

void PointCloudDataDescriptorImp::setHasIntensityData(bool intensityPresent)
{
   if (intensityPresent != mHasIntensityData)
   {
      mHasIntensityData = intensityPresent;
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

EncodingType PointCloudDataDescriptorImp::getIntensityDataType() const
{
   return mIntensityDataType;
}

void PointCloudDataDescriptorImp::setIntensityDataType(EncodingType type)
{
   if (type != mIntensityDataType)
   {
      mIntensityDataType = type;
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

bool PointCloudDataDescriptorImp::hasClassificationData() const
{
   return mHasClassificationData;
}

void PointCloudDataDescriptorImp::setHasClassificationData(bool classificationPresent)
{
   if (classificationPresent != mHasClassificationData)
   {
      mHasClassificationData = classificationPresent;
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

EncodingType PointCloudDataDescriptorImp::getClassificationDataType() const
{
   return mClassificationDataType;
}

void PointCloudDataDescriptorImp::setClassificationDataType(EncodingType type)
{
   if (type != mClassificationDataType)
   {
      mClassificationDataType = type;
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

size_t PointCloudDataDescriptorImp::getPointSizeInBytes() const
{
   size_t pointSize = RasterUtilities::bytesInEncoding(mSpatialDataType) * 3; // x, y, z
   pointSize += sizeof(PointCloudElement::pointIdType);
   pointSize += sizeof(PointCloudElement::validPointType);
   if (mHasIntensityData)
   {
      pointSize += RasterUtilities::bytesInEncoding(mIntensityDataType);
   }
   if (mHasClassificationData)
   {
      pointSize += RasterUtilities::bytesInEncoding(mClassificationDataType);
   }
   return pointSize;
}

DataDescriptor* PointCloudDataDescriptorImp::copy(const string& name, DataElement* pParent) const
{
   PointCloudDataDescriptor* pDescriptor = dynamic_cast<PointCloudDataDescriptor*>(DataDescriptorImp::copy(name, pParent));
   if (pDescriptor != NULL)
   {
      pDescriptor->setPointCount(mPointCount);
      pDescriptor->setArrangement(mArrangement);
      pDescriptor->setXScale(mXScale);
      pDescriptor->setYScale(mYScale);
      pDescriptor->setZScale(mZScale);
      pDescriptor->setXOffset(mXOffset);
      pDescriptor->setYOffset(mYOffset);
      pDescriptor->setZOffset(mZOffset);
      pDescriptor->setXMin(mXMin);
      pDescriptor->setYMin(mYMin);
      pDescriptor->setZMin(mZMin);
      pDescriptor->setXMax(mXMax);
      pDescriptor->setYMax(mYMax);
      pDescriptor->setZMax(mZMax);
      pDescriptor->setSpatialDataType(mSpatialDataType);
      pDescriptor->setHasIntensityData(mHasIntensityData);
      pDescriptor->setIntensityDataType(mIntensityDataType);
      pDescriptor->setHasClassificationData(mHasClassificationData);
      pDescriptor->setClassificationDataType(mClassificationDataType);
   }

   return pDescriptor;
}

void PointCloudDataDescriptorImp::addToMessageLog(Message* pMessage) const
{
   DataDescriptorImp::addToMessageLog(pMessage);

   if (pMessage != NULL)
   {
      return;
   }

   pMessage->addProperty("Point Count", mPointCount);
   if (mArrangement.isValid())
   {
      pMessage->addProperty("Point Arrangement", StringUtilities::toXmlString(mArrangement));
   }
   pMessage->addProperty("X Scale", mXScale);
   pMessage->addProperty("Y Scale", mYScale);
   pMessage->addProperty("Z Scale", mZScale);
   pMessage->addProperty("X Offset", mXOffset);
   pMessage->addProperty("Y Offset", mYOffset);
   pMessage->addProperty("Z Offset", mZOffset);
   pMessage->addProperty("X Min", mXMin);
   pMessage->addProperty("Y Min", mYMin);
   pMessage->addProperty("Z Min", mZMin);
   pMessage->addProperty("X Max", mXMax);
   pMessage->addProperty("Y Max", mYMax);
   pMessage->addProperty("Z Max", mZMax);
   if (mSpatialDataType.isValid())
   {
      pMessage->addProperty("Spatial Data Type", StringUtilities::toXmlString(mSpatialDataType));
   }
   pMessage->addBooleanProperty("Intensity Data Present", mHasIntensityData);
   if (mHasIntensityData && mIntensityDataType.isValid())
   {
      pMessage->addProperty("Intensity Data Type", StringUtilities::toXmlString(mIntensityDataType));
   }
   pMessage->addBooleanProperty("Classification Data Present", mHasClassificationData);
   if (mHasClassificationData && mClassificationDataType.isValid())
   {
      pMessage->addProperty("Classification Data Type", StringUtilities::toXmlString(mClassificationDataType));
   }
}

bool PointCloudDataDescriptorImp::toXml(XMLWriter* pXml) const
{
   if (pXml == NULL)
   {
      return false;
   }

   bool bSuccess = DataDescriptorImp::toXml(pXml);
   if (bSuccess == true)
   {
      pXml->addAttr("pointCount", mPointCount);
      pXml->addAttr("arrangement", mArrangement);
      pXml->addAttr("xScale", mXScale);
      pXml->addAttr("yScale", mYScale);
      pXml->addAttr("zScale", mZScale);
      pXml->addAttr("xOffset", mXOffset);
      pXml->addAttr("yOffset", mYOffset);
      pXml->addAttr("zOffset", mZOffset);
      pXml->addAttr("xMin", mXMin);
      pXml->addAttr("yMin", mYMin);
      pXml->addAttr("zMin", mZMin);
      pXml->addAttr("xMax", mXMax);
      pXml->addAttr("yMax", mYMax);
      pXml->addAttr("zMax", mZMax);
      pXml->addAttr("spatialDataType", mSpatialDataType);
      pXml->addAttr("hasIntensity", mHasIntensityData); 
      pXml->addAttr("intensityDataType", mIntensityDataType);
      pXml->addAttr("hasClassification", mHasClassificationData); 
      pXml->addAttr("classificationDataType", mClassificationDataType);
   }

   return bSuccess;
}

bool PointCloudDataDescriptorImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   VERIFY(pDocument != NULL);

   bool success = DataDescriptorImp::fromXml(pDocument, version);
   if (!success)
   {
      return false;
   }

   DOMElement* pElement = static_cast<DOMElement*>(pDocument);
   mPointCount = StringUtilities::fromXmlString<double>(
      A(pElement->getAttribute(X("pointCount"))));
   mArrangement = StringUtilities::fromXmlString<PointCloudArrangement>(
      A(pElement->getAttribute(X("arrangement"))));
   mXScale = StringUtilities::fromXmlString<double>(
      A(pElement->getAttribute(X("xScale"))));
   mYScale = StringUtilities::fromXmlString<double>(
      A(pElement->getAttribute(X("yScale"))));
   mZScale = StringUtilities::fromXmlString<double>(
      A(pElement->getAttribute(X("zScale"))));
   mXOffset = StringUtilities::fromXmlString<double>(
      A(pElement->getAttribute(X("xOffset"))));
   mYOffset = StringUtilities::fromXmlString<double>(
      A(pElement->getAttribute(X("yOffset"))));
   mZOffset = StringUtilities::fromXmlString<double>(
      A(pElement->getAttribute(X("zOffset"))));
   mXMin = StringUtilities::fromXmlString<double>(
      A(pElement->getAttribute(X("xMin"))));
   mYMin = StringUtilities::fromXmlString<double>(
      A(pElement->getAttribute(X("yMin"))));
   mZMin = StringUtilities::fromXmlString<double>(
      A(pElement->getAttribute(X("zMin"))));
   mXMax = StringUtilities::fromXmlString<double>(
      A(pElement->getAttribute(X("xMax"))));
   mYMax = StringUtilities::fromXmlString<double>(
      A(pElement->getAttribute(X("yMax"))));
   mZMax = StringUtilities::fromXmlString<double>(
      A(pElement->getAttribute(X("zMax"))));
   mSpatialDataType = StringUtilities::fromXmlString<EncodingType>(
      A(pElement->getAttribute(X("spatialDataType"))));
   mHasIntensityData = StringUtilities::fromXmlString<bool>(
      A(pElement->getAttribute(X("hasIntensity"))));
   mIntensityDataType = StringUtilities::fromXmlString<EncodingType>(
      A(pElement->getAttribute(X("intensityDataType"))));
   mHasClassificationData = StringUtilities::fromXmlString<bool>(
      A(pElement->getAttribute(X("hasClassification"))));
   mClassificationDataType = StringUtilities::fromXmlString<EncodingType>(
      A(pElement->getAttribute(X("classificationDataType"))));

   return success;
}

const string& PointCloudDataDescriptorImp::getObjectType() const
{
   static string sType("PointCloudDataDescriptorImp");
   return sType;
}

bool PointCloudDataDescriptorImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "PointCloudDataDescriptor"))
   {
      return true;
   }

   return DataDescriptorImp::isKindOf(className);
}

void PointCloudDataDescriptorImp::getDataDescriptorTypes(vector<string>& classList)
{
   classList.push_back("PointCloudDataDescriptor");
   DataDescriptorImp::getDataDescriptorTypes(classList);
}

bool PointCloudDataDescriptorImp::isKindOfDataDescriptor(const string& className)
{
   if ((className == "PointCloudDataDescriptorImp") || (className == "PointCloudDataDescriptor"))
   {
      return true;
   }

   return DataDescriptorImp::isKindOfDataDescriptor(className);
}
