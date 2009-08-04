/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "OptionsAnnotationLayer.h"

#include "ArcRegionComboBox.h"
#include "ColorType.h"
#include "CustomColorButton.h"
#include "FillStyleComboBox.h"
#include "FontSizeComboBox.h"
#include "GraphicLayer.h"
#include "GraphicUnitsWidget.h"
#include "LabeledSection.h"
#include "LineStyleComboBox.h"
#include "LineWidthComboBox.h"
#include "PixmapGridButton.h"
#include "SymbolTypeGrid.h"

#include <QtGui/QApplication>
#include <QtGui/QCheckBox>
#include <QtGui/QColor>
#include <QtGui/QComboBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QFontComboBox>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QPainter>
#include <QtGui/QSpinBox>
#include <QtGui/QVBoxLayout>

#include <string>

using namespace std;

OptionsAnnotationLayer::OptionsAnnotationLayer() :
   QWidget(NULL)
{
   //NOTE: We are not creating widgets to edit the following
   //Graphic Layer defaults: Rotation, Line Scaled, SymbolName, SymbolSize

   // Common Properties
   QLabel* pLineWidthLabel = new QLabel("Line Width:", this);
   mpLineWidth = new LineWidthComboBox(this);

   QLabel* pLineStyleLabel = new QLabel("Line Style:", this);
   mpLineStyle = new LineStyleComboBox(this);

   QLabel* pLineColorLabel = new QLabel("Line Color:", this);
   mpLineColor = new CustomColorButton(this);
   mpLineColor->usePopupGrid(true);

   QLabel* pHatchStyleLabel = new QLabel("Hatch Style:", this);
   mpHatchStyle = new SymbolTypeButton(this);
   mpHatchStyle->setBorderedSymbols(false);

   QLabel* pAlphaLabel = new QLabel("Opacity:", this);
   mpAlpha = new QSpinBox(this);
   mpAlpha->setRange(0, 100);
   mpAlpha->setSuffix("%");

   QLabel* pFillStyleLabel = new QLabel("Fill Style:", this);
   mpFillStyle = new FillStyleComboBox(this);

   QLabel* pFillColorLabel = new QLabel("Fill Color:", this);
   mpFillColor = new CustomColorButton(this);
   mpFillColor->usePopupGrid(true);

   mpObjectFill = new QCheckBox("Object Fill", this);
   mpObjectBorder = new QCheckBox("Object Border", this);

   QWidget* pCommonPropertiesWidget = new QWidget(this);
   QGridLayout* pCommonPropLayout = new QGridLayout(pCommonPropertiesWidget);
   pCommonPropLayout->setMargin(0);
   pCommonPropLayout->setSpacing(5);
   pCommonPropLayout->addWidget(pLineWidthLabel, 0, 0);
   pCommonPropLayout->addWidget(mpLineWidth, 0, 1, Qt::AlignLeft);
   pCommonPropLayout->addWidget(pLineStyleLabel, 1, 0);
   pCommonPropLayout->addWidget(mpLineStyle, 1, 1, Qt::AlignLeft);
   pCommonPropLayout->addWidget(pLineColorLabel, 2, 0);
   pCommonPropLayout->addWidget(mpLineColor, 2, 1, Qt::AlignLeft);
   pCommonPropLayout->addWidget(pHatchStyleLabel, 3, 0);
   pCommonPropLayout->addWidget(mpHatchStyle, 3, 1, Qt::AlignLeft);
   pCommonPropLayout->addWidget(pAlphaLabel, 4, 0);
   pCommonPropLayout->addWidget(mpAlpha, 4, 1, Qt::AlignLeft);
   pCommonPropLayout->addWidget(pFillStyleLabel, 5, 0);
   pCommonPropLayout->addWidget(mpFillStyle, 5, 1, Qt::AlignLeft);
   pCommonPropLayout->addWidget(pFillColorLabel, 6, 0);
   pCommonPropLayout->addWidget(mpFillColor, 6, 1, Qt::AlignLeft);
   pCommonPropLayout->addWidget(mpObjectFill, 7, 0, 1, 2, Qt::AlignLeft);
   pCommonPropLayout->addWidget(mpObjectBorder, 8, 0, 1, 2, Qt::AlignLeft);
   pCommonPropLayout->setColumnStretch(1, 10);
   LabeledSection* pCommonPropertiesSection = new LabeledSection(pCommonPropertiesWidget,
      "Default Common Properties", this);

   // Text Properties
   QLabel* pTextColorLabel = new QLabel("Font Color:", this);
   mpTextColor = new CustomColorButton(this);
   mpTextColor->usePopupGrid(true);

   QLabel* pTextFontLabel = new QLabel("Font:", this);
   mpTextFont = new QFontComboBox(this);

   QLabel* pTextFontSizeLabel = new QLabel("Font Size:", this);
   mpTextFontSize = new FontSizeComboBox(this);

   mpBoldCheck = new QCheckBox("Bold", this);
   mpItalicsCheck = new QCheckBox("Italics", this);
   mpUnderlineCheck = new QCheckBox("Underline", this);

   QWidget* pTextPropertiesWidget = new QWidget(this);
   QGridLayout* pTextPropLayout = new QGridLayout(pTextPropertiesWidget);
   pTextPropLayout->setMargin(0);
   pTextPropLayout->setSpacing(5);
   pTextPropLayout->addWidget(pTextColorLabel, 0, 0);
   pTextPropLayout->addWidget(mpTextColor, 0, 1, Qt::AlignLeft);
   pTextPropLayout->addWidget(pTextFontLabel, 1, 0);
   pTextPropLayout->addWidget(mpTextFont, 1, 1, Qt::AlignLeft);
   pTextPropLayout->addWidget(pTextFontSizeLabel, 2, 0);
   pTextPropLayout->addWidget(mpTextFontSize, 2, 1, Qt::AlignLeft);
   pTextPropLayout->addWidget(mpBoldCheck, 3, 0, 1, 2, Qt::AlignLeft);
   pTextPropLayout->addWidget(mpItalicsCheck, 4, 0, 1, 2, Qt::AlignLeft);
   pTextPropLayout->addWidget(mpUnderlineCheck, 5, 0, 1, 2, Qt::AlignLeft);
   pTextPropLayout->setColumnStretch(1, 10);
   LabeledSection* pTextPropertiesSection = new LabeledSection(pTextPropertiesWidget, "Default Text Properties", this);

   // Arc Properties
   QLabel* pStartAngleLabel = new QLabel("Start Angle:", this);
   mpStartAngle = new QDoubleSpinBox(this);
   mpStartAngle->setRange(-360.0, 360.0);
   mpStartAngle->setDecimals(1);
   mpStartAngle->setSuffix(" Degrees");

   QLabel* pStopAngleLabel = new QLabel("Stop Angle:", this);
   mpStopAngle = new QDoubleSpinBox(this);
   mpStopAngle->setRange(-360.0, 360.0);
   mpStopAngle->setDecimals(1);
   mpStopAngle->setSuffix(" Degrees");

   QLabel* pArcRegionLabel = new QLabel("Arc Region:", this);
   mpArcRegion = new ArcRegionComboBox(this);

   QWidget* pArcPropertiesWidget = new QWidget(this);
   QGridLayout* pArcPropLayout = new QGridLayout(pArcPropertiesWidget);
   pArcPropLayout->setMargin(0);
   pArcPropLayout->setSpacing(5);
   pArcPropLayout->addWidget(pStartAngleLabel, 0, 0);
   pArcPropLayout->addWidget(mpStartAngle, 0, 1, Qt::AlignLeft);
   pArcPropLayout->addWidget(pStopAngleLabel, 1, 0);
   pArcPropLayout->addWidget(mpStopAngle, 1, 1, Qt::AlignLeft);
   pArcPropLayout->addWidget(pArcRegionLabel, 2, 0);
   pArcPropLayout->addWidget(mpArcRegion, 2, 1, Qt::AlignLeft);
   pArcPropLayout->setColumnStretch(1, 10);
   LabeledSection* pArcPropertiesSection = new LabeledSection(pArcPropertiesWidget, "Default Arc Properties", this);

   // Triangle Properties
   QLabel* pApexLabel = new QLabel("Apex:", this);
   mpApex = new QSpinBox(this);
   mpApex->setRange(0, 100);
   mpApex->setSuffix("%");

   QWidget* pTrianglePropertiesWidget = new QWidget(this);
   QGridLayout* pTrianglePropLayout = new QGridLayout(pTrianglePropertiesWidget);
   pTrianglePropLayout->setMargin(0);
   pTrianglePropLayout->setSpacing(5);
   pTrianglePropLayout->addWidget(pApexLabel, 0, 0);
   pTrianglePropLayout->addWidget(mpApex, 0, 1, Qt::AlignLeft);
   pTrianglePropLayout->setColumnStretch(1, 10);
   LabeledSection* pTrianglePropertiesSection = new LabeledSection(pTrianglePropertiesWidget,
      "Default Triangle Properties", this);

   // Units
   mpUnitsWidget = new GraphicUnitsWidget(this);
   LabeledSection* pUnitsSection = new LabeledSection(mpUnitsWidget, "Default Unit System", this);

   // Dialog layout
   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(10);
   pLayout->addWidget(pCommonPropertiesSection);
   pLayout->addWidget(pTextPropertiesSection);
   pLayout->addWidget(pArcPropertiesSection);
   pLayout->addWidget(pTrianglePropertiesSection);
   pLayout->addWidget(pUnitsSection);
   pLayout->addStretch(10);

   // Initialize From Settings
   mpTextColor->setColor(GraphicLayer::getSettingTextColor());
   mpStartAngle->setValue(GraphicLayer::getSettingStartAngle());
   mpStopAngle->setValue(GraphicLayer::getSettingStopAngle());
   mpObjectFill->setChecked(GraphicLayer::getSettingFill());
   mpObjectBorder->setChecked(GraphicLayer::getSettingBorder());
   mpLineWidth->setCurrentValue(GraphicLayer::getSettingLineWidth());
   mpLineStyle->setCurrentValue(GraphicLayer::getSettingLineStyle());
   mpLineColor->setColor(GraphicLayer::getSettingLineColor());
   mpHatchStyle->setCurrentValue(GraphicLayer::getSettingHatchStyle());
   mpTextFontSize->setCurrentValue(GraphicLayer::getSettingTextFontSize());
   QFont textFont = QApplication::font();
   string fontName = GraphicLayer::getSettingTextFont();
   if (!fontName.empty())
   {
      textFont.setFamily(QString::fromStdString(fontName));
   }
   mpTextFont->setCurrentFont(textFont);
   mpBoldCheck->setChecked(GraphicLayer::getSettingTextBold());
   mpItalicsCheck->setChecked(GraphicLayer::getSettingTextItalics());
   mpUnderlineCheck->setChecked(GraphicLayer::getSettingTextUnderline());
   mpFillStyle->setCurrentValue(GraphicLayer::getSettingFillStyle());
   mpFillColor->setColor(GraphicLayer::getSettingFillColor());
   mpArcRegion->setCurrentValue(GraphicLayer::getSettingArcRegion());
   double apex = GraphicLayer::getSettingApex();
   int apexPercentage = apex * 100;
   mpApex->setValue(apexPercentage);
   double alpha = GraphicLayer::getSettingAlpha();
   int opacity = static_cast<int>(alpha / 2.550f + 0.5f); 
   mpAlpha->setValue(opacity);
   mpUnitsWidget->setUnitSystem(GraphicLayer::getSettingUnitSystem());
}

void OptionsAnnotationLayer::applyChanges()
{
   GraphicLayer::setSettingTextColor(mpTextColor->getColorType());
   GraphicLayer::setSettingStartAngle(mpStartAngle->value());
   GraphicLayer::setSettingStopAngle(mpStopAngle->value());
   GraphicLayer::setSettingFill(mpObjectFill->isChecked());
   GraphicLayer::setSettingBorder(mpObjectBorder->isChecked());
   GraphicLayer::setSettingLineWidth(mpLineWidth->getCurrentValue());
   GraphicLayer::setSettingLineStyle(mpLineStyle->getCurrentValue());
   GraphicLayer::setSettingLineColor(mpLineColor->getColorType());
   GraphicLayer::setSettingHatchStyle(mpHatchStyle->getCurrentValue());
   GraphicLayer::setSettingTextFontSize(mpTextFontSize->getCurrentValue());
   GraphicLayer::setSettingTextFont(mpTextFont->currentFont().family().toStdString());
   GraphicLayer::setSettingTextBold(mpBoldCheck->isChecked());
   GraphicLayer::setSettingTextItalics(mpItalicsCheck->isChecked());
   GraphicLayer::setSettingTextUnderline(mpUnderlineCheck->isChecked());
   GraphicLayer::setSettingFillStyle(mpFillStyle->getCurrentValue());
   GraphicLayer::setSettingFillColor(mpFillColor->getColorType());
   GraphicLayer::setSettingArcRegion(mpArcRegion->getCurrentValue());
   int apexPercentage = mpApex->value();
   double apex = apexPercentage / 100.0;
   GraphicLayer::setSettingApex(apex);
   int alpha = static_cast<int>(mpAlpha->value() * 2.55f + 0.5f);
   if (alpha < 0)
   {
      alpha = 0;
   }
   else if (alpha > 255)
   {
      alpha = 255;
   }
   GraphicLayer::setSettingAlpha(alpha);
   GraphicLayer::setSettingUnitSystem(mpUnitsWidget->getUnitSystem());
}

OptionsAnnotationLayer::~OptionsAnnotationLayer()
{
}
