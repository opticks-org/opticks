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
 * This class provides access to the display properties for a FrameLabelObject.
 *
 * Possible GraphicObjectTypes: GraphicObjectType::FRAME_LABEL_OBJECT.
 *
 * FrameLabelObjects have two modes of operation: Automatic and Manual.
 *
 * Automatic Mode is the default mode for a FrameLabelObject which has been added to a view by the user.
 * When the FrameLabelObject is operating in Automatic Mode it maintains a list of Animation objects which is comprised of:
 *    - All Animation objects in the AnimationController set into the View (ProductView)
 *    - All Animation objects in any RasterLayer in the View (SpatialDataView)
 *
 * The FrameLabelObject will automatically update its list of Animation objects when:
 *    - The object is placed in a different Layer
 *    - An Animation in the list is deleted
 *    - The underlying Layer is put into a different View
 *    - The View changes its AnimationController (ProductView)
 *    - An Animation is added to or removed from the AnimationController (ProductView)
 *    - The AnimationController is deleted (ProductView)
 *    - A Layer is added to or removed from the LayerList (SpatialDataView)
 *    - A RasterLayer in the LayerList changes its Animation (SpatialDataView)
 *
 * Manual Mode is the default mode for a FrameLabelObject which has been created programmatically.
 * When the FrameLabelObject is operating in Manual Mode it will only update its list of Animation objects when
 * an Animation in the list is deleted.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - All notifications documented in TextObject
 */
class FrameLabelObject : public TextObject
{
public:
   /**
    * When multiple Animation objects are present, this setting determines how text is displayed.
    *
    * If this setting is \b true, then the time (or frame number) of the least recent (oldest) Animation is displayed.
    * Otherwise, the time (or frame number) of the most recent (newest) Animation is displayed.
    */
   SETTING(DisplayMinimumFrame, FrameLabelObject, bool, false)

   /**
    *  Sets the Animation objects for the object to monitor.
    *  If the object is in Automatic Mode, this method does nothing.
    *
    *  @param   animations
    *           Animation objects to monitor.
    *
    *  @see setAutoMode(), getAutoMode()
    */
   virtual void setAnimations(const std::vector<Animation*> &animations) = 0;

   /**
    *  Gets the Animation objects being monitored by the object.
    *
    *  @return  Animation objects being monitored.
    */
   virtual const std::vector<Animation*> &getAnimations() const = 0;

   /**
    *  Inserts an Animation into the vector of Animation objects being monitored.
    *  If the object is in Automatic Mode, this method does nothing.
    *
    *  @param   pAnimation
    *           A pointer to an Animation to insert into the vector.
    *
    *  @see setAutoMode(), getAutoMode()
    */
   virtual void insertAnimation(Animation* pAnimation) = 0;

   /**
    *  Erases an Animation from the vector of Animation objects being monitored.
    *  If the object is in Automatic Mode, this method does nothing.
    *
    *  @param   pAnimation
    *           A pointer to an Animation to erase from the vector.
    *
    *  @see setAutoMode(), getAutoMode()
    */
   virtual void eraseAnimation(Animation* pAnimation) = 0;

   /**
    *  Sets the current mode.
    *  If autoMode matches the current mode, this method does nothing.
    *
    *  @param   autoMode
    *           A value of \b true will clear all current Animation objects and operate in Automatic Mode.
    *           A value of \b false will clear all current Animation objects and operate in Manual Mode.
    *
    *  @see getAutoMode()
    */
   virtual void setAutoMode(bool autoMode) = 0;

   /**
    *  Gets the current mode.
    *
    *  @return  \b True if the object is operating in Automatic Mode, \b false otherwise.
    *
    *  @see setAutoMode()
    */
   virtual bool getAutoMode() const = 0;

protected:
   /**
    * This should be destroyed by calling GraphicLayer::removeObject.
    */
   virtual ~FrameLabelObject() {}
};

#endif
