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
   bool insertAnimation(Animation* pAnimation);
   Animation* getAnimation(const QString& strName) const;
   bool hasAnimation(const QString& strName) const;
   bool hasAnimation(Animation* pAnimation) const;
   const std::vector<Animation*>& getAnimations() const;
   unsigned int getNumAnimations() const;
   void removeAnimation(Animation* pAnimation);
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
   bool getResetOnStop() const;
   void setResetOnStop(bool enabled);

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
   virtual bool event(QEvent* pEvent);

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
   void bumpersEnabledChanged(bool enabled);
   void bumperStartChanged(double newValue);
   void bumperStopChanged(double newValue);

protected slots:
   void updateFrameData();
   void runAnimations();
   void advance();
   void destroyAnimation();

private:
   AnimationControllerImp(const AnimationControllerImp& rhs);
   AnimationControllerImp& operator=(const AnimationControllerImp& rhs);

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
   QAction* mpSeparator3Action;
   QAction* mpStoreBumpersAction;
   QAction* mpRestoreBumpersAction;
   QAction* mpCanDropFramesAction;
   QAction* mpResetOnStopAction;
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
   bool insertAnimation(Animation* pAnimation) \
   { \
      return impClass::insertAnimation(pAnimation); \
   } \
   Animation* getAnimation(const std::string& name) const \
   { \
      return impClass::getAnimation(QString::fromStdString(name)); \
   } \
   bool hasAnimation(const std::string& name) const \
   { \
      return impClass::hasAnimation(QString::fromStdString(name)); \
   } \
   bool hasAnimation(Animation* pAnimation) const \
   { \
      return impClass::hasAnimation(pAnimation); \
   } \
   const std::vector<Animation*>& getAnimations() const \
   { \
      return impClass::getAnimations(); \
   } \
   unsigned int getNumAnimations() const \
   { \
      return impClass::getNumAnimations(); \
   } \
   void removeAnimation(Animation* pAnimation) \
   { \
      impClass::removeAnimation(pAnimation); \
   } \
   void destroyAnimation(Animation* pAnimation) \
   { \
      impClass::destroyAnimation(pAnimation); \
   } \
   FrameType getFrameType() const \
   { \
      return impClass::getFrameType(); \
   } \
   void setCurrentFrame(double frameValue) \
   { \
      impClass::setCurrentFrame(frameValue); \
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
      impClass::setBumpersEnabled(enabled); \
   } \
   double getStartBumper() const \
   { \
      return impClass::getStartBumper(); \
   } \
   void setStartBumper(double frameValue) \
   { \
      impClass::setStartBumper(frameValue); \
   } \
   double getStopBumper() const \
   { \
      return impClass::getStopBumper(); \
   } \
   void setStopBumper(double frameValue) \
   { \
      impClass::setStopBumper(frameValue); \
   } \
   bool getResetOnStop() const \
   { \
      return impClass::getResetOnStop(); \
   } \
   void setResetOnStop(bool enableReset) \
   { \
      impClass::setResetOnStop(enableReset); \
   } \
   void setIntervalMultiplier(double multiplier) \
   { \
      impClass::setIntervalMultiplier(multiplier); \
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
      impClass::setAnimationState(state); \
   } \
   AnimationState getAnimationState() const \
   { \
      return impClass::getAnimationState(); \
   } \
   void setAnimationCycle(AnimationCycle cycle) \
   { \
      impClass::setAnimationCycle(cycle); \
   } \
   AnimationCycle getAnimationCycle() const \
   { \
      return impClass::getAnimationCycle(); \
   } \
   void moveToBeginning() \
   { \
      impClass::moveToBeginning(); \
   } \
   void moveToEnd() \
   { \
      impClass::moveToEnd(); \
   } \
   void pause() \
   { \
      impClass::pause(); \
   } \
   void stop() \
   { \
      impClass::stop(); \
   } \
   void setCanDropFrames(bool drop) \
   { \
      impClass::setCanDropFrames(drop); \
   } \
   bool getCanDropFrames() const \
   { \
      return impClass::getCanDropFrames(); \
   } \
   void play() \
   { \
      impClass::play(); \
   } \
   void stepForward() \
   { \
      impClass::stepForward(); \
   } \
   void stepBackward() \
   { \
      impClass::stepBackward(); \
   } \
   void fastForward(double multiplier) \
   { \
      impClass::fastForward(multiplier); \
   } \
   void fastRewind(double multiplier) \
   { \
      impClass::fastRewind(multiplier); \
   }
   
#endif
