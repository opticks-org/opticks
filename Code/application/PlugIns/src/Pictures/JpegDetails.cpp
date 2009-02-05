/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "JpegDetails.h"
#include "OptionsJpegExporter.h"

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

QWidget* JpegDetails::getExportOptionsWidget(const PlugInArgList *)
{
   if (mpOptionsWidget.get() == NULL)
   {
      OptionsJpegExporter* pWidget = new OptionsJpegExporter();
      mpOptionsWidget.reset(pWidget);
      mpOptionsWidget->setPromptUserToSaveSettings(true);
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
   if (mpOptionsWidget.get() != NULL)
   {
      mpOptionsWidget->applyChanges();
      quality = mpOptionsWidget->getCompressionQuality();         
   }

   bool bSuccess = img.save(strFilename, "JPEG", quality);
   return bSuccess;
}

bool JpegDetails::isProduction() const
{
   return APP_IS_PRODUCTION_RELEASE;
}
