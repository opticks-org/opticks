/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ColorType.h"
#include "CustomColorButton.h"
#include "LabeledSection.h"
#include "LatLonLayer.h"
#include "LatLonStyleComboBox.h"
#include "LineWidthComboBox.h"
#include "OptionsLatLonLayer.h"

#include <QtGui/QApplication>
#include <QtGui/QFont>
#include <QtGui/QFontComboBox>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QVBoxLayout>

#include <string>

OptionsLatLonLayer::OptionsLatLonLayer() :
   QWidget(NULL)
{
   // Label Properties
   QLabel* pFontLabel = new QLabel("Font:", this);
   mpFont = new QFontComboBox(this);

   QLabel* pFontSizeLabel = new QLabel("Font Size:", this);
   mpFontSize = new QComboBox(this);
   mpFontSize->setEditable(true);
   mpFontSize->setAutoCompletion(false);
   mpFontSize->setInsertPolicy(QComboBox::NoInsert);
   QList<int> lstFontSizes = QFontDatabase::standardSizes();
   for (int i = 0; i < lstFontSizes.count(); ++i)
   {
      QString strSize = QString::number(lstFontSizes[i]);
      mpFontSize->addItem(strSize);
   }

   QWidget* pLabelPropWidget = new QWidget(this);
   QGridLayout* pLabelPropLayout = new QGridLayout(pLabelPropWidget);
   pLabelPropLayout->setMargin(0);
   pLabelPropLayout->setSpacing(5);
   pLabelPropLayout->addWidget(pFontLabel, 0, 0);
   pLabelPropLayout->addWidget(mpFont, 0, 1, Qt::AlignLeft);
   pLabelPropLayout->addWidget(pFontSizeLabel, 1, 0);
   pLabelPropLayout->addWidget(mpFontSize, 1, 1, Qt::AlignLeft);
   pLabelPropLayout->setColumnStretch(1, 10);
   LabeledSection* pLabelSection = new LabeledSection(pLabelPropWidget, "Default Label Properties", this);

   // Gridline Properties
   QLabel* pStyleLabel = new QLabel("Style:", this);
   mpStyle = new LatLonStyleComboBox(this);

   QLabel* pLineWidthLabel = new QLabel("Width:", this);
   mpLineWidth = new LineWidthComboBox(this);

   QLabel* pColorLabel = new QLabel("Color:", this);
   mpColor = new CustomColorButton(this);
   mpColor->usePopupGrid(true);

   QWidget* pGridlinePropWidget = new QWidget(this);
   QGridLayout* pGridlinePropLayout = new QGridLayout(pGridlinePropWidget);
   pGridlinePropLayout->setMargin(0);
   pGridlinePropLayout->setSpacing(5);
   pGridlinePropLayout->addWidget(pStyleLabel, 0, 0);
   pGridlinePropLayout->addWidget(mpStyle, 0, 1, Qt::AlignLeft);
   pGridlinePropLayout->addWidget(pLineWidthLabel, 1, 0);
   pGridlinePropLayout->addWidget(mpLineWidth, 1, 1, Qt::AlignLeft);
   pGridlinePropLayout->addWidget(pColorLabel, 2, 0);
   pGridlinePropLayout->addWidget(mpColor, 2, 1, Qt::AlignLeft);
   pGridlinePropLayout->setColumnStretch(1, 10);
   LabeledSection* pGridlineSection = new LabeledSection(pGridlinePropWidget, "Default Gridline Properties", this);

   // Dialog layout
   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(10);
   pLayout->addWidget(pLabelSection);
   pLayout->addWidget(pGridlineSection);
   pLayout->addStretch(10);

   // Initialize From Settings
   mpStyle->setCurrentValue(LatLonLayer::getSettingGridlineStyle());
   mpLineWidth->setCurrentValue(LatLonLayer::getSettingGridlineWidth());
   mpFontSize->setEditText(QString::number(LatLonLayer::getSettingFontSize()));
   QFont textFont = QApplication::font();
   std::string fontName = LatLonLayer::getSettingFont();
   if (!fontName.empty())
   {
      textFont.setFamily(QString::fromStdString(fontName));
   }
   mpFont->setCurrentFont(textFont);
   mpColor->setColor(LatLonLayer::getSettingGridlineColor());
}

OptionsLatLonLayer::~OptionsLatLonLayer()
{}

void OptionsLatLonLayer::applyChanges()
{
   LatLonLayer::setSettingGridlineStyle(mpStyle->getCurrentValue());
   LatLonLayer::setSettingGridlineWidth(mpLineWidth->getCurrentValue());
   LatLonLayer::setSettingFontSize(mpFontSize->currentText().toUInt());
   LatLonLayer::setSettingFont(mpFont->currentFont().family().toStdString());
   LatLonLayer::setSettingGridlineColor(mpColor->getColorType());
}
