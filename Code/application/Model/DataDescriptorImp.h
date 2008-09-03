/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DATADESCRIPTORIMP_H
#define DATADESCRIPTORIMP_H

#include "AttachmentPtr.h"
#include "ClassificationAdapter.h"
#include "DataElement.h"
#include "DynamicObjectAdapter.h"
#include "FileDescriptor.h"
#include "FileDescriptorImp.h"
#include "SerializableImp.h"
#include "SubjectImp.h"
#include "TypesFile.h"
#include "xmlreader.h"
#include "xmlwriter.h"

#include <string>

class DataDescriptor;
class DataElement;
class FileDescriptorImp;

using XERCES_CPP_NAMESPACE_QUALIFIER DOMNode;

class DataDescriptorImp : public SubjectImp
{
public:
   DataDescriptorImp(const std::string& name, const std::string& type, DataElement* pParent);
   DataDescriptorImp(const std::string& name, const std::string& type, const std::vector<std::string>& parent);
   ~DataDescriptorImp();

   void metadataModified(Subject &subject, const std::string &signal, const boost::any &data);

   void setName(const std::string& name);
   const std::string& getName() const;
   const std::string& getType() const;
   void setParent(DataElement *pParent);
   DataElement* getParent() const;
   std::vector<std::string> getParentDesignator() const;

   void setClassification(const Classification* pClassification);
   Classification* getClassification();
   const Classification* getClassification() const;
   void setMetadata(const DynamicObject* pMetadata);
   DynamicObject* getMetadata();
   const DynamicObject* getMetadata() const;
   void setProcessingLocation(ProcessingLocation processingLocation);
   ProcessingLocation getProcessingLocation() const;
   void setFileDescriptor(const FileDescriptorImp* pFileDescriptor);
   FileDescriptorImp* getFileDescriptor();
   const FileDescriptorImp* getFileDescriptor() const;

   virtual DataDescriptor* copy() const;
   virtual DataDescriptor* copy(const std::string& name, DataElement* pParent) const;
   virtual DataDescriptor* copy(const std::string& name, const std::vector<std::string>& parent) const;

   virtual void addToMessageLog(Message* pMessage) const;

   virtual bool toXml(XMLWriter* pXml) const;
   virtual bool fromXml(DOMNode* pDocument, unsigned int version);
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   static void getDataDescriptorTypes(std::vector<std::string>& classList);
   static bool isKindOfDataDescriptor(const std::string& className);

   void setImporterName(const std::string &name) { mImporterName = name; }
   std::string getImporterName() const { return mImporterName; }

private:
   void generateParentDesignator();
   std::string mName;
   std::string mType;
   AttachmentPtr<DataElement> mpParent;
   std::vector<std::string> mParentDesignator;
   ClassificationAdapter mClassification;
   DynamicObjectAdapter mMetadata;
   ProcessingLocation mProcessingLocation;
   FileDescriptorImp* mpFileDescriptor;
   std::string mImporterName;
};

#define DATADESCRIPTORADAPTEREXTENSION_CLASSES \
   SUBJECTADAPTEREXTENSION_CLASSES \
   SERIALIZABLEADAPTEREXTENSION_CLASSES

#define DATADESCRIPTORADAPTER_METHODS(impClass) \
   SUBJECTADAPTER_METHODS(impClass) \
   SERIALIZABLEADAPTER_METHODS(impClass) \
   const std::string& getName() const \
   { \
      return impClass::getName(); \
   } \
   const std::string& getType() const \
   { \
      return impClass::getType(); \
   } \
   DataElement* getParent() const \
   { \
      return impClass::getParent(); \
   } \
   std::vector<std::string> getParentDesignator() const \
   { \
      return impClass::getParentDesignator(); \
   } \
   void setClassification(const Classification* pClassification) \
   { \
      impClass::setClassification(pClassification); \
   } \
   Classification *getClassification() \
   { \
      return impClass::getClassification(); \
   } \
   const Classification* getClassification() const \
   { \
      return impClass::getClassification(); \
   } \
   void setMetadata(const DynamicObject* pMetadata) \
   { \
      impClass::setMetadata(pMetadata); \
   } \
   DynamicObject* getMetadata() \
   { \
      return impClass::getMetadata(); \
   } \
   const DynamicObject* getMetadata() const \
   { \
      return impClass::getMetadata(); \
   } \
   void setProcessingLocation(ProcessingLocation processingLocation) \
   { \
      impClass::setProcessingLocation(processingLocation); \
   } \
   ProcessingLocation getProcessingLocation() const \
   { \
      return impClass::getProcessingLocation(); \
   } \
   void setFileDescriptor(const FileDescriptor* pFileDescriptor) \
   { \
      impClass::setFileDescriptor(dynamic_cast<const FileDescriptorImp*>(pFileDescriptor)); \
   } \
   FileDescriptor* getFileDescriptor() \
   { \
      return dynamic_cast<FileDescriptor*>(impClass::getFileDescriptor()); \
   } \
   const FileDescriptor* getFileDescriptor() const \
   { \
      return dynamic_cast<const FileDescriptor*>(impClass::getFileDescriptor()); \
   } \
   DataDescriptor* copy() const \
   { \
      return dynamic_cast<DataDescriptor*>(impClass::copy()); \
   } \
   DataDescriptor* copy(const std::string& name, DataElement* pParent) const \
   { \
      return dynamic_cast<DataDescriptor*>(impClass::copy(name, pParent)); \
   } \
   DataDescriptor* copy(const std::string& name, const std::vector<std::string>& parent) const \
   { \
      return dynamic_cast<DataDescriptor*>(impClass::copy(name, parent)); \
   } \
   void addToMessageLog(Message* pMessage) const \
   { \
      impClass::addToMessageLog(pMessage); \
   }

#endif
