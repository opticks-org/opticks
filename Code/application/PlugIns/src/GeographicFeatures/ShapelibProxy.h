/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SHAPELIBPROXY_H
#define SHAPELIBPROXY_H

#include "Feature.h"
#include "ConnectionParameters.h"
#include "FeatureClassProperties.h"
#include "LocationType.h"

#include <QtCore/QObject>

#include <shapefil.h>

#include <string>
#include <vector>
#include <map>


class ShapelibProxy : public QObject
{
   Q_OBJECT

public:
   ShapelibProxy();
   virtual ~ShapelibProxy();

   bool containsHandle(const std::string &handle) const;

   bool openDataSource(const ArcProxyLib::ConnectionParameters &connParams, 
      std::string &handle, std::string &errorMessage);
   bool closeDataSource(const std::string &handle, std::string &errorMessage);
   bool getFeatureClassProperties(const std::string &handle, 
      ArcProxyLib::FeatureClassProperties &properties, std::string &errorMessage);
   bool query(const std::string &handle, std::string &errorMessage,
      const std::string &whereClause = "", const std::string &labelFormat = "",
      LocationType minClip = LocationType(), LocationType maxClip = LocationType());

   class ShapelibHandle
   {
   public:
      ShapelibHandle() : mShpHandle(NULL), mDbfHandle(NULL)
      {
      }

      ShapelibHandle(const std::string& filename)
      {
         mShpHandle = SHPOpen(filename.c_str(), "rb");
         mDbfHandle = DBFOpen(filename.c_str(), "rb");
      }

      ShapelibHandle(const ShapelibHandle& rhs) :
         mShpHandle(rhs.mShpHandle),
         mDbfHandle(rhs.mDbfHandle)
      {
      }

      void close()
      {
         if (mShpHandle != NULL)
         {
            SHPClose(mShpHandle);
            mShpHandle = NULL;
         }
         if (mDbfHandle != NULL)
         {
            DBFClose(mDbfHandle);
            mDbfHandle = NULL;
         }
      }

      bool isValid() const
      {
         return (mShpHandle != NULL && mDbfHandle != NULL);
      }

      SHPHandle getShpHandle() const
      {
         return mShpHandle;
      }

      DBFHandle getDbfHandle() const
      {
         return mDbfHandle;
      }

   private:
      SHPHandle mShpHandle;
      DBFHandle mDbfHandle;
   };

   static std::string preprocessFormatString(const ShapelibHandle &handle, 
      const std::string &formatString);
   static bool getTypeAndCount(const ShapelibHandle &handle, 
      std::string &errorMessage, ArcProxyLib::FeatureType &featureType, int &count);
   static bool getFieldAttributes(const ShapelibHandle &handle, std::string &name, 
      std::string &type, std::string &value, int field, int feature);

signals:
   void featureLoaded(const ArcProxyLib::Feature &feature);

private:
   ShapelibProxy(const ShapelibProxy& rhs);
   ShapelibProxy& operator=(const ShapelibProxy& rhs);

   std::map<std::string, ShapelibHandle> mHandles;

};

#endif
