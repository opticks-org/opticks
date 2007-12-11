/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef FRAMELABELOBJECT_H
#define FRAMELABELOBJECT_H

#include "ConfigurationSettings.h"
#include "TextObject.h"
#include <vector>

class Animation;

/**
 * This class provides access to the display properties for a frame label object.
 *
 * When a FrameLabelObject is added to a view it watches all animations associated with the view's animation controller.
 * In the special case that a FrameLabelObject is added to a SpatialDataView, it watches all animations associated
 * with the raster layers in the view controlled by the view's animation controller.
 *
 * Possible GraphicObjectTypes: GraphicObjectType::FRAME_LABEL_OBJECT.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - All notifications documented in TextObject.
 */
class FrameLabelObject : public TextObject
{
public:
   /**
    * When multiple animations are present, this setting determines how text is displayed.
    *
    * If this setting is \b true, then the time (or frame number) of the least recent (oldest) animation is displayed.
    * Otherwise, the time (or frame number) of the most recent (newest) animation is displayed.
    */
   SETTING(DisplayMinimumFrame, FrameLabelObject, bool, false)

   /**
    *  Sets the animations for the object to monitor.
    *  If the underlying AnimationController is modified, then all changes using this method will be lost.
    *
    *  @param   animations
    *           Animations to monitor.
    */
   virtual void setAnimations(const std::vector<Animation*> &animations) = 0;

   /**
    *  Gets the animations being monitored by the object.
    *
    *  @return  Animations being monitored.
    */
   virtual const std::vector<Animation*> &getAnimations() const = 0;

   /**
    *  Inserts an animation into the vector of animations being monitored.
    *  If the underlying AnimationController is modified, then all changes using this method will be lost.
    *
    *  @param   pAnimation
    *           A pointer to an animation to insert into the vector.
    */
   virtual void insertAnimation(Animation* pAnimation) = 0;

   /**
    *  Erases an animation from the vector of animations being monitored.
    *  If the underlying AnimationController is modified, then all changes using this method will be lost.
    *
    *  @param   pAnimation
    *           A pointer to an animation to erase from the vector.
    */
   virtual void eraseAnimation(Animation* pAnimation) = 0;

   /**
    *  Sets the autoMode value.
    *
    *  @param   autoMode
    *           A value of \b true will cause the FrameLabelObject to clear its current animations and
    *           automatically regenerate them based on the underlying view.
    *           A value of \b false will cause the FrameLabelObject to clear its current animations and
    *           set its current animation controller to \c NULL.
    */
   virtual void setAutoMode(bool autoMode) = 0;

   /**
    *  Gets the current autoMode value.
    *
    *  @return  The current autoMode setting.
    *
    */
   virtual bool getAutoMode() const = 0;

protected:
   /**
    * This should be destroyed by calling GraphicLayer::removeObject.
    */
   virtual ~FrameLabelObject() {}
};

#endif
