/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"
#include "FilenameImp.h"
#include "StringUtilities.h"
#include "xmlreader.h"

#include <QtCore/QFileInfo>

using namespace std;

FilenameImp::FilenameImp() :
   mFileInfo(new QFileInfo())
{
}

FilenameImp::FilenameImp(const string& filename) :
   mFileInfo(new QFileInfo())
{
   setFullPathAndName(filename);
}

FilenameImp::FilenameImp(const FilenameImp& rhs) :
   mFileInfo(new QFileInfo(*(rhs.mFileInfo.get()))),
   mFilename(rhs.mFilename)
{
}

FilenameImp::~FilenameImp()
{
}

FilenameImp& FilenameImp::operator =(const FilenameImp& rhs)
{
   if (this != &rhs)
   {
      *(mFileInfo.get()) = *(rhs.mFileInfo.get());
      mFilename = rhs.mFilename;
   }

   return *this;
}

const QFileInfo& FilenameImp::getQFileInfo() const
{
   string filename = StringUtilities::expandVariables(mFilename);
   if (mFileInfo->absoluteFilePath().toStdString() != filename)
   {
      mFileInfo->setFile(QString::fromStdString(filename));
   }

   return *mFileInfo.get();
}

void FilenameImp::setFullPathAndName(const string& filename)
{
   string parsedFilename = filename;
   if (filename.substr(0, 7) == "file://")
   {
      parsedFilename = XmlReader::URLtoPath(X(filename.c_str()));
   }

   mFilename = parsedFilename;
}

string FilenameImp::getFullPathAndName() const
{
   string pathAndName;

   const QFileInfo& fileInfo = getQFileInfo();
   QString strFilePath = fileInfo.filePath();
   if (strFilePath.isEmpty() == false)
   {
      strFilePath = fileInfo.absoluteFilePath();
      if (strFilePath.isEmpty() == false)
      {
         pathAndName = strFilePath.toStdString();

#if defined(WIN_API)
         // Convert the drive letter to upper case to ensure string comparisons are successful
         if (pathAndName.size() > 3)
         {
            if (pathAndName[1] == ':' && pathAndName[2] == '/')
            {
               pathAndName[0] = toupper(pathAndName[0]);
            }
         }
#endif
      }
   }

   return pathAndName;
}

string FilenameImp::getPath() const
{
   string path;

   QString strPath = getQFileInfo().absolutePath();
   if (strPath.isEmpty() == false)
   {
      path = strPath.toStdString();
   }

   return path;
}

string FilenameImp::getFileName() const
{
   string filename;

   QString strFileName = getQFileInfo().fileName();
   if (strFileName.isEmpty() == false)
   {
      filename = strFileName.toStdString();
   }

   return filename;
}

string FilenameImp::getTitle() const
{
   string fileTitle;

   QString strBaseName = getQFileInfo().baseName();
   if (strBaseName.isEmpty() == false)
   {
      fileTitle = strBaseName.toStdString();
   }

   return fileTitle;
}

string FilenameImp::getExtension() const
{
   string extension;

   QString strExtension = getQFileInfo().completeSuffix();
   if (strExtension.isEmpty() == false)
   {
      extension = strExtension.toStdString();
   }

   return extension;
}

bool FilenameImp::isDirectory() const
{
   bool bDirectory = getQFileInfo().isDir();

   return bDirectory;
}

const string& FilenameImp::operator=(const string& path)
{
   setFullPathAndName(path);
   return path;
}

bool FilenameImp::operator==(const Filename& other) const
{
   return getFullPathAndName() == other.getFullPathAndName();
}

bool FilenameImp::operator!=(const Filename& other) const
{
   return getFullPathAndName() != other.getFullPathAndName();
}

FilenameImp::operator string() const
{
   return getFullPathAndName();
}

bool FilenameImp::toXml(XMLWriter* pXml) const
{
   return false;
}

bool FilenameImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   return false;
}
