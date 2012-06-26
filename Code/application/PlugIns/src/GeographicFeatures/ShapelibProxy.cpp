/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "FormatStringProcessor.h"
#include "ShapelibProxy.h"

#include <QtCore/QDir>
#include <QtCore/QRectF>
#include <QtCore/QUuid>

#include <boost/lexical_cast.hpp>

namespace
{
   class ShapelibFormatStringPreprocessor : public ArcProxyLib::FormatStringPreprocessor
   {
   public:
      ShapelibFormatStringPreprocessor(const ShapelibProxy::ShapelibHandle &handle, std::string &preprocessedString) : 
         FormatStringPreprocessor(preprocessedString), mHandle(handle)
      {
      }
      virtual ~ShapelibFormatStringPreprocessor() {};

      int getFieldIndex(const std::string &fieldName) const
      {
         return DBFGetFieldIndex(mHandle.getDbfHandle(), fieldName.c_str());
      }

   private:
      ShapelibFormatStringPreprocessor& operator=(const ShapelibFormatStringPreprocessor& rhs);

      const ShapelibProxy::ShapelibHandle& mHandle;
   };

   class ShapelibFormatStringProcessor : public ArcProxyLib::FormatStringProcessor
   {
   public:
      ShapelibFormatStringProcessor(const ShapelibProxy::ShapelibHandle &handle, int feature) :
         mHandle(handle), mFeature(feature)
      {
      }
      virtual ~ShapelibFormatStringProcessor() {};

      std::string getFieldValue(int fieldNumber) const
      {
         std::string name;
         std::string type;
         std::string value;
         ShapelibProxy::getFieldAttributes(mHandle, name, type, value, fieldNumber-1, mFeature);
         return value;
      }

   private:
      ShapelibFormatStringProcessor& operator=(const ShapelibFormatStringProcessor& rhs);

      const ShapelibProxy::ShapelibHandle& mHandle;
      int mFeature;
   };
}

ShapelibProxy::ShapelibProxy()
{
}

ShapelibProxy::~ShapelibProxy()
{
}

bool ShapelibProxy::containsHandle(const std::string &handle) const
{
   return (mHandles.find(handle) != mHandles.end());
}

bool ShapelibProxy::openDataSource(const ArcProxyLib::ConnectionParameters &connParams, 
                                   std::string &handle, std::string &errorMessage)
{
   if (connParams.getConnectionType() != ArcProxyLib::SHAPELIB_CONNECTION)
   {
      errorMessage = "Unknown connection type";
      return false;
   }

   QDir dir(QString::fromStdString(connParams.getDatabase()));
   QString filename = dir.absoluteFilePath(QString::fromStdString(connParams.getFeatureClass()));

   ShapelibHandle shapelibHandle(filename.toStdString());
   if (!shapelibHandle.isValid())
   {
      shapelibHandle.close();
      errorMessage = "Could not open filename";
      return false;
   }

   handle = QUuid::createUuid().toString().toStdString();
   mHandles[handle] = shapelibHandle;

   return true;
}

bool ShapelibProxy::closeDataSource(const std::string &handle, std::string &errorMessage)
{
   std::map<std::string, ShapelibHandle>::iterator iter = mHandles.find(handle);
   if (iter == mHandles.end())
   {
      errorMessage = "Could not find handle";
      return false;
   }

   iter->second.close();
   mHandles.erase(iter);

   return true;
}

bool ShapelibProxy::getFeatureClassProperties(const std::string &handle, 
   ArcProxyLib::FeatureClassProperties &properties, std::string &errorMessage)
{
   std::map<std::string, ShapelibHandle>::iterator iter = mHandles.find(handle);
   if (iter == mHandles.end())
   {
      errorMessage = "Could not find handle";
      return false;
   }

   ShapelibHandle& shapelibHandle = iter->second;

   int count = 0;
   ArcProxyLib::FeatureType featureType = ArcProxyLib::UNKNOWN;
   if (!getTypeAndCount(shapelibHandle, errorMessage, featureType, count))
   {
      return false;
   }
   
   properties.setFeatureCount(count);
   properties.setFeatureType(featureType);

   std::vector<std::string> fields;
   std::vector<std::string> types;
   std::vector<std::string> sampleValues;

   // fields
   int fieldCount = DBFGetFieldCount(shapelibHandle.getDbfHandle());
   for (int i = 0; i < fieldCount; ++i)
   {
      std::string name;
      std::string type;
      std::string value;
      if (getFieldAttributes(shapelibHandle, name, type, value, i, 0))
      {
         fields.push_back(name);
         types.push_back(type);
         sampleValues.push_back(value);
      }
   }

   properties.setFields(fields);
   properties.setTypes(types);
   properties.setSampleValues(sampleValues);

   return true;
}

bool ShapelibProxy::query(const std::string &handle, std::string &errorMessage, 
   const std::string &whereClause, const std::string &labelFormat,
   LocationType minClip, LocationType maxClip)
{
   std::map<std::string, ShapelibHandle>::iterator iter = mHandles.find(handle);
   if (iter == mHandles.end())
   {
      errorMessage = "Could not find handle";
      return false;
   }
   ShapelibHandle& shapelibHandle = iter->second;

   if (!whereClause.empty())
   {
      errorMessage = "Shapelib does not support where clauses";
      return false;
   }

   std::string formattedLabel = preprocessFormatString(shapelibHandle, labelFormat);

   int count = 0;
   ArcProxyLib::FeatureType featureType = ArcProxyLib::UNKNOWN;
   if (!getTypeAndCount(shapelibHandle, errorMessage, featureType, count))
   {
      return false;
   }

   for (int i = 0; i < count; ++i)
   {
      SHPObject* pShpObject = SHPReadObject(shapelibHandle.getShpHandle(), i);
      if (pShpObject == NULL)
      {
         continue;
      }

      QRectF objectRect(QPointF(pShpObject->dfYMin, pShpObject->dfXMin),
         QPointF(pShpObject->dfYMax, pShpObject->dfXMax));
      QRectF clipRect(QPointF(minClip.mX, minClip.mY), QPointF(maxClip.mX, maxClip.mY));

      if ((clipRect.isEmpty() == true) ||
         ((objectRect.isEmpty() == true) && (clipRect.contains(objectRect.topLeft()) == true)) ||
         (objectRect.intersects(clipRect) == true))
      {
         ArcProxyLib::Feature feature;
         feature.setType(featureType);

         for (int vertex = 0; vertex < pShpObject->nVertices; ++vertex)
         {
            feature.addVertex(std::make_pair(pShpObject->padfY[vertex], pShpObject->padfX[vertex]));
         }

         for (int path = 0; path < pShpObject->nParts; ++path)
         {
            feature.addPathAtIndex(pShpObject->panPartStart[path]);
         }

         feature.setLabel(std::for_each(formattedLabel.begin(), formattedLabel.end(), ShapelibFormatStringProcessor(
            shapelibHandle, i)).getProcessedString());
         
         emit featureLoaded(feature);
      }

      SHPDestroyObject(pShpObject);
   }

   return true;
}

bool ShapelibProxy::getTypeAndCount(const ShapelibHandle &handle, std::string &errorMessage, 
                                    ArcProxyLib::FeatureType &featureType, int &count)
{
   int type;
   SHPGetInfo(handle.getShpHandle(), &count, &type, NULL, NULL);

   featureType = ArcProxyLib::UNKNOWN;

   switch (type)
   {
   case SHPT_POINT:  // fall through
   case SHPT_POINTZ:
      featureType = ArcProxyLib::POINT;
      break;
   case SHPT_MULTIPOINT: // fall through
   case SHPT_MULTIPOINTZ:
      featureType = ArcProxyLib::MULTIPOINT;
      break;
   case SHPT_ARC: // fall through
   case SHPT_ARCZ:
      featureType = ArcProxyLib::POLYLINE;
      break;
   case SHPT_POLYGON: // fall through
   case SHPT_POLYGONZ:
      featureType = ArcProxyLib::POLYGON;
      break;
   default:
      errorMessage = "Unknown shape type";
      return false;
   }

   return true;

}

bool ShapelibProxy::getFieldAttributes(const ShapelibHandle &handle, std::string &name,
                                       std::string &type, std::string &value, int field, int feature)
{
   char cname[12]; // defined as maximum length
   DBFFieldType fieldType = DBFGetFieldInfo(handle.getDbfHandle(), field, cname, NULL, NULL);
   
   name = cname;

   switch (fieldType)
   {
   case FTString:
      type = "String";
      value = DBFReadStringAttribute(handle.getDbfHandle(), feature, field);
      break;
   case FTInteger:
      type = "Integer";
      value = boost::lexical_cast<std::string>(
         DBFReadIntegerAttribute(handle.getDbfHandle(), feature, field));
      break;
   case FTDouble:
      type = "Double";
      value = boost::lexical_cast<std::string>(
         DBFReadDoubleAttribute(handle.getDbfHandle(), feature, field));
      break;
   default:
      type = "Unknown";
      value = "[Error: Not stringable]";
      return false;
   }

   return true;
}

std::string ShapelibProxy::preprocessFormatString(const ShapelibHandle &handle, const std::string &formatString)
{
   if (handle.getDbfHandle() == NULL)
   {
      return "";
   }

   std::string preprocessedString;
   ShapelibFormatStringPreprocessor preprocessor(handle, preprocessedString);
   std::for_each(formatString.begin(), formatString.end(), preprocessor);
   return preprocessedString;
}
