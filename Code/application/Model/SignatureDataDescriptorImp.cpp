/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "DataDescriptor.h"
#include "ObjectResource.h"
#include "SignatureDataDescriptor.h"
#include "SignatureDataDescriptorImp.h"
#include "Units.h"

XERCES_CPP_NAMESPACE_USE

SignatureDataDescriptorImp::SignatureDataDescriptorImp(const std::string& name, const std::string& type,
   DataElement* pParent) :
   DataDescriptorImp(name, type, pParent)
{}

SignatureDataDescriptorImp::SignatureDataDescriptorImp(const std::string& name, const std::string& type,
   const std::vector<std::string>& parent) :
   DataDescriptorImp(name, type, parent)
{}

SignatureDataDescriptorImp::~SignatureDataDescriptorImp()
{}

void SignatureDataDescriptorImp::setUnits(const std::string& name, const Units* pUnits)
{
   if (name.empty() || pUnits == NULL)
   {
      return;
   }
   const UnitsImp* pImp = dynamic_cast<const UnitsImp*>(pUnits);
   VERIFYNRV(pImp != NULL);
   std::map<std::string, UnitsImp>::iterator it = mUnits.find(name);
   if (it == mUnits.end())
   {
      mUnits[name] = *pImp;
   }
   else
   {
      if ((it->second) == *pImp)
      {
         return;
      }
      it->second = *pImp;
   }
   notify(SIGNAL_NAME(SignatureDataDescriptor, UnitsChanged), boost::any(
      std::pair<std::string, const Units*>(name, getUnits(name))));
}

const Units* SignatureDataDescriptorImp::getUnits(const std::string& name) const
{
   std::map<std::string, UnitsImp>::const_iterator it = mUnits.find(name);
   if (it != mUnits.end())
   {
      return &it->second;
   }

   return NULL;
}

DataDescriptor* SignatureDataDescriptorImp::copy(const std::string& name, DataElement* pParent) const
{
   SignatureDataDescriptor* pDescriptor =
      dynamic_cast<SignatureDataDescriptor*>(DataDescriptorImp::copy(name, pParent));
   if (pDescriptor != NULL)
   {
      for (std::map<std::string, UnitsImp>::const_iterator it = mUnits.begin();
         it != mUnits.end(); ++it)
      {
         pDescriptor->setUnits(it->first, &it->second);
      }
   }

   return pDescriptor;
}

DataDescriptor* SignatureDataDescriptorImp::copy(const std::string& name, const std::vector<std::string>& parent) const
{
   SignatureDataDescriptor* pDescriptor =
      dynamic_cast<SignatureDataDescriptor*>(DataDescriptorImp::copy(name, parent));
   if (pDescriptor != NULL)
   {
      for (std::map<std::string, UnitsImp>::const_iterator it = mUnits.begin();
         it != mUnits.end(); ++it)
      {
         pDescriptor->setUnits(it->first, &it->second);
      }
   }

   return pDescriptor;
}

bool SignatureDataDescriptorImp::toXml(XMLWriter* pXml) const
{
   if (pXml == NULL)
   {
      return false;
   }

   if (!DataDescriptorImp::toXml(pXml))
   {
      return false;
   }

   for (std::map<std::string, UnitsImp>::const_iterator it = mUnits.begin();
      it != mUnits.end(); ++it)
   {
      pXml->pushAddPoint(pXml->addElement("signatureUnits"));
      pXml->addAttr("componentName", it->first);
      if (!it->second.toXml(pXml))
      {
         return false;
      }
      pXml->popAddPoint();
   }

   return true;
}

bool SignatureDataDescriptorImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   VERIFY(pDocument != NULL);

   if (!DataDescriptorImp::fromXml(pDocument, version))
   {
      return false;
   }

   mUnits.clear();

   for (DOMNode* pChild = pDocument->getFirstChild(); pChild != NULL; pChild = pChild->getNextSibling())
   {
      if (XMLString::equals(pChild->getNodeName(), X("signatureUnits")))
      {
         DOMElement* pElement = static_cast<DOMElement*>(pChild);
         std::string name = A(pElement->getAttribute(X("componentName")));
         if (name.empty())
         {
            return false;
         }
         UnitsImp newUnits;
         if (newUnits.fromXml(pChild, version) == false)
         {
            return false;
         }
         mUnits[name] = newUnits;
      }
   }

   return true;
}

const std::string& SignatureDataDescriptorImp::getObjectType() const
{
   static std::string sType("SignatureDataDescriptorImp");
   return sType;
}

bool SignatureDataDescriptorImp::isKindOf(const std::string& className) const
{
   if ((className == getObjectType()) || (className == "SignatureDataDescriptor"))
   {
      return true;
   }

   return DataDescriptorImp::isKindOf(className);
}

void SignatureDataDescriptorImp::getDataDescriptorTypes(std::vector<std::string>& classList)
{
   classList.push_back("SignatureDataDescriptor");
   DataDescriptorImp::getDataDescriptorTypes(classList);
}

bool SignatureDataDescriptorImp::isKindOfDataDescriptor(const std::string& className)
{
   if ((className == "SignatureDataDescriptorImp") || (className == "SignatureDataDescriptor"))
   {
      return true;
   }

   return DataDescriptorImp::isKindOfDataDescriptor(className);
}

std::set<std::string> SignatureDataDescriptorImp::getUnitNames() const
{
   std::set<std::string> keys;
   for (std::map<std::string, UnitsImp>::const_iterator unit = mUnits.begin(); unit != mUnits.end(); ++unit)
   {
      keys.insert(unit->first);
   }
   return keys;
}
