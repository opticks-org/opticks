/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "MuHttpServer.h"
#include "Slot.h"
#include <ehs.h>
#include <QtCore/QDebug>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QTimer>

MuHttpServer::MuHttpServer(int port, QObject *pParent) :
   QObject(pParent),
   mpTimer(NULL),
   mServerIsRunning(false),
   mAllowNonLocal(false),
   mSession(SIGNAL_NAME(SessionManager, Closed), Slot(this, &MuHttpServer::stop))
{
   if (port > 0)
   {
      setObjectName("mu HTTP server");
      mParams["port"] = port;
      mParams["mode"] = "singlethreaded";
      mpTimer = new QTimer(this);
      mpTimer->setSingleShot(false);
      mpTimer->setInterval(250);
      connect(mpTimer, SIGNAL(timeout()), this, SLOT(processServer()));
   }
}

MuHttpServer::~MuHttpServer()
{
   if (mServerIsRunning)
   {
      StopServer();
   }
}

bool MuHttpServer::start()
{
   if (mParams.empty())
   {
      warning("Invalid parameters.");
      return false;
   }
   if (mServerIsRunning)
   {
      return true;
   }
   switch (StartServer(mParams))
   {
   case STARTSERVER_SUCCESS:
      mpTimer->start(); // fall through
   case STARTSERVER_ALREADYRUNNING:
      for (QMap<QString, EHS*>::iterator it = mRegistrations.begin(); it != mRegistrations.end(); ++it)
      {
         RegisterEHS(it.value(), it.key().toAscii());
      }
      mRegistrations.clear();
      mServerIsRunning = true;
      mSession.reset(Service<SessionManager>().get());
      return true;
   case STARTSERVER_INVALID:
      warning("Invalid server specification.");
      break;
   case STARTSERVER_NODATASPECIFIED:
      warning("No server data specified.");
      break;
   case STARTSERVER_SOCKETSNOTINITIALIZED:
      warning("Unable to initialize server socket.");
      break;
   case STARTSERVER_THREADCREATIONFAILED:
      warning("Unable to create server threads.");
      break;
   case STARTSERVER_FAILED:
   default:
      warning("Server failed for an unknown reason.");
      break;
   }
   return false;
}

void MuHttpServer::stop(Subject &subject, const std::string &signal, const boost::any &v)
{
   StopServer();
   mSession.reset(NULL);
   mServerIsRunning = false;
   mpTimer->stop();
}

void MuHttpServer::registerPath(const QString &path, EHS *pObj)
{
   if (mServerIsRunning)
   {
      RegisterEHS(pObj, path.toAscii());
   }
   else
   {
      mRegistrations[path] = pObj;
   }
}

void MuHttpServer::processServer()
{
   HandleData(0);
}

ResponseCode MuHttpServer::HandleRequest(HttpRequest *pHttpRequest, HttpResponse *pHttpResponse)
{
   debug(pHttpRequest);

   if (!mAllowNonLocal && pHttpRequest->GetAddress() != "127.0.0.1")
   {
      QString errorString = QString("<html><body><h1>Forbidden</h1>"
         "Connection from %1 has been blocked. Only localhost connections are allowed.</body></html>")
         .arg(QString::fromStdString(pHttpRequest->GetAddress()));
      pHttpResponse->SetBody(errorString.toAscii(), errorString.size());
      warning(errorString);
      return HTTPRESPONSECODE_403_FORBIDDEN;
   }

   QString uri = QString::fromStdString(pHttpRequest->sUri).split("?")[0];
   if (pHttpRequest->nRequestMethod == REQUESTMETHOD_GET || pHttpRequest->nRequestMethod == REQUESTMETHOD_POST)
   {
      QString contentType = pHttpRequest->oRequestHeaders["content-type"].c_str();
      QString body = pHttpRequest->sBody.c_str();
      Response rsp = (pHttpRequest->nRequestMethod == REQUESTMETHOD_GET) ?
         getRequest(uri, contentType, body, pHttpRequest->oFormValueMap) :
         postRequest(uri, contentType, body, pHttpRequest->oFormValueMap);
      if (rsp.mCode != HTTPRESPONSECODE_INVALID)
      {
         switch (rsp.mEncoding)
         {
         case Response::ASCII:
            pHttpResponse->SetBody(rsp.mBody.toAscii(), rsp.mBody.toAscii().length());
            break;
         case Response::UTF8:
            pHttpResponse->SetBody(rsp.mBody.toUtf8(), rsp.mBody.toUtf8().length());
            break;
         case Response::OCTET:
            pHttpResponse->SetBody(rsp.mOctets.constData(), rsp.mOctets.length());
            break;
         }
         QMapIterator<QString,QString> headerIt(rsp.mHeaders);
         while (headerIt.hasNext())
         {
            headerIt.next();
            pHttpResponse->oResponseHeaders[headerIt.key().toStdString()] = headerIt.value().toStdString();
         }
         return rsp.mCode;
      }
   }
   // default to responding with an internal server error
   string errorString = "<html><body><h1>Internal server error</h1>An unknown error occured.</body></html>";
   pHttpResponse->SetBody(errorString.c_str(), errorString.size());
   warning(QString::fromStdString(errorString));
   return HTTPRESPONSECODE_500_INTERNALSERVERERROR;
}

MuHttpServer::Response MuHttpServer::postRequest(const QString& uri, const QString& contentType,
                                                 const QString& body, const FormValueMap& form)
{
   Response r;
   r.mCode = HTTPRESPONSECODE_403_FORBIDDEN;
   r.mHeaders["content-type"] = "text/html";
   r.mBody = QString("<html><body><h1>Forbidden</h1>POST not supported.</body></html>");
   return r;
}

void MuHttpServer::debug(HttpRequest* pHttpRequest)
{
}

void MuHttpServer::warning(const QString& msg)
{
}
