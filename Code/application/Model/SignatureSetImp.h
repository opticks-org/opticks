/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SIGNATURESETIMP_H
#define SIGNATURESETIMP_H

#include "DataDescriptorImp.h"
#include "DataElementGroupImp.h"
#include "SignatureImp.h"

#include <vector>

class Signature;

class SignatureSetImp : public SignatureImp
{
public:
   SignatureSetImp(const DataDescriptorImp& descriptor, const std::string& id);
   ~SignatureSetImp();

   void signatureDeleted(Subject &subject, const std::string &signal, const boost::any &data);

   bool insertSignature(Signature* pSignature);
   bool insertSignatures(const std::vector<Signature*>& signatures);
   unsigned int getNumSignatures() const;
   bool hasSignature(Signature* pSignature) const;
   std::vector<Signature*> getSignatures() const;
   bool removeSignature(Signature* pSignature, bool bDelete = false);
   bool removeSignatures(const std::vector<Signature*>& signatures, bool bDelete = false);
   void clear(bool bDelete = false);

   DataElement* copy(const std::string& name, DataElement* pParent) const;

   bool serialize(SessionItemSerializer& serializer) const;
   bool deserialize(SessionItemDeserializer &deserializer);
   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   static bool isKindOfElement(const std::string& className);

private:
   DataDescriptorImp mGroupDescriptor;
   DataElementGroupImp mGroup;
};

#define SIGNATURESETADAPTEREXTENSION_CLASSES \
   SIGNATUREADAPTEREXTENSION_CLASSES

#define SIGNATURESETADAPTER_METHODS(impClass) \
   SIGNATUREADAPTER_METHODS(impClass) \
   bool insertSignature(Signature* pSignature) \
   { \
      return impClass::insertSignature(pSignature); \
   } \
   std::vector<Signature*> getSignatures() const \
   { \
      return impClass::getSignatures(); \
   } \
   bool removeSignature(Signature* pSignature, bool bDelete) \
   { \
      return impClass::removeSignature(pSignature, bDelete); \
   } \
   bool insertSignatures(const std::vector<Signature*>& signatures) \
   { \
      return impClass::insertSignatures(signatures); \
   } \
   bool hasSignature(Signature* pSignature) const \
   { \
      return impClass::hasSignature(pSignature); \
   } \
   unsigned int getNumSignatures() const \
   { \
      return impClass::getNumSignatures(); \
   } \
   bool removeSignatures(const std::vector<Signature*>& signatures, bool bDelete) \
   { \
      return impClass::removeSignatures(signatures, bDelete); \
   } \
   void clear(bool bDelete) \
   { \
      impClass::clear(bDelete); \
   }

#endif
