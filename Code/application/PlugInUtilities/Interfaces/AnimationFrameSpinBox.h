/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ANIMATIONFRAMESPINBOX_H
#define ANIMATIONFRAMESPINBOX_H

#include <vector>

#include <QtGui/QAbstractSpinBox>

#include "TypesFile.h"

class Animation;
class AnimationController;
class AnimationFrame;

/**
 *  A spin box that will allow a user to select a frame of an animation.
 *
 *  There are two types of animations: frame-based that uses frame values
 *  to go though the frames and time-based that go though frames by using the number of
 *  seconds since January 1, 1970. This spin box will handle both of these types.
 */
class AnimationFrameSpinBox : public QAbstractSpinBox
{
   Q_OBJECT

public:
   /**
    *  Creates a new animation spin box.
    *
    *  @param   pParent
    *           The parent widget.
    */
   AnimationFrameSpinBox(QWidget* pParent);

   /**
    *  Destroys the spin box.
    */
   virtual ~AnimationFrameSpinBox();

   /**
    *  This method will set up the vector of animation frames that will be 
    *  used in the spin box.
    *
    *  @param   frames
    *           the animation frame vector.
    *  @param   type
    *           The animation type.
    */
   void setFrames(const std::vector<AnimationFrame>& frames, FrameType type);

   /**
    *  This method will set up the vector of animation frames that will be 
    *  used in the spin box.
    *
    *  @param   pController
    *           the controller that holds the animation frames.
    */
   void setFrames(AnimationController* pController);

   /**
    *  This method will set up the vector of animation frames that will be 
    *  used in the spin box.
    *
    *  @param   pAnimation
    *           A set of animation frames.
    */
   void setFrames(Animation* pAnimation);

   /**
    *  Returns a vector containing the spin box frames.
    *
    *  @return  The spin box frame vector.
    */
   const std::vector<AnimationFrame>& getFrames() const;

   /**
    *  This method will make the spin box select the given animation
    *  frame if it's in the spin box.
    *
    *  @param   frame
    *           The animation frame to set in the spin box.
    */
   void setCurrentFrame(const AnimationFrame& frame);

   /**
    *  This method will get the animation frame that is currently selected
    *  in the spin box.
    *
    *  @return  The selected animation frame in the spin box.
    */
   const AnimationFrame& getCurrentFrame() const;

   /**
    *  Updates the spin box text if the user triggers a step. 
    *
    *  @param   steps
    *           The number of steps.
    */
   virtual void stepBy(int steps);

   /**
    *  Returns the type of animation.
    *
    *  @return  The animation type.
    */
   FrameType getFrameType() const;

protected:
    /**
    *  This method will determine whether the spin box can step up or step down. 
    *
    *  @return  The spin box step state.
    */
   virtual StepEnabled stepEnabled() const;

   /**
    *  This method will use the given index to get a frame from the animation frame vector
    *  and convert the frame value to text.
    *
    *  If there are no frames in the spin box, this method will return an empty string.
    *
    *  @param   index
    *           The frame index used to convert the frame to text.
    *
    *  @return  The string containing the frame value.
    */
   virtual QString convertToText(int index);

signals:
   /**
    *  Called when the spin box text has been changed.
    *
    *  @param   frame
    *           The new selected frame in the spin box.
    */
   void frameChanged(const AnimationFrame& frame);
   
private:
   AnimationFrameSpinBox(const AnimationFrameSpinBox& rhs);
   AnimationFrameSpinBox& operator=(const AnimationFrameSpinBox& rhs);
   FrameType mType;
   std::vector<AnimationFrame> mFrames;
   int mIndex;
};

#endif
