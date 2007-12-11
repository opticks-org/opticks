/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ImageHandler.h"
#include "Layer.h"
#include "SessionManager.h"
#include "SpatialDataView.h"

#include <QtCore/QBuffer>
#include <QtCore/QUrl>
#include <QtGui/QColor>
#include <QtGui/QImage>
#include <QtGui/QImageWriter>

// The encoded image size can not excede this number of bytes.
#define MAX_ENCODED_IMAGE_SIZE (5 * 1024 * 1024)

ImageHandler::ImageHandler(QObject *pParent) : MuHttpServer(0, pParent)
{
}

ImageHandler::ImageHandler(int port, QObject *pParent) : MuHttpServer(port, pParent)
{
}

ImageHandler::~ImageHandler()
{
}

void ImageHandler::getRequest(const QString &uri, const QString &contentType, const QString &body, MuHttpServer::Response &rsp)
{
   QStringList splitUri = uri.split(".");
   QString format = "PNG";
   if(!splitUri.isEmpty())
   {
      format = splitUri.back().toUpper();
   }
   splitUri.pop_back();
   if(format == "JPG")
   {
      format = "JPEG";
   }
   bool success = !format.isEmpty();
   if(success)
   {
      QString itemId = QUrl::fromPercentEncoding(splitUri.join(".").toAscii());
      SessionItem *pItem = Service<SessionManager>()->getSessionItem(itemId.toStdString());
      QImage image;
      Layer *pLayer = dynamic_cast<Layer*>(pItem);
      View *pView = dynamic_cast<View*>(pItem);
      if(pLayer != NULL)
      {
         SpatialDataView *pSDView = dynamic_cast<SpatialDataView*>(pLayer->getView());
         if(pSDView != NULL)
         {
            int bbox[4];
            QColor t = Qt::transparent;
            ColorType transparent = QCOLOR_TO_COLORTYPE(t);
            success = pSDView->getLayerImage(pLayer, image, transparent, bbox);
         }
      }
      else if(pView != NULL)
      {
         success = pView->getCurrentImage(image);
      }
      else
      {
         success = false;
      }
      if(success)
      {
         rsp.mOctets.resize(MAX_ENCODED_IMAGE_SIZE);
         QBuffer buf(&rsp.mOctets);
         buf.open(QIODevice::WriteOnly);
         QImageWriter writer(&buf, format.toAscii());
         success = writer.write(image);
         if(success)
         {
            rsp.mCode = HTTPRESPONSECODE_200_OK;
            rsp.mHeaders["content-type"] = QString("image/%1").arg(format.toLower());
            rsp.mOctets.truncate(buf.pos()+1);
            rsp.mEncoding = Response::OCTET;
         }
      }
   }
   if(!success)
   {
      rsp.mCode = HTTPRESPONSECODE_404_NOTFOUND;
      rsp.mHeaders["content-type"] = "text/html";
      rsp.mBody = "<html><body><h1>Not found</h1>The requested document can not be located or the requested image format is not supported.</body></html>";
   }
}
