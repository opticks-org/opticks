/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ANIMATION_H
#define ANIMATION_H

#include "SessionItem.h"
#include "Subject.h"
#include "AnimationFrame.h"
#include "TypesFile.h"

#include <string>
#include <vector>

/**
 *  A collection of animation frame objects with a current frame.
 *
 *  A animation object provides the means by which other objects can be animated by
 *  providing a set of animation frames, one of which is identified as the current
 *  frame.  A animation by itself does not provide a lot of functionality but simply
 *  notifies attached objects when its current frame is changed.  The real work
 *  of a displaying a animation is therefore in those objects that attach to the
 *  animation object.
 *
 *  To create a useful animation, first create a class with a Slot method.  This class
 *  should contain the logic needed to prepare frames for display.  Next, create
 *  a vector of animation frames for times of interest to the animation.  Create the
 *  animation by calling AnimationController::createAnimation().  Attach the class to the
 *  animation.  Then in the class's Slot method, whenever signalFrameChanged() is the
 *  signal with a non-NULL value, this indicates that the current frame has
 *  changed.  any_cast the value to a AnimationFrame pointer and perform any
 *  specific updates based on the new frame values.
 *
 *  A animation is both created and destroyed from a animation controller.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *    - The object is deleted.
 *    - The following methods are called: setName(), setFrames(), and
 *      setCurrentFrame().
 *    - Other notifications documented in the Subject class.
 *
 *  @see     AnimationFrame, AnimationController
 */
class Animation : public SessionItem, public Subject
{
public:
   /**
    *  Emitted with boost::any<std::string> when an Animation is renamed.
    */
   SIGNAL_METHOD(Animation, Renamed)
   /**
    *  Emitted with boost::any<AnimationFrame*> when the current frame is changed.
    */
   SIGNAL_METHOD(Animation, FrameChanged)
   /**
    *  Emitted with boost::any<std::vector<AnimationFrame> > when the list of frames in the animation changes.
    */
   SIGNAL_METHOD(Animation, FramesChanged)

   /**
    *  Sets the animation name.
    *
    *  @param   name
    *           The new name for the animation.
    *
    *  @notify  This method will notify signalRenamed() with any<std::string>.
    */
   virtual void setName(const std::string& name) = 0;

   /**
    *  Returns the type of frame value that the animation uses to set current frame.
    *
    *  This method returns whether the animation operates on frame numbers or time
    *  values when setting the current frame from a data value.  The frame type
    *  is specified when the animation is created in AnimationController::createAnimation().
    *  Once the animation is created, the frame type cannot be changed.
    *
    *  @return  The animation's frame type.
    *
    *  @see     setCurrentFrame(), getStartValue(), getStopValue()
    */
   virtual FrameType getFrameType() const = 0;

   /**
    *  Sets the frame set that defines this animation.
    *
    *  This method sets a new frame set for the animation.  Once the frames have been
    *  set, they are sorted in ascending order according to the frame number or
    *  the time value.  The current frame is reset to the first frame in the
    *  internal sorted vector.
    *
    *  @param   frames
    *           The new frame set for the animation.
    *
    *  @notify  This method will notify signalFramesChanged() with any<std::vector<AnimationFrame> >.
    */
   virtual void setFrames(const std::vector<AnimationFrame>& frames) = 0;

   /**
    *  Returns the frame set that defines this animation.
    *
    *  @return  The sorted frame set for the animation.
    */
   virtual const std::vector<AnimationFrame>& getFrames() const = 0;

   /**
    *  Returns the total number of frames in the animation.
    *
    *  @return  The total number of frames.
    */
   virtual unsigned int getNumFrames() const = 0;

   /**
    *  Queries whether the animation contains a given frame.
    *
    *  This method does not check the contents of the given frame; it only checks
    *  if the pointer corresponds to the address of one of the internally stored
    *  frames.  To check specific contents of a frame, use AnimationFrame::operator==()
    *  instead.
    *
    *  @param   pFrame
    *           The frame to check for its existence in the animation.
    *
    *  @return  Returns true if the given frame is contained in the animation,
    *           otherwise returns false.
    */
   virtual bool hasFrame(const AnimationFrame* pFrame) const = 0;

   /**
    *  Sets the current frame in the animation.
    *
    *  This method is called primarily as a result of a currently playing animation controller.
    *  Calling it independently from the player will succeed and will have no effect on
    *  the player, but if the player's animation state is AnimationState::PLAY_FORWARD or
    *  AnimationState::PLAY_BACKWARD, this method will likely be called again soon by the
    *  animation controller.  Therefore, it is only potentially useful to call this method
    *  when the player is paused or stopped.
    *
    *  @param   pFrame
    *           The frame to set as the current frame.  The given frame must exist
    *           in the animation.
    *
    *  @notify  This method will notify signalFrameChanged() with any<const AnimationFrame*>.
    */
   virtual void setCurrentFrame(const AnimationFrame* pFrame) = 0;

   /**
    *  Sets the current frame in the animation.
    *
    *  This is a convenience method that allows a animation frame to be set as the current
    *  frame by specifying just the data value.  The method identifies the first frame
    *  in the animation that is greater than or equal to the given frame value.  The given frame
    *  value is compared with the internal frame values according to the animation's frame
    *  type to determine the frame to set as the current frame.  Once the correct frame
    *  has been identified, the overloaded setCurrentFrame(const AnimationFrame*) method is
    *  called with the that frame.
    *
    *  If the given frame value is greater than the animation's stop value, the overloaded
    *  setCurrentFrame(const AnimationFrame*) method is called with a NULL value.
    *
    *  This method is called primarily as a result of a currently playing animation controller.
    *  Calling it independently from the player will succeed and will have no effect on
    *  the player, but if the player's animation state is PLAY_FORWARD or PLAY_BACKWARD,
    *  this method will likely be called again soon by the animation controller.  Therefore, it
    *  is only potentially useful to call this method when the player is paused or
    *  stopped.
    *
    *  @param   frameValue
    *           The frame value that will be used to set the current frame.  The value
    *           should be of the same type as specified by getFrameType().
    *
    *  @notify  This method will notify signalFrameChanged() with any<const AnimationFrame*>.
    *
    *  @see     setCurrentFrame(const AnimationFrame*), getFrameType()
    */
   virtual void setCurrentFrame(double frameValue) = 0;

   /**
    *  Returns the current frame in the animation.
    *
    *  @return  The current frame.
    */
   virtual const AnimationFrame* getCurrentFrame() const = 0;

   /**
    *  Returns the minimum frame value of all frames in the animation.
    *
    *  @return  Returns the value of the first frame of the sorted frame set according
    *           to the type specified by getFrameType().
    */
   virtual double getStartValue() const = 0;

   /**
    *  Returns the maximum frame value of all frames in the animation.
    *
    *  @return  Returns the value of the last frame of the sorted frame set according to
    *           the type specified by getFrameType().
    */
   virtual double getStopValue() const = 0;

   /**
    * Get the value of the next frame.
    *
    * This method is used to find what the value will be for some offset away from the current frame.
    *
    * @param direction
    *        The direction to go. 
    *
    * @param offset
    *        The number of frames to offset. If \c direction is AnimationState::STOP or
    *        AnimationState::PAUSE_FORWARD or AnimationState::PAUSE_BACKWARD, 
    *        this value is ignored.
    *
    * @return The value of the frame asked for.  This method will return a 
    *         negative number if the given offset pushes outside of the animation's
    *         range, or there is an invalid parameter. If \c direction is 
    *         AnimationState::STOP, AnimationState::PAUSE_FORWARD or 
    *         AnimationState::PAUSE_BACKWARD, the value of the
    *         current frame is returned.
    */
   virtual double getNextFrameValue(AnimationState direction, 
      size_t offset = 1) const = 0;

protected:
   /**
    * This should be destroyed by calling AnimationController::destroyAnimation.
    */
   virtual ~Animation() {}
};

#endif
