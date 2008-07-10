/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "OptionsGeneral.h"

#include "AnimationToolBar.h"
#include "ConfigurationSettings.h"
#include "LabeledSection.h"
#include "UtilityServices.h"

#include <QtGui/QCheckBox>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
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

   // Animation
   mpFrameSpeedList = new QListWidget(this);
   QLabel* pFrameSpeedLabel = new QLabel("Default Frame Speeds:", this);
   mFrameSpeeds = AnimationToolBar::getSettingFrameSpeeds();
   for (vector<double>::iterator iter = mFrameSpeeds.begin(); iter < mFrameSpeeds.end(); ++iter)
   {
      mpFrameSpeedList->addItem(QString::number(*iter));
   }

   QWidget* pAnimationLayoutWidget = new QWidget();
   QPushButton *pAddButton = new QPushButton("Add", pAnimationLayoutWidget);
   QPushButton *pRemoveButton = new QPushButton("Remove", pAnimationLayoutWidget);

   QGridLayout *pAnimationLayout = new QGridLayout(pAnimationLayoutWidget);
   pAnimationLayout->setMargin(0);
   pAnimationLayout->setSpacing(5);
   pAnimationLayout->addWidget(pFrameSpeedLabel, 0, 0);
   pAnimationLayout->addWidget(mpFrameSpeedList, 1, 0, 2, 1);
   pAnimationLayout->addWidget(pAddButton, 1, 2);
   pAnimationLayout->addWidget(pRemoveButton, 2, 2, Qt::AlignTop);
   pAnimationLayout->setColumnStretch(0, 20);
   pAnimationLayout->setRowStretch(2, 20);
   LabeledSection* pAnimationSection = new LabeledSection(pAnimationLayoutWidget, "Animation", this);

   VERIFYNR(connect(pAddButton, SIGNAL(clicked()), this, SLOT(addFrameSpeed())));
   VERIFYNR(connect(pRemoveButton, SIGNAL(clicked()), this, SLOT(removeFrameSpeed())));
   VERIFYNR(connect(mpFrameSpeedList, SIGNAL(itemChanged(QListWidgetItem *)), this, 
            SLOT(editFrameSpeedFinished(QListWidgetItem *))));

   // Dialog layout
   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(10);
   pLayout->addWidget(pUndoSection);
   pLayout->addWidget(pThreadingSection);
   pLayout->addWidget(pProgressSection);
   pLayout->addWidget(pAnimationSection, 1000);
   pLayout->addStretch(1);

   mpBufferSpin->setValue(static_cast<int>(ConfigurationSettings::getSettingUndoBufferSize()));
   mpThreadSpin->setValue(static_cast<int>(ConfigurationSettings::getSettingThreadCount()));
   mpProgressClose->setChecked(Progress::getSettingAutoClose());
}

void OptionsGeneral::applyChanges()
{
   ConfigurationSettings::setSettingUndoBufferSize(static_cast<unsigned int>(mpBufferSpin->value()));
   ConfigurationSettings::setSettingThreadCount(static_cast<unsigned int>(mpThreadSpin->value()));
   AnimationToolBar::setSettingFrameSpeeds(mFrameSpeeds);
   Progress::setSettingAutoClose(mpProgressClose->isChecked());
}

void OptionsGeneral::addFrameSpeed()
{
   mpFrameSpeedList->addItem(QString());
   mpFrameSpeedList->setCurrentRow(mpFrameSpeedList->count() - 1);
   mpFrameSpeedList->openPersistentEditor(mpFrameSpeedList->currentItem());
   mpFrameSpeedList->editItem(mpFrameSpeedList->currentItem());
}

void OptionsGeneral::editFrameSpeedFinished(QListWidgetItem *pCurrentItem)
{

   if (pCurrentItem == NULL)
   {
      return;
   }
   vector<double>::iterator iter; 

   iter = std::find(mFrameSpeeds.begin(), mFrameSpeeds.end(), pCurrentItem->text().toDouble());
   if (iter == mFrameSpeeds.end() && pCurrentItem->text().toDouble() > 0.0)  
   {
      mFrameSpeeds.push_back(pCurrentItem->text().toDouble());

      sort(mFrameSpeeds.begin(), mFrameSpeeds.end());
      mpFrameSpeedList->clear();
      for (iter = mFrameSpeeds.begin(); iter < mFrameSpeeds.end(); ++iter)
      {
         mpFrameSpeedList->addItem(QString::number(*iter));
      }
   }
   else
   {
      mpFrameSpeedList->takeItem(mpFrameSpeedList->row(pCurrentItem));
   }
   mpFrameSpeedList->closePersistentEditor(pCurrentItem);
}

void OptionsGeneral::removeFrameSpeed()
{
   if (mpFrameSpeedList->count() > 0)
   {
      if(mpFrameSpeedList->currentItem()->text().toDouble() > 0.0)  
      {
         mFrameSpeeds.erase(std::find(mFrameSpeeds.begin(), mFrameSpeeds.end(), 
                            mpFrameSpeedList->currentItem()->text().toDouble()));
         mpFrameSpeedList->takeItem(mpFrameSpeedList->row(mpFrameSpeedList->currentItem()));
      }
   }
}

OptionsGeneral::~OptionsGeneral()
{
}
