/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QPainter>
#include <QtGui/QPrintDialog>
#include <QtGui/QPrinter>
#include <QtGui/QWidget>

#include "PrintPixmap.h"

/* Prints a QPixmap, optionally displaying the print setup dialog first */
void PrintPixmap(QPixmap pixmap, bool displayDialog, QWidget *pParent, QPrinter *pPrinter)
{
   bool deletePrinter = false;
   if (pixmap.isNull() == true)
   {
      return;
   }

   // Create a printer device. Analogous to a Windows device context
   if (pPrinter == NULL)
   {
      deletePrinter = true;
      pPrinter = new QPrinter();
      if (pPrinter == NULL)
      {
         return;
      }
   }

   bool bPrint = true;
   if (displayDialog)
   {
      QPrintDialog dlg(pPrinter, pParent);
      if (dlg.exec() == QDialog::Rejected)
      {
         bPrint = false;
      }
   }

   if (bPrint == true)
   {
      double dAspect = 1.0; // the aspect ratio of the pixmap = width / height
      dAspect = static_cast<double>(pixmap.width()) / static_cast<double>(pixmap.height());

      // the QPainter provides an interface for drawing to a device, analogous
      // to Windows GDI, with the printer being the device context in this case
      QPainter p;
      if (p.begin(pPrinter) == false)
      {
         if (deletePrinter)
         {
            delete pPrinter;
         }

         return;
      }

      QRect rcViewport = p.viewport(); // the printable area in device coords

      // Determine how large we can make the pixmap on the paper without
      // losing any of it off the edges of the paper. iPrintWidth and iPrintHeight
      // will be the size of the pixmap on the paper, in printer device coords.
      int iPrintWidth = rcViewport.width();
      int iPrintHeight = rcViewport.height();
      double pAspect = (double) iPrintWidth / (double) iPrintHeight; // aspect ratio of the paper

      // unless the aspect ratios of the paper and pixmap are equal, we will have unused
      // space above and below or left and right of the printed image. 
      double ratioAspects = pAspect / dAspect;
      if (ratioAspects > 1.0)
      { 
         // paper is wider than the image: empty space left and right
         // reduce iPrintWidth accordingly
         iPrintWidth = iPrintWidth / ratioAspects;
      }
      else
      { 
         // paper is taller than the image: empty space above and below
         // reduce iPrintHeight accordingly
         iPrintHeight = iPrintHeight * ratioAspects;
      }

      // specify the pixel dimensions of the pixmap
      p.setWindow(pixmap.rect());

      // specify the location and size to draw the pixmap on the paper
      p.setViewport(rcViewport.left() + (rcViewport.width() - iPrintWidth) / 2,
         rcViewport.top() + (rcViewport.height() - iPrintHeight) / 2,
         iPrintWidth, iPrintHeight);

      // draw the pixmap to the print device
      p.drawPixmap(0, 0, pixmap);

      // tell the printer that we are done; this will trigger a form feed
      p.end();
   }

   if (deletePrinter)
   {
      delete pPrinter;
   }
}
