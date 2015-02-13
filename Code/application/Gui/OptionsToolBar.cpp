/*
 * The information in this file is
 * Copyright(c) 2015 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "OptionsToolBar.h"

#include <QtGui/QSpinBox>
#include <QtGui/QLabel>
#include <QtGui/QVBoxLayout>

#include "ConfigurationSettings.h"
#include "LabeledSection.h"
#include "ToolBar.h"

OptionsToolBar::OptionsToolBar() :
   QWidget(NULL),
   mpOpacity(NULL)
{
   QWidget* pTrailWidget = new QWidget(this);
   QGridLayout* pTrailLayout = new QGridLayout(pTrailWidget);
   pTrailLayout->setMargin(0);
   pTrailLayout->setSpacing(5);

   QLabel* pOpacityLabel = new QLabel("Opacity:", this);
   mpOpacity = new QSpinBox(this);
   mpOpacity->setSuffix("%");
   mpOpacity->setMinimum(0);
   mpOpacity->setMaximum(100);
   mpOpacity->setSingleStep(1);

   pTrailLayout->addWidget(pOpacityLabel, 0, 0);
   pTrailLayout->addWidget(mpOpacity, 0, 1, Qt::AlignLeft);
   pTrailLayout->setColumnStretch(1, 10);
   LabeledSection* pTrailSection = new LabeledSection(pTrailWidget, "Default Properties", this);

   // Dialog layout
   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(10);
   pLayout->addWidget(pTrailSection);
   pLayout->addStretch(10);

   mpOpacity->setValue(ToolBar::getSettingOpacity());
}
   
void OptionsToolBar::applyChanges()
{  
   ToolBar::setSettingOpacity(mpOpacity->value());
}

OptionsToolBar::~OptionsToolBar()
{
}
