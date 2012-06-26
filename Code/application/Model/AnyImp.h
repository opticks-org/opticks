/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ANYIMP_H
#define ANYIMP_H

#include "DataElementImp.h"

#include <string>

class AnyData;

class AnyImp : public DataElementImp
{
public:
   AnyImp(const std::string& dataType, const DataDescriptorImp& descriptor, const std::string& id);
   ~AnyImp();

   void setData(AnyData* pData);
   AnyData* getData();
   const AnyData* getData() const;

   DataElement* copy(const std::string& name, DataElement* pParent) const;

   virtual bool serialize(SessionItemSerializer& serializer) const;
   virtual bool deserialize(SessionItemDeserializer& deserializer);

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   static bool isKindOfElement(const std::string& className);
   static void getElementTypes(std::vector<std::string>& classList);

private:
   AnyImp(const AnyImp& rhs);
   AnyData* mpData;
   std::string mDataType;
};

#define ANYADAPTEREXTENSION_CLASSES \
   DATAELEMENTADAPTEREXTENSION_CLASSES

#define ANYADAPTER_METHODS(impClass) \
   DATAELEMENTADAPTER_METHODS(impClass) \
   void setData(AnyData* pData) \
   { \
      impClass::setData(pData); \
   } \
   AnyData* getData() \
   { \
      return impClass::getData(); \
   } \
   const AnyData* getData() const \
   { \
      return impClass::getData(); \
   }

#endif
