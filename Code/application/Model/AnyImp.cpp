/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AnyData.h"
#include "AnyImp.h"
#include "DataElement.h"
#include "SessionItemDeserializer.h"
#include "SessionItemSerializer.h"
#include "Slot.h"

XERCES_CPP_NAMESPACE_USE
using namespace std;

AnyImp::AnyImp(const string& dataType, const DataDescriptorImp& descriptor, const string& id) :
   DataElementImp(descriptor, id),
   mpData(NULL),
   mDataType(dataType)
{
}

AnyImp::~AnyImp()
{
   delete mpData;
}

void AnyImp::setData(AnyData* pData)
{
   if (pData == mpData)
   {
      return;
   }

   if (mpData != NULL)
   {
      Subject* pSubject = dynamic_cast<Subject*>(mpData);
      if (pSubject != NULL)
      {
         pSubject->detach(SIGNAL_NAME(Subject, Modified),
                           Signal(dynamic_cast<Subject*>(this),
                           SIGNAL_NAME(Subject, Modified)));
      }
      delete mpData;
   }

   mpData = pData;
   Subject* pSubject = dynamic_cast<Subject*>(mpData);
   if (pSubject != NULL)
   {
      pSubject->attach(SIGNAL_NAME(Subject, Modified),
                        Signal(dynamic_cast<Subject*>(this),
                        SIGNAL_NAME(Subject, Modified)));
   }
   notify(SIGNAL_NAME(Subject, Modified), boost::any(mpData));
}

AnyData* AnyImp::getData()
{
   return mpData;
}

const AnyData* AnyImp::getData() const
{
   return mpData;
}

DataElement* AnyImp::copy(const string& name, DataElement* pParent) const
{
   AnyData* pDataCopy = NULL;
   if (mpData != NULL)
   {
      pDataCopy = mpData->copy();
      if (pDataCopy == NULL)
      {
         return NULL;
      }
   }

   DataElement* pElement = DataElementImp::copy(name, pParent);

   AnyImp* pAny = dynamic_cast<AnyImp*>(pElement);
   if (pAny != NULL)
   {
      pAny->setData(pDataCopy);
   }

   return pElement;
}

bool AnyImp::serialize(SessionItemSerializer &serializer) const
{
   if (!DataElementImp::serialize(serializer))
   {
      return false;
   }

   if (mpData != NULL)
   {
      serializer.endBlock();
      return mpData->serialize(serializer);
   }

   return true;
}

bool AnyImp::deserialize(SessionItemDeserializer &deserializer)
{
   if (!DataElementImp::deserialize(deserializer))
   {
      return false;
   }

   if (mpData != NULL)
   {
      deserializer.nextBlock();
      return mpData->deserialize(deserializer);
   }

   return true;
}

bool AnyImp::toXml(XMLWriter* pXml) const
{
   if (!DataElementImp::toXml(pXml))
   {
      return false;
   }
   pXml->addAttr("type", mDataType);
   return true;
}

bool AnyImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (!DataElementImp::fromXml(pDocument, version))
   {
      return false;
   }
   mDataType = A(static_cast<DOMElement*>(pDocument)->getAttribute(X("type")));
   return true;
}

const string& AnyImp::getObjectType() const
{
   return mDataType;
}

bool AnyImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "Any") || (className == mDataType))
   {
      return true;
   }

   return DataElementImp::isKindOf(className);
}

bool AnyImp::isKindOfElement(const string& className)
{
   if ((className == "AnyImp") || (className == "Any"))
   {
      return true;
   }

   return DataElementImp::isKindOfElement(className);
}

void AnyImp::getElementTypes(vector<string>& classList)
{
   classList.push_back("Any");
   DataElementImp::getElementTypes(classList);
}
