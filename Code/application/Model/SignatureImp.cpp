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
#include "SignatureImp.h"
#include "UnitsImp.h"

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
         pSignature->setData(dataIter->first, dataIter->second);
      }

      map<string, boost::shared_ptr<UnitsImp> >::const_iterator unitsIter;
      for (unitsIter = mUnits.begin(); unitsIter != mUnits.end(); ++unitsIter)
      {
         pSignature->setUnits(unitsIter->first, unitsIter->second.get());
      }
   }

   return pElement;
}

bool SignatureImp::toXml(XMLWriter* pXml) const
{
   if(!DataElementImp::toXml(pXml))
   {
      return false;
   }
   for(map<string, DataVariant>::const_iterator datum = mData.begin(); datum != mData.end(); ++datum)
   {
      pXml->pushAddPoint(pXml->addElement("data"));
      pXml->addAttr("name", datum->first);
      pXml->pushAddPoint(pXml->addElement("value"));
      pXml->addAttr("type", datum->second.getTypeName());
      if(!datum->second.toXml(pXml))
      {
         return false;
      }
      pXml->popAddPoint();
      map<string, boost::shared_ptr<UnitsImp> >::const_iterator units = mUnits.find(datum->first);
      if(units != mUnits.end())
      {
         pXml->pushAddPoint(pXml->addElement("units"));
         if(!units->second->toXml(pXml))
         {
            return false;
         }
         pXml->popAddPoint();
      }
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
   mUnits.clear();

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
            else if (XMLString::equals(pGChild->getNodeName(), X("units")))
            {
               boost::shared_ptr<UnitsImp> pNewUnits(new UnitsImp);
               if (pNewUnits->fromXml(pGChild, version) == false)
               {
                  return false;
               }

               mUnits[name] = pNewUnits;
            }
         }
      }
   }

   return true;
}

const string& SignatureImp::getObjectType() const
{
   static string type("SignatureImp");
   return type;
}

bool SignatureImp::isKindOf(const string& className) const
{
   if ((className == getObjectType())  || (className == "Signature"))
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

const DataVariant &SignatureImp::getData(string name) const
{
   map<string,DataVariant>::const_iterator ppData = mData.find(name);
   if (ppData == mData.end())
   {
      return mNullData;
   }
   else
   {
      return ppData->second;
   }
}

void SignatureImp::setData(std::string name, const DataVariant &data)
{
   mData[name] = data;
   notify(SIGNAL_NAME(Signature, DataChanged), boost::any(
      std::pair<string,DataVariant>(name, data)));
}

const Units *SignatureImp::getUnits(string name) const
{
   map<string,boost::shared_ptr<UnitsImp> >::const_iterator ppUnits = mUnits.find(name);
   if (ppUnits == mUnits.end())
   {
      boost::shared_ptr<UnitsImp> pUnits(new UnitsImp);
      const_cast<SignatureImp*>(this)->mUnits[name] = pUnits;
      return pUnits.get();
   }
   else
   {
      return ppUnits->second.get();
   }
}

void SignatureImp::setUnits(string name, const Units *pUnits)
{
   map<string,boost::shared_ptr<UnitsImp> >::const_iterator ppUnits = mUnits.find(name);
   if (ppUnits == mUnits.end())
   {
      boost::shared_ptr<UnitsImp> pNewUnits(new UnitsImp);
      *pNewUnits = *static_cast<const UnitsImp*>(pUnits);
      mUnits[name] = pNewUnits;
   }
   else
   {
      *(ppUnits->second) = *static_cast<const UnitsImp*>(pUnits);
   }
   notify(SIGNAL_NAME(Signature, UnitsChanged), boost::any(
      std::pair<string,const Units*>(name,pUnits)));
}

set<string> SignatureImp::getDataNames() const
{
   set<string> keys;
   for(map<string,DataVariant>::const_iterator datum = mData.begin(); datum != mData.end(); ++datum)
   {
      keys.insert(datum->first);
   }
   return keys;
}

set<string> SignatureImp::getUnitNames() const
{
   set<string> keys;
   for(map<string,boost::shared_ptr<UnitsImp> >::const_iterator unit = mUnits.begin(); unit != mUnits.end(); ++unit)
   {
      keys.insert(unit->first);
   }
   return keys;
}
