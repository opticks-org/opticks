/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef AEBIO_H
#define AEBIO_H

#include "AppConfig.h"
#include "ZipResource.h"

#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QUrl>

#include <string>

class Aeb;
class AebIo;

class AebEntry
{
public:
   QString getFilePath() const;
   int64_t getFileSize() const;

protected:
   AebEntry(QString path, size_t size);
   virtual ~AebEntry();
   friend class AebIo;

   QString mFilePath;
   int64_t mFileSize;
};

class AebIo
{
public:
   AebIo(Aeb& obj);
   virtual ~AebIo();

   virtual bool fromFile(const std::string& fname, std::string& errMsg);
   virtual bool toFile(const std::string& fname);

   const QList<const AebEntry*>& getContentPaths(std::string& errMsg) const;
   const AebEntry* getEntry(const QUrl& aebUrl) const;

   bool installFileFromAeb(const AebEntry* pSource, const std::string& destination) const;
   bool compareFileInAeb(const AebEntry* pSource, const std::string& destination) const;

private:
   AebIo& operator=(const AebIo& rhs);

   bool openZipFileIfNeeded() const;
   QByteArray getBytesFromAeb(const QString& url, bool& wasValidPath) const;

   static std::string sAeblPrefix;
   static std::string sAeblTopSubject;
   static std::string sOpticksPrefix;
   mutable QList<const AebEntry*> mContentPaths;
   Aeb& mObj;
   mutable ZipFileResource mZipFile;
   mutable QByteArray mBuf;
   mutable QByteArray mBuf2;
};

#endif