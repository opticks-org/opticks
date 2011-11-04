/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef OPTIONSTHRESHOLDLAYER_H
#define OPTIONSTHRESHOLDLAYER_H

#include <QtGui/QWidget>
#include "AppVersion.h"

class CustomColorButton;
class QCheckBox;
class QDoubleSpinBox;
class PassAreaComboBox;
class RegionUnitsComboBox;
class SymbolTypeButton;

class OptionsThresholdLayer : public QWidget
{
   Q_OBJECT

public:
   OptionsThresholdLayer();
   ~OptionsThresholdLayer();

   void applyChanges();

   static const std::string& getName()
   {
      static std::string var = "Threshold Layer Options";
      return var;
   }

   static const std::string& getOptionName()
   {
      static std::string var = "Layers/Threshold";
      return var;
   }

   static const std::string& getDescription()
   {
      static std::string var = "Widget to display Threshold layer related options for the application";
      return var;
   }

   static const std::string& getShortDescription()
   {
      static std::string var = "Widget to display Threshold layer related related options for the application";
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
      static std::string var = "{D11B8488-D155-499c-9765-DC691B2A960D}";
      return var;
   }

private:
   OptionsThresholdLayer(const OptionsThresholdLayer& rhs);
   OptionsThresholdLayer& operator=(const OptionsThresholdLayer& rhs);
   RegionUnitsComboBox* mpRegionUnits;
   SymbolTypeButton* mpSymbolType;
   QDoubleSpinBox* mpFirstValue;
   QDoubleSpinBox* mpSecondValue;
   PassAreaComboBox* mpPassArea;
   CustomColorButton* mpColor;
   QCheckBox* mpAutoColor;
};

#endif
