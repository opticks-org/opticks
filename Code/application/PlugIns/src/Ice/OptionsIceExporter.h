/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef OPTIONSICEEXPORTER_H
#define OPTIONSICEEXPORTER_H

#include "AppVersion.h"
#include "ConfigurationSettings.h"
#include "IceWriter.h"
#include "LabeledSectionGroup.h"

class QComboBox;
class QLabel;
class QSlider;
class QSpinBox;

class OptionsIceExporter : public LabeledSectionGroup
{
   Q_OBJECT

public:
   OptionsIceExporter();
   ~OptionsIceExporter();

   void applyChanges();
   void setSaveSettings(bool saveSettings);
   IceCompressionType getCompressionType();
   int getGzipCompressionLevel();
   int getChunkSize();

   static const std::string& getName()
   {
      static std::string var = "Ice Exporter Options";
      return var;
   }

   static const std::string& getOptionName()
   {
      static std::string var = "Export/Ice";
      return var;
   }

   static const std::string& getDescription()
   {
      static std::string var = "Widget to display ice exporter related options";
      return var;
   }

   static const std::string& getShortDescription()
   {
      static std::string var = "Widget to display ice exporter related options";
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
      static std::string var = "{520bb1b4-aeda-4a0f-8e11-9a76ef5eb693}";
      return var;
   }

private slots:
   void compressionTypeChanged(const QString& value);
   void gzipCompressionValueChanged(int value);

private:
   OptionsIceExporter(const OptionsIceExporter& rhs);
   OptionsIceExporter operator=(const OptionsIceExporter& rhs);
   QComboBox* mpCompressionTypeCombo;
   QSlider* mpGzipCompressionSlider;
   QLabel* mpGzipLevelValue;
   QSpinBox* mpChunkSize;
   bool mSaveSettings;
};

#endif
