/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */


#ifndef SESSIONRESOURCE_H__
#define SESSIONRESOURCE_H__

#include "SessionManager.h"

/**
 * Prevent saving of the session for a time.
 *
 * This prevents saving of the session (auto-save and user initiated save) while the SessionSaveLock exists.
 * This is used to temporarily stop the session from saving when it would be unwise to
 * save the session. Reference counting is used for locks. If two locks are in place, destruction of the first
 * SessionSaveLock will not unlock session saving, but will decrement the lock count.
 *
 * This is typically created at the beginning of a long, processor or
 * memory intensive procedure. Care should be used when locking session save as it overrides
 * the user's auto-save preference. An example of use follows.
 * @code
 * void processData()
 * {
 *    SessionSaveLock saveLock;
 *    // run the algorithm here
 * }
 * @endcode
 */
class SessionSaveLock
{
public:
   /**
    * Lock session saves.
    */
   SessionSaveLock()
   {
      Service<SessionManager>()->lockSessionSave();
   }

   /**
    * Unlock session saves.
    */
   ~SessionSaveLock()
   {
      Service<SessionManager>()->unlockSessionSave();
   }
};

#endif
