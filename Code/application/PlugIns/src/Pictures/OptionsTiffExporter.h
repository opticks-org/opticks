/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef OPTIONSTIFFEXPORTER_H
#define OPTIONSTIFFEXPORTER_H

#include "AppVersion.h"
#include "ConfigurationSettings.h"
#include "LabeledSectionGroup.h"

class QCheckBox;
class QSpinBox;

class OptionsTiffExporter : public LabeledSectionGroup
{
   Q_OBJECT

public:
   OptionsTiffExporter();
   ~OptionsTiffExporter();

   SETTING(PackBitsCompression, TiffExporter, bool, false);
   SETTING(RowsPerStrip, TiffExporter, unsigned int, 1);

   void setPromptUserToSaveSettings(bool prompt);
   bool getPackBitsCompression();
   void applyChanges();

   static const std::string& getName()
   {
      static std::string var = "GeoTIFF Exporter Options";
      return var;
   }

   static const std::string& getOptionName()
   {
      static std::string var = "Export/GeoTIFF";
      return var;
   }

   static const std::string& getDescription()
   {
      static std::string var = "Widget to display GeoTIFF exporter related options";
      return var;
   }

   static const std::string& getShortDescription()
   {
      static std::string var = "Widget to display GeoTIFF exporter related options";
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
      static std::string var = "{26845CD5-CFEE-4d9b-A227-BD8B296B48D3}";
      return var;
   }

private:
   QCheckBox* mpPackBits;
   QSpinBox* mpRowsPerStrip;
   QCheckBox* mpSaveSettings;
};

#endif
