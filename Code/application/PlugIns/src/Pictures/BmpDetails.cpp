/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "BmpDetails.h"
#include "Exporter.h"
#include "ImageResolutionWidget.h"
#include "OptionsBmpExporter.h"
#include "PicturesPlotWidgetExporter.h"
#include "PicturesViewExporter.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInRegistration.h"
#include "View.h"

REGISTER_PLUGIN(OpticksPictures, BmpPicturesPlotWidgetExporter, PicturesPlotWidgetExporter(new BmpDetails));
REGISTER_PLUGIN(OpticksPictures, BmpPicturesViewExporter, PicturesViewExporter(new BmpDetails));

std::string BmpDetails::name()
{
   return "Bitmap";
}

std::string BmpDetails::shortDescription()
{
   return "BMP";
}

std::string BmpDetails::description()
{
   return "Bit MaP";
}

std::string BmpDetails::extensions()
{
   return "Bitmap Files (*.bmp)";
}

void BmpDetails::computeExportResolution(unsigned int& imageWidth, unsigned int& imageHeight)
{
   calculateExportResolution(imageWidth, imageHeight, 
      OptionsBmpExporter::getSettingAspectRatioLock(),
      OptionsBmpExporter::getSettingUseViewResolution(),
      OptionsBmpExporter::getSettingOutputWidth(),
      OptionsBmpExporter::getSettingOutputHeight());
}

QWidget* BmpDetails::getExportOptionsWidget(const PlugInArgList* pInArgList)
{
   if (mpOptionsWidget.get() == NULL)
   {
      mpOptionsWidget.reset(new ImageResolutionWidget());
      unsigned int computedWidth = 1;
      unsigned int computedHeight = 1;

      View* pView = dynamic_cast<View*>(pInArgList->getPlugInArgValue<View>(Exporter::ExportItemArg()));
      if (pView != NULL)
      {
         QWidget* pViewWidget = pView->getWidget();
         computedWidth = pViewWidget->width();
         computedHeight = pViewWidget->height();
         computeExportResolution(computedWidth, computedHeight);
      }
      mpOptionsWidget->setResolution(computedWidth, computedHeight);
   }

   return mpOptionsWidget.get();
}

bool BmpDetails::savePict(QString strFilename, QImage img, const SessionItem *pItem)
{
   if ((strFilename.isEmpty() == true) || (img.isNull() == true))
   {
      return false;
   }

   unsigned int outputWidth = img.width();
   unsigned int outputHeight = img.height();
   if (mpOptionsWidget.get() == NULL)
   {
      computeExportResolution(outputWidth, outputHeight);
   }
   else
   {
      mpOptionsWidget->getResolution(outputWidth, outputHeight);
   }
   img = img.scaled(outputWidth, outputHeight);
   return img.save(strFilename, "BMP");
}

bool BmpDetails::isProduction() const
{
   return APP_IS_PRODUCTION_RELEASE;
}
