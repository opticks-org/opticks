/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ConfigurationSettings.h"
#include "Filename.h"
#include "MessageLogAdapter.h"
#include "MessageLogMgrImp.h"
#include "SessionManager.h"

#include <QtCore/QTemporaryFile>

using namespace std;

MessageLogMgrImp* MessageLogMgrImp::spInstance = NULL;
bool MessageLogMgrImp::mDestroyed = false;

MessageLogMgrImp::MessageLogMgrImp() :
   mpJournal(NULL)
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

   std::string journalFilename = (mLogPath + "/jour");

   // We don't want to use a QTemporaryFile directly, as with Qt5 QTemporaryFiles won't be
   // reliably written to disk. But we do want a unique filename, so we open a QTemporaryFile
   // just long enough to get a unique filename, close it, then immediately open a QFile or
   // QSaveFile with that name.
   QString tmpFilename;
   {
       QTemporaryFile tmpFile(QString::fromStdString(journalFilename));
       tmpFile.open();
       tmpFilename = tmpFile.fileName();
   }

#if HAVE_QSAVEFILE
   mpJournal = new QSaveFile(tmpFilename);
   QIODevice::OpenMode openMode(QIODevice::WriteOnly);  // QSaveFile::open() does not recognize QIODevice::Append
#else
   mpJournal = new QFile(tmpFilename);
   QIODevice::OpenMode openMode(QIODevice::WriteOnly | QIODevice::Append); // Why Append mode? This filename is unique.
#endif

   if ((mpJournal == NULL) || !mpJournal->open(openMode))
   {
         string msg("Unable to open journal file ");
         msg += mpJournal->fileName().toStdString();
         msg += " for writing ";
         // As logFile hasn't been created yet either, write message to stderr
         std::cerr << msg << std::endl;
   }
   else
   {
       mpJournal->setPermissions(QFileDevice::ReadUser | QFileDevice::WriteUser);
   }
   // Create a default session log
   createLog(Service<SessionManager>()->getName());
}

// This should be unnecessary, as at this point the MessageLogs should already be finalized
void MessageLogMgrImp::finalize()
{
    for( std::map<std::string, MessageLog*>::iterator liter=mLogMap.begin(); liter!= mLogMap.end(); ++liter )
    {
        const std::string & logName = liter->first;
        MessageLog* pLog = liter->second;
        if(pLog)
        {
            for( std::vector<Message*>::iterator miter=pLog->begin(); miter!=pLog->end(); ++miter)
            {
                Message* pMsg = *miter;
                if( pMsg && !pMsg->isFinalized() )
                {
                    pMsg->finalize();
                }
            }
        }
    }
}

MessageLogMgrImp::~MessageLogMgrImp()
{
   finalize();
   notify(SIGNAL_NAME(Subject, Deleted));
   clear();

#if HAVE_QSAVEFILE
   mpJournal->commit();
   // If you really want to treat the journal file as a temporary,
   // you may unlink it here. But then the only time you might ever see the
   // journal is when the Opticks application is actually running --
   // and then only if it has been committed. For now we'll
   // leave it on disk, and see what useful gets written.
   // unlink(mpJournal->fileName().toStdString().c_str());
#else
   mpJournal->close();
//   mpJournal->remove();
#endif
   delete mpJournal;
   mpJournal = NULL;
}

MessageLogMgrImp* MessageLogMgrImp::instance()
{
   if (spInstance == NULL)
   {
      if (mDestroyed)
      {
         throw std::logic_error("Attempting to use MessageLogMgr after destroying it.");
      }

      spInstance = new MessageLogMgrImp;
   }

   return spInstance;
}

void MessageLogMgrImp::destroy()
{
   if (mDestroyed)
   {
      throw std::logic_error("Attempting to destroy MessageLogMgr after destroying it.");
   }

   delete spInstance;
   spInstance = NULL;
   mDestroyed = true;
}

void MessageLogMgrImp::setPath(const string& path)
{
   if ((path.empty() == false) && (path != mLogPath))
   {
      mLogPath = path;
      notify(SIGNAL_NAME(MessageLogMgr, LogPathChanged), mLogPath);
   }
}

MessageLog* MessageLogMgrImp::createLog(const string& logName)
{
   if (getLog(logName) != NULL)
   {
      return NULL;
   }

   MessageLog* pLog = new MessageLogAdapter(logName.c_str(), mLogPath.c_str(), mpJournal);
   mLogMap.insert(pair<string, MessageLog*>(logName, pLog));
   notify(SIGNAL_NAME(MessageLogMgr, LogAdded), pLog);

   return pLog;
}

MessageLog* MessageLogMgrImp::getLog(const string& logName) const
{
   if (logName.empty() == true)
   {
      return NULL;
   }

   map<string, MessageLog*>::const_iterator iter = mLogMap.find(logName);
   if (iter != mLogMap.end())
   {
      return iter->second;
   }

   return NULL;
}

MessageLog* MessageLogMgrImp::getLog() const
{
   return getLog(Service<SessionManager>()->getName());
}

void MessageLogMgrImp::clear()
{
   while (mLogMap.empty() == false)
   {
      map<string, MessageLog*>::iterator iter = mLogMap.begin();
      MessageLog* pLog = iter->second;
      mLogMap.erase(iter);
      notify(SIGNAL_NAME(MessageLogMgr, LogRemoved), pLog);
      delete dynamic_cast<MessageLogAdapter*>(pLog);
   }
}

vector<MessageLog*> MessageLogMgrImp::getLogs() const
{
   vector<MessageLog*> logs;
   for (map<string, MessageLog*>::const_iterator iter = mLogMap.begin(); iter != mLogMap.end(); ++iter)
   {
      MessageLog* pLog = iter->second;
      if (pLog != NULL)
      {
         logs.push_back(pLog);
      }
   }

   return logs;
}

const string& MessageLogMgrImp::getObjectType() const
{
   static string sType("MessageLogMgrImp");
   return sType;
}

bool MessageLogMgrImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "MessageLogMgr"))
   {
      return true;
   }

   return false;
}
