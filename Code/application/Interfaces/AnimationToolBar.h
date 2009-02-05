/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */
#ifndef ANIMATIONTOOLBAR_H
#define ANIMATIONTOOLBAR_H

#include "ConfigurationSettings.h"
#include "ToolBar.h"
#include "TypesFile.h"

#include <string>
#include <vector>

class AnimationController;
class SpatialDataView;

/**
 *  Manages animation controllers and provides a user interface to play their animations.
 *
 *  The animation tool bar is a tool bar that displays all available animation controllers and
 *  widgets to play animations of a currently selected animation controller.  This class provides
 *  the capability to create, activate, and destroy animation controllers. A pointer to the
 *  animation toolbar can be obtained by calling DesktopServices::getWindow() using "Animation"
 *  as the window name and TOOLBAR as the window type.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *    - The tool bar is deleted.  This will only occur on application shut down.
 *    - The setAnimationController() method is called.
 *    - Other notifications documented in the ToolBar class.
 *
 *  @see     AnimationController, ToolBar
 */
class AnimationToolBar : public ToolBar
{
public:
   SETTING(FrameSpeeds, AnimationToolBar, std::vector<double>, std::vector<double>())
   /**
    *  Emitted with any<AnimationController*> when the current animation
    *  controller is changed.
    */
   SIGNAL_METHOD(AnimationToolBar, ControllerChanged)

   /**
    *  Set the current animation controller in the toolbar.
    *
    *  This method activates the given animation controller in the toolbar's
    *  user interface.  The controller execution widgets are updated and
    *  enabled accordingly.
    *
    *  @param   pController
    *           The animation controller to activate in the toolbar.  If
    *           \b NULL is passed in, no controller will be active and all
    *           controller execution widgets will be disabled.
    *
    *  @notify  This method will notify signalControllerChanged() with
    *           any<AnimationController*> if the given controller is
    *           different than the current controller.
    */
   virtual void setAnimationController(AnimationController* pController) = 0;

   /**
    *  Returns the current animation controller.
    *
    *  @return  A pointer to the current animation controller.
    */
   virtual AnimationController* getAnimationController() const = 0;

   /**
    * Hide or show the timestamp on the toolbar.
    *
    * A plugin may want to hide the timestamp if you are providing the same 
    * information in its own window.
    *
    * @param hideTimestamp
    *        true if the timestamp should be hidden, false otherwise.
    *
    * @see getHideTimestamp
    */
    virtual void setHideTimestamp(bool hideTimestamp) = 0;

   /**
    * Gets whether the toolbar's timestamp is hidden.
    *
    * A plugin may want to hide the timestamp if you are providing the same 
    * information in its own window.
    *
    * @return true if the timestamp is be hidden, false otherwise.
    *
    * @see setHideTimestamp
    */
    virtual bool getHideTimestamp() const = 0;

protected:
   /**
    * This will be cleaned up during application close.  Plug-ins do not
    * need to destroy it.
    */
   virtual ~AnimationToolBar() {}
};

#endif
