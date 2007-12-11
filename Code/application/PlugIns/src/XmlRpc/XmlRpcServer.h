/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef XMLRPCSERVER_H
#define XMLRPCSERVER_H

#include "AlgorithmShell.h"
#include "ConfigurationSettings.h"
#include "MuHttpServer.h"

class XmlRpcMethodCall;
class XmlRpcMethodCallImp;

class XmlRpcServer : public MuHttpServer, public AlgorithmShell
{
   Q_OBJECT

public:
   SETTING(XmlRpcServerPort, XmlRpc, int, 0);

   XmlRpcServer();
   ~XmlRpcServer();

   bool setBatch()
   {
      AlgorithmShell::setBatch();
      return false;
   }
   bool getInputSpecification(PlugInArgList *&pInArgList);
   bool getOutputSpecification(PlugInArgList *&pOutArgList);
   bool execute(PlugInArgList *pInArgList, PlugInArgList *pOutArgList);

protected:
   void registerMethodCall(const QString &name, XmlRpcMethodCallImp *pMethodCall);
   void getRequest(const QString &uri, const QString &contentType, const QString &body, MuHttpServer::Response &rsp);
   void postRequest(const QString &uri, const QString &contentType, const QString &body, MuHttpServer::Response &rsp);
   void processMethodCall(const XmlRpcMethodCall &call, MuHttpServer::Response &rsp);

protected slots:
   void debug(HttpRequest *pHttpRequest);
   void warning(const QString &msg);

private:
   QMap<QString, XmlRpcMethodCallImp*> mMethodCalls;
};

#endif
