/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef OPTIONSGEOGRAPHICFEATURES_H
#define OPTIONSGEOGRAPHICFEATURES_H

#include "AppVersion.h"
#include "ConfigurationSettings.h"
#include "LabeledSectionGroup.h"

class QCheckBox;

class OptionsGeographicFeatures : public LabeledSectionGroup
{
   Q_OBJECT

public:
   OptionsGeographicFeatures();
   virtual ~OptionsGeographicFeatures();

   SETTING(UseArcAsDefaultConnection, GeographicFeatures, bool, false);

   void applyChanges();

   static const std::string& getName()
   {
      static std::string var = "Geographic Features Options";
      return var;
   }

   static const std::string& getOptionName()
   {
      static std::string var = "Geographic Features";
      return var;
   }

   static const std::string& getDescription()
   {
      static std::string var = "Widget to display options for managing geographic features.";
      return var;
   }

   static const std::string& getShortDescription()
   {
      return getDescription();
   }

   static const std::string& getCreator()
   {
      static std::string var = "Ball Aerospace & Technologies Corp.";
      return var;
   }

   static const std::string& getCopyright()
   {
      static std::string var = APP_COPYRIGHT;
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
      static std::string var = "{EEE43D5A-DFD1-4712-B21C-DB24BEA1BAD9}";
      return var;
   }

private:
   QCheckBox* mpArcConnectionCheck;
};

#endif
