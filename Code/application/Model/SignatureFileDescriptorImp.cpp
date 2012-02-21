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
#include "ObjectResource.h"
#include "SignatureFileDescriptor.h"
#include "SignatureFileDescriptorImp.h"
#include "Units.h"

XERCES_CPP_NAMESPACE_USE

SignatureFileDescriptorImp::SignatureFileDescriptorImp()
{}

SignatureFileDescriptorImp::~SignatureFileDescriptorImp()
{}

void SignatureFileDescriptorImp::setUnits(const std::string& name, const Units* pUnits)
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
   notify(SIGNAL_NAME(SignatureFileDescriptor, UnitsChanged), boost::any(
      std::pair<std::string, const Units*>(name, getUnits(name))));
}

const Units* SignatureFileDescriptorImp::getUnits(const std::string& name) const
{
   std::map<std::string, UnitsImp>::const_iterator it = mUnits.find(name);
   if (it != mUnits.end())
   {
      return &it->second;
   }

   return NULL;
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

bool SignatureFileDescriptorImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   VERIFY(pDocument != NULL);

   if (!FileDescriptorImp::fromXml(pDocument, version))
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
   for (std::map<std::string, UnitsImp>::const_iterator unit = mUnits.begin();
      unit != mUnits.end(); ++unit)
   {
      keys.insert(unit->first);
   }
   return keys;
}

SignatureFileDescriptorImp& SignatureFileDescriptorImp::operator =(const SignatureFileDescriptorImp& descriptor)
{
   if (this != &descriptor)
   {
      FileDescriptorImp::operator =(descriptor);

      mUnits = descriptor.mUnits;
      notify(SIGNAL_NAME(Subject, Modified));
   }

   return *this;
}
