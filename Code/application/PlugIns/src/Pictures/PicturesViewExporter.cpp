/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "ColorType.h"
#include "PicturesViewExporter.h"
#include "PlugInArgList.h"
#include "ProductView.h"
#include "Progress.h"
#include "View.h"

#include <QtCore/QPoint>
#include <QtCore/QRect>
#include <QtCore/QSize>
#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QtGui/QRgb>

PicturesViewExporter::PicturesViewExporter(PicturesDetails *pDetails) : PicturesExporter(pDetails)
{
   setName(pDetails->name() + " View Exporter");
   setShortDescription(pDetails->shortDescription() + " View Exporter");
   setDescription(pDetails->description() + " View Exporter");
   setSubtype(TypeConverter::toString<View>());
   setDescriptorId("{6EB61452-CA19-4fbc-9F8D-EB972CF165F8}-" + pDetails->name());
   setProductionStatus(pDetails->isProduction());
}

PicturesViewExporter::~PicturesViewExporter()
{
}

bool PicturesViewExporter::getInputSpecification(PlugInArgList*& pArgList)
{
   VERIFY(PicturesExporter::getInputSpecification(pArgList));
   VERIFY(pArgList != NULL);

   VERIFY(pArgList->addArg<View>(ExportItemArg()));

   return true;
}

bool PicturesViewExporter::extractInputArgs(const PlugInArgList* pInArgList)
{
   if (!PicturesExporter::extractInputArgs(pInArgList))
   {
      return false;
   }
   VERIFY(pInArgList != NULL);

   mpItem = pInArgList->getPlugInArgValue<View>(ExportItemArg());
   return (mpItem != NULL);
}

bool PicturesViewExporter::generateImage(QImage &image)
{
   View* pView = dynamic_cast<View*>(mpItem);
   if (pView == NULL)
   {
      return false;
   }

   if (image.isNull())
   {
      pView->getCurrentImage(image);

      ProductView* pProductView = dynamic_cast<ProductView*>(pView);
      if (pProductView != NULL)
      {
         // For Product View, crop the background if the background is different from the paper color.
         QSize imageSize = image.size();
         QRgb paperColor = COLORTYPE_TO_QCOLOR(pProductView->getPaperColor()).rgb();
         QRgb backgroundColor = COLORTYPE_TO_QCOLOR(pProductView->getBackgroundColor()).rgb();
         if (paperColor != backgroundColor)
         {
            // Find the upper left corner of the paper (or image).
            QPoint upperLeft;
            bool upperLeftFound = false;
            for (int row = 0; row < imageSize.height() && upperLeftFound == false; ++row)
            {
               for (int col = 0; col < imageSize.width() && upperLeftFound == false; ++col)
               {
                  QRgb currentPixel = image.pixel(col, row);
                  if (currentPixel != backgroundColor)
                  {
                     upperLeft.setX(col);
                     upperLeft.setY(row);
                     upperLeftFound = true;
                  }
               }
            }

            // Find the lower right corner of the paper (or image).
            QPoint lowerRight;
            bool lowerRightFound = false;
            for (int row = imageSize.height() - 1; row != 0 && lowerRightFound == false; --row)
            {
               for (int col = imageSize.width() -1; col != 0 && lowerRightFound == false; --col)
               {
                  QRgb currentPixel = image.pixel(col, row);
                  if (currentPixel != backgroundColor)
                  {
                     lowerRight.setX(col);
                     lowerRight.setY(row);
                     lowerRightFound = true;
                  }
               }
            }

            if (upperLeftFound && lowerRightFound)
            {
               image = image.copy(QRect(upperLeft, lowerRight));
            }
         }
      }
   }
   else
   {
      QSize imageSize = image.size();
      QSize subImageSize(512, 512);
      QPoint origin(0, imageSize.height() - subImageSize.height());
      QPainter painter(&image);
      int segment = 0;
      View::SubImageIterator* pSubImage = pView->getSubImageIterator(imageSize, subImageSize);
      int totalX;
      int totalTiles;
      pSubImage->count(totalX, totalTiles);
      totalTiles *= totalX;
      while (pSubImage->hasNext())
      {
         QImage subImage;
         if (!pSubImage->next(subImage))
         {
            break;
         }
         painter.drawImage(origin, subImage);
         int newX = origin.x() + subImage.width();
         int newY = origin.y();
         if (newX >= imageSize.width())
         {
            newY -= subImage.height();
            newX = 0;
         }
         origin = QPoint(newX, newY);
         if (mpProgress != NULL)
         {
            int x;
            int y;
            pSubImage->location(x, y);
            int tileNumber = y * totalX + x;
            QString msg = QString("Processing sub-image %1 of %2...").arg(y * totalX + x).arg(totalTiles);
            mpProgress->updateProgress(msg.toStdString(), 100 * tileNumber / totalTiles - 1, NORMAL);
         }
      }
      delete pSubImage;
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress("Saving product...", 99, NORMAL);
      }
   }

   return true;
}
