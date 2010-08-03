/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SYSTEMSERVICES_H
#define SYSTEMSERVICES_H

#include <string>

/**
 *  Provides access to system dependant services like the syslog/eventlog
 */
class SystemServices
{
public:
   /**
    *  Write an message to the syslog/eventlog
    *  The Info, Warning, and Error varients write
    *  messages marked with the specified severity level
    *
    *  @param   message
    *           The message to write to the log
    */
   virtual void WriteLogInfo(std::string message) = 0;
   virtual void WriteLogWarning(std::string message) = 0;
   virtual void WriteLogError(std::string message) = 0;
};

#endif
