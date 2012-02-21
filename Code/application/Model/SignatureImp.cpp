/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "DataVariant.h"
#include "ModelServicesImp.h"
#include "Signature.h"
#include "SignatureDataDescriptor.h"
#include "SignatureImp.h"

#include <set>

XERCES_CPP_NAMESPACE_USE
using namespace std;

SignatureImp::SignatureImp(const DataDescriptorImp& descriptor, const string& id) :
   DataElementImp(descriptor, id)
{
}

SignatureImp::~SignatureImp()
{
}

DataElement* SignatureImp::copy(const string& name, DataElement* pParent) const
{
   DataElement* pElement = DataElementImp::copy(name, pParent);

   SignatureImp* pSignature = dynamic_cast<SignatureImp*>(pElement);
   if (pSignature != NULL)
   {
      map<string, DataVariant>::const_iterator dataIter;
      for (dataIter = mData.begin(); dataIter != mData.end(); ++dataIter)
      {
         pSignature->setData(dataIter->first, const_cast<DataVariant&>(dataIter->second), false);
      }
   }

   return pElement;
}

bool SignatureImp::toXml(XMLWriter* pXml) const
{
   if (!DataElementImp::toXml(pXml))
   {
      return false;
   }
   for (map<string, DataVariant>::const_iterator datum = mData.begin(); datum != mData.end(); ++datum)
   {
      pXml->pushAddPoint(pXml->addElement("data"));
      pXml->addAttr("name", datum->first);
      pXml->pushAddPoint(pXml->addElement("value"));
      pXml->addAttr("type", datum->second.getTypeName());
      if (!datum->second.toXml(pXml))
      {
         return false;
      }
      pXml->popAddPoint();
      pXml->popAddPoint();
   }
   return true;
}

bool SignatureImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if ((pDocument == NULL) || (DataElementImp::fromXml(pDocument, version) == false))
   {
      return false;
   }

   mData.clear();

   DOMElement* pElement = static_cast<DOMElement*>(pDocument);
   for (DOMNode* pChild = pElement->getFirstChild(); pChild != NULL; pChild = pChild->getNextSibling())
   {
      if (XMLString::equals(pChild->getNodeName(), X("data")))
      {
         DOMElement* pDataElement = static_cast<DOMElement*>(pChild);

         string name = A(pDataElement->getAttribute(X("name")));
         if (name.empty() == true)
         {
            return false;
         }

         for (DOMNode* pGChild = pChild->getFirstChild(); pGChild != NULL; pGChild = pGChild->getNextSibling())
         {
            if (XMLString::equals(pGChild->getNodeName(), X("value")))
            {
               DataVariant value;
               if (value.fromXml(pGChild, version) == false)
               {
                  return false;
               }

               mData[name] = value;
            }
         }
      }
   }

   return true;
}

const string& SignatureImp::getObjectType() const
{
   static string sType("SignatureImp");
   return sType;
}

bool SignatureImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "Signature"))
   {
      return true;
   }

   return DataElementImp::isKindOf(className);
}

bool SignatureImp::isKindOfElement(const string& className)
{
   if ((className == "SignatureImp") || (className == "Signature"))
   {
      return true;
   }

   return DataElementImp::isKindOfElement(className);
}

void SignatureImp::getElementTypes(vector<string>& classList)
{
   classList.push_back("Signature");
   DataElementImp::getElementTypes(classList);
}

const DataVariant& SignatureImp::getData(const string& name) const
{
   map<string, DataVariant>::const_iterator ppData = mData.find(name);
   if (ppData == mData.end())
   {
      return mNullData;
   }
   else
   {
      return ppData->second;
   }
}

void SignatureImp::adoptData(const string& name, DataVariant& data)
{
   setData(name, data, true);
}

void SignatureImp::setData(const string& name, DataVariant& data, bool adopt)
{
   if (adopt)
   {
      mData[name].swap(data);
   }
   else
   {
      mData[name] = data;
   }
   notify(SIGNAL_NAME(Signature, DataChanged), boost::any(
      std::pair<string, DataVariant>(name, data)));
}

const Units* SignatureImp::getUnits(const string& name) const
{
   const SignatureDataDescriptor* pDesc = dynamic_cast<const SignatureDataDescriptor*>(getDataDescriptor());
   if (pDesc != NULL)
   {
      return pDesc->getUnits(name);
   }
   return NULL;
}

set<string> SignatureImp::getDataNames() const
{
   set<string> keys;
   for (map<string, DataVariant>::const_iterator datum = mData.begin(); datum != mData.end(); ++datum)
   {
      keys.insert(datum->first);
   }
   return keys;
}

set<string> SignatureImp::getUnitNames() const
{
   const SignatureDataDescriptor* pDesc = dynamic_cast<const SignatureDataDescriptor*>(getDataDescriptor());
   if (pDesc != NULL)
   {
      return pDesc->getUnitNames();
   }
   return set<string>();
}
