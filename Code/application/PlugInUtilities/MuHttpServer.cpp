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

   // Destroy registrations here before the EHS class destructor is called
   for (QMap<QString, EHS*>::iterator it = mRegistrations.begin(); it != mRegistrations.end(); ++it)
   {
      delete it.value();
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

   try
   {
      StartServer(mParams);
      mpTimer->start();
   }
   catch (...)
   {
      warning("Could not start the server.");
      return false;
   }

   for (QMap<QString, EHS*>::iterator it = mRegistrations.begin(); it != mRegistrations.end(); ++it)
   {
      RegisterEHS(it.value(), it.key().toAscii());
   }

   mServerIsRunning = true;
   mSession.reset(Service<SessionManager>().get());
   return true;
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

   if (!mAllowNonLocal && pHttpRequest->RemoteAddress() != "127.0.0.1")
   {
      QString errorString = QString("<html><body><h1>Forbidden</h1>"
         "Connection from %1 has been blocked. Only localhost connections are allowed.</body></html>")
         .arg(QString::fromStdString(pHttpRequest->RemoteAddress()));
      pHttpResponse->SetBody(errorString.toAscii(), errorString.size());
      warning(errorString);
      return HTTPRESPONSECODE_403_FORBIDDEN;
   }

   QString uri = QString::fromStdString(pHttpRequest->Uri()).split("?")[0];
   if (pHttpRequest->Method() == REQUESTMETHOD_GET || pHttpRequest->Method() == REQUESTMETHOD_POST)
   {
      QString contentType = pHttpRequest->Headers("content-type").c_str();
      QString body = pHttpRequest->Body().c_str();
      Response rsp = (pHttpRequest->Method() == REQUESTMETHOD_GET) ?
         getRequest(uri, contentType, body, pHttpRequest->FormValues()) :
         postRequest(uri, contentType, body, pHttpRequest->FormValues());
      if (rsp.mCode != HTTPRESPONSECODE_INVALID && rsp.mEncoding.isValid())
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
            pHttpResponse->SetHeader(headerIt.key().toStdString(), headerIt.value().toStdString());
         }
         return rsp.mCode;
      }
   }
   // default to responding with an internal server error
   std::string errorString = "<html><body><h1>Internal server error</h1>An unknown error occured.</body></html>";
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
