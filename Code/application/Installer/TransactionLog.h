/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef TRANSACTIONLOG_H__
#define TRANSACTIONLOG_H__

#include "Transaction.h"
#include <set>

class Progress;

/**
 * A priority ordered transaction log with rollback capability.
 *
 * Transactions can be added to a log and will be ordered by
 * transaction priority. The log can then be executed. A partially
 * or completely executed log can be rolled back.
 */
class TransactionLog
{
public:
   /**
    * Construct a new TransactionLog.
    *
    * @param pProgress
    *        Optional Progress object where error messages and progress information will be posted.
    */
   TransactionLog(Progress* pProgress = NULL);

   /**
    * Cleanup a TransactionLog.
    */
   ~TransactionLog();

   /**
    * Can the TransactionLog be modified?
    *
    * @return true if the TransactionLog can be modified, false otherwise
    */
   bool canModify() const;

   /**
    * Clear the TransactionLog.
    *
    * The TransactionLog can only be cleared when nothing in the log has been executed.
    *
    * @return true if the TransactionLog was successfully cleared, false otherwise.
    *
    * @see canModify(), execute()
    */
   bool clearTransactions();

   /**
    * Add a Transaction to the log.
    *
    * A Transaction can only be added when nothing in the log has been executed.
    * A partially or completely executed log will fail to add a new Transaction.
    * The Transaction will not be executed immediately but will be prioritized.
    * Lower priority values will be executed first.
    *
    * @param pTransaction
    *        The Transaction to add. The Transaction must be unique.
    *        If the return value is true, the log will take ownership of pTransaction.
    *
    * @return true if the Transaction was successfully added, false otherwise.
    *
    * @see canModify(), execute()
    */
   bool addTransaction(Transaction* pTransaction);

   /**
    * Execute the TransactionLog.
    *
    * @param logFilename
    *        Location of the write-ahead log.
    * @param progressMessage
    *        Message to display while executing the TransactionLog. If this is
    *        empty, a default message will be displayed.
    *
    * @return True if the TransactionLog executed completely without error, false otherwise.
    */
   bool execute(const std::string& logFilename, const std::string& progressMessage = std::string("Executing transactions"));

   /**
    * Rollback a partially or completely executed TransactionLog.
    *
    * This is the reverse of execute(). If no Transactions have been executed
    * this will do nothing. If rollback of a Transaction fails, rollback will
    * stop and the TransactionLog will be in an indeterminate state.
    *
    * @param progressMessage
    *        Message to display while rolling back the transaction log. If this is
    *        empty, a default message will be displayed.
    *
    * @return True if the TransactionLog rolled back completely without error, false otherwise.
    */
   bool rollback(const std::string& progressMessage = std::string("Rolling back transactions"));

   /**
    * Deserialize the TransactionLog from a file.
    *
    * @param filename
    *        The path to the TransactionLog file.
    *
    * @return True if deserialization succeeded, false otherwise. If deserialization
    *         does not succeed, the TransactionLog will be in an indeterminate state.
    */
   bool deserialize(const std::string& filename);

   /**
    * Set the Progress reporting object.
    *
    * @param pProgress
    *        The new progress object.
    */
   void setProgress(Progress* pProgress);

private:
   typedef std::vector<Transaction*> queue_type;

   queue_type mTransactions;
   queue_type::size_type mNext;
   Progress* mpProgress;
   int mProgressCounter;
};

#endif
