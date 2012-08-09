/*
 * The information in this file is
 * Copyright(c) 2012 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "InterpolationComboBox.h"
#include "LabeledSection.h"
#include "OptionQWidgetWrapper.h"
#include "PlugInRegistration.h"
#include "SpatialResamplerOptions.h"

#include <QtGui/QDoubleSpinBox>
#include <QtGui/QLabel>

REGISTER_PLUGIN(OpticksSpatialResampler, SpatialResamplerOptions, OptionQWidgetWrapper<SpatialResamplerOptions>());

SpatialResamplerOptions::SpatialResamplerOptions()
{
   // Add rendering configuration widget and it's associated layout
   QWidget* pResamplingConfigWidget = new QWidget(this);
   QGridLayout* pResamplingConfigLayout = new QGridLayout(pResamplingConfigWidget);
   pResamplingConfigLayout->setMargin(0);
   pResamplingConfigLayout->setSpacing(10);

   // Add X direction scale items
   QLabel* pXScaleFactorLabel = new QLabel("X Scale Factor:", this);
   mpXScaleFactorSpinBox = new QDoubleSpinBox(this);
   mpXScaleFactorSpinBox->setMinimum(0.000001);
   mpXScaleFactorSpinBox->setMaximum(999999.999999);
   mpXScaleFactorSpinBox->setSingleStep(0.1);
   mpXScaleFactorSpinBox->setValue(getSettingXScaleFactor());
   pResamplingConfigLayout->addWidget(pXScaleFactorLabel, 0, 0);
   pResamplingConfigLayout->addWidget(mpXScaleFactorSpinBox, 0, 1);

   // Add Y direction scale items
   QLabel* pYScaleFactorLabel = new QLabel("Y Scale Factor:", this);
   mpYScaleFactorSpinBox = new QDoubleSpinBox(this);
   mpYScaleFactorSpinBox->setMinimum(0.000001);
   mpYScaleFactorSpinBox->setMaximum(999999.999999);
   mpYScaleFactorSpinBox->setSingleStep(0.1);
   mpYScaleFactorSpinBox->setValue(getSettingYScaleFactor());
   pResamplingConfigLayout->addWidget(pYScaleFactorLabel, 1, 0);
   pResamplingConfigLayout->addWidget(mpYScaleFactorSpinBox, 1, 1);

   // Add interpolation method items
   QLabel* pInterpMethodLabel = new QLabel("Interpolation Method: ", this);
   mpInterpMethodComboBox = new InterpolationComboBox(this);
   mpInterpMethodComboBox->setInterpolation(getSettingInterpolationMethod());
   pResamplingConfigLayout->addWidget(pInterpMethodLabel, 2, 0);
   pResamplingConfigLayout->addWidget(mpInterpMethodComboBox, 2, 1);

   pResamplingConfigLayout->setColumnStretch(2, 10);

   // Add overall page labels
   LabeledSection* pResamplingConfiguration = new LabeledSection(pResamplingConfigWidget,
      "Rendering Configuration", this);
   addSection(pResamplingConfiguration);
   addStretch(1);
}

SpatialResamplerOptions::~SpatialResamplerOptions()
{ }

void SpatialResamplerOptions::applyChanges()
{
   setSettingXScaleFactor(mpXScaleFactorSpinBox->value());
   setSettingYScaleFactor(mpYScaleFactorSpinBox->value());
   setSettingInterpolationMethod(mpInterpMethodComboBox->getInterpolation());
}