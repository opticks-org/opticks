/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "OptionsAoiLayer.h"

#include "AoiLayer.h"
#include "ColorType.h"
#include "CustomColorButton.h"
#include "LabeledSection.h"
#include "SymbolTypeGrid.h"

#include <QtGui/QCheckBox>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QVBoxLayout>

#include <string>

using namespace std;

OptionsAoiLayer::OptionsAoiLayer() :
   QWidget(NULL)
{
   // Layer Properties
   QLabel* pMarkerSymbolLabel = new QLabel("Symbol:", this);
   mpMarkerSymbol = new SymbolTypeButton(this);
   mpMarkerSymbol->setBorderedSymbols(true);

   QLabel* pMarkerColorLabel = new QLabel("Color:", this);
   mpMarkerColor = new CustomColorButton(this);
   mpMarkerColor->usePopupGrid(true);

   mpAutoColor = new QCheckBox("Auto Color", this);

   QWidget* pLayerPropWidget = new QWidget(this);
   QGridLayout* pLayerPropLayout = new QGridLayout(pLayerPropWidget);
   pLayerPropLayout->setMargin(0);
   pLayerPropLayout->setSpacing(5);
   pLayerPropLayout->addWidget(pMarkerSymbolLabel, 0, 0);
   pLayerPropLayout->addWidget(mpMarkerSymbol, 0, 1, Qt::AlignLeft);
   pLayerPropLayout->addWidget(pMarkerColorLabel, 1, 0);
   pLayerPropLayout->addWidget(mpMarkerColor, 1, 1, Qt::AlignLeft);
   pLayerPropLayout->addWidget(mpAutoColor, 2, 0, 1, 2);
   pLayerPropLayout->setColumnStretch(1, 10);
   LabeledSection* pMarkerSection = new LabeledSection(pLayerPropWidget, "Default Marker Properties", this);
   
   // Dialog layout
   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(10);
   pLayout->addWidget(pMarkerSection);
   pLayout->addStretch(10);
   
   // Initialize From Settings
   mpMarkerSymbol->setCurrentValue(AoiLayer::getSettingMarkerSymbol());
   mpMarkerColor->setColor(AoiLayer::getSettingMarkerColor());
   mpAutoColor->setChecked(AoiLayer::getSettingAutoColor());
}
   
void OptionsAoiLayer::applyChanges()
{  
   AoiLayer::setSettingMarkerSymbol(mpMarkerSymbol->getCurrentValue());
   AoiLayer::setSettingMarkerColor(mpMarkerColor->getColorType());
   AoiLayer::setSettingAutoColor(mpAutoColor->isChecked());
}

OptionsAoiLayer::~OptionsAoiLayer()
{
}
