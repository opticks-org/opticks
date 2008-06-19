/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PROGRESSRESOURCE_H__
#define PROGRESSRESOURCE_H__

#include "ApplicationServices.h"
#include "Progress.h"
#include "Resource.h"
#include "DesktopServices.h"
#include "UtilityServices.h"

#include <string>

/**
 * The ProgressObject is a trait object for use with the Resource template. 
 *
 * @see ProgressResource
 */
class ProgressObject
{
public:
   /**
   * This is an implementation detail of the ProgressObject class. 
   */
   struct Args
   {
      bool mThreadSafe;
      bool mCreateDialog;
      std::string mCaption;
      Progress *mpProgress;
      Args(bool threadSafe, bool createDialog, const std::string &caption) :
         mThreadSafe(threadSafe), mCreateDialog(createDialog), mCaption(caption), mpProgress(NULL) {}
      Args(Progress *pProgress, bool createDialog, const std::string &caption) :
         mThreadSafe(false), mCreateDialog(createDialog), mCaption(caption), mpProgress(pProgress) {}
   };

   Progress *obtainResource(const Args &args) const 
   {
      Progress *pProgress = args.mpProgress;
      if (pProgress == NULL)
      {
         pProgress = Service<UtilityServices>()->getProgress(args.mThreadSafe);
      }

      if (pProgress != NULL && args.mCreateDialog == true && Service<ApplicationServices>()->isInteractive() == true)
      {
         if (Service<DesktopServices>()->createProgressDialog(args.mCaption, pProgress) == false)
         {
            releaseResource(args, pProgress);
            pProgress = NULL;
         }
      }

      return pProgress;
   }

   void releaseResource(const Args &args, Progress *pProgress) const 
   {
      Service<UtilityServices>()->destroyProgress(pProgress);
   }
};

/**
 *  This is a Resource class that manages Progress objects.
 *
 *  This resource creates a Progress objects and destroys it
 *  when going out of scope. Optionally, a ProgressDialog will
 *  be created for the Progress object. The second form of this
 *  resource takes ownership of an existing Progress object.
*/
class ProgressResource : public Resource<Progress,ProgressObject>
{
public:
   /**
    *  Constructs a Resource object that manages a Progress object.
    *
    *  This creates a new Progress object and optionally associates
    *  a ProgressDialog with it.
    *
    *  @param   caption
    *           If this is not empty, a ProgressDialog will be
    *           created with this caption.
    *  @param   threadSafe
    *           If true, a thread-safe Progress object is created.
    */
   explicit ProgressResource(const std::string &caption = std::string(), bool threadSafe = false) :
         Resource<Progress,ProgressObject>(ProgressObject::Args(threadSafe,
            !caption.empty() && Service<ApplicationServices>()->isInteractive(), caption)) {}
   /**
    *  Constructs a Resource object that manages a Progress object.
    *
    *  This takes ownership of an existing Progress object and optionally associates
    *  a ProgressDialog with it.
    *
    *  @param   progress
    *           The exisiting Progress object.
    *  @param   caption
    *           If this is not empty, a ProgressDialog will be
    *           created with this caption.
    */
   explicit ProgressResource(Progress &progress, const std::string &caption = std::string()) :
         Resource<Progress,ProgressObject>(ProgressObject::Args(&progress,
            !caption.empty() && Service<ApplicationServices>()->isInteractive(), caption)) {}
};

#endif
