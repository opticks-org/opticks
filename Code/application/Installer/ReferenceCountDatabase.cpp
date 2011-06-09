/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ConfigurationSettings.h"
#include "ReferenceCountDatabase.h"
#include "StringUtilities.h"
#include "xmlreader.h"
#include <QtCore/QDataStream>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QString>

ReferenceCountDatabase::ReferenceCountDatabase()
{
   loadDb();
}

ReferenceCountDatabase::~ReferenceCountDatabase()
{
   saveDb();
}

QHash<QString, int>::iterator ReferenceCountDatabase::getReferenceCount(const std::string& filename)
{
   QString npath = normalizePath(filename);
   QHash<QString, int>::iterator foundIter = mDb.find(npath);
   if (foundIter == mDb.end())
   {
      if (exists(npath))
      {
         foundIter = mDb.insert(npath, 1);
      }
      else
      {
         foundIter = mDb.insert(npath, 0);
      }
   }
   return foundIter;
}

QString ReferenceCountDatabase::normalizePath(const std::string& filename)
{
   QDir appPath(QString::fromStdString(Service<ConfigurationSettings>()->getHome()));
   return QDir::fromNativeSeparators(QDir::cleanPath(appPath.relativeFilePath(QString::fromStdString(filename))));
}

bool ReferenceCountDatabase::exists(const QString& normalizedFilename)
{
   QDir appPath(QString::fromStdString(Service<ConfigurationSettings>()->getHome()));
   return QFileInfo(appPath, normalizedFilename).exists();
}

void ReferenceCountDatabase::loadDb()
{
   mDb.clear();
   QFile dbFile(QString::fromStdString(
      Service<ConfigurationSettings>()->getSettingExtensionFilesPath()->getFullPathAndName() + "/refcounts.db"));
   if (!dbFile.open(QIODevice::ReadOnly))
   {
      return;
   }
   QDataStream in(&dbFile);
   QString magic;
   in >> magic;
   if (magic != "refDbV1")
   {
      return;
   }

   while (!in.atEnd())
   {
      QString key;
      int value = 0;
      in >> key >> value;
      mDb[key] = value;
   }
}

void ReferenceCountDatabase::saveDb() const
{
   QFile dbFile(QString::fromStdString(
      Service<ConfigurationSettings>()->getSettingExtensionFilesPath()->getFullPathAndName() + "/refcounts.db"));
   if (!dbFile.open(QIODevice::WriteOnly))
   {
      return;
   }
   bool wroteData = false;
   QDataStream out(&dbFile);
   out << QString("refDbV1");
   for (QHash<QString, int>::const_iterator entry = mDb.begin(); entry != mDb.end(); ++entry)
   {
      if (entry.value() > 1)
      {
         out << entry.key() << entry.value();
         wroteData = true;
      }
   }
   dbFile.close();
   if (!wroteData)
   {
      dbFile.remove();
   }
}