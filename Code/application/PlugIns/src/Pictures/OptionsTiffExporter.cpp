/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QCheckBox>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QRadioButton>
#include <QtGui/QSpinBox>
#include <QtGui/QVBoxLayout>

#include "LabeledSection.h"
#include "OptionQWidgetWrapper.h"
#include "OptionsTiffExporter.h"
#include "PlugInRegistration.h"
#include "ResolutionWidget.h"
#include "StringUtilities.h"
#include "StringUtilitiesMacros.h"

#include <limits>

REGISTER_PLUGIN(OpticksPictures, OptionsTiffExporter, OptionQWidgetWrapper<OptionsTiffExporter>());

namespace StringUtilities
{
   BEGIN_ENUM_MAPPING_ALIAS(OptionsTiffExporter::TransformationMethod, TransformationMethod)
   ADD_ENUM_MAPPING(OptionsTiffExporter::TIE_POINT_PIXEL_SCALE, "Tie Point/Pixel Scale", "TiePointPixelScale")
   ADD_ENUM_MAPPING(OptionsTiffExporter::TRANSFORMATION_MATRIX, "Transformation Matrix", "TransformationMatrix")
   END_ENUM_MAPPING()
}

OptionsTiffExporter::OptionsTiffExporter() :
   LabeledSectionGroup(NULL)
{
   // Compression
   QWidget* pCompressionWidget = new QWidget(this);

   QLabel* pRowsPerStripLabel = new QLabel("Rows Per Strip: ", pCompressionWidget);
   mpRowsPerStrip = new QSpinBox(pCompressionWidget);
   mpRowsPerStrip->setRange(1, std::numeric_limits<int>::max());

   mpPackBits = new QCheckBox("Pack Bits", pCompressionWidget);

   QGridLayout* pCompressionLayout = new QGridLayout(pCompressionWidget);
   pCompressionLayout->setMargin(0);
   pCompressionLayout->setSpacing(5);
   pCompressionLayout->addWidget(pRowsPerStripLabel, 0, 0);
   pCompressionLayout->addWidget(mpRowsPerStrip, 0, 1);
   pCompressionLayout->addWidget(mpPackBits, 1, 1);
   pCompressionLayout->setColumnStretch(2, 10);

   LabeledSection* pCompressionSection = new LabeledSection(pCompressionWidget,
      "Compression Options (TIFF and GeoTIFF)", this);

   // Image resolution
   mpResolutionWidget = new ResolutionWidget(this);
   LabeledSection* pResolutionSection = new LabeledSection(mpResolutionWidget, "Image Size (TIFF only)", this);

   // Coordinate transformation
   QWidget* pTransformationWidget = new QWidget(this);

   QGroupBox* pTransformationGroup = new QGroupBox("Preferred Transformation Method", pTransformationWidget);
   mpTiePointRadio = new QRadioButton("Tie point/pixel scale", pTransformationGroup);
   mpMatrixRadio = new QRadioButton("Transformation matrix", pTransformationGroup);

   QVBoxLayout* pGroupLayout = new QVBoxLayout(pTransformationGroup);
   pGroupLayout->setMargin(10);
   pGroupLayout->setSpacing(5);
   pGroupLayout->addWidget(mpTiePointRadio);
   pGroupLayout->addWidget(mpMatrixRadio);

   QVBoxLayout* pTransformationLayout = new QVBoxLayout(pTransformationWidget);
   pTransformationLayout->setMargin(0);
   pTransformationLayout->setSpacing(5);
   pTransformationLayout->addWidget(pTransformationGroup, 0, Qt::AlignLeft);
   pTransformationLayout->addStretch();

   LabeledSection* pTransformationSection = new LabeledSection(pTransformationWidget,
      "Coordinate Transformation (GeoTIFF only)", this);

   // Initialization
   addSection(pCompressionSection);
   addSection(pResolutionSection);
   addSection(pTransformationSection);
   addStretch(10);
   setSizeHint(350, 250);

   // Initialize From Settings
   mpRowsPerStrip->setValue(static_cast<int>(OptionsTiffExporter::getSettingRowsPerStrip()));
   mpPackBits->setChecked(OptionsTiffExporter::getSettingPackBitsCompression());
   mpResolutionWidget->setAspectRatioLock(OptionsTiffExporter::getSettingAspectRatioLock());
   mpResolutionWidget->setResolution(OptionsTiffExporter::getSettingOutputWidth(),
      OptionsTiffExporter::getSettingOutputHeight());
   mpResolutionWidget->setUseViewResolution(OptionsTiffExporter::getSettingUseViewResolution());

   TransformationMethod transformationMethod =
      StringUtilities::fromXmlString<TransformationMethod>(OptionsTiffExporter::getSettingTransformationMethod());
   if (transformationMethod == TIE_POINT_PIXEL_SCALE)
   {
      mpTiePointRadio->setChecked(true);
   }
   else if (transformationMethod == TRANSFORMATION_MATRIX)
   {
      mpMatrixRadio->setChecked(true);
   }
}

OptionsTiffExporter::~OptionsTiffExporter()
{}

void OptionsTiffExporter::applyChanges()
{
   // Compression
   OptionsTiffExporter::setSettingRowsPerStrip(static_cast<unsigned int>(mpRowsPerStrip->value()));
   OptionsTiffExporter::setSettingPackBitsCompression(mpPackBits->isChecked());

   // Image resolution
   unsigned int outputWidth = 0;
   unsigned int outputHeight = 0;
   mpResolutionWidget->getResolution(outputWidth, outputHeight);

   OptionsTiffExporter::setSettingUseViewResolution(mpResolutionWidget->getUseViewResolution());
   OptionsTiffExporter::setSettingAspectRatioLock(mpResolutionWidget->getAspectRatioLock());
   OptionsTiffExporter::setSettingOutputWidth(outputWidth);
   OptionsTiffExporter::setSettingOutputHeight(outputHeight);

   // Coordinate transformation
   TransformationMethod transformationMethod;
   if (mpTiePointRadio->isChecked() == true)
   {
      transformationMethod = TIE_POINT_PIXEL_SCALE;
   }
   else if (mpMatrixRadio->isChecked() == true)
   {
      transformationMethod = TRANSFORMATION_MATRIX;
   }

   OptionsTiffExporter::setSettingTransformationMethod(StringUtilities::toXmlString<TransformationMethod>(
      transformationMethod));
}
