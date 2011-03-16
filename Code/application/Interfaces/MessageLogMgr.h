/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MESSAGELOGMGR_H
#define MESSAGELOGMGR_H

#include "Service.h"
#include "Subject.h"

#include <string>
#include <vector>

class MessageLog;

/**
 *  Manages reporting of status messages to a log file.
 *
 *  One technique of reporting messages to users is to generate a log file based on
 *  various processing events.  This capability exists in the form of message logs
 *  that are maintained by the MessageLogMgr.  %Message logs are created according to
 *  a unique name, and there is no limit to the number of message logs for a
 *  session.  All message logs are saved in a file on disk in a common message log
 *  directory set that is initially specified in the user options.
 *
 *  The MessageLogMgr class maintains the creation and management of each log and
 *  provides an interface to add a message to a single log.
 *
 *  A default log is automatically created for each user session.  All
 *  messages with the reporting level of SILENT are also written to the session
 *  log.  The session log has the filename "username_session.log", where "username"
 *  is the username of the person currently logged into the system.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The message log path is changed as a result of calling setPath().
 *  - A new MessageLog is created as a result of calling createLog().
 *  - A log is removed when a session is closed.
 *  - The MessageLogMgr is destroyed.  The notification occurs before the
 *    message logs are deleted allowing observers to access the logs during
 *    cleanup.
 *  - Everything else documented in Subject.
 *
 *  @see     MessageReportingLevel
 */
class MessageLogMgr : public Subject
{
public:
   /**
    *  Emitted when the message log path changes with boost::any<std::string>
    *  containing the new path.
    */
   SIGNAL_METHOD(MessageLogMgr, LogPathChanged)

   /**
    *  Emitted when a new log is created by calling createLog() with
    *  boost::any<\link MessageLog \endlink*> containing a pointer to the
    *  created log.
    */
   SIGNAL_METHOD(MessageLogMgr, LogAdded)

   /**
    *  Emitted when a log is removed while closing a session with
    *  boost::any<\link MessageLog \endlink*> containing a pointer to the
    *  removed log.
    */
   SIGNAL_METHOD(MessageLogMgr, LogRemoved)

   /**
    *  Sets a new path for message logs.
    *
    *  This method sets a new directory to which all message logs are written.  This
    *  should not normally be called, since the message log path is set by the user
    *  options.
    *
    *  @param   path
    *           The new message log path.
    */
   virtual void setPath(const std::string& path) = 0;

   /**
    *  Creates a new log with a given name.
    *
    *  @param   logName
    *           The name of the new log to create.
    *
    *  @return  A pointer to the created log.  A \c NULL pointer is returned
    *           if a log with the given name already exists.
    *
    *  @see     getLog(const std::string&) const, getLog(), getLogs()
    */
   virtual MessageLog* createLog(const std::string& logName) = 0;

   /**
    *  Gets a handle to the specified log.
    *
    *  @param   logName
    *           The name of the desired log.
    *
    *  @return  A pointer to the specified log.
    *
    *  @see     getLog(), getLogs(), createLog()
    */
   virtual MessageLog* getLog(const std::string& logName) const = 0;

   /**
    *  Gets a handle to log associated with the current session.
    *
    *  @return  A pointer to the current log.
    *
    *  @see     getLog(const std::string&) const, getLogs(), createLog()
    */
   virtual MessageLog* getLog() const = 0;

   /**
    *  Gets handles for all managed logs.
    *
    *  @return  A vector of pointers to all managed logs.
    *
    *  @see     getLog(const std::string&) const, getLog(), createLog()
    */
   virtual std::vector<MessageLog*> getLogs() const = 0;

protected:
   /**
    * This will be cleaned up during application close.  Plug-ins do not
    * need to destroy it.
    */
   virtual ~MessageLogMgr() {}
};

#endif
