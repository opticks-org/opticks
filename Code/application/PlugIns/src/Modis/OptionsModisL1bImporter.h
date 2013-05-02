/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef OPTIONSMODISL1BIMPORTER_H
#define OPTIONSMODISL1BIMPORTER_H

#include "AppVersion.h"
#include "LabeledSectionGroup.h"

class RasterConversionTypeComboBox;

class OptionsModisL1bImporter : public LabeledSectionGroup
{
   Q_OBJECT

public:
   OptionsModisL1bImporter();
   virtual ~OptionsModisL1bImporter();

   void applyChanges();

   static const std::string& getName()
   {
      static std::string sName = "MODIS L1B Importer Options";
      return sName;
   }

   static const std::string& getOptionName()
   {
      static std::string sOptionName = "Import/MODIS";
      return sOptionName;
   }

   static const std::string& getDescription()
   {
      static std::string sDescription = "A widget to display MODIS L1B Importer related options.";
      return sDescription;
   }

   static const std::string& getShortDescription()
   {
      return getDescription();
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
      static std::string sId = "{EBAC5476-52B3-4C9E-B196-8F0E0874C3FA}";
      return sId;
   }

private:
   RasterConversionTypeComboBox* mpRasterConversionCombo;
};

#endif
