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
class ResolutionWidget;

class OptionsJpegExporter : public LabeledSectionGroup
{
   Q_OBJECT

public:
   OptionsJpegExporter();
   ~OptionsJpegExporter();

   SETTING(CompressionQuality, JpegExporter, unsigned int, 0);
   SETTING(UseViewResolution, JpegExporter, bool, true);
   SETTING(AspectRatioLock, JpegExporter, bool, false);
   SETTING(OutputWidth, JpegExporter, unsigned int, 0);
   SETTING(OutputHeight, JpegExporter, unsigned int, 0);

   void applyChanges();
   unsigned int getCompressionQuality();

   static const std::string& getName()
   {
      static std::string sName = "Jpeg Exporter Options";
      return sName;
   }

   static const std::string& getOptionName()
   {
      static std::string sOptionName = "Export/Jpeg";
      return sOptionName;
   }

   static const std::string& getDescription()
   {
      static std::string sDescription = "Widget to display jpeg exporter related options";
      return sDescription;
   }

   static const std::string& getShortDescription()
   {
      static std::string sShortDescription = "Widget to display jpeg exporter related options";
      return sShortDescription;
   }

   static const std::string& getCreator()
   {
      static std::string sCreator = "Ball Aerospace & Technologies Corp.";
      return sCreator;
   }

   static const std::string& getCopyright()
   {
      static std::string sCopyright = APP_COPYRIGHT_MSG;
      return sCopyright;
   }

   static const std::string& getVersion()
   {
      static std::string sVersion = APP_VERSION_NUMBER;
      return sVersion;
   }

   static bool isProduction()
   {
      return APP_IS_PRODUCTION_RELEASE;
   }

   static const std::string& getDescriptorId()
   {
      static std::string sId = "{1A0EF762-2C18-44b4-8D71-6396AB6A40C2}";
      return sId;
   }

private:
   OptionsJpegExporter(const OptionsJpegExporter& rhs);
   OptionsJpegExporter& operator=(const OptionsJpegExporter& rhs);

   QSlider* mpQualitySlider;
   ResolutionWidget* mpResolutionWidget;
};

#endif
