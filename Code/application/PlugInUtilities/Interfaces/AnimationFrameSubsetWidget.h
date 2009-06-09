/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ANIMATIONFRAMESUBSETWIDGET_H
#define ANIMATIONFRAMESUBSETWIDGET_H

#include <QtGui/QWidget>

#include "AnimationController.h"
#include "AnimationFrame.h"
#include "TypesFile.h"

class AnimationFrameSpinBox;
class QDateTimeEdit;
class QLabel;
class QSlider;

/**
 *  A widget used to set a frame subset.
 *
 *  The frame subset widget will allow users to set a subset between two frames
 *  in an animation. This widget will support both frame-based and time-based animations.
 *  The widget will be made up of two controls, each with its own slider and custom spin box. 
 *  One control will represent the start frame while the other will handle the stop frame.
 */
class AnimationFrameSubsetWidget : public QWidget
{
   Q_OBJECT

public:
   /**
    *  Creates a frame subset widget.
    *
    *  @param   pParent
    *           The parent widget.
    */
   AnimationFrameSubsetWidget(QWidget* pParent);

   /**
    *  Destroys the frame subset widget and all child frame subset items.
    */
   virtual ~AnimationFrameSubsetWidget();

   /**
    *  This method will set up the sliders and the spin boxes using the 
    *  data in the animation controller that is given.
    *
    *  @param   pController
    *           The animation controller containing the animation frames.
    */
   void setFrames(AnimationController* pController);

   /**
    *  This method will set up the sliders and the spin boxes using the 
    *  data in the animation that is given.
    *
    *  @param   pAnimation
    *           The animation used with the widget.
    */
   void setFrames(Animation* pAnimation);

   /**
    *  This method will set up the sliders and the spin boxes using the 
    *  vector of animation frames that is given.
    *
    *  @param   frames
    *           The vector containing the animation frames.
    *  @param   type
    *           The animation type.
    */
   void setFrames(const std::vector<AnimationFrame>& frames, FrameType type);

   /**
    *  Returns the start frame control value.
    *
    *  @return  The start frame value for an animation.
    */
   double getStartFrame() const;

   /**
    *  Sets the value for the start frame control.
    *
    *  @param   start
    *           The start frame in an animation.
    */
   void setStartFrame(double start);

   /**
    *  Returns the stop frame control value.
    *
    *  @return  the stop frame value for an animation.
    */
   double getStopFrame() const;

   /**
    *  Sets the value for the stop frame control.
    *
    *  @param   stop
    *           The stop frame in an animation.
    */
   void setStopFrame(double stop);

   /**
    *  Returns the type of animation.
    *
    *  @return  The animation type.
    */
   FrameType getFrameType() const;

private slots:
   /**
    *  This method will update the start frame control when
    *  its slider moved.
    *
    *  @param   value
    *           The new value for the start frame control.
    */
   void startSliderMoved(int value);

   /**
    *  This method will update the stop frame control when
    *  its slider moved.
    *
    *  @param   value
    *           The new value for the stop frame control.
    */
   void stopSliderMoved(int value);

   /**
    *  This method will update the start frame control slider when
    *  the value in its spin box is changed.
    *
    *  @param   frame
    *           The frame containing the new value.
    */
   void updateStartSlider(const AnimationFrame& frame);

   /**
    *  This method will update the stop frame control slider when
    *  the value in its spin box is changed.
    *
    *  @param   frame
    *           The frame containing the new value.
    */
   void updateStopSlider(const AnimationFrame& frame);

private:
   /**
    *  Sets the type of animation.
    *
    *  @param  type
    *          The animation type.
    */
   void setFrameType(FrameType type);

   QLabel* mpStartLabel;
   QSlider* mpStartSlider;
   AnimationFrameSpinBox* mpStartSpin;
   QLabel* mpStopLabel;
   QSlider* mpStopSlider;
   AnimationFrameSpinBox* mpStopSpin;
   FrameType mType;
};

#endif
