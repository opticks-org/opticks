/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PROPERTIESRASTERLAYER_H
#define PROPERTIESRASTERLAYER_H

#include <QtGui/QAction>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QLabel>
#include <QtGui/QListWidget>
#include <QtGui/QPushButton>
#include <QtGui/QSpinBox>

#include "DimensionDescriptor.h"
#include "LabeledSectionGroup.h"
#include "TypesFile.h"

#include <string>
#include <vector>

class ComplexComponentComboBox;
class RasterElement;
class RasterLayer;
class RegionUnitsComboBox;
class SessionItem;
class StretchTypeComboBox;

class PropertiesRasterLayer : public LabeledSectionGroup
{
   Q_OBJECT

public:
   PropertiesRasterLayer();
   ~PropertiesRasterLayer();

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
   static const std::string& getFilterWarningDialogId();
   static const std::string& getDisplayAsWarningDialogId();
   static bool isProduction();

protected:
   int getElementIndex(RasterElement* pRasterElement) const;
   void setStretchUnits(RasterChannelType channel, RegionUnits newUnits);
   RegionUnits getStretchUnits(RasterChannelType channel) const;
   DimensionDescriptor getSelectedBand(RasterChannelType channel, RasterElement*& pRasterElementOut) const;

protected slots:
   void updateDisplayedBandCombo(int index);
   void updateStretchValuesFromBand();
   void updateStretchValuesFromUnits(RegionUnits units);
   void enableFilterCheck(bool bEnable);
   void enableFilterCombo(bool bEnable);
   void setDisplayBands(QAction* pAction);

private:
   bool mInitializing;
   RasterLayer* mpRasterLayer;
   std::vector<RasterElement*> mRasterElements;

   // Display configuration
   QComboBox* mpDisplayModeCombo;
   QLabel* mpComplexComponentLabel;
   ComplexComponentComboBox* mpComplexComponentCombo;
   QSpinBox* mpOpacitySpin;

   // Grayscale
   QComboBox* mpGrayElementCombo;
   QComboBox* mpGrayBandCombo;
   QDoubleSpinBox* mpGrayLowerSpin;
   QDoubleSpinBox* mpGrayUpperSpin;
   RegionUnits mGrayUnits;
   RegionUnitsComboBox* mpGrayUnitsCombo;
   StretchTypeComboBox* mpGrayStretchTypeCombo;

   // RGB
   QComboBox* mpRedElementCombo;
   QComboBox* mpRedBandCombo;
   QDoubleSpinBox* mpRedLowerSpin;
   QDoubleSpinBox* mpRedUpperSpin;
   RegionUnits mRedUnits;
   RegionUnitsComboBox* mpRedUnitsCombo;

   QComboBox* mpGreenElementCombo;
   QComboBox* mpGreenBandCombo;
   QDoubleSpinBox* mpGreenLowerSpin;
   QDoubleSpinBox* mpGreenUpperSpin;
   RegionUnits mGreenUnits;
   RegionUnitsComboBox* mpGreenUnitsCombo;

   QComboBox* mpBlueElementCombo;
   QComboBox* mpBlueBandCombo;
   QDoubleSpinBox* mpBlueLowerSpin;
   QDoubleSpinBox* mpBlueUpperSpin;
   RegionUnits mBlueUnits;
   RegionUnitsComboBox* mpBlueUnitsCombo;

   QPushButton* mpDisplayBandButton;
   StretchTypeComboBox* mpRgbStretchTypeCombo;

   // Graphics acceleration
   QCheckBox* mpAccelerationCheck;
   QCheckBox* mpFilterCheck;
   QListWidget* mpFilterList;
};

#endif
