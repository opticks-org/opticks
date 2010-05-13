/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DATAELEMENTIMP_H
#define DATAELEMENTIMP_H

#include "SessionItemImp.h"
#include "SubjectImp.h"
#include "DataDescriptor.h"
#include "DataDescriptorImp.h"
#include "xmlreader.h"
#include "xmlwriter.h"

#include <string>

class DataElement;

class DataElementImp : public SessionItemImp, public SubjectImp
{
public:
   DataElementImp(const DataDescriptorImp& descriptor, const std::string& id);
   ~DataElementImp();

   void descriptorRenamed(Subject &subject, const std::string &signal, const boost::any &data);
   void descriptorModified(Subject &subject, const std::string &signal, const boost::any &data);

   DataElementImp& operator =(const DataElementImp& element);

   DataDescriptorImp* getDataDescriptor();
   const DataDescriptorImp* getDataDescriptor() const;

   void setName(const std::string& name);
   std::string getType() const;
   std::string getFilename() const;

   DataElement* getParent() const;
   std::vector<std::string> getParentDesignator() const;
   const Classification* getClassification() const;
   void setClassification(const Classification* pClassification);
   void copyClassification(const DataElement* pElement);
   DynamicObject* getMetadata();
   const DynamicObject* getMetadata() const;

   bool canRename() const;
   bool rename(const std::string& newName, std::string& errorMessage);

   virtual DataElement* copy(const std::string& name, DataElement* pParent) const;

   virtual bool serialize(SessionItemSerializer& serializer) const;
   virtual bool deserialize(SessionItemDeserializer &deserializer);

   virtual bool toXml(XMLWriter* pXml) const;
   virtual bool fromXml(DOMNode* pDocument, unsigned int version);
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   static void getElementTypes(std::vector<std::string>& classList);
   static bool isKindOfElement(const std::string& className);

private:
   DataDescriptorImp* mpDataDescriptor;
};

#define DATAELEMENTADAPTEREXTENSION_CLASSES \
   SESSIONITEMADAPTEREXTENSION_CLASSES \
   SUBJECTADAPTEREXTENSION_CLASSES

#define DATAELEMENTADAPTER_METHODS(impClass) \
   SESSIONITEMADAPTER_METHODS(impClass) \
   SUBJECTADAPTER_METHODS(impClass) \
   DataDescriptor* getDataDescriptor() \
   { \
      return dynamic_cast<DataDescriptor*>(impClass::getDataDescriptor()); \
   } \
   const DataDescriptor* getDataDescriptor() const \
   { \
      return dynamic_cast<const DataDescriptor*>(impClass::getDataDescriptor()); \
   } \
   std::string getType() const \
   { \
      return impClass::getType();\
   } \
   std::string getFilename() const \
   { \
      return impClass::getFilename();\
   } \
   DataElement* getParent() const \
   { \
      return impClass::getParent();\
   } \
   std::vector<std::string> getParentDesignator() const \
   { \
      return impClass::getParentDesignator(); \
   } \
   const Classification* getClassification() const \
   { \
      return impClass::getClassification();\
   } \
   void setClassification(const Classification* pClassification) \
   { \
      return impClass::setClassification(pClassification);\
   } \
   void copyClassification(const DataElement* pElement) \
   { \
      return impClass::copyClassification(pElement); \
   } \
   DynamicObject* getMetadata() \
   { \
      return impClass::getMetadata();\
   } \
   const DynamicObject* getMetadata() const \
   { \
      return impClass::getMetadata();\
   } \
   DataElement* copy(const std::string& name, DataElement* pParent) const \
   { \
      return impClass::copy(name, pParent);\
   }

#endif
