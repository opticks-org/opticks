/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "SignatureSetImp.h"
#include "ModelServicesImp.h"
#include "SessionItemDeserializer.h"
#include "SessionItemSerializer.h"
#include "Signature.h"
#include "SignatureAdapter.h"
#include "SignatureSetAdapter.h"

XERCES_CPP_NAMESPACE_USE
using namespace std;

SignatureSetImp::SignatureSetImp(const DataDescriptorImp& descriptor, const string& id) :
   SignatureImp(descriptor, id),
   mGroupDescriptor(descriptor.getName(), "DataElementGroup", NULL),
   mGroup(mGroupDescriptor, SessionItemImp::generateUniqueId())
{
   mGroup.attach(SIGNAL_NAME(Subject, Modified), Signal(dynamic_cast<Subject*>(this), SIGNAL_NAME(Subject, Modified)));
}

SignatureSetImp::~SignatureSetImp()
{
   mGroup.clear(false);
}

void SignatureSetImp::signatureDeleted(Subject &subject, const string &signal, const boost::any &data)
{
   Signature* pSignature = dynamic_cast<Signature*> (&subject);
   if (NN(pSignature))
   {
      removeSignature(pSignature, false);
   }
}

bool SignatureSetImp::insertSignature(Signature* pSignature)
{
   if (pSignature == NULL)
   {
      return false;
   }

   // Do not insert the signature if it is already in the vector
   if (mGroup.hasElement(pSignature) == true)
   {
      return false;
   }

   return mGroup.insertElement(pSignature);
}

vector<Signature*> SignatureSetImp::getSignatures() const
{
   vector<Signature*> signatures;
   const vector<DataElement*> &elements = mGroup.getElements();
   vector<DataElement*>::const_iterator ppElement;
   for (ppElement=elements.begin(); ppElement!=elements.end(); ++ppElement)
   {
      Signature *pSignature = static_cast<Signature*>(*ppElement);
      if (pSignature)
      {
         signatures.push_back(pSignature);
      }
   }
   return signatures;
}

bool SignatureSetImp::removeSignature(Signature* pSignature, bool bDelete)
{
   if (pSignature == NULL)
   {
      return false;
   }

   return mGroup.removeElement(pSignature, bDelete);
}

bool SignatureSetImp::insertSignatures(const std::vector<Signature*>& signatures)
{
   return mGroup.insertElements(reinterpret_cast<const vector<DataElement*>&>(signatures));
}

unsigned int SignatureSetImp::getNumSignatures() const
{
   return mGroup.getNumElements();
}

bool SignatureSetImp::hasSignature(Signature* pSignature) const
{
   return mGroup.hasElement(pSignature);
}

bool SignatureSetImp::removeSignatures(const std::vector<Signature*>& signatures, bool bDelete)
{
   return mGroup.removeElements(reinterpret_cast<const vector<DataElement*>&>(signatures));
}

void SignatureSetImp::clear(bool bDelete)
{
   mGroup.clear(bDelete);
}

DataElement* SignatureSetImp::copy(const string& name, DataElement* pParent) const
{
   return NULL;
}

bool SignatureSetImp::serialize(SessionItemSerializer& serializer) const
{
   XMLWriter xml("SignatureSet");
   if(!SessionItemImp::toXml(&xml))
   {
      return false;
   }
   const DynamicObject *pMetadata = getMetadata();
   if(pMetadata != NULL)
   {
      xml.pushAddPoint(xml.addElement("Metadata"));
      if(!pMetadata->toXml(&xml))
      {
         return false;
      }
   }
   // the DataDescriptor will be serialized as part of the group's serialization
   if(!mGroup.toXml(&xml))
   {
      return false;
   }
   return serializer.serialize(xml);
}

bool SignatureSetImp::deserialize(SessionItemDeserializer &deserializer)
{
   XmlReader reader(NULL, false);
   DOMDocument *pDoc = deserializer.deserialize(reader);
   if(pDoc == NULL || !SessionItemImp::fromXml(pDoc, XmlBase::VERSION))
   {
      return false;
   }
   DOMElement *pRoot = pDoc->getDocumentElement();
   VERIFY(pRoot != NULL);
   unsigned int formatVersion = 0;
   stringstream str(A(pRoot->getAttribute(X("version"))));
   str >> formatVersion;
   for(DOMNode *pNode = pRoot->getFirstChild(); pNode != NULL; pNode = pNode->getNextSibling())
   {
      if (XMLString::equals(pNode->getNodeName(),X("Metadata")))
      {
         DynamicObject *pMetadata = getMetadata();
         if(pMetadata == NULL || !pMetadata->fromXml(pNode, formatVersion))
         {
            return false;
         }
      }
   }
   return mGroup.fromXml(pRoot, formatVersion);
}

bool SignatureSetImp::toXml(XMLWriter* pXml) const
{
   return false;
}

bool SignatureSetImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   return false;
}

const string& SignatureSetImp::getObjectType() const
{
   static string type("SignatureSetImp");
   return type;
}

bool SignatureSetImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "SignatureSet"))
   {
      return true;
   }

   return SignatureImp::isKindOf(className);
}

bool SignatureSetImp::isKindOfElement(const string& className)
{
   if ((className == "SignatureSetImp") || (className == "SignatureSet"))
   {
      return true;
   }

   return SignatureImp::isKindOfElement(className);
}
