/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef OPTIONSGEOREFERENCE_H
#define OPTIONSGEOREFERENCE_H

#include "AppVersion.h"
#include "TypesFile.h"

#include <QtGui/QWidget>

#include <string>

class DmsFormatTypeComboBox;
class GeocoordTypeComboBox;
class QCheckBox;
class QLabel;

class OptionsGeoreference : public QWidget
{
   Q_OBJECT

public:
   OptionsGeoreference();
   virtual ~OptionsGeoreference();

   void applyChanges();

   static const std::string& getName()
   {
      static std::string var = "Georeference Options";
      return var;
   }

   static const std::string& getOptionName()
   {
      static std::string var = "Georeference";
      return var;
   }

   static const std::string& getDescription()
   {
      return getShortDescription();
   }

   static const std::string& getShortDescription()
   {
      static std::string var = "Widget to display georeference related options for the application";
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

protected slots:
   void createLayerChanged(bool create);
   void geocoordTypeChanged(GeocoordType geocoordType);

private:
   OptionsGeoreference(const OptionsGeoreference& rhs);
   OptionsGeoreference& operator=(const OptionsGeoreference& rhs);

   // Georeference
   QCheckBox* mpAutoGeoreference;
   QCheckBox* mpCreateLayer;
   QCheckBox* mpDisplayLayer;
   GeocoordTypeComboBox* mpGeocoordTypeCombo;
   QLabel* mpLatLonFormatLabel;
   DmsFormatTypeComboBox* mpLatLonFormatCombo;
};

#endif
