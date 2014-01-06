/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "AppConfig.h"
#include "ConnectionParameters.h"
#include "CoordinateTransformation.h"
#include "AppVerify.h"
#include "FeatureManager.h"
#include "FeatureProxyConnector.h"
#include "PlugInManagerServices.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QProcess>
#include <QtCore/QTextStream>
#include <QtNetwork/QLocalServer>
#include <QtNetwork/QLocalSocket>

#include <QtCore/QtDebug>

#include <vector>

#if defined(WIN_API)
#include <Windows.h>
#endif

FeatureProxyConnector *FeatureProxyConnector::instance()
{
   Service<PlugInManagerServices> pPlugInManager;
   std::vector<PlugIn*> proxyInstances = pPlugInManager->getPlugInInstances(FeatureManager::PLUGIN_NAME);
   VERIFY(proxyInstances.size() == 1); // we should never have more than 1 proxy manager

   FeatureManager* pManager = dynamic_cast<FeatureManager*>(proxyInstances.front());
   VERIFY(pManager != NULL);

   return pManager->getProxy();
}

FeatureProxyConnector::FeatureProxyConnector(const QString &executable, QObject *pParent) :
   QObject(pParent),
   mPendingCommand(NO_COMMAND),
   mInitialized(false),
   mExecutable(executable),
   mpServer(NULL),
   mpSocket(NULL),
   mpConnectionTimer(NULL),
   mLastReplyIsError(false)
{
   mpProcess = new QProcess(this);
   VERIFYNR(connect(mpProcess, SIGNAL(finished(int, QProcess::ExitStatus)),
      this, SLOT(proxyExited())));
   VERIFYNR(connect(&mShapelibProxy, SIGNAL(featureLoaded(const ArcProxyLib::Feature&)),
      this, SIGNAL(featureLoaded(const ArcProxyLib::Feature&))));
}

FeatureProxyConnector::~FeatureProxyConnector()
{
   VERIFYNR(mCoordinateTransformations.empty());
   terminate();
   if (mpServer != NULL)
   {
      mpServer->close();
   }
   if (mpSocket != NULL)
   {
      mpSocket->close();
   }
}

bool FeatureProxyConnector::initialize()
{
   long pid = -1;
#if defined(WIN_API)
   pid = GetCurrentProcessId();
#endif

   mpConnectionTimer = new QTimer(this);
   mpConnectionTimer->setSingleShot(true);
   int connectionTimeout = FeatureProxyConnector::getSettingConnectionTimeout();
   mpConnectionTimer->setInterval(connectionTimeout);
   VERIFYNR(connect(mpConnectionTimer, SIGNAL(timeout()), this, SLOT(abortConnection())));

   if (mpProcess->state() != QProcess::NotRunning)
   {
      terminate();
   }
   mpServer = new QLocalServer(this);
   if (!mpServer->listen("OpticksFeatureProxyConnector" + QString::number(pid)))
   {
      terminate();
      return false;
   }

   QStringList args;
   if (pid >= 0)
   {
      args << "-pid" << QString::number(pid);
   }

   mpProcess->start(mExecutable, args);
   if (!mpProcess->waitForStarted())
   {
      terminate();
      return false;
   }

   if (!mpServer->waitForNewConnection(3000))
   {
      terminate();
      return false;
   }
   mpSocket = mpServer->nextPendingConnection();
   mpServer->close();
   mStream.setDevice(mpSocket);
   connect(mpSocket, SIGNAL(readyRead()), this, SLOT(processReply()));
   mStream << APP_VERSION_NUMBER << endl;

   return true;
}

void FeatureProxyConnector::abortConnection()
{
   mResponses.enqueue("ERROR ArcProxy could not connect.");
   mLastReplyIsError = true;
   mPendingCommand = NO_COMMAND; 
}

bool FeatureProxyConnector::processReply()
{
   mLastReplyIsError = false;
   QString data = mPartialResponse + mStream.readAll();
   QStringList lines = data.split("\n", QString::SkipEmptyParts);
   if (data[data.size()-1] != '\n')
   {
      mPartialResponse = lines.takeLast();
   }
   else
   {
      mPartialResponse.clear();
   }

   while (!mLastReplyIsError && !lines.empty())
   {
      QString line = lines.takeFirst();
      if (!mInitialized)
      {
         if (line != APP_VERSION_NUMBER)
         {
            terminate();
            return false;
         }
         mInitialized = true;
         emit initialized();
      }
      else
      {
         QStringList args = line.split(" ", QString::SkipEmptyParts);
         if (!args.empty())
         {
            if (args.front() == "ERROR")
            {
               args.pop_front();
               mResponses.clear();
               mLastReplyIsError = true;
               QString response = args.join(" ");
               mResponses.enqueue(response);
               emit error(response);
            }
            else
            {
               switch (mPendingCommand)
               {
               case OPEN_DATA_SOURCE:
                  if (args.front() == "SUCCESS")
                  {
                     args.pop_front();
                     QString response = args.join(" ");
                     mResponses.enqueue(response);
                     emit dataSourceOpen(response);
                  }
                  break;
               case CLOSE_DATA_SOURCE:
                  if (args.front() == "SUCCESS")
                  {
                     args.pop_front();
                     QString response = args.join(" ");
                     mResponses.enqueue(response);
                     emit dataSourceClosed(response);
                  }
                  break;
               case GET_FEATURE_CLASS_PROPERTIES:
                  if (args.front() == "SUCCESS" && args.size() >= 2)
                  {
                     args.pop_front();
                     QString response = args.join(" ");
                     mResponses.enqueue(response);
                     emit featureClassProperties(response);
                  }
                  break;
               case QUERY:
                  mResponses.enqueue(args.join(" "));
                  if (args.front() != "END")
                  {
                     continue; // don't reset mPendingCommand until we hit the end
                  }
                  break;
               default:
                  mResponses.enqueue(args.join(" "));
                  break;
               }
            }
         }
         mPendingCommand = NO_COMMAND;
      }
   }
   return true;
}

bool FeatureProxyConnector::terminate()
{
   bool success = true;
   mResponses.clear();
   if (mpProcess->state() == QProcess::Running)
   {
      mStream << "END" << endl;
      QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
      success = mpProcess->waitForFinished(500);
      if (!success)
      {
         mpProcess->terminate();
      }
   }
   return success;
}

bool FeatureProxyConnector::openDataSource(const ArcProxyLib::ConnectionParameters &connParams, std::string &handle,
                                    std::string &errorMessage)
{
   if (connParams.getConnectionType() == ArcProxyLib::SHAPELIB_CONNECTION)
   {
      if (mShapelibProxy.openDataSource(connParams, handle, errorMessage) == false)
      {
         return false;
      }

      mCoordinateTransformations[handle] = new CoordinateTransformation(connParams);
      return true;
   }

   if (mpProcess->state() != QProcess::Running)
   {
      errorMessage = "ArcGIS must be installed to use that connection type.";
      return false;
   }
   mResponses.clear();
   mLastReplyIsError = false;
   mStream << "OPEN " << QString::fromStdString(connParams.toString()) << endl;
   mPendingCommand = OPEN_DATA_SOURCE;
   VERIFY(mpConnectionTimer != NULL);
   mpConnectionTimer->start();

   // run the event loop until we get some sort of a reply
   while (mPendingCommand == OPEN_DATA_SOURCE)
   {
      QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
   }
   mpConnectionTimer->stop();
   if (mLastReplyIsError)
   {
      errorMessage = mResponses.dequeue().toStdString();
      return false;
   }

   handle = mResponses.dequeue().toStdString();
   if (connParams.getConnectionType() == ArcProxyLib::SHP_CONNECTION) // TODO: Test with Arc database to determine if this 'if' is needed!
   {
      mCoordinateTransformations[handle] = new CoordinateTransformation(connParams);
   }

   return true;
}

bool FeatureProxyConnector::closeDataSource(const std::string &handle, std::string &errorMessage)
{
   std::map<std::string, CoordinateTransformation*>::iterator iter = mCoordinateTransformations.find(handle);
   if (iter != mCoordinateTransformations.end())
   {
      delete iter->second;
      mCoordinateTransformations.erase(iter);
   }

   if (mShapelibProxy.containsHandle(handle))
   {
      return mShapelibProxy.closeDataSource(handle, errorMessage);
   }

   if (mpProcess->state() == QProcess::Running)
   {
      mResponses.clear();
      mLastReplyIsError = false;
      mStream << "CLOSE " << QString::fromStdString(handle) << endl;
      mPendingCommand = CLOSE_DATA_SOURCE;
      VERIFY(mpConnectionTimer != NULL);
      mpConnectionTimer->start();

      while (mPendingCommand == CLOSE_DATA_SOURCE)
      {
         QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
      }
      mpConnectionTimer->stop();
      if (mLastReplyIsError)
      {
         errorMessage = mResponses.dequeue().toStdString();
      }
      return !mLastReplyIsError;
   }
   return false;
}

bool FeatureProxyConnector::getFeatureClassProperties(
   const std::string &handle, ArcProxyLib::FeatureClassProperties &properties,
   std::string &errorMessage)
{
   if (mShapelibProxy.containsHandle(handle))
   {
      return mShapelibProxy.getFeatureClassProperties(handle, properties, errorMessage);
   }

   if (mpProcess->state() != QProcess::Running)
   {
      return false;
   }
   mResponses.clear();
   mLastReplyIsError = false;
   mStream << "GETFEATURECLASSPROPERTIES " << QString::fromStdString(handle) << endl;
   mPendingCommand = GET_FEATURE_CLASS_PROPERTIES;
   VERIFY(mpConnectionTimer != NULL);
   mpConnectionTimer->start();

   while (mPendingCommand == GET_FEATURE_CLASS_PROPERTIES)
   {
      QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
   }
   mpConnectionTimer->stop();

   if (mLastReplyIsError)
   {
      errorMessage = mResponses.dequeue().toStdString();
      return false;
   }
   properties.fromString(mResponses.dequeue().toStdString());
   return true;
}

bool FeatureProxyConnector::query(const std::string &handle, 
   std::string &errorMessage, const std::string &whereClause, const std::string &labelFormat,
   LocationType minClip, LocationType maxClip)
{
   CoordinateTransformation* pCoordinateTransformation = NULL;
   std::map<std::string, CoordinateTransformation*>::const_iterator iter =
      mCoordinateTransformations.find(handle);
   if (iter != mCoordinateTransformations.end())
   {
      pCoordinateTransformation = iter->second;
   }

   if (mShapelibProxy.containsHandle(handle))
   {
      return mShapelibProxy.query(handle, errorMessage, whereClause,
         labelFormat, minClip, maxClip, pCoordinateTransformation);
   }

   if (mpProcess->state() != QProcess::Running)
   {
      return false;
   }
   mResponses.clear();
   mLastReplyIsError = false;
   QString clipString;
   if (minClip.mX != 0.0 || minClip.mY != 0.0 || maxClip.mX != 0.0 || maxClip.mY != 0.0)
   {
      // Convert from application coordinates to shapefile coordinates.
      if (pCoordinateTransformation != NULL)
      {
         if (pCoordinateTransformation->translateAppToShape(minClip.mY, minClip.mX, minClip.mY, minClip.mX) == false ||
            pCoordinateTransformation->translateAppToShape(maxClip.mY, maxClip.mX, maxClip.mY, maxClip.mX) == false)
         {
            errorMessage = "Coordinate transformation failed for clipping region. Check the .prj file and try again.";
            return false;
         }
      }

      clipString = QString(" CLIP %1 %2 %3 %4").arg(minClip.mY)
                                              .arg(minClip.mX)
                                              .arg(maxClip.mY)
                                              .arg(maxClip.mX);
   }
   QString whereString;
   if (!whereClause.empty())
   {
      whereString = QString(" WHERE %1").arg(
         QUrl::toPercentEncoding(QString::fromStdString(whereClause)).constData());
   }
   QString labelString;
   if (!labelFormat.empty())
   {
      labelString = QString(" LABELFORMAT %1").arg(
         QUrl::toPercentEncoding(QString::fromStdString(labelFormat)).constData());
   }

   mStream << "QUERY " << QString::fromStdString(handle) << clipString << whereString << labelString << endl;
   mPendingCommand = QUERY;

   bool readingResponses = true;
   QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
   while (!mLastReplyIsError && readingResponses)
   {
      while (!mResponses.empty())
      {
         QString response = mResponses.dequeue();
         if (response == "END")
         {
            readingResponses = false;
            break;
         }
         ArcProxyLib::Feature feature;
         feature.fromString(response);
         emit featureLoaded(feature);
      }
      QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
   }
   if (mLastReplyIsError)
   {
      errorMessage = mResponses.dequeue().toStdString();
   }
   return !mLastReplyIsError;
}

bool FeatureProxyConnector::isInitialized() const
{
   return mInitialized;
}

void FeatureProxyConnector::proxyExited()
{
   mResponses.enqueue("ERROR ArcProxy has exited.");
   mLastReplyIsError = true;
   mPendingCommand = NO_COMMAND;
}

std::vector<ArcProxyLib::ConnectionType> 
   FeatureProxyConnector::getAvailableConnectionTypes() const
{
   std::vector<ArcProxyLib::ConnectionType> types;

   if (mpProcess->state() == QProcess::Running)
   {
      types.push_back(ArcProxyLib::SHP_CONNECTION); // should be first to prefer SHP over SHAPELIB
      types.push_back(ArcProxyLib::SDE_CONNECTION);
   }

   types.push_back(ArcProxyLib::SHAPELIB_CONNECTION);

   return types;
}
