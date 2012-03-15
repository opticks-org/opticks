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
#include "SignatureDataDescriptor.h"
#include "SignatureDataDescriptorImp.h"
#include "UnitsAdapter.h"

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
{
   clearUnits();
}

void SignatureDataDescriptorImp::setUnits(const std::string& name, const Units* pUnits)
{
   if (name.empty() || pUnits == NULL)
   {
      return;
   }

   Units* pMapUnits = NULL;

   std::map<std::string, Units*>::iterator it = mUnits.find(name);
   if (it == mUnits.end())
   {
      pMapUnits = new UnitsAdapter();
      pMapUnits->setUnits(pUnits);

      // Attach to the units object to notify when it changes
      VERIFYNR(pMapUnits->attach(SIGNAL_NAME(Subject, Modified),
         Slot(this, &SignatureDataDescriptorImp::notifyUnitsModified)));

      mUnits[name] = pMapUnits;
   }
   else
   {
      pMapUnits = it->second;
      VERIFYNRV(pMapUnits != NULL);

      if (pMapUnits->compare(pUnits) == true)
      {
         return;
      }

      pMapUnits->setUnits(pUnits);
   }

   notify(SIGNAL_NAME(SignatureDataDescriptor, UnitsChanged),
      boost::any(std::pair<std::string, const Units*>(name, pMapUnits)));
}

const Units* SignatureDataDescriptorImp::getUnits(const std::string& name) const
{
   std::map<std::string, Units*>::const_iterator it = mUnits.find(name);
   if (it != mUnits.end())
   {
      return it->second;
   }

   return NULL;
}

DataDescriptor* SignatureDataDescriptorImp::copy(const std::string& name, DataElement* pParent) const
{
   SignatureDataDescriptor* pDescriptor =
      dynamic_cast<SignatureDataDescriptor*>(DataDescriptorImp::copy(name, pParent));
   if (pDescriptor != NULL)
   {
      for (std::map<std::string, Units*>::const_iterator it = mUnits.begin(); it != mUnits.end(); ++it)
      {
         pDescriptor->setUnits(it->first, it->second);
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
      for (std::map<std::string, Units*>::const_iterator it = mUnits.begin(); it != mUnits.end(); ++it)
      {
         pDescriptor->setUnits(it->first, it->second);
      }
   }

   return pDescriptor;
}

bool SignatureDataDescriptorImp::clone(const DataDescriptor* pDescriptor)
{
   const SignatureDataDescriptorImp* pSignatureDescriptor =
      dynamic_cast<const SignatureDataDescriptorImp*>(pDescriptor);
   if ((pSignatureDescriptor == NULL) || (DataDescriptorImp::clone(pDescriptor) == false))
   {
      return false;
   }

   if (pSignatureDescriptor != this)
   {
      std::set<std::string> unitNames = pSignatureDescriptor->getUnitNames();
      for (std::set<std::string>::const_iterator iter = unitNames.begin(); iter != unitNames.end(); ++iter)
      {
         std::string name = *iter;
         if (name.empty() == false)
         {
            setUnits(name, pSignatureDescriptor->getUnits(name));
         }
      }
   }

   return true;
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

   for (std::map<std::string, Units*>::const_iterator it = mUnits.begin(); it != mUnits.end(); ++it)
   {
      pXml->pushAddPoint(pXml->addElement("signatureUnits"));
      pXml->addAttr("componentName", it->first);
      if (!it->second->toXml(pXml))
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

   clearUnits();

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

         Units* pUnits = new UnitsAdapter();
         if (pUnits->fromXml(pChild, version) == false)
         {
            return false;
         }

         // Attach to the units object to notify when it changes
         VERIFYNR(pUnits->attach(SIGNAL_NAME(Subject, Modified),
            Slot(this, &SignatureDataDescriptorImp::notifyUnitsModified)));

         mUnits[name] = pUnits;
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
   for (std::map<std::string, Units*>::const_iterator unit = mUnits.begin(); unit != mUnits.end(); ++unit)
   {
      keys.insert(unit->first);
   }
   return keys;
}

void SignatureDataDescriptorImp::notifyUnitsModified(Subject& subject, const std::string& signal,
                                                     const boost::any& data)
{
   Units* pUnits = dynamic_cast<Units*>(&subject);
   if (pUnits == NULL)
   {
      return;
   }

   for (std::map<std::string, Units*>::iterator iter = mUnits.begin(); iter != mUnits.end(); ++iter)
   {
      if (pUnits == iter->second)
      {
         notify(SIGNAL_NAME(Subject, Modified));
         break;
      }
   }
}

void SignatureDataDescriptorImp::clearUnits()
{
   for (std::map<std::string, Units*>::iterator iter = mUnits.begin(); iter != mUnits.end(); ++iter)
   {
      UnitsAdapter* pUnits = dynamic_cast<UnitsAdapter*>(iter->second);
      if (pUnits != NULL)
      {
         VERIFYNR(pUnits->detach(SIGNAL_NAME(Subject, Modified),
            Slot(this, &SignatureDataDescriptorImp::notifyUnitsModified)));
         delete pUnits;
      }
   }

   mUnits.clear();
}
