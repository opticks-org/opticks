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
class ResolutionWidget;

class OptionsTiffExporter : public LabeledSectionGroup
{
   Q_OBJECT

public:
   OptionsTiffExporter();
   ~OptionsTiffExporter();

   SETTING(PackBitsCompression, TiffExporter, bool, false);
   SETTING(RowsPerStrip, TiffExporter, unsigned int, 1);
   SETTING(UseViewResolution, TiffExporter, bool, true);
   SETTING(AspectRatioLock, TiffExporter, bool, false);
   SETTING(OutputWidth, TiffExporter, unsigned int, 0);
   SETTING(OutputHeight, TiffExporter, unsigned int, 0);

   bool getPackBitsCompression();
   void applyChanges();

   static const std::string& getName()
   {
      static std::string sName = "GeoTIFF Exporter Options";
      return sName;
   }

   static const std::string& getOptionName()
   {
      static std::string sOptionName = "Export/GeoTIFF";
      return sOptionName;
   }

   static const std::string& getDescription()
   {
      static std::string sDescription = "Widget to display GeoTIFF exporter related options";
      return sDescription;
   }

   static const std::string& getShortDescription()
   {
      static std::string sShortDescription = "Widget to display GeoTIFF exporter related options";
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
      static std::string sId = "{26845CD5-CFEE-4d9b-A227-BD8B296B48D3}";
      return sId;
   }

private:
   OptionsTiffExporter(const OptionsTiffExporter& rhs);
   OptionsTiffExporter& operator=(const OptionsTiffExporter& rhs);

   QCheckBox* mpPackBits;
   QSpinBox* mpRowsPerStrip;
   ResolutionWidget* mpResolutionWidget;
};

#endif
