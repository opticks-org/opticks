/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AoiLayer.h"
#include "AoiToolBar.h"
#include "ColorType.h"
#include "CustomColorButton.h"
#include "GraphicObjectTypeGrid.h"
#include "LabeledSection.h"
#include "OptionsAoiLayer.h"
#include "SymbolTypeGrid.h"

#include <QtGui/QCheckBox>
#include <QtGui/QLabel>
#include <QtGui/QLayout>

OptionsAoiLayer::OptionsAoiLayer() :
   QWidget(NULL)
{
   // Pixel marker
   QLabel* pMarkerSymbolLabel = new QLabel("Symbol:", this);
   mpMarkerSymbol = new SymbolTypeButton(this);
   mpMarkerSymbol->setBorderedSymbols(true);

   QLabel* pMarkerColorLabel = new QLabel("Color:", this);
   mpMarkerColor = new CustomColorButton(this);
   mpMarkerColor->usePopupGrid(true);

   mpAutoColor = new QCheckBox("Auto Color", this);

   QWidget* pPixelMarkerWidget = new QWidget(this);
   QGridLayout* pPixelMarkerLayout = new QGridLayout(pPixelMarkerWidget);
   pPixelMarkerLayout->setMargin(0);
   pPixelMarkerLayout->setSpacing(5);
   pPixelMarkerLayout->addWidget(pMarkerSymbolLabel, 0, 0);
   pPixelMarkerLayout->addWidget(mpMarkerSymbol, 0, 1, Qt::AlignLeft);
   pPixelMarkerLayout->addWidget(pMarkerColorLabel, 1, 0);
   pPixelMarkerLayout->addWidget(mpMarkerColor, 1, 1, Qt::AlignLeft);
   pPixelMarkerLayout->addWidget(mpAutoColor, 2, 0, 1, 2);
   pPixelMarkerLayout->setColumnStretch(1, 10);
   LabeledSection* pPixelMarkerSection = new LabeledSection(pPixelMarkerWidget, "Default Pixel Marker", this);

   // Pixel selection
   QLabel* pSelectionToolLabel = new QLabel("Selection Tool:", this);
   mpSelectionTool = new GraphicObjectTypeButton(GraphicObjectTypeGrid::VIEW_AOI, this);

   QWidget* pPixelSelectionWidget = new QWidget(this);
   QGridLayout* pPixelSelectionLayout = new QGridLayout(pPixelSelectionWidget);
   pPixelSelectionLayout->setMargin(0);
   pPixelSelectionLayout->setSpacing(5);
   pPixelSelectionLayout->addWidget(pSelectionToolLabel, 0, 0);
   pPixelSelectionLayout->addWidget(mpSelectionTool, 0, 1, Qt::AlignLeft);
   pPixelSelectionLayout->setColumnStretch(1, 10);
   LabeledSection* pPixelSelectionSection = new LabeledSection(pPixelSelectionWidget, "Pixel Selection", this);

   // Dialog layout
   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(10);
   pLayout->addWidget(pPixelMarkerSection);
   pLayout->addWidget(pPixelSelectionSection);
   pLayout->addStretch(10);

   // Initialize From Settings
   mpMarkerSymbol->setCurrentValue(AoiLayer::getSettingMarkerSymbol());
   mpMarkerColor->setColor(AoiLayer::getSettingMarkerColor());
   mpAutoColor->setChecked(AoiLayer::getSettingAutoColor());
   mpSelectionTool->setCurrentValue(AoiToolBar::getSettingSelectionTool());
}

OptionsAoiLayer::~OptionsAoiLayer()
{}

void OptionsAoiLayer::applyChanges()
{
   AoiLayer::setSettingMarkerSymbol(mpMarkerSymbol->getCurrentValue());
   AoiLayer::setSettingMarkerColor(mpMarkerColor->getColorType());
   AoiLayer::setSettingAutoColor(mpAutoColor->isChecked());
   AoiToolBar::setSettingSelectionTool(mpSelectionTool->getCurrentValue());
}
