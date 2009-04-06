/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AebIo.h"
#include "AppVerify.h"
#include "ConfigurationSettings.h"
#include "Filename.h"
#include "InstallerServices.h"
#include "ObjectResource.h"
#include "ProductView.h"
#include "ReferenceCountDatabase.h"
#include "StringUtilities.h"
#include "Transaction.h"
#include "TypeConverter.h"

#include <boost/utility.hpp>

#include <QtCore/QBuffer>
#include <QtCore/QByteArray>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QStringList>
#include <QtCore/QUrl>

Transaction::Transaction() : mpAebIo(NULL), mpAebEntry(NULL)
{
}

Transaction::~Transaction()
{
}

void Transaction::execute()
{
   if (mpAebIo == NULL)
   {
      throw TransactionFailure("Bad AebIo");
   }
   if (mpAebEntry == NULL)
   {
      throw TransactionFailure("Bad entry in Aeb");
   }
   QFile dst(QString::fromStdString(mDst));
   if (!dst.exists())
   {
      QFileInfo info(QString::fromStdString(mDst));
      QDir dstDir = info.absoluteDir();
      if (!dstDir.exists())
      {
         dstDir.mkpath(".");
      }
      if (!mpAebIo->installFileFromAeb(mpAebEntry, mDst))
      {
         throw TransactionFailure("Unable to copy " + mpAebEntry->getFilePath().toStdString() + " from Aeb to " + mDst + ".");
      }
   }
   else
   {
      if (mpAebIo->compareFileInAeb(mpAebEntry, mDst))
      {
         // if they are the same, increment ref count
         ReferenceCountDatabase& db = Service<InstallerServices>()->getReferenceCountDatabase();
         QHash<QString, int>::iterator iter = db.getReferenceCount(mDst);
         iter.value()++;
      }
      else
      {
         throw TransactionFailure("Unable to copy " + mpAebEntry->getFilePath().toStdString() + " from Aeb to " + mDst + ".\n"
            "Destination exists but is different than source.");
      }
   }
}

void Transaction::rollback()
{
   ReferenceCountDatabase& db = Service<InstallerServices>()->getReferenceCountDatabase();
   QHash<QString, int>::iterator iter = db.getReferenceCount(mDst);
   if (iter.value() == 1)
   {
      QString dst = QString::fromStdString(mDst);
      if (!QFile::remove(dst))
      {
         throw TransactionFailure("Unable to remove " + mDst);
      }
      QFileInfo(dst).absoluteDir().rmpath(".");
   }

   // dec ref count
   iter.value()--;
}

bool Transaction::operator<(const Transaction& other) const
{
   return mDst < other.mDst;
}

bool Transaction::operator>(const Transaction& other) const
{
   return other < *this;
}

bool Transaction::operator==(const Transaction& other) const
{
   // equality compares only the destination...copying a different file
   // to the same destination results in the same result. the first
   // transaction added typically takes presedence. both source files are either
   // the same which will lead to an incorrect ref count or are different which will guarantee
   // failure of the transaction log...if they are different, we simply ignore all but the
   // first transaction
   return mDst == other.mDst;
}

const AebEntry* Transaction::getSource() const
{
   return mpAebEntry;
}

void Transaction::setSource(const AebEntry* pSource)
{
   mpAebEntry = pSource;
}

void Transaction::setAebIo(const AebIo* pAebIo)
{
   mpAebIo = pAebIo;
}

std::string Transaction::getDestination() const
{
   return mDst;
}

void Transaction::setDestination(const std::string& destination)
{
   mDst = destination;
}
