/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "GeoTiffExportOptionsWidget.h"
#include "LabeledSection.h"
#include "StringUtilities.h"

#include <QtGui/QCheckBox>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QRadioButton>
#include <QtGui/QSpinBox>
#include <QtGui/QVBoxLayout>

#include <limits>

GeoTiffExportOptionsWidget::GeoTiffExportOptionsWidget() :
   LabeledSectionGroup(NULL)
{
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
      "Coordinate Transformation", this);

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

   LabeledSection* pCompressionSection = new LabeledSection(pCompressionWidget, "Compression Options", this);

   // Initialization
   addSection(pTransformationSection);
   addSection(pCompressionSection);
   addStretch(10);
   setSizeHint(350, 250);

   // Initialization from settings
   OptionsTiffExporter::TransformationMethod transformationMethod =
      StringUtilities::fromXmlString<OptionsTiffExporter::TransformationMethod>(
      OptionsTiffExporter::getSettingTransformationMethod());
   if (transformationMethod == OptionsTiffExporter::TIE_POINT_PIXEL_SCALE)
   {
      mpTiePointRadio->setChecked(true);
   }
   else if (transformationMethod == OptionsTiffExporter::TRANSFORMATION_MATRIX)
   {
      mpMatrixRadio->setChecked(true);
   }

   mpRowsPerStrip->setValue(static_cast<int>(OptionsTiffExporter::getSettingRowsPerStrip()));
   mpPackBits->setChecked(OptionsTiffExporter::getSettingPackBitsCompression());
}

GeoTiffExportOptionsWidget::~GeoTiffExportOptionsWidget()
{}

OptionsTiffExporter::TransformationMethod GeoTiffExportOptionsWidget::getTransformationMethod() const
{
   if (mpTiePointRadio->isChecked() == true)
   {
      return OptionsTiffExporter::TIE_POINT_PIXEL_SCALE;
   }
   else if (mpMatrixRadio->isChecked() == true)
   {
      return OptionsTiffExporter::TRANSFORMATION_MATRIX;
   }

   return OptionsTiffExporter::TransformationMethod();
}

int GeoTiffExportOptionsWidget::getRowsPerStrip() const
{
   return mpRowsPerStrip->value();
}

bool GeoTiffExportOptionsWidget::getPackBitsCompression() const
{
   return mpPackBits->isChecked();
}
