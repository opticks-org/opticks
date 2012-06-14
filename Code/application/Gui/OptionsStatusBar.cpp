/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QCheckBox>
#include <QtGui/QGridLayout>
#include <QtGui/QVBoxLayout>

#include "OptionsStatusBar.h"
#include "ConfigurationSettings.h"
#include "LabeledSection.h"

OptionsStatusBar::OptionsStatusBar() :
   QWidget(NULL)
{
   // Displayed Fields
   QWidget* pFieldsWidget = new QWidget(this);

   mpPixelCoordsCheck = new QCheckBox("Pixel Coordinate", pFieldsWidget);
   mpGeoCoordsCheck = new QCheckBox("Geo Coordinate", pFieldsWidget);
   mpCubeValueCheck = new QCheckBox("Cube Value", pFieldsWidget);
   mpCubeValueUnitsCheck = new QCheckBox("Cube Value Units", pFieldsWidget);
   mpResultValueCheck = new QCheckBox("Result Layer Value", pFieldsWidget);
   mpRotationValueCheck = new QCheckBox("Rotation", pFieldsWidget);
   mpElevationValueCheck = new QCheckBox("Elevation", pFieldsWidget);

   LabeledSection* pFieldsSection = new LabeledSection(pFieldsWidget, "Displayed Fields", this);

   QGridLayout* pFieldsLayout = new QGridLayout(pFieldsWidget);
   pFieldsLayout->setMargin(0);
   pFieldsLayout->setSpacing(5);
   pFieldsLayout->addWidget(mpPixelCoordsCheck, 0, 0);
   pFieldsLayout->addWidget(mpGeoCoordsCheck, 1, 0);
   pFieldsLayout->addWidget(mpCubeValueCheck, 2, 0);
   pFieldsLayout->addWidget(mpCubeValueUnitsCheck, 2, 1);
   pFieldsLayout->addWidget(mpResultValueCheck, 3, 0);
   pFieldsLayout->addWidget(mpRotationValueCheck, 4, 0);
   pFieldsLayout->addWidget(mpElevationValueCheck, 5, 0);
   pFieldsLayout->setRowStretch(5, 10);
   pFieldsLayout->setColumnStretch(1, 10);

   // Dialog layout
   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(5);
   pLayout->addWidget(pFieldsSection);
   pLayout->addStretch(10);

   VERIFYNR(connect(mpCubeValueCheck, SIGNAL(toggled(bool)), mpCubeValueUnitsCheck, SLOT(setEnabled(bool))));

   // Initialize From Settings
   mpPixelCoordsCheck->setChecked(ConfigurationSettings::getSettingShowStatusBarPixelCoords());
   mpGeoCoordsCheck->setChecked(ConfigurationSettings::getSettingShowStatusBarGeoCoords());
   mpCubeValueUnitsCheck->setChecked(ConfigurationSettings::getSettingShowStatusBarCubeValueUnits());
   mpCubeValueCheck->setChecked(ConfigurationSettings::getSettingShowStatusBarCubeValue());
   mpResultValueCheck->setChecked(ConfigurationSettings::getSettingShowStatusBarResultValue());
   mpRotationValueCheck->setChecked(ConfigurationSettings::getSettingShowStatusBarRotationValue());
   mpElevationValueCheck->setChecked(ConfigurationSettings::getSettingShowStatusBarElevationValue());
}
   
void OptionsStatusBar::applyChanges()
{  
   ConfigurationSettings::setSettingShowStatusBarPixelCoords(mpPixelCoordsCheck->isChecked());
   ConfigurationSettings::setSettingShowStatusBarGeoCoords(mpGeoCoordsCheck->isChecked());
   ConfigurationSettings::setSettingShowStatusBarCubeValue(mpCubeValueCheck->isChecked());
   ConfigurationSettings::setSettingShowStatusBarCubeValueUnits(mpCubeValueUnitsCheck->isChecked());
   ConfigurationSettings::setSettingShowStatusBarResultValue(mpResultValueCheck->isChecked());
   ConfigurationSettings::setSettingShowStatusBarRotationValue(mpRotationValueCheck->isChecked());
   ConfigurationSettings::setSettingShowStatusBarElevationValue(mpElevationValueCheck->isChecked());
}

OptionsStatusBar::~OptionsStatusBar()
{
}
