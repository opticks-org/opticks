/*
 * The information in this file is
 * Copyright(c) 2021 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "LabeledSection.h"
#include "OptionsImport.h"
#include "RasterElementImporterShell.h"

#include <QtWidgets/QCheckBox>
#include <QtWidgets/QVBoxLayout>

OptionsImport::OptionsImport() :
   QWidget(NULL)
{
   // Background Import
   mpBackgroundImport = new QCheckBox("Import in background", this);
   LabeledSection* pGeneralSection = new LabeledSection(mpBackgroundImport, "General Import Options", this);

   // Dialog layout
   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(10);
   pLayout->addWidget(pGeneralSection);
   pLayout->addStretch(10);

   // Initialization
   mpBackgroundImport->setChecked(RasterElementImporterShell::getSettingBackgroundImport());
}

void OptionsImport::applyChanges()
{
   // Settings
   RasterElementImporterShell::setSettingBackgroundImport(mpBackgroundImport->isChecked());
}

OptionsImport::~OptionsImport()
{}
