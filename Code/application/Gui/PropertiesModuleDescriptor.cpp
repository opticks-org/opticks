/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

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
   QWidget* pModuleWidget = new QWidget(this);
   QLabel* pNameLabel = new QLabel("Name:", pModuleWidget);
   mpNameLabel = new QLabel(pModuleWidget);
   QLabel* pVersionLabel = new QLabel("Version:", pModuleWidget);
   mpVersionLabel = new QLabel(pModuleWidget);
   QLabel* pDescriptionLabel = new QLabel("Description:", pModuleWidget);
   mpDescriptionLabel = new QLabel(pModuleWidget);
   QLabel* pPlugInsLabel = new QLabel("Number of Plug-Ins:", pModuleWidget);
   mpPlugInsLabel = new QLabel(pModuleWidget);
   QLabel* pValidatedLabel = new QLabel("Validated:", pModuleWidget);
   mpValidatedLabel = new QLabel(pModuleWidget);
   QLabel* pLoadedLabel = new QLabel("Loaded:", pModuleWidget);
   mpLoadedLabel = new QLabel(pModuleWidget);
   LabeledSection* pModuleSection = new LabeledSection(pModuleWidget, "Module", this);

   QGridLayout* pModuleGrid = new QGridLayout(pModuleWidget);
   pModuleGrid->setMargin(0);
   pModuleGrid->setSpacing(5);
   pModuleGrid->addWidget(pNameLabel, 0, 0);
   pModuleGrid->addWidget(mpNameLabel, 0, 2);
   pModuleGrid->addWidget(pVersionLabel, 1, 0);
   pModuleGrid->addWidget(mpVersionLabel, 1, 2);
   pModuleGrid->addWidget(pDescriptionLabel, 2, 0);
   pModuleGrid->addWidget(mpDescriptionLabel, 2, 2);
   pModuleGrid->addWidget(pPlugInsLabel, 3, 0);
   pModuleGrid->addWidget(mpPlugInsLabel, 3, 2);
   pModuleGrid->addWidget(pValidatedLabel, 4, 0);
   pModuleGrid->addWidget(mpValidatedLabel, 4, 2);
   pModuleGrid->addWidget(pLoadedLabel, 5, 0);
   pModuleGrid->addWidget(mpLoadedLabel, 5, 2);
   pModuleGrid->setColumnMinimumWidth(1, 15);
   pModuleGrid->setColumnStretch(2, 10);

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

   // Module
   mpNameLabel->setText(QString::fromStdString(pModule->getName()));
   mpVersionLabel->setText(QString::fromStdString(pModule->getVersion()));
   mpDescriptionLabel->setText(QString::fromStdString(pModule->getDescription()));
   mpPlugInsLabel->setText(QString::number(pModule->getNumPlugIns()));
   mpValidatedLabel->setText(pModule->isValidatedModule() ? "Yes" : "No");
   mpLoadedLabel->setText(pModule->isLoaded() ? "Yes" : "No");

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
