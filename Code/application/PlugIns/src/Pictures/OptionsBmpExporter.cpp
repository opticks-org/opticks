/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "LabeledSection.h"
#include "OptionsBmpExporter.h"
#include "OptionQWidgetWrapper.h"
#include "PlugInRegistration.h"
#include "ResolutionWidget.h"

REGISTER_PLUGIN(OpticksPictures, OptionsBmpExporter, OptionQWidgetWrapper<OptionsBmpExporter>());

OptionsBmpExporter::OptionsBmpExporter() :
   LabeledSectionGroup(NULL)
{
   // Resolution section
   mpResolutionWidget = new ResolutionWidget(this);
   LabeledSection* pResolutionSection = new LabeledSection(mpResolutionWidget, "Image Size", this);
   mpResolutionWidget->setAspectRatioLock(OptionsBmpExporter::getSettingAspectRatioLock());
   mpResolutionWidget->setResolution(OptionsBmpExporter::getSettingOutputWidth(),
      OptionsBmpExporter::getSettingOutputHeight());
   mpResolutionWidget->setUseViewResolution(OptionsBmpExporter::getSettingUseViewResolution());

   // Initialization
   addSection(pResolutionSection);
   addStretch(10);
   setSizeHint(350, 150);

}

OptionsBmpExporter::~OptionsBmpExporter()
{
}

void OptionsBmpExporter::applyChanges()
{
   unsigned int outputWidth;
   unsigned int outputHeight;

   mpResolutionWidget->getResolution(outputWidth, outputHeight);
   OptionsBmpExporter::setSettingUseViewResolution(mpResolutionWidget->getUseViewResolution());
   OptionsBmpExporter::setSettingAspectRatioLock(mpResolutionWidget->getAspectRatioLock());
   OptionsBmpExporter::setSettingOutputWidth(outputWidth);
   OptionsBmpExporter::setSettingOutputHeight(outputHeight);
}
