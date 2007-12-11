/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"
#include "MessageLogMgrImp.h"
#include "ConfigurationSettingsImp.h"
#include "DateTimeImp.h"
#include "MessageLogAdapter.h"
#include "SessionManager.h"

#include "QtCore/QTemporaryFile"

using namespace std;

#define BUFFER_SIZE 8096

MessageLogMgrImp* MessageLogMgrImp::spInstance = NULL;

MessageLogMgrImp::MessageLogMgrImp() : mLogPath()
{
   const Filename* pMessageLogPath = ConfigurationSettings::getSettingMessageLogPath();
   if (pMessageLogPath != NULL)
   {
      mLogPath = pMessageLogPath->getFullPathAndName();
   }

   if (mLogPath.empty())
   {
      const Filename* pTempPath = ConfigurationSettings::getSettingTempPath();
      if (pTempPath != NULL)
      {
         mLogPath = pTempPath->getFullPathAndName();
      }
   }
   char *tmpname(tempnam(mLogPath.c_str(),"journ"));
   mpJournal = new QTemporaryFile(QString::fromStdString(mLogPath) + "/journ");
   mpJournal->open(QIODevice::WriteOnly);
   mpJournal->setPermissions(QFile::WriteOwner);
}

MessageLogMgrImp::~MessageLogMgrImp()
{
   notify(SIGNAL_NAME(Subject, Deleted));
   map<string, MessageLog*>::iterator it;
   for (it = mLogMap.begin(); it != mLogMap.end(); it++)
   {
      MessageLogAdapter* pLog(static_cast<MessageLogAdapter*>(it->second));
      delete pLog;
   }

   mpJournal->close();
   mpJournal->remove();
   delete mpJournal;
   mpJournal = NULL;
}

MessageLogMgrImp* MessageLogMgrImp::instance()
{
   if (spInstance == NULL)
   {
      if(mDestroyed)
      {
         throw std::logic_error("Attempting to use MessageLogMgr after "
            "destroying it.");
      }
      spInstance = new MessageLogMgrImp;
   }

   return spInstance;
}

void MessageLogMgrImp::destroy()
{
   if(mDestroyed)
   {
      throw std::logic_error("Attempting to destroy MessageLogMgr after "
         "destroying it.");
   }
   delete spInstance;
   spInstance = NULL;
   mDestroyed = true;
}

bool MessageLogMgrImp::mDestroyed = false;

void MessageLogMgrImp::setPath(const string& path)
{
   mLogPath = path;
}

MessageLog *MessageLogMgrImp::getLog(const string &logName)
{
   if(mLogMap.count(logName) == 0)
   {
      MessageLog* pLog(NULL);
      pLog = static_cast<MessageLog *>(new MessageLogAdapter(logName.c_str(), mLogPath.c_str(),mpJournal));
      mLogMap.insert(pair<string, MessageLog*>(logName, pLog));
      notify(SIGNAL_NAME(Subject, Modified));
   }
   return mLogMap[logName];
}

MessageLog *MessageLogMgrImp::getLog()
{
   Service<SessionManager> pSessionMgr;
   return getLog(pSessionMgr->getName());
}

vector<MessageLog*> MessageLogMgrImp::getLogs() const
{
   vector<MessageLog*> logs;

   map<string, MessageLog*>::const_iterator iter = mLogMap.begin();
   while (iter != mLogMap.end())
   {
      MessageLog* pLog = NULL;
      pLog = iter->second;
      if (pLog != NULL)
      {
         logs.push_back(pLog);
      }

      ++iter;
   }

   return logs;
}


const string& MessageLogMgrImp::getObjectType() const
{
   static string type("MessageLogMgrImp");
   return type;
}

bool MessageLogMgrImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "MessageLogMgr"))
   {
      return true;
   }

   return false;
}
