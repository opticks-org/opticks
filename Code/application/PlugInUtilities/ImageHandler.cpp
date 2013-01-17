/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "DimensionDescriptor.h"
#include "ImageHandler.h"
#include "Layer.h"
#include "SessionManager.h"
#include "SpatialDataView.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterLayer.h"
#include "StringUtilities.h"
#include "Undo.h"

#include <QtCore/QBuffer>
#include <QtCore/QUrl>
#include <QtGui/QColor>
#include <QtGui/QImage>
#include <QtGui/QImageWriter>
#include <QtGui/QPainter>
#include <QtGui/QWidget>

// The encoded image size can not excede this number of bytes.
#define MAX_ENCODED_IMAGE_SIZE (5 * 1024 * 1024)

ImageHandler::ImageHandler(QObject* pParent) :
   MuHttpServer(0, pParent)
{}

ImageHandler::ImageHandler(int port, QObject* pParent) :
   MuHttpServer(port, pParent)
{}

ImageHandler::~ImageHandler()
{}

MuHttpServer::Response ImageHandler::getRequest(const QString& uri, const QString& contentType,
                                                const QString& body, const FormValueMap& form)
{
   Response r;
   QStringList splitUri = uri.split(".");
   QString format = "PNG";
   if (!splitUri.isEmpty())
   {
      format = splitUri.back().toUpper();
   }
   splitUri.pop_back();
   if (format == "JPG")
   {
      format = "JPEG";
   }
   bool success = !format.isEmpty();
   if (success)
   {
      QString itemId = QUrl::fromPercentEncoding(splitUri.join(".").toAscii());
      SessionItem* pItem = Service<SessionManager>()->getSessionItem(itemId.toStdString());
      r.mOctets.resize(MAX_ENCODED_IMAGE_SIZE);
      QBuffer buffer(&r.mOctets);
      int band = -1;
      FormValueMap::const_iterator it;
      if ((it = form.find("band")) != form.end() || (it = form.find("frame")) != form.end())
      {
         band = StringUtilities::fromXmlString<int>(it->second.m_sBody);
      }

      success = getSessionItemImage(pItem, buffer, format, band);
      if (success == true)
      {
         r.mCode = HTTPRESPONSECODE_200_OK;
         r.mHeaders["content-type"] = QString("image/%1").arg(format.toLower());
         r.mOctets.truncate(buffer.pos()+1);
         r.mEncoding = Response::OCTET;
      }
   }
   if (!success)
   {
      r.mCode = HTTPRESPONSECODE_404_NOTFOUND;
      r.mHeaders["content-type"] = "text/html";
      r.mBody = "<html><body><h1>Not found</h1>The requested document can not be located or the requested image "
                "format is not supported.</body></html>";
   }
   return r;
}

bool ImageHandler::getSessionItemImage(SessionItem* pItem, QBuffer& buffer, const QString& format, int band, int* pBbox)
{
   if (format.isEmpty())
   {
      return false;
   }
   bool success = true;
   QImage image;
   Layer* pLayer = dynamic_cast<Layer*>(pItem);
   View* pView = dynamic_cast<View*>(pItem);
   if (pLayer != NULL)
   {
      SpatialDataView* pSDView = dynamic_cast<SpatialDataView*>(pLayer->getView());
      if (pSDView != NULL)
      {
         UndoLock ulock(pSDView);
         DimensionDescriptor cur;
         DisplayMode mode;
         RasterLayer* pRasterLayer = dynamic_cast<RasterLayer*>(pLayer);
         if (band >= 0 && pRasterLayer != NULL)
         {
            RasterElement* pRaster = pRasterLayer->getDisplayedRasterElement(GRAY);
            DimensionDescriptor bandDesc =
               static_cast<RasterDataDescriptor*>(pRaster->getDataDescriptor())->getActiveBand(band);
            cur = pRasterLayer->getDisplayedBand(GRAY);
            mode = pRasterLayer->getDisplayMode();
            pRasterLayer->setDisplayedBand(GRAY, bandDesc);
            pRasterLayer->setDisplayMode(GRAYSCALE_MODE);
         }
         int bbox[4] = {0, 0, 0, 0};
         QColor t = Qt::transparent;
         ColorType realTransparent = QCOLOR_TO_COLORTYPE(t);
         {
            QImage dummyImage;
            int dummyBbox[4] = {0,0,0,0};
            pSDView->getLayerImage(pLayer, dummyImage, realTransparent, dummyBbox);
         }
         ColorType transparent(255, 255, 254);
         success = pSDView->getLayerImage(pLayer, image, transparent, bbox);
         if (pBbox != NULL)
         {
            memcpy(pBbox, bbox, sizeof(bbox));
         }
         QImage alphaChannel(image.size(), QImage::Format_Indexed8);
         if (image.hasAlphaChannel())
         {
            alphaChannel = image.alphaChannel();
         }
         else
         {
            alphaChannel.fill(0xff);
         }
         QRgb realTransColor = COLORTYPE_TO_QCOLOR(realTransparent).rgb();
         QRgb transColor = COLORTYPE_TO_QCOLOR(transparent).rgb();
         for (int y = 0; y < image.height(); y++)
         {
            for (int x = 0; x < image.width(); x++)
            {
               if (image.pixel(x, y) == transColor || image.pixel(x, y) == realTransColor)
               {
                  alphaChannel.setPixel(x, y, 0x00);
               }
            }
         }
         image.setAlphaChannel(alphaChannel);
         if (mode.isValid())
         {
            pRasterLayer->setDisplayedBand(GRAY, cur);
            pRasterLayer->setDisplayMode(mode);
         }
      }
   }
   else if (pView != NULL)
   {
      success = pView->getCurrentImage(image);
   }
   else
   {
      success = false;
   }
   if (success)
   {
      buffer.open(QIODevice::WriteOnly);
      QImageWriter writer(&buffer, format.toAscii());
      success = writer.write(image);
   }
   return success;
}
