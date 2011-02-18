/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
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
         double dMinX = 0.0;
         double dMinY = 0.0;
         double dMaxX = 0.0;
         double dMaxY = 0.0;

         bool topFound = false;
         bool bottomFound = false;
         bool leftFound = false;
         bool rightFound = false;

         double dExtentXMin;
         double dExtentYMin;
         double dExtentXMax;
         double dExtentYMax;

         pProductView->getExtents(dMinX, dMinY, dMaxX, dMaxY);

         // In order to export the product view image correctly, we must first remove the view's extents.
         // We can do this by multiplying the difference between the max and min values by the value
         // we get from using this formula.
         double dOffset = 0.03 / 1.03;
         double dMarginX = (dMaxX - dMinX) * dOffset;
         double dMarginY = (dMaxY - dMinY) * dOffset;

         QPoint topLeft(0, 0);
         QPoint bottomRight(image.width(), image.height());

         pProductView->translateWorldToScreen(dMinX + dMarginX, dMinY + dMarginY, dExtentXMin, dExtentYMin);
         pProductView->translateWorldToScreen(dMaxX - dMarginX, dMaxY - dMarginY, dExtentXMax, dExtentYMax);

         dExtentYMin = image.height() - dExtentYMin;
         dExtentYMax = image.height() - dExtentYMax;

         if (dExtentXMin > 0.0)
         {
            topLeft.setX(dExtentXMin);
            leftFound = true;
         }
         if (dExtentXMax < image.width())
         {
            bottomRight.setX(dExtentXMax);
            rightFound = true;
         }
         if (dExtentYMin > 0.0)
         {
            topLeft.setY(dExtentYMin);
            topFound = true;
         }
         if (dExtentYMax < image.height())
         {
            bottomRight.setY(dExtentYMax);
            bottomFound = true;
         }

         if (topFound || leftFound || bottomFound || rightFound)
         {
            image = image.copy(QRect(topLeft, bottomRight));
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
