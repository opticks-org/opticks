/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef FEATUREPROXYCONNECTOR_H
#define FEATUREPROXYCONNECTOR_H

#include "EnumWrapper.h"
#include "Feature.h"
#include "ConnectionParameters.h"
#include "FeatureClassProperties.h"
#include "LocationType.h"
#include "ShapelibProxy.h"
#include "ConfigurationSettings.h"

#include <QtCore/QObject>
#include <QtCore/QQueue>
#include <QtCore/QString>
#include <QtCore/QTextStream>
#include <QtCore/QTimer>
#include <string>
#include <vector>

class QProcess;
class QTcpServer;
class QTcpSocket;

/**
 *  This manages the connection one or more feature proxies, currently the
 *  separate ArcProxy executable and the ShapelibProxy.
 *
 *  When communicating with the ArcProxy, this class implements a C++ api to 
 *  the ArcProxy communication protocol and manages the life of the ArcProxy 
 *  process. This protocol is a text based protocol with one command per line 
 *  as detailed below.
 *
 *  The first line sent to ArcProxy is the version string of this build.
 *  ArcProxy verifies this is a supported version and responds with the
 *  version string ArcProxy was built with. Currently, these versions need to
 *  be identical.
 *
 *  As a response to any command, ArcProxy may return an error defined as:
 *  ERROR This is the error string to display to the user.
 *
 *  Commands to ArcProxy consist of a verb, optional arguments, and a newline.
 *  Responses consist of one or more lines. When more than one line is applicable,
 *  response completion is communicated by sending END followed by a new line.
 *  Valid verbs are as follows.
 *
 *  END
 *        This ends the ArcProxy session, closing the process down. There is no reply.
 *  OPEN <string>
 *        This will open a new feature class. <string> should be generated with
 *        ArcProxyLib::ConnectionParameters::toString().  The response
 *        will be an error or SUCCESS UID  where UID is a string identifying the connection.
 *  CLOSE UID
 *        This will close an open feature class. UID must be a feature class id returned from OPEN.
 *        The reply is an error or SUCCESS UID  where UID with the feature class id that was closed.
 *  GETFEATURECLASSPROPERTIES UID
 *        This will return the number of features in an open feature class denoted by UID.
 *        The reply is an error or SUCCESS <string> where <string> is a string which can be parsed by
 *        ArcProxyLib::FeatureClassProperties::fromString().
 *  QUERY UID Arguments
 *        This will perform a query on an open feature class denoted by UID. Arguments can be specified to
 *        control the query parameters and are optional. The reply is a sequence of features detailed below.
 *        Currently supported Arguments follow.
 *    CLIP minX minY maxX maxY
 *         This limits the request to features which fall within the envelope defined by the parameters.
 *         The parameters are latitude and longitude specified in the WGS84 system.
 *  Responses to a QUERY. Each will be a feature or an error. A response of END indicates the termination
 *  of the response stream. Each feature begins with the feature type followed by one or more coordinates.
 *  Coordinates are latitude/longitude in WGS84. A coordinate may also be PATH indicating that the next
 *  coordinate begins a new path or segment. An optional LABEL parameter will be indicated with LABEL=<label>.
 *  Features can be:
 *     POINT [label] coordinate
 *     MULTIPOINT [label] coordinate ...
 *     POLYLINE [label] coordinate ...
 *     POLYGON [label] coordinate ...
 */
class FeatureProxyConnector : public QObject
{
   Q_OBJECT

public:

   SETTING(ConnectionTimeout, FeatureProxyConnector, int, 5000)

   static FeatureProxyConnector *instance();

   FeatureProxyConnector(const QString &executable, QObject *pParent = NULL);
   ~FeatureProxyConnector();

   bool initialize();
   bool terminate();
   bool openDataSource(const ArcProxyLib::ConnectionParameters &connParams, 
      std::string &handle, std::string &errorMessage);
   bool closeDataSource(const std::string &handle, std::string &errorMessage);
   bool getFeatureClassProperties(const std::string &handle, 
      ArcProxyLib::FeatureClassProperties &properties, std::string &errorMessage);
   bool query(const std::string &handle, std::string &errorMessage,
      const std::string &whereClause = "", const std::string &labelFormat = "",
      LocationType minClip = LocationType(), LocationType maxClip = LocationType());
   bool isInitialized() const;

   std::vector<ArcProxyLib::ConnectionType> getAvailableConnectionTypes() const;

signals:
   void error(const QString &errorMessage);
   void initialized();
   void dataSourceOpen(const QString &handle);
   void dataSourceClosed(const QString &handle);
   void featureClassProperties(const QString &properties);
   void features(std::vector<std::string> features);
   void featureLoaded(const ArcProxyLib::Feature &feature);

private slots:
   bool processReply();
   void abortConnection();
   void proxyExited();

private:
   enum CommandsTypeEnum
   {
      NO_COMMAND, OPEN_DATA_SOURCE, CLOSE_DATA_SOURCE, GET_FEATURE_CLASS_PROPERTIES, QUERY
   };

   /**
    * @EnumWrapper FeatureProxyConnector::CommandsTypeEnum.
    */
   typedef EnumWrapper<CommandsTypeEnum> CommandsType;

   CommandsType mPendingCommand;
   bool mInitialized;
   QString mExecutable;
   QProcess* mpProcess;
   QTextStream mStream;
   QTcpServer* mpServer;
   QTcpSocket* mpSocket;
   QTimer* mpConnectionTimer;

   QQueue<QString> mResponses;
   QString mPartialResponse;
   bool mLastReplyIsError;

   ShapelibProxy mShapelibProxy;
};

#endif
