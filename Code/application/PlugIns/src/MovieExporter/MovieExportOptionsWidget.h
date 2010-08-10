/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MOVIEEXPORTOPTIONSWIDGET_H
#define MOVIEEXPORTOPTIONSWIDGET_H

#include "LabeledSectionGroup.h"

class AdvancedOptionsWidget;
class AnimationFrameSubsetWidget;
class BitrateWidget;
class FramerateWidget;
class ViewResolutionWidget;

class MovieExportOptionsWidget : public LabeledSectionGroup
{
public:
   MovieExportOptionsWidget();
   ~MovieExportOptionsWidget();

   ViewResolutionWidget* getResolutionWidget() const;
   BitrateWidget* getBitrateWidget() const;
   FramerateWidget* getFramerateWidget() const;
   AnimationFrameSubsetWidget* getSubsetWidget() const;
   AdvancedOptionsWidget* getAdvancedWidget() const;

private:
   ViewResolutionWidget* mpResolutionWidget;
   BitrateWidget* mpBitrateWidget;
   FramerateWidget* mpFramerateWidget;
   AnimationFrameSubsetWidget* mpSubsetWidget;
   AdvancedOptionsWidget* mpAdvancedWidget;
};

#endif
