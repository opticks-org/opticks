/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef IMAGEHANDLER_H
#define IMAGEHANDLER_H

#include "MuHttpServer.h"

class QString;

/**
 * Publish view and layer images via HTTP
 */
class ImageHandler : public MuHttpServer
{
   Q_OBJECT

public:
   /**
    * Construct a new ImageHandler.
    *
    * @param pParent
    *        Qt parent object
    */
   ImageHandler(QObject *pParent = NULL);

   /**
    * Construct a new ImageHandler.
    *
    * @param port
    *        TCP port where the server should listen. If this is 0, a server will
    *        not be started. This is used to add the ImageHandler to an existing
    *        server using MuHttpServer::registerPath().
    * @param pParent
    *        Qt parent object
    */
   ImageHandler(int port, QObject *pParent = NULL);

   /**
    * Destructor
    */
   ~ImageHandler();

protected:
   /**
    * @copydoc MuHttpServer::getRequest()
    *
    * This method serves view and layer images. The request URL should be
    * the SessionItem ID of the view or layer. The file extension represents
    * the format of the image generated. Any image format supported by the current
    * build of Qt is supported. This requires at least PNG support. If no extension
    * is specified, PNG is assumed.
    */
   void getRequest(const QString &uri, const QString &contentType, const QString &body, MuHttpServer::Response &rsp);
};

#endif
