/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef FEATURECLASSPROPERTIES_H
#define FEATURECLASSPROPERTIES_H

#include "Feature.h"

#include <algorithm>
#include <string>
#include <vector>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QUrl>

namespace
{
   class PercentEncodeAppend
   {
   public:
      PercentEncodeAppend(const std::string &separator) : mSeparator(separator)
      {
      }

      void operator()(const std::string &raw)
      {
         mBase += QUrl::toPercentEncoding(QString::fromStdString(raw)).constData() + mSeparator;
      }

      std::string mBase;
      std::string mSeparator;
   };

   class PercentDecode
   {
   public:
      std::string operator()(const QString &encoded)
      {
         return QUrl::fromPercentEncoding(encoded.toAscii()).toStdString();
      }
   };
}

namespace ArcProxyLib
{
   class FeatureClassProperties
   {
   public:
      FeatureClassProperties() : mFeatureType(UNKNOWN), mFeatureCount(0)
      {
      }

      bool fromString(const std::string &featureClassString)
      {
         QStringList pairs = QString::fromStdString(featureClassString).split(" ");

         mFeatureType = UNKNOWN;
         for(QStringList::const_iterator pit = pairs.begin(); pit != pairs.end(); ++pit)
         {
            QStringList tmp = pit->split("=");
            if(tmp.size() == 2)
            {
               if(tmp[0] == "TYPE")
               {
                  if (tmp[1] == "POINT")
                  {
                     mFeatureType = POINT;
                  }
                  else if (tmp[1] == "MULTIPOINT")
                  {
                     mFeatureType = MULTIPOINT;
                  }
                  else if (tmp[1] == "POLYLINE")
                  {
                     mFeatureType = POLYLINE;
                  }
                  else if (tmp[1] == "POLYGON")
                  {
                     mFeatureType = POLYGON;
                  }
               }
               else if (tmp[0] == "FIELDS")
               {
                  QStringList fields = tmp[1].split(",", QString::SkipEmptyParts);
                  mFields.resize(fields.size());
                  std::transform(fields.begin(), fields.end(), mFields.begin(), PercentDecode());
               }
               else if (tmp[0] == "TYPES")
               {
                  QStringList types = tmp[1].split(",", QString::SkipEmptyParts);
                  mTypes.resize(types.size());
                  std::transform(types.begin(), types.end(), mTypes.begin(), PercentDecode());
               }
               else if (tmp[0] == "SAMPLEVALUES")
               {
                  QStringList sampleValues = tmp[1].split(",", QString::SkipEmptyParts);
                  mSampleValues.resize(sampleValues.size());
                  std::transform(sampleValues.begin(), sampleValues.end(), mSampleValues.begin(), PercentDecode());
               }
               else if (tmp[0] == "COUNT")
               {
                  mFeatureCount = tmp[1].toLongLong();
               }
            }
         }

         return true;
      }

      std::string toString() const
      {
         std::string propertiesString = "TYPE=";

         switch (mFeatureType)
         {
         case POINT:
            propertiesString += "POINT";
            break;
         case MULTIPOINT:
            propertiesString += "MULTIPOINT";
            break;
         case POLYLINE:
            propertiesString += "POLYLINE";
            break;
         case POLYGON:
            propertiesString += "POLYGON";
            break;
         default:
            propertiesString += "UNKNOWN";
            break;
         }

         propertiesString += " FIELDS=";
         propertiesString += std::for_each(mFields.begin(), mFields.end(), PercentEncodeAppend(",")).mBase;

         propertiesString += " TYPES=";
         propertiesString += std::for_each(mTypes.begin(), mTypes.end(), PercentEncodeAppend(",")).mBase;

         propertiesString += " SAMPLEVALUES=";
         propertiesString += std::for_each(mSampleValues.begin(), mSampleValues.end(), PercentEncodeAppend(",")).mBase;

         QString count = QString(" COUNT=%1").arg(mFeatureCount);
         propertiesString += count.toStdString();

         return propertiesString;
      }

      FeatureType getFeatureType() const { return mFeatureType; }
      void setFeatureType(FeatureType featureType) { mFeatureType = featureType;}
      std::vector<std::string> getFields() const { return mFields; }
      void setFields(const std::vector<std::string> &fields) { mFields = fields; }
      std::vector<std::string> getTypes() const { return mTypes; }
      void setTypes(const std::vector<std::string> &types) { mTypes = types; }
      std::vector<std::string> getSampleValues() const { return mSampleValues; }
      void setSampleValues(const std::vector<std::string> &sampleValues) { mSampleValues = sampleValues; }
      qlonglong getFeatureCount() const { return mFeatureCount; }
      void setFeatureCount(qlonglong featureCount) { mFeatureCount = featureCount; }

   private:
      FeatureType mFeatureType;
      std::vector<std::string> mFields;
      std::vector<std::string> mTypes;
      std::vector<std::string> mSampleValues;
      qlonglong mFeatureCount;

   };
}

#endif