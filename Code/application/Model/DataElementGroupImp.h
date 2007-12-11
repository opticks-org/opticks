/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DATA_ELEMENT_GROUP_IMP
#define DATA_ELEMENT_GROUP_IMP

#include "DataElementImp.h"

class DataElementGroupImp : public DataElementImp
{
public:
   DataElementGroupImp(const DataDescriptorImp& descriptor, const std::string& id);
   virtual ~DataElementGroupImp();

   bool insertElement(DataElement* pElement);
   bool insertElements(const std::vector<DataElement*>& elements);
   bool hasElement(DataElement* pElement) const;
   unsigned int getNumElements() const;
   const std::vector<DataElement*>& getElements() const;
   bool removeElement(DataElement* pElement, bool bDelete = false);
   bool removeElements(const std::vector<DataElement*>& elements, bool bDelete = false);
   void clear(bool bDelete = false);
   bool enforceUniformity(bool enforce);
   bool isUniform() const;

   void elementDeleted(Subject &subject, const std::string &signal, const boost::any &data);

   DataElement* copy(const std::string& name, DataElement *pParent) const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   static void getElementTypes(std::vector<std::string>& classList);
   static bool isKindOfElement(const std::string& className);

private:
   std::vector<DataElement*> mElements;
   bool mEnforceUniformity;
   bool mNotificationEnabled;
};

#define DATAELEMENTGROUPADAPTER_METHODS(impClass) \
   DATAELEMENTADAPTER_METHODS(impClass) \
   bool insertElement(DataElement* pElement) \
   { \
      return impClass::insertElement(pElement); \
   } \
   bool insertElements(const std::vector<DataElement*>& elements) \
   { \
      return impClass::insertElements(elements); \
   } \
   bool hasElement(DataElement* pElement) const \
   { \
      return impClass::hasElement(pElement); \
   } \
   unsigned int getNumElements() const \
   { \
      return impClass::getNumElements(); \
   } \
   const std::vector<DataElement*>& getElements() const \
   { \
      return impClass::getElements(); \
   } \
   bool removeElement(DataElement* pElement, bool bDelete) \
   { \
      return impClass::removeElement(pElement, bDelete); \
   } \
   bool removeElements(const std::vector<DataElement*>& elements, bool bDelete) \
   { \
      return impClass::removeElements(elements, bDelete); \
   } \
   bool enforceUniformity(bool enforce) \
   { \
      return impClass::enforceUniformity(enforce); \
   } \
   bool isUniform() const \
   { \
      return impClass::isUniform(); \
   } \
   void clear(bool bDelete) \
   { \
      impClass::clear(bDelete); \
   }

#endif
