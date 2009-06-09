/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"

#include "OptionsRasterLayer.h"

#if defined(CG_SUPPORTED)
#include "CgContext.h"
#endif 
#include "ComplexComponentComboBox.h"
#include "LabeledSection.h"
#include "RasterLayer.h"
#include "RegionUnitsComboBox.h"
#include "StretchTypeComboBox.h"

#include <QtGui/QCheckBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QVBoxLayout>

#include <limits>
#include <string>

using namespace std;

OptionsRasterLayer::OptionsRasterLayer() :
   QWidget(NULL)
{
   // Image Properties
   QLabel* pComplexComponentLabel = new QLabel("Complex Component:", this);
   mpComplexComponent = new ComplexComponentComboBox(this);

   mpUseGpuImage = new QCheckBox("Enable Dynamic Texture Generation", this);  // Do not have an option to select an
                                                                              // initial image filter until required
                                                                              // to do so

   mpFastContrast = new QCheckBox("Fast Contrast", this);
   
   mpBackgroundTileGen = new QCheckBox("Background Tile Generation", this);

   QWidget* pImagePropWidget = new QWidget(this);
   QGridLayout* pImagePropLayout = new QGridLayout(pImagePropWidget);
   pImagePropLayout->setMargin(0);
   pImagePropLayout->setSpacing(5);
   pImagePropLayout->addWidget(pComplexComponentLabel, 0, 0);
   pImagePropLayout->addWidget(mpComplexComponent, 0, 1, Qt::AlignLeft);
   pImagePropLayout->addWidget(mpUseGpuImage, 1, 0, 1, 2);
   pImagePropLayout->addWidget(mpFastContrast, 2, 0, 1, 2);
   pImagePropLayout->addWidget(mpBackgroundTileGen, 3, 0, 1, 2);
   pImagePropLayout->setColumnStretch(1, 10);
   LabeledSection* pImageSection = new LabeledSection(pImagePropWidget, "Default Image Properties", this);

   // RGB Stretch
   QLabel* pRgbStretchLabel = new QLabel("RGB Stretch Type:", this);
   mpRgbStretch = new StretchTypeComboBox(this);

   QLabel* pRedStretchLabel = new QLabel("Red Stretch Units:", this);
   mpRedStretch = new RegionUnitsComboBox(this);

   QLabel* pRedLowerLabel = new QLabel("Red Lower Value:", this);
   mpRedLowerValue = new QDoubleSpinBox(this);
   mpRedLowerValue->setRange(-1 * numeric_limits<double>::max(), numeric_limits<double>::max());

   QLabel* pRedUpperLabel = new QLabel("Red Upper Value:", this);
   mpRedUpperValue = new QDoubleSpinBox(this);
   mpRedUpperValue->setRange(-1 * numeric_limits<double>::max(), numeric_limits<double>::max());

   QLabel* pGreenStretchLabel = new QLabel("Green Stretch Units:", this);
   mpGreenStretch = new RegionUnitsComboBox(this);

   QLabel* pGreenLowerLabel = new QLabel("Green Lower Value:", this);
   mpGreenLowerValue = new QDoubleSpinBox(this);
   mpGreenLowerValue->setRange(-1 * numeric_limits<double>::max(), numeric_limits<double>::max());

   QLabel* pGreenUpperLabel = new QLabel("Green Upper Value:", this);
   mpGreenUpperValue = new QDoubleSpinBox(this);
   mpGreenUpperValue->setRange(-1 * numeric_limits<double>::max(), numeric_limits<double>::max());

   QLabel* pBlueStretchLabel = new QLabel("Blue Stretch Units:", this);
   mpBlueStretch = new RegionUnitsComboBox(this);

   QLabel* pBlueLowerLabel = new QLabel("Blue Lower Value:", this);
   mpBlueLowerValue = new QDoubleSpinBox(this);
   mpBlueLowerValue->setRange(-1 * numeric_limits<double>::max(), numeric_limits<double>::max());

   QLabel* pBlueUpperLabel = new QLabel("Blue Upper Value:", this);
   mpBlueUpperValue = new QDoubleSpinBox(this);
   mpBlueUpperValue->setRange(-1 * numeric_limits<double>::max(), numeric_limits<double>::max());

   QWidget* pRgbPropWidget = new QWidget(this);
   QGridLayout* pRgbPropLayout = new QGridLayout(pRgbPropWidget);
   pRgbPropLayout->setMargin(0);
   pRgbPropLayout->setSpacing(5);
   pRgbPropLayout->addWidget(pRgbStretchLabel, 0, 0);
   pRgbPropLayout->addWidget(mpRgbStretch, 0, 1, Qt::AlignLeft);
   pRgbPropLayout->addWidget(pRedStretchLabel, 1, 0);
   pRgbPropLayout->addWidget(mpRedStretch, 1, 1, Qt::AlignLeft);
   pRgbPropLayout->addWidget(pRedLowerLabel, 2, 0);
   pRgbPropLayout->addWidget(mpRedLowerValue, 2, 1, Qt::AlignLeft);
   pRgbPropLayout->addWidget(pRedUpperLabel, 3, 0);
   pRgbPropLayout->addWidget(mpRedUpperValue, 3, 1, Qt::AlignLeft);
   pRgbPropLayout->addWidget(pGreenStretchLabel, 4, 0);
   pRgbPropLayout->addWidget(mpGreenStretch, 4, 1, Qt::AlignLeft);
   pRgbPropLayout->addWidget(pGreenLowerLabel, 5, 0);
   pRgbPropLayout->addWidget(mpGreenLowerValue, 5, 1, Qt::AlignLeft);
   pRgbPropLayout->addWidget(pGreenUpperLabel, 6, 0);
   pRgbPropLayout->addWidget(mpGreenUpperValue, 6, 1, Qt::AlignLeft);
   pRgbPropLayout->addWidget(pBlueStretchLabel, 7, 0);
   pRgbPropLayout->addWidget(mpBlueStretch, 7, 1, Qt::AlignLeft);
   pRgbPropLayout->addWidget(pBlueLowerLabel, 8, 0);
   pRgbPropLayout->addWidget(mpBlueLowerValue, 8, 1, Qt::AlignLeft);
   pRgbPropLayout->addWidget(pBlueUpperLabel, 9, 0);
   pRgbPropLayout->addWidget(mpBlueUpperValue, 9, 1, Qt::AlignLeft);
   pRgbPropLayout->setColumnStretch(1, 10);
   LabeledSection* pRgbSection = new LabeledSection(pRgbPropWidget, "Default RGB Stretch", this);

   // Grayscale Stretch
   QLabel* pGrayscaleStretchLabel = new QLabel("Stretch Type:", this);
   mpGrayscaleStretch = new StretchTypeComboBox(this);

   QLabel* pGrayStretchLabel = new QLabel("Stretch Units:", this);
   mpGrayStretch = new RegionUnitsComboBox(this);

   QLabel* pGrayLowerLabel = new QLabel("Lower Value:", this);
   mpGrayLowerValue = new QDoubleSpinBox(this);
   mpGrayLowerValue->setRange(-1 * numeric_limits<double>::max(), numeric_limits<double>::max());

   QLabel* pGrayUpperLabel = new QLabel("Upper Value:", this);
   mpGrayUpperValue = new QDoubleSpinBox(this);
   mpGrayUpperValue->setRange(-1 * numeric_limits<double>::max(), numeric_limits<double>::max());

   QWidget* pGrayPropWidget = new QWidget(this);
   QGridLayout* pGrayPropLayout = new QGridLayout(pGrayPropWidget);
   pGrayPropLayout->setMargin(0);
   pGrayPropLayout->setSpacing(5);
   pGrayPropLayout->addWidget(pGrayscaleStretchLabel, 0, 0);
   pGrayPropLayout->addWidget(mpGrayscaleStretch, 0, 1, Qt::AlignLeft);
   pGrayPropLayout->addWidget(pGrayStretchLabel, 1, 0);
   pGrayPropLayout->addWidget(mpGrayStretch, 1, 1, Qt::AlignLeft);
   pGrayPropLayout->addWidget(pGrayLowerLabel, 2, 0);
   pGrayPropLayout->addWidget(mpGrayLowerValue, 2, 1, Qt::AlignLeft);
   pGrayPropLayout->addWidget(pGrayUpperLabel, 3, 0);
   pGrayPropLayout->addWidget(mpGrayUpperValue, 3, 1, Qt::AlignLeft);
   pGrayPropLayout->setColumnStretch(1, 10);
   LabeledSection* pGraySection = new LabeledSection(pGrayPropWidget, "Default Grayscale Stretch", this);
  
   // Dialog layout
   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(10);
   pLayout->addWidget(pImageSection);
   pLayout->addWidget(pRgbSection);
   pLayout->addWidget(pGraySection);
   pLayout->addStretch(10);
   
   bool systemSupportsGpuImage = false;
#if defined(CG_SUPPORTED)
   if (CgContext::instance() != NULL)
   {
      systemSupportsGpuImage = true;
   }
#endif

   mpUseGpuImage->setEnabled(systemSupportsGpuImage);

   // Initialize From Settings
   mpUseGpuImage->setChecked(RasterLayer::getSettingGpuImage());
   mpRedUpperValue->setValue(RasterLayer::getSettingRedUpperStretchValue());
   mpRedStretch->setCurrentValue(RasterLayer::getSettingRedStretchUnits());
   mpRedLowerValue->setValue(RasterLayer::getSettingRedLowerStretchValue());
   mpGreenUpperValue->setValue(RasterLayer::getSettingGreenUpperStretchValue());
   mpGreenStretch->setCurrentValue(RasterLayer::getSettingGreenStretchUnits());
   mpGreenLowerValue->setValue(RasterLayer::getSettingGreenLowerStretchValue());
   mpBlueUpperValue->setValue(RasterLayer::getSettingBlueUpperStretchValue());
   mpBlueStretch->setCurrentValue(RasterLayer::getSettingBlueStretchUnits());
   mpBlueLowerValue->setValue(RasterLayer::getSettingBlueLowerStretchValue());
   mpGrayUpperValue->setValue(RasterLayer::getSettingGrayUpperStretchValue());
   mpGrayStretch->setCurrentValue(RasterLayer::getSettingGrayscaleStretchUnits());
   mpGrayLowerValue->setValue(RasterLayer::getSettingGrayLowerStretchValue());
   mpFastContrast->setChecked(RasterLayer::getSettingFastContrastStretch());
   mpComplexComponent->setCurrentValue(RasterLayer::getSettingComplexComponent());
   mpBackgroundTileGen->setChecked(RasterLayer::getSettingBackgroundTileGeneration());
   mpRgbStretch->setCurrentValue(RasterLayer::getSettingRgbStretchType());
   mpGrayscaleStretch->setCurrentValue(RasterLayer::getSettingGrayscaleStretchType());
}
   
void OptionsRasterLayer::applyChanges()
{  
   RasterLayer::setSettingGpuImage(mpUseGpuImage->isChecked());
   RasterLayer::setSettingRedUpperStretchValue(mpRedUpperValue->value());
   RasterLayer::setSettingRedStretchUnits(mpRedStretch->getCurrentValue());
   RasterLayer::setSettingRedLowerStretchValue(mpRedLowerValue->value());
   RasterLayer::setSettingGreenUpperStretchValue(mpGreenUpperValue->value());
   RasterLayer::setSettingGreenStretchUnits(mpGreenStretch->getCurrentValue());
   RasterLayer::setSettingGreenLowerStretchValue(mpGreenLowerValue->value());
   RasterLayer::setSettingBlueUpperStretchValue(mpBlueUpperValue->value());
   RasterLayer::setSettingBlueStretchUnits(mpBlueStretch->getCurrentValue());
   RasterLayer::setSettingBlueLowerStretchValue(mpBlueLowerValue->value());
   RasterLayer::setSettingGrayUpperStretchValue(mpGrayUpperValue->value());
   RasterLayer::setSettingGrayscaleStretchUnits(mpGrayStretch->getCurrentValue());
   RasterLayer::setSettingGrayLowerStretchValue(mpGrayLowerValue->value());
   RasterLayer::setSettingFastContrastStretch(mpFastContrast->isChecked());
   RasterLayer::setSettingComplexComponent(mpComplexComponent->getCurrentValue());
   RasterLayer::setSettingBackgroundTileGeneration(mpBackgroundTileGen->isChecked());
   RasterLayer::setSettingRgbStretchType(mpRgbStretch->getCurrentValue());
   RasterLayer::setSettingGrayscaleStretchType(mpGrayscaleStretch->getCurrentValue());
}

OptionsRasterLayer::~OptionsRasterLayer()
{
}
