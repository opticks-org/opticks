/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef MESSAGELOGMGRIMP_H
#define MESSAGELOGMGRIMP_H

#include "MessageLogMgr.h"
#include "SubjectImp.h"

#include <string>
#include <map>
#include <vector>

class MessageLog;
class Message;
class Step;
class QFile;

class MessageLogMgrImp : public MessageLogMgr, public SubjectImp
{
public:
   static MessageLogMgrImp* instance();
   static void destroy();

   virtual void setPath(const std::string& path);
   virtual MessageLog *getLog(const std::string &logName);
   virtual MessageLog *getLog();
   virtual std::vector<MessageLog*> getLogs() const;

   virtual const std::string& getObjectType() const;
   virtual bool isKindOf(const std::string& className) const;

   bool attach(const std::string &signal, const Slot &slot)
   {
      return SubjectImp::attach(signal, slot);
   }
   bool detach(const std::string &signal, const Slot &slot)
   {
      return SubjectImp::detach(signal, slot);
   }

   void clear(); //should only be called after the current session has been closed, otherwise crashes will result.

protected:
   MessageLogMgrImp();
   virtual ~MessageLogMgrImp();

private:
   static MessageLogMgrImp* spInstance;
   static bool mDestroyed;

   std::map<std::string, MessageLog*> mLogMap;
   std::string mLogPath;
   QFile* mpJournal;
};

#endif   // MESSAGELOGMGRIMP_H
