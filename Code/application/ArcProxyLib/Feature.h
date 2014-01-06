/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef FEATURE_H__
#define FEATURE_H__

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QUrl>

#include <utility>
#include <vector>

namespace ArcProxyLib
{
   enum FeatureType { POINT, MULTIPOINT, POLYLINE, POLYGON, UNKNOWN };

   class Feature
   {
   public:
      Feature() :
         mType(UNKNOWN)
      {}

      Feature(const Feature& feature) :
         mType(feature.mType),
         mVertices(feature.mVertices),
         mPaths(feature.mPaths),
         mAttributeValues(feature.mAttributeValues),
         mLabel(feature.mLabel)
      {}

      ~Feature() {}

      FeatureType getType() const { return mType; }
      void setType(FeatureType type) { mType = type; }
      const std::string& getLabel() const { return mLabel; }
      void setLabel(const std::string &label) { mLabel = label; }
      const std::vector<std::string>& getAttributes() const { return mAttributeValues; }
      void setAttributes(const std::vector<std::string>& attributes) { mAttributeValues = attributes; }
      const std::vector<std::pair<double, double> > &getVertices() const { return mVertices; }
      const std::vector<size_t> &getPaths() const { return mPaths; }

      void addVertex(std::pair<double, double> v) { mVertices.push_back(v); }
      void setVertices(const std::vector<std::pair<double, double> >& v) { mVertices = v; }
      void startNewPath() { mPaths.push_back(mVertices.size()); }
      void addPathAtIndex(unsigned int path) { mPaths.push_back(path); }

      void fromString(const QString &featureString)
      {
         QStringList featureArgs = featureString.split(" ", QString::SkipEmptyParts);

         mType = UNKNOWN;
         if (featureArgs.front() == "POINT")
         {
            mType = POINT;
         }
         else if (featureArgs.front() == "MULTIPOINT")
         {
            mType = MULTIPOINT;
         }
         else if (featureArgs.front() == "POLYLINE")
         {
            mType = POLYLINE;
         }
         else if (featureArgs.front() == "POLYGON")
         {
            mType = POLYGON;
         }
         for (featureArgs.pop_front(); !featureArgs.empty(); featureArgs.pop_front())
         {
            if (featureArgs.front() == "PATH")
            {
               startNewPath();
            }
            else if (featureArgs.front().startsWith("ATTRIBUTES"))
            {
               mAttributeValues.clear();
               QStringList attributes = featureArgs.front().split("=");
               if (attributes.size() == 2)
               {
                  QString endOfText(" ");
                  endOfText[0] = 3;
                  QStringList attributeValues = attributes[1].split(endOfText);
                  if (attributeValues.empty() == false)
                  {
                     int numValues = attributeValues.size();
                     mAttributeValues.resize(numValues);

                     for (int i = 0; i < numValues; ++i)
                     {
                        mAttributeValues[i] = QUrl::fromPercentEncoding(attributeValues[i].toAscii()).toStdString();
                     }
                  }
               }
            }
            else if (featureArgs.front().startsWith("LABEL"))
            {
               QStringList label = featureArgs.front().split("=");
               if (label.size() == 2)
               {
                  mLabel = QUrl::fromPercentEncoding(label[1].toAscii()).toStdString();
               }
            }
            else
            {
               QStringList coords = featureArgs.front().split(",");
               if (coords.size() == 2)
               {
                  addVertex(std::make_pair(coords[0].toDouble(), coords[1].toDouble()));
               }
            }
         }
      }

   private:
      FeatureType mType;
      std::vector<std::pair<double, double> > mVertices;
      std::vector<size_t> mPaths;
      std::vector<std::string> mAttributeValues;
      std::string mLabel;
   };
}

#endif
