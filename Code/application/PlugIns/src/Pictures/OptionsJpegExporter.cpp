/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "OptionsJpegExporter.h"

#include "LabeledSection.h"
#include "SessionManager.h"

#include <QtGui/QCheckBox>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QSlider>
#include <QtGui/QVBoxLayout>

#include <string>

using namespace std;

OptionsJpegExporter::OptionsJpegExporter() :
   LabeledSectionGroup(NULL)
{
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

   //Save Settings
   mpSaveSettings = new QCheckBox("Save Settings");
   mpSaveSettings->hide();

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
   pQualityLayout->addWidget(mpSaveSettings, 2, 0, 1, 2);
   pQualityLayout->setColumnStretch(1, 10);
   pQualityLayout->setRowStretch(3, 10);
   pQualityLayout->setColumnMinimumWidth(0, 10);

   LabeledSection* pQualitySection = new LabeledSection(pQualityLayoutWidget, "Compression Options", this);

   // Initialization
   addSection(pQualitySection);
   addStretch(10);
   setSizeHint(350, 150);

   // Initialize From Settings
   mpQualitySlider->setValue(OptionsJpegExporter::getSettingCompressionQuality());
   pCurrentValue->setNum(static_cast<int>(OptionsJpegExporter::getSettingCompressionQuality()));

   // Connections
   connect(mpQualitySlider, SIGNAL(valueChanged(int)), pCurrentValue, SLOT(setNum(int)));
}
   
void OptionsJpegExporter::applyChanges()
{  
   if (mpSaveSettings->isHidden() || mpSaveSettings->isChecked())
   {
      OptionsJpegExporter::setSettingCompressionQuality(mpQualitySlider->value());
   }
}

OptionsJpegExporter::~OptionsJpegExporter()
{
}

void OptionsJpegExporter::setPromptUserToSaveSettings(bool prompt)
{
   mpSaveSettings->setVisible(prompt);
}

unsigned int OptionsJpegExporter::getCompressionQuality()
{
   return mpQualitySlider->value();
}

