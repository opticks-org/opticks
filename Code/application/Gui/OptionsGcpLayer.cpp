/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "OptionsGcpLayer.h"

#include "ColorType.h"
#include "CustomColorButton.h"
#include "GcpLayer.h"
#include "GcpSymbolGrid.h"
#include "LabeledSection.h"

#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QSpinBox>
#include <QtGui/QVBoxLayout>

#include <limits>
#include <string>

using namespace std;

OptionsGcpLayer::OptionsGcpLayer() :
   QWidget(NULL)
{
   // Layer Properties
   QLabel* pSymbolLabel = new QLabel("Symbol:", this);
   mpMarkerSymbol = new GcpSymbolButton(this);

   QLabel* pSymbolSizeLabel = new QLabel("Symbol Size:", this);
   mpMarkerSize = new QSpinBox(this);
   mpMarkerSize->setRange(0, numeric_limits<int>::max());

   QLabel* pColorLabel = new QLabel("Color:", this);
   mpMarkerColor = new CustomColorButton(this);
   mpMarkerColor->usePopupGrid(true);

   QWidget* pLayerPropWidget = new QWidget(this);
   QGridLayout* pLayerPropLayout = new QGridLayout(pLayerPropWidget);
   pLayerPropLayout->setMargin(0);
   pLayerPropLayout->setSpacing(5);
   pLayerPropLayout->addWidget(pSymbolLabel, 0, 0);
   pLayerPropLayout->addWidget(mpMarkerSymbol, 0, 1, Qt::AlignLeft);
   pLayerPropLayout->addWidget(pSymbolSizeLabel, 1, 0);
   pLayerPropLayout->addWidget(mpMarkerSize, 1, 1, Qt::AlignLeft);
   pLayerPropLayout->addWidget(pColorLabel, 2, 0);
   pLayerPropLayout->addWidget(mpMarkerColor, 2, 1, Qt::AlignLeft);
   pLayerPropLayout->setColumnStretch(1, 10);
   LabeledSection* pMarkerSection = new LabeledSection(pLayerPropWidget, "Default Marker Properties", this);
   
   // Dialog layout
   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(10);
   pLayout->addWidget(pMarkerSection);
   pLayout->addStretch(10);
   
   // Initialize From Settings
   mpMarkerSize->setValue(GcpLayer::getSettingMarkerSize());
   mpMarkerSymbol->setCurrentValue(GcpLayer::getSettingMarkerSymbol());
   mpMarkerColor->setColor(GcpLayer::getSettingMarkerColor());
}
   
void OptionsGcpLayer::applyChanges()
{  
   GcpLayer::setSettingMarkerSize(mpMarkerSize->value());
   GcpLayer::setSettingMarkerSymbol(mpMarkerSymbol->getCurrentValue());
   GcpLayer::setSettingMarkerColor(mpMarkerColor->getColorType());
}

OptionsGcpLayer::~OptionsGcpLayer()
{
}
