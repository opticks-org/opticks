/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SIMPLEANIMATION_H__
#define SIMPLEANIMATION_H__

#include "AppConfig.h"
#include "SpecialMetadata.h"

#define FRAME_TIMES_METADATA_NAME (std::string("FrameTimes"))
#define FRAME_TIMES_METADATA_PATH (SPECIAL_METADATA_NAME + "/" + BAND_METADATA_NAME + "/" + FRAME_TIMES_METADATA_NAME)

class AnimationController;
class Layer;

#ifdef __cplusplus
extern "C"
{
#endif
   /** \addtogroup simple_api */
   /*@{*/

   /**
    * @file SimpleAnimation.h
    * This file contains API utilities for controlling animations.
    */

   /**
    * Animation callback function prototype.
    *
    * Passed a pointer to the Animation which caused the callback to execute.
    * The name, number, and time for the frame are passed in but may be invalid depending
    * on the AnimationController type. (frame type does not have valid frame times).
    * A caller specified value is also passed.
    */
   typedef void(*animation_callback_t)(const char* pAnimation, const char* pFrameName,
                                       uint32_t frameNumber, double frameTime, void* pUser);

   /**
    * Get a handle to a named AnimationController.
    *
    * @param pName
    *        \c NULL terminated name of the animation controller. If this is \c NULL
    *        the active controller will be returned.
    * @return The AnimationController handle or \c NULL on error.
    */
   EXPORT_SYMBOL AnimationController* getAnimationController(const char* pName);

   /**
    * Create a new AnimationController.
    *
    * @param pName
    *        \c NULL terminated name for the animation controller.
    *        If this exists, an error will be set.
    * @param timeBased
    *        If non-zero, a time based controller is created (using real numbers for times).
    *        If zero, a frame based controller is created (using intergers for frame numbers).
    * @return The new AnimationController handle or \c NULL on error.
    */
   EXPORT_SYMBOL AnimationController* createAnimationController(const char* pName, int timeBased);

   /**
    * Permanently destroy an AnimationController.
    *
    * Suitable for use as a cleanup callback.
    *
    * @param pController
    *        The AnimationController to destroy.
    *        No error checking is performed on this value and a \c NULL will cause a NOOP.
    */
   EXPORT_SYMBOL void destroyAnimationController(AnimationController* pController);

   /**
    * Activate an AnimationController so that it is controlled by the animation toolbar.
    *
    * @param pController
    *        The AnimationController to activate.
    * @return Zero on success, non-zero on error.
    */
   EXPORT_SYMBOL int activateAnimationController(AnimationController* pController);

   /**
    * Get the state of the AnimationController.
    *
    * @param pController
    *        The AnimationController to query.
    * @return The state of the controller.
    *         0 -> Stopped, 1 -> Playing forward, 2 -> Playing backward,
    *         3 -> Paused and will play forward on resume, 4 -> Paused and will play backward on resume
    *         Zero may indicate an error and getLastError() should be checked.
    */
   EXPORT_SYMBOL uint32_t getAnimationControllerState(AnimationController* pController);

   /**
    * Set the state of the AnimationController.
    *
    * @param pController
    *        The AnimationController to mutate.
    * @param state
    *         The new state of the controller.
    *         0 -> Stop, 1 -> Play forward, 2 -> Play backward,
    *         3 -> Pause and play forward on resume, 4 -> Pause and will play backward on resume
    * @return Zero on success or non-zero on error.
    */
   EXPORT_SYMBOL int setAnimationControllerState(AnimationController* pController, uint32_t state);

   /**
    * Begin playback of an animation controller in the current direction.
    *
    * @param pController
    *        The AnimationController to mutate.
    * @return Zero on success or non-zero on error.
    */
   EXPORT_SYMBOL int playAnimationController(AnimationController* pController);

   /**
    * Pause playback of an animation controller.
    *
    * @param pController
    *        The AnimationController to mutate.
    * @return Zero on success or non-zero on error.
    */
   EXPORT_SYMBOL int pauseAnimationController(AnimationController* pController);

   /**
    * Stop playback of an animation controller.
    *
    * @param pController
    *        The AnimationController to mutate.
    * @return Zero on success or non-zero on error.
    */
   EXPORT_SYMBOL int stopAnimationController(AnimationController* pController);

   /**
    * Get the cycle mode of the AnimationController.
    *
    * @param pController
    *        The AnimationController to query.
    * @return The cycle mode of the controller.
    *         0 -> Play once and reset, 1 -> Repeat play, 2 -> Reverse direction at end,
    *         Zero may indicate an error and getLastError() should be checked.
    */
   EXPORT_SYMBOL uint32_t getAnimationControllerCycle(AnimationController* pController);

   /**
    * Set the cycle mode of the AnimationController.
    *
    * @param pController
    *        The AnimationController to mutate.
    * @param cycle
    *         The new cycle mode of the controller.
    *         0 -> Play once and reset, 1 -> Repeat play, 2 -> Reverse direction at end,
    * @return Zero on success or non-zero on error.
    */
   EXPORT_SYMBOL int setAnimationControllerCycle(AnimationController* pController, uint32_t cycle);

   /**
    * Will the AnimationController drop frames to maintain the play rate?
    *
    * @param pController
    *        The AnimationController to query.
    * @return Zero if the controller will slow playback instead of dropping frames.
    *         Non-zero if the controller will drop frames to maintain the playback rate.
    *         Zero may indicate an error and getLastError() should be checked.
    */
   EXPORT_SYMBOL int canAnimationControllerDropFrames(AnimationController* pController);

   /**
    * Allow the AnimationController to drop frames to maintain the play rate/
    *
    * @param pController
    *        The AnimationController to mutate.
    * @param dropFrames
    *         Zero if the controller should slow playback instead of dropping frames.
    *         Non-zero if the controller should drop frames to maintain the playback rate.
    * @return Zero on success or non-zero on error.
    */
   EXPORT_SYMBOL int setAnimationControllerCanDropFrames(AnimationController* pController, int dropFrames);

   /**
    * Get the interval multiplier of an AnimationController.
    *
    * @param pController
    *        The AnimationController to query.
    * @return The interval multiplier. This speeds up or slows down
    *         the playback in a linear fashion. For example: a value
    *         of 2.0 doubles the playback rate and a value of 0.5 halves
    *         the playback rate. A value of NaN indicates an error.
    */
   EXPORT_SYMBOL double getAnimationControllerIntervalMultiplier(AnimationController* pController);

   /**
    * Set the interval multiplier of an AnimationController.
    *
    * @param pController
    *        The AnimationController to query.
    * @param multiplier
    *         The interval multiplier. This speeds up or slows down
    *         the playback in a linear fashion. For example: a value
    *         of 2.0 doubles the playback rate and a value of 0.5 halves
    *         the playback rate.
    * @return Zero on success or non-zero on error.
    */
   EXPORT_SYMBOL int setAnimationControllerIntervalMultiplier(AnimationController* pController, double multiplier);

   /**
    * Attach a RasterLayer to an AnimationController.
    *
    * If the controller if frame based, this will attach frames on a one-to-one basis.
    * If the controller is time based, this will attach frames starting at the AnimationController's start time
    * (or current system time if there are no frames in the controller) with a frame rate of 1fps.
    * If the RasterElement in the RasterLayer has a metadata entry in "Special/Band/FrameTimes"
    * which is a vector<double> with a frame time for each frame in the RasterElement, those frame
    * times will be used.
    *
    * The name of the Animation which attaches the RaseterLayer will be the same as the Layer.
    * If an attachment with this name exists already, an error will occur.
    *
    * @param pController
    *        The AnimationController to mutate.
    * @param pLayer
    *        The RasterLayer to attach.
    * @return Zero on success or non-zero on error.
    */
   EXPORT_SYMBOL int attachRasterLayerToAnimationController(AnimationController* pController, Layer* pLayer);

   /**
    * Attach a callback function to an AnimationController.
    *
    * @param pController
    *        The AnimationController to mutate.
    * @param pName
    *        The \c NULL terminate name of the Animation which attaches the callback.
    *        If an attachment with this name exists already, an error will occur.
    * @param pCallback
    *        The callback function.
    * @param pUser
    *        Handle to pass to the pUser field in the callback. This may be \c NULL.
    * @param frameCount
    *        The number of frames which will be animated.
    * @param pFrameTimes
    *        If \c pController is a frame based AnimationController, this should be \c NULL.
    *        If \c pController is a time based AnimationController, this is an array of length \c frameCount
    *        with the frame times or if \c NULL, the start time (or current system time) will be user to generate
    *        \c frameCount times 1 second appart.
    * @return \c NULL on error or a callback handle. This must be passed to destroyAnimationControllerAttachment()
    *         to clean up and release the allocated memory.
    */
   EXPORT_SYMBOL void* attachCallbackToAnimationController(AnimationController* pController, const char* pName,
                                                           animation_callback_t pCallback, void* pUser,
                                                           uint32_t frameCount, double* pFrameTimes);

   /**
    * Permanently destroy an attachment point from an AnimationController.
    *
    * Suitable for use as a cleanup callback.
    *
    * @param pController
    *        The AnimationController to mutate.
    *        No error checking is performed on this value and a \c NULL will cause a NOOP.
    * @param pName
    *        The name of the Animation attachment point to destroy.
    *        No error checking is performed on this value and a \c NULL will cause a NOOP.
    * @param pHandler
    *        If the attachment was a callback attachment, this is the return value of
    *        attachCallbackToAnimationController(). If the attachment was not a
    *        callback attachment, this is \c NULL.
    */
   EXPORT_SYMBOL void destroyAnimationControllerAttachment(AnimationController* pController,
                                                           const char* pName, void* pHandler);

   /*@}*/
#ifdef __cplusplus
}
#endif

#endif
