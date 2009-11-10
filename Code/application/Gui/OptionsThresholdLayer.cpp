/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "OptionsThresholdLayer.h"

#include "ColorType.h"
#include "CustomColorButton.h"
#include "LabeledSection.h"
#include "PassAreaComboBox.h"
#include "RegionUnitsComboBox.h"
#include "ThresholdLayer.h"
#include "SymbolTypeGrid.h"

#include <QtGui/QCheckBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QVBoxLayout>

#include <limits>
#include <string>

using namespace std;

OptionsThresholdLayer::OptionsThresholdLayer() :
   QWidget(NULL)
{
   // Marker Properties
   QLabel* pSymbolLabel = new QLabel("Symbol:", this);
   mpSymbolType = new SymbolTypeButton(this);
   mpSymbolType->setBorderedSymbols(true);

   QLabel* pColorLabel = new QLabel("Color:", this);
   mpColor = new CustomColorButton(this);
   mpColor->usePopupGrid(true);

   mpAutoColor = new QCheckBox("Auto Color", this);

   QWidget* pMarkerPropWidget = new QWidget(this);
   QGridLayout* pMarkerPropLayout = new QGridLayout(pMarkerPropWidget);
   pMarkerPropLayout->setMargin(0);
   pMarkerPropLayout->setSpacing(5);
   pMarkerPropLayout->addWidget(pSymbolLabel, 0, 0);
   pMarkerPropLayout->addWidget(mpSymbolType, 0, 1, Qt::AlignLeft);
   pMarkerPropLayout->addWidget(pColorLabel, 1, 0);
   pMarkerPropLayout->addWidget(mpColor, 1, 1, Qt::AlignLeft);
   pMarkerPropLayout->addWidget(mpAutoColor, 2, 0, 1, 2, Qt::AlignLeft);
   pMarkerPropLayout->setColumnStretch(1, 10);
   LabeledSection* pMarkerSection = new LabeledSection(pMarkerPropWidget, "Default Marker Properties", this);

   // Pass Area Properties
   QLabel* pPassAreaLabel = new QLabel("Pass Area:", this);
   mpPassArea = new PassAreaComboBox(this);

   QLabel* pRegionUnitsLabel = new QLabel("Pass Area Units:", this);
   mpRegionUnits = new RegionUnitsComboBox(this);

   QLabel* pFirstValueLabel = new QLabel("First Value:", this);
   mpFirstValue = new QDoubleSpinBox(this);
   mpFirstValue->setRange(-1 * numeric_limits<double>::max(), numeric_limits<double>::max());

   QLabel* pSecondValueLabel = new QLabel("Second Value:", this);
   mpSecondValue = new QDoubleSpinBox();
   mpSecondValue->setRange(-1 * numeric_limits<double>::max(), numeric_limits<double>::max());

   QWidget* pPassPropWidget = new QWidget(this);
   QGridLayout* pPassPropLayout = new QGridLayout(pPassPropWidget);
   pPassPropLayout->setMargin(0);
   pPassPropLayout->setSpacing(5);
   pPassPropLayout->addWidget(pPassAreaLabel, 0, 0);
   pPassPropLayout->addWidget(mpPassArea, 0, 1, Qt::AlignLeft);
   pPassPropLayout->addWidget(pRegionUnitsLabel, 1, 0);
   pPassPropLayout->addWidget(mpRegionUnits, 1, 1, Qt::AlignLeft);
   pPassPropLayout->addWidget(pFirstValueLabel, 2, 0);
   pPassPropLayout->addWidget(mpFirstValue, 2, 1, Qt::AlignLeft);
   pPassPropLayout->addWidget(pSecondValueLabel, 3, 0);
   pPassPropLayout->addWidget(mpSecondValue, 3, 1, Qt::AlignLeft);
   pPassPropLayout->setColumnStretch(1, 10);
   LabeledSection* pPassSection = new LabeledSection(pPassPropWidget, "Default Pass Area", this);
   
   // Dialog layout
   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(10);
   pLayout->addWidget(pMarkerSection);
   pLayout->addWidget(pPassSection);
   pLayout->addStretch(10);
   
   // Initialize From Settings
   mpRegionUnits->setCurrentValue(ThresholdLayer::getSettingRegionUnits());
   mpSymbolType->setCurrentValue(ThresholdLayer::getSettingMarkerSymbol());
   mpFirstValue->setValue(ThresholdLayer::getSettingFirstValue());
   mpSecondValue->setValue(ThresholdLayer::getSettingSecondValue());
   mpPassArea->setCurrentValue(ThresholdLayer::getSettingPassArea());
   mpColor->setColor(ThresholdLayer::getSettingMarkerColor());
   mpAutoColor->setChecked(ThresholdLayer::getSettingAutoColor());
}
   
void OptionsThresholdLayer::applyChanges()
{  
   ThresholdLayer::setSettingRegionUnits(mpRegionUnits->getCurrentValue());
   ThresholdLayer::setSettingMarkerSymbol(mpSymbolType->getCurrentValue());
   ThresholdLayer::setSettingFirstValue(mpFirstValue->value());
   ThresholdLayer::setSettingSecondValue(mpSecondValue->value());
   ThresholdLayer::setSettingPassArea(mpPassArea->getCurrentValue());
   ThresholdLayer::setSettingMarkerColor(mpColor->getColorType());
   ThresholdLayer::setSettingAutoColor(mpAutoColor->isChecked());
}

OptionsThresholdLayer::~OptionsThresholdLayer()
{
}
