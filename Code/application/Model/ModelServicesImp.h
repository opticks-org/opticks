/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MODELSERVICESIMP_H
#define MODELSERVICESIMP_H

#include <xercesc/dom/DOM.hpp>

#include "DataElement.h"
#include "ModelServices.h"
#include "SettableSessionItemAdapter.h"
#include "StringUtilities.h"
#include "SubjectImp.h"
#include "XercesIncludes.h"

#include <vector>

using XERCES_CPP_NAMESPACE_QUALIFIER DOMElement;

class ModelServicesImp : public ModelServices, public SubjectImp, public SettableSessionItemAdapter
{
public:
   static ModelServicesImp* instance();
   static void destroy();

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   SUBJECTADAPTER_METHODS(SubjectImp)

   DataDescriptor* createDataDescriptor(const std::string& name, const std::string& type,
      DataElement* pParent) const;
   DataDescriptor* createDataDescriptor(const std::string& name, const std::string& type,
      const std::vector<std::string> &parent) const;
   void destroyDataDescriptor(DataDescriptor* pDescriptor) const;
   ImportDescriptor* createImportDescriptor(const std::string& name, const std::string& type,
      DataElement* pParent, bool bImported = true) const;
   ImportDescriptor* createImportDescriptor(const std::string& name, const std::string& type,
      const std::vector<std::string> &parent, bool bImported = true) const;
   ImportDescriptor* createImportDescriptor(DataDescriptor* pDescriptor, bool bImported = true) const;
   void destroyImportDescriptor(ImportDescriptor* pImportDescriptor) const;

   bool addElementType(const std::string& elementType);
   bool hasElementType(const std::string& elementType) const;
   const std::vector<std::string>& getValidElementTypes() const;

   std::vector<DataElement*> createElements(const std::vector<DataDescriptor*> &descriptors);
   DataElement* createElement(const DataDescriptor* pDescriptor);
   DataElement* createElement(const DataDescriptor* pDescriptor, const std::string &id);
   DataElement* createElement(const std::string& name, const std::string& type, DataElement* pParent);
   DataElement* getElement(const std::string& name, const std::string& type, DataElement* pParent) const;
   DataElement* getElement(const std::vector<std::string>& designator, const std::string& type) const;
   std::vector<DataElement*> getElements(const std::string& type) const;
   std::vector<DataElement*> getElements(DataElement* pParent, const std::string& type) const;
   std::vector<DataElement*> getElements(const std::string& filename, const std::string& type) const;
   std::vector<std::string> getElementNames(const std::string& type) const;
   std::vector<std::string> getElementNames(DataElement* pParent, const std::string& type) const;
   std::vector<std::string> getElementNames(const std::string& filename, const std::string& type) const;
   bool setElementName(DataElement* pElement, const std::string& name);
   bool setElementParent(DataElement* pElement, DataElement *pParent);
   bool removeElement(DataElement* pElement);
   bool destroyElement(DataElement* pElement);
   void clear();

   char* getMemoryBlock(size_t size);
   void deleteMemoryBlock(char* memory); 

   double getDataValue(EncodingType type, void* pData, int iIndex) const;
   double getDataValue(EncodingType type, void* pData, ComplexComponent component, int iIndex) const;

   bool isKindOfElement(const std::string& className, const std::string& elementName) const;
   void getElementTypes(const std::string& className, std::vector<std::string>& classList) const;
   bool isKindOfDataDescriptor(const std::string& className, const std::string& descriptorName) const;
   void getDataDescriptorTypes(const std::string& className, std::vector<std::string>& classList) const;
   bool isKindOfFileDescriptor(const std::string& className, const std::string& descriptorName) const;
   void getFileDescriptorTypes(const std::string& className, std::vector<std::string>& classList) const;

   bool addElement(DataElement* pElement);

   bool serialize(SessionItemSerializer &serializer) const;
   bool deserialize(SessionItemDeserializer &deserializer);

protected:
   ModelServicesImp();
   ~ModelServicesImp();

private:
   class Key
   {
   public:
      Key(const std::string& name, DataElement* pParent) :
         mName(StringUtilities::toLower(name)),
         mpParent(pParent)
      {
      }

      explicit Key(DataElement& element) :
         mName(StringUtilities::toLower(element.getName())),
         mpParent(element.getParent())
      {
      }

      bool operator==(const Key&rhs) const
      {
         if ((mName == rhs.mName) && (mpParent == rhs.mpParent))
         {
            return true;
         }

         return false;
      }

      bool operator!=(const Key&rhs) const
      {
         return !(*this == rhs);
      }

      bool operator<(const Key& rhs) const
      {
         if (mName < rhs.mName)
         {
            return true;
         }
         else if (mName > rhs.mName)
         {
            return false;
         }
         else
         {
            if (mpParent < rhs.mpParent)
            {
               return true;
            }
         }

         return false;
      }

   private:
      std::string mName;
      const DataElement* mpParent;
   };

   static ModelServicesImp* spInstance;
   static bool mDestroyed;
   std::vector<std::string> mElementTypes;
   std::multimap<Key, DataElement*> mElements;

   std::multimap<Key, DataElement*>::iterator findElement(DataElement* pElement);
   std::multimap<Key, DataElement*>::iterator findElement(const Key& key, const std::string& type);
   std::multimap<Key, DataElement*>::const_iterator findElement(const Key& key, const std::string& type) const;
};

#endif
