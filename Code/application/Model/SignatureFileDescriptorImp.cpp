/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "FileDescriptor.h"
#include "SignatureFileDescriptor.h"
#include "SignatureFileDescriptorImp.h"
#include "UnitsAdapter.h"

XERCES_CPP_NAMESPACE_USE

SignatureFileDescriptorImp::SignatureFileDescriptorImp()
{}

SignatureFileDescriptorImp::~SignatureFileDescriptorImp()
{
   clearUnits();
}

void SignatureFileDescriptorImp::setUnits(const std::string& name, const Units* pUnits)
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
         Slot(this, &SignatureFileDescriptorImp::notifyUnitsModified)));

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

   notify(SIGNAL_NAME(SignatureFileDescriptor, UnitsChanged),
      boost::any(std::pair<std::string, const Units*>(name, pMapUnits)));
}

const Units* SignatureFileDescriptorImp::getUnits(const std::string& name) const
{
   std::map<std::string, Units*>::const_iterator it = mUnits.find(name);
   if (it != mUnits.end())
   {
      return it->second;
   }

   return NULL;
}

bool SignatureFileDescriptorImp::clone(const FileDescriptor* pFileDescriptor)
{
   const SignatureFileDescriptorImp* pSignatureFileDescriptor =
      dynamic_cast<const SignatureFileDescriptorImp*>(pFileDescriptor);
   if ((pSignatureFileDescriptor == NULL) || (FileDescriptorImp::clone(pFileDescriptor) == false))
   {
      return false;
   }

   if (pSignatureFileDescriptor != this)
   {
      std::set<std::string> unitNames = pSignatureFileDescriptor->getUnitNames();
      for (std::set<std::string>::const_iterator iter = unitNames.begin(); iter != unitNames.end(); ++iter)
      {
         std::string name = *iter;
         if (name.empty() == false)
         {
            setUnits(name, pSignatureFileDescriptor->getUnits(name));
         }
      }
   }

   return true;
}

bool SignatureFileDescriptorImp::toXml(XMLWriter* pXml) const
{
   if (pXml == NULL)
   {
      return false;
   }

   if (!FileDescriptorImp::toXml(pXml))
   {
      return false;
   }

   pXml->addAttr("type", "SignatureFileDescriptor");
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

bool SignatureFileDescriptorImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   VERIFY(pDocument != NULL);

   if (!FileDescriptorImp::fromXml(pDocument, version))
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
            Slot(this, &SignatureFileDescriptorImp::notifyUnitsModified)));

         mUnits[name] = pUnits;
      }
   }

   return true;
}

const std::string& SignatureFileDescriptorImp::getObjectType() const
{
   static std::string sType("SignatureFileDescriptorImp");
   return sType;
}

bool SignatureFileDescriptorImp::isKindOf(const std::string& className) const
{
   if ((className == getObjectType()) || (className == "SignatureFileDescriptor"))
   {
      return true;
   }

   return FileDescriptorImp::isKindOf(className);
}

void SignatureFileDescriptorImp::getFileDescriptorTypes(std::vector<std::string>& classList)
{
   classList.push_back("SignatureFileDescriptor");
   FileDescriptorImp::getFileDescriptorTypes(classList);
}

bool SignatureFileDescriptorImp::isKindOfFileDescriptor(const std::string& className)
{
   if ((className == "SignatureFileDescriptorImp") || (className == "SignatureFileDescriptor"))
   {
      return true;
   }

   return FileDescriptorImp::isKindOfFileDescriptor(className);
}

std::set<std::string> SignatureFileDescriptorImp::getUnitNames() const
{
   std::set<std::string> keys;
   for (std::map<std::string, Units*>::const_iterator unit = mUnits.begin(); unit != mUnits.end(); ++unit)
   {
      keys.insert(unit->first);
   }
   return keys;
}

void SignatureFileDescriptorImp::notifyUnitsModified(Subject& subject, const std::string& signal,
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

void SignatureFileDescriptorImp::clearUnits()
{
   for (std::map<std::string, Units*>::iterator iter = mUnits.begin(); iter != mUnits.end(); ++iter)
   {
      UnitsAdapter* pUnits = dynamic_cast<UnitsAdapter*>(iter->second);
      if (pUnits != NULL)
      {
         VERIFYNR(pUnits->detach(SIGNAL_NAME(Subject, Modified),
            Slot(this, &SignatureFileDescriptorImp::notifyUnitsModified)));
         delete pUnits;
      }
   }

   mUnits.clear();
}
