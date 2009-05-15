/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "JpegDetails.h"
#include "JpegExportOptionsWidget.h"
#include "OptionsJpegExporter.h"
#include "PicturesPlotWidgetExporter.h"
#include "PicturesViewExporter.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInRegistration.h"
#include "View.h"

REGISTER_PLUGIN(OpticksPictures, JpegPicturesPlotWidgetExporter, PicturesPlotWidgetExporter(new JpegDetails));
REGISTER_PLUGIN(OpticksPictures, JpegPicturesViewExporter, PicturesViewExporter(new JpegDetails));

JpegDetails::JpegDetails() : mpOptionsWidget(NULL)
{
}

std::string JpegDetails::name()
{
   return "JPEG";
}

std::string JpegDetails::shortDescription()
{
   return "JPEG";
}

std::string JpegDetails::description()
{
   return "JPEG";
}

std::string JpegDetails::extensions()
{
   return "JPEG Files (*.jpg)";
}

QWidget* JpegDetails::getExportOptionsWidget(const PlugInArgList* pInArgList)
{
   if (mpOptionsWidget.get() == NULL)
   {
      mpOptionsWidget.reset(new JpegExportOptionsWidget());
      View* pView = dynamic_cast<View*>(pInArgList->getPlugInArgValue<View>(Exporter::ExportItemArg()));
      unsigned int computedWidth = 1;
      unsigned int computedHeight = 1;
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

bool JpegDetails::savePict(QString strFilename, QImage img, const SessionItem *pItem)
{
   if ((strFilename.isEmpty() == true) || (img.isNull() == true))
   {
      return false;
   }

   unsigned int quality = OptionsJpegExporter::getSettingCompressionQuality();
   unsigned int outputWidth = img.width(); 
   unsigned int outputHeight = img.height();
   if (mpOptionsWidget.get() == NULL)
   {
      computeExportResolution(outputWidth, outputHeight);
   }
   else
   {
      mpOptionsWidget->getResolution(outputWidth, outputHeight);
      quality = mpOptionsWidget->getCompressionQuality();
   }
   img = img.scaled(outputWidth, outputHeight);
   return img.save(strFilename, "JPEG", quality);   
}

bool JpegDetails::isProduction() const
{
   return APP_IS_PRODUCTION_RELEASE;
}

void JpegDetails::computeExportResolution(unsigned int& imageWidth, unsigned int& imageHeight)
{
   calculateExportResolution(imageWidth, imageHeight, 
      OptionsJpegExporter::getSettingAspectRatioLock(),
      OptionsJpegExporter::getSettingUseViewResolution(),
      OptionsJpegExporter::getSettingOutputWidth(),
      OptionsJpegExporter::getSettingOutputHeight());
}
