/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PROPERTIESTHRESHOLDLAYER_H
#define PROPERTIESTHRESHOLDLAYER_H

#include "LabeledSectionGroup.h"
#include "Modifier.h"
#include "TypesFile.h"

#include <string>

class CustomColorButton;
class PassAreaComboBox;
class QComboBox;
class QDoubleSpinBox;
class QLabel;
class RegionUnitsComboBox;
class SessionItem;
class StretchTypeComboBox;
class SymbolTypeButton;
class ThresholdLayer;

class PropertiesThresholdLayer : public LabeledSectionGroup
{
   Q_OBJECT

public:
   PropertiesThresholdLayer();
   virtual ~PropertiesThresholdLayer();

   bool initialize(SessionItem* pSessionItem);
   bool applyChanges();

   static const std::string& getName();
   static const std::string& getPropertiesName();
   static const std::string& getDescription();
   static const std::string& getShortDescription();
   static const std::string& getCreator();
   static const std::string& getCopyright();
   static const std::string& getVersion();
   static const std::string& getDescriptorId();
   static bool isProduction();

protected slots:
   void setPassArea(PassArea passArea);
   void setRegionUnits(RegionUnits newUnits);

private:
   PropertiesThresholdLayer(const PropertiesThresholdLayer& rhs);
   PropertiesThresholdLayer& operator=(const PropertiesThresholdLayer& rhs);
   ThresholdLayer* mpThresholdLayer;
   RegionUnits mUnits;

   // Display
   QComboBox* mpDisplayBand;
   Modifier mDisplayModifier;

   // Pixel marker
   SymbolTypeButton* mpSymbolButton;
   CustomColorButton* mpColorButton;
   Modifier mMarkerModifier;

   // Pass area
   PassAreaComboBox* mpPassAreaCombo;
   RegionUnitsComboBox* mpUnitsCombo;
   QDoubleSpinBox* mpFirstValueSpin;
   QLabel* mpSecondValueLabel;
   QDoubleSpinBox* mpSecondValueSpin;
   Modifier mPassModifier;
};

#endif
