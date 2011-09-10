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

#include "DimensionDescriptor.h"
#include "LabeledSectionGroup.h"
#include "Modifier.h"
#include "TypesFile.h"

#include <string>
#include <vector>

class ComplexComponentComboBox;
class QAction;
class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QMenu;
class QListWidget;
class QPushButton;
class QSpinBox;
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
   void initializeStretchMenu();
   void setGrayStretch(QAction* pAction);
   void setRgbStretch(QAction* pAction);
   void removeStretchFavorite();

private:
   bool mInitializing;
   RasterLayer* mpRasterLayer;
   std::vector<RasterElement*> mRasterElements;

   // Display configuration
   QComboBox* mpDisplayModeCombo;
   QLabel* mpComplexComponentLabel;
   ComplexComponentComboBox* mpComplexComponentCombo;
   QSpinBox* mpOpacitySpin;
   Modifier mDisplayConfigModifier;

   // Grayscale
   QComboBox* mpGrayElementCombo;
   QComboBox* mpGrayBandCombo;
   QDoubleSpinBox* mpGrayLowerSpin;
   QDoubleSpinBox* mpGrayUpperSpin;
   RegionUnits mGrayUnits;
   RegionUnitsComboBox* mpGrayUnitsCombo;
   StretchTypeComboBox* mpGrayStretchTypeCombo;
   Modifier mGrayscaleModifier;

   QMenu* mpGrayStretchMenu;
   QAction* mpAddFavoriteGrayAction;
   QAction* mpRemoveFavoriteAction;

   // RGB
   QComboBox* mpRedElementCombo;
   QComboBox* mpRedBandCombo;
   QDoubleSpinBox* mpRedLowerSpin;
   QDoubleSpinBox* mpRedUpperSpin;
   RegionUnits mRedUnits;
   RegionUnitsComboBox* mpRedUnitsCombo;
   Modifier mRgbModifier;

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

   QMenu* mpRgbStretchMenu;
   QAction* mpAddFavoriteRedAction;
   QAction* mpAddFavoriteGreenAction;
   QAction* mpAddFavoriteBlueAction;

   // Graphics acceleration
   QCheckBox* mpAccelerationCheck;
   QCheckBox* mpFilterCheck;
   QListWidget* mpFilterList;
   Modifier mGraphicsAccModifier;
};

#endif
