/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "DmsFormatTypeComboBox.h"
#include "GeocoordTypeComboBox.h"
#include "GeoreferenceDescriptor.h"
#include "LabeledSection.h"
#include "OptionsGeoreference.h"

#include <QtGui/QCheckBox>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QVBoxLayout>

OptionsGeoreference::OptionsGeoreference() :
   QWidget(NULL),
   mpAutoGeoreference(NULL),
   mpCreateLayer(NULL),
   mpDisplayLayer(NULL),
   mpGeocoordTypeCombo(NULL),
   mpLatLonFormatLabel(NULL),
   mpLatLonFormatCombo(NULL)
{
   // Georeference
   QWidget* pGeoWidget = new QWidget(this);
   mpAutoGeoreference = new QCheckBox("Automatically georeference on import", pGeoWidget);
   mpCreateLayer = new QCheckBox("Create latitude/longitude layer", pGeoWidget);
   mpDisplayLayer = new QCheckBox("Display latitude/longitude layer", pGeoWidget);
   QLabel* pGeocoordTypeLabel = new QLabel("Geocoordinate Type:", pGeoWidget);
   mpGeocoordTypeCombo = new GeocoordTypeComboBox(pGeoWidget);
   mpLatLonFormatLabel = new QLabel("Latitude/Longitude Format:", pGeoWidget);
   mpLatLonFormatCombo = new DmsFormatTypeComboBox(pGeoWidget);

   QGridLayout* pGeoLayout = new QGridLayout(pGeoWidget);
   pGeoLayout->setMargin(0);
   pGeoLayout->setSpacing(5);
   pGeoLayout->addWidget(mpAutoGeoreference, 0, 0, 1, 2);
   pGeoLayout->addWidget(mpCreateLayer, 1, 0, 1, 2);
   pGeoLayout->addWidget(mpDisplayLayer, 2, 0, 1, 2);
   pGeoLayout->addWidget(pGeocoordTypeLabel, 3, 0);
   pGeoLayout->addWidget(mpGeocoordTypeCombo, 3, 1, Qt::AlignLeft);
   pGeoLayout->addWidget(mpLatLonFormatLabel, 4, 0);
   pGeoLayout->addWidget(mpLatLonFormatCombo, 4, 1, Qt::AlignLeft);
   pGeoLayout->setRowStretch(5, 10);
   pGeoLayout->setColumnStretch(1, 10);

   LabeledSection* pGeoSection = new LabeledSection(pGeoWidget, "Georeference", this);

   // Layout
   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(10);
   pLayout->addWidget(pGeoSection, 10);

   // Initialization
   mpAutoGeoreference->setChecked(GeoreferenceDescriptor::getSettingAutoGeoreference());
   mpCreateLayer->setChecked(GeoreferenceDescriptor::getSettingCreateLayer());
   mpDisplayLayer->setChecked(
      mpCreateLayer->isChecked() ? GeoreferenceDescriptor::getSettingDisplayLayer() : false);
   mpDisplayLayer->setEnabled(mpCreateLayer->isChecked());
   mpGeocoordTypeCombo->setGeocoordType(GeoreferenceDescriptor::getSettingGeocoordType());
   mpLatLonFormatLabel->setEnabled(GeoreferenceDescriptor::getSettingGeocoordType() == GEOCOORD_LATLON);
   mpLatLonFormatCombo->setCurrentValue(GeoreferenceDescriptor::getSettingLatLonFormat());
   mpLatLonFormatCombo->setEnabled(GeoreferenceDescriptor::getSettingGeocoordType() == GEOCOORD_LATLON);

   // Connections
   VERIFYNR(connect(mpCreateLayer, SIGNAL(toggled(bool)), this, SLOT(createLayerChanged(bool))));
   VERIFYNR(connect(mpGeocoordTypeCombo, SIGNAL(geocoordTypeChanged(GeocoordType)), this,
      SLOT(geocoordTypeChanged(GeocoordType))));
}

OptionsGeoreference::~OptionsGeoreference()
{}

void OptionsGeoreference::applyChanges()
{
   GeoreferenceDescriptor::setSettingAutoGeoreference(mpAutoGeoreference->isChecked());
   GeoreferenceDescriptor::setSettingCreateLayer(mpCreateLayer->isChecked());
   GeoreferenceDescriptor::setSettingDisplayLayer(mpDisplayLayer->isChecked());
   GeoreferenceDescriptor::setSettingGeocoordType(mpGeocoordTypeCombo->getGeocoordType());
   GeoreferenceDescriptor::setSettingLatLonFormat(mpLatLonFormatCombo->getCurrentValue());
}

void OptionsGeoreference::createLayerChanged(bool create)
{
   if (create == false)
   {
      mpDisplayLayer->setChecked(false);
   }

   mpDisplayLayer->setEnabled(create);
}

void OptionsGeoreference::geocoordTypeChanged(GeocoordType geocoordType)
{
   bool enableLatLonFormat = (geocoordType == GEOCOORD_LATLON);
   mpLatLonFormatLabel->setEnabled(enableLatLonFormat);
   mpLatLonFormatCombo->setEnabled(enableLatLonFormat);
}
