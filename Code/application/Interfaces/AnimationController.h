/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ANIMATIONCONTROLLER_H
#define ANIMATIONCONTROLLER_H

#include "ConfigurationSettings.h"
#include "SessionItem.h"
#include "Subject.h"
#include "TypesFile.h"

#include <string>
#include <vector>

#include <boost/rational.hpp>

class Animation;

/**
 *  Plays one or more animations synchronously.
 *
 *  An animation controller allows multiple animations to be played synchronously at a particular
 *  rate.  This can be particularly useful with time-based controllers.  In addition to
 *  the animations, the controller stores a start value, stop value, and current value.  These
 *  values are updated automatically when the frames in an animation change, or when an
 *  animation is destroyed.  The start value is defined as the minimum value across all
 *  frames in all animations according to the frame value type specified at the creation
 *  of the controller.  Similarly, the stop value is defined as the maximum value across
 *  all frames in all animations according to the controller's frame value type.  Animations must
 *  be created within the animation controller, which guarantees that the animation and the controller
 *  will have the same frame type.
 *
 *  When playing animations, each animation is updated at a constant frequency, specified by
 *  getFrequency().  This means that rate at which Animation::setCurrentFrame() is called
 *  does not change.  The value that is passed to the animation when the current frame is
 *  set can be modified by changing the interval multiplier.  This has the effect of
 *  speeding up or slowing down the animations.
 *
 *  The frame rate is based on the frequency and the interval multiplier.  In some cases
 *  the desired frame rate cannot be achieved due to limitations in the hardware.  If
 *  this occurs, frames will be dropped when playing through the animations to preserve the
 *  integrity of the frame rate.
 *
 *  An animation controller is created and destroyed from the animation window.  When an animation controller
 *  is destroyed, all animations contained in the controller are also destroyed.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *    - The controller is deleted.
 *    - The following methods are called: createAnimation(), setCurrentFrame(),
 *      setIntervalMultiplier(), setAnimationState(), setAnimationCycle(), and
 *      destroyAnimation().
 *    - The frame set in one of the animations changes.
 *    - Other notifications documented in the Subject class.
 *
 *  @see     Animation, AnimationDialog
 */
class AnimationController : public SessionItem, public Subject
{
public:
   SETTING(FrameSpeedSelection, AnimationController, double, 0)
   SETTING(AnimationCycleSelection, AnimationController, AnimationCycle, PLAY_ONCE)
   SETTING(CanDropFrames, AnimationController, bool, false)

   /**
    *  Emitted with boost::any<std::string> when the controller is renamed.
    *
    *  @see     AnimationServices::renameAnimationController()
    */
   SIGNAL_METHOD(AnimationController, Renamed)

   /**
    *  Emitted with boost::any<Animation*> when an Animation is added to the
    *  controller.
    */
   SIGNAL_METHOD(AnimationController, AnimationAdded)

   /**
    *  Emitted with boost::any<double> when the current frame changes.
    *
    *  @see getCurrentFrame()
    */
   SIGNAL_METHOD(AnimationController, FrameChanged)

   /**
    *  Emitted with boost::any<double> when the speed multiplier changes.
    */
   SIGNAL_METHOD(AnimationController, IntervalMultiplierChanged)

   /**
    *  Emitted with boost::any<AnimationState> when the animation state of the
    *  controller changes.
    */
   SIGNAL_METHOD(AnimationController, AnimationStateChanged)

   /**
    *  Emitted with boost::any<AnimationCycle> when the animation cycle of the
    *  controller changes.
    */
   SIGNAL_METHOD(AnimationController, AnimationCycleChanged)

   /**
    *  Emitted with boost::any<Animation*> when an Animation is removed from
    *  the controller. This signal will not be emitted when the controller
    *  is removing Animations within its destructor.
    */
   SIGNAL_METHOD(AnimationController, AnimationRemoved)

   /**
    *  Emitted when the frame range changes for any reason.
    */
   SIGNAL_METHOD(AnimationController, FrameRangeChanged)

   /**
   *  Emitted with boost::any<bool> when the enabled status of the playback
   *  bumpers changes for any reason.
   */
   SIGNAL_METHOD(AnimationController, BumpersEnabledChanged)

   /**
   *  Emitted with boost::any<double> when the start playback bumper changes for any reason.
   */
   SIGNAL_METHOD(AnimationController, BumperStartChanged)

   /**
   *  Emitted with boost::any<double> when the stop playback bumper changes for any reason.
   */
   SIGNAL_METHOD(AnimationController, BumperStopChanged)

   /**
    *  Creates a new animation and adds it to the controller.
    *
    *  This method creates a new animation with the given name and the same frame type as
    *  the controller.  The animation is then added to the internal vector of animations.  The
    *  animation name must be unique within the controller.
    *
    *  @param   name
    *           The animation name.  This name cannot be empty and must be unique for all
    *           animations in the controller.
    *
    *  @return  A pointer to the new animation.  NULL is returned if an animation of the given
    *           name already exists in the controller.
    *
    *  @notify  This method will notify signalAnimationAdded() with any<Animation*> after the animation
    *           is added to the internal vector.
    */
   virtual Animation* createAnimation(const std::string& name) = 0;

   /**
    *  Retrieves an animation with a given name.
    *
    *  @param   name
    *           The animation name.
    *
    *  @return  A pointer to the existing animation in the controller.  NULL is returned if no
    *           animation exists in the controller with the given name.
    */
   virtual Animation* getAnimation(const std::string& name) const = 0;

   /**
    *  Queries whether an animation with a given name exists in the controller.
    *
    *  @param   name
    *           The animation name.
    *
    *  @return  True if an animation with the given name exists in the controller, otherwise
    *           false.
    */
   virtual bool hasAnimation(const std::string& name) const = 0;

   /**
    *  Retrieves all animations in the controller.
    *
    *  @return  A const reference to the internal vector of animations.  The vector should
    *           not be modified in any way.  To change the contents of the vector, use
    *           createAnimation() and destroyAnimation() instead.
    */
   virtual const std::vector<Animation*>& getAnimations() const = 0;

   /**
    *  Returns the number of animations in the controller.
    *
    *  This is a convenience method that is identical to getAnimations().size().
    *
    *  @return  The number of animations in the controller.
    */
   virtual unsigned int getNumAnimations() const = 0;

   /**
    *  Removes an existing animation from the controller and deletes it.
    *
    *  @param   pAnimation
    *           The animation to destroy.
    *
    *  @notify  This method will notify signalAnimationRemoved() with any<Animation*> after the animation
    *           has been removed from the controller but before the animation is deleted.  This
    *           method will also notify a second time if the result of removing the
    *           animation changes the start and/or stop values for the controller.  This
    *           notification will occur after the removal notification and before the
    *           animation is deleted.
    */
   virtual void destroyAnimation(Animation* pAnimation) = 0;

   /**
    *  Returns the type of frame value that the controller uses to set the current frame
    *  in the animations.
    *
    *  This method returns whether the controller operates on frame numbers or time values
    *  when setting the current frame in the animations.  The frame type is specified when
    *  the controller is created in AnimationToolBar::createAnimationController().  Once the controller is
    *  created, the frame type cannot be changed.
    *
    *  @return  The animation controller's frame type.
    *
    *  @see     setCurrentFrame(), getCurrentFrame(), getStartFrame(), getStopFrame()
    */
   virtual FrameType getFrameType() const = 0;

   /**
    *  Sets the current frame value in the controller.
    *
    *  This method sets the controller's frame value, and automatically sets the current
    *  frame in each animation with the given value.  This may or may not actually set the
    *  current frame in the animation.  Since the value is the same across all animations in
    *  the controller, it is the responsibility of the animation to determine whether or not to
    *  update its current frame based on the value.
    *
    *  @param   frameValue
    *           The new frame value for the controller.  The value should correspond to the
    *           type returned by getFrameType().
    *
    *  @notify  This method will notify signalFrameChanged() with any<const AnimationFrame*>.
    */
   virtual void setCurrentFrame(double frameValue) = 0;

   /**
    *  Returns the current frame value in the controller.
    *
    *  @return  The current frame value.  The value corresponds to the type returned by
    *           getFrameType().
    */
   virtual double getCurrentFrame() const = 0;

   /**
    *  Returns the starting frame value in the controller.
    *
    *  The starting frame value is defined as the minimum frame value across all frames
    *  in all animations.
    *
    *  @return  The starting frame value.  The value corresponds to the type returned by
    *           getFrameType().
    */
   virtual double getStartFrame() const = 0;

   /**
    *  Returns the ending frame value in the controller.
    *
    *  The ending frame value is defined as the maximum frame value across all frames
    *  in all animations.
    *
    *  @return  The ending frame value.  The value corresponds to the type returned by
    *           getFrameType().
    */
   virtual double getStopFrame() const = 0;

   /**
    *  Returns the enabled status of the playback bumpers in the controller.
    *
    *  When the playback bumpers are enabled (status of \c true), the animation playback
    *  will be limited to the frame values between the start and stop playback bumpers.
    *
    *  @return  The enabled status of the playback bumpers.
    */
   virtual bool getBumpersEnabled() const = 0;

   /**
    *  Sets the enabled status of the playback bumpers in the controller.
    *
    *  When the playback bumpers are enabled (status of \c true), the animation playback
    *  will be limited to the frame values between the start and stop playback bumpers.
    *
    *  @param  enabled
    *          The enabled status of the playback bumpers.
    *
    *  @notify  This method will notify signalBumpersEnabledChanged() with boost::any<bool>.
    */
   virtual void setBumpersEnabled(bool enabled) = 0;

   /**
    *  Returns the start playback bumper value in the controller.
    *
    *  The start playback bumper value is defined as the first frame value across all frames
    *  in all animations that will be played back when the playback bumpers are enabled.
    *
    *  @return  The start bumper value.  The value corresponds to the type returned by
    *           getFrameType().
    */
   virtual double getStartBumper() const = 0;

   /**
    *  Sets the start playback bumper value in the controller.
    *
    *  The start playback bumper value is defined as the first frame value across all frames
    *  in all animations that will be played back when the playback bumpers are enabled.
    *
    *  @param  frameValue
    *          The start bumper value.
    *
    *  @notify  This method will notify signalBumperStartChanged() with boost::any<double>.
    */
   virtual void setStartBumper(double frameValue) = 0;

   /**
   *  Returns the stop playback bumper value in the controller.
   *
   *  The stop playback bumper value is defined as the last frame value across all frames
   *  in all animations that will be played back when the playback bumpers are enabled.
   *
   *  @return  The stop bumper value.  The value corresponds to the type returned by
   *           getFrameType().
   */
   virtual double getStopBumper() const = 0;

   /**
    *  Sets the stop playback bumper value in the controller.
    *
    *  The stop playback bumper value is defined as the last frame value across all frames
    *  in all animations that will be played back when the playback bumpers are enabled.
    *
    *  @param  frameValue
    *          The stop bumper value.
    *
    *  @notify  This method will notify signalBumperStopChanged() with boost::any<double>.
    */
   virtual void setStopBumper(double frameValue) = 0;

   /**
    *  Sets the multiplier value associated with the value interval that is used when
    *  the controller is advanced.
    *
    *  This method provides the capability to effectively speed up or slow down the
    *  animations.  The interval is defined as the value that is set when the controller is
    *  advanced based on the frequency.  The multipler will increase or decrease the
    *  interval value used to set the current frame at the same frequency.
    *
    *  @param   multiplier
    *           The value to mutliply the interval by when advancing frames in each
    *           animation.  A value between 0.0 and 1.0 will effectively slow down the
    *           animations and a value greater than 1.0 will speed up the animations.  This
    *           method does nothing if the value is less than or equal to 0.0.  The
    *           default multiplier is 1.0.
    *
    *  @notify  This method will notify signalIntervalMultiplierChanged() with any<double>.
    *
    *  @see     getFrequency()
    */
   virtual void setIntervalMultiplier(double multiplier) = 0;

   /**
    *  Returns the multiplier value associated with the value interval that is used
    *  when the controller is advanced.
    *
    *  @return  The interval multipler value.
    *
    *  @see     setIntervalMultiplier()
    */
   virtual double getIntervalMultiplier() const = 0;

   /**
    *  Returns the frequency of when the animations in the controller are updated.
    *
    *  @return  The frequency of how often animations are updated.
    *
    *  @see     setIntervalMultiplier()
    */
   virtual const int getFrequency() const = 0;
   
   /**
    *  Returns the suggested minimum frame rate.
    *
    *  This is the minimum frame rate needed to completely
    *  capture all frames accurately. This defaults to
    *  60fps which is currently the highest frame rate a
    *  AnimationController can display at 1x playback speed. If
    *  a lower minimum frame rate is possible, plug-ins may
    *  set the value with setMinimumFrameRate().
    *  Frame rates are represented as frames/seconds
    *
    *  @return The minimum suggested frame rate.
    */
   virtual boost::rational<int> getMinimumFrameRate() const = 0;
   
   /**
    *  Sets the suggested minimum frame rate.
    *
    *  This is the minimum frame rate needed to completely
    *  capture all frames accurately. This defaults to
    *  60fps which is currently the highest frame rate a
    *  AnimationController can display at 1x playback speed. If
    *  a lower minimum frame rate is possible, plug-ins
    *  should set this value.
    *  Frame rates are represented as frames/seconds
    *
    *  @param  frameRate
    *          The minimum suggested frame rate.
    */
   virtual void setMinimumFrameRate(boost::rational<int> frameRate) = 0;

   /**
    *  Starts or stops the controller.
    *
    *  The animation state applies to the controller so all animations are affected by the new
    *  state.
    *
    *  @param   state
    *           The new animation state for the controller.
    *
    *  @notify  This method will notify signalAnimationStateChanged() with any<AnimationState>.
    *
    *  @see     AnimationState, play(), pause(), stop()
    */
   virtual void setAnimationState(AnimationState state) = 0;

   /**
    *  Returns the current animation state of the controller.
    *
    *  @return  The current animation state.
    *
    *  @see     AnimationState
    */
   virtual AnimationState getAnimationState() const = 0;

   /**
    *  Sets the behavior of the controller after the stop frame value is set.
    *
    *  @param   cycle
    *           The new animation cycle for the controller.
    *
    *  @notify  This method will notify signalAnimationCycleChanged() with any<AnimationCycle>.
    *
    *  @see     AnimationCycle
    */
   virtual void setAnimationCycle(AnimationCycle cycle) = 0;

   /**
    *  Returns the current animation cycle of the controller.
    *
    *  @return  The current animation cycle.
    *
    *  @see     AnimationCycle
    */
   virtual AnimationCycle getAnimationCycle() const = 0;

   /**
    *  Sets the current frame to the starting frame value.
    *
    *  @see     moveToEnd(), setCurrentFrame()
    */
   virtual void moveToBeginning() = 0;

   /**
    *  Sets the current frame to the ending frame value.
    *
    *  @see     moveToBeginning(), setCurrentFrame()
    */
   virtual void moveToEnd() = 0;

   /**
    *  Starts playing animations.
    *
    *  If the controller was previously paused or stopped, the animations play
    *  in the forward direction.  If the controller was previously playing
    *  backward, it continues to do so.
    *
    *  @see     fastForward(), fastRewind()
    */
   virtual void play() = 0;
   
   /**
    *  Stops playing animations without changing the current frame.
    *
    *  @see     stop()
    */
   virtual void pause() = 0;

   /**
    *  Stops playing animations and changes the current frame.
    *
    *  After stopping the controller the current frame is changed to the
    *  starting frame if the previous animation state was
    *  AnimationState::PLAY_FORWARD.  If the previous animation state was
    *  AnimationState::PLAY_BACKWARD the current frame is changed to the ending
    *  frame.
    *
    *  @see     pause(), moveToBeginning(), moveToEnd()
    */
   virtual void stop() = 0;

   /**
    *  Sets the interval multiplier and plays in the forward direction.
    *
    *  @param   multiplier
    *           The desired interval multiplier.
    *
    *  @see     fastRewind(), setIntervalMultiplier()
    */
   virtual void fastForward(double multiplier) = 0;

   /**
    *  Sets the interval multiplier and plays in the backward direction.
    *
    *  @param   multiplier
    *           The desired interval multiplier.
    *
    *  @see     fastForward(), setIntervalMultiplier()
    */
   virtual void fastRewind(double multiplier) = 0;

   /**
    *  Changes the current frame to the next frame in the frame list.
    *
    *  The current frame is changed to the next frame in the frame list.
    *
    *  @see     fastForward(), fastRewind()
    */
   virtual void stepForward() = 0;

   /**
    *  Changes the current frame to the previous frame in the frame list.
    *
    *  The current frame is changed to the previous frame in the frame list.
    *
    *  @see     fastForward(), fastRewind()
    */
   virtual void stepBackward() = 0;

   /**
    *  Sets the controller to play the attached animations at without dropping frames.
    *
    *  Due to system limitations, frames may be dropped in order to achieve the desired
    *  play rate.  This method controls whether or not this is allowed.  This is true
    *  by default.
    *
    *  @param   drop
    *           True if frames are allowed to be dropped, false otherwise.
    */
   virtual void setCanDropFrames(bool drop) = 0;

   /**
    * Gets whether the controller is allowed to drop frames in order to play at 
    * the desired rate.
    *
    * @return True if the play may drop frames, false otherwise.
    *
    * @see setCanDropFrames()
    */
   virtual bool getCanDropFrames() const = 0;


protected:
   /**
    * This should be destroyed by calling AnimationToolbar::destroyAnimationController.
    */
   virtual ~AnimationController() {}
};

#endif
