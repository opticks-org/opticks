/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <sys/timeb.h>

#include <QtCore/QString>
#include <QtGui/QApplication>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QIcon>
#include <QtGui/QLabel>
#include <QtGui/QMessageBox>

#include "AnimationAdapter.h"
#include "AnimationController.h"
#include "AnimationControllerImp.h"
#include "AnimationFrameSubsetWidget.h"
#include "AnimationServices.h"
#include "ApplicationServices.h"
#include "AppVerify.h"
#include "ContextMenuAction.h"
#include "ContextMenuActions.h"
#include "FileResource.h"
#include "SessionItemDeserializer.h"
#include "SessionItemSerializer.h"
#include "SessionManager.h"
#include "TypeConverter.h"
#include "XercesIncludes.h"
#include "xmlreader.h"
#include "xmlwriter.h"

#include <limits>
#include <list>
#include <fstream>
using boost::rational;
using boost::rational_cast;
using namespace std;
XERCES_CPP_NAMESPACE_USE

namespace
{
   double smallestDecrement(double d)
   {
      return d * (1 - numeric_limits<double>::epsilon());
   }
}

list<AnimationControllerImp*> AnimationControllerImp::mRunningControllers;
list<AnimationControllerImp*>::iterator AnimationControllerImp::mppActiveController = mRunningControllers.end();

AnimationControllerImp::AnimationControllerImp(FrameType frameType, const string& id) :
   SessionItemImp(id),
   mFrameType(frameType),
   mStartFrame(-1.0),
   mStopFrame(-1.0),
   mCurrentFrame(-1.0),
   mMaxCurrentTime(0.0),
   mEffectiveCurrentTime(0.0),
   mStartBumper(-1.0),
   mStopBumper(-1.0),
   mPlaybackStartFrame(mStartFrame),
   mPlaybackStopFrame(mStopFrame),
   mFrequency(60),
   mMinimumFrameRate(mFrequency, 1),
   mInterval(AnimationController::getSettingFrameSpeedSelection() / mFrequency),
   mState(STOP),
   mCycle(AnimationController::getSettingAnimationCycleSelection()),
   mStartTime(0.0),
   mCanDropFrames(true),
   mBumpersEnabled(false),
   mpPlayAction(NULL),
   mpPauseAction(NULL),
   mpStopAction(NULL),
   mpChangeDirectionAction(NULL),
   mpStepBackwardAction(NULL),
   mpStepForwardAction(NULL),
   mpBumpersEnabledAction(NULL),
   mpLeftBumperToCurrentAction(NULL),
   mpRightBumperToCurrentAction(NULL),
   mpAdjustBumpersAction(NULL),
   mpResetBumpersAction(NULL),
   mpSeparatorAction(NULL),
   mpSeparator2Action(NULL),
   mpStoreBumpersAction(NULL),
   mpRestoreBumpersAction(NULL)
{
   // Context menu actions
   mpPlayAction = new QAction("Play", this);
   mpPlayAction->setAutoRepeat(false);

   mpPauseAction = new QAction("Pause", this);
   mpPauseAction->setAutoRepeat(false);

   mpStopAction = new QAction("Stop", this);
   mpStopAction->setAutoRepeat(false);

   mpChangeDirectionAction = new QAction("Change Direction", this);
   mpChangeDirectionAction->setAutoRepeat(false);

   mpStepBackwardAction = new QAction("Step Backward", this);
   mpStepBackwardAction->setAutoRepeat(false);

   mpStepForwardAction = new QAction("Step Forward", this);
   mpStepForwardAction->setAutoRepeat(false);

   mpBumpersEnabledAction = new QAction("Bumpers Enabled", this);
   mpBumpersEnabledAction->setAutoRepeat(false);
   mpBumpersEnabledAction->setCheckable(true);
   mpBumpersEnabledAction->setChecked(false);

   mpResetBumpersAction = new QAction("Reset Bumpers", this);
   mpResetBumpersAction->setAutoRepeat(false);

   mpLeftBumperToCurrentAction = new QAction("Left Bumper to Current Frame", this);
   mpLeftBumperToCurrentAction->setAutoRepeat(false);

   mpRightBumperToCurrentAction = new QAction("Right Bumper to Current Frame", this);
   mpRightBumperToCurrentAction->setAutoRepeat(false);

   mpAdjustBumpersAction = new QAction("Adjust Bumpers...", this);
   mpAdjustBumpersAction->setAutoRepeat(false);

   mpStoreBumpersAction = new QAction("Store Bumpers", this);
   mpStoreBumpersAction->setAutoRepeat(false);

   mpRestoreBumpersAction = new QAction("Restore Bumpers", this);
   mpRestoreBumpersAction->setAutoRepeat(false);

   mpSeparatorAction = new QAction("", this);
   mpSeparatorAction->setSeparator(true);
   mpSeparator2Action = new QAction("", this);
   mpSeparator2Action->setSeparator(true);

   // Initialization
   mpPlayAction->setIcon(QIcon(":/icons/PlayForward"));
   mpPauseAction->setIcon(QIcon(":/icons/Pause"));
   mpStopAction->setIcon(QIcon(":/icons/Stop"));
   mpChangeDirectionAction->setIcon(QIcon(":/icons/ChangeDirection"));
   mpStepBackwardAction->setIcon(QIcon(":/icons/AdvanceBackward"));
   mpStepForwardAction->setIcon(QIcon(":/icons/AdvanceForward"));
   setIcon(QIcon(":/icons/Animation"));

   // Connections
   VERIFYNR(connect(mpPlayAction, SIGNAL(triggered()), this, SLOT(play())));
   VERIFYNR(connect(mpPauseAction, SIGNAL(triggered()), this, SLOT(pause())));
   VERIFYNR(connect(mpStopAction, SIGNAL(triggered()), this, SLOT(stop())));
   VERIFYNR(connect(mpChangeDirectionAction, SIGNAL(triggered()), this, SLOT(changeDirection())));
   VERIFYNR(connect(mpStepBackwardAction, SIGNAL(triggered()), this, SLOT(stepBackward())));
   VERIFYNR(connect(mpStepForwardAction, SIGNAL(triggered()), this, SLOT(stepForward())));
   VERIFYNR(connect(mpBumpersEnabledAction, SIGNAL(triggered(bool)), this, SLOT(changeBumpersEnabled(bool))));
   VERIFYNR(connect(mpLeftBumperToCurrentAction, SIGNAL(triggered()), this, SLOT(snapStartBumperToFrame())));
   VERIFYNR(connect(mpRightBumperToCurrentAction, SIGNAL(triggered()), this, SLOT(snapStopBumperToFrame())));
   VERIFYNR(connect(mpAdjustBumpersAction, SIGNAL(triggered()), this, SLOT(adjustBumpers())));
   VERIFYNR(connect(mpResetBumpersAction, SIGNAL(triggered()), this, SLOT(resetBumpers())));
   VERIFYNR(connect(mpStoreBumpersAction, SIGNAL(triggered()), this, SLOT(storeBumpers())));
   VERIFYNR(connect(mpRestoreBumpersAction, SIGNAL(triggered()), this, SLOT(restoreBumpers())));
}

AnimationControllerImp::~AnimationControllerImp()
{
   vector<Animation*> movies = getAnimations();
   for (unsigned int i = 0; i < movies.size(); ++i)
   {
      Animation* pAnimation = movies[i];
      if (pAnimation != NULL)
      {
         destroyAnimation(pAnimation);
      }
   }
}

const string& AnimationControllerImp::getObjectType() const
{
   static string type = "AnimationControllerImp";
   return type;
}

bool AnimationControllerImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "AnimationController"))
   {
      return true;
   }

   return false;
}

list<ContextMenuAction> AnimationControllerImp::getContextMenuActions() const
{
   list<ContextMenuAction> menuActions = SessionItemImp::getContextMenuActions();
   if (mAnimations.empty() == false)  // only add actions if there are animations
   {
      menuActions.push_front(ContextMenuAction(mpStepBackwardAction, APP_ANIMATIONCONTROLLER_STEP_BACKWARD_ACTION));
      menuActions.push_front(ContextMenuAction(mpStepForwardAction, APP_ANIMATIONCONTROLLER_STEP_FORWARD_ACTION));

      AnimationState animationState = getAnimationState();
      if (animationState == STOP)
      {
         menuActions.push_front(ContextMenuAction(mpPlayAction, APP_ANIMATIONCONTROLLER_PLAY_ACTION));
      }
      else if (animationState == PAUSE_FORWARD || animationState == PAUSE_BACKWARD)
      {
         menuActions.push_front(ContextMenuAction(mpStopAction, APP_ANIMATIONCONTROLLER_STOP_ACTION));
         menuActions.push_front(ContextMenuAction(mpPlayAction, APP_ANIMATIONCONTROLLER_PLAY_ACTION));
      }
      else
      {
         menuActions.push_front(ContextMenuAction(mpChangeDirectionAction, 
            APP_ANIMATIONCONTROLLER_CHANGE_DIRECTION_ACTION));
         menuActions.push_front(ContextMenuAction(mpStopAction, APP_ANIMATIONCONTROLLER_STOP_ACTION));
         menuActions.push_front(ContextMenuAction(mpPauseAction, APP_ANIMATIONCONTROLLER_PAUSE_ACTION));
      }

      menuActions.push_back(ContextMenuAction(mpSeparatorAction, APP_ANIMATIONCONTROLLER_BUMPER_SEPARATOR_ACTION));
      menuActions.push_back(ContextMenuAction(mpBumpersEnabledAction, APP_ANIMATIONCONTROLLER_ENABLE_BUMPERS_ACTION));
      menuActions.push_back(ContextMenuAction(mpLeftBumperToCurrentAction,
         APP_ANIMATIONCONTROLLER_SET_LEFT_BUMPER_ACTION));
      menuActions.push_back(ContextMenuAction(mpRightBumperToCurrentAction,
         APP_ANIMATIONCONTROLLER_SET_RIGHT_BUMPER_ACTION));
      menuActions.push_back(ContextMenuAction(mpAdjustBumpersAction, APP_ANIMATIONCONTROLLER_ADJUST_BUMPERS_ACTION));
      menuActions.push_back(ContextMenuAction(mpResetBumpersAction, APP_ANIMATIONCONTROLLER_RESET_BUMPERS_ACTION));
      menuActions.push_back(ContextMenuAction(mpSeparator2Action,
         APP_ANIMATIONCONTROLLER_STORE_BUMPERS_SEPARATOR_ACTION));
      menuActions.push_back(ContextMenuAction(mpStoreBumpersAction, APP_ANIMATIONCONTROLLER_STORE_BUMPERS_ACTION));
      Service<ConfigurationSettings> pSettings;
      string bumperPath("AnimationController/Bumpers/BumpersActive");
      DataVariant bumpersEnabled = pSettings->getSetting(bumperPath);
      mpRestoreBumpersAction->setEnabled(bumpersEnabled.isValid());
      menuActions.push_back(ContextMenuAction(mpRestoreBumpersAction, APP_ANIMATIONCONTROLLER_RESTORE_BUMPERS_ACTION));
   }

   return menuActions;
}

void AnimationControllerImp::movieDeleted(Subject &subject, const string &signal, const boost::any &v)
{
   Animation* pAnimation = dynamic_cast<Animation*> (&subject);
   if (pAnimation != NULL)
   {
      removeAnimation(pAnimation);
   }
}

void AnimationControllerImp::setName(const string& name)
{
   if (name != getName())
   {
      SessionItemImp::setName(name);
      emit renamed(QString::fromStdString(name));
      notify(SIGNAL_NAME(AnimationController, Renamed), boost::any(name));
   }
}

Animation* AnimationControllerImp::createAnimation(const QString& strName)
{
   if (strName.isEmpty() == true)
   {
      return NULL;
   }

   if (hasAnimation(strName) == true)
   {
      return NULL;
   }

   AnimationAdapter* pAnimation = new AnimationAdapter(mFrameType, SessionItemImp::generateUniqueId());
   if (pAnimation != NULL)
   {
      string movieName = strName.toStdString();
      pAnimation->setName(movieName);

      connect(pAnimation, SIGNAL(framesChanged(const std::vector<AnimationFrame>&)), this, SLOT(updateFrameData()));
      connect(pAnimation, SIGNAL(objectDetached()), this, SLOT(destroyAnimation()));
      mAnimations.push_back(pAnimation);
      pAnimation->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &AnimationControllerImp::movieDeleted));
      emit animationAdded(pAnimation);
      notify(SIGNAL_NAME(AnimationController, AnimationAdded), boost::any(static_cast<Animation*>(pAnimation)));
   }

   return pAnimation;
}

Animation* AnimationControllerImp::getAnimation(const QString& strName) const
{
   if (strName.isEmpty() == true)
   {
      return NULL;
   }

   vector<Animation*>::const_iterator iter = mAnimations.begin();
   while (iter != mAnimations.end())
   {
      Animation* pAnimation = *iter;
      if (pAnimation != NULL)
      {
         string movieName = strName.toStdString();
         string currentName = pAnimation->getName();

         if (currentName == movieName)
         {
            return pAnimation;
         }
      }

      ++iter;
   }

   return NULL;
}

bool AnimationControllerImp::hasAnimation(const QString& strName) const
{
   Animation* pAnimation = getAnimation(strName);
   return (pAnimation != NULL);
}

const vector<Animation*>& AnimationControllerImp::getAnimations() const
{
   return mAnimations;
}

unsigned int AnimationControllerImp::getNumAnimations() const
{
   return (mAnimations.size());
}

void AnimationControllerImp::destroyAnimation(Animation* pAnimation)
{
   if (pAnimation != NULL)
   {
      removeAnimation(pAnimation);
      delete (dynamic_cast<AnimationImp*> (pAnimation));
   }
}

FrameType AnimationControllerImp::getFrameType() const
{
   return mFrameType;
}

void AnimationControllerImp::setCurrentFrame(double frameValue)
{
   if (frameValue < mPlaybackStartFrame)
   {
      frameValue = mPlaybackStartFrame;
   }

   if (frameValue > mPlaybackStopFrame)
   {
      frameValue = mPlaybackStopFrame;
   }

   if (frameValue != mCurrentFrame)
   {
      mCurrentFrame = frameValue;
      emit frameChanged(mCurrentFrame);
      notify(SIGNAL_NAME(AnimationController, FrameChanged), boost::any(mCurrentFrame));

      // Set the current frame in each movie
      vector<Animation*>::const_iterator iter = mAnimations.begin();
      while (iter != mAnimations.end())
      {
         Animation* pAnimation = *iter;
         if (pAnimation != NULL)
         {
            pAnimation->setCurrentFrame(mCurrentFrame);
         }

         ++iter;
      }
   }
}

double AnimationControllerImp::getCurrentFrame() const
{
   return mCurrentFrame;
}

double AnimationControllerImp::getStartFrame() const
{
   return mStartFrame;
}

double AnimationControllerImp::getStopFrame() const
{
   return mStopFrame;
}

void AnimationControllerImp::setIntervalMultiplier(double multiplier)
{
   double dInterval = multiplier / mFrequency;
   if (dInterval != mInterval)
   {
      AnimationController::setSettingFrameSpeedSelection(multiplier); 
      mInterval = dInterval;
      emit intervalMultiplierChanged(multiplier);
      notify(SIGNAL_NAME(AnimationController, IntervalMultiplierChanged), boost::any(multiplier));
   }
}

double AnimationControllerImp::getIntervalMultiplier() const
{
   return (mInterval * mFrequency);
}

const int AnimationControllerImp::getFrequency() const
{
   return mFrequency;
}

rational<int> AnimationControllerImp::getMinimumFrameRate() const
{
   return mMinimumFrameRate;
}

void AnimationControllerImp::setMinimumFrameRate(rational<int> frameRate)
{
   mMinimumFrameRate = frameRate;
}

void AnimationControllerImp::setAnimationState(AnimationState state)
{
   if (state != mState)
   {
      mState = state;
      emit animationStateChanged(mState);
      notify(SIGNAL_NAME(AnimationController, AnimationStateChanged), boost::any(state));
   }
}

AnimationState AnimationControllerImp::getAnimationState() const
{
   return mState;
}

void AnimationControllerImp::setAnimationCycle(AnimationCycle cycle)
{
   if (cycle != mCycle)
   {
      mCycle = cycle;
      emit animationCycleChanged(mCycle);
      notify(SIGNAL_NAME(AnimationController, AnimationCycleChanged), boost::any(cycle));
   }
}

AnimationCycle AnimationControllerImp::getAnimationCycle() const
{
   return mCycle;
}

void AnimationControllerImp::moveToBeginning()
{
   setCurrentFrame(mPlaybackStartFrame);
}

void AnimationControllerImp::moveToEnd()
{
   setCurrentFrame(mPlaybackStopFrame);
}

void AnimationControllerImp::play()
{
   AnimationState previousState = getAnimationState();

   // Do not override a play backward state
   if (previousState != PLAY_BACKWARD)
   {
      if (previousState == PAUSE_BACKWARD)
      {
         setAnimationState(PLAY_BACKWARD);
      }
      else
      {
         setAnimationState(PLAY_FORWARD);
      }
   }
   if (find(mRunningControllers.begin(), mRunningControllers.end(), this) == mRunningControllers.end())
   {
      // Update the system time in milliseconds at timer start
      timeb timeStruct;
      ftime(&timeStruct);
      mStartTime = timeStruct.time*1000.0 + timeStruct.millitm;
      mEffectiveCurrentTime = mStartTime;

      // System clock resolution limits the total frame rate to 60 frames per second.
      // This is ameliorated by allowing the animation to continue to update in
      // 1 ms incremements up to just under the next system clock time.
      mMaxCurrentTime = mStartTime + 1000.0*smallestDecrement(1/60.0);
      mRunningControllers.push_back(this);
   }
   if (mRunningControllers.size() == 1)
   {
      runAnimations();
   }
}

void AnimationControllerImp::removeFromRunningControllers()
{
   list<AnimationControllerImp*>::iterator iter = find(mRunningControllers.begin(), mRunningControllers.end(), this);
   if (iter != mRunningControllers.end())
   {
      if (iter == mppActiveController)
      {
         ++mppActiveController;
      }
      mRunningControllers.erase(iter);
   }
}

void AnimationControllerImp::pause()
{
   AnimationState previousState = getAnimationState();

   if (previousState == PLAY_FORWARD || previousState == PLAY_BACKWARD)
   {
      removeFromRunningControllers();
      if (previousState == PLAY_FORWARD)
      {
         setAnimationState(PAUSE_FORWARD);
      }
      else
      {
         setAnimationState(PAUSE_BACKWARD);
      }
   }
}

void AnimationControllerImp::changeDirection()
{
   AnimationState previousState = getAnimationState();

   if (previousState == PLAY_FORWARD)
   {
      setAnimationState(PLAY_BACKWARD);
   } 
   else if (previousState == PAUSE_FORWARD)
   {
      setAnimationState(PAUSE_BACKWARD);
   } 
   else if (previousState == PLAY_BACKWARD)
   {
      setAnimationState(PLAY_FORWARD);
   } 
   else if (previousState == PAUSE_BACKWARD)
   {
      setAnimationState(PAUSE_FORWARD);
   }
}

void AnimationControllerImp::stop()
{
   AnimationState previousState = getAnimationState();

   if (previousState != STOP)
   {
      removeFromRunningControllers();
      setAnimationState(STOP);

      if (previousState == PLAY_BACKWARD || previousState == PAUSE_BACKWARD)
      {
         moveToEnd();
      }
      else
      {
         moveToBeginning();
      }
   }
}

void AnimationControllerImp::stepForward()
{
   if (mAnimations.empty())
   {
      return;
   }

   const double currentFrame = getCurrentFrame();
   double nextValue = numeric_limits<double>::max();
   for (vector<Animation*>::const_iterator iter = mAnimations.begin();
      iter != mAnimations.end(); ++iter)
   {
      Animation* pAnimation = *iter;
      VERIFYNRV(pAnimation != NULL);

      const double startValue = pAnimation->getStartValue();
      const double stopValue = pAnimation->getStopValue();
      double animationNext;
      if (currentFrame < startValue)
      {
         // All frames are after currentFrame, so use the start frame
         animationNext = startValue;
      }
      else if (currentFrame > stopValue)
      {
         // All frames are before currentFrame
         animationNext = -1;
      }
      else
      {
         // If the animation is after currentFrame, use the animation's current frame
         const double animationCurr = pAnimation->getNextFrameValue(PLAY_FORWARD, 0);
         if (animationCurr > currentFrame)
         {
            animationNext = animationCurr;
         }
         else
         {
            // Otherwise, use the next frame
            animationNext = pAnimation->getNextFrameValue(PLAY_FORWARD, 1);
         }
      }

      if (animationNext >= 0)
      {
         nextValue = min(nextValue, animationNext);
      }
   }

   // set frame to have at least one attached movie
   // to step forward to its next frame
   setCurrentFrame(nextValue);
}

void AnimationControllerImp::stepBackward()
{
   if (mAnimations.empty())
   {
      return;
   }

   double currentFrame = getCurrentFrame();

   // If type is FRAME_ID, need to adjust value to actual frame number in case animation was paused
   // and currentFrame is not a whole number. If we don't adjust, then first step back will just set
   // frame to the whole number for the frame, e.g., value = 37.335 and step back will set it to 37
   // and not 36. Don't have to do this for step forward since value of 37.335 will be set to 38.
   if (getFrameType() == FRAME_ID)
   {
      currentFrame = floor(currentFrame);
   }

   double prevValue = numeric_limits<double>::min();
   for (vector<Animation*>::const_iterator iter = mAnimations.begin();
      iter != mAnimations.end(); ++iter)
   {
      Animation* pAnimation = *iter;
      VERIFYNRV(pAnimation != NULL);
      const double startValue = pAnimation->getStartValue();
      const double stopValue = pAnimation->getStopValue();
      double animationPrev;
      if (currentFrame < startValue)
      {
         // All frames are after currentFrame (no frames should be displayed)
         animationPrev = -1;
      }
      else if (currentFrame > stopValue)
      {
         // All frames are before currentFrame, so use the stop frame
         animationPrev = stopValue;
      }
      else
      {
         // If the animation is before currentFrame, use the animation's current frame
         const double animationCurr = pAnimation->getNextFrameValue(PLAY_BACKWARD, 0);
         if (animationCurr < currentFrame)
         {
            animationPrev = animationCurr;
         }
         else
         {
            // Otherwise, use the previous frame
            animationPrev = pAnimation->getNextFrameValue(PLAY_BACKWARD, 1);
         }
      }

      if (animationPrev >= 0)
      {
         prevValue = max(prevValue, animationPrev);
      }
   }

   setCurrentFrame(prevValue);
}

void AnimationControllerImp::fastForward(double multiplier)
{
   setIntervalMultiplier(multiplier);
   setAnimationState(PLAY_FORWARD);
   play();
}

void AnimationControllerImp::fastRewind(double multiplier)
{
   setIntervalMultiplier(multiplier);
   setAnimationState(PLAY_BACKWARD);
   play();
}

void AnimationControllerImp::removeAnimation(Animation* pAnimation)
{
   if (pAnimation == NULL)
   {
      return;
   }

   vector<Animation*>::iterator iter = mAnimations.begin();
   while (iter != mAnimations.end())
   {
      Animation* pCurrentAnimation = *iter;
      if (pCurrentAnimation == pAnimation)
      {
         AnimationImp* pAnimationImp = dynamic_cast<AnimationImp*> (pAnimation);
         if (pAnimationImp != NULL)
         {
            disconnect(pAnimationImp, SIGNAL(framesChanged(const std::vector<AnimationFrame>&)),
               this, SLOT(updateFrameData()));
            disconnect(pAnimationImp, SIGNAL(objectDetached()), this, SLOT(destroyAnimation()));
         }

         mAnimations.erase(iter);
         pAnimation->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &AnimationControllerImp::movieDeleted));
         emit animationRemoved(pAnimation);
         notify(SIGNAL_NAME(AnimationController, AnimationRemoved), boost::any(pAnimation));
         updateFrameData();
         break;
      }

      ++iter;
   }
}

void AnimationControllerImp::updateFrameData()
{
   // Get the minimum start value and the maximum stop value from all movies
   double startFrame = numeric_limits<double>::max();
   double stopFrame = -1.0;

   vector<Animation*>::iterator iter = mAnimations.begin();
   while (iter != mAnimations.end())
   {
      Animation* pAnimation = *iter;
      if (pAnimation != NULL)
      {
         double currentStart = pAnimation->getStartValue();
         if (currentStart != -1.0 && currentStart < startFrame)
         {
            startFrame = currentStart;
         }

         double currentStop = pAnimation->getStopValue();
         if (currentStop != -1.0 && currentStop > stopFrame)
         {
            stopFrame = currentStop;
         }
      }

      ++iter;
   }

   if (startFrame == numeric_limits<double>::max())
   {
      startFrame = -1.0;
   }

   if ((startFrame != mStartFrame) || (stopFrame != mStopFrame))
   {
      bool shouldStop = false;
      bool shouldMoveToBeginning = false;
      bool shouldMoveToEnd = false;

      // If currentFrame is at the beginning, then keep it at the beginning.
      const double currentFrame = getCurrentFrame();
      if (currentFrame == mStartFrame)
      {
         shouldMoveToBeginning = true;
      }
      // If currentFrame is at the end, then keep it at the end.
      else if (currentFrame == mStopFrame)
      {
         shouldMoveToEnd = true;
      }

      // Stop the controller if the start frame is invalid.
      if (startFrame == -1)
      {
         shouldStop = true;
         shouldMoveToBeginning = true;
      }
      // If the current frame is before the start frame, update the current frame.
      else if (currentFrame < startFrame)
      {
         shouldMoveToBeginning = true;
      }
      // If the current frame is after the stop frame, update the current frame.
      else if (currentFrame > stopFrame)
      {
         shouldMoveToEnd = true;
      }

      // Update the start and stop values
      mStartFrame = startFrame;
      if (mStartBumper < mStartFrame)
      {
         mStartBumper = mStartFrame;
      }
      mStopFrame = stopFrame;
      if (mStopBumper > mStopFrame || mStopBumper < 0.0)  // unset bumper has value of -1.0
      {
         mStopBumper = mStopFrame;
      }
      if (mpBumpersEnabledAction->isChecked())
      {
         mPlaybackStartFrame = mStartBumper;
         mPlaybackStopFrame = mStopBumper;
      }
      else
      {
         mPlaybackStartFrame = mStartFrame;
         mPlaybackStopFrame = mStopFrame;
      }

      if (shouldStop == true)
      {
         stop();
      }

      if (shouldMoveToBeginning == true)
      {
         moveToBeginning();
      }

      if (shouldMoveToEnd == true)
      {
         moveToEnd();
      }

      // Send change notification
      emit frameRangeChanged();
      notify(SIGNAL_NAME(AnimationController, FrameRangeChanged));
   }

   const double currentFrame = getCurrentFrame();
   mCurrentFrame = -1;
   setCurrentFrame(currentFrame);
}

void AnimationControllerImp::runAnimations()
{
   while (mRunningControllers.empty() == false)
   {
      mppActiveController = mRunningControllers.begin();
      while (mppActiveController != mRunningControllers.end())
      {
         AnimationControllerImp* pActiveController = *mppActiveController;
         VERIFYNR(pActiveController != NULL);
         pActiveController->advance();
         QApplication::processEvents();
         QApplication::processEvents(QEventLoop::DeferredDeletion);
         if (mppActiveController != mRunningControllers.end() && pActiveController == *mppActiveController)
         {
            ++mppActiveController;
         }
         // else, it was already incremented when being removed from the list in ACI::stop
      }
   }
}

void AnimationControllerImp::advance()
{
   // Get the current system time in milliseconds
   timeb timeStruct;
   ftime(&timeStruct);
   double currentTime = timeStruct.time*1000.0 + timeStruct.millitm;

   // Compute a ratio of the elapsed system time since the last timeout and expected time
   double expectedTime = 1000.0 / mFrequency;
   double elapsedTime = currentTime - mStartTime;
   if (elapsedTime == 0.0 && getCanDropFrames() == false)
   {
      elapsedTime = 1.0;
      if (mEffectiveCurrentTime + elapsedTime > mMaxCurrentTime)
      {
         elapsedTime = mMaxCurrentTime - mEffectiveCurrentTime;
      }
      mEffectiveCurrentTime += elapsedTime;
   }
   else
   {
      mMaxCurrentTime = currentTime + 1000.0*smallestDecrement(1/60.0);
      elapsedTime = currentTime - mEffectiveCurrentTime;
      mEffectiveCurrentTime = currentTime;
   }
   double intervalMultiplier = elapsedTime / expectedTime;

   // Update the start time for the next timeout
   mStartTime = currentTime;

   // Get the value of the next frame
   double nextFrame = -1.0;
   if (mState == PLAY_FORWARD)
   {
      if (mCurrentFrame >= mPlaybackStopFrame)
      {
         if (mCycle == PLAY_ONCE)
         {
            stop();
            return;
         }
         else if (mCycle == REPEAT)
         {
            nextFrame = mPlaybackStartFrame;
         }
         else if (mCycle == BOUNCE)
         {
            if (mPlaybackStartFrame != mPlaybackStopFrame)
            {
               nextFrame = mCurrentFrame - (mInterval * intervalMultiplier);
            }
            else
            {
               nextFrame = mPlaybackStartFrame;
            }

            setAnimationState(PLAY_BACKWARD);
         }
      }
      else if (mCurrentFrame < mPlaybackStartFrame)
      {
         nextFrame = mPlaybackStartFrame;
      }
      else
      {
         nextFrame = mCurrentFrame + (mInterval * intervalMultiplier);
      }
   }
   else if (mState == PLAY_BACKWARD)
   {
      if (mCurrentFrame <= mPlaybackStartFrame)
      {
         if (mCycle == PLAY_ONCE)
         {
            stop();
            return;
         }
         else if (mCycle == REPEAT)
         {
            nextFrame = mPlaybackStopFrame;
         }
         else if (mCycle == BOUNCE)
         {
            if (mPlaybackStartFrame != mPlaybackStopFrame)
            {
               nextFrame = mCurrentFrame + (mInterval * intervalMultiplier);
            }
            else
            {
               nextFrame = mPlaybackStopFrame;
            }

            setAnimationState(PLAY_FORWARD);
         }
      }
      else if (mCurrentFrame > mPlaybackStopFrame)
      {
         nextFrame = mPlaybackStopFrame;
      }
      else
      {
         nextFrame = mCurrentFrame - (mInterval * intervalMultiplier);
      }
   }

   // Set the next value as the current value
   setCurrentFrame(getNextValue(nextFrame));
}

void AnimationControllerImp::destroyAnimation()
{
   // An object has been detached from the movie, so destroy it if the last object watching it is this player
   AnimationImp* pAnimationImp = dynamic_cast<AnimationImp*>(sender());
   if (pAnimationImp != NULL)
   {
      const list<SafeSlot>& frameChangedObservers = pAnimationImp->getSlots(SIGNAL_NAME(Animation, FrameChanged));
      if (frameChangedObservers.empty())
      {
         const list<SafeSlot>& deletedObservers = pAnimationImp->getSlots(SIGNAL_NAME(Subject, Deleted));
         if (deletedObservers.size() == 1)
         {
            if (deletedObservers.front() == Slot(this, &AnimationControllerImp::movieDeleted))
            {
               Animation* pAnimation = dynamic_cast<Animation*> (pAnimationImp);
               VERIFYNRV(pAnimation != NULL);
               destroyAnimation(pAnimation);
            }
         }
      }
   }
}

void AnimationControllerImp::setCanDropFrames(bool drop)
{
   if (mCanDropFrames != drop)
   {
      mCanDropFrames = drop;
      emit canDropFramesChanged(drop);
   }
}

bool AnimationControllerImp::getCanDropFrames() const
{
   return mCanDropFrames;
}

double AnimationControllerImp::getNextValue(double value) const
{
   double newValue = value;
   AnimationState state = getAnimationState();
   if (state == PLAY_FORWARD)
   {
      for (vector<Animation*>::const_iterator iter = mAnimations.begin();
         iter != mAnimations.end(); ++iter)
      {
         Animation* pAnimation = *iter;
         VERIFYRV(pAnimation != NULL, -1.0);

         double movieValue = pAnimation->getNextFrameValue(state, 2);
         if (movieValue >= 0)
         {
            newValue = min(newValue, movieValue);
         }
      }
      if (newValue != value)
      {
         newValue = smallestDecrement(newValue); // need to be somewhat shy of two frames away
      }
   }
   else if (state == PLAY_BACKWARD)
   {
      for (vector<Animation*>::const_iterator iter = mAnimations.begin();
         iter != mAnimations.end(); ++iter)
      {
         Animation* pAnimation = *iter;
         VERIFYRV(pAnimation != NULL, -1.0);

         double movieValue = pAnimation->getNextFrameValue(state, 1);
         if (movieValue >= 0)
         {
            newValue = max(newValue, movieValue);
         }
      }
   }

   if (newValue != value)
   {
      if (getCanDropFrames())
      {
         mpDesktop->setStatusBarMessage("Frames are being dropped to preserve the frame speed.");

         return value;
      }

      mpDesktop->setStatusBarMessage("Slowing down to avoid dropping frames.");
   }

   return newValue;
}

bool AnimationControllerImp::serialize(SessionItemSerializer& serializer) const
{
   XMLWriter xml("AnimationController");

   if (!SessionItemImp::toXml(&xml))
   {
      return false;
   }

   xml.addAttr("frametype", mFrameType);
   xml.addAttr("startframe", mStartFrame);
   xml.addAttr("stopframe", mStopFrame);
   xml.addAttr("currentframe", mCurrentFrame);
   xml.addAttr("startbumper", mStartBumper);
   xml.addAttr("stopbumper", mStopBumper);
   xml.addAttr("bumpersenabled", mBumpersEnabled);
   // ideally, this would use StringUtilities but we don't want
   // to impose a boost requirement on everything that uses StringUtilities
   xml.addAttr("minimumframerate", QString("%1/%2").arg(mMinimumFrameRate.numerator())
                                                   .arg(mMinimumFrameRate.denominator()).toStdString());
   xml.addAttr("interval", mInterval);
   xml.addAttr("cycle", mCycle);
   xml.addAttr("dropframes", mCanDropFrames);

   Service<AnimationServices> pServices;
   if (dynamic_cast<const AnimationController*>(this) == pServices->getCurrentAnimationController())
   {
      xml.addAttr("selected", true);
   }

   for (vector<Animation*>::const_iterator it = mAnimations.begin(); it != mAnimations.end(); ++it)
   {
      xml.addAttr("id", (*it)->getId(), xml.addElement("animation"));
   }

   return serializer.serialize(xml);
}

bool AnimationControllerImp::deserialize(SessionItemDeserializer &deserializer)
{
   XmlReader reader(NULL, false);
   DOMElement* pRoot = deserializer.deserialize(reader, "AnimationController");
   if (pRoot == NULL || !SessionItemImp::fromXml(pRoot, XmlBase::VERSION))
   {
      return false;
   }
   mFrameType = StringUtilities::fromXmlString<FrameType>(
      A(pRoot->getAttribute(X("frametype"))));
   mStartFrame = StringUtilities::fromXmlString<double>(
      A(pRoot->getAttribute(X("startframe"))));
   mStopFrame = StringUtilities::fromXmlString<double>(
      A(pRoot->getAttribute(X("stopframe"))));
   mCurrentFrame = StringUtilities::fromXmlString<double>(
      A(pRoot->getAttribute(X("currentframe"))));
   resetBumpers();  // initialize to start and stop frames
   setStartBumper(StringUtilities::fromXmlString<double>(
      A(pRoot->getAttribute(X("startbumper")))));
   setStopBumper(StringUtilities::fromXmlString<double>(
      A(pRoot->getAttribute(X("stopbumper")))));
   setBumpersEnabled(StringUtilities::fromXmlString<bool>(
      A(pRoot->getAttribute(X("bumpersenabled")))));

   if (mBumpersEnabled)
   {
      mPlaybackStartFrame = mStartBumper;
      mPlaybackStopFrame = mStopBumper;
   }
   else
   {
      mPlaybackStartFrame = mStartFrame;
      mPlaybackStopFrame = mStopFrame;
   }
   QStringList minFrameRate = QString(A(pRoot->getAttribute(X("minimumframerate")))).split("/");
   if (minFrameRate.size() != 2)
   {
      return false;
   }
   mMinimumFrameRate.assign(minFrameRate[0].toInt(), minFrameRate[1].toInt());
   mInterval = StringUtilities::fromXmlString<double>(
      A(pRoot->getAttribute(X("interval"))));
   mCycle = StringUtilities::fromXmlString<AnimationCycle>(
      A(pRoot->getAttribute(X("cycle"))));
   mCanDropFrames = StringUtilities::fromXmlString<bool>(
      A(pRoot->getAttribute(X("dropframes"))));
   for (DOMNode *pNode = pRoot->getFirstChild(); pNode != NULL; pNode = pNode->getNextSibling())
   {
      if (XMLString::equals(pNode->getNodeName(), X("DisplayText")))
      {
         setDisplayText(A(pNode->getTextContent()));
      }
      else if (XMLString::equals(pNode->getNodeName(), X("animation")))
      {
         DOMElement* pElement(static_cast<DOMElement*>(pNode));
         string id = A(pElement->getAttribute(X("id")));
         AnimationImp* pAnimation = dynamic_cast<AnimationImp*>(Service<SessionManager>()->getSessionItem(id));
         if (pAnimation == NULL)
         {
            return false;
         }
         connect(pAnimation, SIGNAL(framesChanged(const std::vector<AnimationFrame>&)), this, SLOT(updateFrameData()));
         connect(pAnimation, SIGNAL(objectDetached()), this, SLOT(destroyAnimation()));
         mAnimations.push_back(dynamic_cast<Animation*>(pAnimation));
         pAnimation->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &AnimationControllerImp::movieDeleted));
      }
   }
   setCurrentFrame(mCurrentFrame);

#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : Should this be moved into the tool bar? (mconsidine)")
   if (pRoot->hasAttribute(X("selected")))
   {
      Service<AnimationServices> pServices;
      pServices->setCurrentAnimationController(dynamic_cast<AnimationController*>(this));
   }
   return true;
}

bool AnimationControllerImp::getBumpersEnabled() const
{
   return mBumpersEnabled;
}

void AnimationControllerImp::setBumpersEnabled(bool enabled)
{
   if (enabled == mBumpersEnabled)
   {
      return;
   }

   mBumpersEnabled = enabled;
   mpBumpersEnabledAction->setChecked(mBumpersEnabled);
   if (mBumpersEnabled)
   {
      mPlaybackStartFrame = mStartBumper;
      mPlaybackStopFrame = mStopBumper;
      if (mCurrentFrame < mPlaybackStartFrame)
      {
         setCurrentFrame(mPlaybackStartFrame);
      }
      else if (mCurrentFrame > mPlaybackStopFrame)
      {
         setCurrentFrame(mPlaybackStopFrame);
      }
   }
   else
   {
      mPlaybackStartFrame = mStartFrame;
      mPlaybackStopFrame = mStopFrame;
   }

   emit bumpersEnabledChanged(mBumpersEnabled);
   notify(SIGNAL_NAME(AnimationController, BumpersEnabledChanged), boost::any(mBumpersEnabled));
}

void AnimationControllerImp::resetBumpers()
{
   setStartBumper(mStartFrame);
   setStopBumper(mStopFrame);
}
void AnimationControllerImp::setStartBumper(double frameValue)
{
   double newValue = frameValue;
   if (getFrameType() == FRAME_ID)
   {
      newValue = floor(frameValue + 0.5);
   }

   if (newValue == mStartBumper)
   {
      return;
   }

   mStartBumper = newValue;

   if (mStartBumper < mStartFrame)
   {
      mStartBumper = mStartFrame;
   }

   if (mStartBumper > mStopBumper)
   {
      mStartBumper = mStopBumper;
   }

   if (mBumpersEnabled)
   {
      mPlaybackStartFrame = mStartBumper;
   }

   if (mCurrentFrame < mPlaybackStartFrame)
   {
      setCurrentFrame(mPlaybackStartFrame);
   }

   if (!mBumpersEnabled && mStartBumper >= 0.0)  // invalid bumper = -1.0
   {
      setBumpersEnabled(true);
   }

   emit bumperStartChanged(mStartBumper);
   notify(SIGNAL_NAME(AnimationController, BumperStartChanged), boost::any(mStartBumper));
}

void AnimationControllerImp::setStopBumper(double frameValue)
{
   double newValue = frameValue;
   if (getFrameType() == FRAME_ID)
   {
      newValue = floor(frameValue + 0.5);
   }

   if (newValue == mStopBumper)
   {
      return;
   }

   mStopBumper = newValue;

   if (mStopBumper > mStopFrame || mStopBumper < 0.0)  // value of -1.0 for reset or uninitialized bumper
   {
      mStopBumper = mStopFrame;
   }

   if (mStopBumper < mStartBumper)
   {
      mStopBumper = mStartBumper;
   }

   if (mBumpersEnabled)
   {
      mPlaybackStopFrame = mStopBumper;
   }

   if (mCurrentFrame > mPlaybackStopFrame)
   {
      setCurrentFrame(mPlaybackStopFrame);
   }

   if (!mBumpersEnabled && mStopBumper >= 0.0)  // invalid bumper = -1.0
   {
      setBumpersEnabled(true);
   }
   emit bumperStopChanged(mStopBumper);
   notify(SIGNAL_NAME(AnimationController, BumperStopChanged), boost::any(mStopBumper));
}

void AnimationControllerImp::snapStartBumperToFrame()
{
   setStartBumper(mCurrentFrame);
}

void AnimationControllerImp::snapStopBumperToFrame()
{
   setStopBumper(mCurrentFrame);
}

double AnimationControllerImp::getStartBumper() const
{
   return mStartBumper;
}

double AnimationControllerImp::getStopBumper() const
{
   return mStopBumper;
}

void AnimationControllerImp::changeBumpersEnabled(bool enabled)
{
   setBumpersEnabled(enabled);
}

void AnimationControllerImp::storeBumpers()
{
   Service<ConfigurationSettings> pSettings;
   string bumperPath("AnimationController/Bumpers");
   pSettings->setSetting(bumperPath + "/BumpersActive", mBumpersEnabled);
   pSettings->setSetting(bumperPath + "/StartBumper", mStartBumper);
   pSettings->setSetting(bumperPath + "/StopBumper", mStopBumper);
}

void AnimationControllerImp::restoreBumpers()
{
   Service<ConfigurationSettings> pSettings;
   string bumperPath("AnimationController/Bumpers");
   bool validRestore(true);
   bool bumpersEnabled;
   double startBumper;
   double stopBumper;
   const bool* pBoolValue = dv_cast<bool>(&pSettings->getSetting(bumperPath + "/BumpersActive"));
   if (pBoolValue != NULL)
   {
      bumpersEnabled = *pBoolValue;
   }
   else
   {
      validRestore = false;
   }

   const double* pDoubleValue = dv_cast<double>(&pSettings->getSetting(bumperPath + "/StartBumper"));
   if (pDoubleValue != NULL)
   {
      startBumper = *pDoubleValue;
   }
   else
   {
      validRestore = false;
   }

   pDoubleValue = dv_cast<double>(&pSettings->getSetting(bumperPath + "/StopBumper"));
   if (pDoubleValue != NULL)
   {
      stopBumper = *pDoubleValue;
   }
   else
   {
      validRestore = false;
   }

   if (validRestore && startBumper >= 0.0 && stopBumper >= 0.0)
   {
      setStartBumper(startBumper);
      setStopBumper(stopBumper);
      setBumpersEnabled(bumpersEnabled);
   }
   else
   {
      if (Service<ApplicationServices>()->isBatch() == false)
      {
         QMessageBox::critical(mpDesktop->getMainWidget(), "Restore Bumpers Failed",
            "Could not retrieve valid bumper set from Configuration Settings");
      }
   }
}

void AnimationControllerImp::adjustBumpers()
{
   QDialog dlg(mpDesktop->getMainWidget());
   dlg.setWindowTitle("Adjust Bumpers");
   dlg.setModal(true);
   QVBoxLayout* pLayout = new QVBoxLayout(&dlg);
   AnimationFrameSubsetWidget* pFrameSubset = new AnimationFrameSubsetWidget(&dlg);
   pFrameSubset->setFrames(dynamic_cast<AnimationController*>(this));
   pFrameSubset->setStartFrame(mStartBumper);
   pFrameSubset->setStopFrame(mStopBumper);
   QDialogButtonBox* pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
      Qt::Horizontal, &dlg);
   QFrame* pLine = new QFrame(&dlg);
   pLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);
   pLayout->setMargin(10);
   pLayout->setSpacing(10);
   pLayout->addWidget(pFrameSubset);
   pLayout->addStretch();
   pLayout->addWidget(pLine);
   pLayout->addWidget(pButtonBox);
   VERIFYNR(connect(pButtonBox, SIGNAL(accepted()), &dlg, SLOT(accept())));
   VERIFYNR(connect(pButtonBox, SIGNAL(rejected()), &dlg, SLOT(reject())));

   if (dlg.exec() == QDialog::Accepted)
   {
      setStartBumper(pFrameSubset->getStartFrame());
      setStopBumper(pFrameSubset->getStopFrame());
   }
}
