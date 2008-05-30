/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef __FILENAMEIMP_H
#define __FILENAMEIMP_H

#include "Filename.h"

#include <memory>
#include <string>

class QFileInfo;

class FilenameImp : public Filename
{
public:
   FilenameImp();
   FilenameImp(const std::string& pathAndName);
   FilenameImp(const FilenameImp& rhs);
   virtual ~FilenameImp();

   virtual FilenameImp& operator =(const FilenameImp& rhs);

   void setFullPathAndName(const std::string& pathAndName);

   std::string getFullPathAndName() const;
   std::string getPath() const;
   std::string getFileName() const;
   std::string getTitle() const;
   std::string getExtension() const;

   bool isDirectory() const;

   const std::string& operator=(const std::string& path);
   bool operator==(const Filename& other) const;
   bool operator!=(const Filename& other) const;
   operator std::string() const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

private:
   const QFileInfo& getQFileInfo() const;
   std::auto_ptr<QFileInfo> mFileInfo;
   std::string mFilename;
};

#endif   // __FILENAMEIMP_H
