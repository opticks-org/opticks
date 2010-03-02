/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef TRANSACTION_H__
#define TRANSACTION_H__

#include "Aeb.h"
#include "Serializable.h"
#include <exception>
#include <string>

class AebEntry;
class AebIo;

/**
 * This class indicates a failure processing a transaction during execution or rollback.
 */
class TransactionFailure : public std::runtime_error
{
public:
   TransactionFailure(const std::string &msg) : std::runtime_error(msg.c_str()) {}
};

/**
 * Interface for a transaction.
 * Specifies serialization routines, a priority, and priority based ordering.
 */
class Transaction
{
public:
   Transaction();
   ~Transaction();

   /**
    * Process a transaction.
    * @throw TransactionFailure on error
    */
   void execute();

   /**
    * Reverse a transaction that has successfully processed.
    * This should not be called if a transaction has not successfully returned from execute()
    * @throw TransactionFailure on error
    */
   void rollback();

   bool operator<(const Transaction& other) const;
   bool operator>(const Transaction& other) const;
   bool operator==(const Transaction& other) const;

   const AebEntry* getSource() const; 
   void setSource(const AebEntry* pSource);

   void setAebIo(const AebIo* pAebIo);

   std::string getDestination() const;
   void setDestination(const std::string& destination);

private:
   const AebEntry* mpAebEntry;
   std::string mDst;
   const AebIo* mpAebIo;
};

struct TransactionComparator
{
   bool operator()(Transaction* pA, Transaction* pB) const
   {
      if (pA == NULL)
      {
         return false;
      }
      if (pB == NULL)
      {
         return true;
      }
      return *pA < *pB;
   }
};

#endif
