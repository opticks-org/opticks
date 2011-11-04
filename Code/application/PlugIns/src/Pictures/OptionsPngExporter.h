/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef OPTIONSPNGEXPORTER_H
#define OPTIONSPNGEXPORTER_H

#include "AppVersion.h"
#include "ConfigurationSettings.h"
#include "LabeledSectionGroup.h"
#include "ResolutionWidget.h"

class QCheckBox;

class OptionsPngExporter : public LabeledSectionGroup
{
   Q_OBJECT

public:
   OptionsPngExporter();
   ~OptionsPngExporter();

   SETTING(UseViewResolution, PngExporter, bool, true);
   SETTING(AspectRatioLock, PngExporter, bool, false);
   SETTING(OutputWidth, PngExporter, unsigned int, 0);
   SETTING(OutputHeight, PngExporter, unsigned int, 0);

   void applyChanges();

   static const std::string& getName()
   {
      static std::string sName = "Png Exporter Options";
      return sName;
   }

   static const std::string& getOptionName()
   {
      static std::string sOptionName = "Export/Png";
      return sOptionName;
   }

   static const std::string& getDescription()
   {
      static std::string sDescription = "Widget to display Png exporter related options";
      return sDescription;
   }

   static const std::string& getShortDescription()
   {
      static std::string sShortDescription = "Widget to display Png exporter related options";
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
      static std::string sId = "{ED1FB522-8104-443c-8FA3-E7FEB6D62F97}";
      return sId;
   }

private:
   OptionsPngExporter(const OptionsPngExporter& rhs);
   OptionsPngExporter& operator=(const OptionsPngExporter& rhs);

   ResolutionWidget* mpResolutionWidget;
};

#endif
