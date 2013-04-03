/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef OPTIONSBMPEXPORTER_H
#define OPTIONSBMPEXPORTER_H

#include "AppVersion.h"
#include "ConfigurationSettings.h"
#include "LabeledSectionGroup.h"
#include "ResolutionWidget.h"

class QCheckBox;

class OptionsBmpExporter : public LabeledSectionGroup
{
   Q_OBJECT

public:
   OptionsBmpExporter();
   ~OptionsBmpExporter();

   SETTING(UseViewResolution, BmpExporter, bool, true);
   SETTING(AspectRatioLock, BmpExporter, bool, false);
   SETTING(OutputWidth, BmpExporter, unsigned int, 0);
   SETTING(OutputHeight, BmpExporter, unsigned int, 0);

   void applyChanges();

   static const std::string& getName()
   {
      static std::string sName = "Bmp Exporter Options";
      return sName;
   }

   static const std::string& getOptionName()
   {
      static std::string sOptionName = "Export/Bmp";
      return sOptionName;
   }

   static const std::string& getDescription()
   {
      static std::string sDescription = "Widget to display Bmp exporter related options";
      return sDescription;
   }

   static const std::string& getShortDescription()
   {
      static std::string sShortDescription = "Widget to display Bmp exporter related options";
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
      static std::string sId = "{3DB38103-4694-4900-9FB1-4AFAF1DE709C}";
      return sId;
   }

private:
   OptionsBmpExporter(const OptionsBmpExporter& rhs);
   OptionsBmpExporter& operator=(const OptionsBmpExporter& rhs);

   QCheckBox* mpSaveSettings;
   ResolutionWidget* mpResolutionWidget;
};

#endif
