/*
 * The information in this file is
 * Copyright(c) 2012 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "LabeledSection.h"
#include "Layer.h"
#include "OptionsLayer.h"

#include <QtGui/QCheckBox>
#include <QtGui/QGridLayout>
#include <QtGui/QStyleOptionButton>
#include <QtGui/QVBoxLayout>

OptionsLayer::OptionsLayer() :
   QWidget(NULL)
{
   // Default Layer Properties labeled section
   QWidget* pLayerPropWidget = new QWidget(this);
   mpRenameElementCheck = new QCheckBox("Rename the element when renaming the layer", pLayerPropWidget);
   mpWarnRenameCheck = new QCheckBox("Warn when multiple layers are displaying the element to rename",
      pLayerPropWidget);

   QGridLayout* pLayerPropLayout = new QGridLayout(pLayerPropWidget);
   pLayerPropLayout->setMargin(0);
   pLayerPropLayout->setSpacing(5);
   pLayerPropLayout->addWidget(mpRenameElementCheck, 0, 0, 1, 2);
   pLayerPropLayout->addWidget(mpWarnRenameCheck, 1, 1);
   pLayerPropLayout->setColumnStretch(1, 10);

   QStyleOptionButton option;
   option.initFrom(mpRenameElementCheck);
   int checkWidth = style()->subElementRect(QStyle::SE_CheckBoxIndicator, &option).width();
   pLayerPropLayout->setColumnMinimumWidth(0, checkWidth);

   LabeledSection* pLayerPropSection = new LabeledSection(pLayerPropWidget, "Default Layer Properties", this);

   // Layout
   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(10);
   pLayout->addWidget(pLayerPropSection);
   pLayout->addStretch(10);

   // Initialization
   mpRenameElementCheck->setChecked(Layer::getSettingRenameElement());
   mpWarnRenameCheck->setChecked(Layer::getSettingWarnElementRename());
   mpWarnRenameCheck->setEnabled(Layer::getSettingRenameElement());

   // Connections
   VERIFYNR(connect(mpRenameElementCheck, SIGNAL(toggled(bool)), mpWarnRenameCheck, SLOT(setEnabled(bool))));
}

OptionsLayer::~OptionsLayer()
{}

void OptionsLayer::applyChanges()
{
   Layer::setSettingRenameElement(mpRenameElementCheck->isChecked());
   Layer::setSettingWarnElementRename(mpWarnRenameCheck->isChecked());
}
