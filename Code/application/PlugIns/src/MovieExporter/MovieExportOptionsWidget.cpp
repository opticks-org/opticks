/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AdvancedOptionsWidget.h"
#include "AnimationFrameSubsetWidget.h"
#include "BitrateWidget.h"
#include "FramerateWidget.h"
#include "LabeledSection.h"
#include "MovieExportOptionsWidget.h"
#include "ViewResolutionWidget.h"

#include <QtGui/QLayout>

MovieExportOptionsWidget::MovieExportOptionsWidget() :
   LabeledSectionGroup(NULL)
{
   // Resolution section
   mpResolutionWidget = new ViewResolutionWidget(this);
   LabeledSection* pResolutionSection = new LabeledSection(mpResolutionWidget, "Output Resolution", this);

   // Playback section
   QWidget* pPlaybackWidget = new QWidget(this);
   mpBitrateWidget = new BitrateWidget(pPlaybackWidget);
   mpFramerateWidget = new FramerateWidget(pPlaybackWidget);

   QVBoxLayout* pPlaybackLayout = new QVBoxLayout(pPlaybackWidget);
   pPlaybackLayout->setMargin(0);
   pPlaybackLayout->setSpacing(10);
   pPlaybackLayout->addWidget(mpBitrateWidget);
   pPlaybackLayout->addWidget(mpFramerateWidget);

   LabeledSection* pPlaybackSection = new LabeledSection(pPlaybackWidget, "Playback", this);

   // Subset section
   mpSubsetWidget = new AnimationFrameSubsetWidget(this);
   LabeledSection* pSubsetSection = new LabeledSection(mpSubsetWidget, "Export Subset", this);

   // Advanced options section
   mpAdvancedWidget = new AdvancedOptionsWidget(this);
   LabeledSection* pAdvancedSection = new LabeledSection(mpAdvancedWidget, "Advanced Options", this);
   pAdvancedSection->collapse();

   // Initialization
   addSection(pResolutionSection);
   addSection(pPlaybackSection);
   addSection(pSubsetSection);
   addSection(pAdvancedSection);
   addStretch(10);
   setSizeHint(455, 450);
}

MovieExportOptionsWidget::~MovieExportOptionsWidget()
{}

ViewResolutionWidget* MovieExportOptionsWidget::getResolutionWidget() const
{
   return mpResolutionWidget;
}

BitrateWidget* MovieExportOptionsWidget::getBitrateWidget() const
{
   return mpBitrateWidget;
}

FramerateWidget* MovieExportOptionsWidget::getFramerateWidget() const
{
   return mpFramerateWidget;
}

AnimationFrameSubsetWidget* MovieExportOptionsWidget::getSubsetWidget() const
{
   return mpSubsetWidget;
}

AdvancedOptionsWidget* MovieExportOptionsWidget::getAdvancedWidget() const
{
   return mpAdvancedWidget;
}
