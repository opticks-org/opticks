/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef OPTIONSJPEGEXPORTER_H
#define OPTIONSJPEGEXPORTER_H

#include "AppVersion.h"
#include "ConfigurationSettings.h"
#include "LabeledSectionGroup.h"

class QCheckBox;
class QSlider;

class OptionsJpegExporter : public LabeledSectionGroup
{
   Q_OBJECT

public:
   OptionsJpegExporter();
   ~OptionsJpegExporter();

   SETTING(CompressionQuality, JpegExporter, unsigned int, 0);

   void applyChanges();
   void setPromptUserToSaveSettings(bool prompt);
   unsigned int getCompressionQuality();

   static const std::string& getName()
   {
      static std::string var = "Jpeg Exporter Options";
      return var;
   }

   static const std::string& getOptionName()
   {
      static std::string var = "Export/Jpeg";
      return var;
   }

   static const std::string& getDescription()
   {
      static std::string var = "Widget to display jpeg exporter related options";
      return var;
   }

   static const std::string& getShortDescription()
   {
      static std::string var = "Widget to display jpeg exporter related options";
      return var;
   }

   static const std::string& getCreator()
   {
      static std::string var = "Ball Aerospace & Technologies Corp.";
      return var;
   }

   static const std::string& getCopyright()
   {
      static std::string var = APP_COPYRIGHT_MSG;
      return var;
   }

   static const std::string& getVersion()
   {
      static std::string var = APP_VERSION_NUMBER;
      return var;
   }

   static bool isProduction()
   {
      return APP_IS_PRODUCTION_RELEASE;
   }

   static const std::string& getDescriptorId()
   {
      static std::string var = "{1A0EF762-2C18-44b4-8D71-6396AB6A40C2}";
      return var;
   }

private:
   QCheckBox* mpSaveSettings;
   QSlider *mpQualitySlider;
};

#endif
