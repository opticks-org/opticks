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

#include "TypesFile.h"
#include "MessageLog.h"
#include "Service.h"
#include "Subject.h"

#include <string>
#include <vector>

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
 *  - A new MessageLog is implicitly created by asking for a previously non-existing
 *    log. 
 *  - The %MessageLogMgr is destroyed. This is send before the MessageLogs are deleted
 *    allowing observers to access the MessageLogs during cleanup.
 *  - Everything else documented in Subject.
 *
 *  @see     MessageReportingLevel
 */
class MessageLogMgr : public Subject
{
public:
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
    *  Gets a handle to the specified log.
    *
    *  @param   logName
    *           The name of the desired log.
    *
    *  @return A pointer to the specified log.
    */
   virtual MessageLog *getLog(const std::string &logName) = 0;

   /**
    *  Gets a handle to log associated with the current session.
    *
    *  @return A pointer to the current log.
    */
   virtual MessageLog *getLog() = 0;

   /**
    *  Gets handles for all managed logs.
    *
    *  @return  A vector of pointers to all managed logs.
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
