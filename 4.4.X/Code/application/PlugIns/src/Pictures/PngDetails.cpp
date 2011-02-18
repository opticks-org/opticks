/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "ImageResolutionWidget.h"
#include "OptionsPngExporter.h"
#include "PicturesPlotWidgetExporter.h"
#include "PicturesViewExporter.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInRegistration.h"
#include "PngDetails.h"
#include "View.h"

REGISTER_PLUGIN(OpticksPictures, PngPicturesPlotWidgetExporter, PicturesPlotWidgetExporter(new PngDetails));
REGISTER_PLUGIN(OpticksPictures, PngPicturesViewExporter, PicturesViewExporter(new PngDetails));

std::string PngDetails::name()
{
   return "PNG";
}

std::string PngDetails::shortDescription()
{
   return "PNG";
}

std::string PngDetails::description()
{
   return "Portable Network Graphics (PNG)";
}

std::string PngDetails::extensions()
{
   return "Portable Network Graphics Files (*.png)";
}

bool PngDetails::savePict(QString strFilename, QImage img, const SessionItem *pItem)
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
   // for some reason we have problems with Alpha on Solaris
   // we don't care about the alpha channel for product
   // export so we'll just convert the image.
   img = img.convertToFormat(QImage::Format_RGB32);
   return img.save(strFilename, "PNG");
}

QWidget* PngDetails::getExportOptionsWidget(const PlugInArgList* pInArgList)
{
   if (mpOptionsWidget.get() == NULL)
   {
      mpOptionsWidget.reset(new ImageResolutionWidget());
      unsigned int outputWidth = 0;
      unsigned int outputHeight = 0;

      View* pView = dynamic_cast<View*>(pInArgList->getPlugInArgValue<View>(Exporter::ExportItemArg()));
      if (pView != NULL)
      {
         QWidget* pViewWidget = pView->getWidget();
         outputWidth = pViewWidget->width();
         outputHeight = pViewWidget->height();
         computeExportResolution(outputWidth, outputHeight);
      }
      mpOptionsWidget->setResolution(outputWidth, outputHeight);
   }

   return mpOptionsWidget.get();
}

void PngDetails::computeExportResolution(unsigned int& imageWidth, unsigned int& imageHeight)
{
   calculateExportResolution(imageWidth, imageHeight, 
      OptionsPngExporter::getSettingAspectRatioLock(),
      OptionsPngExporter::getSettingUseViewResolution(),
      OptionsPngExporter::getSettingOutputWidth(),
      OptionsPngExporter::getSettingOutputHeight());
}

bool PngDetails::isProduction() const
{
   return APP_IS_PRODUCTION_RELEASE;
}
