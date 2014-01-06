/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "LabeledSection.h"
#include "OptionQWidgetWrapper.h"
#include "OptionsGeographicFeatures.h"
#include "PlugInRegistration.h"

#include <QtGui/QCheckBox>
#include <QtGui/QVBoxLayout>

REGISTER_PLUGIN(OpticksGeographicFeatures, OptionsGeographicFeatures, OptionQWidgetWrapper<OptionsGeographicFeatures>);

OptionsGeographicFeatures::OptionsGeographicFeatures()
{
   // Connection section
   mpArcConnectionCheck = new QCheckBox("Use ArcGIS");

   QWidget* pConnectionWidget = new QWidget(this);
   QVBoxLayout* pConnectionLayout = new QVBoxLayout(pConnectionWidget);
   pConnectionLayout->setMargin(0);
   pConnectionLayout->setSpacing(10);
   pConnectionLayout->addWidget(mpArcConnectionCheck);
   pConnectionLayout->addStretch();

   LabeledSection* pConnectionSection = new LabeledSection(pConnectionWidget, "Connection", this);

   // Layout
   addSection(pConnectionSection);
   addStretch(10);
   setSizeHint(100, 100);

   // Initialization
   mpArcConnectionCheck->setChecked(OptionsGeographicFeatures::getSettingUseArcAsDefaultConnection());
}

OptionsGeographicFeatures::~OptionsGeographicFeatures()
{}

void OptionsGeographicFeatures::applyChanges()
{
   OptionsGeographicFeatures::setSettingUseArcAsDefaultConnection(mpArcConnectionCheck->isChecked());
}
