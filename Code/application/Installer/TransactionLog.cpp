/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "FileResource.h"
#include "InstallerServices.h"
#include "MessageLogMgr.h"
#include "Progress.h"
#include "ReferenceCountDatabase.h"
#include "TransactionLog.h"

#include <QtCore/QDataStream>
#include <QtCore/QFile>

#include <memory>

XERCES_CPP_NAMESPACE_USE

TransactionLog::TransactionLog(Progress* pProgress) :
   mNext(0),
   mpProgress(pProgress),
   mProgressCounter(0)
{
}

TransactionLog::~TransactionLog()
{
   mNext = 0;
   clearTransactions();
   Service<InstallerServices>()->getReferenceCountDatabase().saveDb();
}

bool TransactionLog::canModify() const
{
   return mNext == 0;
}

bool TransactionLog::clearTransactions()
{
   if (!canModify())
   {
      return false;
   }
   for (queue_type::iterator it = mTransactions.begin(); it != mTransactions.end(); ++it)
   {
      delete *it;
   }
   mTransactions.clear();
   mProgressCounter = 0;
   mNext = 0;
   return true;
}

bool TransactionLog::addTransaction(Transaction* pTransaction)
{
   if (!canModify())
   {
      return false;
   }
   mTransactions.push_back(pTransaction);
   mNext = 0;
   mProgressCounter = 0;
   return true;
}

bool TransactionLog::execute(const std::string& logFilename, const std::string& progressMessage)
{
   QFile walFile(QString::fromStdString(logFilename));
   if (walFile.exists())
   {
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress("Incomplete transaction log exists!", 0, ERRORS);
      }
      return false;
   }
   if (!walFile.open(QIODevice::WriteOnly))
   {
      return false;
   }
   QDataStream wal(&walFile);
   wal << QString("transactionLogV1");
   while (mNext != mTransactions.size())
   {
      try
      {
         if (mpProgress != NULL)
         {
            mpProgress->updateProgress(progressMessage, abs(mProgressCounter) * 100 / mTransactions.size(), NORMAL);
         }
         // write intent
         Transaction* pTrans = mTransactions[mNext];
         wal << QString::fromStdString(pTrans->getDestination());
         walFile.flush();

         // execute transaction
         pTrans->execute();
         mNext++;
         // indicate completion
         wal << true;
         walFile.flush();

         ++mProgressCounter;
      }
      catch (const TransactionFailure& exc)
      {
         if (mpProgress != NULL)
         {
            mpProgress->updateProgress(exc.what(), abs(mProgressCounter) * 100 / mTransactions.size(), ERRORS);
         }
         return false;
      }
   }
   walFile.close();
   if (mpProgress != NULL)
   {
      mpProgress->updateProgress(progressMessage, 100, NORMAL);
   }
   return true;
}

bool TransactionLog::rollback(const std::string& progressMessage)
{
   try
   {
      while (mNext != 0)
      {
         if (mpProgress != NULL)
         {
            mpProgress->updateProgress(progressMessage, abs(--mProgressCounter) * 100 / mTransactions.size(), NORMAL);
         }
         Transaction* pTrans = mTransactions[--mNext];
         pTrans->rollback();
      }
   }
   catch (const TransactionFailure& exc)
   {
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(exc.what(), abs(mProgressCounter) * 100 / mTransactions.size(), ERRORS);
      }
      return false;
   }
   if (mpProgress != NULL)
   {
      mpProgress->updateProgress(progressMessage, 100, NORMAL);
   }
   return true;
}

bool TransactionLog::deserialize(const std::string& filename)
{
   QFile tlogFile(QString::fromStdString(filename));
   if (!tlogFile.open(QIODevice::ReadOnly))
   {
      return false;
   }
   clearTransactions();

   QDataStream in(&tlogFile);
   QString magic;
   in >> magic;
   if (magic != "transactionLogV1")
   {
      return false;
   }

   Transaction* pLast = NULL;
   while (!in.atEnd())
   {
      QString destName;
      in >> destName;
      if (destName.isNull())
      {
         return false;
      }
      std::auto_ptr<Transaction> pTrans(new Transaction());
      pTrans->setDestination(destName.toStdString());
      mTransactions.push_back(pTrans.release());
      if (!in.atEnd())
      {
         bool complete = false;
         in >> complete;
         if (!complete)
         {
            pLast = pTrans.get();
         }
      }
   }
   mNext = mTransactions.empty() ? 0 : mTransactions.size();
   if (pLast != NULL)
   {
      for (std::vector<Transaction*>::size_type i = 0; i < mTransactions.size(); ++i)
      {
         if (mTransactions[i] == pLast)
         {
            mNext = i + 1;
            break;
         }
      }
   }
   return true;
}

void TransactionLog::setProgress(Progress* pProgress)
{
   mpProgress = pProgress;
}
