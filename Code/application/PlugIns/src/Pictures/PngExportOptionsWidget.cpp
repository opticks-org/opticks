/*
 * The information in this file is
 * Copyright(c) 2014 Ball Aerospace & Technologies Corporation
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
#include "OptionsPngExporter.h"
#include "PngExportOptionsWidget.h"

#include <limits>
using namespace std;

PngExportOptionsWidget::PngExportOptionsWidget() :
   LabeledSectionGroup(NULL)
{
   // Resolution section
   mpResolutionWidget = new ImageResolutionWidget();
   LabeledSection* pResolutionSection = new LabeledSection(mpResolutionWidget, "Image Size", this);
   mpResolutionWidget->setAspectRatioLock(OptionsPngExporter::getSettingAspectRatioLock());
   mpResolutionWidget->setResolution(OptionsPngExporter::getSettingOutputWidth(),
      OptionsPngExporter::getSettingOutputHeight());

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
   addSection(mpBackgroundSection);
   addStretch(10);
   setSizeHint(350, 250);

   // Initialize From Settings
   mpBackgroundColorTransparent->setChecked(
      OptionsPngExporter::getSettingSetBackgroundColorTransparent());
}

PngExportOptionsWidget::~PngExportOptionsWidget()
{}

void PngExportOptionsWidget::setResolution(unsigned int width, unsigned int height)
{
   mpResolutionWidget->setResolution(width, height);
}

void PngExportOptionsWidget::getResolution(unsigned int& width, unsigned int& height) const
{
   mpResolutionWidget->getResolution(width, height);
}

unsigned int PngExportOptionsWidget::getRowsPerStrip() const
{
   return static_cast<unsigned int>(mpRowsPerStrip->value());
}

bool PngExportOptionsWidget::getBackgroundColorTransparent() const
{
   return mpBackgroundColorTransparent->isChecked();
}


void PngExportOptionsWidget::showBackgroundColorTransparentCheckbox(bool bShow) 
{
   if (mpBackgroundSection != NULL)
   {
      mpBackgroundSection->setVisible(bShow);
   }
   return;
}