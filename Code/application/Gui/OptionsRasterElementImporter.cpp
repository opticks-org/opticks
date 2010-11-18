/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "LabeledSection.h"
#include "MutuallyExclusiveListWidget.h"
#include "OptionsRasterElementImporter.h"
#include "PlugInDescriptor.h"
#include "PlugInManagerServices.h"
#include "RasterElementImporterShell.h"

#include <QtGui/QCheckBox>
#include <QtGui/QGroupBox>
#include <QtGui/QLayout>
#include <QtGui/QRadioButton>

#include <vector>

OptionsRasterElementImporter::OptionsRasterElementImporter() :
   QWidget(NULL),
   mpAutoGeorefGroup(NULL),
   mpImporterPlugInRadio(NULL),
   mpPlugInList(NULL),
   mpLatLonLayerCheck(NULL)
{
   // Georeference
   mpAutoGeorefGroup = new QGroupBox("Automatically georeference on import", this);
   mpAutoGeorefGroup->setCheckable(true);

   mpImporterPlugInRadio = new QRadioButton("Use plug-in determined by the importer", mpAutoGeorefGroup);
   QRadioButton* pPlugInRadio = new QRadioButton("Use best available plug-in:", mpAutoGeorefGroup);
   mpPlugInList = new MutuallyExclusiveListWidget(mpAutoGeorefGroup);

   mpLatLonLayerCheck = new QCheckBox("Display latitude/longitude layer", mpAutoGeorefGroup);

   LabeledSection* pGeoreferenceSection = new LabeledSection(mpAutoGeorefGroup, "Georeference", this);

   // Layout
   QGridLayout* pAutoGeorefGrid = new QGridLayout(mpAutoGeorefGroup);
   pAutoGeorefGrid->setMargin(10);
   pAutoGeorefGrid->setSpacing(10);
   pAutoGeorefGrid->addWidget(mpImporterPlugInRadio, 0, 0, 1, 2);
   pAutoGeorefGrid->addWidget(pPlugInRadio, 1, 0, 1, 2);
   pAutoGeorefGrid->addWidget(mpPlugInList, 2, 1);
   pAutoGeorefGrid->addWidget(mpLatLonLayerCheck, 3, 0, 1, 2);
   pAutoGeorefGrid->setColumnMinimumWidth(0, 12);
   pAutoGeorefGrid->setRowStretch(2, 10);
   pAutoGeorefGrid->setColumnStretch(1, 10);

   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(10);
   pLayout->addWidget(pGeoreferenceSection, 10);

   // Initialization
   mpAutoGeorefGroup->setChecked(RasterElementImporterShell::getSettingAutoGeoreference());
   mpImporterPlugInRadio->setChecked(RasterElementImporterShell::getSettingImporterGeoreferencePlugIn());
   pPlugInRadio->setChecked(!RasterElementImporterShell::getSettingImporterGeoreferencePlugIn());
   mpPlugInList->setDisabled(RasterElementImporterShell::getSettingImporterGeoreferencePlugIn());
   mpLatLonLayerCheck->setChecked(RasterElementImporterShell::getSettingDisplayLatLonLayer());

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

   std::vector<std::string> geoPlugIns = RasterElementImporterShell::getSettingGeoreferencePlugIns();
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
   VERIFYNR(connect(pPlugInRadio, SIGNAL(toggled(bool)), mpPlugInList, SLOT(setEnabled(bool))));
}

OptionsRasterElementImporter::~OptionsRasterElementImporter()
{}

void OptionsRasterElementImporter::applyChanges()
{
   RasterElementImporterShell::setSettingAutoGeoreference(mpAutoGeorefGroup->isChecked());
   RasterElementImporterShell::setSettingImporterGeoreferencePlugIn(mpImporterPlugInRadio->isChecked());
   RasterElementImporterShell::setSettingDisplayLatLonLayer(mpLatLonLayerCheck->isChecked());

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

   RasterElementImporterShell::setSettingGeoreferencePlugIns(plugIns);
}
