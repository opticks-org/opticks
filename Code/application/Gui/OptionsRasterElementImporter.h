/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef OPTIONSRASTERELEMENTIMPORTER_H
#define OPTIONSRASTERELEMENTIMPORTER_H

#include "AppVersion.h"

#include <QtGui/QWidget>

#include <string>

class MutuallyExclusiveListWidget;
class QCheckBox;
class QGroupBox;
class QRadioButton;

class OptionsRasterElementImporter : public QWidget
{
   Q_OBJECT

public:
   OptionsRasterElementImporter();
   virtual ~OptionsRasterElementImporter();

   void applyChanges();

   static const std::string& getName()
   {
      static std::string var = "Raster Element Importer Options";
      return var;
   }

   static const std::string& getOptionName()
   {
      static std::string var = "Import/Raster Element";
      return var;
   }

   static const std::string& getDescription()
   {
      static std::string var = "Widget to display raster element importer related options for the application";
      return var;
   }

   static const std::string& getShortDescription()
   {
      static std::string var = "Widget to display raster element importer related options for the application";
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
      static std::string var = "{329665D9-9075-42EC-A76B-62C70783D3C1}";
      return var;
   }

private:
   OptionsRasterElementImporter(const OptionsRasterElementImporter& rhs);
   OptionsRasterElementImporter& operator=(const OptionsRasterElementImporter& rhs);
   QGroupBox* mpAutoGeorefGroup;
   QRadioButton* mpImporterPlugInRadio;
   MutuallyExclusiveListWidget* mpPlugInList;
   QCheckBox* mpLatLonLayerCheck;
};

#endif
