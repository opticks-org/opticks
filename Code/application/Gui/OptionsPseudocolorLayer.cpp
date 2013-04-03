/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "OptionsPseudocolorLayer.h"

#include "LabeledSection.h"
#include "PseudocolorLayer.h"
#include "SymbolTypeGrid.h"

#include <QtGui/QApplication>
#include <QtGui/QFont>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QVBoxLayout>

#include <string>

using namespace std;

OptionsPseudocolorLayer::OptionsPseudocolorLayer() :
   QWidget(NULL)
{  
   // Marker Properties
   QLabel* pSymbolLabel = new QLabel("Symbol:", this);
   mpSymbolType = new SymbolTypeButton(this);
   mpSymbolType->setBorderedSymbols(true);

   QWidget* pMarkerPropWidget = new QWidget(this);
   QGridLayout* pMarkerPropLayout = new QGridLayout(pMarkerPropWidget);
   pMarkerPropLayout->setMargin(0);
   pMarkerPropLayout->setSpacing(5);
   pMarkerPropLayout->addWidget(pSymbolLabel, 0, 0);
   pMarkerPropLayout->addWidget(mpSymbolType, 0, 1, Qt::AlignLeft);
   pMarkerPropLayout->setColumnStretch(1, 10);
   LabeledSection* pMarkerSection = new LabeledSection(pMarkerPropWidget, "Default Marker Properties", this);
   
   // Dialog layout
   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(10);
   pLayout->addWidget(pMarkerSection);
   pLayout->addStretch(10);
   
   // Initialize From Settings
   mpSymbolType->setCurrentValue(PseudocolorLayer::getSettingMarkerSymbol());
}
   
void OptionsPseudocolorLayer::applyChanges()
{
   PseudocolorLayer::setSettingMarkerSymbol(mpSymbolType->getCurrentValue());
}

OptionsPseudocolorLayer::~OptionsPseudocolorLayer()
{
}
