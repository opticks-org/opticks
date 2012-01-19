/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "Georeference.h"
#include "LabeledSection.h"
#include "MutuallyExclusiveListWidget.h"
#include "OptionsGeoreference.h"
#include "PlugInDescriptor.h"
#include "PlugInManagerServices.h"

#include <QtGui/QCheckBox>
#include <QtGui/QGridLayout>
#include <QtGui/QRadioButton>
#include <QtGui/QStyle>
#include <QtGui/QStyleOptionButton>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

#include <vector>

OptionsGeoreference::OptionsGeoreference() :
   QWidget(NULL),
   mpAutoGeoreference(NULL),
   mpImporterPlugInRadio(NULL),
   mpBestPlugInRadio(NULL),
   mpPlugInList(NULL),
   mpCreateLayer(NULL),
   mpDisplayLayer(NULL)
{
   // Georeference
   QWidget* pGeoWidget = new QWidget(this);
   mpCreateLayer = new QCheckBox("Create latitude/longitude layer", pGeoWidget);
   mpDisplayLayer = new QCheckBox("Display latitude/longitude layer", pGeoWidget);
   QVBoxLayout* pGeoLayout = new QVBoxLayout(pGeoWidget);
   pGeoLayout->setMargin(0);
   pGeoLayout->setSpacing(10);
   pGeoLayout->addWidget(mpCreateLayer);
   pGeoLayout->addWidget(mpDisplayLayer);
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
   pAutoGeorefGrid->setSpacing(10);
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
   VERIFYNR(connect(mpBestPlugInRadio, SIGNAL(toggled(bool)), mpPlugInList, SLOT(setEnabled(bool))));
   VERIFYNR(connect(mpCreateLayer, SIGNAL(toggled(bool)), this, SLOT(createLayerChanged(bool))));
   VERIFYNR(connect(mpAutoGeoreference, SIGNAL(toggled(bool)), this, SLOT(enableAutoOptions(bool))));
}

OptionsGeoreference::~OptionsGeoreference()
{}

void OptionsGeoreference::applyChanges()
{
   Georeference::setSettingAutoGeoreference(mpAutoGeoreference->isChecked());
   Georeference::setSettingImporterGeoreferencePlugIn(mpImporterPlugInRadio->isChecked());
   Georeference::setSettingCreateLatLonLayer(mpCreateLayer->isChecked());
   Georeference::setSettingDisplayLatLonLayer(mpDisplayLayer->isChecked());

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

void OptionsGeoreference::enableAutoOptions(bool enabled)
{
   mpImporterPlugInRadio->setEnabled(enabled);
   mpBestPlugInRadio->setEnabled(enabled);
   mpPlugInList->setEnabled(enabled && mpBestPlugInRadio->isChecked());
}
