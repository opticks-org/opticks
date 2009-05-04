/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QCheckBox>
#include <QtGui/QLabel>
#include <QtGui/QGridLayout>
#include <QtGui/QSpinBox>

#include "LabeledSection.h"
#include "OptionQWidgetWrapper.h"
#include "OptionsTiffExporter.h"
#include "PlugInRegistration.h"

#include <limits>
#include <string>
using namespace std;

REGISTER_PLUGIN(OpticksPictures, OptionsTiffExporter, OptionQWidgetWrapper<OptionsTiffExporter>());

OptionsTiffExporter::OptionsTiffExporter() :
   LabeledSectionGroup(NULL)
{
   // Pack Bits
   QWidget* pPackBitsLayoutWidget = new QWidget(this);

   mpPackBits = new QCheckBox("Pack Bits", pPackBitsLayoutWidget);

   QLabel* pRowsPerStripLabel = new QLabel("Rows Per Strip: ", pPackBitsLayoutWidget);
   mpRowsPerStrip = new QSpinBox(pPackBitsLayoutWidget);
   mpRowsPerStrip->setRange(1, numeric_limits<int>::max());

   mpSaveSettings = new QCheckBox("Save Settings", pPackBitsLayoutWidget);
   mpSaveSettings->hide();

   QGridLayout* pLayout = new QGridLayout(pPackBitsLayoutWidget);
   pLayout->setMargin(0);
   pLayout->setSpacing(5);
   pLayout->addWidget(pRowsPerStripLabel, 0, 0);
   pLayout->addWidget(mpRowsPerStrip, 0, 1);
   pLayout->addWidget(mpPackBits, 1, 1);
   pLayout->addWidget(mpSaveSettings, 2, 1);
   pLayout->setColumnStretch(2, 10);


   LabeledSection* pPackBitsSection = new LabeledSection(pPackBitsLayoutWidget, "Compression Options", this);

   // Initialization
   addSection(pPackBitsSection);
   addStretch(10);
   setSizeHint(300, 100);

   // Initialize From Settings
   mpPackBits->setChecked(OptionsTiffExporter::getSettingPackBitsCompression());
   mpRowsPerStrip->setValue(static_cast<int>(OptionsTiffExporter::getSettingRowsPerStrip()));
}

void OptionsTiffExporter::applyChanges()
{
   if (mpSaveSettings->isHidden() || mpSaveSettings->isChecked())
   {
      OptionsTiffExporter::setSettingPackBitsCompression(mpPackBits->isChecked());
      OptionsTiffExporter::setSettingRowsPerStrip(static_cast<unsigned int>(mpRowsPerStrip->value()));
   }
}

OptionsTiffExporter::~OptionsTiffExporter()
{
}

void OptionsTiffExporter::setPromptUserToSaveSettings(bool prompt)
{
   mpSaveSettings->setVisible(prompt);
}

bool OptionsTiffExporter::getPackBitsCompression()
{
   return mpPackBits->isChecked();
}
