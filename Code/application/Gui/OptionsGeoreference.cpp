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
#include "Georeference.h"
#include "LabeledSection.h"
#include "MutuallyExclusiveListWidget.h"
#include "OptionsGeoreference.h"
#include "PlugInDescriptor.h"
#include "PlugInManagerServices.h"

#include <QtGui/QCheckBox>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QRadioButton>
#include <QtGui/QStyle>
#include <QtGui/QStyleOptionButton>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

#include <vector>

OptionsGeoreference::OptionsGeoreference() :
   QWidget(NULL),
   mpCreateLayer(NULL),
   mpDisplayLayer(NULL),
   mpGeocoordTypeCombo(NULL),
   mpLatLonFormatLabel(NULL),
   mpLatLonFormatCombo(NULL),
   mpAutoGeoreference(NULL),
   mpImporterPlugInRadio(NULL),
   mpBestPlugInRadio(NULL),
   mpPlugInList(NULL)
{
   // Georeference
   QWidget* pGeoWidget = new QWidget(this);
   mpCreateLayer = new QCheckBox("Create latitude/longitude layer", pGeoWidget);
   mpDisplayLayer = new QCheckBox("Display latitude/longitude layer", pGeoWidget);
   QLabel* pGeocoordTypeLabel = new QLabel("Geocoordinate Type:", pGeoWidget);
   mpGeocoordTypeCombo = new GeocoordTypeComboBox(pGeoWidget);
   mpLatLonFormatLabel = new QLabel("Latitude/Longitude Format:", pGeoWidget);
   mpLatLonFormatCombo = new DmsFormatTypeComboBox(pGeoWidget);

   QGridLayout* pGeoLayout = new QGridLayout(pGeoWidget);
   pGeoLayout->setMargin(0);
   pGeoLayout->setSpacing(5);
   pGeoLayout->addWidget(mpCreateLayer, 0, 0, 1, 2);
   pGeoLayout->addWidget(mpDisplayLayer, 1, 0, 1, 2);
   pGeoLayout->addWidget(pGeocoordTypeLabel, 2, 0);
   pGeoLayout->addWidget(mpGeocoordTypeCombo, 2, 1, Qt::AlignLeft);
   pGeoLayout->addWidget(mpLatLonFormatLabel, 3, 0);
   pGeoLayout->addWidget(mpLatLonFormatCombo, 3, 1, Qt::AlignLeft);
   pGeoLayout->setColumnStretch(1, 10);

   LabeledSection* pGeoSection = new LabeledSection(pGeoWidget, "Georeference", this);

   // Auto-Georeference
   QWidget* pAutoWidget = new QWidget(this);
   mpAutoGeoreference = new QCheckBox("Automatically georeference on import", pAutoWidget);

   mpImporterPlugInRadio = new QRadioButton("Use plug-in determined by the importer", pAutoWidget);
   mpBestPlugInRadio = new QRadioButton("Use best available plug-in:", pAutoWidget);
   mpPlugInList = new MutuallyExclusiveListWidget(pAutoWidget);

   QStyle* pStyle = style();
   VERIFYNRV(pStyle != NULL);

   QStyleOptionButton checkOption;
   checkOption.initFrom(mpAutoGeoreference);
   int checkWidth = pStyle->subElementRect(QStyle::SE_CheckBoxIndicator, &checkOption).width();

   QStyleOptionButton radioOption;
   radioOption.initFrom(mpBestPlugInRadio);
   int radioWidth = pStyle->subElementRect(QStyle::SE_RadioButtonIndicator, &radioOption).width();

   QGridLayout* pAutoGeorefGrid = new QGridLayout(pAutoWidget);
   pAutoGeorefGrid->setMargin(0);
   pAutoGeorefGrid->setSpacing(5);
   pAutoGeorefGrid->addWidget(mpAutoGeoreference, 0, 0, 1, 3);
   pAutoGeorefGrid->addWidget(mpImporterPlugInRadio, 1, 1, 1, 2);
   pAutoGeorefGrid->addWidget(mpBestPlugInRadio, 2, 1, 1, 2);
   pAutoGeorefGrid->addWidget(mpPlugInList, 3, 2);
   pAutoGeorefGrid->setColumnMinimumWidth(0, checkWidth);
   pAutoGeorefGrid->setColumnMinimumWidth(1, radioWidth);
   pAutoGeorefGrid->setRowStretch(3, 10);
   pAutoGeorefGrid->setColumnStretch(2, 10);

   LabeledSection* pAutoGeoSection = new LabeledSection(pAutoWidget, "Auto-Georeference", this);

   // Layout
   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(10);
   pLayout->addWidget(pGeoSection);
   pLayout->addWidget(pAutoGeoSection, 10);

   // Initialization
   mpCreateLayer->setChecked(Georeference::getSettingCreateLatLonLayer());
   mpDisplayLayer->setChecked(
      mpCreateLayer->isChecked() ? Georeference::getSettingDisplayLatLonLayer() : false);
   mpDisplayLayer->setEnabled(mpCreateLayer->isChecked());
   mpGeocoordTypeCombo->setGeocoordType(Georeference::getSettingGeocoordType());
   mpLatLonFormatLabel->setEnabled(Georeference::getSettingGeocoordType() == GEOCOORD_LATLON);
   mpLatLonFormatCombo->setCurrentValue(Georeference::getSettingLatLonFormat());
   mpLatLonFormatCombo->setEnabled(Georeference::getSettingGeocoordType() == GEOCOORD_LATLON);

   mpAutoGeoreference->setChecked(Georeference::getSettingAutoGeoreference());
   mpImporterPlugInRadio->setChecked(Georeference::getSettingImporterGeoreferencePlugIn());
   mpBestPlugInRadio->setChecked(!Georeference::getSettingImporterGeoreferencePlugIn());
   mpPlugInList->setDisabled(Georeference::getSettingImporterGeoreferencePlugIn());
   enableAutoOptions(mpAutoGeoreference->isChecked());
   Service<PlugInManagerServices> pManager;
   QStringList plugInNames;

   std::vector<PlugInDescriptor*> descriptors = pManager->getPlugInDescriptors("Georeference");
   for (std::vector<PlugInDescriptor*>::const_iterator iter = descriptors.begin(); iter != descriptors.end(); ++iter)
   {
      PlugInDescriptor* pDescriptor = *iter;
      if (pDescriptor != NULL)
      {
         const std::string& plugInName = pDescriptor->getName();
         if (plugInName.empty() == false)
         {
            plugInNames.append(QString::fromStdString(plugInName));
         }
      }
   }

   QStringList selectedPlugInNames;

   std::vector<std::string> geoPlugIns = Georeference::getSettingGeoreferencePlugIns();
   for (std::vector<std::string>::const_iterator iter = geoPlugIns.begin(); iter != geoPlugIns.end(); ++iter)
   {
      std::string plugInName = *iter;
      if (plugInName.empty() == false)
      {
         selectedPlugInNames.append(QString::fromStdString(plugInName));
      }
   }

   mpPlugInList->setAvailableItemsLabel("Available Plug-Ins:");
   mpPlugInList->setAvailableItems(plugInNames);
   mpPlugInList->setSelectedItemsLabel("Preferred Plug-Ins:");
   mpPlugInList->selectItems(selectedPlugInNames);

   // Connections
   VERIFYNR(connect(mpCreateLayer, SIGNAL(toggled(bool)), this, SLOT(createLayerChanged(bool))));
   VERIFYNR(connect(mpGeocoordTypeCombo, SIGNAL(geocoordTypeChanged(GeocoordType)), this,
      SLOT(geocoordTypeChanged(GeocoordType))));
   VERIFYNR(connect(mpAutoGeoreference, SIGNAL(toggled(bool)), this, SLOT(enableAutoOptions(bool))));
   VERIFYNR(connect(mpBestPlugInRadio, SIGNAL(toggled(bool)), mpPlugInList, SLOT(setEnabled(bool))));
}

OptionsGeoreference::~OptionsGeoreference()
{}

void OptionsGeoreference::applyChanges()
{
   Georeference::setSettingCreateLatLonLayer(mpCreateLayer->isChecked());
   Georeference::setSettingDisplayLatLonLayer(mpDisplayLayer->isChecked());
   Georeference::setSettingGeocoordType(mpGeocoordTypeCombo->getGeocoordType());
   if (mpGeocoordTypeCombo->getGeocoordType() == GEOCOORD_LATLON)
   {
      Georeference::setSettingLatLonFormat(mpLatLonFormatCombo->getCurrentValue());
   }

   Georeference::setSettingAutoGeoreference(mpAutoGeoreference->isChecked());
   Georeference::setSettingImporterGeoreferencePlugIn(mpImporterPlugInRadio->isChecked());

   std::vector<std::string> plugIns;

   QStringList selectedPlugIns = mpPlugInList->getSelectedItems();
   for (int i = 0; i < selectedPlugIns.count(); ++i)
   {
      QString plugInName = selectedPlugIns[i];
      if (plugInName.isEmpty() == false)
      {
         plugIns.push_back(plugInName.toStdString());
      }
   }

   Georeference::setSettingGeoreferencePlugIns(plugIns);
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

void OptionsGeoreference::enableAutoOptions(bool enabled)
{
   mpImporterPlugInRadio->setEnabled(enabled);
   mpBestPlugInRadio->setEnabled(enabled);
   mpPlugInList->setEnabled(enabled && mpBestPlugInRadio->isChecked());
}
