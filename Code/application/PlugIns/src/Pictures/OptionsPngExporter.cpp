/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "LabeledSection.h"
#include "OptionsPngExporter.h"
#include "OptionQWidgetWrapper.h"
#include "PlugInRegistration.h"
#include "ResolutionWidget.h"

REGISTER_PLUGIN(OpticksPictures, OptionsPngExporter, OptionQWidgetWrapper<OptionsPngExporter>());

OptionsPngExporter::OptionsPngExporter() :
   LabeledSectionGroup(NULL)
{
   // Resolution section
   mpResolutionWidget = new ResolutionWidget(this);
   LabeledSection* pResolutionSection = new LabeledSection(mpResolutionWidget, "Image Size", this);
   mpResolutionWidget->setAspectRatioLock(OptionsPngExporter::getSettingAspectRatioLock());
   mpResolutionWidget->setResolution(OptionsPngExporter::getSettingOutputWidth(),
      OptionsPngExporter::getSettingOutputHeight());
   mpResolutionWidget->setUseViewResolution(OptionsPngExporter::getSettingUseViewResolution());

   // Initialization
   addSection(pResolutionSection);
   addStretch(10);
   setSizeHint(350, 150);

}

void OptionsPngExporter::applyChanges()
{
   unsigned int outputWidth;
   unsigned int outputHeight;

   OptionsPngExporter::setSettingUseViewResolution(mpResolutionWidget->getUseViewResolution());
   mpResolutionWidget->getResolution(outputWidth, outputHeight);
   OptionsPngExporter::setSettingAspectRatioLock(mpResolutionWidget->getAspectRatioLock());
   OptionsPngExporter::setSettingOutputWidth(outputWidth);
   OptionsPngExporter::setSettingOutputHeight(outputHeight);
}

OptionsPngExporter::~OptionsPngExporter()
{
}
