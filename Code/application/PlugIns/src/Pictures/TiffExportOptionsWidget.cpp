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

#include "ImageResolutionWidget.h"
#include "LabeledSection.h"
#include "OptionsTiffExporter.h"
#include "TiffExportOptionsWidget.h"

#include <limits>
using namespace std;

TiffExportOptionsWidget::TiffExportOptionsWidget() :
   LabeledSectionGroup(NULL), mpBackgroundSection(NULL)
{
   // Resolution section
   mpResolutionWidget = new ImageResolutionWidget();
   LabeledSection* pResolutionSection = new LabeledSection(mpResolutionWidget, "Image Size", this);
   mpResolutionWidget->setAspectRatioLock(OptionsTiffExporter::getSettingAspectRatioLock());
   mpResolutionWidget->setResolution(OptionsTiffExporter::getSettingOutputWidth(),
      OptionsTiffExporter::getSettingOutputHeight());

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

   // Background Color Transparent
   QWidget* pBackgroundColorTransparentLayoutWidget = new QWidget(this);

   mpBackgroundColorTransparent = new QCheckBox("Background Color Transparent", 
      pBackgroundColorTransparentLayoutWidget);

   QVBoxLayout* pVLayout = new QVBoxLayout(pBackgroundColorTransparentLayoutWidget);
   pVLayout->setMargin(0);
   pVLayout->setSpacing(5);
   pVLayout->addWidget(mpBackgroundColorTransparent);

   mpBackgroundSection = new LabeledSection(
      pBackgroundColorTransparentLayoutWidget, "Background Options", this);

   // Initialization
   addSection(pResolutionSection);
   addSection(pPackBitsSection);
   addSection(mpBackgroundSection);
   addStretch(10);
   setSizeHint(350, 250);

   // Initialize From Settings
   mpPackBits->setChecked(OptionsTiffExporter::getSettingPackBitsCompression());
   mpRowsPerStrip->setValue(static_cast<int>(OptionsTiffExporter::getSettingRowsPerStrip()));
   mpBackgroundColorTransparent->setChecked(
      OptionsTiffExporter::getSettingSetBackgroundColorTransparent());
}

TiffExportOptionsWidget::~TiffExportOptionsWidget()
{}

void TiffExportOptionsWidget::setResolution(unsigned int width, unsigned int height)
{
   mpResolutionWidget->setResolution(width, height);
}

void TiffExportOptionsWidget::getResolution(unsigned int& width, unsigned int& height) const
{
   mpResolutionWidget->getResolution(width, height);
}

unsigned int TiffExportOptionsWidget::getRowsPerStrip() const
{
   return static_cast<unsigned int>(mpRowsPerStrip->value());
}

bool TiffExportOptionsWidget::getPackBitsCompression() const
{
   return mpPackBits->isChecked();
}

bool TiffExportOptionsWidget::getBackgroundColorTransparent() const
{
   return mpBackgroundColorTransparent->isChecked();
}

void TiffExportOptionsWidget::showBackgroundColorTransparentCheckbox(bool bShow) 
{
   if (mpBackgroundSection != NULL)
   {
      mpBackgroundSection->setVisible(bShow);
   }
   return;
}