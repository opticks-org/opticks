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

#include <map>
#include <string>
#include <vector>

class MessageLog;
class QFile;

class MessageLogMgrImp : public MessageLogMgr, public SubjectImp
{
public:
   static MessageLogMgrImp* instance();
   static void destroy();

   virtual void setPath(const std::string& path);
   virtual MessageLog* createLog(const std::string& logName);
   virtual MessageLog* getLog(const std::string& logName) const;
   virtual MessageLog* getLog() const;
   virtual std::vector<MessageLog*> getLogs() const;

   void clear();

   virtual const std::string& getObjectType() const;
   virtual bool isKindOf(const std::string& className) const;

   SUBJECTADAPTER_METHODS(SubjectImp)

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

#endif
