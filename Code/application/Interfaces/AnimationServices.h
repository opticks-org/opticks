/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ANIMATIONSERVICES_H
#define ANIMATIONSERVICES_H

#include "Service.h"
#include "Subject.h"
#include "TypesFile.h"

#include <string>
#include <vector>

class AnimationController;

/**
 *  Manages animation controllers within the application.
 *
 *  This class provides the capability to create and destroy animation
 *  controllers.  A pointer to this class can be obtained by creating a
 *  Service<AnimationServices> instance.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The following methods are called: createAnimationController() and
 *    destroyAnimationController().
 *  - Other notifications documented in the Subject class.
 *
 *  @see     AnimationController
 */
class AnimationServices : public Subject
{
public:
   /**
    *  Emitted with any<AnimationController*> when an animation controller is
    *  created.
    */
   SIGNAL_METHOD(AnimationServices, ControllerCreated)

   /**
    *  Emitted with any<AnimationController*> when an animation controller is
    *  destroyed.
    */
   SIGNAL_METHOD(AnimationServices, ControllerDestroyed)

   /**
    *  Creates a new animation controller.
    *
    *  This method creates a new animation controller with the given name and
    *  frame type.  The controller name must be unique within the application.
    *
    *  @param   name
    *           The animation controller name.  This name cannot be empty and
    *           must be unique for all controllers in the application.
    *  @param   frameType
    *           The frame type for the animation controller.  Once the
    *           controller is created, the frame type cannot be changed.
    *
    *  @return  A pointer to the new animation controller.  \b NULL is returned
    *           if an animation controller of the given name already exists,
    *           regardless of the frame type.
    *
    *  @notify  This method will notify signalControllerCreated() with
    *           any<AnimationController*> if the controller is successfully
    *           created.
    */
   virtual AnimationController* createAnimationController(const std::string& name, FrameType frameType) = 0;

   /**
    *  Queries whether an animation controller with a given name exists.
    *
    *  @param   name
    *           The controller name.
    *
    *  @return  Returns \b true if an animation controller with the given name
    *           exists, otherwise returns \b false.
    */
   virtual bool hasAnimationController(const std::string& name) const = 0;

   /**
    *  Returns the animation controller with a given name.
    *
    *  @param   name
    *           The controller name.
    *
    *  @return  A pointer to the existing animation controller with the given
    *           name.  \b NULL is returned if no animation controller exists
    *           with the given name.
    */
   virtual AnimationController* getAnimationController(const std::string& name) const = 0;

   /**
    *  Retrieves all animation controllers.
    *
    *  @return  A vector of animation controller pointers.  The returned vector
    *           should not be modified.  To change the vector contents, call
    *           createAnimationController() and destroyAnimationController()
    *           instead.
    */
   virtual const std::vector<AnimationController*>& getAnimationControllers() const = 0;

   /**
    *  Returns the current number of animation controllers.
    *
    *  This is a convenience method that is identical to
    *  getAnimationControllers().size().
    *
    *  @return  The current number of animation controllers.
    */
   virtual unsigned int getNumAnimationControllers() const = 0;

   /**
    *  Sets a new name for an animation controller.
    *
    *  @param   pController
    *           The animation controller to rename.
    *  @param   newName
    *           The new name for the controller.  The name cannot be the same
    *           as the name of any existing controller.
    *
    *  @return  Returns \b true if the controller was successfully renamed.
    *           Returns \b false if \em newName is empty or if an existing
    *           controller already has this name.
    *
    *  @notify  This method will notify AnimationController::signalRenamed()
    *           with boost::any<std::string> if the name is successfully set on
    *           the controller.
    */
   virtual bool renameAnimationController(AnimationController* pController, const std::string& newName) = 0;

   /**
    *  Destroys an existing animation controller.
    *
    *  This method destroys the given animation controller and all of the
    *  animations that it contains.
    *
    *  @param   pController
    *           The animation controller to destroy.
    *
    *  @notify  This method will notify signalControllerDestroyed() with
    *           any<AnimationController*> after the animation controller has
    *           removed but before the it is deleted.
    *
    *  @see     Animation
    */
   virtual void destroyAnimationController(AnimationController* pController) = 0;

protected:
   /**
    *  This class will be destroyed during application close.  Plug-ins do not
    *  need to destroy it.
    */
   virtual ~AnimationServices() {}
};

#endif
