/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "OptionsGeneral.h"

#include "ConfigurationSettings.h"
#include "LabeledSection.h"
#include "UtilityServices.h"

#include <QtGui/QCheckBox>
#include <QtGui/QLabel>
#include <QtGui/QSpinBox>
#include <QtGui/QVBoxLayout>

using namespace std;

OptionsGeneral::OptionsGeneral() :
   QWidget(NULL)
{
   // Undo/redo
   QLabel* pBufferLabel = new QLabel("Buffer Size:", this);
   mpBufferSpin = new QSpinBox(this);
   mpBufferSpin->setRange(1, 3000);
   mpBufferSpin->setSingleStep(1);
   mpBufferSpin->setSuffix(" Actions");
   mpBufferSpin->setToolTip("Maximum number of undo actions per view");

   QWidget* pBufferLayoutWidget = new QWidget();
   QHBoxLayout* pUndoLayout = new QHBoxLayout(pBufferLayoutWidget);
   pUndoLayout->setMargin(0);
   pUndoLayout->setSpacing(5);
   pUndoLayout->addWidget(pBufferLabel);
   pUndoLayout->addWidget(mpBufferSpin);
   pUndoLayout->addStretch(10);
   LabeledSection* pUndoSection = new LabeledSection(pBufferLayoutWidget, "Undo/Redo", this);

   // Threading
   QLabel* pThreadCountLabel = new QLabel("Worker Threads:", this);
   unsigned int maxThreadCount = Service<UtilityServices>()->getNumProcessors();
   mpThreadSpin = new QSpinBox(this);
   mpThreadSpin->setMinimum(1);
   mpThreadSpin->setMaximum(maxThreadCount);
   mpThreadSpin->setSingleStep(1);
   QWidget* pThreadingLayoutWidget = new QWidget(this);
   QHBoxLayout* pThreadingLayout = new QHBoxLayout(pThreadingLayoutWidget);
   pThreadingLayout->setMargin(0);
   pThreadingLayout->setSpacing(5);
   pThreadingLayout->addWidget(pThreadCountLabel);
   pThreadingLayout->addWidget(mpThreadSpin);
   pThreadingLayout->addStretch(10);
   LabeledSection* pThreadingSection = new LabeledSection(pThreadingLayoutWidget, "Multi-threading", this);

   // Progress Dialog
   mpProgressClose = new QCheckBox("Automatically close on process completion", this);
   LabeledSection* pProgressSection = new LabeledSection(mpProgressClose, "Progress Dialog", this);

   // Dialog layout
   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(10);
   pLayout->addWidget(pUndoSection);
   pLayout->addWidget(pThreadingSection);
   pLayout->addWidget(pProgressSection);
   pLayout->addStretch(10);

   mpBufferSpin->setValue(static_cast<int>(ConfigurationSettings::getSettingUndoBufferSize()));
   mpThreadSpin->setValue(static_cast<int>(ConfigurationSettings::getSettingThreadCount()));
   mpProgressClose->setChecked(Progress::getSettingAutoClose());
}

void OptionsGeneral::applyChanges()
{
   ConfigurationSettings::setSettingUndoBufferSize(static_cast<unsigned int>(mpBufferSpin->value()));
   ConfigurationSettings::setSettingThreadCount(static_cast<unsigned int>(mpThreadSpin->value()));
   Progress::setSettingAutoClose(mpProgressClose->isChecked());
}

OptionsGeneral::~OptionsGeneral()
{
}
