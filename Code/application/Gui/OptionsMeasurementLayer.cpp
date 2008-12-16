/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QApplication>
#include <QtGui/QCheckBox>
#include <QtGui/QColor>
#include <QtGui/QComboBox>
#include <QtGui/QFontComboBox>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QPainter>
#include <QtGui/QSpinBox>
#include <QtGui/QVBoxLayout>

#include "AppAssert.h"
#include "ColorType.h"
#include "CustomColorButton.h"
#include "FontSizeComboBox.h"
#include "Icons.h"
#include "LabeledSection.h"
#include "LineStyleComboBox.h"
#include "LineWidthComboBox.h"
#include "MeasurementToolBar.h"
#include "MeasurementLayer.h"
#include "OptionsMeasurementLayer.h"
#include "PixmapGrid.h"
#include "PixmapGridButton.h"

#include <string>

using namespace std;

OptionsMeasurementLayer::OptionsMeasurementLayer()
{
   Icons* pIcons = Icons::instance();
   REQUIRE(pIcons != NULL);

   // Line Properties
   QLabel* pLineWidthLabel = new QLabel("Line Width:", this);
   mpLineWidth = new LineWidthComboBox(this);

   QLabel* pLineStyleLabel = new QLabel("Line Style:", this);
   mpLineStyle = new LineStyleComboBox(this);

   QLabel* pLineColorLabel = new QLabel("Line Color:", this);
   mpLineColor = new CustomColorButton(this);
   mpLineColor->usePopupGrid(true);

   QWidget* pLinePropertiesWidget = new QWidget(this);
   QGridLayout* pLinePropLayout = new QGridLayout(pLinePropertiesWidget);
   pLinePropLayout->setMargin(0);
   pLinePropLayout->setSpacing(5);
   pLinePropLayout->addWidget(pLineWidthLabel, 0, 0);
   pLinePropLayout->addWidget(mpLineWidth, 0, 1, Qt::AlignLeft);
   pLinePropLayout->addWidget(pLineStyleLabel, 1, 0);
   pLinePropLayout->addWidget(mpLineStyle, 1, 1, Qt::AlignLeft);
   pLinePropLayout->addWidget(pLineColorLabel, 2, 0);
   pLinePropLayout->addWidget(mpLineColor, 2, 1, Qt::AlignLeft);
   pLinePropLayout->setColumnStretch(1, 10);
   LabeledSection* pLinePropertiesSection = new LabeledSection(pLinePropertiesWidget, "Default Line Properties", this);

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

   // Display Properties
   mpDisplayBearing = new QCheckBox("Display Bearing Label", this);
   mpDisplayDistance = new QCheckBox("Display Distance Label", this);
   mpDisplayEndPoints = new QCheckBox("Display End Points Label", this);

   QLabel* pBearingPrecLabel = new QLabel("Bearing Precision:", this);
   mpBearingPrecision = new QSpinBox(this);
   mpBearingPrecision->setRange(0, 10);

   QLabel* pDistancePrecLabel = new QLabel("Distance Precision:", this);
   mpDistancePrecision = new QSpinBox(this);
   mpDistancePrecision->setRange(0, 10);

   QLabel* pEndPointsPrecLabel = new QLabel("End Points Precision:", this);
   mpEndPointsPrecision = new QSpinBox(this);
   mpEndPointsPrecision->setRange(0, 10);

   // Location units button
   QLabel* pLocLabel = new QLabel("Location Units:", this);
   mpLocationUnits = new LocationUnitsButton(this);
   mpLocationUnits->setStatusTip("Allows the units for the location to be changed");
   mpLocationUnits->setToolTip("Location Units");

   // Distance units button
   QLabel* pDisLabel = new QLabel("Distance Units:", this);
   mpDistanceUnits = new DistanceUnitsButton(this);
   mpDistanceUnits->setStatusTip("Allows the units for the distance to be changed");
   mpDistanceUnits->setToolTip("Distance Units");

   QWidget* pDisplayPropertiesWidget = new QWidget(this);
   QGridLayout* pDisplayPropLayout = new QGridLayout(pDisplayPropertiesWidget);
   pDisplayPropLayout->setMargin(0);
   pDisplayPropLayout->setSpacing(5);
   pDisplayPropLayout->addWidget(mpDisplayBearing, 0, 0, 1, 2);
   pDisplayPropLayout->addWidget(pBearingPrecLabel, 0, 2, Qt::AlignRight);
   pDisplayPropLayout->addWidget(mpBearingPrecision, 0, 3);
   pDisplayPropLayout->addWidget(mpDisplayDistance, 1, 0, 1, 2);
   pDisplayPropLayout->addWidget(pDistancePrecLabel, 1, 2, Qt::AlignRight);
   pDisplayPropLayout->addWidget(mpDistancePrecision, 1, 3);
   pDisplayPropLayout->addWidget(mpDisplayEndPoints, 2, 0, 1, 2);
   pDisplayPropLayout->addWidget(pEndPointsPrecLabel, 2, 2, Qt::AlignRight);
   pDisplayPropLayout->addWidget(mpEndPointsPrecision, 2, 3);
   pDisplayPropLayout->addWidget(pLocLabel, 3, 0, Qt::AlignRight);
   pDisplayPropLayout->addWidget(mpLocationUnits, 3, 1);
   pDisplayPropLayout->addWidget(pDisLabel, 3, 2, Qt::AlignRight);
   pDisplayPropLayout->addWidget(mpDistanceUnits, 3, 3);
   pDisplayPropLayout->setColumnStretch(4, 10);
   LabeledSection* pDisplayPropertiesSection = new LabeledSection(pDisplayPropertiesWidget,
      "Default Display Properties", this);

   // Dialog layout
   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(10);
   pLayout->addWidget(pDisplayPropertiesSection);
   pLayout->addWidget(pLinePropertiesSection);
   pLayout->addWidget(pTextPropertiesSection);
   pLayout->addStretch(10);

   // Initialize From Settings
   mpDisplayBearing->setChecked(MeasurementLayer::getSettingDisplayBearingLabel());
   mpDisplayDistance->setChecked(MeasurementLayer::getSettingDisplayDistanceLabel());
   mpDisplayEndPoints->setChecked(MeasurementLayer::getSettingDisplayEndPointsLabel());
   mpBearingPrecision->setValue(MeasurementLayer::getSettingBearingPrecision());
   mpDistancePrecision->setValue(MeasurementLayer::getSettingDistancePrecision());
   mpEndPointsPrecision->setValue(MeasurementLayer::getSettingEndPointsPrecision());
   mpLocationUnits->setCurrentValue(MeasurementLayer::getSettingGeocoordType(), 
      MeasurementLayer::getSettingGeoFormat());
   mpDistanceUnits->setCurrentValue(MeasurementLayer::getSettingDistanceUnits());
   mpLineWidth->setCurrentValue(MeasurementLayer::getSettingLineWidth());
   mpLineStyle->setCurrentValue(MeasurementLayer::getSettingLineStyle());
   mpLineColor->setColor(MeasurementLayer::getSettingLineColor());
   mpTextColor->setColor(MeasurementLayer::getSettingTextColor());
   mpTextFontSize->setCurrentValue(MeasurementLayer::getSettingTextFontSize());
   QFont textFont = QApplication::font();
   string fontName = MeasurementLayer::getSettingTextFont();
   if (!fontName.empty())
   {
      textFont.setFamily(QString::fromStdString(fontName));
   }
   mpTextFont->setCurrentFont(textFont);
   mpBoldCheck->setChecked(MeasurementLayer::getSettingTextBold());
   mpItalicsCheck->setChecked(MeasurementLayer::getSettingTextItalics());
   mpUnderlineCheck->setChecked(MeasurementLayer::getSettingTextUnderline());
}

OptionsMeasurementLayer::~OptionsMeasurementLayer()
{
}

void OptionsMeasurementLayer::applyChanges()
{  
   MeasurementLayer::setSettingDisplayBearingLabel(mpDisplayBearing->isChecked());
   MeasurementLayer::setSettingDisplayDistanceLabel(mpDisplayDistance->isChecked());
   MeasurementLayer::setSettingDisplayEndPointsLabel(mpDisplayEndPoints->isChecked());
   MeasurementLayer::setSettingBearingPrecision(mpBearingPrecision->value());
   MeasurementLayer::setSettingDistancePrecision(mpDistancePrecision->value());
   MeasurementLayer::setSettingEndPointsPrecision(mpEndPointsPrecision->value());
   MeasurementLayer::setSettingDistanceUnits(mpDistanceUnits->getCurrentValue());

   GeocoordType geoType;
   DmsFormatType geoFormat;
   mpLocationUnits->getCurrentValue(geoType, geoFormat);
   MeasurementLayer::setSettingGeocoordType(geoType);
   MeasurementLayer::setSettingGeoFormat(geoFormat);

   MeasurementLayer::setSettingLineWidth(mpLineWidth->getCurrentValue());
   MeasurementLayer::setSettingLineStyle(mpLineStyle->getCurrentValue());
   MeasurementLayer::setSettingLineColor(mpLineColor->getColorType());
   MeasurementLayer::setSettingTextColor(mpTextColor->getColorType());
   MeasurementLayer::setSettingTextFontSize(mpTextFontSize->getCurrentValue());
   MeasurementLayer::setSettingTextFont(mpTextFont->currentFont().family().toStdString());
   MeasurementLayer::setSettingTextBold(mpBoldCheck->isChecked());
   MeasurementLayer::setSettingTextItalics(mpItalicsCheck->isChecked());
   MeasurementLayer::setSettingTextUnderline(mpUnderlineCheck->isChecked());
}
