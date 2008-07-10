/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "OptionsOverviewWindow.h"

#include "ColorType.h"
#include "CustomColorButton.h"
#include "LabeledSection.h"
#include "SpatialDataWindow.h"

#include <QtGui/QColor>
#include <QtGui/QLabel>
#include <QtGui/QSpinBox>
#include <QtGui/QVBoxLayout>

#include <string>

using namespace std;

OptionsOverviewWindow::OptionsOverviewWindow() :
   QWidget(NULL)
{
   // Trail Properties
   QLabel* pTrailColorLabel = new QLabel("Color:", this);
   mpTrailColor = new CustomColorButton(this);
   mpTrailColor->usePopupGrid(true);

   QLabel* pTrailOpacityLabel = new QLabel("Opacity:", this);
   mpTrailOpacity = new QSpinBox(this);
   mpTrailOpacity->setSuffix("%");
   mpTrailOpacity->setMinimum(0);
   mpTrailOpacity->setMaximum(100);
   mpTrailOpacity->setSingleStep(1);

   QLabel* pTrailThresholdLabel = new QLabel("Zoom Threshold:", this);
   mpTrailThreshold = new QSpinBox(this);
   mpTrailThreshold->setSuffix("%");
   mpTrailThreshold->setMinimum(10);
   mpTrailThreshold->setMaximum(1000);
   mpTrailThreshold->setSingleStep(10);

   QWidget* pTrailWidget = new QWidget(this);
   QGridLayout* pTrailLayout = new QGridLayout(pTrailWidget);
   pTrailLayout->setMargin(0);
   pTrailLayout->setSpacing(5);
   pTrailLayout->addWidget(pTrailColorLabel, 0, 0);
   pTrailLayout->addWidget(mpTrailColor, 0, 1, Qt::AlignLeft);
   pTrailLayout->addWidget(pTrailOpacityLabel, 1, 0);
   pTrailLayout->addWidget(mpTrailOpacity, 1, 1, Qt::AlignLeft);
   pTrailLayout->addWidget(pTrailThresholdLabel, 2, 0);
   pTrailLayout->addWidget(mpTrailThreshold, 2, 1, Qt::AlignLeft);
   pTrailLayout->setColumnStretch(1, 10);
   LabeledSection* pTrailSection = new LabeledSection(pTrailWidget, "Default Trail Properties", this);
   
   // Dialog layout
   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(10);
   pLayout->addWidget(pTrailSection);
   pLayout->addStretch(10);
   
   // Initialize From Settings
   ColorType trailColor = SpatialDataWindow::getSettingOverviewTrailColor();
   int opacity = int(trailColor.mAlpha / 2.550f + 0.5f); 
   trailColor.mAlpha = 255;
   mpTrailColor->setColor(trailColor);

   mpTrailOpacity->setValue(opacity);
   mpTrailThreshold->setValue(SpatialDataWindow::getSettingOverviewTrailThreshold());
}
   
void OptionsOverviewWindow::applyChanges()
{  
   ColorType trailColor = mpTrailColor->getColorType();
   int alpha = int(mpTrailOpacity->value() * 2.55f + 0.5f);
   if (alpha < 0)
   {
      alpha = 0;
   }
   else if (alpha > 255)
   {
      alpha = 255;
   }
   trailColor.mAlpha = alpha;
   SpatialDataWindow::setSettingOverviewTrailColor(trailColor);
   SpatialDataWindow::setSettingOverviewTrailThreshold(mpTrailThreshold->value());
}

OptionsOverviewWindow::~OptionsOverviewWindow()
{
}
