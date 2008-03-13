/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef FILEDESCRIPTORIMP_H
#define FILEDESCRIPTORIMP_H

#include "FilenameImp.h"
#include "SerializableImp.h"
#include "SubjectImp.h"
#include "TypesFile.h"
#include "xmlreader.h"
#include "xmlwriter.h"

#include <string>

class FileDescriptorImp : public SubjectImp, public Serializable
{
public:
   FileDescriptorImp();
   virtual ~FileDescriptorImp();

   FileDescriptorImp& operator =(const FileDescriptorImp& descriptor);

   void setFilename(const std::string& filename);
   void setFilename(const Filename& filename);
   const Filename& getFilename() const;

   void setDatasetLocation(const std::string& datasetLocation);
   const std::string& getDatasetLocation() const;

   void setEndian(EndianType endian);
   EndianType getEndian() const;

   virtual void addToMessageLog(Message* pMessage) const;

   virtual bool toXml(XMLWriter* pXml) const;
   virtual bool fromXml(DOMNode* pDocument, unsigned int version);
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   static void getFileDescriptorTypes(std::vector<std::string>& classList);
   static bool isKindOfFileDescriptor(const std::string& className);

private:
   FilenameImp mFilename;
   std::string mDatasetLocation;
   EndianType mEndian;
};

#define FILEDESCRIPTORADAPTER_METHODS(impClass) \
   SUBJECTADAPTER_METHODS(impClass) \
   SERIALIZABLEADAPTER_METHODS(impClass) \
   void setFilename(const std::string& filename) \
   { \
      impClass::setFilename(filename); \
   } \
   void setFilename(const Filename& filename) \
   { \
      impClass::setFilename(filename); \
   } \
   const Filename& getFilename() const \
   { \
      return impClass::getFilename(); \
   } \
   void setDatasetLocation(const std::string& datasetLocation) \
   { \
      impClass::setDatasetLocation(datasetLocation); \
   } \
   const std::string& getDatasetLocation() const \
   { \
      return impClass::getDatasetLocation(); \
   } \
   void setEndian(EndianType endian) \
   { \
      impClass::setEndian(endian); \
   } \
   EndianType getEndian() const \
   { \
      return impClass::getEndian(); \
   } \
   void addToMessageLog(Message* pMessage) const \
   { \
      impClass::addToMessageLog(pMessage); \
   }

#endif
