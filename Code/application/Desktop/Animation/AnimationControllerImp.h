/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ANIMATIONCONTROLLERIMP_H
#define ANIMATIONCONTROLLERIMP_H

#include <QtCore/QObject>
#include <QtCore/QTimer>
#include <QtGui/QAction>

#include "DesktopServices.h"
#include "SessionItemImp.h"
#include "SubjectImp.h"
#include "TypesFile.h"
#include "Service.h"

#include <boost/rational.hpp>
#include <list>
#include <vector>

class Animation;

class AnimationControllerImp : public QObject, public SessionItemImp, public SubjectImp
{
   Q_OBJECT

public:
   AnimationControllerImp(FrameType frameType, const std::string& id);
   ~AnimationControllerImp();

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   std::list<ContextMenuAction> getContextMenuActions() const;

   void movieDeleted(Subject &subject, const std::string &signal, const boost::any &v);

   void setName(const std::string& name);

   Animation* createAnimation(const QString& strName);
   Animation* getAnimation(const QString& strName) const;
   bool hasAnimation(const QString& strName) const;
   const std::vector<Animation*>& getAnimations() const;
   unsigned int getNumAnimations() const;
   void destroyAnimation(Animation* pAnimation);

   FrameType getFrameType() const;
   void setCurrentFrame(double frameValue);
   double getCurrentFrame() const;
   double getStartFrame() const;
   double getStopFrame() const;
   bool getBumpersEnabled() const;
   void setBumpersEnabled(bool enabled);
   double getStartBumper() const;
   void setStartBumper(double frameValue);
   double getStopBumper() const;
   void setStopBumper(double frameValue);

   void setIntervalMultiplier(double multiplier);
   double getIntervalMultiplier() const;
   const int getFrequency() const;

   boost::rational<int> getMinimumFrameRate() const;
   void setMinimumFrameRate(boost::rational<int> frameRate);

   void setAnimationState(AnimationState state);
   AnimationState getAnimationState() const;

   void setAnimationCycle(AnimationCycle cycle);
   AnimationCycle getAnimationCycle() const;

   void setCanDropFrames(bool drop);
   bool getCanDropFrames() const;

   double getNextValue(double value) const;

public slots:
   void moveToBeginning();
   void moveToEnd();
   void play();
   void pause();
   void stop();
   void changeDirection();

   void stepForward();
   void stepBackward();
   void fastForward(double);
   void fastRewind(double);
   void changeBumpersEnabled(bool enabled);
   void snapStartBumperToFrame();
   void snapStopBumperToFrame();
   void adjustBumpers();
   void resetBumpers();
   void storeBumpers();
   void restoreBumpers();

   bool serialize(SessionItemSerializer& serializer) const;
   bool deserialize(SessionItemDeserializer &deserializer);

signals:
   void renamed(const QString& strName);
   void animationAdded(Animation* pAnimation);
   void animationRemoved(Animation* pAnimation);
   void frameRangeChanged();
   void frameChanged(double frameValue);
   void animationStateChanged(AnimationState state);
   void animationCycleChanged(AnimationCycle cycle);
   void intervalMultiplierChanged(double multiplier);
   void canDropFramesChanged(bool canDropFrames);
   void bumpersEnabledChanged(bool enabled);
   void bumperStartChanged(double newValue);
   void bumperStopChanged(double newValue);

protected:
   void removeAnimation(Animation* pAnimation);

protected slots:
   void updateFrameData();
   void runAnimations();
   void advance();
   void destroyAnimation();

private:
   void removeFromRunningControllers();

   Service<DesktopServices> mpDesktop;

   std::vector<Animation*> mAnimations;

   FrameType mFrameType;
   double mStartFrame;
   double mStopFrame;
   double mCurrentFrame;
   double mMaxCurrentTime;
   double mEffectiveCurrentTime;
   double mStartBumper;
   double mStopBumper;
   double mPlaybackStartFrame;
   double mPlaybackStopFrame;
   const int mFrequency;
   boost::rational<int> mMinimumFrameRate;
   double mInterval;

   AnimationState mState;
   AnimationCycle mCycle;

   double mStartTime;
   bool mCanDropFrames;
   bool mBumpersEnabled;

   QAction* mpPlayAction;
   QAction* mpPauseAction;
   QAction* mpStopAction;
   QAction* mpChangeDirectionAction;
   QAction* mpStepBackwardAction;
   QAction* mpStepForwardAction;
   QAction* mpBumpersEnabledAction;
   QAction* mpLeftBumperToCurrentAction;
   QAction* mpRightBumperToCurrentAction;
   QAction* mpAdjustBumpersAction;
   QAction* mpResetBumpersAction;
   QAction* mpSeparatorAction;
   QAction* mpSeparator2Action;
   QAction* mpStoreBumpersAction;
   QAction* mpRestoreBumpersAction;
   static std::list<AnimationControllerImp*> mRunningControllers;
   static std::list<AnimationControllerImp*>::iterator mppActiveController;
};

#define ANIMATIONCONTROLLERADAPTEREXTENSION_CLASSES \
   SESSIONITEMADAPTEREXTENSION_CLASSES \
   SUBJECTADAPTEREXTENSION_CLASSES

#define ANIMATIONCONTROLLERADAPTER_METHODS(impClass) \
   SESSIONITEMADAPTER_METHODS(impClass) \
   SUBJECTADAPTER_METHODS(impClass) \
   Animation* createAnimation(const std::string& name) \
   { \
      return impClass::createAnimation(QString::fromStdString(name)); \
   } \
   Animation* getAnimation(const std::string& name) const \
   { \
      return impClass::getAnimation(QString::fromStdString(name)); \
   } \
   bool hasAnimation(const std::string& name) const \
   { \
      return impClass::hasAnimation(QString::fromStdString(name)); \
   } \
   const std::vector<Animation*>& getAnimations() const \
   { \
      return impClass::getAnimations(); \
   } \
   unsigned int getNumAnimations() const \
   { \
      return impClass::getNumAnimations(); \
   } \
   void destroyAnimation(Animation* pAnimation) \
   { \
      return impClass::destroyAnimation(pAnimation); \
   } \
   FrameType getFrameType() const \
   { \
      return impClass::getFrameType(); \
   } \
   void setCurrentFrame(double frameValue) \
   { \
      return impClass::setCurrentFrame(frameValue); \
   } \
   double getCurrentFrame() const \
   { \
      return impClass::getCurrentFrame(); \
   } \
   double getStartFrame() const \
   { \
      return impClass::getStartFrame(); \
   } \
   double getStopFrame() const \
   { \
      return impClass::getStopFrame(); \
   } \
   bool getBumpersEnabled() const \
   { \
      return impClass::getBumpersEnabled(); \
   } \
   void setBumpersEnabled(bool enabled) \
   { \
      return impClass::setBumpersEnabled(enabled); \
   } \
   double getStartBumper() const \
   { \
      return impClass::getStartBumper(); \
   } \
   void setStartBumper(double frameValue) \
   { \
      return impClass::setStartBumper(frameValue); \
   } \
   double getStopBumper() const \
   { \
      return impClass::getStopBumper(); \
   } \
   void setStopBumper(double frameValue) \
   { \
      return impClass::setStopBumper(frameValue); \
   } \
   void setIntervalMultiplier(double multiplier) \
   { \
      return impClass::setIntervalMultiplier(multiplier); \
   } \
   double getIntervalMultiplier() const \
   { \
      return impClass::getIntervalMultiplier(); \
   } \
   const int getFrequency() const \
   { \
      return impClass::getFrequency(); \
   } \
   boost::rational<int> getMinimumFrameRate() const \
   { \
      return impClass::getMinimumFrameRate(); \
   } \
   void setMinimumFrameRate(boost::rational<int> frameRate) \
   { \
      impClass::setMinimumFrameRate(frameRate); \
   } \
   void setAnimationState(AnimationState state) \
   { \
      return impClass::setAnimationState(state); \
   } \
   AnimationState getAnimationState() const \
   { \
      return impClass::getAnimationState(); \
   } \
   void setAnimationCycle(AnimationCycle cycle) \
   { \
      return impClass::setAnimationCycle(cycle); \
   } \
   AnimationCycle getAnimationCycle() const \
   { \
      return impClass::getAnimationCycle(); \
   } \
   void moveToBeginning() \
   { \
      return impClass::moveToBeginning(); \
   } \
   void moveToEnd() \
   { \
      return impClass::moveToEnd(); \
   } \
   void pause() \
   { \
      return impClass::pause(); \
   } \
   void stop() \
   { \
      return impClass::stop(); \
   } \
   void setCanDropFrames(bool drop) \
   { \
      return impClass::setCanDropFrames(drop); \
   } \
   bool getCanDropFrames() const \
   { \
      return impClass::getCanDropFrames(); \
   } \
   void play() \
   { \
      return impClass::play(); \
   } \
   void stepForward() \
   { \
      return impClass::stepForward(); \
   } \
   void stepBackward() \
   { \
      return impClass::stepBackward(); \
   } \
   void fastForward(double multiplier) \
   { \
      return impClass::fastForward(multiplier); \
   } \
   void fastRewind(double multiplier) \
   { \
      return impClass::fastRewind(multiplier); \
   }
   
#endif
