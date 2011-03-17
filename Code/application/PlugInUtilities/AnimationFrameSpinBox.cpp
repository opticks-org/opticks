/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "Animation.h"
#include "AnimationController.h"
#include "AnimationFrame.h"
#include "AnimationFrameSpinBox.h"

#include <QtCore/QDateTime>
#include <QtGui/QLineEdit>

#include <algorithm>
#include <math.h>
using namespace std;

namespace
{
   bool AnimationFrameCompare(const AnimationFrame& frame1, const AnimationFrame& frame2)
   {
      return frame1.mFrameNumber == frame2.mFrameNumber;
   };

   bool AnimationTimeCompare(const AnimationFrame& frame1, const AnimationFrame& frame2)
   {
      return frame1.mTime == frame2.mTime;
   };
}

AnimationFrameSpinBox::AnimationFrameSpinBox(QWidget* pParent) :
   QAbstractSpinBox(pParent),
   mType(FRAME_ID),
   mIndex(0)
{
   setWrapping(true);
   setFixedWidth(150);

   QLineEdit* pLineEdit = lineEdit();
   if (pLineEdit != NULL)
   {
      pLineEdit->setReadOnly(true);
   }
}

AnimationFrameSpinBox::~AnimationFrameSpinBox()
{
   mFrames.clear();
}

void AnimationFrameSpinBox::setFrames(const std::vector<AnimationFrame>& frames, FrameType type)
{
   mFrames = frames;
   mType = type;
}

void AnimationFrameSpinBox::setFrames(AnimationController* pController)
{
   mFrames.clear();
   mType = pController->getFrameType();

   if (pController == NULL)
   {
      return;
   }

   const std::vector<Animation*>& anim = pController->getAnimations();
   for (std::vector<Animation*>::const_iterator iter = anim.begin(); iter != anim.end(); ++iter)
   {
      Animation* pAnimation = *iter;
      if (pAnimation != NULL)
      {
         const std::vector<AnimationFrame>& tempFrames = pAnimation->getFrames();

         for (std::vector<AnimationFrame>::const_iterator frameIter = tempFrames.begin();
            frameIter != tempFrames.end(); ++frameIter)
         {
            mFrames.push_back(*frameIter);
         }
      }
   }

   sort(mFrames.begin(), mFrames.end());

   if (mType == FRAME_ID)
   {
      mFrames.erase(unique(mFrames.begin(), mFrames.end(), AnimationFrameCompare), mFrames.end());
   }
   else
   {
      mFrames.erase(unique(mFrames.begin(), mFrames.end(), AnimationTimeCompare), mFrames.end());
   }
}

void AnimationFrameSpinBox::setFrames(Animation* pAnimation)
{
   mFrames.clear();
   mType = FrameType();

   if (pAnimation != NULL)
   {
      mType = pAnimation->getFrameType();
      mFrames = pAnimation->getFrames();
   }
}

const std::vector<AnimationFrame>& AnimationFrameSpinBox::getFrames() const
{
   return mFrames;
}

void AnimationFrameSpinBox::setCurrentFrame(const AnimationFrame& frame)
{
   for (unsigned int i = 0;i < mFrames.size(); ++i)
   {
      double check1 = mType == FRAME_ID ? mFrames[i].mFrameNumber : mFrames[i].mTime;
      double check2 = mType == FRAME_ID ? frame.mFrameNumber : frame.mTime;
      if (check1 == check2)
      {
         QString spinBoxText = convertToText(i);
         if (!spinBoxText.isEmpty())
         {
            mIndex = i;
            QLineEdit* pLineEdit = lineEdit();
            if (pLineEdit != NULL)
            {
               pLineEdit->setText(spinBoxText);
            }
            
            emit frameChanged(mFrames[mIndex]);
         }
         break;
      }
   }
}

const AnimationFrame& AnimationFrameSpinBox::getCurrentFrame() const
{
   static AnimationFrame frame;
   if (mFrames.empty())
   {
      return frame;
   }
   if (mIndex < 0 || mIndex > static_cast<int>(mFrames.size()))
   {
      return frame;
   }

   return mFrames[mIndex];
}

void AnimationFrameSpinBox::stepBy(int steps)
{
   int index = mIndex + steps;
   if (wrapping())
   {
      while (index >= static_cast<int>(mFrames.size()))
      {
         index -= mFrames.size();
      }
      while (index < 0)
      {
         index += mFrames.size();
      }
   }
   else
   {
      if (index < 0 || index > static_cast<int>(mFrames.size()))
      {
         return;
      }
   }

   setCurrentFrame(mFrames[index]);
}

QAbstractSpinBox::StepEnabled AnimationFrameSpinBox::stepEnabled() const
{
   if (isReadOnly())
   {
      return StepNone;
   }

   if (wrapping())
   {
      return StepEnabled(StepUpEnabled | StepDownEnabled);
   }

   StepEnabled ret = StepNone;

   if (mIndex < static_cast<int>(mFrames.size() - 1))
   {
      ret |= StepUpEnabled;
   }
   if (mIndex > 0)
   {
      ret |= StepDownEnabled;
   }

   return ret;
}

FrameType AnimationFrameSpinBox::getFrameType() const
{
   return mType;
}

QString AnimationFrameSpinBox::convertToText(int index)
{
   QString frameText;

   if (mFrames.empty())
   {
      return frameText;
   }

   if (index < 0 || index >= static_cast<int>(mFrames.size()))
   {
      return frameText;
   }

   AnimationFrame frame = mFrames[index];

   if (mType == FRAME_ID)
   {
      frameText = QString::number(frame.mFrameNumber+1);
   }
   else
   {
      double seconds = floor(frame.mTime);
      int milliseconds = static_cast<int>((frame.mTime - seconds) * 1000.0);

      QDateTime dateTime;
      dateTime.setTime_t(static_cast<unsigned int>(seconds));
      dateTime.setTime(dateTime.time().addMSecs(milliseconds));
      dateTime = dateTime.toUTC();

      frameText = dateTime.toString("yyyy/MM/dd hh:mm:ss.zzz");
   }

   return frameText;
}
