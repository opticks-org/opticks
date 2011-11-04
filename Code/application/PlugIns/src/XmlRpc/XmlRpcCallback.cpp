/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "MessageLogResource.h"
#include "UtilityServices.h"
#include "xmlreader.h"
#include "XmlRpc.h"
#include "XmlRpcArrayParam.h"
#include "XmlRpcCallback.h"
#include "XmlRpcStructParam.h"

#include <list>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QTextStream>
#include <QtCore/QUrl>
#include <QtNetwork/QHttp>
#include <string>
#include <QtCore/QtDebug>

using namespace std;
XERCES_CPP_NAMESPACE_USE

//#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : Remove qDebug() calls " \
//   "after debugging is complete (tclarke)")

XmlRpcCallback::XmlRpcCallback(const QString &url,
                               const QString &method,
                               const XmlRpcStructParam &params,
                               QObject *pParent) : QObject(pParent), mUrl(url), mMethod(method), mpHttpConnection(NULL)
{
}

XmlRpcCallback::~XmlRpcCallback()
{
   delete mpHttpConnection;
}

void XmlRpcCallback::call(const XmlRpcParams &params) const
{
   XmlRpcMethodCall call(mMethod, params);
   QUrl url(mUrl);
   if (mpHttpConnection == NULL)
   {
      mpHttpConnection = new QHttp(url.host(), url.port(), const_cast<XmlRpcCallback*>(this));
      VERIFYNR(connect(mpHttpConnection, SIGNAL(requestFinished(int, bool)),
         this, SLOT(processRequestFinished(int, bool))));
      VERIFYNR(connect(mpHttpConnection, SIGNAL(readyRead(const QHttpResponseHeader&)),
         this, SLOT(processResponseHeader(const QHttpResponseHeader&))));
   }

   QByteArray rsp((call.toXml()).toUtf8());
   qDebug() << rsp;
   QString path = url.path();
   if (path.isEmpty())
   {
      path = "/";
   }
   QHttpRequestHeader header("POST", path);
   header.setContentType("text/xml");
   header.setContentLength(rsp.size());
   header.setValue("Host", QString("%1:%2").arg(url.host()).arg(url.port()));
   header.setValue("User-Agent", APP_NAME);
   mpHttpConnection->request(header, rsp);
}

void XmlRpcCallback::processRequestFinished(int, bool error)
{
   if (error)
   {
      QString errorMessage = "Unknown error message";
      if (mpHttpConnection != NULL)
      {
         errorMessage = mpHttpConnection->errorString();
      }
      MessageResource msg("Connection Error", "app", "9247DAFA-BA4F-4C9A-A460-7E2D1AEC70BC");
      msg->addProperty("message", errorMessage.toStdString());
      callComplete();
   }
}

void XmlRpcCallback::processResponseHeader(const QHttpResponseHeader &header)
{
   int code = header.statusCode();
   if (code >= 400 && code < 600) // 40x is a client error and 50x is a server error
   {
      MessageResource msg("HTTP Error", "app", "9655BB26-90AF-4CCC-A2FD-19DC48CA309E");
      msg->addProperty("message", header.reasonPhrase().toStdString());
   }
   else if (code >= 200 && code < 300) // 20x is a success
   {
      try
      {
         XmlReader xml(Service<UtilityServices>()->getMessageLog()->getLog(), false);
         string str = mpHttpConnection->readAll().constData();
         XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument *pDoc = xml.parseString(str);
         DOMElement* pRoot = NULL;
         if (pDoc != NULL)
         {
            pRoot = pDoc->getDocumentElement();
         }
         if (pRoot == NULL)
         {
            throw ""; // send us to the error handling code
         }
         XmlRpcMethodResponse response(pRoot);
         const XmlRpcParam* pRval = response.returnValue();
         if (pRval == NULL || !pRval->value().toBool())
         {
            MessageResource msg("RPC Failed", "app", "1AE1C9B3-8AEE-40F4-856B-8CC8BC58F49E");
            msg->addProperty("message", "The XML-RPC method call failed.");
         }
      }
      catch (...)
      {
         MessageResource msg("Response Parse Error", "app", "75313F9F-FF5F-4E2A-8385-7FBD70F662DE");
         msg->addProperty("message", "Unable to parse response.");
      }
   }
   else
   {
      MessageResource msg("Unknown response code", "app", "967B1838-864F-4B72-A4C5-A3D487958623");
      msg->addProperty("message",
         QString("Unknown response %1: %2").arg(code).arg(header.reasonPhrase()).toStdString());
   }
   callComplete();
}
