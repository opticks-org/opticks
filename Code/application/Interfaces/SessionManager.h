/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SESSION_MANAGER
#define SESSION_MANAGER

#include "ConfigurationSettings.h"
#include "EnumWrapper.h"
#include "Service.h"
#include "Subject.h"

#include <string>
#include <vector>

class Progress;
class SessionItem;
class SessionSaveLock;

/**
 *  \ingroup ServiceModule
 *  Provides interfaces for saving and restoring the session and for creating
 *  a new session.
 *
 *  The session-save process is as follows:
 *  1) The session manager retrieves a pointer to each SessionItem, including:
 *    a) Data Elements
 *    b) Windows
 *    c) Views
 *    d) Layers
 *    e) PlotSets
 *    f) Plot Widgets
 *    g) Animation Controllers
 *    i) Animations
 *    j) Modules
 *    k) PlugIn Descriptors
 *    l) PlugIn instances
 *  2) The session directory is created, if necessary
 *  3) The session directory is cleared of obsolete SessionItem files
 *  4) For each SessionItem, it creates a SessionItemSerializer and calls SessionItem::serialize()
 *  5) The Session index file is written.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The session is closed.
 *  - Everything else documented in Subject.
 *
 *  @see        SessionItem, SessionItemSerializer
 */
class SessionManager : public Subject
{
public:
   /**
    *  Specifies the success of an attempt to serialize the session.
    */
   enum SerializationStatusEnum
   { 
      SUCCESS,  /**< All session items were successfully saved */
      PARTIAL_SUCCESS, /**< Some session items were successfully saved */
      LOCKED, /**< Session saving has been temporarily locked */
      FAILURE /**< No session items were successfully saved */
   };

   /**
    * @EnumWrapper SessionManager::SerializationStatusEnum.
    */
   typedef EnumWrapper<SerializationStatusEnum> SerializationStatus;

   SETTING(QueryForSave, SessionManager, SessionSaveType, SESSION_QUERY_SAVE)
   SETTING(AutoSaveEnabled, SessionManager, bool, false)
   SETTING(AutoSaveInterval, SessionManager, unsigned int, 30)

   /**
    *  Emitted with a null boost::any just prior to saving a session.
    */
   SIGNAL_METHOD(SessionManager, AboutToSaveSession)

   /**
    *  Emitted with a null boost::any just prior to restoring a session.
    */
   SIGNAL_METHOD(SessionManager, AboutToRestore)

   /**
    *  Emitted with a null boost::any when the session has finished restoring itself.
    */
   SIGNAL_METHOD(SessionManager, SessionRestored)

   /**
    *  Emitted after the session save has finished with a boost::any<std::pair<\link SessionManager::SerializationStatus SerializationStatus\endlink, std::string>>
    *  containing the status of the serialization and the filename of the saved session. If the status
    *  is \link SessionManager::FAILURE FAILURE\endlink, the filename string will be empty.
    */
   SIGNAL_METHOD(SessionManager, SessionSaved)

   /**
    *  Emitted when a session is closed.
    *
    *  The session is still fully created when this signal is emitted.
    */
   SIGNAL_METHOD(SessionManager, Closed)

   /**
    * Gets the name of the current session.
    *
    * This method returns the unique name given to the session when it was
    * created. This will be in the form of a UUID.
    *
    * @return   The name of the session.
    */
   virtual std::string getName() const = 0;

   /**
    * Saves the current session.
    *
    * This method saves all data, windows, layers, animations and active 
    * plug-ins to disk so they can be restored to the same state later.
    * 
    * @param filename
    *            The full pathname of the file the session is to be saved in.
    * @param pProgress
    *            If not \c NULL, progress during session save will be reported
    *            to the supplied Progress object.
    *
    * @return   A pair of values indicating the success of the save and, if
    *          the session was only partially saved, which session items
    *          failed to save. The std::string in the std::pair contains
    *          a user readable type of the associated SessionItem. This type
    *          should only be used for display to the user and logging.
    */
   virtual std::pair<SerializationStatus,std::vector<std::pair<SessionItem*, std::string> > > serialize(const std::string &filename, Progress *pProgress) = 0;

   /**
    *  Retrieves a SessionItem during session restore based on its id.
    *
    *  This method retrieves a specified session item during restore. It is only
    *  guaranteed to work during session restore.
    *
    *  @param id
    *            The session id of the SessionItem to retrieve
    *
    *  @return The pointer to the corresponding SessionItem, if found or \c NULL otherwise.
    */
   virtual SessionItem *getSessionItem(const std::string &id) = 0;

   /**
    *  Is the session currently saving?
    *
    *  @return True if session serialization is in progress, false if it is not.
    */
   virtual bool isSessionSaving() const = 0;

   /**
    *  Is the session currently loading?
    *
    *  @return True if session deserialization is in progress, false if it is not.
    */
   virtual bool isSessionLoading() const = 0;

   /**
    *  Is session save locked?
    *
    *  If this returns true, calls to serialize() will fail.
    *
    *  @return True if session save is currently locked, false if it is allowed.
    */
   virtual bool isSessionSaveLocked() const = 0;

protected:
   /**
    * This will be cleaned up during application close.  Plug-ins do not
    * need to destroy it.
    */
   virtual ~SessionManager() {}

   /**
    *  Prevent the session from saving.
    *
    *  Locking session save will cause calls to serialize() to fail.
    *  @see SessionSaveLock to use this feature.
    */
   virtual void lockSessionSave() = 0;

   /**
    *  Unlock session saving.
    *
    *  This reverses a previous call to lockSessionSave(). If multiple calls to
    *  lockSessionSave() have been made, this unlocks one of those calls.
    *  @see SessionSaveLock to use this feature.
    */
   virtual void unlockSessionSave() = 0;
   friend class SessionSaveLock;
};

#endif
