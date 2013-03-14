/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <math.h>

#include "Animation.h"
#include "AnimationFrame.h"
#include "AnimationFrameSpinBox.h"
#include "AnimationFrameSubsetWidget.h"
#include "AppVerify.h"

#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QSlider>

#include <limits>

AnimationFrameSubsetWidget::AnimationFrameSubsetWidget(QWidget* pParent) :
   QWidget(pParent),
   mpStartLabel(NULL),
   mpStartSlider(NULL),
   mpStartSpin(NULL),
   mpStopLabel(NULL),
   mpStopSlider(NULL),
   mpStopSpin(NULL)
{
   mpStartLabel = new QLabel("Start Time:", this);
   mpStartSlider = new QSlider(Qt::Horizontal, this);
   mpStartSlider->setEnabled(false);
   VERIFYNR(connect(mpStartSlider, SIGNAL(valueChanged(int)), this, SLOT(startSliderMoved(int))));

   mpStartSpin = new AnimationFrameSpinBox(this);
   mpStartSpin->setEnabled(false);
   VERIFYNR(connect(mpStartSpin, SIGNAL(frameChanged(const AnimationFrame&)), this,
      SLOT(updateStartSlider(const AnimationFrame&))));

   mpStopLabel = new QLabel("Stop Time:", this);
   mpStopSlider = new QSlider(Qt::Horizontal, this);
   mpStopSlider->setEnabled(false);
   VERIFYNR(connect(mpStopSlider, SIGNAL(valueChanged(int)), this, SLOT(stopSliderMoved(int))));

   mpStopSpin = new AnimationFrameSpinBox(this);
   mpStopSpin->setEnabled(false);
   VERIFYNR(connect(mpStopSpin, SIGNAL(frameChanged(const AnimationFrame&)), this,
      SLOT(updateStopSlider(const AnimationFrame&))));

   QGridLayout* pSubsetLayout = new QGridLayout(this);
   pSubsetLayout->setMargin(0);
   pSubsetLayout->setSpacing(10);
   pSubsetLayout->addWidget(mpStartLabel, 0, 0);
   pSubsetLayout->addWidget(mpStartSlider, 1, 0);
   pSubsetLayout->addWidget(mpStartSpin, 1, 1);
   pSubsetLayout->addWidget(mpStopLabel, 2, 0);
   pSubsetLayout->addWidget(mpStopSlider, 3, 0);
   pSubsetLayout->addWidget(mpStopSpin, 3, 1);
   pSubsetLayout->setColumnStretch(0, 10);
}

AnimationFrameSubsetWidget::~AnimationFrameSubsetWidget()
{}

void AnimationFrameSubsetWidget::setFrames(AnimationController* pController)
{
   mType = FrameType();

   if (pController == NULL)
   {
      return;
   }

   const std::vector<Animation*>& anim = pController->getAnimations();
   if (anim.empty())
   {
      return;
   }

   setFrameType(pController->getFrameType());

   mpStartSpin->setFrames(pController);
   mpStartSpin->setEnabled(true);
   mpStopSpin->setFrames(pController);
   mpStopSpin->setEnabled(true);

   const std::vector<AnimationFrame>& frames = mpStartSpin->getFrames();

   mpStartSlider->setRange(0, frames.size()-1);
   mpStartSlider->setEnabled(true);
   mpStopSlider->setRange(0, frames.size()-1);
   mpStopSlider->setEnabled(true);
}

void AnimationFrameSubsetWidget::setFrames(Animation* pAnimation)
{
   mType = FrameType();

   if (pAnimation == NULL)
   {
      return;
   }

   setFrameType(pAnimation->getFrameType());

   mpStartSpin->setFrames(pAnimation);
   mpStartSpin->setEnabled(true);
   mpStopSpin->setFrames(pAnimation);
   mpStopSpin->setEnabled(true);

   const std::vector<AnimationFrame>& frames = pAnimation->getFrames();

   mpStartSlider->setRange(0, frames.size()-1);
   mpStartSlider->setEnabled(true);
   mpStopSlider->setRange(0, frames.size()-1);
   mpStopSlider->setEnabled(true);
}

void AnimationFrameSubsetWidget::setFrames(const std::vector<AnimationFrame>& frames, FrameType type)
{
   setFrameType(type);

   mpStartSpin->setFrames(frames, type);
   mpStartSpin->setEnabled(true);
   mpStopSpin->setFrames(frames, type);
   mpStopSpin->setEnabled(true);

   mpStartSlider->setRange(0, frames.size()-1);
   mpStartSlider->setEnabled(true);
   mpStopSlider->setRange(0, frames.size()-1);
   mpStopSlider->setEnabled(true);
}

double AnimationFrameSubsetWidget::getStartFrame() const
{
   const AnimationFrame& frame = mpStartSpin->getCurrentFrame();
   double value = 0;
   if (mType == FRAME_ID)
   {
      value = frame.mFrameNumber;
   }
   else
   {
      value = frame.mTime;
   }
   return value;
}

void AnimationFrameSubsetWidget::setStartFrame(double start)
{
   const std::vector<AnimationFrame>& frames = mpStartSpin->getFrames();
   if (frames.empty())
   {
      return;
   }

   unsigned int index = 0;
   for (unsigned int i = 0; i < frames.size(); ++i)
   {
      if (mpStartSpin->getFrameType() == FRAME_ID)
      {
         if (frames[i].mFrameNumber == start)
         { 
            index = i;
            break;
         }
      }
      else 
      {
         if (frames[i].mTime == start || (i + 1) == frames.size())
         {
            index = i;
            break;
         }

         double newStart = start + std::numeric_limits<double>::epsilon();

         if (frames[i].mTime < newStart && frames[i+1].mTime > newStart)
         {
            index = i;
            break;
         }
      }
   }

   mpStartSlider->setValue(index);
   mpStartSpin->setCurrentFrame(frames[index]);
}

double AnimationFrameSubsetWidget::getStopFrame() const
{
   const AnimationFrame& frame = mpStopSpin->getCurrentFrame();
   double value = 0;
   if (mType == FRAME_ID)
   {
      value = frame.mFrameNumber;
   }
   else
   {
      value = frame.mTime;
   }
   return value;
}

void AnimationFrameSubsetWidget::setStopFrame(double stop)
{
   const std::vector<AnimationFrame>& frames = mpStopSpin->getFrames();
   if (frames.empty())
   {
      return;
   }

   unsigned int index = 0;
   for (unsigned int i = 0; i < frames.size(); ++i)
   {
      if (mpStopSpin->getFrameType() == FRAME_ID)
      {
         if (frames[i].mFrameNumber == stop)
         {
            index = i;
            break;
         }
      }
      else
      {
         if (frames[i].mTime == stop || (i + 1) == frames.size())
         {
            index = i;
            break;
         }

         double newStop = stop + std::numeric_limits<double>::epsilon();

         if ((frames[i].mTime < newStop) && (frames[i+1].mTime > newStop))
         {
            index = i;
            break;
         }
      }
   }

   mpStopSlider->setValue(index);
   mpStopSpin->setCurrentFrame(frames[index]);
}

void AnimationFrameSubsetWidget::startSliderMoved(int value)
{
   const std::vector<AnimationFrame>& frames = mpStartSpin->getFrames();
   if (value < 0 || value >= static_cast<int>(frames.size()))
   {
      return;
   }

   mpStartSpin->setCurrentFrame(frames[value]);

   if (value > mpStopSlider->value())
   {
      mpStopSlider->setValue(value);
   }
}

void AnimationFrameSubsetWidget::stopSliderMoved(int value)
{
   const std::vector<AnimationFrame>& frames = mpStopSpin->getFrames();
   if (value < 0 || value >= static_cast<int>(frames.size()))
   {
      return;
   }

   mpStopSpin->setCurrentFrame(frames[value]);

   if (value < mpStartSlider->value())
   {
      mpStartSlider->setValue(value);
   }
}

void AnimationFrameSubsetWidget::updateStartSlider(const AnimationFrame& frame)
{
   const std::vector<AnimationFrame>& frames = mpStartSpin->getFrames();
   for(unsigned int i = 0; i < frames.size(); ++i)
   {
      double check1 = mType == FRAME_ID ? frames[i].mFrameNumber : frames[i].mTime;
      double check2 = mType == FRAME_ID ? frame.mFrameNumber : frame.mTime;
      if (check1 == check2)
      {
         mpStartSlider->setValue(i);
         break;
      }
   }
}

void AnimationFrameSubsetWidget::updateStopSlider(const AnimationFrame& frame)
{
   const std::vector<AnimationFrame>& frames = mpStartSpin->getFrames();
   for(unsigned int i = 0; i < frames.size(); ++i)
   {
      double check1 = mType == FRAME_ID ? frames[i].mFrameNumber : frames[i].mTime;
      double check2 = mType == FRAME_ID ? frame.mFrameNumber : frame.mTime;
      if (check1 == check2)
      {
         mpStopSlider->setValue(i);
         break;
      }
   }
}

void AnimationFrameSubsetWidget::setFrameType(FrameType type)
{
   mType = type;

   switch (mType)
   {
   case FRAME_ID:
      mpStartLabel->setText("Start Frame:");
      mpStartSlider->setToolTip("First frame to export, 1-based frame number");
      mpStartSpin->setToolTip("First frame to export, 1-based frame number");
      mpStopLabel->setText("Stop Frame:");
      mpStopSlider->setToolTip("Last frame to export, 1-based frame number");
      mpStopSpin->setToolTip("Last frame to export, 1-based frame number");
      break;
   case FRAME_TIME:           // Fall through
   case FRAME_ELAPSED_TIME:
      mpStartLabel->setText("Start Time:");
      mpStartSlider->setToolTip("Time of first frame to export");
      mpStartSpin->setToolTip("Time of first frame to export");
      mpStopLabel->setText("Stop Time:");
      mpStopSlider->setToolTip("Time of last frame to export");
      mpStopSpin->setToolTip("Time of last frame to export");
      break;
   default:
      break;
   }
}

FrameType AnimationFrameSubsetWidget::getFrameType() const
{
   return mType;
}
