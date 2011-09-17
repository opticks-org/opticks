/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CONNECTIONPARAMETERS_H__
#define CONNECTIONPARAMETERS_H__

#include <string>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QUrl>

#ifndef ARC_PROXY
#include "DataVariant.h"
#include "DynamicObject.h"
#include "ObjectResource.h"
#endif

namespace ArcProxyLib
{
   enum ConnectionType
   {
      UNKNOWN_CONNECTION,
      SDE_CONNECTION,
      SHP_CONNECTION,
      SHAPELIB_CONNECTION
   };

   class ConnectionParameters
   {
   public:

      ConnectionParameters() : mConnectionType(UNKNOWN_CONNECTION)
      {
      }

      ConnectionParameters(const ConnectionParameters &rhs) :
         mUser(rhs.mUser),
         mPassword(rhs.mPassword),
         mDatabase(rhs.mDatabase),
         mServer(rhs.mServer),
         mInstance(rhs.mInstance),
         mVersion(rhs.mVersion),
         mFeatureClass(rhs.mFeatureClass),
         mConnectionType(rhs.mConnectionType)
      {
      }
      
      ~ConnectionParameters()
      {
      }

      bool operator==(const ConnectionParameters &rhs) const
      {
         return (mUser == rhs.mUser && mPassword == rhs.mPassword &&
            mDatabase == rhs.mDatabase && mServer == rhs.mServer &&
            mInstance == rhs.mInstance && mVersion == rhs.mVersion &&
            mFeatureClass == rhs.mFeatureClass && mConnectionType == rhs.mConnectionType);

      }

      bool operator!=(const ConnectionParameters &rhs) const
      {
         return !operator==(rhs);
      }

      void fromString (const std::string &connectionString)
      {
         QStringList pairs = QString::fromStdString(connectionString).split(" ");

         for(QStringList::const_iterator pit = pairs.begin(); pit != pairs.end(); ++pit)
         {
            QStringList tmp = pit->split("=");
            if(tmp.size() == 2)
            {
               if(tmp[0] == "USER")
               {
                  mUser = QUrl::fromPercentEncoding(tmp[1].toAscii()).toStdString();
               }
               else if(tmp[0] == "PASSWORD")
               {
                  mPassword = QUrl::fromPercentEncoding(tmp[1].toAscii()).toStdString();
               }
               else if(tmp[0] == "DATABASE")
               {
                  mDatabase = QUrl::fromPercentEncoding(tmp[1].toAscii()).toStdString();
               }
               else if(tmp[0] == "SERVER")
               {
                  mServer = QUrl::fromPercentEncoding(tmp[1].toAscii()).toStdString();
               }
               else if(tmp[0] == "INSTANCE")
               {
                  mInstance = QUrl::fromPercentEncoding(tmp[1].toAscii()).toStdString();
               }
               else if(tmp[0] == "VERSION")
               {
                  mVersion = QUrl::fromPercentEncoding(tmp[1].toAscii()).toStdString();
               }
               else if(tmp[0] == "FEATURECLASS")
               {
                  mFeatureClass = QUrl::fromPercentEncoding(tmp[1].toAscii()).toStdString();
               }
               else if(tmp[0] == "CONNECTIONTYPE")
               {
                  mConnectionType = static_cast<ConnectionType>(tmp[1].toInt());
               }
            }
         }
      }

      std::string toString() const
      {
         QStringList pairs;
         if(!mUser.empty())
         {
            pairs << QString("USER=%1").arg(QUrl::toPercentEncoding(mUser.c_str()).constData());
         }
         if(!mPassword.empty())
         {
            pairs << QString("PASSWORD=%1").arg(QUrl::toPercentEncoding(mPassword.c_str()).constData());
         }
         if(!mDatabase.empty())
         {
            pairs << QString("DATABASE=%1").arg(QUrl::toPercentEncoding(mDatabase.c_str()).constData());
         }
         if(!mServer.empty())
         {
            pairs << QString("SERVER=%1").arg(QUrl::toPercentEncoding(mServer.c_str()).constData());
         }
         if(!mInstance.empty())
         {
            pairs << QString("INSTANCE=%1").arg(QUrl::toPercentEncoding(mInstance.c_str()).constData());
         }
         if(!mVersion.empty())
         {
            pairs << QString("VERSION=%1").arg(QUrl::toPercentEncoding(mVersion.c_str()).constData());
         }
         if(!mFeatureClass.empty())
         {
            pairs << QString("FEATURECLASS=%1").arg(QUrl::toPercentEncoding(mFeatureClass.c_str()).constData());
         }
         
         pairs << QString("CONNECTIONTYPE=%1").arg(static_cast<int>(mConnectionType));

         return pairs.join(" ").toStdString();
      }

      std::string getUser() const { return mUser; }
      std::string getPassword() const { return mPassword; }
      std::string getDatabase() const { return mDatabase; }
      std::string getServer() const { return mServer; }
      std::string getInstance() const { return mInstance; }
      std::string getVersion() const { return mVersion; }
      std::string getFeatureClass() const { return mFeatureClass; }

      void setUser(const std::string &user) { mUser = user; }
      void setPassword(const std::string &password) { mPassword = password; }
      void setDatabase(const std::string &database) { mDatabase = database; }
      void setServer(const std::string &server) { mServer = server; }
      void setInstance(const std::string &instance) { mInstance = instance; }
      void setVersion(const std::string &version) { mVersion = version; }
      void setFeatureClass(const std::string &featureClass) { mFeatureClass = featureClass; }

      void setConnectionType(ConnectionType connectionType)
      {
         mConnectionType = connectionType;
      }

      ConnectionType getConnectionType() const
      {
         return mConnectionType;
      }

   #ifndef ARC_PROXY
   public:
      FactoryResource<DynamicObject> toDynamicObject() const
      {
         FactoryResource<DynamicObject> pDynObj;

         pDynObj->setAttribute("user", mUser);
         pDynObj->setAttribute("password", mPassword);
         pDynObj->setAttribute("database", mDatabase);
         pDynObj->setAttribute("server", mServer);
         pDynObj->setAttribute("instance", mInstance);
         pDynObj->setAttribute("version", mVersion);
         pDynObj->setAttribute("featureClass", mFeatureClass);
         pDynObj->setAttribute("connectionType", static_cast<int>(mConnectionType));

         return pDynObj;
      }

      bool fromDynamicObject(const DynamicObject *pDynObj)
      {
         if (pDynObj == NULL)
         {
            return false;
         }

         pDynObj->getAttribute("user").getValue(mUser);
         pDynObj->getAttribute("password").getValue(mPassword);
         pDynObj->getAttribute("database").getValue(mDatabase);
         pDynObj->getAttribute("server").getValue(mServer);
         pDynObj->getAttribute("instance").getValue(mInstance);
         pDynObj->getAttribute("version").getValue(mVersion);
         pDynObj->getAttribute("featureClass").getValue(mFeatureClass);
         int connectionType;
         if (pDynObj->getAttribute("connectionType").getValue(connectionType))
         {
            mConnectionType = static_cast<ConnectionType>(connectionType);
         }

         return true;
      }

   #endif

   private:
      std::string mUser;
      std::string mPassword;
      std::string mDatabase;
      std::string mServer;
      std::string mInstance;
      std::string mVersion;
      std::string mFeatureClass;
      ConnectionType mConnectionType;
   };
}

#endif