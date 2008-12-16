/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"
#include "DateTimeImp.h"
#include "FileFinderImp.h"
#include "FilenameImp.h"
#include "TimeStruct.h"
#include <QtCore/QDateTime>

#if defined(WIN_API)
#include <windows.h>
#endif

using namespace std;

FileFinderImp::FileFinderImp() :
   mCurrentEntry(mEntryList.end())
{
}

FileFinderImp::~FileFinderImp()
{
}

bool FileFinderImp::findFile(const string& path, const string& criteria, bool includeDirectories)
{
   mDir.setPath(QString::fromStdString(path));
   mDir.setFilter(includeDirectories ? QDir::AllEntries : QDir::Files);
   mDir.setNameFilters(QStringList() << QString::fromStdString(criteria));
   mDir.setSorting(QDir::Name);
   if (!mDir.exists())
   {
      mDir = QDir();
      mEntryList.clear();
   }
   else
   {
      mEntryList = mDir.entryInfoList();
   }
   mCurrentEntry = mEntryList.end();
   return !mEntryList.isEmpty();
}

bool FileFinderImp::findNextFile()
{
   if (mEntryList.isEmpty())
   {
      return false;
   }
   if (mCurrentEntry == mEntryList.end())
   {
      mCurrentEntry = mEntryList.begin();
   }
   else
   {
      ++mCurrentEntry;
   }
   if (mCurrentEntry == mEntryList.end())
   {
      mEntryList.clear();
      mCurrentEntry = mEntryList.end();
      return false;
   }
   return true;
}

double FileFinderImp::getLength() const
{
   return (mCurrentEntry != mEntryList.end()) ? (mCurrentEntry->size()) : 0.0;
}

string FileFinderImp::getFileName() const
{
   string name;
   if (mCurrentEntry != mEntryList.end())
   {
      name = mCurrentEntry->fileName().toStdString();
   }
   return name;
}

string FileFinderImp::getFilePath() const
{
   string path;
   if (mCurrentEntry != mEntryList.end())
   {
      path = mCurrentEntry->absolutePath().toStdString();
      if (!path.empty() && path[path.size()-1] != '/')
      {
         path += "/";
      }
   }
   return path;
}

bool FileFinderImp::getFileTitle(string& fileTitle) const
{
   if (mCurrentEntry != mEntryList.end())
   {
      fileTitle = mCurrentEntry->baseName().toStdString();
      return true;
   }
   return false;
}

bool FileFinderImp::getFullPath(string& fullFilePath) const
{
   if (mCurrentEntry != mEntryList.end())
   {
      fullFilePath = mCurrentEntry->absoluteFilePath().toStdString();
      return true;
   }
   return false;
}

void FileFinderImp::getLastAccessTime(DateTime& dt) const
{
   if (mCurrentEntry != mEntryList.end())
   {
#if defined(WIN_API)
#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : Workaround for Qt 4.3.1 issue #186743 (dadkins)")
      WIN32_FILE_ATTRIBUTE_DATA fileData;
      if (GetFileAttributesEx(mCurrentEntry->absoluteFilePath().toStdString().c_str(),
         GetFileExInfoStandard, &fileData))
      {
         SYSTEMTIME systemTime;
         if (FileTimeToSystemTime(&fileData.ftLastAccessTime, &systemTime))
         {
            dt.set(systemTime.wYear, systemTime.wMonth, systemTime.wDay,
               systemTime.wHour, systemTime.wMinute, systemTime.wSecond);
         }
      }
#else
      QDateTime qdt = mCurrentEntry->lastRead().toUTC();
      if (qdt.isValid())
      {
         dt.set(qdt.date().year(), qdt.date().month(), qdt.date().day(), qdt.time().hour(),
            qdt.time().minute(), qdt.time().second());
      }
#endif
   }
}

void FileFinderImp::getLastModificationTime(DateTime& dt) const
{
   if (mCurrentEntry != mEntryList.end())
   {
#if defined(WIN_API)
#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : Workaround for Qt 4.3.1 issue #186743 (dadkins)")
      WIN32_FILE_ATTRIBUTE_DATA fileData;
      if (GetFileAttributesEx(mCurrentEntry->absoluteFilePath().toStdString().c_str(),
         GetFileExInfoStandard, &fileData))
      {
         SYSTEMTIME systemTime;
         if (FileTimeToSystemTime(&fileData.ftLastWriteTime, &systemTime))
         {
            dt.set(systemTime.wYear, systemTime.wMonth, systemTime.wDay,
               systemTime.wHour, systemTime.wMinute, systemTime.wSecond);
         }
      }
#else
      QDateTime qdt = mCurrentEntry->lastModified().toUTC();
      if (qdt.isValid())
      {
         dt.set(qdt.date().year(), qdt.date().month(), qdt.date().day(), qdt.time().hour(),
            qdt.time().minute(), qdt.time().second());
      }
#endif
   }
}

bool FileFinderImp::isDots() const
{
   return (mCurrentEntry != mEntryList.end()) &&
      (mCurrentEntry->fileName() == "." || mCurrentEntry->fileName() == "..");
}

bool FileFinderImp::isDirectory() const
{
   return (mCurrentEntry != mEntryList.end()) && mCurrentEntry->isDir();
}

bool FileFinderImp::isReadOnly() const
{
   return (mCurrentEntry != mEntryList.end()) && !mCurrentEntry->isWritable();
}
