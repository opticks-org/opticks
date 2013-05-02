/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "LabeledSection.h"
#include "ModisL1bImporter.h"
#include "ModisUtilities.h"
#include "OptionsModisL1bImporter.h"
#include "OptionQWidgetWrapper.h"
#include "PlugInRegistration.h"
#include "RasterConversionTypeComboBox.h"
#include "StringUtilities.h"

#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>

#include <string>

REGISTER_PLUGIN(OpticksModis, OptionsModisL1bImporter, OptionQWidgetWrapper<OptionsModisL1bImporter>());

OptionsModisL1bImporter::OptionsModisL1bImporter() :
   LabeledSectionGroup(NULL)
{
   // Raster values section
   QWidget* pRasterWidget = new QWidget(this);

   QLabel* pRasterConversionLabel = new QLabel("Raster Value Conversion:", pRasterWidget);
   mpRasterConversionCombo = new RasterConversionTypeComboBox(this);

   QHBoxLayout* pRasterLayout = new QHBoxLayout(pRasterWidget);
   pRasterLayout->setMargin(0);
   pRasterLayout->setSpacing(5);
   pRasterLayout->addWidget(pRasterConversionLabel);
   pRasterLayout->addWidget(mpRasterConversionCombo);
   pRasterLayout->addStretch();

   LabeledSection* pRasterValuesSection = new LabeledSection(pRasterWidget, "Raster Pixel Values", this);

   // Initialization
   addSection(pRasterValuesSection);
   addStretch(10);
   setSizeHint(350, 150);

   // Initialization from settings
   std::string rasterConversionText = ModisL1bImporter::getSettingRasterConversion();
   ModisUtilities::RasterConversionType rasterConversion =
      StringUtilities::fromXmlString<ModisUtilities::RasterConversionType>(rasterConversionText);
   mpRasterConversionCombo->setRasterConversion(rasterConversion);
}

OptionsModisL1bImporter::~OptionsModisL1bImporter()
{}

void OptionsModisL1bImporter::applyChanges()
{
   ModisUtilities::RasterConversionType rasterConversion = mpRasterConversionCombo->getRasterConversion();
   std::string rasterConversionText = StringUtilities::toXmlString(rasterConversion);
   ModisL1bImporter::setSettingRasterConversion(rasterConversionText);
}
