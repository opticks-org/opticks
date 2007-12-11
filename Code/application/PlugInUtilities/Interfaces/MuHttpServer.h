/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MUHTTPSERVER_H
#define MUHTTPSERVER_H

#include "EnumWrapper.h"
#include <ehs.h>
#include <QtCore/QMap>
#include <QtCore/QObject>
#include <QtCore/QString>

class QTextStream;
class QTimer;

/**
 * This class provides a framework for creating HTTP micro servers in Qt.
 */
class MuHttpServer : public QObject, public EHS
{
   Q_OBJECT

public:
   /**
    * This structure contains information used to send a response to an HTTP request.
    */
   struct Response
   {
      /**
       * The body of the response message if the response in 8-bit safe. (i.e. ASCII)
       */
      QString mBody;
      
      /**
       * The body of the response message if the response in not 8-bit safe. (i.e. a raw image)
       */
      QByteArray mOctets;

      /**
       * Additional HTTP headers to attach to the response message.
       *
       * The key is the name of the header and the value is the header data.
       */
      QMap<QString,QString> mHeaders;

      /**
       * The type of response.
       *
       * This can be the numeric code defined by the HTTP RFCs or
       * a convenience enum value as defined by the EHS library. (for example, HTTPRESPONSE_200_OK)
       */
      ResponseCode mCode;

      // What character encoding is the response body?
      enum CharacterEncodingTypeEnum
      {
         ASCII, /**< The response body is ASCII encoded and contained in the mBody member. */
         UTF8,  /**< The response body is UTF-8 encoded and contained in the mBody member. */
         OCTET  /**< The response body consists of raw octets and is contained in the mOctets member. */
      };
      
      /**
       * @EnumWrapper MuHttpServer::Response::CharacterEncodingTypeEnum.
       */
      typedef EnumWrapper<CharacterEncodingTypeEnum> CharacterEncodingType;
      
      /**
       * What character encoding is the response body?
       */
      CharacterEncodingType mEncoding;

      /**
       * Create an uninitialized Response with a default encoding type of ASCII.
       */
      Response() : mCode(HTTPRESPONSECODE_INVALID), mEncoding(ASCII) {}
   };

   /**
    * Initialize a MuHttpServer object.
    *
    * @param port
    *        The TCP port that this server will listen. If port is 0, this
    *        object is assumed to be a child handler and no network socket
    *        will be set up.
    * @param pParent
    *        The Qt parent of this object.
    *
    * @see registerPath()
    */
   MuHttpServer(int port = 80, QObject *pParent = NULL);

   /**
    * Clean up the MuHttpServer object and close network sockets.
    */
   virtual ~MuHttpServer();

   /**
    * Listen to the network and start processing requests.
    *
    * @return True if successful or if the server is already running, false if there is an error.
    */
   bool start();

   /**
    * Attach a server path to a response object.
    *
    * Requests from the registered path will be handled by pObj.
    * For example, registerPath("/test", this) will cause the following
    * requests to be handled by the this object.
    *  http://localhost/test/
    *  http://localhost/test/foo
    *  http://localhost/test/bar
    * The following will not be handled by the this object.
    *  http://localhost/test
    *  http://localhost/test/foo/
    *  http://localhost/test/foo/bar
    *  http://localhost/foo/
    *
    * Any paths not explicitly registered will be handled by the root EHS object.
    *
    * @param path
    *        The path prefix for processing.
    * @param pObj
    *        The object that wants to processing path.
    */
   void registerPath(const QString &path, EHS *pObj);

protected:
   /**
    * This handles HTTP POST requests.
    *
    * POST requests encode form data in the request body.
    *
    * The default behavior is to return an HTTP 403 Forbidden response
    * to the requesting client.
    *
    * @param uri
    *        The URI of the request. If this object is handling sub requests
    *        via registerPath(), this will be a partial URI rooted at the path
    *        this object is registered to handle.
    * @param contentType
    *        The HTTP Content-type of the request.
    * @param body
    *        The body of the request.
    * @param rsp
    *        Out param containing an HTTP Response.
    */
   virtual void postRequest(const QString &uri, const QString &contentType, const QString &body, Response &rsp);

   /**
    * This handles HTTP GET requests.
    *
    * GET requests encode form data in the request URL using ? and &.
    *
    * @param uri
    *        The URI of the request. If this object is handling sub requests
    *        via registerPath(), this will be a partial URI rooted at the path
    *        this object is registered to handle.
    * @param contentType
    *        The HTTP Content-type of the request.
    * @param body
    *        The body of the request. This is usually empty with GET requests.
    * @param rsp
    *        Out param containing an HTTP Response.
    */
   virtual void getRequest(const QString &uri, const QString &contentType, const QString &body, Response &rsp) = 0;

protected slots:
   /**
    * Handle new and existing connections.
    *
    * This runs one cycle of the server's request loop.
    * The default behavior of a MuHttpServer is to call this slot
    * every 250ms.
    */
   void processServer();

   /**
    * This provides debugging information about an HttpRequest.
    *
    * The default behavior is to do nothing. If an implementation wants
    * to log all requests for debugging purposes, this method should be overridden.
    *
    * @param pHttpRequest
    *        The request object.
    */
   virtual void debug(HttpRequest *pHttpRequest);

   /**
    * Display a waning message.
    *
    * The default behavior is to do nothing. If an implementation wants
    * to display extra warning messages, this method should be overridden.
    * This method is called when the underlying HTTP server code throws an error
    * during socket setup, conntection, or processing. It is also called when
    * postRequest() or getRequest() return an HTTPRESPONSECODE_INVALID indicating
    * an internal server error.
    *
    * @param msg
    *        A user displayable warning message.
    */
   virtual void warning(const QString &msg);

   /**
    * Allow connections from machines other than localhost.
    *
    * The default is to allow only localhost connections.
    *
    * @param val
    *        If true, any machine can connect. If false, only localhost can connect.
    */
   void allowNonLocalConnections(bool val)
   {
      mAllowNonLocal = val;
   }

private:
   ResponseCode HandleRequest(HttpRequest *pHttpRequest, HttpResponse *pHttpResponse);

   EHSServerParameters mParams;
   QTimer *mpTimer;
   QMap<QString, EHS*> mRegistrations;
   bool mServerIsRunning;
   bool mAllowNonLocal;
};

#endif
