/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "OptionsTiePointLayer.h"

#include "ColorType.h"
#include "CustomColorButton.h"
#include "LabeledSection.h"
#include "TiePointLayer.h"

#include <QtGui/QApplication>
#include <QtGui/QCheckBox>
#include <QtGui/QFont>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QSpinBox>
#include <QtGui/QVBoxLayout>

#include <limits>
#include <string>

using namespace std;

OptionsTiePointLayer::OptionsTiePointLayer() :
   QWidget(NULL)
{
   // Marker Properties
   QLabel* pSymbolSizeLabel = new QLabel("Symbol Size:", this);
   mpSymbolSize = new QSpinBox(this);
   mpSymbolSize->setRange(0, numeric_limits<int>::max());

   QLabel* pColorLabel = new QLabel("Color:", this);
   mpColor = new CustomColorButton(this);
   mpColor->usePopupGrid(true);

   mpAutoColor = new QCheckBox("Auto Color", this);

   QWidget* pMarkerPropWidget = new QWidget(this);
   QGridLayout* pMarkerPropLayout = new QGridLayout(pMarkerPropWidget);
   pMarkerPropLayout->setMargin(0);
   pMarkerPropLayout->setSpacing(5);
   pMarkerPropLayout->addWidget(pSymbolSizeLabel, 0, 0);
   pMarkerPropLayout->addWidget(mpSymbolSize, 0, 1, Qt::AlignLeft);
   pMarkerPropLayout->addWidget(pColorLabel, 1, 0);
   pMarkerPropLayout->addWidget(mpColor, 1, 1, Qt::AlignLeft);
   pMarkerPropLayout->addWidget(mpAutoColor, 2, 0, 1, 2, Qt::AlignLeft);
   pMarkerPropLayout->setColumnStretch(1, 10);
   LabeledSection* pMarkerSection = new LabeledSection(pMarkerPropWidget, "Default Marker Properties", this);

   //Label Properties
   mpLabelsEnabled = new QCheckBox("Show Labels", this);
   LabeledSection* pLabelSection = new LabeledSection(mpLabelsEnabled, "Default Label Properties", this);
   
   // Dialog layout
   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(10);
   pLayout->addWidget(pMarkerSection);
   pLayout->addWidget(pLabelSection);
   pLayout->addStretch(10);
   
   // Initialize From Settings
   mpSymbolSize->setValue(TiePointLayer::getSettingMarkerSize());
   mpLabelsEnabled->setChecked(TiePointLayer::getSettingLabelEnabled());
   mpColor->setColor(TiePointLayer::getSettingMarkerColor());
   mpAutoColor->setChecked(TiePointLayer::getSettingAutoColor());   
}
   
void OptionsTiePointLayer::applyChanges()
{  
   TiePointLayer::setSettingMarkerSize(mpSymbolSize->value());
   TiePointLayer::setSettingLabelEnabled(mpLabelsEnabled->isChecked());
   TiePointLayer::setSettingMarkerColor(mpColor->getColorType());
   TiePointLayer::setSettingAutoColor(mpAutoColor->isChecked());
}

OptionsTiePointLayer::~OptionsTiePointLayer()
{
}
