/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef FILEFINDERIMP_H
#define FILEFINDERIMP_H

#include "AppConfig.h"
#include "FileFinder.h"
#include <QtCore/QDir>

class FileFinderImp : public FileFinder
{
public:
   FileFinderImp();
   ~FileFinderImp();

   virtual bool findFile(const std::string& path, const std::string& criteria, bool includeDirectories = false);
   virtual bool findNextFile();
   double getLength() const;
   std::string getFileName() const;
   std::string getFilePath() const;
   bool getFileTitle(std::string& fileTitle) const;
   bool getFullPath(std::string& fullFilePath) const;
   void getLastAccessTime(DateTime& dt) const;
   void getLastModificationTime(DateTime& dt) const;
   bool isDots() const;
   bool isDirectory() const;
   bool isReadOnly() const;

private:
   QDir mDir;
   QFileInfoList mEntryList;
   QFileInfoList::const_iterator mCurrentEntry;
};

#endif
