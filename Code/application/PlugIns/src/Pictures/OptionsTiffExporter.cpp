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
#include "ResolutionWidget.h"

#include <limits>
#include <string>
using namespace std;

REGISTER_PLUGIN(OpticksPictures, OptionsTiffExporter, OptionQWidgetWrapper<OptionsTiffExporter>());

OptionsTiffExporter::OptionsTiffExporter() :
   LabeledSectionGroup(NULL)
{
   // Resolution section
   mpResolutionWidget = new ResolutionWidget(this);
   LabeledSection* pResolutionSection = new LabeledSection(mpResolutionWidget, "Image Size", this);
   mpResolutionWidget->setAspectRatioLock(OptionsTiffExporter::getSettingAspectRatioLock());
   mpResolutionWidget->setResolution(OptionsTiffExporter::getSettingOutputWidth(),
      OptionsTiffExporter::getSettingOutputHeight());
   mpResolutionWidget->setUseViewResolution(OptionsTiffExporter::getSettingUseViewResolution());

   // Pack Bits
   QWidget* pPackBitsLayoutWidget = new QWidget(this);

   mpPackBits = new QCheckBox("Pack Bits", pPackBitsLayoutWidget);

   QLabel* pRowsPerStripLabel = new QLabel("Rows Per Strip: ", pPackBitsLayoutWidget);
   mpRowsPerStrip = new QSpinBox(pPackBitsLayoutWidget);
   mpRowsPerStrip->setRange(1, numeric_limits<int>::max());

   QGridLayout* pLayout = new QGridLayout(pPackBitsLayoutWidget);
   pLayout->setMargin(0);
   pLayout->setSpacing(5);
   pLayout->addWidget(pRowsPerStripLabel, 0, 0);
   pLayout->addWidget(mpRowsPerStrip, 0, 1);
   pLayout->addWidget(mpPackBits, 1, 1);
   pLayout->setColumnStretch(2, 10);


   LabeledSection* pPackBitsSection = new LabeledSection(pPackBitsLayoutWidget, "Compression Options", this);

   // Initialization
   addSection(pResolutionSection);
   addSection(pPackBitsSection);
   addStretch(10);
   setSizeHint(350, 250);

   // Initialize From Settings
   mpPackBits->setChecked(OptionsTiffExporter::getSettingPackBitsCompression());
   mpRowsPerStrip->setValue(static_cast<int>(OptionsTiffExporter::getSettingRowsPerStrip()));
}

void OptionsTiffExporter::applyChanges()
{
   unsigned int outputWidth;
   unsigned int outputHeight;

   OptionsTiffExporter::setSettingPackBitsCompression(mpPackBits->isChecked());
   OptionsTiffExporter::setSettingRowsPerStrip(static_cast<unsigned int>(mpRowsPerStrip->value()));
   OptionsTiffExporter::setSettingUseViewResolution(mpResolutionWidget->getUseViewResolution());
   mpResolutionWidget->getResolution(outputWidth, outputHeight);
   OptionsTiffExporter::setSettingAspectRatioLock(mpResolutionWidget->getAspectRatioLock());
   OptionsTiffExporter::setSettingOutputWidth(outputWidth);
   OptionsTiffExporter::setSettingOutputHeight(outputHeight);
}

OptionsTiffExporter::~OptionsTiffExporter()
{
}

bool OptionsTiffExporter::getPackBitsCompression()
{
   return mpPackBits->isChecked();
}
