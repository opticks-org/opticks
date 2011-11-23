/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ANIMATIONFRAME_H
#define ANIMATIONFRAME_H

#include <string>

/**
 *  Stores information about a single frame in an animation.
 *
 *  An animation frame contains three values: name, frame number, and time.  The
 *  name is not used internally, but is provided so that objects can provide a
 *  meaningful way for the user to uniquely identify a frame.  The frame number
 *  is zero-based and should be unique to each frame in an animation.  The time
 *  value corresponds to either seconds after midnight on January 1, 1970 UTC or
 *  elapsed seconds after the start of the animation.  A double precision value
 *  is used to allow milliseconds to be specified as a fractional second.  A
 *  negative time value, which is the default state, indicates that time is not
 *  associated with the frame.
 *
 *  @see     Animation
 */
class AnimationFrame
{
public:
   /**
    *  Constructs a default empty animation frame.
    */
   AnimationFrame() :
      mName(""),
      mFrameNumber(0),
      mTime(-1.0)
   {}

   /**
    *  Constructs an animation frame with no associated time.
    *
    *  @param   name
    *           The frame name.
    *  @param   frameNumber
    *           The frame number.  %Any positive number can be used to create
    *           the frame, but the number should be unique for all frames in an
    *           animation.
    *
    *  @see     Animation
    */
   AnimationFrame(const std::string& name, unsigned int frameNumber) :
      mName(name),
      mFrameNumber(frameNumber),
      mTime(-1.0)
   {}

   /**
    *  Constructs an animation frame with an associated time value.
    *
    *  @param   name
    *           The frame name.
    *  @param   frameNumber
    *           The frame number.  %Any positive number can be used to create
    *           the frame, but the number should be unique for all frames in an
    *           animation.
    *  @param   time
    *           The frame time in seconds after midnight, January 1, 1970 UTC or
    *           elapsed seconds after the start of the animation.  A fractional
    *           second represents milliseconds.
    *
    *  @see     Animation
    */
   AnimationFrame(const std::string& name, unsigned int frameNumber, double time) :
      mName(name),
      mFrameNumber(frameNumber),
      mTime(time)
   {}

   /**
    *  Queries whether this frame has a lesser frame number or time than this
    *  frame.
    *
    *  @param   frame
    *           The animation frame to compare against this frame.
    *
    *  @return  If the time value of this frame and \e frame are both valid,
    *           returns \c true if the time value of this frame is less than the
    *           time value of \e frame.  If the time value of either this frame
    *           or \e frame is invalid (negative), returns \c true if the frame
    *           number of this frame is less than the frame number of
    *           \e frame.
    */
   bool operator< (const AnimationFrame& frame) const
   {
      if ((mTime >= 0.0) && (frame.mTime >= 0.0))
      {
         return (mTime < frame.mTime);
      }

      return (mFrameNumber < frame.mFrameNumber);
   }

   /**
    *  Queries whether this frame has a greater frame number or time than this
    *  frame.
    *
    *  @param   frame
    *           The animation frame to compare against this frame.
    *
    *  @return  If the time value of this frame and \e frame are both valid,
    *           returns \c true if the time value of this frame is greater than
    *           the time value of \e frame.  If the time value of either this
    *           frame or \e frame is invalid (negative), returns \c true if the
    *           frame number of this frame is greater than the frame number of
    *           \e frame.
    */
   bool operator> (const AnimationFrame& frame) const
   {
      if ((mTime >= 0.0) && (frame.mTime >= 0.0))
      {
         return (mTime > frame.mTime);
      }

      return (mFrameNumber > frame.mFrameNumber);
   }

   /**
    *  Queries whether an animation frame is identical to this frame.
    *
    *  @param   frame
    *           The animation frame to compare against this frame.
    *
    *  @return  Returns \c true if both the frame number and time of this frame
    *           are equal to those of \e frame.
    */
   bool operator== (const AnimationFrame& frame) const
   {
      return ((mFrameNumber == frame.mFrameNumber) && (mTime == frame.mTime));
   }

   /**
    *  Queries whether an animation frame is different from this frame.
    *
    *  @param   frame
    *           The animation frame to compare against this frame.
    *
    *  @return  Returns \c true if both the frame number and time of this frame
    *           are different than those of \e frame.
    */
   bool operator!= (const AnimationFrame& frame) const
   {
      return ((mFrameNumber != frame.mFrameNumber) || (mTime != frame.mTime));
   }

   /**
    *  The frame name.
    */
   std::string mName;

   /**
    *  The frame number.
    *
    *  The frame number is zero-based and each frame in an animation should have
    *  a unique number.
    *
    *  @see     Animation
    */
   unsigned int mFrameNumber;

   /**
    *  The frame time.
    *
    *  If the frame type is ::FRAME_TIME, the time is specified in seconds after
    *  midnight, January 1, 1970 UTC.  If the frame type is
    *  ::FRAME_ELAPSED_TIME, the time is specified in elapsed seconds after the
    *  start of the animation.  In both cases, a fractional second represents
    *  milliseconds.
    */
   double mTime;
};

#endif
