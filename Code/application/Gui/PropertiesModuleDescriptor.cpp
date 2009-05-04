/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */
#include <QtGui/QFormLayout>
#include <QtGui/QLayout>

#include "AppVersion.h"
#include "DateTime.h"
#include "LabeledSection.h"
#include "ModuleDescriptor.h"
#include "PropertiesModuleDescriptor.h"

using namespace std;

PropertiesModuleDescriptor::PropertiesModuleDescriptor() :
   LabeledSectionGroup(NULL)
{
   // Module
   mpModuleWidget = new QWidget(this);
   mpNameLabel = new QLabel(mpModuleWidget);
   mpVersionLabel = new QLabel(mpModuleWidget);
   mpDescriptionLabel = new QLabel(mpModuleWidget);
   mpPlugInsLabel = new QLabel(mpModuleWidget);
   mpValidatedLabel = new QLabel(mpModuleWidget);
   mpLoadedLabel = new QLabel(mpModuleWidget);
   LabeledSection* pModuleSection = new LabeledSection(mpModuleWidget, "Module", this);

   // File
   QWidget* pFileWidget = new QWidget(this);
   QLabel* pFilenameLabel = new QLabel("Filename:", pFileWidget);
   mpFilenameLabel = new QLabel(pFileWidget);
   QLabel* pFileSizeLabel = new QLabel("File Size:", pFileWidget);
   mpFileSizeLabel = new QLabel(pFileWidget);
   QLabel* pFileDateLabel = new QLabel("File Date:", pFileWidget);
   mpFileDateLabel = new QLabel(pFileWidget);
   LabeledSection* pFileSection = new LabeledSection(pFileWidget, "File", this);

   QGridLayout* pFileGrid = new QGridLayout(pFileWidget);
   pFileGrid->setMargin(0);
   pFileGrid->setSpacing(5);
   pFileGrid->addWidget(pFilenameLabel, 0, 0);
   pFileGrid->addWidget(mpFilenameLabel, 0, 2);
   pFileGrid->addWidget(pFileSizeLabel, 1, 0);
   pFileGrid->addWidget(mpFileSizeLabel, 1, 2);
   pFileGrid->addWidget(pFileDateLabel, 2, 0);
   pFileGrid->addWidget(mpFileDateLabel, 2, 2);
   pFileGrid->setColumnMinimumWidth(1, 15);
   pFileGrid->setColumnStretch(2, 10);

   // Initialization
   addSection(pModuleSection);
   addSection(pFileSection);
   addStretch(10);
   setSizeHint(600, 300);
}

PropertiesModuleDescriptor::~PropertiesModuleDescriptor()
{
}

bool PropertiesModuleDescriptor::initialize(SessionItem* pSessionItem)
{
   ModuleDescriptor* pModule = dynamic_cast<ModuleDescriptor*>(pSessionItem);
   if (pModule == NULL)
   {
      return false;
   }

   QFormLayout* pFormLayout = new QFormLayout(mpModuleWidget); //replace existing layout
   pFormLayout->setMargin(0);
   pFormLayout->setSpacing(5);
   //overridding QFormLayout's auto-detection of system style because
   //we are displaying QLabel's in the second column and not editable widgets
   //so we don't want the auto-detected style for QFormLayout.
   pFormLayout->setLabelAlignment(Qt::AlignLeft);
   pFormLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
   pFormLayout->addRow("Name:", mpNameLabel);

   // Module
   mpNameLabel->setText(QString::fromStdString(pModule->getName()));
   if (pModule->getModuleVersion() == 1)
   {
      pFormLayout->addRow("Version:", mpVersionLabel);
      mpVersionLabel->setText(QString::fromStdString(pModule->getVersion()));
      pFormLayout->addRow("Description:", mpDescriptionLabel);
      mpDescriptionLabel->setText(QString::fromStdString(pModule->getDescription()));
   }
   pFormLayout->addRow("Number of Plug-Ins:", mpPlugInsLabel);
   mpPlugInsLabel->setText(QString::number(pModule->getNumPlugIns()));
   if (pModule->getModuleVersion() == 1)
   {
      mpValidatedLabel->setText(pModule->isValidatedModule() ? "Yes" : "No");
      pFormLayout->addRow("Validated:", mpValidatedLabel);
   }
   pFormLayout->addRow("Loaded:", mpLoadedLabel);
   mpLoadedLabel->setText(pModule->isLoaded() ? "Yes" : "No");
   if (pModule->getModuleVersion() == 1)
   {
      QLabel* pLegacyWarning = new QLabel("This is a legacy module", mpModuleWidget);
      pLegacyWarning->setStyleSheet("color: blue");
      pFormLayout->addRow(pLegacyWarning);
   }

   // File
   unsigned int fileSize = static_cast<unsigned int>(pModule->getFileSize());
   QString strFileSize = QString::number(fileSize) + " bytes";

   QString strDate;

   const DateTime* pDate = pModule->getFileDate();
   if (pDate != NULL)
   {
      strDate = QString::fromStdString(pDate->getFormattedUtc("%d %b %Y"));
   }

   mpFilenameLabel->setText(QString::fromStdString(pModule->getFileName()));
   mpFileSizeLabel->setText(strFileSize);
   mpFileDateLabel->setText(strDate);

   return true;
}

bool PropertiesModuleDescriptor::applyChanges()
{
   // No modifications to the module descriptor are needed
   return true;
}

const string& PropertiesModuleDescriptor::getName()
{
   static string name = "Module Properties";
   return name;
}

const string& PropertiesModuleDescriptor::getPropertiesName()
{
   static string propertiesName = "Module";
   return propertiesName;
}

const string& PropertiesModuleDescriptor::getDescription()
{
   static string description = "General setting properties of a module";
   return description;
}

const string& PropertiesModuleDescriptor::getShortDescription()
{
   static string description;
   return description;
}

const string& PropertiesModuleDescriptor::getCreator()
{
   static string creator = "Ball Aerospace & Technologies Corp.";
   return creator;
}

const string& PropertiesModuleDescriptor::getCopyright()
{
   static string copyright = APP_COPYRIGHT_MSG;
   return copyright;
}

const string& PropertiesModuleDescriptor::getVersion()
{
   static string version = APP_VERSION_NUMBER;
   return version;
}

const string& PropertiesModuleDescriptor::getDescriptorId()
{
   static string id = "{32AD6E23-108A-46EC-AC70-0A651B158364}";
   return id;
}

bool PropertiesModuleDescriptor::isProduction()
{
   return APP_IS_PRODUCTION_RELEASE;
}
