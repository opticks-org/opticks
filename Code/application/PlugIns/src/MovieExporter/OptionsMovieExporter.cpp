/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AdvancedOptionsWidget.h"
#include "BitrateWidget.h"
#include "LabeledSection.h"
#include "OptionQWidgetWrapper.h"
#include "OptionsMovieExporter.h"
#include "PlugInRegistration.h"
#include "ViewResolutionWidget.h"

REGISTER_PLUGIN(OpticksMovieExporter, OptionsMovieExporter, OptionQWidgetWrapper<OptionsMovieExporter>());

OptionsMovieExporter::OptionsMovieExporter() :
   LabeledSectionGroup(NULL)
{
   // Resolution section
   mpResolutionWidget = new ViewResolutionWidget(this);
   mpResolutionWidget->setResolution(QSize(OptionsMovieExporter::getSettingWidth(),
      OptionsMovieExporter::getSettingHeight()));

   LabeledSection* pResolutionSection = new LabeledSection(mpResolutionWidget, "Output Resolution", this);

   // Playback section
   mpBitrateWidget = new BitrateWidget(this);
   mpBitrateWidget->setBitrate(OptionsMovieExporter::getSettingBitrate());

   LabeledSection* pPlaybackSection = new LabeledSection(mpBitrateWidget, "Playback", this);

   // Advanced options section
   mpAdvancedWidget = new AdvancedOptionsWidget(this);
   mpAdvancedWidget->setMeMethod(OptionsMovieExporter::getSettingMeMethod());
   mpAdvancedWidget->setGopSize(OptionsMovieExporter::getSettingGopSize());
   mpAdvancedWidget->setQCompress(OptionsMovieExporter::getSettingQCompress());
   mpAdvancedWidget->setQBlur(OptionsMovieExporter::getSettingQBlur());
   mpAdvancedWidget->setQMinimum(OptionsMovieExporter::getSettingQMinimum());
   mpAdvancedWidget->setQMaximum(OptionsMovieExporter::getSettingQMaximum());
   mpAdvancedWidget->setQDiffMaximum(OptionsMovieExporter::getSettingQDiffMaximum());
   mpAdvancedWidget->setMaxBFrames(OptionsMovieExporter::getSettingMaxBFrames());
   mpAdvancedWidget->setBQuantFactor(OptionsMovieExporter::getSettingBQuantFactor());
   mpAdvancedWidget->setBQuantOffset(OptionsMovieExporter::getSettingBQuantOffset());
   mpAdvancedWidget->setDiaSize(OptionsMovieExporter::getSettingDiaSize());
   mpAdvancedWidget->setFlags(OptionsMovieExporter::getSettingFlags());

   LabeledSection* pAdvancedSection = new LabeledSection(mpAdvancedWidget, "Advanced Options", this);

   // Initialization
   addSection(pResolutionSection);
   addSection(pPlaybackSection);
   addSection(pAdvancedSection);
   addStretch(10);
   setSizeHint(450, 450);
}

OptionsMovieExporter::~OptionsMovieExporter()
{}

void OptionsMovieExporter::applyChanges()
{
   QSize resolution = mpResolutionWidget->getResolution();
   OptionsMovieExporter::setSettingWidth(resolution.width());
   OptionsMovieExporter::setSettingHeight(resolution.height());
   OptionsMovieExporter::setSettingBitrate(mpBitrateWidget->getBitrate());
   OptionsMovieExporter::setSettingMeMethod(mpAdvancedWidget->getMeMethod());
   OptionsMovieExporter::setSettingGopSize(mpAdvancedWidget->getGopSize());
   OptionsMovieExporter::setSettingQCompress(mpAdvancedWidget->getQCompress());
   OptionsMovieExporter::setSettingQBlur(mpAdvancedWidget->getQBlur());
   OptionsMovieExporter::setSettingQMinimum(mpAdvancedWidget->getQMinimum());
   OptionsMovieExporter::setSettingQMaximum(mpAdvancedWidget->getQMaximum());
   OptionsMovieExporter::setSettingQDiffMaximum(mpAdvancedWidget->getQDiffMaximum());
   OptionsMovieExporter::setSettingMaxBFrames(mpAdvancedWidget->getMaxBFrames());
   OptionsMovieExporter::setSettingBQuantFactor(mpAdvancedWidget->getBQuantFactor());
   OptionsMovieExporter::setSettingBQuantOffset(mpAdvancedWidget->getBQuantOffset());
   OptionsMovieExporter::setSettingDiaSize(mpAdvancedWidget->getDiaSize());
   OptionsMovieExporter::setSettingFlags(mpAdvancedWidget->getFlags());
}
