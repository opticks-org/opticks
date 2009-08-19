/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "LabeledSection.h"
#include "JpegExportOptionsWidget.h"
#include "OptionsJpegExporter.h"
#include "ImageResolutionWidget.h"

#include <QtGui/QCheckBox>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QSlider>

JpegExportOptionsWidget::JpegExportOptionsWidget() :
   LabeledSectionGroup(NULL)
{
   // Resolution section
   mpResolutionWidget = new ImageResolutionWidget();
   LabeledSection* pResolutionSection = new LabeledSection(mpResolutionWidget, "Image Size", this);
   mpResolutionWidget->setAspectRatioLock(OptionsJpegExporter::getSettingAspectRatioLock());
   mpResolutionWidget->setResolution(OptionsJpegExporter::getSettingOutputWidth(),
      OptionsJpegExporter::getSettingOutputHeight());

   // Compression Quality
   QWidget* pQualityLayoutWidget = new QWidget(this);

   QLabel* pQualityLabel = new QLabel("Quality:", pQualityLayoutWidget);
   mpQualitySlider = new QSlider(pQualityLayoutWidget);
   mpQualitySlider->setOrientation(Qt::Horizontal);
   mpQualitySlider->setRange(0, 100);
   mpQualitySlider->setSingleStep(10);

   QLabel* pCurrentValue = new QLabel(pQualityLayoutWidget);
   QLabel* pMinValue = new QLabel("0", pQualityLayoutWidget);
   QLabel* pMaxValue = new QLabel("100", pQualityLayoutWidget);

   // Layout 
   QHBoxLayout* pSliderLayout = new QHBoxLayout();
   pSliderLayout->setMargin(0);
   pSliderLayout->setSpacing(5);
   pSliderLayout->addWidget(pMinValue);
   pSliderLayout->addWidget(mpQualitySlider, 10);
   pSliderLayout->addWidget(pMaxValue);

   QHBoxLayout* pCurValLayout = new QHBoxLayout();
   pCurValLayout->setMargin(0);
   pCurValLayout->setSpacing(5);
   pCurValLayout->addWidget(pQualityLabel);
   pCurValLayout->addWidget(pCurrentValue);
   pCurValLayout->addStretch();

   QGridLayout* pQualityLayout = new QGridLayout(pQualityLayoutWidget);
   pQualityLayout->setMargin(0);
   pQualityLayout->setSpacing(5);
   pQualityLayout->addLayout(pCurValLayout, 0, 0, 1, 2);
   pQualityLayout->addLayout(pSliderLayout, 1, 1);
   pQualityLayout->setColumnStretch(1, 10);
   pQualityLayout->setRowStretch(3, 10);
   pQualityLayout->setColumnMinimumWidth(0, 10);

   LabeledSection* pQualitySection = new LabeledSection(pQualityLayoutWidget, "Compression Options", this);

   // Initialization
   addSection(pResolutionSection);
   addSection(pQualitySection);
   addStretch(10);
   setSizeHint(350, 250);

   // Initialize From Settings
   mpQualitySlider->setValue(OptionsJpegExporter::getSettingCompressionQuality());
   pCurrentValue->setNum(static_cast<int>(OptionsJpegExporter::getSettingCompressionQuality()));

   // Connections
   connect(mpQualitySlider, SIGNAL(valueChanged(int)), pCurrentValue, SLOT(setNum(int)));
}

JpegExportOptionsWidget::~JpegExportOptionsWidget()
{
}

void JpegExportOptionsWidget::setResolution(unsigned int width, unsigned int height)
{
   mpResolutionWidget->setResolution(width, height);
}

void JpegExportOptionsWidget::getResolution(unsigned int& width, unsigned int& height)
{
   mpResolutionWidget->getResolution(width, height);
}

unsigned int JpegExportOptionsWidget::getCompressionQuality()
{
   return mpQualitySlider->value();
}
